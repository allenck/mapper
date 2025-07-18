#include "editconnectionsdlg.h"
#include "ui_editconnectionsdlg.h"
#include <QCompleter>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include <QClipboard>
#include <QApplication>
//#include <unistd.h>
#include "vptr.h"
#include "mymessagebox.h"
#include "exceptions.h"
#include "exportsql.h"
#include "odbcutil.h"

EditConnectionsDlg::EditConnectionsDlg( QWidget *parent) :
  QDialog(parent),
  ui(new Ui::editConnectionsDlg)
{
    ui->setupUi(this);
    ui->lblHelp->setText("");
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
    ui->lblHelp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    config = Configuration::instance();
    odbcUtil = ODBCUtil::instance();
    currCity = config->currCity;
    this->setWindowTitle(tr("Edit Connections "));
    drivers = QSqlDatabase::drivers();
    ui->cbODBCDsn->setEditable(false);

   // connection that is being edited
   connection = currCity->connections.at(currCity->curConnectionId);

   refreshCities();


   dbTypes <<"MySql"<<"MsSql"<<"Sqlite"<<"PostgreSQL";  // currently supported database types.
   for(int i=0; i < dbTypes.count(); i++)
   {
     ui->cbDbType->addItem(dbTypes.at(i), (DBTYPE)i);
   }
   ui->cbDriverType->clear();
   ui->cbDriverType->addItems(drivers);
   ui->cbConnect->clear();
   ui->cbConnect->addItem("Local");
   ui->cbConnect->addItem("Direct");
   ui->cbConnect->addItem("ODBC");

   ui->cbDbType->setCurrentText(connection->servertype());
   setupComboBoxes(connection->servertype());

//   int index = 0;
   for(int i=0; i < config->currCity->connections.count(); i++)
   {
     if(config->currCity->connections.at(i)->cityName().isEmpty()) //get well code!
       config->currCity->connections.at(i)->setCityName(config->currCity->name());
     City* selectedCity = config->cityMap.value(ui->cbCities->currentText());
     Connection* connection = selectedCity->connections.at(i);
     setControls(connection->connectionType());
     ui->cbConnections->addItem(connection->description(),VPtr<Connection>::asQVariant(connection));
   }

   onDbTypeChanged(connection->servertype());

//   ui->cbDbType->setCurrentText(connection->servertype());

//   QString dbType =  ui->cbDbType->currentText();
//   setupComboBoxes(dbType);

   connect(ui->cbDriverType, SIGNAL(currentTextChanged(QString)), this, SLOT(cbDriverTypeSelectionChanged(QString)));

//   connect(ui->cbConnect, &QComboBox::currentTextChanged, [=]{
//    setControls(ui->cbConnect->currentText());
//    if(ui->cbConnect->currentText() == "ODBC")
//        setControls(ui->cbConnect->currentText());
//    });

    connect(ui->cbDbType,SIGNAL(currentTextChanged(QString)), this,  SLOT(onDbTypeChanged(QString)));

    connect(ui->cbCities, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitiesSelectionChanged(int)));
    connect(ui->cbCities, SIGNAL(editTextChanged(QString)), this, SLOT(cbCitiesTextChanged(QString)));
    connect(ui->cbCities->lineEdit(), SIGNAL(editingFinished()), this, SLOT(cbCitiesLeave()));

    ui->cbConnections->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->cbConnections, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(cbConnections_contextMenuRequested(QPoint)));
    connect(ui->cbConnections, SIGNAL(currentIndexChanged(int)), this, SLOT(cbConnectionsSelectionChanged(int)));
    connect(ui->cbConnections, SIGNAL(editTextChanged(QString)), this, SLOT(cbConnectionsTextChanged(QString)));
    connect(ui->cbConnections->lineEdit(), &QLineEdit::editingFinished, [=]{
       connection->setDescription(ui->cbConnections->lineEdit()->text());
       connection->setDirty(true);
    });
    connect(ui->btnTest,SIGNAL(clicked(bool)), this, SLOT(btnTestClicked()));
    connect(ui->btnNew, &QPushButton::clicked, [=]{
    newConnection();
    });
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(btnCancelClicked()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(btnSaveClicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDeleteClicked()));
    connect(ui->btnAbort, &QPushButton::clicked, [=]{exit(EXIT_FAILURE);});
    //connect(ui->cbConnections->lineEdit(), SIGNAL(editingFinished()),this, SLOT(cbConnectionsLeave()));
    connect(ui->txtHost, SIGNAL(editingFinished()), this, SLOT(txtHostLeave()));
    connect(ui->txtPort, SIGNAL(editingFinished()), this, SLOT(txtPortLeave()));
    connect(ui->txtPWD, SIGNAL(editingFinished()), this, SLOT(txtPwdLeave()));
    connect(ui->txtSqliteFileName, SIGNAL(textChanged(QString)), this, SLOT(txtDsnTextChanged(QString)));
    connect(ui->txtSqliteFileName, SIGNAL(editingFinished()), this, SLOT(ontxtSqliteFileName_editingFinished()));
    ui->tbView->setIcon(QIcon(":/show-password.png"));
    ui->txtPWD->setEchoMode(QLineEdit::Password);


    connect(ui->tbView, &QToolButton::clicked, [=](bool checked){
    if(checked)
    {
        ui->tbView->setIcon(QIcon(":/show-password.png"));
        ui->txtPWD->setEchoMode(QLineEdit::Password);
    }
    else
    {
        ui->tbView->setIcon(QIcon(":/hidden.png"));
        ui->txtPWD->setEchoMode(QLineEdit::Normal);
    }
    });

    connect(ui->tbReload, &QToolButton::clicked, [=]{
       openTestDb();
      if(db.isOpen())
          populateDatabases();
      else
      {
          ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
          ui->lblHelp->setText(tr("not open!"));
      }
    });

    connect(ui->cbUseDatabase->lineEdit(), &QLineEdit::editingFinished, [=](){
        QString database = ui->cbUseDatabase->currentText();
        if(ui->cbDbType->currentText() == "PostgreSQL" && ui->cbConnect->currentText() == "Direct")
        {
            db.close();
            db.setDatabaseName(database);
            ui->cbUseDatabase->setCurrentText(database);
            if(!db.open())
            {
                ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
                ui->lblHelp->setText(tr("not open!"));
            }
            return;
        }
        if(ui->cbDbType->currentText() == "MsSql" || ui->cbDbType->currentText() == "MySql")
        {
          if(db.open() && !database.isEmpty())
          {
             QSqlQuery query = QSqlQuery(db);
             QString commandText = "use " + database;
             if(!query.exec(commandText))
             {
              if(query.lastError().driverText().startsWith("Unknown database"))
              {
                int rslt = MyMessageBox::question(nullptr, tr("Create database?"), tr("The database %1 does not exist on the server."
                           "\nDo you wish to create it?").arg(database));
                if(rslt == QMessageBox::Yes)
                    SQL::instance()->createSqlDatabase(database, db, ui->cbDbType->currentText());
              }
              else
              {
               int rslt = MyMessageBox::critical(nullptr, tr("Sql Error"), tr("An SqL error has occurred.\n"
                                               "Sql error:%1\nquery: \"%2\"").arg(query.lastError().text()).arg(query.lastQuery()),
                                               QMessageBox::Retry|QMessageBox::Ignore|QMessageBox::Abort);
                return;
              }
             }
          }
        }
   });

   connect(ui->cbODBCDsn, SIGNAL(currentIndexChanged(int)), this, SLOT(cbODBCDsn_currentIndex_changed(int)));

   ui->cbConnections->setCurrentIndex(ui->cbConnections->
                                       findText(connection->description()));
   cbConnectionsSelectionChanged(ui->cbConnections->currentIndex());

   timer = new QTimer(this);
   timer->setInterval(1000);
   connect(timer, SIGNAL(timeout()), this, SLOT(quickProcess()));
}

void EditConnectionsDlg::setupComboBoxes(QString txt)
{

 if(ui->cbDbType->currentText()=="Sqlite")
 {
  ui->cbDriverType->setCurrentText("QSQLITE");
  ui->txtSqliteFileName->setText(connection->sqlite_fileName());
 }
 else if(ui->cbDbType->currentText()=="MySql")
 {
  ui->cbConnect->addItem("Direct");
  ui->cbConnect->addItem("ODBC");
  ui->cbDriverType->setCurrentText(connection->driver());
  ui->txtDefaultDb->setText(connection->defaultSqlDatabase());
  ui->cbUseDatabase->setCurrentText(connection->database());
 }
 else if(txt == "MsSql")// MsSql
 {
  ui->cbDriverType->setCurrentText(connection->driver());
  ui->txtDefaultDb->setText(connection->defaultSqlDatabase());
  ui->cbUseDatabase->setCurrentText(connection->database());
 }
 else
  return;

   setControls(ui->cbConnect->currentText());

}

