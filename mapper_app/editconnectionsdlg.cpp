#include "editconnectionsdlg.h"
#include "ui_editconnectionsdlg.h"
#include <QCompleter>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include <QClipboard>
#include <QApplication>

EditConnectionsDlg::EditConnectionsDlg( QWidget *parent) :
  QDialog(parent),
  ui(new Ui::editConnectionsDlg)
{
  ui->setupUi(this);
  ui->lblHelp->setText("");
  config = Configuration::instance();
  this->setWindowTitle(tr("Edit Connections"));
  drivers = QSqlDatabase::drivers();
  dbType <<"MySql"<<"MsSql"<<"Sqlite";  // currently supported database types.
  for(int i=0; i < dbType.count(); i++)
  {
   ui->cbDbType->addItem(dbType.at(i));
  }
  for(int i=0; i < drivers.count(); i++)
  {
   ui->cbDriverType->addItem(drivers.at(i));
  }
  connect(ui->cbDriverType, SIGNAL(currentIndexChanged(int)), this, SLOT(cbDriverTypeSelectionChanged(int)));
  ui->cbDriverType->setCurrentText("QSQLITE");

  connect(ui->cbDbType, &QComboBox::currentTextChanged, [=]{
   setControls((DBTYPE)ui->cbDbType->currentIndex());
  });
  ui->cbDbType->setCurrentIndex(Sqlite);

  int index=0, i=0;
  ui->cbCities->clear();
  foreach(City* c, config->cityList)
  {
   ui->cbCities->addItem(c->name());
   if(c->id == config->currCity->id)
    index = i;
   i++;
  }
  connect(ui->cbCities, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitiesSelectionChanged(int)));
  connect(ui->cbCities, SIGNAL(editTextChanged(QString)), this, SLOT(cbCitiesTextChanged(QString)));
  connect(ui->cbCities->lineEdit(), SIGNAL(editingFinished()), this, SLOT(cbCitiesLeave()));
  ui->cbCities->setCurrentIndex(index);

  index = 0;
  ui->cbConnections->addItem(tr("Add new connection"));
  for(int i=0; i < config->currCity->connections.count(); i++)
  {
   City* selectedCity = config->cityList.value(ui->cbCities->currentText());
   Connection* c = selectedCity->connections.values().at(i);
   ui->cbConnections->addItem(c->description());
   if(c->id() == config->currCity->curConnectionId)
    index = i+1;
  }
  connect(ui->cbConnections, SIGNAL(currentIndexChanged(int)), this, SLOT(cbConnectionsSelectionChanged(int)));
  connect(ui->cbConnections, SIGNAL(editTextChanged(QString)), this, SLOT(cbConnectionsTextChanged(QString)));
  ui->cbConnections->setCurrentIndex(index);

  connect(ui->btnTest,SIGNAL(clicked(bool)), this, SLOT(btnTestClicked()));
  connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(btnCancelClicked()));
  connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOKClicked()));
  connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDeleteClicked()));
  connect(ui->btnAbort, &QPushButton::clicked, [=]{exit(EXIT_FAILURE);});
    //connect(ui->cbConnections, SIGNAL(signalFocusOut()),this, SLOT(cbConnectionsLeave()));
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


  timer = new QTimer(this);
  timer->setInterval(1000);
  connect(timer, SIGNAL(timeout()), this, SLOT(quickProcess()));
}

EditConnectionsDlg::~EditConnectionsDlg()
{
 delete ui;
}

void EditConnectionsDlg::cbCitiesSelectionChanged(int sel)
{
 Q_UNUSED(sel)
 QString name = ui->cbCities->currentText();
 foreach(City* c, config->cityList)
 {
  if(c->name() == name)
  {
//   ui->cbConnections->clear();
//   ui->cbConnections->addItem(tr("Add new connection"));
//   foreach(connection cn, c->connections)
//   {
//    ui->cbConnections->addItem(cn.description);
//   }
   ui->cbConnections->setCurrentIndex(c->curConnectionId+1);
   cbConnectionsSelectionChanged(c->curConnectionId+1);
   return;
  }
 }

}
void EditConnectionsDlg::cbCitiesTextChanged(QString text)
{
    bCbCitiesTextChanged = true;
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
    ui->cbConnections->addItem(tr("Add new connection"));
    ui->txtDbOrDSN->setText("");

    foreach(Connection* cn, c->connections)
    {
     ui->cbConnections->addItem(cn->description());
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
  config->cityList.insert(name, c);
  c->id = config->cityList.count()-1;
  ui->cbConnections->clear();
  ui->cbConnections->addItem(tr("Add new connection"));
  ui->txtDbOrDSN->setText("");
 }
}

