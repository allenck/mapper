#ifndef CONSOLEINTERFACE_H
#define CONSOLEINTERFACE_H

#include <QObject>
#include "console_global.h"

class CONSOLESHARED_EXPORT ConsoleInterface : public QObject
{
    Q_OBJECT
public:
    //explicit ConsoleInterface(QObject *parent = 0);
    ~ConsoleInterface();
    static ConsoleInterface* instance();
    void sendMessage(QString);
signals:
    void message(QString);
public slots:
private:
    ConsoleInterface(QObject* parent = 0);
    static ConsoleInterface* _instance;

};

#endif // CONSOLEINTERFACE_H