void EditConnectionsDlg::onDbTypeChanged(QString dbType)
{
 disconnect(ui->cbDbType,SIGNAL(currentTextChanged(QString)), this, SLOT(onDbTypeChanged(QString)));
 disconnect(ui->cbDriverType, SIGNAL(currentTextChanged(QString)),this,SLOT(cbDriverTypeSelectionChanged(QString)));
 if(dbType == "Sqlite")
 {
     ui->label_2->setVisible(true);
     ui->txtSqliteFileName->setVisible(true);
     ui->tbBrowse->setVisible(true);
  foreach (QString driver, drivers) {
   int index = ui->cbDriverType->findText(driver);
   setComboBoxItemEnabled(ui->cbDriverType,index, driver == "QSQLITE" || driver == "QSQLITE3");
  }

     setComboBoxItemEnabled(ui->cbConnect, 0, true); // Local"
     setComboBoxItemEnabled(ui->cbConnect, 1, false); // Direct");
     setComboBoxItemEnabled(ui->cbConnect, 2, false); // ODBC");
     ui->cbConnect->setCurrentText("Local");
     ui->cbDriverType->setCurrentText("QSQLITE");
     setControls("Local");
 }
 else if(dbType == "MySql")
 {
     ui->label_2->setVisible(false);
     ui->txtSqliteFileName->setVisible(false);
     ui->tbBrowse->setVisible(false);

  ui->cbODBCDsn->setVisible(true);
  odbcUtil->fillDSNCombo(ui->cbODBCDsn,"MySql");
  foreach (QString driver, drivers) {
   int index = ui->cbDriverType->findText(driver);
   setComboBoxItemEnabled(ui->cbDriverType,index,
                          driver == "QMYSQL" || driver == "QMYSQL3"
                          || driver == "QMARIADB" || driver == "QODBC"
                          || driver == "QODBC3");
  }

     setComboBoxItemEnabled(ui->cbConnect, 0, false); // Local"
#ifdef Q_OS_WIN
     setComboBoxItemEnabled(ui->cbConnect, 1, false); // Direct");
#else
     setComboBoxItemEnabled(ui->cbConnect, 1, true); // Direct");
#endif
     setComboBoxItemEnabled(ui->cbConnect, 2, ui->cbDriverType->currentText() == "ODBC"
                            ||ui->cbDriverType->currentText() == "ODBC"); // ODBC3");
 }
 else if(dbType == "MsSql")
 {
     ui->label_2->setVisible(false);
     ui->txtSqliteFileName->setVisible(false);
     ui->tbBrowse->setVisible(false);
     ui->cbODBCDsn->setVisible(true);
     odbcUtil->fillDSNCombo(ui->cbODBCDsn,"MsSql");
  foreach (QString driver, drivers) {
   int index = ui->cbDriverType->findText(driver);
   setComboBoxItemEnabled(ui->cbDriverType,index,
                         driver == "QODBC" || driver == "QODBC3" );
  }
     setComboBoxItemEnabled(ui->cbConnect, 0, false); // Local"
     setComboBoxItemEnabled(ui->cbConnect, 1, false); // Direct");
     setComboBoxItemEnabled(ui->cbConnect, 2, true); // ODBC");
 }
 else if(dbType == "PostgreSQL")
 {
     ui->label_2->setVisible(false);
     ui->txtSqliteFileName->setVisible(false);
     ui->tbBrowse->setVisible(false);
     ui->cbODBCDsn->setVisible(true);
     ui->cbDriverType->setCurrentText("QPSQL");
     ui->cbConnect->setCurrentText("Direct");
     ui->cbDbType->setCurrentText("PostgreSQL");
     odbcUtil->fillDSNCombo(ui->cbODBCDsn,"PostgreSQL");
     foreach (QString driver, drivers) {
         int index = ui->cbDriverType->findText(driver);
         setComboBoxItemEnabled(ui->cbDriverType,index,
                                driver == "QODBC" || driver == "QODBC3" || driver == "QPSQL");
     }
     setComboBoxItemEnabled(ui->cbConnect, 0, false); // Local"
     setComboBoxItemEnabled(ui->cbConnect, 1, true); // Direct");
     setComboBoxItemEnabled(ui->cbConnect, 2, true); // ODBC");
     qApp->processEvents();
 }
 else
 {
  throw IllegalArgumentException("invalid dbType");
 }
 if(dbType != "Sqlite")
 {
  QString driverType = ui->cbDriverType->currentText();
  // driver type must be either QMYSQL QMARIADB or QODBC
  if(!(driverType == "QMYSQL" || driverType == "QMYSQL3" ||
     driverType == "QMARIADB" || driverType == "QODBC" ||
     driverType == "QODBC3"))
  {
   ui->cbDriverType->setCurrentText("QODBC");
   ui->cbConnect->setCurrentText("ODBC");
   setControls("ODBC");
  }
 }
 connect(ui->cbDbType,SIGNAL(currentTextChanged(QString)), this, SLOT(onDbTypeChanged(QString)));
 connect(ui->cbDriverType, SIGNAL(currentTextChanged(QString)),this,SLOT(cbDriverTypeSelectionChanged(QString)));
}

void EditConnectionsDlg::setCity(City* city)
{
    //refreshCities();

    int ix = ui->cbCities->findData(VPtr<City>::asQVariant(city));
    if(ix <0)
    {
     ui->cbCities->addItem(city->name(), VPtr<City>::asQVariant(city));
     ix = ui->cbCities->count()-1;
    }
    ui->cbCities->setCurrentIndex(ix);
    currCity = city;
    connection = city->connections.at(0);

    ui->cbConnections->clear();
    for(int i=0; i < city->connections.count(); i++)
    {
     Connection* connection = city->connections.at(i);
     setControls(connection->connectionType());
     ui->cbConnections->addItem(connection->description(),VPtr<Connection>::asQVariant(connection));
    }
}

EditConnectionsDlg::~EditConnectionsDlg()
{
 delete ui;
}

void EditConnectionsDlg::cbCitiesSelectionChanged(int sel)
{
 Q_UNUSED(sel)
// QString name = ui->cbCities->currentText();
// foreach(City* c, config->cityList)
// {
//  if(c->name() == name)
//  {
   currCity = VPtr<City>::asPtr(ui->cbCities->currentData());
   if(currCity == nullptr)
       return;
   ui->cbConnections->clear();
   //ui->cbConnections->addItem(tr("Add new connection"));
   foreach(Connection* cn, currCity->connections)
   {
    ui->cbConnections->addItem(cn->description(), VPtr<Connection>::asQVariant(cn));
   }
   //ui->cbConnections->setCurrentIndex(currCity->curConnectionId);
   ui->cbConnections->setCurrentText(currCity->connections.at(currCity->curConnectionId)->description());
   cbConnectionsSelectionChanged(currCity->curConnectionId);
   ui->btnNew->setEnabled(true);
   return;
//  }
// }
}

void EditConnectionsDlg::cbCitiesTextChanged(QString text)
{
    bCbCitiesTextChanged = true;
    if(ui->cbCities->isEditable())
        return;
    if(ui->cbCities->currentIndex() < ui->cbCities->count())
    {
        ui->cbCities->setCurrentIndex(ui->cbCities->count()-1);
    }
}


void EditConnectionsDlg::cbCitiesLeave()
{
 if(bCbCitiesTextChanged)
 {
  QString name = ui->cbCities->currentText();

  foreach(City* c, config->cityList)
  {
   // name already present
   if(c->name() == name)
   {
    ui->cbConnections->clear();
    //ui->cbConnections->addItem(tr("Add new connection"));
    ui->txtSqliteFileName->setText("");

    foreach(Connection* cn, c->connections)
    {
     ui->cbConnections->addItem(cn->description(), VPtr<Connection>::asQVariant(cn));
    }
    return;
   }
  }
  // new city
  City* c = new City();
  c->name() = name;
  c->curConnectionId = -1;
  c->curExportConnId = -1;
  c->id = 0;
  ui->cbCities->addItem(name);
  config->cityList.append(c);
  c->id = config->cityList.count()-1;
  ui->cbConnections->clear();
  //ui->cbConnections->addItem(tr("Add new connection"));
  ui->txtSqliteFileName->setText("");
 }
}

void EditConnectionsDlg::cbConnectionsSelectionChanged(int sel)
{
 ui->lblHelp->setText("");
 // ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 if(sel < 0)
  return;
 //ui->btnSave->setText(tr("update"));
 connection = VPtr<Connection>::asPtr(ui->cbConnections->itemData(sel));
 if(connection == nullptr)
  connection = new Connection();
 connectionChanging = true;

 ui->btnDelete->setEnabled(true);

 ui->cbDbType->setCurrentIndex(ui->cbDbType->findText(connection->servertype()));
 ui->cbDriverType->setCurrentIndex(ui->cbDriverType->findText(connection->driver()));
 ui->cbConnect->setCurrentIndex(ui->cbConnect->findText(connection->connectionType()));
 setControls(connection->connectionType());

 if(ui->cbConnect->currentText()== "Local") // Local
 {
  ui->txtSqliteFileName->setText(connection->sqlite_fileName());
 }
 else if(ui->cbConnect->currentText() == "Direct") // Direct
 {
  ui->txtHost->setText(connection->host());
  ui->txtPort->setText(QString::number(connection->port()));
  if(connection->port() < 1)
  {
      if(connection->servertype() == "MsSql")
          ui->txtPort->setText("1443");
      if(connection->servertype() == "MySql")
          ui->txtPort->setText("3306");
      if(connection->servertype() == "PostgreSQL")
          ui->txtPort->setText("5432");
  }
  ui->txtUserId->setText(connection->userId());
  ui->txtPWD->setText(connection->pwd());
  ui->cbODBCDsn->setCurrentIndex(ui->cbODBCDsn->findData(connection->dsn()));
  ui->txtDefaultDb->setText(connection->defaultSqlDatabase());
  ui->cbUseDatabase->setCurrentText(connection->database());
  if(connection->database().isEmpty())
      connection->setDatabase(connection->defaultSqlDatabase());
  if(openTestDb())
  {
      if(ui->cbConnect->currentText() != "Sqlite")
      {
        populateDatabases();
      }
      ui->lblHelp->setStyleSheet("color: green");
      ui->lblHelp->setText(tr("OK database has %1 tables").arg(db.tables().count()));

  }
  ui->cbUseDatabase->setCurrentText(connection->defaultSqlDatabase());
 }
 else
 {
     // ODBC (MsSql, MySql/MariaDb or PostgreSQL
  ui->cbUseDatabase->setCurrentText(connection->defaultSqlDatabase());
  ui->txtDefaultDb->setText(connection->defaultSqlDatabase());
  ui->cbODBCDsn->setVisible(true);
  odbcUtil->fillDSNCombo(ui->cbODBCDsn, ui->cbDbType->currentText());
  ui->cbODBCDsn->setCurrentIndex(ui->cbODBCDsn->findData(connection->dsn()));
  QString ODBCDsn = ui->cbODBCDsn->currentData().toString();
  db = QSqlDatabase::addDatabase(connection->driver(),"testConnection");
  db.setDatabaseName(ODBCDsn);
  db.setUserName(connection->userId());
  db.setPassword(connection->pwd());


  //QList<QPair<QString,QString>> pairs = odbcPairMap.value(ui->cbODBCDsn->currentData().toString());
  QMap<QString,QString> map = odbcPairMap.value(ui->cbODBCDsn->currentData().toString());
  //for(std::pair <QString,QString> pair : pairs)
  QMapIterator<QString,QString> iter(map);
  while(iter.hasNext())
  {
      iter.next();
      QString key = iter.key();
      QString val = iter.value();
      if(key.compare("port",Qt::CaseInsensitive)==0)
      {
          ui->txtPort->setPlaceholderText(val);
          //db.setPort(pair.second.toInt());
      }
      if(key.compare("PWD",Qt::CaseInsensitive)==0)
      {
          ui->txtPWD->setPlaceholderText("dsn password");
      }
      if(key.compare("SERVER",Qt::CaseInsensitive)==0)
      {
          ui->txtHost->setPlaceholderText(val);
          //db.setHostName(pair.second);
      }
      if(key.compare("UID",Qt::CaseInsensitive)==0)
      {
          ui->txtUserId->setPlaceholderText(val);
          //db.setUserName(pair.second);
      }
      // if(testConnection())
      // {
      //     QStringList list = SQL::instance()->showMySqlDatabases(db);
      //     QCompleter* completer = new QCompleter(list);
      //     ui->txtUseDatabase->setCompleter(completer);
      //     ui->lblHelp->setStyleSheet("color: green");
      // }
  }

  if(connection->servertype() == "PostgreSQL" && connection->connectionType() == "ODBC" && !connection->database().isEmpty())
  {
      db.setDatabaseName(connection->dsn());
      db.setConnectOptions(tr("database=%1;").arg(connection->database()));
  }
  // if(connection->servertype() == "PostgreSQL" && connection->connectionType() == "ODBC" && !connection->connectString().isEmpty())
  // {
  //     db.setDatabaseName(connection->connectString());
  // }
  ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
  if(!db.open())
  {
      qDebug() << displayDbInfo(db);
    ui->lblHelp->setText(db.lastError().text());
  }
  else
  {
      ui->lblHelp->setStyleSheet("color: green");
      ui->lblHelp->setText(tr("OK database has %1 tables").arg(db.tables().count()));
  }
  ui->txtUserId->setText(connection->userId());
  ui->txtPWD->setText(connection->pwd());
  ui->txtPWD->setEnabled(true);
  ui->cbUseDatabase->setCurrentText(connection->database());
 }
 connectionChanging=false;
}