void EditConnectionsDlg::cbConnectionsSelectionChanged(int sel)
{
 ui->lblHelp->setText("");
 if(sel < 0)
  return;

 Connection* c;
 if(sel== 0)
 {
  ui->btnOK->setText(tr("Add"));
  ui->btnDelete->setEnabled(false);
  ui->cbDriverType->setCurrentIndex(-1);
  return;
 }
 else
 {
  ui->btnDelete->setEnabled(false);
  City * currCity = NULL;
  for(int i = 0; i < config->cityList.count(); i++)
  {
   currCity = config->cityList.values().at(i);
   if(currCity->name() == ui->cbCities->currentText())
   {
    currCity = config->cityList.values().at(i);
    if((sel-1) > currCity->connections.count()-1)
     sel = currCity->connections.count();
    c = currCity->connections.values().at(sel-1);
    ui->btnOK->setText(tr("Update"));
    //if(c->id != config->currConnection.id) //??
    ui->btnDelete->setEnabled(true);
    ui->cbDriverType->setCurrentIndex(-1);
    for(int i =0; i<drivers.count(); i++)
    {
     if(drivers.at(i)== c->driver())
     {
      ui->cbDriverType->setCurrentIndex(i);
      break;
     }
    }
    ui->cbDbType->setCurrentText(c->servertype());
    switch ((DBTYPE)ui->cbDbType->currentIndex()) {
    case Sqlite:
     ui->txtDbOrDSN->setText(c->database());
     break;
    case MySql:
     ui->txtHost->setText(c->host());
     ui->txtPort->setText(QString::number(c->port()));
     ui->txtUID->setText(c->uid());
     ui->txtPWD->setText(c->pwd());
     if(openTestDb())
     {
      QStringList list = SQL::instance()->showMySqlDatabases(db);
      ui->cbUseDatabase->clear();
      ui->cbUseDatabase->addItems(list);
     }
     ui->cbUseDatabase->setCurrentText(c->useDatabase());
     break;
    case MsSql:
     ui->txtHost->setText(c->host());
     ui->txtPort->setText(QString::number(c->port()));
     ui->txtUID->setText(c->uid());
     ui->txtPWD->setText(c->pwd());
     if(openTestDb())
     {
      QStringList list = SQL::instance()->showMsSqlDatabases(db);
      ui->cbUseDatabase->clear();
      ui->cbUseDatabase->addItems(list);
     }
     ui->cbUseDatabase->setCurrentText(c->useDatabase());
     ui->txtDbOrDSN->setText(c->database());
    }
   }
  }
 }
}

void EditConnectionsDlg::cbDriverTypeSelectionChanged(int sel)
{
 Q_UNUSED(sel)
 QString text = ui->cbDriverType->currentText();
 if(text == "QSQLITE")
 {
  ui->cbDbType->setCurrentIndex(Sqlite);
 }
 else if(text == "QODBC" || text == "QODBC3")
 {
  ui->cbDbType->setCurrentIndex(MsSql);
 }
 else
 {
  ui->cbDbType->setCurrentIndex(MySql);
 }
}

