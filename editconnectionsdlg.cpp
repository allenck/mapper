#include "editconnectionsdlg.h"
#include "ui_editconnectionsdlg.h"
#include <QCompleter>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include <QClipboard>
#include <QApplication>
#include "vptr.h"
#include "mymessagebox.h"
#include "exceptions.h"

EditConnectionsDlg::EditConnectionsDlg( QWidget *parent) :
  QDialog(parent),
  ui(new Ui::editConnectionsDlg)
{
    ui->setupUi(this);
    ui->lblHelp->setText("");
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
    config = Configuration::instance();
    currCity = config->currCity;
    this->setWindowTitle(tr("Edit Connections"));
    drivers = QSqlDatabase::drivers();

   // connection that is being edited
   connection = currCity->connections.at(currCity->curConnectionId);

   refreshCities();


   dbTypes <<"MySql"<<"MsSql"<<"Sqlite";  // currently supported database types.

   for(int i=0; i < dbTypes.count(); i++)
   {
     ui->cbDbType->addItem(dbTypes.at(i), (DBTYPE)i);
   }

   int index = 0;
   for(int i=0; i < config->currCity->connections.count(); i++)
   {
    City* selectedCity = config->cityMap.value(ui->cbCities->currentText());
    Connection* connection = selectedCity->connections.at(i);
    setControls(connection->connectionType());
    ui->cbConnections->addItem(connection->description(),VPtr<Connection>::asQVariant(connection));

    if(this->connection == connection)
    {
     index = i;
    }
   }
   ui->cbDbType->setCurrentText(connection->servertype());

   QString dbType =  ui->cbDbType->currentText();
   setupComboBoxes();
   cbConnectionsSelectionChanged(index);

    connect(ui->cbDriverType, SIGNAL(currentIndexChanged(int)), this, SLOT(cbDriverTypeSelectionChanged(int)));

    connect(ui->cbConnect, &QComboBox::currentTextChanged, [=]{
    setControls(ui->cbConnect->currentText());
    if(ui->cbConnect->currentText() == "ODBC")
        setControls(ui->cbConnect->currentText());
    });

    connect(ui->cbDbType,SIGNAL(currentIndexChanged()), this,  SLOT(setupComboBoxes()));

    connect(ui->cbCities, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitiesSelectionChanged(int)));
    connect(ui->cbCities, SIGNAL(editTextChanged(QString)), this, SLOT(cbCitiesTextChanged(QString)));
    connect(ui->cbCities->lineEdit(), SIGNAL(editingFinished()), this, SLOT(cbCitiesLeave()));

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
    connect(ui->cbConnections->lineEdit(), SIGNAL(editingFinished()),this, SLOT(cbConnectionsLeave()));
    connect(ui->txtHost, SIGNAL(editingFinished()), this, SLOT(txtHostLeave()));
    connect(ui->txtPort, SIGNAL(editingFinished()), this, SLOT(txtPortLeave()));
    connect(ui->txtPWD, SIGNAL(editingFinished()), this, SLOT(txtPwdLeave()));
    connect(ui->txtDbOrDSN, SIGNAL(textChanged(QString)), this, SLOT(txtDsnTextChanged(QString)));
    connect(ui->txtDbOrDSN, SIGNAL(editingFinished()), this, SLOT(ontxtDbOrDsn_editingFinished()));
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

    connect(ui->txtUseDatabase, &QLineEdit::editingFinished, [=](){
      QString database = ui->txtUseDatabase->text();
      if(db.isOpen() && !database.isEmpty())
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
    });

//   connect(config, &Configuration::newCityCreated, [=]{
//      refreshCities();
//   });

//   connect(ui->btnNew, &QPushButton::clicked, [=]{
//    ui->cbDriverType->setCurrentIndex(2);
//    ui->txtDbOrDSN->setText("");
//    ui->txtHost->setText("");
//    ui->txtPort->setText("");
//    ui->txtPWD->setText("");
//    ui->txtUserId->setText("");
//    ui->lblHelp->setText("");
//    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
//    ui->cbDbType->setCurrentIndex(-1);
//    connection = new Connection();
//    ui->cbConnections->lineEdit()->setPlaceholderText(tr("enter connetion description"));
//    ui->cbDriverType->setCurrentText("QSQLITE");
//    ui->cbConnect->setCurrentText("Local");
//    ui->cbDbType->setCurrentText("Sqlite");
//    ui->btnOK->setEnabled(false);
//   });


   timer = new QTimer(this);
   timer->setInterval(1000);
   connect(timer, SIGNAL(timeout()), this, SLOT(quickProcess()));
}