void EditConnectionsDlg::cbODBCDsn_currentIndex_changed(int ix)
{
    if(ui->cbConnect->currentText() == "ODBC")
    {
        QMap<QString,QString> map = odbcPairMap.value(ui->cbODBCDsn->currentData().toString());
        if(ui->cbDbType->currentText() == "PostgreSQL")
        {
            if(map.contains("Data Source"))
            {
                ui->txtHost->setText(map.value("Data Source"));
            }
            if(map.contains("User ID"))
            {
                ui->txtUserId->setText(map.value("User ID"));
            }
            if(map.contains("Database"))
            {
                ui->txtDefaultDb->setText(map.value("Database"));
                ui->cbUseDatabase->setPlaceholderText(map.value("Database"));
            }
        }

        if(ui->cbDbType->currentText() == "MsSql")
        {
            if(map.contains("Database"))
            {
                ui->txtDefaultDb->setText(map.value("Database"));
                ui->cbUseDatabase->setPlaceholderText(map.value("Database"));
            }
            if(map.contains("Server"))
            {
                ui->txtHost->setText(map.value("Server"));
                ui->txtHost->setPlaceholderText(map.value("Server"));
            }
            if(map.contains("LastUser"))
            {
                ui->txtUserId->setText(map.value("LastUser"));
                ui->txtUserId->setPlaceholderText(map.value("LastUser"));
                ui->txtPWD->setPlaceholderText("enter password");
            }
        }
        if(ui->cbDbType->currentText() == "MySql")
        {
            if(map.contains("DATABASE"))
            {
                ui->txtDefaultDb->setText(map.value("DATABASE"));
                ui->cbUseDatabase->setPlaceholderText(map.value("DATABASE"));
            }
            if(map.contains("SERVER"))
            {
                ui->txtHost->setText(map.value("SERVER"));
                ui->txtHost->setPlaceholderText(map.value("SERVER"));
            }
            if(map.contains("UID"))
            {
                ui->txtUserId->setText(map.value("UID"));
                ui->txtUserId->setText(map.value("UID"));
                ui->txtPWD->setText(map.value("PWD"));
            }
        }
    }
}

void EditConnectionsDlg::cbConnections_contextMenuRequested(QPoint pos)
{
    QMenu menu;
    QAction* deleteAct = new QAction(tr("Delete connection"),this);
    connect(deleteAct, &QAction::triggered, [=]{
        QString txt = ui->cbConnections->currentText();
        int index = ui->cbConnections->currentIndex();
        Connection* c = VPtr<Connection>::asPtr(ui->cbConnections->currentData());
        config->currCity->connections.removeOne(c);
    });
    menu.addAction(deleteAct);
    menu.exec(QCursor::pos());
}

void EditConnectionsDlg::cbDriverTypeSelectionChanged(QString sel)
{
 Q_UNUSED(sel)
 QString text = ui->cbDriverType->currentText();
 QString connectionType = ui->cbConnect->currentText();
 Connection*  c = VPtr<Connection>::asPtr(ui->cbConnections->currentData());
 //QString driverType = c->driver();
 QString driverType = ui->cbDriverType->currentText();
 QString dbType = ui->cbDbType->currentText();

 if(dbType == "Sqlite" && (driverType == "QSQLITE" || driverType == "QSQLITE3"))
 {
  ui->cbDbType->setCurrentIndex(ui->cbDbType->findText("Sqlite"));
  if(ui->cbConnect->currentText() != "Local")
   ui->cbConnect->setCurrentText("Local");
 }
 else if(dbType == "MySql" && (driverType == "QMYSQL" || driverType == "QMYSQL3"
         || driverType == "QMARIADB" || driverType == "QMARIADB"))
 {
  ui->cbDbType->setCurrentIndex(ui->cbDbType->findText("MySql"));
    //ui->cbConnect->setCurrentIndex(ui->cbConnect->findText(c->connectionType()));
    ui->cbODBCDsn->setVisible(true);
    setComboBoxItemEnabled(ui->cbConnect, 1, true); // Direct");
    ui->cbConnect->setCurrentIndex(1);
    setControls("Direct");
    qDebug() << QCoreApplication::libraryPaths().join(",");
    qDebug() << QSqlDatabase::drivers().join(",");
    ui->txtPort->setText("3306");
 }
 else if(dbType == "PostgreSQL" && driverType == "QPSQL")
 {
     ui->cbODBCDsn->setVisible(false);
     setComboBoxItemEnabled(ui->cbConnect, 1, true); // Direct");
     ui->cbConnect->setCurrentIndex(1);
     setControls("Direct");
     ui->txtHost->setEnabled(true);
     ui->txtPort->setEnabled(true);
     ui->txtPWD->setEnabled(true);
     ui->txtUserId->setEnabled(true);
     ui->txtPort->setText("5432");
 }
 else if((dbType == "MySql" || dbType == "MsSql"|| dbType == "PostgreSQL" ) &&(ui->cbDriverType->currentText() == "QODBC"
          || ui->cbDriverType->currentText() == "QODBC3"))
 {
     ui->cbODBCDsn->setVisible(true);
     odbcUtil->fillDSNCombo(ui->cbODBCDsn, dbType);
     if(!((ui->cbDbType->currentText() == "MySql") || (ui->cbDbType->currentText() == "MsSql")))
         ui->cbDbType->setCurrentIndex(ui->cbDbType->findText(c->servertype()));
     ui->txtHost->setEnabled(true);
     ui->txtPort->setEnabled(true);
     ui->txtPWD->setEnabled(true);
     ui->txtUserId->setEnabled(true);
     if(dbType == "MySql")
         ui->txtPort->setText("3306");
     if(dbType == "PostgreSQL")
         ui->txtPort->setText("5432");
     if(dbType == "MsSql")
         ui->txtPort->setText("1443");
 }
 setControls(ui->cbConnect->currentText());
}