void EditConnectionsDlg::setControls(DBTYPE dbType)
{
 switch (dbType) {
 case MySql:
  ui->label_2->setText("Database:");
  ui->cbDbType->setCurrentIndex(0);
  ui->txtHost->setEnabled(true);
  ui->txtPort->setEnabled(true);
  ui->txtDbOrDSN->setText("");
  ui->txtDbOrDSN->setPlaceholderText("select MySqlDatabase");
  ui->cbConnections->setCurrentText(tr("MySql Connection"));
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUID->setEnabled(true);
  ui->txtPort->setText("3306");

  // use database (required for ODBC)
  ui->label_10->setVisible(true);
  ui->cbUseDatabase->setVisible(true);
  // host (required for ODBC, MYSQL)
  ui->label_4->setVisible(true);
  ui->label_5->setVisible(true);
  ui->txtHost->setVisible(true);
  ui->txtPort->setVisible(true);
  // userId/Password (required for ODBC, MYSQL)
  ui->label_6->setVisible(true);
  ui->txtUID->setVisible(true);
  ui->label_7->setVisible(true);
  ui->txtPWD->setVisible(true);
  ui->tbView->setVisible(true);
  ui->tbBrowse->setVisible(false); // browse, only for Sqlite.
  ui->label_2->setVisible(false);
  ui->label_2->setText(tr("DSN")+":");
  ui->txtDbOrDSN->setVisible(false);
  break;

 case MsSql:
  ui->label_2->setText("DSN:");
  ui->cbDbType->setCurrentIndex(1);
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->txtPWD->setEnabled(true);
  ui->tbView->setEnabled(true);
  ui->txtUID->setEnabled(true);
#ifdef Q_WS_WIN
  QSettings winReg("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
  databases = winReg.childKeys();
  qDebug() << QString("%1").arg(databases.count());
#else
  findODBCDsn(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini", &databases);
  findODBCDsn("/etc/odbc.ini", &databases);
#endif
  // use database (required for ODBC)
  ui->label_10->setVisible(false);
  ui->cbUseDatabase->setVisible(false);
  // host (required for ODBC, MYSQL)
  ui->label_4->setVisible(true);
  ui->label_5->setVisible(true);
  ui->txtHost->setVisible(true);
  ui->txtPort->setVisible(true);
  // userId/Password (required for ODBC, MYSQL)
  ui->label_6->setVisible(true);
  ui->txtUID->setVisible(true);
  ui->label_7->setVisible(true);
  ui->txtPWD->setVisible(true);
  ui->tbView->setVisible(true);
  ui->tbBrowse->setVisible(false); // browse, only for Sqlite.
  ui->label_2->setVisible(true);
  ui->label_2->setText(tr("Schema")+":");
  ui->txtDbOrDSN->setVisible(true);
 break;
 case Sqlite:
  ui->label_2->setText("Database:");
  ui->cbDbType->setCurrentIndex(2);  // "Sqlite"
  ui->txtHost->setEnabled(false);
  ui->txtPort->setEnabled(false);
  ui->tbBrowse->setVisible(true);
//#ifdef Q_WS_WIN
  ui->txtPWD->setEnabled(false);
  ui->tbView->setEnabled(false);
  ui->txtUID->setEnabled(false);
  ui->txtHost->setText("");
  ui->txtPort->setText("");
  ui->txtPWD->setText("");
  ui->txtUID->setText("");

  // use database (required for ODBC)
  ui->label_10->setVisible(false);
  ui->cbUseDatabase->setVisible(false);
  // host (required for ODBC, MYSQL)
  ui->label_4->setVisible(false);
  ui->label_5->setVisible(false);
  ui->txtHost->setVisible(false);
  ui->txtPort->setVisible(false);
  // userId/Password (required for ODBC, MYSQL)
  ui->label_6->setVisible(false);
  ui->txtUID->setVisible(false);
  ui->label_7->setVisible(false);
  ui->txtPWD->setVisible(false);
  ui->tbView->setVisible(false);
  ui->tbBrowse->setVisible(true); // browse, only for Sqlite.
  ui->label_2->setVisible(true);
  ui->label_2->setText(tr("DSN")+":");
  ui->txtDbOrDSN->setVisible(true);break;
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
 qApp->processEvents();
 testConnection();
}

void EditConnectionsDlg::btnCancelClicked()
{
 this->rejected();
 this->close();
}

void EditConnectionsDlg::btnOKClicked()
{
 ui->lblHelp->setText(tr(""));
 if(ui->cbConnections->currentText() == "Add new connection")
 {
  ui->lblHelp->setText(tr("Enter the new connection's description"));
  return;
 }
 if(ui->txtDbOrDSN->text() == "" && ui->cbUseDatabase->currentText() == "Sqlite")
 {
  ui->lblHelp->setText(tr("Database or DSN name required"));
  return;
 }
 if(ui->cbDriverType->currentText() == "")
 {
  ui->lblHelp->setText(tr("Select driver Type"));
  return;
 }
 if(ui->cbDriverType->currentText() == "QSQLITE")
 {
  if(ui->cbDbType->currentText() != "Sqlite")
  {
   ui->lblHelp->setText(tr("Invalid db type!"));
   return;
  }
 }
 if(ui->cbDriverType->currentText() == "QMYSQL" ||  ui->cbDriverType->currentText() == "QMYSQL3")
 {
  if(ui->cbDbType->currentText() != "MySql")
  {
   ui->lblHelp->setText(tr("Invalid db type!"));
   return;
  }
 }
 if(ui->cbDriverType->currentText() != "QODBC" && ui->cbDriverType->currentText() != "QODBC3" && ui->cbDriverType->currentText() != "QSQLITE")
 {
  if(ui->txtHost->text() == "")
  {
   ui->lblHelp->setText(tr("Host name or IP required"));
   return;
  }
  //ui->cbDbType->setCurrentIndex(0); // MySql
 }
// else
// {
//  if(ui->txtUID->text() == "")
//  {
//   ui->lblHelp->setText(tr("User Id required"));
//   return;
//  }
 if(ui->cbDbType->currentText() != "Sqlite")
 {
  if(ui->txtUID->text() == "")
  {
   ui->lblHelp->setText(tr("User Id required"));
   return;
  }
 }
 QString savedb = ui->cbUseDatabase->currentText();
 if(!testConnection())
  return;

 City * currCity = NULL;
 for(int i=0; i<config->cityList.count(); i++)
 {
  currCity = config->cityList.values().at(i);
  if(currCity->name().isEmpty()) currCity->setName(ui->cbCities->currentText());
  if(currCity->name() == ui->cbCities->currentText())
  {
   Connection* c = new Connection();
   if(ui->cbConnections->currentIndex() < 1)
   {
    //c = new connection();
    c->setDescription(ui->cbConnections->currentText());
    c->setId(currCity->connections.count());
   }
   else
   {
    c = currCity->connections.values().at(ui->cbConnections->currentIndex()-1);
    c->setDescription(ui->cbConnections->currentText());
   }
   if(ui->cbDbType->currentText() == "MsSql")
   {
    c->setDSN(ui->txtDbOrDSN->text());
    //QString useDatabase = ui->cbUseDatabase->currentText();
    c->setUseDatabase(savedb);
   }
   else
   {
    QFileInfo info(ui->txtDbOrDSN->text());
    if(!info.isAbsolute())
    {
     c->setDatabase(info.completeBaseName());
    }
    else
     c->setDatabase(ui->txtDbOrDSN->text());
    c->setHost(ui->txtHost->text());
    c->setPort(ui->txtPort->text().toInt());
   }
   c->setUID(ui->txtUID->text());
   c->setPWD(ui->txtPWD->text());
   c->setServerType(ui->cbDbType->currentText());
   c->setDriver(ui->cbDriverType->currentText());
   c->setUseDatabase(ui->cbUseDatabase->currentText());

   if(ui->cbCities->currentText() == config->currCity->name() && c->id() == config->currConnection->id())
    config->currConnection = c;
   config->currentCityId = config->currCity->id;
   if(ui->cbConnections->currentIndex() < 1)
   {
    c->setId(currCity->connections.count());
    currCity->connections.insert(c->description(),c);
   }
   else
    currCity->connections.insert(ui->cbConnections->currentText(),c);

   if(config->currCity->id == currCity->id)
    config->currCity->connections = currCity->connections;

//   ui->cbConnections->clear();
//   ui->cbConnections->addItem(tr("Add new connection"));
//   for(int i=0; i < currCity->connections.count(); i++)
//   {
//    Connection* c = currCity->connections.at(i);
//    ui->cbConnections->addItem(c->description());
//   }
   //config->cityList.replace(config->currentCityId, config->currCity);
   config->saveSettings();
  }
 }
 for(int i= config->cityList.count()-1; i >=0; i--)
 {
  City* currCity = config->cityList.values().at(i);
  if(currCity->connections.count()==0)
  {
   config->cityList.remove(currCity->name());
  }
 }
 if(ui->cbCities->currentText() == config->currCity->name()
    && !config->currCity->connections.values().at(config->currentCityId)->isOpen())
 {
  int cityIx = ui->cbCities->currentIndex();
  QString city = ui->cbCities->currentText();
  config->currCity= config->cityList.value(city);
  int connIx = ui->cbConnections->currentIndex();
  QString connect = ui->cbConnections->currentText();
  //config->currCity->connections.at(connIx-1);
  config->currentCityId = config->currCity->id;

  config->currCity->curConnectionId = config->currCity->connections.values().at(connIx-1)->id();
  config->currConnection = config->currCity->connections.values().at(connIx-1);
  qDebug() << "new connection selected: " << config->currConnection->description();
 }
 this->accept();
 this->close();
}

void EditConnectionsDlg::btnDeleteClicked()
{
 for(int j=0; j< config->cityList.count(); j++)
 {
  City * currCity = config->cityList.values().at(j);
  if(ui->cbCities->currentText() == currCity->name())
  {
   for(int i =0; i< currCity->connections.count(); i++)
   {
    Connection* c = config->currCity->connections.values().at(i);
    if(ui->cbConnections->currentText() == c->description())
    {
     currCity->connections.remove(c->description());
     ui->cbConnections->clear();
     ui->cbConnections->addItem(tr("Add new connection"));
     for(i=0; i < currCity->connections.count(); i++)
     {
      Connection* c = (Connection*)&(currCity->connections.values().at(i));
      ui->cbConnections->addItem(c->description());
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
  City* currCity = config->cityList.value(currCity->name());
  if(currCity->connections.count()==0)
  {
   config->cityList.remove(currCity->name());
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
  int ix = ui->cbConnections->currentIndex();
  if(ix > 0)
  {
   Connection* c = config->currCity->connections.values().at(ix-1);
   c->setDescription(ui->cbConnections->currentText());
   //config->currCity->connections.replace(ix-1, c);
   config->currCity->connections.insert(c->description(), c);
  }
 }
 bCbConnectionsTextChanged=false;
}

bool EditConnectionsDlg::testConnection()
{
 if(!openTestDb())
  return false;
 this->setCursor(QCursor(Qt::WaitCursor));

 timer->start(1000);
 ui->lblHelp->setText(tr("Testing connection..."));
 qApp->processEvents();
 if(ui->cbDbType->currentText() == "Sqlite")
 {
  QString fn = ui->txtDbOrDSN->text();
  if(!fn.toLower().endsWith(".sqlite3"))
   fn.append(".sqlite3");
  QFileInfo file(fn);
//  if(!file.isAbsolute())
//      file = QFileInfo("Resources/databases/"+fn);
  if(file.exists() && !file.isWritable())
  {
   QMessageBox::warning(this, tr("Warning"), tr("The file pathname '%1' is invalid or not writable").arg(file.absoluteFilePath()));
   return false;
  }
  if(!file.exists())
  {
   QMessageBox::StandardButton btn = QMessageBox::information(this, tr("Question"), tr("Sqlite db '%1' does not exist\nDo you wish to create a new database?").arg(file.absoluteFilePath()),QMessageBox::Yes | QMessageBox::No);
   if(btn == QMessageBox::Yes)
   {
    if(db.open())
     createSqliteTables(db);
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

 ui->lblHelp->setText(tr("Connection succeeded"));
 timer->stop();
 this->setCursor(QCursor(Qt::ArrowCursor));

 db.close();
 return true;
}

bool EditConnectionsDlg::openTestDb()
{
//    if(ui->txtDbOrDSN->text()=="")
//        return false;
 db = QSqlDatabase::addDatabase(ui->cbDriverType->currentText(),"testConnection");
 if(ui->cbDbType->currentText() == "Sqlite"  )
 {
  db.setDatabaseName(ui->txtDbOrDSN->text());
  ui->txtHost->setText("");
  db.setHostName(ui->txtHost->text());
 }
 else
 {
  db.setDatabaseName(ui->cbUseDatabase->currentText());
  db.setHostName(ui->txtHost->text());
  int port = ui->txtPort->text().toInt();
  if(port > 0)
   db.setPort(port);
 }
 db.setUserName(ui->txtUID->text());
 db.setPassword(ui->txtPWD->text());
 bool bOpen = db.open();
 if(bOpen)
  return true;
 ui->lblHelp->setText(tr("error: %1 %2").arg(db.driverName()).arg(db.lastError().driverText()));
 return false;
}


void EditConnectionsDlg::populateDatabases()
{
 if(!openTestDb())
  return;
 QString currDatabase = ui->cbUseDatabase->currentText();
  if(ui->cbDriverType->currentText() == "QODBC" || ui->cbDriverType->currentText() == "QODBC3")
  {
   // Select * from Sys.Databases
   QStringList databases;
   QSqlQuery query = QSqlQuery(db);
   if(!query.exec("Select * from Sys.Databases"))
    SQLERROR(query);
   else
   {
    while(query.next())
    {
     QString dbname = query.value(0).toString();
     if(dbname == "master" || dbname == "tempdb" || dbname == "msdb" || dbname == "model")
      continue;
     databases.append(query.value(0).toString());
     qDebug() << "available: " << query.value(0).toString();
    }
    ui->cbUseDatabase->clear();
    ui->cbUseDatabase->addItems(databases);
   }
  }
  else
  {
   // MySql
   QStringList list = SQL::instance()->showMySqlDatabases(db);
   ui->cbUseDatabase->clear();
   ui->cbUseDatabase->addItems(list);
   if(list.contains(currDatabase))
    ui->cbUseDatabase->setCurrentText(currDatabase);
  }
  db.close();
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
   dbName = query.value(0).toString();
  }
  ui->cbUseDatabase->setCurrentIndex( ui->cbUseDatabase->findText(dbName));
 }
 return dbName;
}

void EditConnectionsDlg::quickProcess()
{
 qApp->processEvents();
}

void EditConnectionsDlg::txtHostLeave()
{
 ui->lblHelp->setText("");
 qint16 port;
 if(ui->cbDriverType->currentText() == "QMYSQL" || ui->cbDriverType->currentText() == "QMYSQL3")
  port = 3306;
 if(ui->txtPort->text().toInt()> 0)
  port = ui->txtPort->text().toInt();
 socket.connectToHost(ui->txtHost->text(), port, QIODevice::ReadWrite);
 if(!socket.waitForReadyRead(1000))
 {
  ui->lblHelp->setText(socket.errorString());
  ui->txtHost->setFocus();
 }
 socket.close();
}

void EditConnectionsDlg::txtPortLeave()
{
 ui->lblHelp->setText("");
 qint16 port;
 if(ui->cbDriverType->currentText() == "QMYSQL" || ui->cbDriverType->currentText() == "QMYSQL3")
  port = 3306;
 if(ui->txtPort->text().toInt()> 0)
  port = ui->txtPort->text().toInt();
 socket.connectToHost(ui->txtHost->text(), port, QIODevice::ReadWrite);
 if(!socket.waitForReadyRead(1000))
 {
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
 QString basePath = MainWindow::pwd + QDir::separator() + "Resources/databases" + QDir::separator();
 QFileInfo info(ui->txtDbOrDSN->text());
 if(info.exists())
  basePath=info.canonicalPath();

 QString path = QFileDialog::getOpenFileName(this, "Select Sqlite database",basePath, "Sqlite3 databases (*.sqlite3);; All Files (*)");
 if(!path.isEmpty())
 {
  QFileInfo pInfo(path);
  if(pInfo.isAbsolute())
  {
   QString cwd = QDir::currentPath();
   if(path.startsWith(MainWindow::pwd))
   {
    ui->txtDbOrDSN->setText(path.mid(MainWindow::pwd.length()+1));
   }
  }
  else
   ui->txtDbOrDSN->setText(path);
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
  ui->lblHelp->setText(file.errorString() + " '" + file.fileName()+"'");
  //return;
 }
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
 if(ui->cbDriverType->currentText() == "QSQLITE")
 {
  QFileInfo info;
  if(!ui->txtDbOrDSN->text().toLower().endsWith("sqlite3"))
   ui->txtDbOrDSN->setText(ui->txtDbOrDSN->text().append(".sqlite3"));
  info.setFile(ui->txtDbOrDSN->text());
  if(info.isAbsolute())
  ui->txtDbOrDSN->setText(info.absoluteFilePath());
  else
   ui->txtDbOrDSN->setText(info.completeBaseName());
 }
 else if (ui->cbDriverType->currentText() == "QODBC" || ui->cbDriverType->currentText() == "QODBC3")
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
   ui->txtDbOrDSN->setText("");
  }
  else
  {
    if(db.isOpen())
    {
       int ret = QMessageBox::question(this, tr("Create new database?"), tr("The database '%1' does not exist on the server. Do you wish to create it?").arg(dbName));
       if(ret == QMessageBox::Yes)
       {
        SQL::instance()->createMySqlDatabase(dbName, db);
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
