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
    QString setup;
    QString type;
    QString descr;
    QString host;
    int port = -1;
    QString user;
    QString pswd;
};



class DSN;
class QFile;
class QComboBox;
class QFileSystemWatcher;
class ODBCUtil : public QObject
{
    Q_OBJECT
public:
    static ODBCUtil* instance();
#ifndef Q_OS_WINDOWS
    void getDrivers();
    void getDSNs(QString ini);
#endif
    DSN* getDsn(QString dsn);
    QList<DSN*> getDsnList(bool bUserDSN);
    Driver* getDriverByLib(QString filepath);
    Driver* getDriverByName(QString name);
    void fillDSNCombo(QComboBox* box, QString type, QString city = "");
    QString connectString(QString connector, QString host, int port, QString user, QString pswd, QString database);
    QString connectString2(QString driver, QString host, int port, QString user, QString pswd, QString database);
    static QString mimerConnectString(const DSN *dsn, const QString &dbName, const QString &server, QString userid, QString password);
    static bool isIp(QString txt);
    QString getShortPath(const QString &longPath);
    void initialize();
    QList<Driver*> drivers(QString type);
    void addDsn(DSN* dsn);
    void setUpdate(bool b){bUpdating=b;}
    QStringList dsnNames() {return dsnByName.keys();}

signals:
    void odbc_changed();

private:
    explicit ODBCUtil(QObject *parent = nullptr);
    //QMap<QString, QMap<QString,QString>> odbcPairMap;
    bool bUpdating = false;
    bool bUpdateHappened = false;
    QStringList names;
    static ODBCUtil* _instance;
    QString currLine;
    QMap<QString, QString> mapDsn;
#ifndef Q_OS_WINDOWS
    QList<QPair<QString, QString> > parseSection();
    QString getSection(QString str);
    void updateDriverInfo(Driver* drv, QList<QPair<QString, QString >> pairs);
#endif
    QMultiMap<QString,Driver*> drvByType;
    QMap<QString,Driver*> drvByLib;
    QMap<QString,Driver*> drvByName;
    QFile* ini = nullptr;
    QList<DSN*> _dsnList;   // list of all DSNs
    DSN* _dsn = nullptr;
    QMap<QString, DSN*> dsnByName;
    QMultiMap<QString, DSN*> dsnByType;
    QFileSystemWatcher * odbcinstWatcher = nullptr;
    void getWinDSNs();
    void getWinDrivers();
    QStringList validTypes = {"MySql","MsSql","PostgreSQL", "MimerSql"};
    QList<int> excludedDriverIds{1046,25,533}; // Excel, dBase, MS Access
    QList<DSN*> _selectDsnList; // current list of DSNs in combobox.
    void onComboBoxIndexChanged(int index);

    friend class DialogNewConnection;
};


#endif // ODBCUTIL_H
