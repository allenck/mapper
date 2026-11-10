#include "connection.h"
#include <qsqldatabase.h>
#include <QDebug>
#include <QDir>
#include "configuration.h"
#include "dsn.h"
#include "sql.h"
#include <QSqlQuery>
#include "exportsql.h"
#include "odbcutil.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include "securevault.h"
#include "odbcutil.h"

Connection::Connection(QObject *parent) : QObject(parent)
{
 //_DSN = "";
 //_servertype = "Sqlite";
 bOpen = false;
 _uuid = QUuid::createUuid();
 config = Configuration::instance();
 vault = new SecureVault(this);
 odbcUtil = ODBCUtil::instance();

}

Connection::Connection(QUuid uuid, QObject* parent) : QObject(parent)
{
  _uuid = uuid;
    config = Configuration::instance();
  vault = new SecureVault(this);
  odbcUtil = ODBCUtil::instance();
}

Connection::Connection(const Connection& o){
 this->_id = o._id;
 this->_driver = o._driver;
 this->_description =o._description;
 this->_DSName =o._DSName;
 this->_userId = o._userId;
 this->_PWD = o._PWD;
 this->_database = o._database;
 this->_hostName = o._hostName;
 this->_port = o._port;
 //this->_useDatabase = o._useDatabase;
 //this->db = o.db;
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
 this->odbcUtil = o.odbcUtil;
}


//QSqlDatabase Connection::getDb() { return db;}

QString Connection::connectionName(){return _connectionName;}

void Connection::setConnectionName(QString name) {
    Q_ASSERT(!name.isEmpty());
    _connectionName =name;
}