void EditConnectionsDlg::setupComboBoxes()
{
 if(ui->cbDbType->currentText()=="Sqlite")
 {
  ui->cbConnect->clear();
  ui->cbConnect->addItem("Local");
  ui->cbDriverType->clear();
  ui->cbDriverType->addItem("QSQLITE");
  ui->cbDriverType->setCurrentText("QSQLITE");
 }
 else if(ui->cbDbType->currentText()=="MySql")
 {
  ui->cbConnect->clear();
  ui->cbConnect->addItem("Direct");
  ui->cbConnect->addItem("ODBC");
  ui->cbDriverType->clear();
  ui->cbDriverType->addItem("QMYSQL");
  ui->cbDriverType->addItem("QMYSQL3");
  ui->cbDriverType->addItem("QMARIADB");
  ui->cbDriverType->addItem("QODBC");
  ui->cbDriverType->addItem("QODBC3");
  ui->cbDriverType->setCurrentText(connection->driver());
 }
 else // MsSql
 {
  ui->cbConnect->clear();
  ui->cbConnect->addItem("ODBC");
  ui->cbDriverType->setCurrentText("QODBC");
  ui->cbDriverType->clear();
  ui->cbDriverType->addItem("QODBC");
  ui->cbDriverType->addItem("QODBC3");
  ui->cbDriverType->setCurrentText(connection->driver());
 }

   setControls(ui->cbConnect->currentText());

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
   ui->cbConnections->clear();
   //ui->cbConnections->addItem(tr("Add new connection"));
   foreach(Connection* cn, currCity->connections)
   {
    ui->cbConnections->addItem(cn->description(), VPtr<Connection>::asQVariant(cn));
   }
   ui->cbConnections->setCurrentIndex(currCity->curConnectionId);
   cbConnectionsSelectionChanged(currCity->curConnectionId);
   ui->btnNew->setEnabled(true);
   return;
//  }
// }
}

void EditConnectionsDlg::cbCitiesTextChanged(QString text)
{
    bCbCitiesTextChanged = true;
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
    ui->txtDbOrDSN->setText("");

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
  ui->txtDbOrDSN->setText("");
 }
}

void EditConnectionsDlg::cbConnectionsSelectionChanged(int sel)
{
 ui->lblHelp->setText("");
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 if(sel < 0)
  return;
 //ui->btnSave->setText(tr("update"));
 connection = VPtr<Connection>::asPtr(ui->cbConnections->itemData(sel));
 if(connection == nullptr)
     connection = new Connection();
 setControls(ui->cbConnect->currentText());
// if(currCity->name() == ui->cbCities->currentText())
// {
//    if((sel) > currCity->connections.count())
//     sel = currCity->connections.count()-1;
//    c = currCity->connections.at(sel);
//    ui->btnOK->setText(tr("Update"));
    //if(c->id != config->currConnection.id) //??
    ui->btnDelete->setEnabled(true);
    ui->cbDriverType->setCurrentIndex(-1);
    for(int i =0; i<drivers.count(); i++)
    {
     if(drivers.at(i)== connection->driver())
     {
      ui->cbDriverType->setCurrentIndex(i);
      break;
     }
    }
    ui->cbDbType->setCurrentText(connection->servertype());
    ui->cbDriverType->setCurrentText(connection->driver());
    ui->cbConnect->setCurrentText(connection->connectionType());
    if(ui->cbConnect->currentText()== "Local") // Local
    {
         ui->txtDbOrDSN->setText(connection->sqlite_fileName());
    }
    else if(ui->cbConnect->currentText() == "Direct") // Direct
    {
         ui->txtHost->setText(connection->host());
         ui->txtPort->setText(QString::number(connection->port()));
         ui->txtUserId->setText(connection->userId());
         ui->txtPWD->setText(connection->pwd());
         if(openTestDb())
         {
          QStringList list = SQL::instance()->showMySqlDatabases(db);
          QCompleter* completer = new QCompleter(list);
          ui->txtUseDatabase->setCompleter(completer);
         }
         ui->txtUseDatabase->setText(connection->defaultSqlDatabase());
     }
     else
     { // ODBC (MsSql or MySql/MariaDb
        ui->txtUseDatabase->setText(connection->defaultSqlDatabase());
        //ui->txtDbOrDSN->setText(c->odbc_connectorName());
        ui->cbODBCDsn->setCurrentIndex(ui->cbODBCDsn->findText(connection->odbc_connectorName()));
        ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
        ui->lblHelp->setText(tr("connecting ..."));
        if(openTestDb())
        {
          QStringList list;
          if(ui->cbDbType->currentText() == "MsSql")
            list = SQL::instance()->showMsSqlDatabases(db);
          else
            list = SQL::instance()->showMySqlDatabases(db);

          QCompleter* completer = new QCompleter(list);
          ui->txtUseDatabase->setCompleter(completer);
        }
    }
}

