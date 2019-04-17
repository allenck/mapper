#include "connection.h"
#include <qsqldatabase.h>

Connection::Connection(QObject *parent) : QObject(parent)
{
 _DSN = "";
 _servertype = "MsSql";
 bOpen = false;

}

QSqlDatabase Connection::getDb() { return db;}