QSqlDatabase Connection::configure(const QString cName)
{
    qDebug() << "Connection: CWD = " << QDir::currentPath();
    Q_ASSERT(!cName.isEmpty());
    Q_ASSERT(!config->currConnection->driver().isEmpty());
    _connectionName = cName;
    sql = SQL::instance();
    sql->setConnectionName(cName);
    QSqlDatabase db = QSqlDatabase::addDatabase(config->currConnection->driver(),cName);
 // QSqlDatabase db = QSqlDatabase::database(cName);
 // if (QSqlDatabase::contains(cName) && db.driverName() =="QSQLITE") {
 //     db = QSqlDatabase::database(cName);
 // } else {
 //     db = QSqlDatabase::addDatabase("QSQLITE", cName);
 // }
    configureDb(db, this);
    // check for presence of Parameters table.
    QStringList tableList;
    QStringList sysTableList;

// if(ok)
// {
    if(config->currConnection->connectionType()== "ODBC" || config->currConnection->connectionType()== "Sqlite")
        bOpen = db.open();
    else
        bOpen = db.open(_userId, _PWD);
    if((bOpen))
    {
        setOpen(true);
        tableList = db.tables();
        sysTableList = db.tables(QSql::SystemTables);
        QString cn = db.connectionName();
        Q_ASSERT(!cn.isEmpty());
        _connectionName = cn;
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
    if(tableList.isEmpty())
       throw Exception(tr("No tables exist in this database"));
   sysTableList = db.tables(QSql::SystemTables);
   ExportSql* eSql = new ExportSql(false);
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
       if(!SQL::instance()->doesFunctionExist("distance", config->currConnection->servertype()))
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
} //end configure

void Connection::configureDb(QSqlDatabase db, Connection* currConnection)
{
    QString driver = db.driverName();
    if(currConnection->connectionType() == "Sqlite" )
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
    if(currConnection->connectionType() != "Sqlite"
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


bool Connection::connectToCityMimer(const QString &dbName, const QString &server, const QString &connectionName,
                                    QString userid, QString password)
{
    // Ensure we isolate the connection tracking string
    //const QString connectionName = "MimerCityConnection";

    if (QSqlDatabase::contains(connectionName)) {
        {
            QSqlDatabase oldDb = QSqlDatabase::database(connectionName);
            if (oldDb.isOpen()) oldDb.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }

    // Initialize the primary QODBC driver base
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);

    // === ADD THIS OPTION FOR MIMER SQL SYSTEM CATALOGS ===
    // Forces the ODBC layer to build a client-side static scrollable cache
    //db.setConnectOptions("SQL_ATTR_CURSOR_TYPE=SQL_CURSOR_STATIC");
    //db.setConnectOptions("SQL_ATTR_ANSI_APP=FALSE");
    // Build a DSN-less connection string using the verified parameters from dbVis
    // QString connectionString = QString(
    //                                "Driver={MIMER};"
    //                                "Protocol=tcp;"
    //                                "Node=%2;"
    //                                "Service=1360;"  // Verified from your dbVis Connection URL string
    //                                "Database=%1;"
    //                                "Uid=%3;"
    //                                "Pwd=%4;"
    //                                ).arg(dbName.toLower(), server,userid,password);
    QString connectionString = ODBCUtil::mimerConnectString(dbName,server,userid,password);

    db.setDatabaseName(connectionString);

    if (!db.open()) {
        qDebug() << "Mimer Error targeting" << dbName << ":" << db.lastError().driverText();
        return false;
    }

    qDebug() << "Successfully attached to Mimer SQL engine for:" << dbName;
    return true;
}

bool Connection::connectToPostgreSQL(const DSN* dsn, const QString &server, const QString &connectionName,
                                    const QString &useDatabase, const QString &userid, const QString &password)
{

    if (QSqlDatabase::contains(connectionName)) {
        {
            QSqlDatabase oldDb = QSqlDatabase::database(connectionName);
            if (oldDb.isOpen()) oldDb.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }

    // Initialize the primary QODBC driver base
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);

    QString connectionString = ODBCUtil::instance()->connectString2("{" + dsn->driverName() + "}",server, dsn->port,
                                          userid, password,useDatabase );
    qDebug() << "PostgreSQL connectString: " << connectionString;
    db.setDatabaseName(connectionString);

    if (!db.open()) {
        qDebug() << "PostgreSQL Error targeting" << useDatabase << ":" << db.lastError().driverText();
        return false;
    }

    qDebug() << "Successfully attached to PostgreSQL engine for:" << useDatabase;
    return true;
}

/*static*/ QMap<QString,QString> Connection::decodeConnectionString(QString cs)
{
    QMap<QString,QString> map = QMap<QString,QString>();
    QStringList sl = cs.split(";");
    foreach (QString s, sl)
    {
        int ix = s.indexOf("=");
        map.insert(s.mid(0,ix).trimmed(), s.mid(ix+1).trimmed());
    }
    return map;
}

QStringList Connection::tables(){
    QSqlDatabase db = QSqlDatabase::database(_connectionName);

    return db.tables();
}

void Connection::close(QSqlDatabase db)
{
    db.close();
    bOpen = false;
}

QString Connection::createSecureVaultKey()
{
    City* city = config->currCity;
    _key.clear();
    if(!_connectString.isEmpty())
    {
        QMap<QString,QString> map = decodeConnectionString(_connectString);
        // Driver={PostgreSQL ODBC Driver(UNICODE)};Server=ubuntu-2;Port=5432;User Id=allen;Password=iic723@knobacres;Database=stlouis;sslmode=prefer;
        // Driver={MIMER};Protocol=tcp;Node=ubuntu-2;Service=1360;Database=stlouis_db;Uid=mapperuser;Pwd=iic723@knobacres;
        QString driver = map.value("Driver");
        if(driver == "{MIMER}")
        {
            _key.append(map.value("Uid") + "|");
            _key.append(city->getLabel() + "|");
            _key.append("MimerSql") + "|";
            _key.append(map.value("Node"));
            //*password = map.value("Pwd");
        }
        else if(driver.contains("PostgreSQL") )
        {
            _key.append(map.value("User Id") + "|");
            _key.append(city->getLabel() + "|");
            _key.append("PostgreSQL") + "|";
            _key.append(map.value("Server"));
            //*password = map.value("Password");
        }
        else
        {
            qCritical() << "unknown connection string driver: " << driver;
            return _key;
        }
    }
    if(userId().isEmpty())
        return _key;
    if(servertype() == "Sqlite")
        return _key;
    _key = userId() + "|";
    _key.append(city->getLabel() +"|");
    _key.append(servertype() + "|");
    if(!host().isEmpty())
        _key.append(host());
    else
    {
        if(!_DSName.isEmpty())
        {

            DSN* dsn = odbcUtil->getDsn(_DSName);
            if(dsn && !dsn->server.isEmpty())
            {
                _key.append(dsn->server);
                return _key;
            }
        }
        // TODO: get values from a connection string
    }

    return _key;
}

QString Connection::secureVaultKey()
{
    if(_key.isEmpty())
        createSecureVaultKey();
    return _key;
}

QString Connection::pwd() {
    if(bPwdInVault)
        return _PWD;

    vault->loadPassword(createSecureVaultKey(),[=](const QString &password){
        if(password.isEmpty())
            qDebug() << "no saved password for " <<_key;
        else
        {
            //ui->txtPWD->setText(password);
            //_dsn->setPassword(password);
            _PWD = password;
            bPwdInVault = true;
        }
    });
    return _PWD;
}

void Connection::setPWD(QString p) {
    vault->savePassword(createSecureVaultKey(),p);
    _PWD = p;
}
