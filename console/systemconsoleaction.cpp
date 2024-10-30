#include "systemconsoleaction.h"
#include "systemconsole.h"
//#include "windowinterface.h"
#include <QMainWindow>


/*public*/  SystemConsoleAction::SystemConsoleAction(QObject *parent) :
  QAction(tr("System console"), parent){
    //super(tr("TitleConsole"));
    connect(this, SIGNAL(triggered()), this, SLOT(actionPerformed()));
}

//@Override
/*public*/  void SystemConsoleAction::actionPerformed() {
    // Show system console
    SystemConsole::getInstance()->getFrame(parentWidget())->setVisible(true);
}

//// never invoked, because we overrode actionPerformed above
////@Override
///*public*/  JmriPanel makePanel() {
//    throw new IllegalArgumentException("Should not be invoked"); // NOI18N
//}
/*public*/ void SystemConsoleAction::close()
{
 SystemConsole::getInstance()->closeConsole();
}
