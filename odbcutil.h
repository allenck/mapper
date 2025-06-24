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

struct DSN
{
    QString name;
    QString lib;
    QString type;
    QString descr;
    QString driver;
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

signals:
    void odbc_changed();

private:
    explicit ODBCUtil(QObject *parent = nullptr);
    QMap<QString, QList<QPair<QString,QString>>> odbcPairMap;
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

};

#endif // ODBCUTIL_H