void EditConnectionsDlg::setControls(QString txt)
{
 if(txt == "Direct")
 {
  ui->label_2->setVisible(false);
  ui->txtSqliteFileName->setVisible(false);
  ui->tbBrowse->setVisible(false);
  //ui->cbDbType->setCurrentIndex(ix);
  ui->txtHost->setEnabled(true);
  ui->txtPort->setEnabled(true);
  ui->txtSqliteFileName->setText("");
  ui->txtSqliteFileName->setPlaceholderText("select MySqlDatabase");
  //ui->cbConnections->setCurrentText(tr("MySql Connection"));
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUserId->setEnabled(true);
  if(ui->cbDbType->currentText() == "MySql")
      ui->txtPort->setText("3306");
  if(ui->cbDbType->currentText() == "PostgreSQL")
      ui->txtPort->setText("5432");
  if(ui->cbDbType->currentText() == "MsSql")
      ui->txtPort->setText("1443");


  // use database (required for ODBC or Direct)
  ui->label_10->setVisible(true);
  ui->cbUseDatabase->setVisible(true);
  ui->label_13->setVisible(true);
  ui->txtDefaultDb->setVisible(true);
  ui->tbReload->setVisible(true);
  // host (required for ODBC, MYSQL)
  ui->label_4->setVisible(true);
  ui->label_5->setVisible(true);
  ui->txtHost->setVisible(true);
  ui->txtPort->setVisible(true);
  // userId/Password (required for ODBC, MYSQL)
  ui->label_6->setVisible(true);
  ui->txtUserId->setVisible(true);
  ui->label_7->setVisible(true);
  ui->txtPWD->setVisible(true);
  ui->tbView->setVisible(true);
  ui->tbBrowse->setVisible(false); // browse, only for Sqlite.
  ui->label_2->setVisible(false);
  ui->label_2->setText(tr("DSN")+":");
  ui->txtSqliteFileName->setVisible(false);
  ui->txtSqliteFileName->setCompleter(nullptr);
  ui->label_12->setVisible(false);
  ui->cbODBCDsn->setVisible(false);
 }
 else if(txt == "ODBC")
 {
  ui->txtPWD->setVisible(true);
  ui->tbView->setVisible(true);

  ui->label_2->setText("DSN:");
  //ui->cbDbType->setCurrentIndex(1);
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUserId->setEnabled(true);

  //odbcPairMap.clear();

#ifdef Q_OS_WIN
  QSettings winReg1("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
  QSettings winReg2("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
  //odbcMap.clear();
  //QList<QPair<QString,QString> > list;
  QMap<QString,QString> map;
  QStringList iniKeys;
  databases = winReg1.childKeys();
  qDebug() << QString("%1").arg(databases.count());
  foreach (QString key, databases) {
     QString keyVal = winReg1.value(key).toString();
     // if( keyVal.contains("SQL Server"))
     // {
         //list.clear();
     map.clear();
     QString regKey = QString("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\") + key;
     QSettings winReg(regKey,QSettings::NativeFormat);

     iniKeys = winReg.childKeys();
     for(QString iniKey : iniKeys)
     {
         QStringList childKeys = winReg.childKeys();
         for(QString iniKey3 : childKeys)
         {
             QPair<QString,QString>  pair(iniKey3, winReg.value(iniKey3).toString());
             //list.append(pair);
             map.insert(iniKey3, winReg.value(iniKey3).toString());
         }
     }
     //odbcPairMap.insert(key, list);
     odbcPairMap.insert(key,map);
  }

  // add also systemDsn
  databases = winReg2.childKeys();
  qDebug() << QString("%1").arg(databases.count());
  foreach (QString key, databases) {
      QString keyVal = winReg2.value(key).toString();
      // if( keyVal.contains("SQL Server"))
      // {
      map.clear();
      QString regKey = QString("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI\\") + key;
      QSettings winReg(regKey,QSettings::NativeFormat);

      iniKeys = winReg.childKeys();
      for(QString iniKey : iniKeys)
      {
          QStringList childKeys = winReg.childKeys();
          for(QString iniKey3 : childKeys)
          {
              //QPair<QString,QString>  pair(iniKey3, winReg.value(iniKey3).toString());
              //list.append(pair);
              map.insert(iniKey3, winReg.value(iniKey3).toString());
          }
      }
      odbcPairMap.insert(key, map);
  }
#else
// #  ifdef Q_OS_MACOS
//   findODBCDsn("/Library/ODBC/odbc.ini", &databases);
//   findODBCDsn(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini", &databases);
// #  else
//   // location of unixODBC config files
//   findODBCDsn(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini", &databases);
//   //findODBCDsn("/usr/local/etc/odbc.ini", &databases);
//   findODBCDsn("/etc/odbc.ini", &databases);
// #  endif
  connect(odbcUtil, &ODBCUtil::odbc_changed,this,[=]{
      odbcUtil->fillDSNCombo(ui->cbODBCDsn, ui->cbDbType->currentText());
  });
#endif
  // use database (required for ODBC)
  ui->label_10->setVisible(true);
  ui->cbUseDatabase->setVisible(true);
  ui->label_13->setVisible(true);
  ui->txtDefaultDb->setVisible(true);
  ui->tbReload->setVisible(true);
  // host (required for MYSQL)
  ui->label_4->setVisible(true); // Host:
  ui->label_5->setVisible(true); // Port:
  ui->txtHost->setVisible(true);
  ui->txtHost->setEnabled(true);
  ui->txtPort->setEnabled(true);
  ui->txtPort->setVisible(true);
  // userId/Password (required for MYSQL)
  ui->label_6->setVisible(true); // User id:
  ui->txtUserId->setEnabled(true);
  ui->txtUserId->setVisible(true);
  ui->label_7->setVisible(true); // Password:
  ui->txtPWD->setEnabled(true);
  ui->txtPWD->setVisible(true);
  ui->tbView->setVisible(true); // view password
  ui->tbBrowse->setVisible(false); // browse, only for Sqlite.
  ui->label_2->setVisible(false);
  ui->txtSqliteFileName->setVisible(true);
  ui->txtDefaultDb->setEnabled(false);
  ui->tbBrowse->setVisible(false);
  //ui->label_2->setText(tr("ODBC DSN")+":");
  ui->txtSqliteFileName->setVisible(false);
  ui->label_12->setVisible(true);
  ui->cbODBCDsn->setVisible(true);
  odbcUtil->fillDSNCombo(ui->cbODBCDsn, ui->cbDbType->currentText());
#ifdef  Q_OS_WIN
  ui->cbODBCDsn->clear();
  QMapIterator<QString, QMap<QString,QString>> iter(odbcPairMap);
  while(iter.hasNext())
  {
      iter.next();
      //if(iter.key() == ui->cbDbType->currentText())
      QMap<QString,QString> map = iter.value();
      QString description;
      QMapIterator<QString,QString> iter2(map);
      while(iter2.hasNext())
      {
          iter2.next();
          if(iter2.key().compare("Description",Qt::CaseInsensitive)==0)
          {
              description = iter2.value();
              break;
          }
      }
          //sources.append(iter.key());
      //if(ui->cbODBCDsn->findData(iter.key())<0 && description.startsWith(ui->cbDbType->currentText()))
        ui->cbODBCDsn->addItem(iter.key() + " - " + description, iter.key());
  }
  //ui->cbODBCDsn->addItems(sources);
#endif
 }
 else if(txt == "Local") // Local (Sqlite only)
 {
  ui->label_2->setText("Database:");
  //ui->cbDbType->setCurrentIndex(2);  // "Sqlite"
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->tbBrowse->setVisible(true);
  ui->txtPWD->setEnabled(false);
  ui->tbView->setEnabled(false);
  ui->txtUserId->setEnabled(false);
  ui->txtHost->setText("");
  ui->txtPort->setText("");
  ui->txtPWD->setText("");
  ui->txtUserId->setText("");

  // use database (required for ODBC)
  ui->label_10->setVisible(false);
  ui->cbUseDatabase->setVisible(false);
  ui->label_13->setVisible(false);
  ui->txtDefaultDb->setVisible(false);
  ui->tbReload->setVisible(false);
  // host (required for ODBC, MYSQL)
  ui->label_4->setVisible(false);
  ui->label_5->setVisible(false);
  ui->txtHost->setVisible(false);
  ui->txtPort->setVisible(false);
  // userId/Password (required for ODBC, MYSQL)
  ui->label_6->setVisible(false);
  ui->txtUserId->setVisible(false);
  ui->label_7->setVisible(false);
  ui->txtPWD->setVisible(false);
  ui->tbView->setVisible(false);
  ui->tbBrowse->setVisible(true); // browse, only for Sqlite.
  ui->label_2->setVisible(true);
  ui->label_2->setText(tr("DSN")+":");
  ui->txtSqliteFileName->setVisible(true);
  ui->txtSqliteFileName->setCompleter(nullptr);
  ui->label_12->setVisible(false);
  ui->cbODBCDsn->setVisible(false);
 }
}

#if 0
#ifndef Q_OS_WIN
void EditConnectionsDlg::findODBCDsn(QString iniFile, QStringList* dsnList)
{
 QFile ini(iniFile);
 QList<QPair<QString, QString>> list;
 QString entry;

 if(ini.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  while(!ini.atEnd())
  {
   QString str = ini.readLine();
   if(str.contains("[ODBC Data Sources]") || str.contains("[Default]") || str.contains("FreeTDS"))
    continue;

   if(str.contains("["))
   {
    if(!entry.isEmpty())
    {
     odbcPairMap.insert(entry,list);
     entry = "";
     list.clear();
    }

    int first = str.indexOf("[")+1;
    int last = str.indexOf("]");
    if((last - first) > 0)
    {
      entry = str.mid(first, last-first);
      if(entry == "ODBC")
        continue;
      if(!dsnList->contains(entry.trimmed()))
        dsnList->append(entry.trimmed());
    }
   }
   else
   {
    // get parameter pairs
    QStringList sl = str.split("=");
    if(sl.count() ==2)
    {
      QPair<QString, QString> pair(sl.at(0).trimmed(),sl.at(1).trimmed());
      list.append(pair);
    }
   }
  }
  odbcPairMap.insert(entry,list);
 }
 else
 {
  qDebug()<< ini.errorString();
 }
 ini.close();

 qDebug()<<ini.fileName();
 qDebug()<< "number of databases" << QString("%1").arg(databases.count());
}

QString EditConnectionsDlg::getODBCDSNValue(QString iniFile, QString dsn, QString key)
{
    QFile ini(iniFile);
    if(ini.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      while(!ini.atEnd())
      {
         QString str = ini.readLine();
          if(str.contains("["+dsn+"]"))
         {
             while(!ini.atEnd())
             {
                 QString str2 = ini.readLine().trimmed();
                 QStringList sl = str2.split("=");
                 if(sl.count()==2 && sl.at(0).trimmed()== key)
                   return sl.at(1).trimmed();
             }
         }
      }
    }
    return "";
}
#endif
#endif
void EditConnectionsDlg::btnTestClicked()
{
   removeEmptyFiles();
   ui->lblHelp->setText("");
   ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
   qApp->processEvents();
   connection->setValid(testConnection());
   setCursor(Qt::ArrowCursor);
}

void EditConnectionsDlg::btnCancelClicked()
{
 if(ui->cbConnect->currentText() == "Local")
 {
    removeEmptyFiles();
 }
 this->rejected();
 this->close();
}

void EditConnectionsDlg::removeEmptyFiles()
{
    QDir dbDir(basePath);
    QStringList filters;
    filters << "*.sqlite3" ;
    dbDir.setNameFilters(filters);
    dbDir.setFilter(QDir::Files);
    QFileInfoList list = dbDir.entryInfoList();
    for(QFileInfo fi : list)
    {
        if(fi.size() == 0 && fi.exists())
        {
          QFile f(fi.absoluteFilePath());
          f.remove();
        }
    }
}

void EditConnectionsDlg::btnSaveClicked()
{
   ui->lblHelp->setText(tr(""));
   ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
   if(ui->cbConnections->currentText().isEmpty())
   {
      ui->lblHelp->setStyleSheet("QLabel {  color : blue; }");
      ui->lblHelp->setText(tr("Enter the new connection's description"));
      ui->btnSave->setEnabled(false);
      return;
   }
   if(ui->txtSqliteFileName->text() == "" && ui->cbDbType->currentText() == "Sqlite")
   {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
     ui->lblHelp->setText(tr("Database or DSN name required"));
     ui->btnSave->setEnabled(false);
     return;
   }
   if(ui->cbDriverType->currentText() == "")
   {
     ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
     ui->lblHelp->setText(tr("Select driver Type"));
     ui->btnSave->setEnabled(false);
     return;
   }
   if(ui->cbConnections->currentText().isEmpty())
   {
      ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
      ui->lblHelp->setText(tr("Connection description cannot be blank and must be unique!"));
      ui->btnSave->setEnabled(false);
      return;
   }
   if(ui->cbConnect->currentText() == "Direct")
   {
      if(ui->txtUserId->text() == "")
      {
         ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
         ui->lblHelp->setText(tr("User Id required"));
         ui->btnSave->setEnabled(false);
         return;
      }
   }
//   if(connection->cityName() != currCity->name())
//   {
//       //throw IllegalArgumentException("internal error, connection not for city");
//       ui->lblHelp->setStyleSheet("color:red");
//       ui->lblHelp->setText("connection city name is invalid");
//   }
    //QString savedb = ui->cbUseDatabase->currenttext();

   if(!testConnection(true)) // create files or databases
     return;

   if(!config->cityList.contains(currCity))
       config->addCity(currCity);

   connection->setDescription(ui->cbConnections->currentText());
   connection->setDriver(ui->cbDriverType->currentText());
   connection->setServerType(ui->cbDbType->currentText());
   connection->setConnectionType(ui->cbConnect->currentText());
   connection->setCityName(currCity->name());
   connection->setHost(ui->txtHost->text());
   int id = findId(connection);

  if(ui->cbDbType->currentText() == "MsSql" || ui->cbDbType->currentText() == "PostgreSQL")
  {
     //connection->setOdbcConnectorName(ui->txtSqliteFileName->text());
     connection->setDSN(ui->cbODBCDsn->currentData().toString());
     connection->setDefaultSqlDatabase(ui->txtDefaultDb->text());
     connection->setDatabase(ui->cbUseDatabase->currentText());
     connection->setUserId(ui->txtUserId->text());
     connection->setPWD(ui->txtPWD->text());
     if(ui->cbDbType->currentText() == "PostgreSQL" && ui->cbConnect->currentText()=="ODBC")
         connection->setConnectString(odbcUtil->connectString(ui->cbODBCDsn->currentData().toString(),
                                                              ui->txtHost->text(), ui->txtPort->text().toInt(), ui->txtUserId->text(),
                                                              ui->txtPWD->text(), ui->cbUseDatabase->currentText().toLower()));

  }
  else if(ui->cbDbType->currentText() == "Sqlite")
  {
       QFileInfo info(ui->txtSqliteFileName->text());
       if(!info.isAbsolute())
       {
//#ifndef Q_OS_MACOS
        info = QFileInfo("Resources/databases/" + ui->txtSqliteFileName->text());
// #else
//         info = QFileInfo(config->macOSPublic+ "databases/" + ui->txtSqliteFileName->text());
// #endif
        connection->setSqliteFileName(info.fileName());
       }
       else
       {
        connection->setSqliteFileName(info.absoluteFilePath());
       }
       if(!info.exists())
       {

          SQL::instance()->executeScript(":/sql/sqlite3_createTables.sql", db);

          SQL::instance()->insertParameters(parms, db);

          SQL::instance()->addCompany("New Company", -1, "1870/01/01", "1950/12/31");

          SQL::instance()->executeScript("/sql/create_routeView.sql");

          SQL::instance()->updateTractionType(1, "Electric", "#FF0000", 0, db);

          SQL::instance()->updateTractionType(2, "Horse/Mule", "#61380b",0, db);

       }
  }
  else // MySql or MsSql
  {
   if(ui->cbConnect->currentText() == "ODBC")
   {
    connection->setDSN(ui->cbODBCDsn->currentData().toString());
    if(ui->txtHost->isEnabled())
     connection->setHost(ui->txtHost->text());
    if(ui->txtPort->isEnabled())
     connection->setPort(ui->txtPort->text().toInt());
    if(ui->txtUserId->isEnabled())
     connection->setUserId(ui->txtUserId->text());
    if(ui->txtPWD->isEnabled())
     connection->setPWD(ui->txtPWD->text());
   }
   else
   {
    connection->setHost(ui->txtHost->text());
    connection->setPort(ui->txtPort->text().toInt());
    connection->setUserId(ui->txtUserId->text());
    connection->setPWD(ui->txtPWD->text());
   }
   connection->setServerType(ui->cbDbType->currentText());
   connection->setDefaultSqlDatabase(ui->cbUseDatabase->currentText());
   connection->setDatabase(ui->txtDefaultDb->text());
  }

  if(ui->cbCities->currentText() == config->currCity->name()
     && connection->uniqueId() == config->currConnection->uniqueId())
   config->currConnection = connection;
  config->currentCityId = config->currCity->id;
  if(ui->cbConnections->currentIndex() < 1)
  {
   connection->setId(currCity->connections.count());
   if(!currCity->connections.contains(connection))
   {
//     currCity->connections.append(connection);
//     currCity->connectionNames.append(connection->description());
       currCity->addConnection(connection);
   }
  }
  else
  {
   if(!currCity->connections.contains(connection))
   {
//     currCity->connections.append(connection);
//     currCity->connectionNames.append(ui->cbConnections->currentText());
//     currCity->connectionMap.insert(connection->uniqueId().toString(), connection);
       currCity->addConnection(connection);
   }
  }
  if(config->currCity->id == currCity->id)
   config->currCity->connections = currCity->connections;



  config->saveSettings();
  ui->btnNew->setEnabled(true);

  this->accept();
  this->close();
}

void EditConnectionsDlg::btnDeleteClicked()
{
 bool deleted = false;
 for(int j=0; j< config->cityList.count(); j++)
 {
  City * currCity = config->cityList.at(j);
  if(ui->cbCities->currentText() == currCity->name())
  {
      if(QMessageBox::warning(this, tr("Confirm delete"),
                              tr("Are you sure that you want to delete city '%1'?")
                              .arg(currCity->name()),
                              QMessageBox::Yes |QMessageBox::No) == QMessageBox::No)
        return;
   for(int i =0; i< currCity->connections.count(); i++)
   {
    Connection* c = config->currCity->connections.at(i);
    if(c->servertype() == "Sqlite")
    {
//#ifndef Q_OS_MACOS
      QFile f("Resources/databases/"+c->sqlite_fileName());
// #else
//         QFile f(config->macOSPublic+"/databases/"+c->sqlite_fileName());
// #endif
      if(f.exists())
          f.remove();
    }
   }
//   if(config->currCity->id == currCity->id)
//    config->currCity->connections = currCity->connections;
      if(currCity)
      config->cityList.removeAt(j);
      deleted = true;
      continue;
  }
  if(deleted)
   currCity->id--; // adjust id of remaining cities.
 }
 config->saveSettings();
 refreshCities();
}

void EditConnectionsDlg::cbConnectionsTextChanged(QString text)
{
 bCbConnectionsTextChanged=true;
}

void EditConnectionsDlg::cbConnectionsLeave()
{
    ui->lblHelp->setText("");
 if(bCbConnectionsTextChanged)
 {
#if 0
  int ix = ui->cbConnections->currentIndex();
  if(ix >= 0)
  {
   Connection* c = config->currCity->connections.at(ix);
   c->setDescription(ui->cbConnections->currentText());
   //config->currCity->connections.replace(ix-1, c);
//   switch(QMessageBox::question(this, tr("Add a new connection"), tr("Do you wish to add a new connection with this name\n"
//                                                              "or just change the connection name?\n"
//                                                              "Yes to add a new connection, \n"
//                                                              "No to change the connection name\n"
//                                                              "Ignore to change nothing!"),
//                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore))
//   {
//    case QMessageBox::Yes:
//       newConnection();
//       break;
//   case QMessageBox::No:
//       break;
//   case QMessageBox::Ignore:
//       ui->cbConnections->setCurrentText(connection->connectionName());
//       break;
//   }
   if(!config->currCity->connections.contains(c))
   {
//    config->currCity->connections.append(c);
//    config->currCity->connectionNames.append(c->description());
       config->currCity->addConnection(c);
   }
  }
#else
  // verify that this connectionName is not currently used elsewhere
  foreach(Connection* c, currCity->connections)
  {
   if(ui->cbConnections->currentText() == c->connectionName())
   {
    ui->lblHelp->setText(tr("The connection name \"%1\" is already used for this city\n"
                            "Please enter another name that is unique").arg(ui->cbConnections->currentText()));
    ui->cbConnections->lineEdit()->setText("");
    break;
   }
  }
  connection->setConnectionName(ui->cbConnections->currentText());
#endif
 }
 bCbConnectionsTextChanged=false;
}

void EditConnectionsDlg::newConnection(){
    ui->btnNew->setEnabled(false);
    ui->btnSave->setEnabled(false);
    ui->cbDriverType->setCurrentIndex(0); //QSQLITE
    ui->txtSqliteFileName->setText("");
    ui->txtHost->setText("");
    ui->txtPort->setText("");
    ui->txtPWD->setText("");
    ui->txtUserId->setText("");
    ui->lblHelp->setText("");
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
    ui->cbDbType->setCurrentIndex(2); // Sqlite
    ui->cbCities->setEditable(true);
    // ui->cbCities->setCurrentText("");
    // ui->cbCities->lineEdit()->setPlaceholderText(tr("enter a city name. e.g. 'St Louis, MO'"));
    connection = new Connection();
    connection->cityName() = ui->cbCities->currentText();
    //connection->setCityName(currCity->name());
    connect(ui->cbCities->lineEdit(), &QLineEdit::editingFinished, [=] {
        QString cityName = ui->cbCities->lineEdit()->text();
        if(!config->cityMap.contains(cityName))
        {
          City* newCity = new City();
          newCity->setName(cityName);
          ui->cbCities->addItem(newCity->name(), VPtr<City>::asQVariant(newCity));
          currCity = newCity;
          connection->setCityName(cityName);
        }
    });
    populateDatabases();
    connection->setConnectionName("");
    ui->cbConnections->addItem("",VPtr<Connection>::asQVariant(connection));
    ui->cbConnections->setCurrentIndex(ui->cbConnections->findData(VPtr<Connection>::asQVariant(connection)));
    ui->cbConnections->lineEdit()->setPlaceholderText(tr("enter unique connection description"));
    //ui->cbConnections->lineEdit()->setPlaceholderText(tr("enter connetion description"));
    //ui->cbDriverType->setCurrentIndex(-1);
    //ui->cbDbType->setCurrentIndex(-1);
    ui->cbConnect->setCurrentIndex(0); // local
    ui->btnSave->setEnabled(false);
    ui->cbDriverType->setFocus();
}

bool EditConnectionsDlg::testConnection(bool bCreate)
{
 ui->lblHelp->setText("");
 if(!openTestDb())
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
     ui->lblHelp->setText(db.lastError().text());
  return false;
 }
 this->setCursor(QCursor(Qt::WaitCursor));
 ui->btnSave->setEnabled(true);

 if(ui->cbConnections->lineEdit()->text().isEmpty())
     ui->cbConnections->lineEdit()->setText(ui->cbDbType->currentText() + " " + ui->txtHost->text() + " " + ui->cbConnect->currentText() + " connection");

 timer->start(1000);
 ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
 ui->lblHelp->setText(tr("Testing connection..."));
 qApp->processEvents();
 if(ui->cbDbType->currentText() == "Sqlite")
 {
  QString fn = ui->txtSqliteFileName->text();
  if(!fn.toLower().endsWith(".sqlite3"))
   fn.append(".sqlite3");
  QFileInfo file(fn);
  if(!file.isAbsolute())
//#ifndef Q_OS_MACOS
      file = QFileInfo("Resources/databases/"+fn);
// #else
//       file = QFileInfo(config->macOSPublic+"/databases/"+fn);
// #endif
  if( file.exists() && !file.isWritable())
  {
   MyMessageBox::warning(this, tr("Warning"), tr("The file pathname '%1' is invalid or not writable").arg(file.absoluteFilePath()));
   return false;
  }
  if(!file.exists() || db.tables().count() == 0)
  {
     if(!bCreate)
     {
         ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
         ui->lblHelp->setText(tr("%1 tables are defined in %2").arg(db.tables().count()).arg(fn));
         this->setCursor(QCursor(Qt::ArrowCursor));
       return true;
     }
//   MyMessageBox* mb = new MyMessageBox(this, tr("Question"), tr("Sqlite db '%1' does not exist,br>"
//                                      "Do you wish to create a new database?").arg(file.absoluteFilePath()), QMessageBox::Question,QMessageBox::Yes | QMessageBox::No);
//   int btn = mb->exec();
   int btn = QMessageBox::question(this, tr("Question"), tr("Sqlite db '%1' does not exist<br>Do you wish to create a new database?").arg(file.absoluteFilePath()),QMessageBox::Yes | QMessageBox::No);

   if(btn == QMessageBox::Yes)
   {
    if(db.open())
    {
     createSqliteTables(db);
     //SQL::instance()->executeScript(":/sql/CreateMySqlFunctions.sql");
     SQL::instance()->insertParameters(parms, db);
    }
   }
   else
   {
    this->setCursor(QCursor(Qt::ArrowCursor));
    ui->txtDefaultDb->clear();
    return false;
   }
  }
  //ui->txtSqliteFileName->setText(file.completeBaseName());
  QStringList tableList = db.tables();
  ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
  ui->lblHelp->setText(tr("Connection succeeded. %1 Has %2 tables!<br> %3")
                           .arg(ui->txtSqliteFileName->text()).arg(tableList.count()).arg(tableList.join(", ")));

 } // end Sqlite
 else
 {
     QString defaultDb = getDatabase();
     ui->txtDefaultDb->setText(defaultDb);
     if(!ui->cbUseDatabase->currentText().isEmpty())
     {
         setDatabase(ui->cbUseDatabase->currentText());
     }
     if(ui->cbDbType->currentText() == "ODBC")
     {
         db.setDatabaseName(ui->cbODBCDsn->currentData().toString());
     }
 }
 if(!db.isOpen())
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
  ui->lblHelp->setText( db.lastError().text());
  timer->stop();
  this->setCursor(QCursor(Qt::ArrowCursor));
  return false;
 }

 QString currDb = getDatabase();
#if 0
#ifdef Q_OS_MACOS
 // iODBC not setting default db
 if(currDb.isEmpty())
 {
     QString defaultDb;
     defaultDb= getODBCDSNValue(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini",
                                 ui->cbODBCDsn->currentData().toString(), "database");
     if(defaultDb.isEmpty())
        defaultDb= getODBCDSNValue( "/Library/ODBC/odbc.ini",ui->cbODBCDsn->currentText(), "database");
     if(!defaultDb.isEmpty())
     {
         if(SQL::instance()->useDatabase(defaultDb, ui->cbDbType->currentText(),db))
         {
             currDb = getDatabase();
             ui->txtUseDatabase->setText(currDb);
             ui->txtDefaultDb->setText(defaultDb);
         }
         else
             qDebug() << QString("useDatabase(%1)").arg(defaultDb) << " failed";
     }
 }
#endif
#endif
 QStringList tableList = db.tables();
 bool bOpen = db.open(/*ui->txtUserId->text(), ui->txtPWD->text()*/);
 if(tableList.contains("Parameters"))
    parms = SQL::instance()->getParameters(db);
 if(!currDb.isEmpty() && !tableList.contains("Parameters", Qt::CaseInsensitive))
 {
  //createSqliteTables(db);
  ExportSql* expSql = new ExportSql(Configuration::instance(), false);
  if(ui->cbDbType->currentText() == "Sqlite")
      expSql->createParametersTable(db, ui->cbDbType->currentText());
  else
  {
      QMessageBox::warning(nullptr, tr("Setup database"),
                               tr("No tables exist in this database. \n"
                               "The database administrator must create the tables and users.\n"
                               "Or you can export to this database."));
      ui->lblHelp->setStyleSheet("color: magenta");
      ui->lblHelp->setText("Connection OK but no tables exist!");
      return true;
  }
 }

 QString odbcDsn = db.databaseName();

 //if(ui->cbDbType->currentText() =="MsSql" || ui->cbDbType->currentText() =="MySql")
 if(ui->cbDbType->currentText() != "Sqlite")
 {
     if(!currDb.isEmpty())
     {
         ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
         ui->lblHelp->setText(tr("Connection succeeded. %1 Has %2 tables!<br> %3").arg(currDb).arg(tableList.count()).arg(tableList.join(", ")));
         ui->txtDefaultDb->setText(currDb);
         //parms = SQL::instance()->getParameters(db);
         currCity->setCenter(LatLng(parms.lat, parms.lon));
         ui->txtDefaultDb->setText(currDb);
     }
     else
     {
         if(!ui->cbUseDatabase->currentText().isEmpty())
         {
             if(!SQL::instance()->useDatabase(ui->cbUseDatabase->currentText(),ui->cbDbType->currentText(), db))
             {
                 ui->lblHelp->setStyleSheet("color:red");
                 ui->lblHelp->setText(tr("unable to set used database %1").arg(currDb));
                 return false;
             }
         }
         else
         {
             ui->lblHelp->setStyleSheet("color:red");
             ui->lblHelp->setText(tr("there is no database specified"));
             populateDatabases();
             return false;
         }
         ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
         ui->lblHelp->setText(tr("Connection succeeded. No default database. %1 Has %2 tables!<br> %3").arg(currDb).arg(tableList.count()).arg(tableList.join(", ")));
         currCity->setCenter(LatLng(parms.lat, parms.lon));
         //parms = SQL::instance()->getParameters(db);
         if(parms.city == ui->cbCities->currentText())
          currCity->setCenter(LatLng(parms.lat, parms.lon));
         else
             QMessageBox::warning(this, tr("City Name"), tr("The city name entered, '%1'; is not"
                                                            " the same as in the database:'%2'\n"
                                                            "The name in the database will be used.")
                                  .arg(ui->cbCities->currentText(),parms.city));
         currCity->setNameOverride(parms.city);
         ui->cbCities->setCurrentText(parms.city);
     }
     if(!ui->cbUseDatabase->currentText().isEmpty())
     {
        populateDatabases();
     }
     if(ui->cbConnections->currentText().isEmpty() && !ui->cbCities->currentText().isEmpty())
     {
         ui->cbConnections->setCurrentText(ui->cbCities->currentText() + " " + ui->cbDbType->currentText()
                                           + " " + ui->cbConnect->currentText() + " " + ui->txtHost->text());
     }
 }
 else
 {
  ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
  ui->lblHelp->setText(tr("Connection succeeded. %1 Has %2 tables!<br> %3").arg(currDb).arg(tableList.count())
                           .arg(tableList.join(", ")));

 }
 timer->stop();
 this->setCursor(QCursor(Qt::ArrowCursor));

 db.close();
 ui->btnSave->setEnabled(true);
 return true;
}

bool EditConnectionsDlg::openTestDb()
{
   if(!_testConnection)
     _testConnection = new Connection();
   setCursor(Qt::WaitCursor);
   _testConnection->setConnectionType(ui->cbConnect->currentText());
   _testConnection->setDriver(ui->cbDriverType->currentText());
   _testConnection->setServerType(ui->cbDbType->currentText());
   db = QSqlDatabase::addDatabase(ui->cbDriverType->currentText(),"testConnection");
   _testConnection->setDb(db);
   if(ui->cbConnect->currentText() == "Direct")
   {
       db.setPort(ui->txtPort->text().toInt());
       if(ui->cbDbType->currentText() == "PostgreSQL")
           db.setDatabaseName(ui->cbUseDatabase->currentText());
   }
 if(ui->cbDbType->currentText() == "Sqlite"  )
 {
  QFileInfo info(ui->txtSqliteFileName->text());
  if(info.isAbsolute())
    db.setDatabaseName( ui->txtSqliteFileName->text());
  else
   db.setDatabaseName("Resources/databases/" + ui->txtSqliteFileName->text());
  _testConnection->setSqliteFileName(db.databaseName());
 }
 else if(ui->cbConnect->currentText() == "ODBC")
 {
  QString connector = ui->cbODBCDsn->currentData().toString();
#ifndef Q_OS_WIN
  QString connstring = QString("Driver=psqlodbcw.so;Server=%1;Port=%2;User Id=%3;Password=%4;Database=%5;")
                           .arg(ui->txtHost->text()).arg(ui->txtPort->text()).arg(ui->txtUserId->text(),
                                ui->txtPWD->text(),ui->cbUseDatabase->currentText());
  QString connstring2 = odbcUtil->connectString(connector,ui->txtHost->text(),ui->txtPort->text().toInt(),
                                                ui->txtUserId->text(),ui->txtPWD->text(),ui->cbUseDatabase->currentText());

#else // windows
  //QList<QPair<QString, QString>> list = odbcPairMap.value(connector);
  QMap<QString,QString> map = odbcPairMap.value(connector);
  QString driver;
  QString servername;
  QString port;
  QString UID;
  QString password;
  QString database;
  QStringList keys = {"Driver","Servername", "Port", "UID", "Password","Database"};

  //for(QPair<QString,QString> pair : list)
  QMapIterator<QString,QString> iter(map);
  while(iter.hasNext())
  {
      iter.next();
      QString first = iter.key();
      if(keys.contains(first,Qt::CaseInsensitive))
      {
        if(first == "Driver")
              driver = iter.value();
        if(first == "Server")
            servername = iter.value();
        if(first == "Port")
            port = iter.value();
        if(first == "UID")
            UID = iter.value();
        if(first == "Password")
            password = iter.value();
        if(first == "Database")
            database = iter.value();
      }
  }
  // ui->txtHost->setText(servername);
  db.setHostName(servername);
  // ui->txtPWD->setText((password));
  db.setPassword(ui->txtPWD->text());
  // ui->txtUserId->setText(UID);
  db.setUserName(ui->txtUserId->text());
  //
  if(!ui->txtUserId->text().isEmpty())
  {
      db.setUserName(ui->txtUserId->text());
      UID = ui->txtUserId->text();
  }
  if(!ui->txtPWD->text().isEmpty())
  {
      db.setPassword(ui->txtPWD->text());
      password = ui->txtPWD->text();
  }
  if(!ui->cbUseDatabase->currentText().isEmpty())
  {
      db.setDatabaseName(ui->cbUseDatabase->currentText());
      database = ui->cbUseDatabase->currentText();
  }

  QString connstring2 = QString("Driver=%1;Server=%2;Port=%3;Uid=%4;Pwd=%5;Database=%6;")
                            .arg(driver,servername,port,UID,password,database);

#endif
  if(ui->cbDbType->currentText() == "PostgreSQL")
  {
 #ifdef Q_OS_WIN
      db.setDatabaseName(connector);
      //db.setDatabaseName(connstring2);
 #else
      db.setDatabaseName(connstring2);
#endif
    _testConnection->setDSN(connector);
    _testConnection->setConnectString(connstring2);
  }
  else
  {
    db.setDatabaseName(connector);
   _testConnection->setDSN(connector);
  }

  if(ui->cbDbType->currentText() == "MySql" || ui->cbDbType->currentText() == "MsSql" || ui->cbDbType->currentText() == "PostgreSQL")
  {
      // userid, server, etc are not defined in the DSN override here
      if(db.userName().isEmpty())
      {
       ui->txtUserId->setEnabled(true);
       db.setUserName(ui->txtUserId->text());
      }
      if(db.password().isEmpty())
      {
       ui->txtPWD->setEnabled(true);
       db.setPassword(ui->txtPWD->text());
      }
      if(db.hostName().isEmpty())
      {
       ui->txtHost->setEnabled(true);
       db.setHostName(ui->txtHost->text());

       qDebug() << db.hostName();
      }
      if(db.port()< 1)
      {
       ui->txtPort->setEnabled(true);
       if(!ui->txtPort->text().isEmpty())
        db.setPort(ui->txtPort->text().toInt());
      }
      // if(!ui->cbUseDatabase->currenttext().isEmpty())
      //     db.setDatabaseName(ui->cbUseDatabase->currenttext());
  }
  else
  {
     db.setUserName(ui->txtUserId->text());
     db.setPassword(ui->txtPWD->text());
  }
 }
 else if( ui->cbConnect->currentText()== "Direct")
 { // MySql or PostgreSQL
  db.setDatabaseName(ui->cbUseDatabase->currentText());
  db.setHostName(ui->txtHost->text());
  int port = ui->txtPort->text().toInt();
  if(port > 0)
   db.setPort(port);
  db.setUserName(ui->txtUserId->text());
  db.setPassword(ui->txtPWD->text());
 }
 else
 {
     QString msg = tr("invalid configuration db type: %1, driver: %2, connType: %3")
                       .arg(ui->cbDbType->currentText(), ui->cbDriverType->currentText(), ui->cbConnect->currentText());
     //throw IllegalArgumentException(msg);
     ui->lblHelp->setText(msg);
     return false;
 }
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText(tr(""));
 _testConnection->setUserId(db.userName());
 _testConnection->setPWD(db.password());
 _testConnection->setHost(db.hostName());
 _testConnection->setPort(db.port());
 _testConnection->setConnectionName(db.connectionName());
 _testConnection->setTables(db.tables());
 _testConnection->setDSN(db.connectionName());
 _testConnection->setDriver(db.driverName());
 _testConnection->setDatabase(db.databaseName());
 bool bOpen = db.open(ui->txtUserId->text(), ui->txtPWD->text());
 qDebug() << displayDbInfo(db);
 if(bOpen)
 {

  if(ui->cbDbType->currentText() != "Sqlite" )
  {
    QString defaultDatabase = getDatabase();
    ui->txtDefaultDb->setText(defaultDatabase);
    populateDatabases();
    // if(!availableDatabases.contains( ui->cbUseDatabase->currenttext()))
    //     ui->txtUseDatabase->setText(defaultDatabase);
  }
  setCursor(Qt::ArrowCursor);
  return true;
 }
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText(tr("error: %1 %2").arg(db.driverName()).arg(db.lastError().driverText()));
 setCursor(Qt::ArrowCursor);
 return false;
}


bool EditConnectionsDlg::populateDatabases()
{
  QString currDatabase = ui->cbUseDatabase->currentText();
  QSqlQuery query = QSqlQuery(db);

  if(!db.open())
  {
      QSqlError err = db.lastError();
      QString txt = displayDbInfo(db);
      //throw SQLException(tr("database not open ") +txt );
      qDebug() << txt;
      return false;
  }

  //QStringList list;
  if(ui->cbDbType->currentText() == "MySql")
      availableDatabases = SQL::instance()->showMySqlDatabases(db);
  else if(ui->cbDbType->currentText() == "MsSql")
      availableDatabases = SQL::instance()->showMsSqlDatabases(db);
  else if(ui->cbDbType->currentText() == "PostgreSQL")
      availableDatabases = SQL::instance()->showPostgreSQLDatabases(db);
  else
      return false; // Sqlite
  ui->cbUseDatabase->clear();
  ui->cbUseDatabase->addItems(availableDatabases);
    if(ui->cbUseDatabase->currentText().isEmpty() && availableDatabases.count()==1)
     ui->cbUseDatabase->setCurrentText(availableDatabases.at(0));
    if(!currDatabase.isEmpty() && !availableDatabases.contains(currDatabase))
    {
        int rslt = QMessageBox::question(nullptr, tr("Create database?"),
                                         tr("The database %1 does not exist on the server.<br>"
                                            "Do you wish to create it?").arg(currDatabase));
        if(rslt == QMessageBox::Yes)
        {
          // if(SQL::instance()->useDatabase("master",ui->cbDbType->currentText(), db))
          // {
            if(SQL::instance()->createSqlDatabase(currDatabase, db, "ui->cbDbType->currentText()"))
            {
                if(ui->cbDbType->currentText() == "MySql")
                 SQL::instance()->executeScript(":/sql/CreateMySqlFunction.sql");
                else if(ui->cbDbType->currentText() == "MsSql")
                    SQL::instance()->executeScript(":/sql/CreateMsSqlFunction.sql");
                else if(ui->cbDbType->currentText() == "PostgreSQL")
                        SQL::instance()->executeScript(":/sql/CreatePostgreSQLFunction.sql");
                else throw IllegalArgumentException("invalid db type");
            }

        }
        if(rslt == QMessageBox::No)
        {
            ui->cbUseDatabase->clear();
            return false;
        }
    }
    else
    {
        // database exists
        qDebug() << "database " << currDatabase << " exists";
    }
   }
//   }
//   else if(ui->cbDbType->currentText() == "MySql")
//   {
//    // MySql
//    QStringList availableDatabases = SQL::instance()->showMySqlDatabases(db);
//    QCompleter* completer = new QCompleter(availableDatabases);
//    ui->txtUseDatabase->setCompleter(completer);
//    if(ui->cbUseDatabase->currenttext().isEmpty() && availableDatabases.size()==1)
//        ui->txtUseDatabase->setText(availableDatabases.at(0));
//    if(!availableDatabases.contains(currDatabase))
//    {
//          QString database = currDatabase;
//          if(db.isOpen())
//          {
//           QSqlQuery query = QSqlQuery(db);
//           bool retry = true;
//           while (retry)
//           {
//             if(!database.isEmpty())
//             {
//               QString commandText = "use " + database;
//               if(!query.exec(commandText))
//               {
//                 if(query.lastError().databaseText().contains("Unknown database"))
//                 {
//                   int rslt = QMessageBox::question(nullptr, tr("Create database?"),
//                                                    tr("The database %1 does not exist on the server.<br>"
//                                                       "Do you wish to create it?").arg(database));
//                   if(rslt == QMessageBox::Yes)
//                   {
//                     SQL::instance()->createSqlDatabase(database, db, ui->cbDbType->currentText());
//                     SQL::instance()->executeScript(":/sql/CreateMySqlFunctions.sql");
//                   }
//                   else if(rslt == QMessageBox::No)
//                   {
//                      retry = false;
//                   }
//                 }
//                 else
//                 {
//                   int rslt = QMessageBox::critical(nullptr, tr("Sql Error"), tr("An SqL error has occurred.\n"
//                                             "Sql error:%1\nquery:\" %2\"").arg(query.lastError().text()).arg(query.lastQuery()),
//                                             QMessageBox::Retry|QMessageBox::Ignore);
//                   if(rslt == QMessageBox::Retry)
//                     continue;
//                   else retry = false;
//                 }
//             }
//            }
//             else
//                 break;
//           }
//        }
//    }
//   }
//   else if(ui->cbDbType->currentText() != "Sqlite"){
//    // Sqlite
// //#ifndef Q_OS_MACOS
//       QDir dir("Resources/databases");
// // #else
// //       QDir dir(config->macOSPublic + "/databases");
// // #endif
//       QStringList nameFilter = {"*.sqlite3"};
//       if(dir.exists())
//       {
//           QStringList databases = dir.entryList(nameFilter, QDir::Files);
//           QCompleter *completer = new QCompleter(databases, this);
//           completer->setCaseSensitivity(Qt::CaseInsensitive);
//           ui->txtSqliteFileName->setCompleter(completer);
//       }
//   }
//   // PostgreSQL DSN can only connect to one database so trying to do this is invalid
//   if(ui->cbDbType->currentText() != "PostgreSQL")
//   {
//       // use desired db
//       if(!currDatabase.isEmpty())
//       {
//           if(!query.exec("use "+ currDatabase))
//           {
//             qDebug() << query.lastError().driverText()+ query.lastError().databaseText();
//             ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
//             ui->lblHelp->setText(query.lastError().driverText() + query.lastError().databaseText());
//             return false;
//           }
//           ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
//           ui->lblHelp->setText(tr("using ")+currDatabase);
//       }
//       else
//       {
//           ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
//           ui->lblHelp->setText(tr("using PostgreSQL database")+currDatabase);
//       }
//   }
//   return true;
// }

QString EditConnectionsDlg::getDatabase()
{
 if(!db.isOpen()) return QString();

 QSqlQuery query = QSqlQuery(db);
 QString dbName ="";
 QString commandText;
 if(ui->cbDbType->currentText() == "MsSql")
  commandText = "SELECT DB_NAME() ";
 else if(ui->cbDbType->currentText() == "MySql")
  commandText = "SELECT DATABASE()";
 else if(ui->cbDbType->currentText() == "PostgreSQL")
     commandText = "SELECT current_database()";
 else
  return dbName;
 if(!query.exec(commandText))
 {
  SQLERROR(std::move(query));
 }
 else
 {
  while(query.next())
  {
   if(!query.value(0).isNull())
   dbName = query.value(0).toString();
  }
  ui->txtDefaultDb->setText(dbName);
 }
 return dbName;
}

bool EditConnectionsDlg::setDatabase(QString useDatabase)
{
    QString currentDb = getDatabase();
    if(currentDb == useDatabase)
        return true;
    QSqlQuery query = QSqlQuery(db);
    if(!db.open())
        throw SQLException(tr("db not open!"));
    QString commandText;
    //if(ui->cbDbType->currentText() == "MsSql")
     commandText = "use " +useDatabase;
    if(!query.exec(commandText))
    {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        return false;
    }
    return true;

}

void EditConnectionsDlg::quickProcess()
{
 qApp->processEvents();
}

void EditConnectionsDlg::txtHostLeave()
{
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText("");
 qint16 port =0;
 if(ui->cbDbType->currentText() == "MySql")
     ui->txtPort->setText("3306");
 else if(ui->cbDbType->currentText() == "PostgreSQL")
     ui->txtPort->setText("5432");
 else if(ui->cbDbType->currentText() == "MsSql")
     ui->txtPort->setText("1443");

 if(ui->txtPort->text().toInt()> 0)
  port = ui->txtPort->text().toInt();
 socket.connectToHost(ui->txtHost->text(), port, QIODevice::ReadWrite);
 if(!socket.waitForReadyRead(1000))
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
  ui->lblHelp->setText(socket.errorString());
  ui->txtHost->setFocus();
 }
 socket.close();
}