void EditConnectionsDlg::cbDriverTypeSelectionChanged(int sel)
{
 Q_UNUSED(sel)
 QString text = ui->cbDriverType->currentText();
 QString connectionType = ui->cbConnect->currentText();
 QString driverType = ui->cbDriverType->currentText();
 //ui->cbConnections->setCurrentText(ui->cbCities->currentText() + " " + driverType +" connection" );

 if(driverType == "QSQLITE" || driverType == "QSQLITE3")
 {
  ui->cbDbType->setCurrentIndex((DBTYPE)Sqlite);
  ui->cbConnect->setCurrentText("Local");
 }
 else if(driverType == "QMYSQL" || driverType == "QMYSQL3"
         || driverType == "QMARIADB" || driverType == "QMARIADB")
 {
  ui->cbDbType->setCurrentIndex((DBTYPE)MySql);
  if(ui->cbConnect->currentText() == "Local")
      ui->cbConnect->setCurrentIndex(-1);
 }
 else if(ui->cbDriverType->currentText() == "QODBC" || ui->cbDriverType->currentText() == "QODBC3")
 {
  ui->cbDbType->setCurrentIndex((DBTYPE)MsSql);
  ui->cbConnect->setCurrentText("ODBC");
 }
 setControls(ui->cbConnect->currentText());
}

