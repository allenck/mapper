#include "consoleinterface.h"
#include <QDebug>
ConsoleInterface::ConsoleInterface(QObject *parent) :
    QObject(parent)
{
}
ConsoleInterface::~ConsoleInterface() {}
/*static*/ ConsoleInterface* ConsoleInterface::_instance = NULL;

/*static*/ ConsoleInterface* ConsoleInterface::instance()
{
 if(_instance == NULL) _instance = new ConsoleInterface();
 return _instance;
}
void ConsoleInterface::sendMessage(QString s)
{
 if(s.contains("QSqlQuery::value: not positioned on a valid record"))
  qDebug() << "debug halt";
 emit message(s);
}