void EditConnectionsDlg::txtPortLeave()
{
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText("");
 qint16 port =0;
 if(ui->cbDbType->currentText() == "MySql")
     ui->txtPort->setText("3306");
 else if(ui->cbDbType->currentText() == "PostgreSQL")
     ui->txtPort->setText("5432");
 else if(ui->cbDbType->currentText() == "MsSql")
     ui->txtPort->setText("1443");
 if(ui->txtPort->text().toInt()> 0)
  port = ui->txtPort->text().toInt();
 socket.connectToHost(ui->txtHost->text(), port, QIODevice::ReadWrite);
 if(!socket.waitForReadyRead(1000))
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
  ui->lblHelp->setText(socket.errorString());
  ui->txtHost->setFocus();
 }
 socket.close();
}

void EditConnectionsDlg::txtPwdLeave()
{
 if(!openTestDb())
  return;
 populateDatabases();
}

void EditConnectionsDlg::txtDsnTextChanged(QString text)
{
 QCompleter *completer = new QCompleter(databases, this);
 completer->setCaseSensitivity(Qt::CaseInsensitive);
 ui->txtSqliteFileName->setCompleter(completer);
}

void EditConnectionsDlg::on_tbBrowse_clicked()
{
//#ifndef Q_OS_MACOS
   basePath = MainWindow::pwd + QDir::separator() + "Resources" + QDir::separator() +"databases" + QDir::separator();
// #else
//    basePath = config->macOSPublic + QDir::separator() +"databases" + QDir::separator();

// #endif
// QFileInfo info(ui->txtSqliteFileName->text());
// if(info.exists())
//  basePath=info.canonicalPath();

 QString path = QFileDialog::getOpenFileName(this, "Select Sqlite database",basePath, "Sqlite3 databases (*.sqlite3);; All Files (*)");
 if(!path.isEmpty())
 {
  QFileInfo pInfo(path);
  if(pInfo.isAbsolute() && !path.startsWith( basePath))
  {
    ui->txtSqliteFileName->setText(path);
  }
  else
   ui->txtSqliteFileName->setText(pInfo.fileName());
 }
}

