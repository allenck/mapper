#include "odbcutil.h"
#include "QtWidgets/qcombobox.h"
#include "exceptions.h"
#include "qdir.h"
#include "qsettings.h"
#include <QFileSystemWatcher>

ODBCUtil* ODBCUtil::_instance = nullptr;
ODBCUtil::ODBCUtil(QObject *parent)
    : QObject{parent}
{
    _instance = this;
    initialize();
}

void ODBCUtil::initialize()
{
    QString odbcini;
    QString odbcinst;
#  ifdef Q_OS_MACOS
    odbcini = "/Library/ODBC/odbc.ini";
    odbcinst = "/Library/ODBC/odbcinst.ini";
#else
    odbcini = "/etc/odbc.ini";
    odbcinst = "/etc/odbcinst.ini";
#endif
    dsnByName.clear();
#ifndef Q_OS_WIN
    getDrivers();
    getDSNs(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini");
    getDSNs(odbcini);
#else
    getWinDrivers();
    getWinDSNs();
#endif
    QStringList paths = {odbcini,odbcinst,QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini"};
    odbcinstWatcher = new QFileSystemWatcher(paths);
    connect(odbcinstWatcher,&QFileSystemWatcher::fileChanged,this,[=]{
        initialize();
        emit odbc_changed();
    });
}

ODBCUtil* ODBCUtil::instance()
{
    if(!_instance)
        _instance = new ODBCUtil();
    return _instance;
}

void ODBCUtil::getDrivers()
{
    QString odbcinst;
#  ifdef Q_OS_MACOS
    odbcinst = "/Library/ODBC/odbcinst.ini";
#else
    odbcinst = "/etc/odbcinst.ini";
#endif
    drvByName.clear();
    drvByLib.clear();
    ini = new QFile(odbcinst);
    if(ini->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!ini->atEnd())
        {
            if(!currLine.startsWith("["))
                currLine = ini->readLine().trimmed();

            if(currLine.trimmed().startsWith("["))
            {
                QString sectionName = getSection(currLine);
                if(sectionName == "ODBC")
                    continue;
                if(sectionName == "ODBC Drivers")
                {
                    QList<QPair<QString, QString> > pairs = parseSection();
                    for(QPair<QString, QString > p : pairs) {
                        if(p.second.compare("installed",Qt::CaseInsensitive)==0)
                        {
                            Driver* drv = new Driver();
                            drv->name = p.first;
                            if(drv->name.startsWith("MySql",Qt::CaseInsensitive))
                                drv->type = "MySql";
                            else if(drv->name.startsWith("PostgreSQL",Qt::CaseInsensitive))
                                drv->type = "PostgreSQL";
                            else if(drv->name.contains("SQL Server",Qt::CaseInsensitive))
                                drv->type = "MsSql";
                            else
                                drv->type = "Other";
                            //descrToLib.append(drv);
                            drvByName.insert(drv->name, drv);
                        }
                        // else {
                        //     qDebug() << "ignoring " << p.first << " " << p.second;
                        // }
                    }
                    continue;
                }

                // Section assumed to be a driver
                QList<QPair<QString, QString> > pairs = parseSection();
                bool bUpdated = false;
                for(QPair<QString, QString > p : pairs) {
                    if(p.first.compare("driver", Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->lib = p.second;
                            drvByLib.insert(drv->lib, drv);
                            bUpdated = true;
                            break;
                         }
                    }
                    if(p.first.compare("description",Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->descr = p.second;
                            bUpdated = true;
                            break;
                        }
                    }
                    if(p.first.compare("servername",Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->host = p.second;
                            bUpdated = true;
                            break;
                        }
                    }
                    if(p.first.compare("port",Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->port = p.second.toInt();
                            bUpdated = true;
                            break;
                        }
                    }
                    if(p.first.compare("username",Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->user = p.second;
                            bUpdated = true;
                            break;
                        }
                    }
                    if(p.first.compare("password",Qt::CaseInsensitive)==0)
                    {
                        Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->pswd = p.second;
                            bUpdated = true;
                            break;
                        }
                    }
                }
                if(!bUpdated)
                {
                    Driver* drv = new Driver();
                    drv->name = sectionName;
                    if(drv->name.startsWith("MySql",Qt::CaseInsensitive))
                        drv->type = "MySql";
                    else if(drv->name.startsWith("PostgreSQL",Qt::CaseInsensitive))
                    {
                        drv->type = "PostgreSQL";
                        if(drv->port <=0)
                            drv->port = 5432;
                    }
                    else
                        drv->type = "MsSql";
                    updateDriverInfo(drv,pairs);
                    drvByName.insert(drv->name,drv);
                }
            }
        }
    }
}

