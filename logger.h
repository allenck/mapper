#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
public slots:
    void messageOutput(QString msg);
signals:

private:
    static QPointer<QFile> log_file;// = nullptr;
};

#endif // LOGGER_H
