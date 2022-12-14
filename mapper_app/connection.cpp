#include "connection.h"
#include <qsqldatabase.h>

Connection::Connection(QObject *parent) : QObject(parent)
{
 _DSN = "";
 _servertype = "Sqlite";
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
