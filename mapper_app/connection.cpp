#include "connection.h"
#include <qsqldatabase.h>

Connection::Connection(QObject *parent) : QObject(parent)
{
 _DSN = "";
 _servertype = "Sqlite";
 bOpen = false;

}

QSqlDatabase Connection::getDb() { return db;}
