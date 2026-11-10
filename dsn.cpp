#include "dsn.h"
#include "qtcpsocket.h"
#include <qdebug.h>
#include "securevault.h"

DSN::DSN(QObject *parent)
    : QObject{parent}
{
    _pairMap = new QMap<QString,QString>(); // keys are all Upper!
    _upperToAct = new QMap<QString,QString>();
    connect(this, &DSN::serverVerificationCompleted, this, [=](const QString &host, bool isAccessible, const QString &errorMessage){
        bServerAccessible = isAccessible;
    });
    vault = new SecureVault(this);
}

DSN::DSN(const DSN& other)
{
    name =other.name;
    _lib = other._lib;
    type = other.type;
    descr = other.descr;
    _driverName = other._driverName;
    server = other.server;
    port = other.port;
    database = other.database;
    userId = other.userId;
    _password = other._password;
    sslMode = other.sslMode;
    location = other.location;
    _pairMap = new QMap<QString,QString>(*other._pairMap); // keys are all Upper!
    _upperToAct = new QMap<QString,QString>(*other._upperToAct);
    userDsn = other.userDsn;
    bServerAccessible = other.bServerAccessible;
    bDirty = other.bDirty;
    setup = other.setup;
    _driver = other._driver;
}

void DSN::addPair(QString s1, QString s2)
{
    Q_ASSERT(!s1.isEmpty());
    QString currVal = _pairMap->value(s1.toUpper());
    if(!currVal.isEmpty() && s2.isEmpty())
        qDebug() << s1 << " is being erased";
    _pairMap->insert(s1.toUpper(),s2);
    _upperToAct->insert(s1.toUpper(),s1);
    Q_ASSERT(_pairMap->count() == _upperToAct->count());

    if(s1.toUpper() == "TYPE") type = s2;
    if(s1.toUpper() == "DESCRIPTION") descr = s2;
    if(s1.toUpper() == "SERVER") server = s2;
    if(s1.toUpper() == "DATABASE") database = s2;
    if(s1.toUpper() == "PWD") _password = s2;
    if(s1.toUpper() == "PASSWORD") _password = s2;
    if(s1.toUpper() == "UID") userId = s2;
    if(s1.toUpper() == "USER") userId = s2;
    if(s1.toUpper() == "SERVER") server = s2;
    if(s1.toUpper() == "SSLMODE") sslMode = s2;
    if(s1.toUpper() == "LOCATION") location = s2;
    if(s1.toUpper() == "DRIVER") _driver = s2;
    if(s1.toUpper() == "SETUP") setup = s2;

    //emit dsnChanged(this);
}

void DSN::removePair(QString oldKey)
{
    _pairMap->remove(oldKey.toUpper());
    _upperToAct->remove(oldKey.toUpper());
    Q_ASSERT(_pairMap->count() == _upperToAct->count());
}

#if 0
bool DSN::verifyPairs()
{
    bool rslt = true;
    // verify that certain properties match the pair map.
    if(_pairMap->value("DESCRIPTION") != descr)
    {
        qDebug() << "Description differs in pairMap " << descr << " != " << _pairMap->value("DESCRIPTION");
        rslt = false;
    }
    if(_pairMap->value("DRIVER") != _driver)
    {
        if(!_pairMap->value("DRIVER").isEmpty() && !_driver.isEmpty())
        {
            addPair("DRIVER", _driver);
            return true;
        }
        else
        {
            qDebug() << "Driver differs in pairMap " << _driver << " != " << _pairMap->value("DRIVER");

            rslt = false;
        }
    }
    if(_pairMap->value("TYPE") != type)
    {
        qDebug() << "Type differs in pairMap " << type << " != " << _pairMap->value("TYPE");

        rslt = false;
    }
    if(_pairMap->value("SERVER") != server)
    {
        qDebug() << "Server differs in pairMap " << server << " != " << _pairMap->value("SERVER");
        if(server.isEmpty())
        {
            server = _pairMap->value("SERVER");
            return true;
        }
        rslt = false;
    }
    if(_pairMap->value("UID") != userId)
    {
        qDebug() << "Uid differs in pairMap " << userId << " != " << _pairMap->value("UID");

        rslt = false;
    }
    if(_pairMap->value("DATABASE") != database)
    {
        qDebug() << "Database differs in pairMap " << database << " != " << _pairMap->value("DATABASE");

        rslt = false;
    }
    if(_pairMap->value("LOCATION") != location)
    {
        qDebug() << "Location differs in pairMap " << location << " != " << _pairMap->value("LOCATION");

        rslt = false;
    }
    return rslt;
}
#endif
void DSN::verifyServerAccessibility(const QString &host, quint16 port)
{
    // Allocate the socket on the heap, parented to 'this' for safety
    QTcpSocket *socket = new QTcpSocket(this);

    // Track if a response has already been handled to prevent double-signaling
    // (e.g., if a connection times out right as an error occurs)
    auto hasResponded = std::make_shared<bool>(false);

    // Context: Successful connection
    connect(socket, &QTcpSocket::connected, this, [=]() {
        if (*hasResponded) return;
        *hasResponded = true;

        emit serverVerificationCompleted(host, true);

        socket->disconnect(); // Clear connections before deletion
        socket->deleteLater();
    });

    // Context: Network/Port access failure
    connect(socket, &QAbstractSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error) {
        Q_UNUSED(error);
        if (*hasResponded) return;
        *hasResponded = true;

        emit serverVerificationCompleted(host, false, socket->errorString());

        socket->disconnect();
        socket->deleteLater();
    });

    // Initiate the non-blocking connection request
    // For Mimer SQL, the default server port is 1360 (or adjust to your target port)
    socket->connectToHost(host, port);
}

int DSN::maxKeyLen()
{
    int maxKeyLen =-1;
    foreach(QString key, pairMap()->keys())
        if(key.length()> maxKeyLen) maxKeyLen = key.length();
    return maxKeyLen;
}

/*static*/ QString DSN::createSecureVaultKey()
{
    _key.clear();
    if(userId.isEmpty())
        return _key;
    // key is dsn name | userDsn Y/N | user name
    _key = name + "|";
    _key = _key.append(userDsn?"Y":"N");
    _key = _key.append("|");
    _key = _key.append(userId);
    return _key;
}

QString DSN::password()
{
    if(_password.isEmpty())
    {
        vault->loadPassword(createSecureVaultKey(),[=](const QString &password) {
            if(password.isEmpty())
                qDebug() << "no saved password for " <<_key;
            else
            {
                //ui->txtPWD->setText(password);
                //_dsn->setPassword(password);
                _password = password;
                bPwdInVault = true;
            }
        });
    }
    return _password;
}

void DSN::setPassword(QString password)
{
    if(password.isEmpty())
        return;
    if(_password != password)
    {
        vault->savePassword(createSecureVaultKey(),password);
        vault->loadPassword(createSecureVaultKey(),[=](const QString &password) {
            if(password.isEmpty())
                qDebug() << "no saved password for " <<_key;
            else
            {
                //ui->txtPWD->setText(password);
                //_dsn->setPassword(password);
                _password = password;
                bPwdInVault = true;
            }
        });
    }
}

QString DSN::secureVaultKey()
{
    if(_key.isEmpty())
    {
        createSecureVaultKey();
    }
    return _key;
}