bool EditConnectionsDlg::createSqliteTables(QSqlDatabase db)
{
 if(!db.open())
  return false;
 QFile file(":/sql/sqlite3_create_tables.sql");
 if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  qDebug()<<file.errorString() + " '" + file.fileName()+"'";
  ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
  ui->lblHelp->setText(file.errorString() + " '" + file.fileName()+"'");
  //return;
 }
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText(tr("Creating tables"));
 QTextStream in(&file);
 QString sql;
 while(!in.atEnd())
 {
  sql = sql +  in.readLine();
  if(sql.contains(";"))
  {
   QSqlQuery query = QSqlQuery(db);
   if(!query.exec(sql))
   {
    QSqlError err = query.lastError();
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
    ui->lblHelp->setText(err.text());
    return false;
   }
   sql="";
  }
 }
 return true;
}
void EditConnectionsDlg::ontxtSqliteFileName_editingFinished()
{
 QString dbName = ui->txtSqliteFileName->text();
 if(ui->cbDbType->currentText() == "Sqlite")
 {
     if(ui->txtSqliteFileName->text().isEmpty())
         return;
  QFileInfo info;
  if(!ui->txtSqliteFileName->text().toLower().endsWith("sqlite3"))
   ui->txtSqliteFileName->setText(ui->txtSqliteFileName->text().append(".sqlite3"));
  if(ui->txtSqliteFileName->text().startsWith("/"))
   info.setFile(ui->txtSqliteFileName->text());
  else
//#ifndef Q_OS_MACOS
   info.setFile("Resources/databases/" + ui->txtSqliteFileName->text());
// #else
//    info.setFile(config->macOSPublic + "/databases/" + ui->txtSqliteFileName->text());

// #endif

//  if(info.isAbsolute())
//   ui->txtSqliteFileName->setText(info.absoluteFilePath());
//  else
  ui->txtSqliteFileName->setText(info.fileName());
  if(!info.exists())
  {
      QMessageBox::warning(this, "New file", tr("The file entered does not exist. If you coninue "
                                                " a new, empty, database will be created. "
                                                "If this is not what you wish, enter the name of "
                                                "an existing sqlite3 file!"));
  }
 }
 else if (ui->cbDbType->currentText() == "Msql")
 {
  populateDatabases();
  getDatabase();
 }
 else if(ui->cbDbType->currentText() == "MySql")
 {
  if(ui->cbConnect->currentText()== "ODBC")
   //handleOverrides(ui->txtSqliteFileName->text());
  if(databases.contains(dbName))
  {
   if(!verifyDatabase(dbName))
   {
    QMessageBox::critical(this, tr("Invalid database"), tr("Not a Mapper database!"));
   }
  }
  else
  {
    if(db.isOpen())
    {
       int ret = QMessageBox::question(this, tr("Create new database?"), tr("The database '") +dbName + tr("' does not exist on the server.<br>Do you wish to create it?"));
       if(ret == QMessageBox::Yes)
       {
        if(SQL::instance()->useDatabase("master",ui->cbDbType->currentText(), db))
        {
         SQL::instance()->createSqlDatabase(dbName, db, ui->cbDbType->currentText());
        }
       }
    }
  }
 }
}