void EditConnectionsDlg::setControls(QString txt)
{
 if(txt == "Direct")
 {
  ui->label_2->setVisible(false);
  ui->txtDbOrDSN->setVisible(false);
  ui->tbBrowse->setVisible(false);
  //ui->cbDbType->setCurrentIndex(ix);
  ui->txtHost->setEnabled(true);
  ui->txtPort->setEnabled(true);
  ui->txtDbOrDSN->setText("");
  ui->txtDbOrDSN->setPlaceholderText("select MySqlDatabase");
  //ui->cbConnections->setCurrentText(tr("MySql Connection"));
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUserId->setEnabled(true);
  ui->txtPort->setText("3306");

  // use database (required for ODBC or Direct)
  ui->label_10->setVisible(true);
  ui->txtUseDatabase->setVisible(true);
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
  ui->txtDbOrDSN->setVisible(false);
  ui->txtDbOrDSN->setCompleter(nullptr);
  ui->label_12->setVisible(false);
  ui->cbODBCDsn->setVisible(false);
 }
 else if(txt == "ODBC")
 {
  ui->label_2->setText("DSN:");
  //ui->cbDbType->setCurrentIndex(1);
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUserId->setEnabled(true);
#ifdef Q_OS_WIN
  QSettings winReg("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);

  odbcMap.clear();

  databases = winReg.childKeys();
  qDebug() << QString("%1").arg(databases.count());
  foreach (QString key, databases) {
     QString keyVal = winReg.value(key).toString();
     if( keyVal.contains("SQL Server"))
         odbcMap.insert("MsSql", key);
     else if(keyVal.contains("MySQL"))
         odbcMap.insert("MySql", key);
  }
#else
  findODBCDsn(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini", &databases);
  findODBCDsn("/etc/odbc.ini", &databases);
#endif
  // use database (required for ODBC)
  ui->label_10->setVisible(true);
  ui->txtUseDatabase->setVisible(true);
  ui->tbReload->setVisible(true);
  // host (required for MYSQL)
  ui->label_4->setVisible(false);
  ui->label_5->setVisible(false);
  ui->txtHost->setVisible(false);
  ui->txtPort->setVisible(false);
  // userId/Password (required for MYSQL)
  ui->label_6->setVisible(false);
  ui->txtUserId->setVisible(false);
  ui->label_7->setVisible(false);
  ui->txtPWD->setVisible(false);
  ui->tbView->setVisible(false);
  ui->tbBrowse->setVisible(false); // browse, only for Sqlite.
  ui->label_2->setVisible(false);
  ui->txtDbOrDSN->setVisible(false);
  ui->tbBrowse->setVisible(false);
  //ui->label_2->setText(tr("ODBC DSN")+":");
  ui->txtDbOrDSN->setVisible(false);
  //ui->txtDbOrDSN->setCompleter(new QCompleter(databases));
  ui->label_12->setVisible(true);
  ui->cbODBCDsn->setVisible(true);
  ui->cbODBCDsn->clear();
  //ui->cbODBCDsn->addItems(databases);
  QStringList sources;
  QMapIterator<QString, QString> iter(odbcMap);
  while(iter.hasNext())
  {
      iter.next();
      if(iter.key() == ui->cbDbType->currentText())
          sources.append(iter.value());
  }
  ui->cbODBCDsn->addItems(sources);
 }
 else if(txt == "Local") // Local (Sqlite only)
 {
  ui->label_2->setText("Database:");
  ui->cbDbType->setCurrentIndex(2);  // "Sqlite"
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->tbBrowse->setVisible(true);
//#ifdef Q_WS_WIN
  ui->txtPWD->setEnabled(false);
  ui->tbView->setEnabled(false);
  ui->txtUserId->setEnabled(false);
  ui->txtHost->setText("");
  ui->txtPort->setText("");
  ui->txtPWD->setText("");
  ui->txtUserId->setText("");

  // use database (required for ODBC)
  ui->label_10->setVisible(false);
  ui->txtUseDatabase->setVisible(false);
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
  ui->txtDbOrDSN->setVisible(true);
  ui->txtDbOrDSN->setCompleter(nullptr);
  ui->label_12->setVisible(false);
  ui->cbODBCDsn->setVisible(false);
 }
}

#ifndef Q_WS_WIN
void EditConnectionsDlg::findODBCDsn(QString iniFile, QStringList* dsnList)
{
 QFile ini(iniFile);
 if(ini.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  while(!ini.atEnd())
  {
   QString str = ini.readLine();
   if(str.contains("[ODBC Data Sources]") || str.contains("[Default]") || str.contains("FreeTDS"))
    continue;

   if(str.contains("["))
   {
    int first = str.indexOf("[")+1;
    int last = str.indexOf("]");
    if((last - first) > 0)
    {
     QString entry = str.mid(first, last-first);
     dsnList->append(entry.trimmed());
    }
   }
  }
 }
 else
 {
  qDebug()<< ini.errorString();
 }
 ini.close();

 qDebug()<<ini.fileName();
 qDebug() << QString("%1").arg(databases.count());
}
#endif

void EditConnectionsDlg::btnTestClicked()
{
 ui->lblHelp->setText("");
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 qApp->processEvents();
 connection->setValid(testConnection());
}

void EditConnectionsDlg::btnCancelClicked()
{
 this->rejected();
 this->close();
}

void EditConnectionsDlg::btnSaveClicked()
{
   ui->lblHelp->setText(tr(""));
   ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
   if(ui->cbConnections->currentText().isEmpty())
   {
      ui->lblHelp->setStyleSheet("QLabel {  color : blue; }");
      ui->lblHelp->setText(tr("Enter the new connection's description"));
      return;
   }
    if(ui->txtDbOrDSN->text() == "" && ui->cbDbType->currentText() == "Sqlite")
    {
      ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
      ui->lblHelp->setText(tr("Database or DSN name required"));
      return;
    }
    if(ui->cbDriverType->currentText() == "")
    {
      ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
      ui->lblHelp->setText(tr("Select driver Type"));
      return;
    }
    if(ui->cbConnections->currentText().isEmpty())
    {
       ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
       ui->lblHelp->setText(tr("Connection description cannot be blank and must be unique!"));
       return;
    }
    if(ui->cbConnect->currentText() == "Direct")
    {
       if(ui->txtUserId->text() == "")
       {
          ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
          ui->lblHelp->setText(tr("User Id required"));
        return;
       }
    }
    if(connection->cityName() != currCity->name())
    {
        throw IllegalArgumentException("internal error, connection not for city");
    }
    QString savedb = ui->txtUseDatabase->text();

    if(!testConnection(true)) // create files or databases
      return;

    if(!config->cityList.contains(currCity))
        config->addCity(currCity);


    connection->setDescription(ui->cbConnections->currentText());
    connection->setDriver(ui->cbDriverType->currentText());
    connection->setServerType(ui->cbDbType->currentText());
    connection->setConnectionType(ui->cbConnect->currentText());


   if(ui->cbDbType->currentText() == "MsSql")
   {
      //connection->setOdbcConnectorName(ui->txtDbOrDSN->text());
       connection->setOdbcConnectorName(ui->cbODBCDsn->currentText());
      connection->setDefaultSqlDatabase(ui->txtUseDatabase->text());
      connection->setUseDatabase(savedb);
   }
   else if(ui->cbDbType->currentText() == "Sqlite")
   {
        QFileInfo info(ui->txtDbOrDSN->text());
        if(!info.isAbsolute())
        {
         info = QFileInfo("Resources/databases/" + ui->txtDbOrDSN->text());
         connection->setSqliteFileName(info.fileName());
        }
        else
        {
         connection->setSqliteFileName(info.absoluteFilePath());
        }
        if(!info.exists())
        {

           SQL::instance()->executeScript(":/sqlite3_createTables.sql", db);
           Parameters p;
           p.lat = currCity->center.lat();
           p.lon = currCity->center.lon();
           p.city = currCity->name();
           p.bAlphaRoutes = true;
           SQL::instance()->insertParameters(p, db);
        }
   }
   else // MySql
   {
    connection->setHost(ui->txtHost->text());
    connection->setPort(ui->txtPort->text().toInt());
    connection->setUserId(ui->txtUserId->text());
    connection->setPWD(ui->txtPWD->text());
    connection->setServerType(ui->cbDbType->currentText());
    connection->setDefaultSqlDatabase(ui->txtUseDatabase->text());
    if(ui->cbConnect->currentText() == "ODBC")
    {
     connection->setOdbcConnectorName(ui->cbODBCDsn->currentText());
    }
   }

   if(ui->cbCities->currentText() == config->currCity->name() && connection->uniqueId() == config->currConnection->uniqueId())
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

#if 0
 for(int i= config->cityList.count()-1; i >=0; i--)
 {
  City* currCity = config->cityList.at(i);
  if(currCity->connections.count()==0)
  {
   config->cityList.removeAt(i);
  }
 }
 if(ui->cbCities->currentText() == config->currCity->name()
    && !config->currCity->connections.at(config->currentCityId)->isOpen())
 {
  int cityIx = ui->cbCities->currentIndex();
  QString city = ui->cbCities->currentText();
  config->currCity= config->cityList.at(config->cityNames().indexOf(city));
  int connIx = ui->cbConnections->currentIndex();
  QString connect = ui->cbConnections->currentText();
  //config->currCity->connections.at(connIx-1);
  config->currentCityId = config->currCity->id;

  config->currCity->curConnectionId = config->currCity->connections.at(connIx)->id();
  config->currConnection = config->currCity->connections.at(connIx);
  qDebug() << "new connection selected: " << config->currConnection->description();
 }
#endif
 this->accept();
 this->close();
}

void EditConnectionsDlg::btnDeleteClicked()
{
 for(int j=0; j< config->cityList.count(); j++)
 {
  City * currCity = config->cityList.at(j);
  if(ui->cbCities->currentText() == currCity->name())
  {
   for(int i =0; i< currCity->connections.count(); i++)
   {
    Connection* c = config->currCity->connections.at(i);
    if(ui->cbConnections->currentText() == c->description())
    {
     //currCity->connections.remove(c->description());
        currCity->connections.removeOne(c);
        //currCity->connectionNames.removeOne(c->description());
        currCity->connectionMap.remove(c->uniqueId().toString());
     ui->cbConnections->clear();
     //ui->cbConnections->addItem(tr("Add new connection"));
     for(i=0; i < currCity->connections.count(); i++)
     {
      Connection* c = (Connection*)&(currCity->connections.at(i));
      ui->cbConnections->addItem(c->description(),VPtr<Connection>::asQVariant(c));
      c->setId(i);
      //currCity->connections.replace(i,c);
     }
    }
    break;
   }
   if(config->currCity->id == currCity->id)
    config->currCity->connections = currCity->connections;
  }
  config->saveSettings();
 }
 for(int i= config->cityList.count()-1; i >=0; i--)
 {
  City* currCity = config->cityList.at(i);
  if(currCity->connections.count()==0)
  {
   config->cityList.removeAt(i);
  }
 }
}

void EditConnectionsDlg::cbConnectionsTextChanged(QString text)
{
 bCbConnectionsTextChanged=true;
}

void EditConnectionsDlg::cbConnectionsLeave()
{
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
    ui->btnNew->setEnabled(true);
    ui->btnSave->setEnabled(false);
    ui->cbDriverType->setCurrentIndex(2);
    ui->txtDbOrDSN->setText("");
    ui->txtHost->setText("");
    ui->txtPort->setText("");
    ui->txtPWD->setText("");
    ui->txtUserId->setText("");
    ui->lblHelp->setText("");
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
    ui->cbDbType->setCurrentIndex(-1);
    connection = new Connection();
    connection->setCityName(currCity->name());
    connection->setConnectionName("");
    ui->cbConnections->addItem("",VPtr<Connection>::asQVariant(connection));
    ui->cbConnections->setCurrentIndex(ui->cbConnections->findData(VPtr<Connection>::asQVariant(connection)));
    ui->cbConnections->lineEdit()->setPlaceholderText(tr("enter unique connction name"));
    //ui->cbConnections->lineEdit()->setPlaceholderText(tr("enter connetion description"));
    ui->cbDriverType->setCurrentIndex(-1);
    ui->cbDbType->setCurrentIndex(-1);
    ui->cbConnect->setCurrentIndex(-1);
    ui->btnSave->setEnabled(false);
    ui->cbDriverType->setFocus();
}

bool EditConnectionsDlg::testConnection(bool bCreate)
{
 if(!openTestDb())
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
     ui->lblHelp->setText(db.lastError().text());
  return false;
 }
 this->setCursor(QCursor(Qt::WaitCursor));

 timer->start(1000);
 ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
 ui->lblHelp->setText(tr("Testing connection..."));
 qApp->processEvents();
 if(ui->cbDbType->currentText() == "Sqlite")
 {
  QString fn = ui->txtDbOrDSN->text();
  if(!fn.toLower().endsWith(".sqlite3"))
   fn.append(".sqlite3");
  QFileInfo file(fn);
  if(!file.isAbsolute())
      file = QFileInfo("Resources/databases/"+fn);
  if(file.exists() && !file.isWritable())
  {
   MyMessageBox::warning(this, tr("Warning"), tr("The file pathname '%1' is invalid or not writable").arg(file.absoluteFilePath()));
   return false;
  }
  if(!file.exists() || db.tables().count() == 0)
  {
     if(!bCreate)
     {
         ui->lblHelp->setStyleSheet("QLabel {  color : #FF8000; }");
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
     SQL::instance()->executeScript(":/CreateMySqlFunctions.sql");
     Parameters p;
     p.lat = currCity->center.lat();
     p.lon = currCity->center.lon();
     p.city = currCity->name();
     p.bAlphaRoutes = true;
     SQL::instance()->insertParameters(p, db);
    }
   }
   else
   {
    this->setCursor(QCursor(Qt::ArrowCursor));
    return false;
   }
  }
  //ui->txtDbOrDSN->setText(file.completeBaseName());
 }

 if(!db.open())
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
  ui->lblHelp->setText( db.lastError().text());
  timer->stop();
  this->setCursor(QCursor(Qt::ArrowCursor));
  return false;
 }

 QStringList tableList = db.tables();
 if(!tableList.contains("Parameters", Qt::CaseInsensitive))
 {
  createSqliteTables(db);
 }

 QString currDb = db.databaseName();
 if(ui->cbDbType->currentText() =="MsSql")
 {
     QString currDb = getDatabase();
     ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
     ui->lblHelp->setText(tr("Connection succeeded. %1 Has %2 tables!<br> %3").arg(currDb).arg(tableList.count()).arg(tableList.join(", ")));
 }
 else
 {
     ui->lblHelp->setStyleSheet("QLabel {  color : green; }");
     ui->lblHelp->setText(tr("Connection succeeded. %1 Has %2 tables!<br> %3").arg(currDb).arg(tableList.count()).arg(tableList.join(", ")));
 }
 timer->stop();
 this->setCursor(QCursor(Qt::ArrowCursor));

 db.close();
 ui->btnSave->setEnabled(true);
 return true;
}

