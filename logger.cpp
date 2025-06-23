#include "logger.h"
#include <QPointer>
#include "systemconsole2.h"
#include <QDir>
#include <QDebug>

Logger::Logger(QObject *parent)
    : QObject{parent}
{
    //QObject::connect(ConsoleInterface::instance(), SIGNAL(message(QString)), this,  SLOT(messageOutput(QString)));
    connect(SystemConsole2::instance(), SIGNAL(outMessage(QString)), this, SLOT(messageOutput(QString)));

    if (!QDir("./data/").exists())
    {
        if (!QDir().mkdir("./data/"))
        {
   //         DEBUG_FUNCTION("Cant open data folder! Exiting...");
//            return 0;
        }
    }

    QFile file("./data/log.txt");

    log_file = &file;

    if (!log_file->open(QFile::WriteOnly | QFile::Text | QFile::Append))
    {
        qInstallMessageHandler(nullptr);
        qDebug() << "Couldn't log to file!";
    }
}

/*static*/ QPointer<QFile> Logger::log_file = nullptr;

void Logger::messageOutput(/*QtMsgType type, const QMessageLogContext &context,*/ const QString msg)
{
//    Q_UNUSED(type)
//    Q_UNUSED(context)

    printf("%s\n", qPrintable(msg));
    fflush(stdout);

    if (log_file)
    {
        log_file->write(msg.toLatin1());
        log_file->write("\n");
        log_file->flush();
    }
}
