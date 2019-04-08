#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include "data.h"
#include <QSettings>
#include <iostream>
#include <QDebug>
#include "settingsdb.h"

class Configuration;
class SQL;
class Connection : public QObject
{
 Q_OBJECT
 qint32 _id;
 QString _driver;
 QString _description;
 QString _DSN;
 QString _UID;
 QString _PWD;
 QString _database;
 QString _hostName;
 qint32  _port;
 QString _useDatabase;
 QSqlDatabase db;
 SQL* sql;
 QString _servertype; // "MsSql (default), "MySql"
 Configuration* config;
 bool bOpen;
public:
 Connection()
 {
  _DSN = "";
  _servertype = "MsSql";
  bOpen = false;
 }
 bool isOpen() {return bOpen;}
 void setServerType(QString s) { _servertype = s;}
 QString servertype() {return _servertype;}
 QSqlDatabase configure(const QString cName = QLatin1String(QSqlDatabase::defaultConnection));
 QSqlDatabase getDb();
 qint32 id() {return _id;}
 void setId(qint32 i) {_id = i;}
 QString description() { return _description;}
 void setDescription(QString s) {_description = s;}
 QString driver() {return _driver;}
 void setDriver(QString d) {_driver = d;}
 QString dsn() {return _DSN;}
 void setDSN(QString d) {_DSN = d;}
 QString uid() {return _UID;}
 void setUID(QString u) {_UID = u;}
 QString pwd() {return _PWD;}
 void setPWD(QString p) {_PWD = p;}
 QString database() {return _database;}
 void setDatabase(QString p) {_database = p;}
 QString host() {return _hostName;}
 void setHost(QString h) {_hostName = h;}
 qint32 port() {return _port;}
 void setPort(qint32 p) {_port = p;}
 QString useDatabase() {return _useDatabase;}
 void setUseDatabase(QString u) {_useDatabase = u;}

};

class Overlay
{
public:
 //qint32 id;
 QString name;
 QString description;
 qint32 opacity;
 int minZoom;
 int maxZoom;
 QString source;
 bool bLocal;
 Bounds bounds; // west longitude, south Latitude, east longitude, north latitude
 QString sCenter; // longitude, latitude, zoom level
 QStringList urls;
 Overlay()
 {
  source="acksoft";
  bLocal = false;
 }

 Overlay(QString name, int opacity = 65)
 {
  this->name = name;
  this->opacity = opacity;
  source="acksoft";
  bLocal = false;
  minZoom = 8;
  maxZoom = 16;
 }
};

class City
{
public:
 qint32 id;
 QString name;
 QList<Connection*> connections;
 QList<Overlay*> overlays;
 qint32 curConnectionId;
 qint32 curExportConnId;
 qint32 curOverlayId;
 LatLng center;
 QString mapType;
 qint32 zoom;
 bool bAlphaRoutes;
 bool bNoPanOpt;
 bool bGeocoderRequest;
 qint32 lastRoute;
 QString lastRouteName;
 QString lastRouteEndDate;
 bool bDisplayStationMarkers;
 bool bDisplayTerminalMarkers;
 bool bDisplayRouteComments;
 bool bShowOverlay;
 qint32 companyKey;
 qint32 routeSortType;
 QString savedClipboard;
 bool bUserMap;
 City()
 {
  curExportConnId = 0;
  lastRoute = 0;
  routeSortType = 0;
 }
};

struct query
{
 bool b_stop_query_on_error;
 bool b_sql_execute_after_loading;
 QString s_query_path;
 QByteArray geometry;
};

class Configuration : public QObject
{
 Q_OBJECT
public:
 City* currCity;
 Connection* currConnection;
 void getSettings();
 void saveSettings();
 void setOverlay(Overlay* ov);
 QList<City*> cityList;
 QMap<QString, Overlay*> overlayList;
 qint32 currentCityId;
 query q;
 static Configuration* instance();
 bool bDisplayWebDebug;
 bool bRunInBrowser;
 QStringList localOverlayList;
 QStringList georeferencedList;
 QString path;

private:
 static Configuration* _instance;
 explicit Configuration(QObject *parent = 0);

signals:

public slots:

};

#endif // CONFIGURATION_H