QString ODBCUtil::getSection(QString str)
{
    return str.mid(1, str.indexOf("]")-1);
}

QList<QPair<QString, QString>> ODBCUtil::parseSection()
{
    QList<QPair<QString, QString>> list;
    while(!ini->atEnd())
    {
        currLine = ini->readLine().trimmed();
        if(currLine.startsWith("["))
            break;
        if(currLine.contains("="))
        {
            QStringList sl =  currLine.split("=");
            if(sl.count()!= 2)
                continue;
            list.append(QPair<QString,QString>(sl.at(0).trimmed(),sl.at(1).trimmed()));
        }
    }
    return list;
}

DSN* ODBCUtil::getDsn(QString dsn)
{
    return dsnByName.value(dsn);
}


void ODBCUtil::updateDriverInfo(Driver* drv, QList<QPair<QString, QString >> pairs)
{
    for(QPair<QString, QString > p : pairs) {
        if(p.first.compare("driver", Qt::CaseInsensitive)==0)
        {
            drv->lib = p.second;
            drvByLib.insert(drv->lib, drv);
        }
        if(p.first.compare("description",Qt::CaseInsensitive)==0)
        {
            drv->descr = p.second;
        }
    }
}

void ODBCUtil::getDSNs(QString odbcini)
{
    ini = new QFile(odbcini);
    if(ini->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!ini->atEnd())
        {
            if(!currLine.startsWith("["))
                currLine = ini->readLine().trimmed();
            if(currLine.startsWith("["))
            {
                QString sectionName = getSection(currLine.trimmed());
                QList<QPair<QString, QString> > pairs = parseSection();
                if(sectionName == "ODBC")
                    continue;
                if(sectionName == "ODBC Data Sources")
                {
                   for(QPair<QString, QString > p : pairs) {
                       DSN* dsn = new DSN();
                       if(ini->fileName() == ".odbc.ini")
                           dsn->userDsn = true;
                       dsn->name = p.first;
                       Driver* drv;
                       if(dsn->lib.startsWith("/"))
                       {
                           drv = drvByLib.value(p.second);
                           dsn->lib =drv->lib;
                           dsn->type = drv->type;
                           dsn->driverName = drv->name;
                       }
                       else {
                           drv = drvByName.value(p.second);
                           dsn->lib = p.second;
                           //dsn->lib = drv->lib;
                           dsn->type = drv->type;
                           dsn->driverName =drv->name;
                       }
                       dsnByName.insert(dsn->name, dsn);
                    }
                }
                else
                {
                    // dsn detail
                    DSN* dsn = dsnByName.value(sectionName,new DSN());
                    if(dsn->name.isEmpty())
                    {
                        // new DSN in dsnByName
                        dsn->name = sectionName;
                        dsnByName.insert(dsn->name, dsn);
                    }
                    if(ini->fileName() == ".odbc.ini")
                        dsn->userDsn = true;
                    Driver* drv;
                    for(QPair<QString, QString > p : pairs) {
                        if(p.first.compare("driver",Qt::CaseInsensitive)==0)
                        {
                            if(p.second.startsWith("/"))
                            {
                                drv = drvByLib.value(p.second);
                                dsn->driverName =drv->name;
                                dsn->type = drv->type;
                            }
                            else {
                                drv = drvByName.value(p.second);
                                dsn->driverName = p.second;
                                dsn->lib = drv->lib;
                                dsn->type = drv->type;
                            }
                        }
                        if(p.first.compare("description",Qt::CaseInsensitive)==0)
                            dsn->descr = p.second;
                    }
                    dsnByName.insert(dsn->name, dsn);
                }
            }
        }
    }
}