bool EditConnectionsDlg::verifyDatabase(QString name)
{
 QStringList list = db.tables();
 if(!list.isEmpty())
 {
  if(list.count()< 12 )
  {
   qDebug() << name << " has only " << list.count() << " of 12 tables";
   return false;
  }
  if(!(list.contains("Parameters") && list.contains("Segments") && list.contains("Companies")
       && list.contains("Routes") && list.contains("altRoute")))
  {
   qDebug() << name << " missing key tables";
   return false;

  }
  return true;
 }
 if(list.isEmpty())
 {
  SQL::instance()->executeScript(":/databases/mySql_createDatabase.sql",db);
  SQL::instance()->executeScript(":/databases/CreateMySqlFunction.sql", db);
 }
 return true;
}

void EditConnectionsDlg::refreshCities()
{
    int index=0, i=0;
    ui->cbCities->clear();
    foreach(City* c, config->cityList)
    {
     ui->cbCities->addItem(c->name(), VPtr<City>::asQVariant(c));
     if(c->id == config->currCity->id)
      index = i;
     i++;
    }
    ui->cbCities->setCurrentIndex(index);
}

void EditConnectionsDlg::setComboBoxItemEnabled(QComboBox * comboBox, int index, bool enabled)
{
    auto * model = qobject_cast<QStandardItemModel*>(comboBox->model());
    assert(model);
    if(!model) return;

    auto * item = model->item(index);
    assert(item);
    if(!item) return;
    item->setEnabled(enabled);
}

