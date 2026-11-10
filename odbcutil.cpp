#include "odbcutil.h"
#include "QtWidgets/qcombobox.h"
#include "exceptions.h"
#include "qdir.h"
#include "qsettings.h"
#include "qstandarditemmodel.h"
#include <QFileSystemWatcher>
#include <QHostInfo>
#include <QMessageBox>
#include "dsn.h"
#ifdef Q_OS_WIN
//#include <qt_windows.h> // Includes Windows.h safely in Qt environments
#include "registrywatcher.h"
#endif

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
    dsnByType.clear();
    _dsnList.clear();
#ifndef Q_OS_WIN
    getDrivers();
    getDSNs(QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini");
    getDSNs(odbcini);
    foreach(DSN*dsn, dsnByName.values())
    {
        if(validTypes.contains(dsn->type))
            dsnByType.insert(dsn->type, dsn);
    };
    QStringList paths = {odbcini,odbcinst,QDir::home().absolutePath() + QDir::separator()+ ".odbc.ini"};
    odbcinstWatcher = new QFileSystemWatcher(paths);
    connect(odbcinstWatcher,&QFileSystemWatcher::fileChanged,this,[=]{
        initialize();
        emit odbc_changed();
    });
#else
    getWinDrivers();
    getWinDSNs();

    // For HKEY_CURRENT_USER (User DSNs)
    RegistryWatcher *w1 = new RegistryWatcher(HKEY_CURRENT_USER, "Software\\ODBC\\ODBC.INI\\ODBC Data Sources", this);
    connect(w1, &RegistryWatcher::registryKeyChanged, this,&ODBCUtil::getWinDSNs);

    // For HKEY_LOCAL_MACHINE (System DSNs)
    RegistryWatcher *w2 = new RegistryWatcher(HKEY_LOCAL_MACHINE, "Software\\ODBC\\ODBC.INI\\ODBC Data Sources", this);
    connect(w2, &RegistryWatcher::registryKeyChanged, this, &ODBCUtil::getWinDSNs);
    RegistryWatcher *w3 = new RegistryWatcher(HKEY_LOCAL_MACHINE,"SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers", this);
    connect(w3,&RegistryWatcher::registryKeyChanged, this, &ODBCUtil::getWinDrivers);
    RegistryWatcher *w4 = new RegistryWatcher(HKEY_LOCAL_MACHINE,"SOFTWARE\\ODBC\\ODBCINST.INI\\", this);
    connect(w3,&RegistryWatcher::registryKeyChanged, this, &ODBCUtil::getWinDrivers);
#endif
}

ODBCUtil* ODBCUtil::instance()
{
    if(!_instance)
        _instance = new ODBCUtil();
    return _instance;
}

