#ifndef DSN_H
#define DSN_H

#include <QObject>
#include <QMap>
class DSN : public QObject
{
public:
    DSN(QObject *parent=0);
    ~DSN() {}
    DSN(const DSN& other);
    void addPair(QString s1, QString s2);
    QMap<QString,QString>* pairMap() {return _pairMap;}
    QString upperToAct(QString u) {return _upperToAct->value(u);}
    QString lib() {return _lib;}
    QString driverName() {return _driverName;}
    void setLib(QString lib){_lib=lib;}
    void setDriverName(QString driverName){_driverName = driverName;}
    bool verifyPairs();
    void removePair(QString key);
    QString driver() {return _driver;}
    void setDriver(QString driver) {_driver = driver;}
    QString name;
    QString type;
    QString descr;
    QString server;
    int port=-1;
    QString database;
    QString userId;
    QString password;
    QString sslMode;
    QString location;
    bool userDsn =false;
    bool bServerAccessible = true;
    bool bDirty = false;
    bool bExclude = false;
    QString setup;

private:
    QString _driverName;
    QString _lib;
    QMap<QString,QString>* _pairMap =nullptr; // keys are all Upper!
    QMap<QString,QString>* _upperToAct = nullptr;
    QString _driver; // either drivername or lib

    friend class ODBCUtil;
};
#endif // DSN_H