void ODBCUtil::fillDSNCombo(QComboBox* box, QString type)
{
    if(box->isHidden() || !box->isEnabled())
        return;
    if(type.isEmpty() || type == "Sqlite")
        return;
    if(dsnByName.values().empty())
        return;
    box->setEnabled(true);
    QStringList validTypes = {"MySql","MsSql","PostgreSQL"};
    if(!validTypes.contains(type))
        throw IllegalArgumentException("ODBCUtil::fillDSNCombo unknown dbType '" + type +"'");

    QString currDsn = box->currentData().toString();
    box->clear();
    foreach(DSN* dsn , dsnByName.values())
    {
        if(dsn->type == type)
        {
            box->addItem(dsn->name+"-"+dsn->descr, dsn->name);
        }
    }
    if(box->count())
        box->setCurrentIndex(0);
    if(!currDsn.isEmpty())
    {
        int ix = box->findData(currDsn);
        if(ix>=0)
            box->setCurrentIndex(ix);
    }
}

// create the connect string
QString ODBCUtil::connectString(QString connector, QString host, int port, QString user, QString pswd, QString database)
{
    DSN* dsn = dsnByName.value(connector);
    if(!dsn)
        return "";
    Driver* drv = drvByName.value(dsn->driverName);
    QString dHost = drv->host;
    if(!host.isEmpty())
        dHost = host;
    int dPort = drv->port;
    if(port > 0)
        dPort = port;
    QString dUser = drv->user;
    if(!user.isEmpty())
        dUser = user;
    QString dPswd = drv->pswd;
    if(!pswd.isEmpty())
        dPswd = pswd;
    QString connstring = QString("Driver=%1;Server=%2;Port=%3;User Id=%4;Password=%5;Database=%6;")
    .arg(drv->lib,host).arg(dPort).arg(dUser,dPswd,database.toLower());
    return connstring;
}

QString ODBCUtil::connectString2(QString driver, QString host, int port, QString user, QString pswd, QString database)
{
    QString connstring = QString("Driver=" + driver +";");
    if(!host.isEmpty())
        connstring.append("Server="+host+";");

    if(port>0)
        connstring.append("Port="+QString::number(port)+";");
    if(!user.isEmpty())
        connstring.append("User Id="+user+";");
    if(!pswd.isEmpty())
        connstring.append("Password="+pswd+";");
    if(!database.isEmpty())
        connstring.append("Database="+database+";");
    return connstring;
}

