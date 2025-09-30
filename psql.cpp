#include "psql.h"
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QProcessEnvironment>

Psql::Psql(QObject *parent)
    : QObject{parent}
{}

/*static*/ bool Psql::runPsql(QString host, QString user, QString password, QString database, QString file)
{
    QStringList parms = {"-h", host, "-U", user, "-d", database, "-f", file, "-W"};
    QStringList connectString = {QString("host=localhost port=5432 dbname=%1 user=%2 password=%3 connect_timeout=10").arg(database,user,password)};

    QProcess *process = new QProcess();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment(); // Get parent's environment
    env.insert("PGPASSWORD", password); // Add a new variable
    process->setProcessEnvironment(env); // Set the modified environment for the child process

    process->setReadChannel(QProcess::StandardOutput);
    // Connect signals to monitor the process
    QObject::connect(process, &QProcess::started, [&]() {
        qDebug() << "Process started!" << process->program() << "state:" << process->state();
    });

    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [&](int exitCode, QProcess::ExitStatus exitStatus) {
                         qDebug() << "Process finished with exit code:" << exitCode;
                         return true; // Quit the application once the process finishes
                     });

    connect(process, &QProcess::readyReadStandardOutput, [=](){
        QString txt = process->readAllStandardOutput();
        qDebug() << "Standard Output:" << txt;
        if(txt == QString("Password for user %1:").arg(user))
            process->write(password.toLatin1());
    });

    connect(process, &QProcess::readyReadStandardError, [=](){
        QString txt = process->readAllStandardError();
        qDebug() << "Standard Error:" << txt;
    });
    connect(process,&QProcess::errorOccurred, [&](QProcess::ProcessError error){
        qCritical() << "returned error: " << error;
    });
    //process->start("psql", connectString);
    process->start("psql");
    if(!process->waitForStarted())
        return false;
    //process->waitForReadyRead();
    // process->write(password.toLatin1());
    // process->closeWriteChannel();
    if(process->waitForFinished())
    {
        qInfo() << "psql finished";
        return true;
    }
    qCritical() << "psql failed " << process->error();
    return false;

}
