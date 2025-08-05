#ifndef ODBCUTIL_H
#define ODBCUTIL_H

#include "qdir.h"
#include <QObject>
#include <QList>
#include <QPair>
#include <QMap>

struct Driver
{
    QString name;
    QString lib;
    QString type;
    QString descr;
    QString host;
    int port = -1;
    QString user;
    QString pswd;
};

class DSN : public QObject
{
public:
    DSN() {}
    ~DSN() {}
    QString name;
    QString lib;
    QString type;
    QString descr;
    QString driverName;
    QString server;
    int port=-1;
    QString database;
    QString userId;
    QString password;
    bool userDsn =false;
};

class QFile;
class QComboBox;
class QFileSystemWatcher;
class ODBCUtil : public QObject
{
    Q_OBJECT
public:
    static ODBCUtil* instance();
    void getDrivers();
    void getDSNs(QString ini);
    void fillDSNCombo(QComboBox* box, QString type);
    QString connectString(QString connector, QString host, int port, QString user, QString pswd, QString database);
    QString connectString2(QString driver, QString host, int port, QString user, QString pswd, QString database);
    DSN* getDsn(QString dsn);

signals:
    void odbc_changed();

private:
    explicit ODBCUtil(QObject *parent = nullptr);
    QMap<QString, QMap<QString,QString>> odbcPairMap;
    QStringList databases;
    static ODBCUtil* _instance;
    QString currLine;
    QMap<QString, QString> mapDsn;
    QList<QPair<QString, QString> > parseSection();
    QString getSection(QString str);
    QMap<QString,Driver*> drvByName;
    QMap<QString,Driver*> drvByLib;
    QFile* ini = nullptr;
    void updateDriverInfo(Driver* drv, QList<QPair<QString, QString >> pairs);
    QMap<QString, DSN*> dsnByName;
    void initialize();
    QFileSystemWatcher * odbcinstWatcher = nullptr;
    void getWinDSNs();
    void getWinDrivers();
};

#endif // ODBCUTIL_H
