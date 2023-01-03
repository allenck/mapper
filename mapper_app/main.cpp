#include <QtGui>
//#include "webviewbridge.h"
#include "mainwindow.h"
#include <QMessageBox>
#include "systemconsole.h"
#include "consoleinterface.h"
#include <QString>
#include "myapplication.h"
//#include "logger.h"
#include <QMutexLocker>
#include <QIODevice>
#include <QFile>
#include "configuration.h"

#if 0
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    if(Logger::instance())
        Logger::instance()->logMessage(type, context, msg);
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
const QString logFilePath = "mapper.log";
bool logToFile = true;
bool firstTime = true;
QBasicMutex mutex;
void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&mutex);
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    ConsoleInterface::instance()->sendMessage(logLevelName + ": "+ msg);
    logToFile = Configuration::instance()->loggingOn();
    if (logToFile) {
        QString txt = QString("%1 %2: %3   (%4.%5)").arg(formattedTime, logLevelName, msg,  context.file).arg(context.line);
        QFile outFile(logFilePath);
        if(firstTime)
        {
            outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
            firstTime = false;
        }
        else
            outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << '\n';
        outFile.close();
    } else {
        fprintf(stdout, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stdout);
    }

//    if (type == QtFatalMsg)
//        abort();
}

int main(int argc, char *argv[])
{

    QByteArray envVar = qgetenv("QTDIR");       //  check if the app is ran in Qt Creator

    if (envVar.isEmpty())
        logToFile = true;
    ConsoleInterface::instance(); // create singleton class.
    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging

 QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

 //QApplication a(argc, argv);
 QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
 MyApplication a(argc, argv);

 MainWindow w(argc, argv);
 w.show();

 return a.exec();
}
