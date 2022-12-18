#include "connection.h"
#include <qsqldatabase.h>
#include <QDebug>
#include <QDir>
#include "configuration.h"
#include "sql.h"

Connection::Connection(QObject *parent) : QObject(parent)
{
 //_DSN = "";
 //_servertype = "Sqlite";
 bOpen = false;

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
// if(ok)
// {
 if((bOpen = db.open()))
 {
  if(config->currConnection->servertype() != "Sqlite")
   return db;
  if(config->currConnection->useDatabase() != "default" && config->currConnection->useDatabase() != "")
  {
   QSqlQuery query = QSqlQuery(db);
   //QString cmd = QString("use [%1]").arg(config->currConnection->useDatabase());
   QString cmd = QString("use %1").arg(config->currConnection->useDatabase());
   if(!query.exec(cmd))
   {
    SQLERROR(query);
    db.close();
    bOpen = false;
    return db;
   }
   if(config->currConnection->servertype() == "Sqlite")
    sql->checkTables(db);
   tableList = db.tables();

   if(!tableList.contains("Parameters",Qt::CaseInsensitive))
   {
    bOpen = false;
    return db;
   }
   else
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
        db->setUserName(currConnection->uid());
        db->setPassword(currConnection->pwd());
        db->setDatabaseName(currConnection->mySqlDatabase());
    }
    else if(currConnection->connectionType() == "ODBC")
    {
        db->setDatabaseName(currConnection->odbc_connectorName());
    }
    else
    {
     throw IllegalArgumentException(tr("invalid driver name: '%1'").arg(driver));
    }
}