void ODBCUtil::getWinDSNs()
{
    QSettings winReg1("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
    QSettings winReg2("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
    //odbcMap.clear();
    //QList<QPair<QString,QString> > list;
    QMap<QString,QString> map;
    QStringList iniKeys;
    databases = winReg1.childKeys();
    qDebug() << QString("%1").arg(databases.count());
    foreach (QString key, databases) {
        QString keyVal = winReg1.value(key).toString();
        // if( keyVal.contains("SQL Server"))
        // {
        //list.clear();
        map.clear();
        QString regKey = QString("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\") + key;
        QSettings winReg(regKey,QSettings::NativeFormat);
        DSN* dsn = new DSN();
        dsn->name = key;
        dsn->userDsn = true;
        iniKeys = winReg.childKeys();
        for(QString iniKey : iniKeys)
        {
            QStringList childKeys = winReg.childKeys();
            for(QString iniKey3 : childKeys)
            {
                QPair<QString,QString>  pair(iniKey3, winReg.value(iniKey3).toString());
                //list.append(pair);
                map.insert(iniKey3, winReg.value(iniKey3).toString());
                if(iniKey3 == "Driver")
                {
                    QString lib = winReg.value(iniKey3).toString();
                    Driver* pDriver = drvByLib.value(lib);
                    dsn->lib = lib;
                    dsn->driverName = pDriver->name;
                }
                if(iniKey3.compare("Description",Qt::CaseInsensitive)==0)
                {
                    dsn->descr = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Server", Qt::CaseInsensitive)==0)
                {
                    dsn->server = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Database", Qt::CaseInsensitive)==0)
                {
                    dsn->database = winReg.value(iniKey3).toString();
                }

            }
        }
        //odbcPairMap.insert(key, list);
        odbcPairMap.insert(key,map);
        dsnByName.insert(key, dsn);
    }

    // add also systemDsn
    databases = winReg2.childKeys();
    qDebug() << QString("%1").arg(databases.count());
    foreach (QString key, databases) {
        QString keyVal = winReg2.value(key).toString();
        DSN* dsn = new DSN();
        dsn->name = key;
        dsn->userDsn = false;
        map.clear();
        QString regKey = QString("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI\\") + key;
        QSettings winReg(regKey,QSettings::NativeFormat);

        iniKeys = winReg.childKeys();
        for(QString iniKey : iniKeys)
        {
            QStringList childKeys = winReg.childKeys();
            for(QString iniKey3 : childKeys)
            {
                //QPair<QString,QString>  pair(iniKey3, winReg.value(iniKey3).toString());
                //list.append(pair);
                map.insert(iniKey3, winReg.value(iniKey3).toString());
                if(iniKey3 == "Driver")
                {
                    QString lib = winReg.value(iniKey3).toString();
                    Driver* pDriver = drvByLib.value(lib);
                    dsn->lib = lib;
                    dsn->driverName = pDriver->name;
                    dsn->type = pDriver->type;
                }
                if(iniKey3.compare("Description",Qt::CaseInsensitive)==0)
                {
                    dsn->descr = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Server", Qt::CaseInsensitive)==0)
                {
                    dsn->server = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Servername", Qt::CaseInsensitive)==0)
                {
                    dsn->server = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Port", Qt::CaseInsensitive)==0)
                {
                    dsn->port = winReg.value(iniKey3).toInt();
                }
                if(iniKey3.compare("Database", Qt::CaseInsensitive)==0)
                {
                    dsn->database = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Username", Qt::CaseInsensitive)==0)
                {
                    dsn->userId = winReg.value(iniKey3).toString();
                }
                if(iniKey3.compare("Password", Qt::CaseInsensitive)==0)
                {
                    dsn->password = winReg.value(iniKey3).toString();
                }
            }
        }
        odbcPairMap.insert(key, map);
        dsnByName.insert(key, dsn);

    }
}

void ODBCUtil::getWinDrivers()
{
    QString regKey = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers");
    QString drvKey = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\");

    QSettings winReg(regKey,QSettings::NativeFormat);
    QStringList drvKeys = winReg.childKeys();
    for(QString drvName : drvKeys)
    {
        Driver* driver = new Driver();
        driver->name = drvName;
        if(drvName.startsWith("PostgreSQL",Qt::CaseInsensitive))
            driver->type = "PostgreSQL";
        else if(drvName.startsWith("MySql",Qt::CaseInsensitive))
            driver->type = "MySql";
        else if(drvName.contains("Sql Server",Qt::CaseInsensitive))
            driver->type = "MsSql";
        else
            driver->type = "other";
        QSettings drvReg(drvKey+ drvName, QSettings::NativeFormat);
        QStringList childKeys = drvReg.childKeys();
        for(QString iniKey3 : childKeys)
        {
            if(iniKey3 == "Driver")
            {
                driver->lib = drvReg.value(iniKey3).toString();
            }
        }
        drvByName.insert(drvName, driver);
        if(driver->lib.contains("%WINDIR%"))
            driver->lib = driver->lib.replace("%WINDIR%","C:\\windows");
        drvByLib.insert(driver->lib, driver);
    }
}