// Parameters for new database;
void EditConnectionsDlg::setParameter(Parameters p)
{
 parms = p;
}

#if 0
// handle parameters (host, user, port, etc specified in ODBC )
void EditConnectionsDlg::handleOverrides(QString dbName)
{
 ui->lblHelp->setText("");
 QList<QPair<QString,QString>> list = odbcPairMap.value(dbName);
 if(list.isEmpty())
  return;
 for(QPair<QString,QString> pair : list)
 {
  QString first = pair.first;
  if(first.toLower() == "host" || first.toLower() == "server")
  {
   ui->txtHost->setText(pair.second);
   ui->txtHost->setEnabled(false);
  }
  else if(first.toLower() == "port")
  {
   ui->txtPort->setText(pair.second);
   ui->txtPort->setEnabled(false);
  }
  else if(first.toLower() == "user" || first.toLower() == "uid")
  {
   ui->txtUserId->setText(pair.second);
   ui->txtUserId->setEnabled(false);
  }
  else if(first.toLower() == "password" || first.toLower() == "pwd")
  {
   ui->txtPWD->setText(pair.second);
   ui->txtPWD->setEnabled(false);
  }
  else if(first.toLower() == "database")
  {
   ui->txtDefaultDb->setText(pair.second);
   ui->txtDefaultDb->setEnabled(false);
  }
 }
}
#endif

int EditConnectionsDlg::findId(Connection* c)
{
 for(Connection connection : currCity->connections)
 {
  if(c->uniqueId() == connection.uniqueId())
   return connection.id();
 }
 return -1;
}

QString EditConnectionsDlg::displayDbInfo(QSqlDatabase db)
{
    QString txt;
    txt.append(tr("connection name: %1 ").arg(db.connectionName()));
    txt.append(tr("driverName: %1 ").arg(db.driverName()));
    txt.append(tr("databaseName: %1 ").arg(db.databaseName()));
    txt.append(tr("hostName: %1 port: %2 ").arg(db.hostName()).arg(db.port()));
    txt.append(tr("userName: %1 pwd: %2 ").arg(db.userName(),db.password()));
    txt.append(tr("isOpen: %1 isValid: %2 ").arg(db.isOpen()?"true":"false", db.isValid()?"true":"false"));
    txt.append(tr("last error: %1 ").arg(db.lastError().text()));
    return txt;
}
