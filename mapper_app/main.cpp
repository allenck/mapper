#include <QtGui>
#include "webviewbridge.h"
#include "mainwindow.h"
#include <QMessageBox>
#include "../console/systemconsole.h"
#include "../console/consoleinterface.h"
#include <QString>
#include "myapplication.h"

#if QT_VERSION < 0x050000
void myMessageOutput(QtMsgType type, const char *msg)
{
    //QMessageBox::information(NULL, "info", msg);
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
     SystemConsole::getInstance()->On_appendText(QString("Debug: %1\n").arg(msg));
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
     SystemConsole::getInstance()->On_appendText(QString("Warning: %1\n").arg(msg));
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
     SystemConsole::getInstance()->On_appendText(QString("Critical: %1\n").arg(msg));
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
     SystemConsole::getInstance()->On_appendText(QString("Fatal: %1\n").arg(msg));
        abort();
    }
    fflush(stderr);
}
#else
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        ConsoleInterface::instance()->sendMessage("Debug: "+ msg);
        break;
#if QT_VERSION > 0x050400
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        ConsoleInterface::instance()->sendMessage("Info: "+ msg);
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        ConsoleInterface::instance()->sendMessage("Warning: "+ msg);
     break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        ConsoleInterface::instance()->sendMessage("Critical: "+ msg);
     break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        ConsoleInterface::instance()->sendMessage("Fatal: "+ msg);
        abort();
    }
}
#endif
int main(int argc, char *argv[])
{
 //QApplication::setStyle("gtk+");
 //if(argv[1] == "debug")
#ifndef QT_DEBUG
# if QT_VERSION < 0x050000
 qInstallMsgHandler(myMessageOutput);
# else
  qInstallMessageHandler(myMessageOutput);
# endif
#endif
 QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
 //QApplication a(argc, argv);
 QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
 MyApplication a(argc, argv);

 mainWindow w(argc, argv);
 w.show();

 return a.exec();
}
