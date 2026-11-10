#ifndef DSN_H
#define DSN_H

#include <QObject>
#include <QMap>

class SecureVault;
class DSN : public QObject
{
    Q_OBJECT
public:
    DSN(QObject *parent=0);
    ~DSN() {}
    DSN(const DSN& other);
    void addPair(QString s1, QString s2);
    QMap<QString,QString>* pairMap()const {return _pairMap;}
    QString upperToAct(QString u) const {return _upperToAct->value(u);}
    QString lib() const {return _lib;}
    QString driverName() const {return _driverName;}
    void setLib(QString lib){_lib=lib;}
    void setDriverName(QString driverName){_driverName = driverName;}
    //bool verifyPairs();
    void removePair(QString key);
    QString driver()const {return _driver;}
    void setDriver(QString driver) {_driver = driver;}
    int maxKeyLen();
    void verifyServerAccessibility(const QString &host, quint16 port);
    QString createSecureVaultKey();
    QString name;
    QString type;
    QString descr;
    QString server;
    int port=-1;
    QString database;
    QString userId;
    QString pwdInDsn;  // password stored in the DSN
    QString sslMode;
    QString location;
    QString setup;
    bool userDsn =false;
    bool bServerAccessible = true;
    bool bDirty = false;
    bool bExclude = false;
    bool bPwdInVault = false;
    QString password();
    void setPassword(QString password);
    QString secureVaultKey();

signals:
    void dsnChanged(DSN* dsn);
    void serverVerificationCompleted(const QString &host, bool isAccessible, const QString &errorMessage = QString());

private:
    QString _driverName;
    QString _lib;
    QMap<QString,QString>* _pairMap =nullptr; // keys are all Upper!
    QMap<QString,QString>* _upperToAct = nullptr;
    QString _driver; // either drivername or lib
    QString _password;
    SecureVault* vault = nullptr;
    QString _key; // secure vault key

    friend class ODBCUtil;
};
#endif // DSN_H
