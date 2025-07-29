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
 //this->_useDatabase = o._useDatabase;
 this->db = o.db;
 this->_servertype = o._servertype; // "MsSql (default), "MySql"
 this->bOpen = o.bOpen;
 this->_cityName = o._cityName;
 this->_connectionName = o._connectionName;
 this->_sqlite_fileName = o._sqlite_fileName;
 //this->_odbc_connectorName = o._odbc_connectorName;
 this->_defaultSqlDatabase = o._defaultSqlDatabase;
 //this->_mySqlDatabase = o._mySqlDatabase;
 this->_connectionType = o._connectionType;
 this->_uuid = o._uuid;
}


QSqlDatabase Connection::getDb() { return db;}

//QString Connection::dbType(QString name)
//{
// if(name == "QSQLITE") return "Sqlite";
// if(name == "QMARIADB") return "MySql";
// if(name == "QMYSQL") return "MySql";
// if(name == "QMYSQL3") return "MySql";
// if(name == "QODBC") return "MsSql";
// if(name == "QODBC3") return "MsSql";
// if(name == "QPSQL") return "PGres";
// if(name == "QPSQL7") return "PGres";
// if(name == "QOCI") return "Oracle";
// return "";
//}

QSqlDatabase Connection::configure(const QString cName)
{
 qDebug() << "Connection: CWD = " << QDir::currentPath();
 config = Configuration::instance();
 sql = SQL::instance();
 db = QSqlDatabase::addDatabase(config->currConnection->driver(),cName);
 configureDb(db, this, config);
 // check for presence of Parameters table.
 QStringList tableList;
 QStringList sysTableList;

// if(ok)
// {
 if(config->currConnection->connectionType()== "ODBC")
     bOpen = db.open();
 else
     bOpen = db.open(_userId, _PWD);
 if((bOpen))
 {
  if(config->currConnection->servertype() != "Sqlite")
  {
#ifndef NO_UDF
   if(!config->currConnection->_sqlite_user_function_loaded)
    config->currConnection->_sqlite_user_function_loaded =sql->loadSqlite3Functions(db);
#endif
  }
  if(config->currConnection->database()  != "")
  {
    // PostgreSQL does not support a user to access different databases from a login. A new connection
    // must be made to switch.
    if(config->currConnection->servertype() != "PostgreSQL")
    {
       QSqlQuery query = QSqlQuery(db);
       //QString cmd = QString("use [%1]").arg(config->currConnection->mySqlDatabase());
       QString cmd = QString("use %1").arg(config->currConnection->database());
       if(!query.exec(cmd))
       {
        SQLERROR(std::move(query));
        db.close();
        bOpen = false;
        return db;
       }
    }
   //if(config->currConnection->servertype() == "Sqlite" )
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
       if(!tableList.contains("StreetDef", Qt::CaseInsensitive))
       {
           if(!eSql->createStreetDefTable(db, config->currConnection->servertype()))
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
       if(!SQL::instance()->doesFunctionExist("distance", config->currConnection->servertype(), db))
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
   sql->checkTables(db);
  }
  else
   sql->checkTables(db);

#ifndef NO_UDF
  if(config->currConnection->servertype() == "Sqlite" && sql->loadSqlite3Functions(db) )
  {
   config->currConnection->_sqlite_user_function_loaded = true;
  }
#endif
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

void Connection::configureDb(QSqlDatabase db, Connection* currConnection, Configuration* config)
{
    QString driver = db.driverName();
    if(currConnection->connectionType() == "Local" )
    {
#ifndef Q_OS_MACOS
        QString dbName = currConnection->sqlite_fileName();
#else

        QString dbName = QDir::currentPath() + "/Resources/databases/" + currConnection->sqlite_fileName();
#endif
        QFileInfo info(dbName);
        if(!info.isWritable())
        {
            QFile::setPermissions(dbName,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
        }
        if(!info.isAbsolute() )
        {
         if(!dbName.startsWith("Resources/databases/"))
          dbName = "Resources/databases/" + dbName;
         if(!dbName.endsWith(".sqlite3"))
          dbName.append(".sqlite3");
         //dbName = QDir(config->path + QDir::separator() + QDir::separator()+ dbName).path();
        }
        db.setDatabaseName(dbName);
    }
    else if(currConnection->connectionType() == "Direct")
    {
        if(currConnection->host() != "")
         db.setHostName(currConnection->host());
        if(currConnection->port() > 0)
         db.setPort(currConnection->port());
        db.setUserName(currConnection->userId());
        db.setPassword(currConnection->pwd());
        //db.setDatabaseName(currConnection->defaultSqlDatabase());
        db.setDatabaseName(currConnection->database());
    }
    else if(currConnection->connectionType() == "ODBC")
    {
        if(currConnection->_connectString.isEmpty())
        {
            db.setDatabaseName(currConnection->dsn());
            db.setUserName(currConnection->userId());
            db.setPassword(currConnection->pwd());
        }
        else
        {
            db.setDatabaseName(currConnection->_connectString);
        }
    }
    else
    {
     throw IllegalArgumentException(tr("invalid driver name: '%1'").arg(driver));
    }
    if(currConnection->connectionType() != "Local"
            && currConnection->servertype() != "PostgreSQL")
    {
      if(db.open() && !currConnection->defaultSqlDatabase().isEmpty())
      {
        QSqlQuery query = QSqlQuery(db);
        if(!query.exec(QString("use %1").arg(currConnection->database())))
        {
            QMessageBox::critical(nullptr, tr("Sql error"), tr("An sql error has occured! \n")
                         + query.lastError().text() + " query: " + query.lastQuery());
        }
      }
    }
}