#ifndef Q_OS_WINDOWS
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
    drvByType.clear();
    ini = new QFile(odbcinst);
    if(ini->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!ini->atEnd())
        {
            if(!currLine.startsWith("["))
                currLine = ini->readLine().trimmed();
            QString sectionName;
            if(currLine.trimmed().startsWith("["))
            {
                sectionName = getSection(currLine);
                if(sectionName == "ODBC")
                    continue;
                if(sectionName == "ODBC Drivers")
                {
                    QList<QPair<QString, QString> > pairMap = parseSection();
                    for(QPair<QString, QString > p : pairMap) {
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
                            else if(drv->name.contains("MimerSql", Qt::CaseInsensitive))
                                drv->type = "MimerSql";
                            else
                                drv->type = "Other";
                            //descrToLib.append(drv);
                            drvByName.insert(drv->name, drv);
                            drvByType.insert(drv->type, drv);
                        }
                        // else {
                        //     qDebug() << "ignoring " << p.first << " " << p.second;
                        // }
                    }
                    continue;
                }

                // Section assumed to be a driver
                QList<QPair<QString, QString> > pairMap = parseSection();
                bool bUpdated = false;
                Driver* drv = drvByName.value(sectionName);
                for(QPair<QString, QString > p : pairMap) {
                    if(p.first.compare("driver", Qt::CaseInsensitive)==0)
                    {
                        if(drv)
                        {
                            if(p.second.startsWith('/'))
                            {
                                QFileInfo info(p.second);
                                if(!info.exists())
                                {
                                    qWarning() << "Warning! ODBC driver " << sectionName << " library " << p.second << " not found!";
                                }
                            }
                            else
                            {
                                QFileInfo info("/usr/lib/x86_64-linux-gnu/odbc/" + p.second);
                                if(!info.exists())
                                {
                                    qWarning() << "Warning! ODBC driver " << sectionName << " library " << p.second << " not found!";
                                }
                            }
                            drv->lib = p.second;
                            drv->name = sectionName;
                            drvByLib.insert(drv->lib, drv);
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("description",Qt::CaseInsensitive)==0)
                    {
                        //Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->descr = p.second;
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("servername",Qt::CaseInsensitive)==0)
                    {
                        //Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->host = p.second;
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("port",Qt::CaseInsensitive)==0)
                    {
                        //Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->port = p.second.toInt();
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("username",Qt::CaseInsensitive)==0)
                    {
                        //Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->user = p.second;
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("password",Qt::CaseInsensitive)==0)
                    {
                        //Driver* drv = drvByName.value(sectionName);
                        if(drv)
                        {
                            drv->pswd = p.second;
                            bUpdated = true;
                            continue;
                        }
                    }
                    if(p.first.compare("Type",Qt::CaseInsensitive)==0)
                    {
                        if(drv)
                        {
                            drv->type = p.second;
                            bUpdated = true;
                            continue;
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
                    updateDriverInfo(drv,pairMap);
                    drvByName.insert(drv->name,drv);
                    drvByType.insert(drv->type, drv);
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



void ODBCUtil::updateDriverInfo(Driver* drv, QList<QPair<QString, QString >> pairMap)
{
    for(QPair<QString, QString > p : pairMap) {
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
                QList<QPair<QString, QString> > pairMap = parseSection();
                if(sectionName == "ODBC" || sectionName == "ODBC Data Sources")
                    continue;
                DSN* dsn = nullptr;
                // if(sectionName == "ODBC Data Sources")
                // {
                //     for(QPair<QString, QString > p : pairMap) {
                //         dsn = new DSN();
                //         dsn->pairMap = new QList<QPair<QString,QString>>();
                //         dsnList.append(dsn);
                //         if(ini->fileName() == ".odbc.ini")
                //             dsn->userDsn = true;
                //         dsn->name = p.first;
                //         Driver* drv;
                //         if(dsn->lib.startsWith("/"))
                //         {
                //             drv = drvByLib.value(p.second);
                //             dsn->lib =drv->lib;
                //             dsn->type = drv->type;
                //             dsn->driverName = drv->name;
                //         }
                //         else {
                //             drv = drvByName.value(p.second);
                //             dsn->lib = p.second;
                //             //dsn->lib = drv->lib;
                //             dsn->type = drv->type;
                //             dsn->driverName =drv->name;
                //         }
                //         dsnByName.insert(dsn->name, dsn);
                //     }
                // }
                // else
                if(sectionName.startsWith("Database"))
                {
                    dsn->database = sectionName.mid(8).trimmed();
                    dsn->addPair("database", dsn->database);
                }
                else
                {
                    // dsn detail (inside the else block of sectionName parsing)
                    DSN* dsn = dsnByName.value(sectionName, new DSN());
                    dsn = new DSN();
                    dsn->_pairMap = new QMap<QString,QString>();
                    _dsnList.append(dsn);
                    if(dsn->name.isEmpty())
                    {
                        dsn->name = sectionName;
                        //dsnByName.insert(dsn->name, dsn);
                    }
                    if(ini->fileName() == ".odbc.ini")
                        dsn->userDsn = true;

                    Driver* drv = nullptr;
                    for(QPair<QString, QString > p : pairMap) {
                        // dsn->_pairMap->insert(p.first.toUpper(),p.second);
                        // dsn->_upperToAct->insert(p.first.toUpper(), p.first);
                        dsn->addPair(p.first.toUpper(),p.second);
                        if(p.first.compare("driver", Qt::CaseInsensitive) == 0)
                        {
                            if(p.second.startsWith("/"))
                            {
                                drv = drvByLib.value(p.second);
                                if(drv) {
                                    dsn->setDriverName(drv->name);
                                    dsn->setLib(p.second);
                                    // Only default the type if a custom type override hasn't explicitly been provided yet
                                    if(dsn->type.isEmpty())
                                    {
                                        dsn->type = drv->type;
                                        dsn->addPair("Type", drv->type);
                                    }
                                    dsn->setDriver(p.second);
                                }
                            }
                            else {
                                drv = drvByName.value(p.second);
                                dsn->addPair("Driver",p.second);
                                if(drv)
                                {
                                    dsn->setDriverName(p.second);
                                    dsn->setLib(drv->lib);
                                    if(dsn->type.isEmpty())
                                    {
                                        dsn->type = drv->type;
                                        dsn->addPair("Type",drv->type);
                                    }
                                    dsn->setDriver(p.second);

                                }
                            }
                        }
                        if(p.first.compare("description", Qt::CaseInsensitive) == 0)
                            dsn->descr = p.second;
                        if(p.first.compare("server", Qt::CaseInsensitive) == 0 || p.first.compare("servername", Qt::CaseInsensitive) == 0)
                            dsn->server = p.second;
                        if(p.first.compare("database", Qt::CaseInsensitive) == 0)
                            dsn->database = p.second;
                        if(p.first.compare("port", Qt::CaseInsensitive) == 0)
                            dsn->port = p.second.toInt();
                        if(p.first.compare("username", Qt::CaseInsensitive) == 0 || p.first.compare("uid", Qt::CaseInsensitive) == 0)
                            dsn->userId = p.second;
                        if(p.first.compare("password", Qt::CaseInsensitive) == 0 || p.first.compare("pwd", Qt::CaseInsensitive) == 0)
                        {
                            dsn->password = p.second;
                            dsn->pwdInDsn = p.second;
                        }
                        if(p.first.compare("location", Qt::CaseInsensitive) == 0)
                            dsn->location = p.second;
                        if(p.first.compare("type", Qt::CaseInsensitive) == 0)
                            dsn->type = p.second;

                        // === THE MISSING LINK FOR LINUX CUSTOM KEYS ===
                        if(p.first.compare("type", Qt::CaseInsensitive) == 0)
                        {
                            // Intercept your custom Type=MimerSQL key
                            if(p.second.compare("MimerSQL", Qt::CaseInsensitive) == 0) {
                                dsn->type = "MimerSql"; // Syncs with your validTypes restriction array
                                dsn->addPair("Type", "MimerSql");
                            } else {
                                dsn->type = p.second;
                                dsn->addPair("Type", p.second);
                            }
                        }
                    }

                    // Re-index into the specialized MultiMap so fillDSNCombo can see it
                    // if(validTypes.contains(dsn->type)) {
                    //     dsnByType.insert(dsn->type, dsn);
                    // }
                    dsnByName.insert(dsn->name, dsn);
                    if(!dsn->type.isEmpty())
                        dsnByType.insert(dsn->type, dsn);
                }
            }
        }

    }
}
#endif
DSN* ODBCUtil::getDsn(QString dsn)
{
    return dsnByName.value(dsn);
}

Driver* ODBCUtil::getDriverByLib(QString filepath)
{
    return drvByLib.value(filepath);
}

Driver* ODBCUtil::getDriverByName(QString name)
{
    return drvByName.value(name);
}

void ODBCUtil::fillDSNCombo(QComboBox* box, QString type, QString city)
{
    // if(box->isHidden() || !box->isEnabled())
    //     return;
    if( type == "Sqlite")
        return;
    if(dsnByName.values().count()==0)
        return;
    box->setEnabled(true);
    if(!type.isEmpty() && !validTypes.contains(type))
        throw IllegalArgumentException("ODBCUtil::fillDSNCombo unknown dbType '" + type +"'");

    QString currDsn = box->currentData().toString();
    box->clear();
    int ix =0;
    if(type.isEmpty())
        _selectDsnList = this->_dsnList;
    else
        _selectDsnList = dsnByType.values(type);

    foreach(DSN* dsn, _selectDsnList)
    {
        if(dsn->pairMap()->contains("DRIVERID") && excludedDriverIds.contains(dsn->pairMap()->value("DRIVERID").toInt()))
        {
            dsn->bExclude = true;
            continue;
        }
        if(dsn->pairMap() == nullptr)
            continue;
        if(dsn->type == type || type.isEmpty())
        {
            if(dsn->location.isEmpty())
                box->addItem(dsn->name+" -- "+dsn->descr, dsn->name);
            else
            {
                if(!city.isEmpty() && dsn->location != city)
                    continue;
                box->addItem(dsn->name+" -- "+dsn->descr, dsn->name);
            }
            const QModelIndex idx = box->model()->index(ix, 0);

            if(dsn->userDsn)
                box->model()->setData(idx, QColor("#ccffe5"), Qt::BackgroundRole);

            if(!dsn->bServerAccessible)
            {
                auto* model = qobject_cast<QStandardItemModel*>(box->model());
                if (model) {
                    QStandardItem* item = model->item(ix);
                    if (item) {
                        item->setEnabled(false);
                    }
                }
            }
        }
        ix ++;
    }
    if(box->count())
        box->setCurrentIndex(0);
    // else
    //     box->lineEdit()->setPlaceholderText(tr("no %1 DSNs defined!").arg(type));
    if(!currDsn.isEmpty())
    {
        int ix = box->findData(currDsn);
        if(ix>=0)
            box->setCurrentIndex(ix);
    }
    // Connect the signal (Qt 5 / Qt 6 syntax)
    connect(box, &QComboBox::currentIndexChanged, this, &ODBCUtil::onComboBoxIndexChanged);
}

void ODBCUtil::onComboBoxIndexChanged(int index) {
    QComboBox* box = qobject_cast<QComboBox*>(sender());
    if (!box) return;

    // Fetch the background role color from the model index
    QModelIndex idx = box->model()->index(index, 0);
    QVariant bgColorVariant = box->model()->data(idx, Qt::BackgroundRole);

    if (bgColorVariant.isValid()) {
        QColor color = bgColorVariant.value<QColor>();
        // Apply the custom color directly to the box's background
        box->setStyleSheet(QString(
                               "QComboBox { background-color: %1; }"
                               "QComboBox::drop-down { background-color: transparent; }" // Blends arrow container
                               ).arg(color.name()));
    } else {
        // Reset to default styling if no BackgroundRole is set
        box->setStyleSheet("");
    }
}
// create the connect string
QString ODBCUtil::connectString(QString connector, QString host, int port, QString user, QString pswd, QString database)
{
    DSN* dsn = dsnByName.value(connector);
    if(!dsn)
        return "";
    Driver* drv = drvByType.value(dsn->_driverName);
    if(!drv)
        return "";

    // Resolve parameter hierarchy (Runtime value -> DSN file value -> Global driver value)
    QString dHost = host.isEmpty() ? (!dsn->server.isEmpty() ? dsn->server : drv->host) : host;
    if(dHost.isEmpty()) dHost = "localhost";

    int dPort = (port > 0) ? port : ((dsn->port > 0) ? dsn->port : ((drv->port > 0) ? drv->port : 1360));
    QString dUser = user.isEmpty() ? (!dsn->userId.isEmpty() ? dsn->userId : drv->user) : user;
    QString dPswd = pswd.isEmpty() ? (!dsn->password().isEmpty() ? dsn->password() : drv->pswd) : pswd;
    QString dDatabase = database.isEmpty() ? (!dsn->database.isEmpty() ? dsn->database : connector) : database;

    QString connstring;

    // Check if we are dealing with a Mimer SQL DSN connection format
    if(dsn->type == "MimerSql") {
        // Mimer SQL explicitly demands Uid and Pwd tokens
        connstring = QString("Driver=%1;Server=%2;Port=%3;Uid=%4;Pwd=%5;Database=%6;")
                         .arg(drv->lib, dHost).arg(dPort).arg(dUser, dPswd, dDatabase.toLower());
    } else {
        // Fallback layout for standard SQL Server / MySQL drivers
        connstring = QString("Driver=%1;Server=%2;Port=%3;User Id=%4;Password=%5;Database=%6;")
                         .arg(drv->lib, dHost).arg(dPort).arg(dUser, dPswd, dDatabase.toLower());
    }

    return connstring;
}


// used bp PostgreSQL
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

/*static*/ QString ODBCUtil::mimerConnectString(const QString &dbName, const QString &server, QString userid, QString password)
{
    // === ADD THIS OPTION FOR MIMER SQL SYSTEM CATALOGS ===
    // Forces the ODBC layer to build a client-side static scrollable cache
    //db.setConnectOptions("SQL_ATTR_CURSOR_TYPE=SQL_CURSOR_STATIC");
    //db.setConnectOptions("SQL_ATTR_ANSI_APP=FALSE");
    // Build a DSN-less connection string using the verified parameters from dbVis
    QList<Driver*> list = ODBCUtil::instance()->drvByType.values("MimerSql");
    if(list.isEmpty())
        return "";
    QString connectionString = QString(
                                   "DRIVER={%5};"
                                   "PROTOCOL=tcp;"
                                   "NODE=%2;"
                                   "SERVICE=1360;"  // Verified from your dbVis Connection URL string
                                   "DATABASE=%1;"
                                   "UID=%3;"
                                   "PWD=%4;"
                                   ).arg(dbName.toLower(), server,userid,password, list.at(0)->name);
   return connectionString;
}

#ifdef Q_OS_WINDOWS
void ODBCUtil::getWinDSNs()
{
    if(bUpdating)
    {
        bUpdateHappened = true;
        return;
    }
    // QSettings winReg1("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
    // QSettings winReg2("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
    QStringList hives{"HKEY_CURRENT_USER","HKEY_LOCAL_MACHINE"};
    // QList<QSettings*> settings = QList<QSettings*>();
    // settings << &winReg1 <<  &winReg1;
    //odbcMap.clear();
    //QList<QPair<QString,QString> > list;
    //QMap<QString,QString> map;
    QStringList iniKeys;
    //odbcPairMap.clear();
    dsnByName.clear();
    dsnByType.clear();
    _dsnList.clear();
    bool bUserDsn = true; // first pass true, second pass false (Systen DSNs)
    foreach (QString hive, hives) {
        QString path = hive + "\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources";
        names = QSettings(path,QSettings::NativeFormat).childKeys();
        qDebug() << hive << " dsns.count " << QString("%1").arg(names.count());
        foreach (QString key, names) {
            QString keyVal = QSettings(path).value(key).toString();
            //map.clear();
            QString regKey = QString(hive+"\\Software\\ODBC\\ODBC.INI\\") + key;
            QSettings winReg(regKey,QSettings::NativeFormat);
            _dsn = new DSN();
            _dsnList.append(_dsn);
            _dsn->name = key;
            _dsn->userDsn = bUserDsn;
            iniKeys = winReg.childKeys();
            bool bError = false;
            for(QString iniKey : iniKeys)
            {
                QStringList childKeys = winReg.childKeys();
                for(QString iniKey3 : childKeys)
                {
                    QPair<QString,QString>  pair(iniKey3, winReg.value(iniKey3).toString());
                    _dsn->addPair(pair.first,pair.second);
                    //map.insert(iniKey3, winReg.value(iniKey3).toString());
                    if(iniKey3.compare("Driver", Qt::CaseInsensitive)==0)
                    {
                        QString value = winReg.value(iniKey3).toString();
                        QString driverVal;
                        if(value.contains("\\"))
                        {
                            driverVal = QDir::cleanPath(value);
                            _dsn->_lib = driverVal;
                            if(drvByLib.value(driverVal))
                            {
                                _dsn->_driverName = drvByLib.value(driverVal)->name;
                                _dsn->type = drvByLib.value(driverVal)->type;
                            }
                        }
                        else
                        {
                            driverVal = value;
                            _dsn->_driverName = driverVal;
                            Driver* drv = drvByName.value(driverVal);
                            if(drv)
                            {
                                _dsn->_lib = drv->lib;
                                _dsn->type = drv->type;
                            }
                        }
                        _dsn->_driver = value;
                        if(!_dsn->type.isEmpty())
                            _dsn->addPair("Type",_dsn->type);
                        Driver* pDriver = drvByLib.value(_dsn->_lib);
                        if(pDriver)
                        {
                            if(_dsn->_lib.contains(' '))
                                _dsn->_lib = getShortPath(_dsn->_lib);
                            _dsn->_driverName = pDriver->name;
                            _dsn->type = pDriver->type;
                            _dsn->addPair("Type", _dsn->type);
                            if(!pDriver->host.isEmpty())
                            {
                                _dsn->server = pDriver->host;
                                _dsn->addPair("Server", _dsn->server);
                            }
                            _dsn->port = pDriver->port;
                            _dsn->addPair("Port", QString::number(_dsn->port));
                            if(!pDriver->user.isEmpty())
                            {
                                _dsn->userId = pDriver->user;
                                _dsn->addPair("Uid", _dsn->userId);
                            }
                            if(!pDriver->pswd.isEmpty())
                            {
                                _dsn->setPassword(pDriver->pswd);
                                _dsn->addPair("Pwd", _dsn->password());
                            }
                        }
                        _dsn->addPair("Driver", _dsn->_driver);

                    }
                    if(iniKey3.compare("Description",Qt::CaseInsensitive)==0)
                    {
                        _dsn->descr = winReg.value(iniKey3).toString();
                        _dsn->addPair("Description", _dsn->descr);
                        if(_dsn->descr.contains("MsSql",Qt::CaseInsensitive)
                            || _dsn->descr.contains("Sql Server",Qt::CaseInsensitive))
                            _dsn->type = "MsSql";
                        if(_dsn->descr.contains("MySql",Qt::CaseInsensitive))
                            _dsn->type = "MySql";
                        if(_dsn->descr.contains("PostgreSQL",Qt::CaseInsensitive))
                            _dsn->type = "PostgreSQL";

                        if(!_dsn->type.isEmpty())
                            _dsn->addPair("Type", _dsn->type);
                    }
                    if(iniKey3.compare("Server", Qt::CaseInsensitive)==0 || iniKey3.compare("Servername", Qt::CaseInsensitive)==0)
                    {
                        QString server = winReg.value(iniKey3).toString();
                        QString host;
                        if(server.contains("\\"))
                        {
                            host = server.mid(0, server.indexOf("\\"));
                        }
                        else
                            if(server.contains("/"))
                            {
                                host = server.mid(0, server.indexOf("/"));
                            }
                            else
                                host = server;
                        // QHostInfo info = QHostInfo::fromName(host);
                        // if(info.error())
                        // {
                        //     //QMessageBox::warning(nullptr, tr("Warning"), tr("Invalid server: %1 for %2 error: %3").arg(server, iniKey3, info.errorString()));
                        //     qDebug() << tr("Invalid server: %1 for %2 error: %3").arg(server, iniKey3, info.errorString());
                        //     bError = true;
                        //     _dsn->bServerAccessible=false;
                        //     break;
                        // }
                            int port =-1;
                            if(_dsn->port >0 )
                                port = _dsn->port;
                            else if (_dsn->type == "MySql")
                                    port = 3306;
                            else if (_dsn->type == "MsSql")
                                    port = 1433;
                            else if (_dsn->type == "MimerSql")
                                    port = 1360;
                            else if (_dsn->type == "PostgreSQL")
                                    port = 5432;
                            _dsn->verifyServerAccessibility(host,port);
                        _dsn->server = server;
                        _dsn->addPair("Server", _dsn->server);
                    }
                    if(iniKey3.compare("Port", Qt::CaseInsensitive)==0 )
                    {
                        _dsn->port = winReg.value(iniKey3).toInt();
                        _dsn->addPair("Port",winReg.value(iniKey3).toString());
                    }
                    if(iniKey3.compare("UID", Qt::CaseInsensitive)==0 || iniKey3.compare("Username", Qt::CaseInsensitive)==0)
                    {
                        _dsn->userId = winReg.value(iniKey3).toString();
                        _dsn->addPair("Uid", _dsn->userId);
                    }
                    if(iniKey3.compare("PWD", Qt::CaseInsensitive)==0 || iniKey3.compare("Password", Qt::CaseInsensitive)==0)
                    {
                        _dsn->setPassword(winReg.value(iniKey3).toString());
                        _dsn->addPair("Pwd", _dsn->password());
                        _dsn->pwdInDsn = _dsn->password();
                    }

                    if(iniKey3.compare("Database", Qt::CaseInsensitive)==0)
                    {
                        _dsn->database = winReg.value(iniKey3).toString();
                        _dsn->addPair("Database", _dsn->database);
                   }
                    if(iniKey3.compare("SSLMODE", Qt::CaseInsensitive)==0)
                    {
                        _dsn->sslMode = winReg.value(iniKey3).toString();
                        _dsn->addPair("SslMode", _dsn->sslMode);
                    }
                    if(iniKey3.compare("Type", Qt::CaseInsensitive)==0)
                    {
                        _dsn->type =  winReg.value(iniKey3).toString();
                        if(_dsn->type.compare("MimerSql",Qt::CaseInsensitive)==0)
                            _dsn->type = "MimerSql";
                        _dsn->addPair("Type", _dsn->type);
                    }
                    if(iniKey3.compare("Location", Qt::CaseInsensitive)==0)
                    {
                        _dsn->location = winReg.value(iniKey3).toString();
                        _dsn->addPair("Location", _dsn->location);
                    }
                    if(iniKey3.compare("Setup", Qt::CaseInsensitive)==0)
                    {
                        _dsn->setup = winReg.value(iniKey3).toString();
                        _dsn->addPair("Setup", _dsn->setup);
                    }
                }
                if(bError)
                    break;
            }
            //odbcPairMap.insert(key, list);
            //odbcPairMap.insert(key,map);
            if(!dsnByName.contains(key))
            {
                dsnByName.insert(key, _dsn);
                if(validTypes.contains(_dsn->type))
                    dsnByType.insert(_dsn->type,_dsn);
            }
        } // end of hive
        bUserDsn = false;
    }
    emit odbc_changed();
}


void ODBCUtil::getWinDrivers()
{
    QString regKey = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers");
    QString drvKey = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\");

    QSettings winReg(regKey,QSettings::NativeFormat);
    QStringList drvKeys = winReg.childKeys();

    drvByLib.clear();
    drvByName.clear();
    drvByType.clear();
    for(QString drvName : drvKeys)
    {
        Driver* driver = new Driver();
        driver->name = drvName;
        if(drvName.contains("PostgreSQL",Qt::CaseInsensitive))
            driver->type = "PostgreSQL";
        else if(drvName.contains("MySql",Qt::CaseInsensitive))
            driver->type = "MySql";
        else if(drvName.contains("Sql Server",Qt::CaseInsensitive))
            driver->type = "MsSql";
        else if(drvName.contains("MIMER",Qt::CaseInsensitive) )
            driver->type = "MimerSql";
        else
            driver->type = "other";
        QSettings drvReg(drvKey+ drvName, QSettings::NativeFormat);
        QStringList childKeys = drvReg.childKeys();
        for(QString iniKey3 : childKeys)
        {
            if(iniKey3 == "Driver")
            {
                driver->lib = QDir::cleanPath(drvReg.value(iniKey3).toString());
                if(driver->lib.contains("mimodb",Qt::CaseInsensitive))
                    driver->type = "MimerSql";
                break;
            }
            if(iniKey3 == "Setup")
            {
                driver->setup = QDir::cleanPath(drvReg.value(iniKey3).toString());
            }
        }
        QFileInfo info = QFileInfo(driver->lib);
        if(!info.exists())
            continue;
        drvByType.insert(driver->type, driver);
        if(driver->lib.contains("%WINDIR%"))
            driver->lib = driver->lib.replace("%WINDIR%","C:\\windows");
        drvByName.insert(driver->name, driver);
    }
    foreach(Driver* drv, drvByName.values())
    {
        drvByLib.insert(drv->lib,drv);
    }
    emit odbc_changed();
}
#endif

bool ODBCUtil::isIp(QString txt)
{
    // ist txt a valid ip address exptression?
    static QRegularExpression ipV4 ("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    QRegularExpressionMatch match = ipV4.match(txt);
    if(match.hasMatch())
        return true;
    static QRegularExpression ipV6("\\A(?:[A-F0-9]{1,4}:){7}[A-F0-9]{1,4}\\Z");
    match = ipV6.match(txt.toUpper());
    return match.hasMatch();
}

QString ODBCUtil::getShortPath(const QString &longPath)
{
#ifdef Q_OS_WIN
    // 1. Convert Qt-style forward slashes to Windows-native backslashes
    QString nativePath = QDir::toNativeSeparators(longPath);

    // 2. Convert QString to a null-terminated wchar_t array for the Win32 API
    const wchar_t* longPathW = reinterpret_cast<const wchar_t*>(nativePath.utf16());

    // 3. Query the required buffer size first
    unsigned long bufferSize = GetShortPathNameW(longPathW, nullptr, 0);
    if (bufferSize == 0) {
        return longPath; // Returns original path if the file/folder doesn't exist
    }

    // 4. Allocate buffer and perform the actual conversion
    std::wstring shortPathBuffer(bufferSize, L'\0');
    GetShortPathNameW(longPathW, &shortPathBuffer[0], bufferSize);

    // 5. Clean up the trailing null character and return as QString
    return QString::fromWCharArray(shortPathBuffer.c_str());
#else
    return longPath; // No-op on non-Windows operating systems
#endif
}

void ODBCUtil::addDsn(DSN* dsn)
{
    if(!dsnByName.contains(dsn->name))
    {
        dsnByName.insert(dsn->name,dsn);
        if(!dsn->type.isEmpty())
            dsnByType.insert(dsn->type,dsn);
    }
}

QList<Driver*> ODBCUtil::drivers(QString type)
{
    if(type.isEmpty())
      return drvByName.values();
    else
      return drvByType.values(type);
}

QList<DSN*> ODBCUtil::getDsnList(bool bUserDSN)
{
    QList<DSN*> list;
    foreach (DSN* dsn, _dsnList) {
        if(dsn->userDsn == bUserDSN)
            list.append(dsn);
    }
    return list;
}


// ***********************************************************************

