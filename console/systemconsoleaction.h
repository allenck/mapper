#ifndef SYSTEMCONSOLEACTION_H
#define SYSTEMCONSOLEACTION_H

#include <QAction>
#include "console_global.h"

//class WindowInterface;
class CONSOLESHARED_EXPORT SystemConsoleAction : public QAction
{
 Q_OBJECT
public:
 explicit SystemConsoleAction(QObject *parent = 0);
 /*public*/  SystemConsoleAction(QString s, QObject* wi);
 /*public*/  SystemConsoleAction(QString s, QIcon i, QObject* wi);
 /*public*/ void close();

signals:

public slots:
 /*public*/  void actionPerformed(ActionEvent* e = 0);

private:
 void common();
};

#endif // SYSTEMCONSOLEACTION_H
