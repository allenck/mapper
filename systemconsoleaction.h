#ifndef SYSTEMCONSOLEACTION_H
#define SYSTEMCONSOLEACTION_H

#include <QAction>
#include "console_global.h"

//class WindowInterface;
class SystemConsoleAction : public QAction
{
 Q_OBJECT
public:
 explicit SystemConsoleAction(QObject *parent = 0);
 /*public*/ void close();

signals:

public slots:
 /*public*/  void actionPerformed();

private:
};

#endif // SYSTEMCONSOLEACTION_H
