#include "connection.h"
#include <qsqldatabase.h>
#include <QDebug>
#include <QDir>
#include "configuration.h"
#include "sql.h"
#include <QSqlQuery>
#include "exportsql.h"

Connection::Connection(QObject *parent) : QObject(parent)
{
 //_DSN = "";
 //_servertype = "Sqlite";
 bOpen = false;
 _uuid = QUuid::createUuid();
}

Connection::Connection(QUuid uuid, QObject* parent) : QObject(parent)
{
  _uuid = uuid;
}

Connection::Connection(const Connection& o){
 this->_id = o._id;
 this->_driver = o._driver;
 this->_description =o._description;
 this->_DSN =o._DSN;
 this->_userId = o._userId;
 this->_PWD = o._PWD;
 this->_database = o._database;
 this->_hostName = o._hostName;
 this->_port = o._port;
 this->_useDatabase = o._useDatabase;
 this->db = o.db;
 this->_servertype = o._servertype; // "MsSql (default), "MySql"
 this->bOpen = o.bOpen;
 this->_cityName = o._cityName;
 this->_connectionName = o._connectionName;
 this->_sqlite_fileName = o._sqlite_fileName;
 this->_odbc_connectorName = o._odbc_connectorName;
 this->_defaultSqlDatabase = o._defaultSqlDatabase;
 //this->_mySqlDatabase = o._mySqlDatabase;
 this->_connectionType = o._connectionType;
 this->_uuid = o._uuid;
}


QSqlDatabase Connection::getDb() { return db;}

QString Connection::dbType(QString name)
{
 if(name == "QSQLITE") return "Sqlite";
 if(name == "QMARIADB") return "MySql";
 if(name == "QMYSQL") return "MySql";
 if(name == "QMYSQL3") return "MySql";
 if(name == "QODBC") return "MsSql";
 if(name == "QODBC3") return "MsSql";
 if(name == "QPSQL") return "PGres";
 if(name == "QPSQL7") return "PGres";
 if(name == "QOCI") return "Oracle";
 return "";
}

QSqlDatabase Connection::configure(const QString cName)
{
 qDebug() << "Connection: CWD = " << QDir::currentPath();
 config = Configuration::instance();
 sql = SQL::instance();
  db = QSqlDatabase::addDatabase(config->currConnection->driver(),cName);
 configureDb(&db, this);
 // check for presence of Parameters table.
 QStringList tableList;
 QStringList sysTableList;

// if(ok)
// {
 if((bOpen = db.open()))
 {
  if(config->currConnection->servertype() != "Sqlite")
  {
      sql->checkTables(db);
      //return db;
  }
  if(config->currConnection->defaultSqlDatabase()  != "")
  {
   QSqlQuery query = QSqlQuery(db);
   //QString cmd = QString("use [%1]").arg(config->currConnection->mySqlDatabase());
   QString cmd = QString("use %1").arg(config->currConnection->defaultSqlDatabase());
   if(!query.exec(cmd))
   {
    SQLERROR(query);
    db.close();
    bOpen = false;
    return db;
   }
   if(config->currConnection->servertype() == "Sqlite" )
    sql->checkTables(db);
   tableList = db.tables();
   sysTableList = db.tables(QSql::SystemTables);
   ExportSql* eSql = new ExportSql(config, false);
   bool tableError = false;
   try {
       if(!tableList.contains("Parameters",Qt::CaseInsensitive))
       {
        if(!eSql->createParametersTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Comments",Qt::CaseInsensitive))
       {
        if(!eSql->createCommentsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("RouteComments",Qt::CaseInsensitive))
       {
        if(!eSql->createRouteCommentsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Segments",Qt::CaseInsensitive))
       {
        if(!eSql->createSegmentsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Companies",Qt::CaseInsensitive))
       {
        if(!eSql->createCompaniesTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Intersections",Qt::CaseInsensitive))
       {
        if(!eSql->createIntersectionsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("altRoute",Qt::CaseInsensitive))
       {
        if(!eSql->createAltRouteTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Terminals",Qt::CaseInsensitive))
       {
        if(!eSql->createTerminalsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Stations",Qt::CaseInsensitive))
       {
        if(!eSql->createStationsTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("TractionTypes",Qt::CaseInsensitive))
       {
        if(!eSql->createTractionTypesTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!tableList.contains("Routes",Qt::CaseInsensitive))
       {
        if(!eSql->createRouteTable(db, config->currConnection->servertype()))
          throw Exception();
       }
       if(!SQL::instance()->doesFunctionExist("distance", db))
       {
//           if(!eSql->createMsSqlFunctions(db))
//               throw Exception("distance function error");
       }
   }
   catch (Exception ex)
   {
      tableError = true;
   }
   if(tableError)
   {
    QSqlError err = db.lastError();
    QString msg;
    msg = err.text() + "\n"
           + db.driverName() + "\n"
           + "User:" + db.userName() + "\n"
           + "Host:" + db.hostName()+ "\n"
           + "Connection name: " + db.connectionName() + "\n"
           + "DSN:" + db.databaseName() + "\n"
    + "driver: " + config->currConnection->driver() + "\n";
    if(config->currConnection->driver()=="QSQLITE")
    {
     QFileInfo info(db.databaseName());
     msg.append(info.absoluteFilePath());
    }
    QMessageBox::warning(NULL, "Warning", msg);
   }
  }
  if(sql->loadSqlite3Functions() && config->currConnection->servertype() == "Sqlite")
  {
   sql->checkTables(db);
  }
 }
 else
 {
  qDebug() << "unable to open " << db.databaseName() << " " << db.lastError().text();
  QFileInfo info(db.databaseName());
  if(info.exists())
   qDebug() << "file exists.";
  else
   qDebug() << "file not found.";
 }
  return db;
}

void Connection::configureDb(QSqlDatabase* db, Connection* currConnection)
{
    QString driver = db->driverName();
    if(currConnection->connectionType() == "Local" )
    {
        QString dbName = currConnection->sqlite_fileName();
        QFileInfo info(dbName);
        if(!info.isAbsolute() )
        {
         if(!dbName.startsWith("Resources/databases/"))
          dbName = "Resources/databases/" + dbName;
         if(!dbName.endsWith(".sqlite3"))
          dbName.append(".sqlite3");
         //dbName = QDir(config->path + QDir::separator() + QDir::separator()+ dbName).path();
        }
        db->setDatabaseName(dbName);
    }
    else if(currConnection->connectionType() == "Direct")
    {
        if(currConnection->host() != "")
         db->setHostName(currConnection->host());
        if(currConnection->port() > 0)
         db->setPort(currConnection->port());
        db->setUserName(currConnection->userId());
        db->setPassword(currConnection->pwd());
        db->setDatabaseName(currConnection->defaultSqlDatabase());
    }
    else if(currConnection->connectionType() == "ODBC")
    {
        db->setDatabaseName(currConnection->odbc_connectorName());
    }
    else
    {
     throw IllegalArgumentException(tr("invalid driver name: '%1'").arg(driver));
    }
    if(currConnection->connectionType() != "Local" )
    {
      if(db->open() && !currConnection->defaultSqlDatabase().isEmpty())
      {
        QSqlQuery query = QSqlQuery(*db);
        if(!query.exec(QString("use %1").arg(currConnection->defaultSqlDatabase())))
        {
            QMessageBox::critical(nullptr, tr("Sql error"), tr("An sql error has occured! \n")
                         + query.lastError().text() + " query: " + query.lastQuery());
        }
      }
    }
}