bool EditConnectionsDlg::openTestDb()
{
    setCursor(Qt::WaitCursor);
//    if(ui->txtDbOrDSN->text()=="")
//        return false;
 db = QSqlDatabase::addDatabase(ui->cbDriverType->currentText(),"testConnection");
 if(ui->cbDbType->currentText() == "Sqlite"  )
 {
  QFileInfo info(ui->txtDbOrDSN->text());
  if(info.isAbsolute())
    db.setDatabaseName( ui->txtDbOrDSN->text());
  else
   db.setDatabaseName("Resources/databases/" + ui->txtDbOrDSN->text());
//  ui->txtHost->setText("");
//  db.setHostName(ui->txtHost->text());
 }
 else if(ui->cbConnect->currentText() == "ODBC")
 {
  //QString connector = ui->txtDbOrDSN->text();
  QString connector = ui->cbODBCDsn->currentText();
  db.setDatabaseName(connector);
 }
 else if( ui->cbConnect->currentText()== "Direct")
 { // MySql
  db.setDatabaseName(ui->txtUseDatabase->text());
  db.setHostName(ui->txtHost->text());
  int port = ui->txtPort->text().toInt();
  if(port > 0)
   db.setPort(port);
  db.setUserName(ui->txtUserId->text());
  db.setPassword(ui->txtPWD->text());
 }
 else
     throw IllegalArgumentException(tr("invalid server type: %1").arg(ui->cbDbType->currentText()));
 ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText(tr(""));
 bool bOpen = db.open();
 if(bOpen)
 {
  if(ui->cbDbType->currentText() != "Sqlite" )
  {
      populateDatabases();
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
  QString currDatabase = ui->txtUseDatabase->text();
  QSqlQuery query = QSqlQuery(db);

  if(ui->cbDbType->currentText() == "MsSql")
  {
   // Select * from Sys.Databases
   QStringList databases;
   if(!query.exec("SELECT name FROM sys.databases"))
   {
    SQLERROR(query);
    SQL::instance()->sqlErrorMessage(query,QMessageBox::Ok);
    return false;
   }
   else
   {
    while(query.next())
    {
     databases.append(query.value(0).toString());
     qDebug() << "available: " << query.value(0).toString();
    }
    QCompleter* completer = new QCompleter(databases);
    ui->txtUseDatabase->setCompleter(completer);
    if(ui->txtUseDatabase->text().isEmpty() && databases.count()==1)
     ui->txtUseDatabase->setText(databases.at(0));
    if(!currDatabase.isEmpty() && !databases.contains(currDatabase))
    {
        int rslt = QMessageBox::question(nullptr, tr("Create database?"),
                                         tr("The database %1 does not exist on the server.<br>"
                                            "Do you wish to create it?").arg(currDatabase));
        if(rslt == QMessageBox::Yes)
        {
          if(SQL::instance()->useDatabase("master", db))
          {
            if(SQL::instance()->createSqlDatabase(currDatabase, db, "MsSql"))
            {
//          SQL::instance()->createSqlDatabase(currDatabase, db, ui->cbDbType->currentText());
             SQL::instance()->executeScript(":/CreateMySqlFunctions.sql");
            }
          }
        }
        if(rslt == QMessageBox::No)
            return false;
    }
    else
    {
        // database exists
        qDebug() << "database " << currDatabase << " exists";
    }
   }
  }
  else
  {
   // MySql
   QStringList list = SQL::instance()->showMySqlDatabases(db);
   QCompleter* completer = new QCompleter(list);
   ui->txtUseDatabase->setCompleter(completer);
   if(ui->txtUseDatabase->text().isEmpty() && list.size()==1)
       ui->txtUseDatabase->setText(list.at(0));
   if(!list.contains(currDatabase))
   {
         QString database = currDatabase;
         if(db.isOpen())
         {
          QSqlQuery query = QSqlQuery(db);
          bool retry = true;
          while (retry)
          {
            if(!database.isEmpty())
            {
              QString commandText = "use " + database;
              if(!query.exec(commandText))
              {
                if(query.lastError().databaseText().contains("Unknown database"))
                {
                  int rslt = QMessageBox::question(nullptr, tr("Create database?"), tr("The database %1 does not exist on the server.<br>Do you wish to create it?").arg(database));
                  if(rslt == QMessageBox::Yes)
                  {
                    SQL::instance()->createSqlDatabase(database, db, ui->cbDbType->currentText());
                    SQL::instance()->executeScript(":/CreateMySqlFunctions.sql");
                  }
                }
                else
                {
                  int rslt = QMessageBox::critical(nullptr, tr("Sql Error"), tr("An SqL error has occurred.\n"
                                            "Sql error:%1\nquery:\" %2\"").arg(query.lastError().text()).arg(query.lastQuery()),
                                            QMessageBox::Retry|QMessageBox::Ignore);
                  if(rslt == QMessageBox::Retry)
                    continue;
                  else retry = false;
                }
            }
           }
            else
                break;
          }
       }
   }
  }

  // use desired db
  if(!currDatabase.isEmpty())
  {
      if(!query.exec("use "+ currDatabase))
      {
        qDebug() << query.lastError().driverText()+ query.lastError().databaseText();
        ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
        ui->lblHelp->setText(query.lastError().driverText() + query.lastError().databaseText());
        return false;
      }
      ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
      ui->lblHelp->setText(tr("using ")+currDatabase);
  }
  return true;
}

QString EditConnectionsDlg::getDatabase()
{
 if(!openTestDb()) return QString();

 QSqlQuery query = QSqlQuery(db);
 QString dbName ="";

 if(!query.exec("SELECT DB_NAME() "))
 {
  SQLERROR(query);
 }
 else
 {
  while(query.next())
  {
   if(!query.value(0).isNull())
   dbName = query.value(0).toString();
  }
  ui->txtUseDatabase->setText(dbName);
 }
 return dbName;
}

void EditConnectionsDlg::quickProcess()
{
 qApp->processEvents();
}

void EditConnectionsDlg::txtHostLeave()
{
    ui->lblHelp->setStyleSheet("QLabel {  color : red; }");
 ui->lblHelp->setText("");
 qint16 port;
 if(ui->cbDriverType->currentText() == "QMYSQL" || ui->cbDriverType->currentText() == "QMYSQL3")
  port = 3306;
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
 qint16 port;
 if(ui->cbDriverType->currentText() == "QMYSQL" || ui->cbDriverType->currentText() == "QMYSQL3")
  port = 3306;
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
 populateDatabases();
 if(!(ui->cbDriverType->currentText() == "ODBC" || ui->cbDriverType->currentText() == "ODBC3"))
 {
  SQL* sql = SQL::instance();
  databases = sql->showDatabases("testConnection", ui->cbDbType->currentText());
  qDebug() << " databases: " << databases;
  if(databases.count() >0)
  {
   ui->txtDbOrDSN->setText(databases.at(0));
  }
 }
}

void EditConnectionsDlg::txtDsnTextChanged(QString text)
{
//    if(ui->cbDriverType->currentText() == "QODBC" || ui->cbDriverType->currentText() == "QODBC3")
//        return;
 QCompleter *completer = new QCompleter(databases, this);
 completer->setCaseSensitivity(Qt::CaseInsensitive);
 ui->txtDbOrDSN->setCompleter(completer);
}

void EditConnectionsDlg::on_tbBrowse_clicked()
{
 QString basePath = MainWindow::pwd + QDir::separator() + "Resources" + QDir::separator() +"databases" + QDir::separator();
// QFileInfo info(ui->txtDbOrDSN->text());
// if(info.exists())
//  basePath=info.canonicalPath();

 QString path = QFileDialog::getOpenFileName(this, "Select Sqlite database",basePath, "Sqlite3 databases (*.sqlite3);; All Files (*)");
 if(!path.isEmpty())
 {
  QFileInfo pInfo(path);
  if(pInfo.isAbsolute() && !path.startsWith( basePath))
  {
    ui->txtDbOrDSN->setText(path);
  }
  else
   ui->txtDbOrDSN->setText(pInfo.fileName());
 }
}

bool EditConnectionsDlg::createSqliteTables(QSqlDatabase db)
{
 if(!db.open())
  return false;
 QFile file(":/sqlite3_create_tables.sql");
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
void EditConnectionsDlg::ontxtDbOrDsn_editingFinished()
{
 QString dbName = ui->txtDbOrDSN->text();
 if(ui->cbDbType->currentText() == "Sqlite")
 {
  QFileInfo info;
  if(!ui->txtDbOrDSN->text().toLower().endsWith("sqlite3"))
   ui->txtDbOrDSN->setText(ui->txtDbOrDSN->text().append(".sqlite3"));
  info.setFile(ui->txtDbOrDSN->text());
  if(info.isAbsolute())
  ui->txtDbOrDSN->setText(info.absoluteFilePath());
  else
   ui->txtDbOrDSN->setText(info.fileName());
 }
 else if (ui->cbDbType->currentText() == "Msql")
 {
  populateDatabases();
  getDatabase();
 }
 else if(ui->cbDbType->currentText() == "MySql")
 {
  ;
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
        if(SQL::instance()->useDatabase("master", db))
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
