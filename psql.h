#ifndef PSQL_H
#define PSQL_H

#include <QObject>

class Psql : public QObject
{
    Q_OBJECT
protected:
    explicit Psql(QObject *parent = nullptr);
public:
    static bool runPsql(QString host, QString user, QString password, QString database, QString file);

signals:
};

#endif // PSQL_H
