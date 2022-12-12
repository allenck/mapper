#include "configuration.h"
#include "sql.h"
#include "overlay.h"
#include "data.h"

Configuration::Configuration(QObject *parent) :
    QObject(parent)
{
 currentCityId = -1;
 qRegisterMetaType<Bounds>("Bounds");
 //qRegisterMetaTypeStreamOperators<Bounds>("Bounds");
}

void Configuration::saveSettings()
{
// if(currConnection->id() != currCity->curConnectionId)
// {
//  qDebug() << "connection id mismatch: currConnection->id = " +QString("%1").arg(currConnection->id()) + " vs. currCity->curConnectionId = " + QString("%1").arg(currCity->curConnectionId)+"\n";
//  exit(EXIT_FAILURE);
// }
 currCity->connections.insert(currConnection->description(), currConnection);
 // Save any changes to currentCity
 if(currentCityId >= 0)
  cityList.values().replace(currentCityId, currCity);


 QSettings* settings = new QSettings();
 //settingsDb settings;
 settings->beginWriteArray("cities");
 for(int i=0; i< cityList.count(); i++)
 {
  settings->setArrayIndex(i);
  City* c = cityList.values().at(i);
  //currCity->setName(c->name());
  settings->setValue("id", c->id);
  settings->setValue("name", c->name());
  settings->setValue("lat", c->center.lat());
  settings->setValue("lon", c->center.lon());
  settings->setValue("mapType", c->mapType);
  settings->setValue("zoom", c->zoom);
  settings->setValue("currConnection", c->curConnectionId);
  settings->setValue("currExportConnId", c->curExportConnId);
  settings->setValue("AlphaRoutes", c->bAlphaRoutes);
  settings->setValue("noPanOpt", c->bNoPanOpt);
  settings->setValue("GeocoderRequest", c->bGeocoderRequest);
  settings->setValue("lastRoute", c->lastRoute);
  settings->setValue("lastRouteName", c->lastRouteName);
  settings->setValue("lastRouteEndDate", c->lastRouteEndDate);
  settings->setValue("displayStationMarkers", c->bDisplayStationMarkers);
  settings->setValue("displayTerminalMarkers", c->bDisplayTerminalMarkers);
  settings->setValue("displayRouteComments", c->bDisplayRouteComments);
  settings->setValue("showOverlay", c->bShowOverlay);
  settings->setValue("companyKey", c->companyKey);
  settings->setValue("routeSortType", c->routeSortType);
  settings->setValue("savedClipboard", c->savedClipboard);
  settings->setValue("userMap", c->bUserMap);
  settings->setValue("bounds", c->bounds().toString());
  //settings.setValue("connection",c->curConnectionId);
  settings->beginWriteArray("connections");
  for(int j=0; j < c->connections.count(); j++)
  {
   Connection* cn = c->connections.values().at(j);
   settings->setArrayIndex(j);
   settings->setValue("id", cn->id());
   settings->setValue("database", cn->database());
   settings->setValue("DSN", cn->dsn());
   settings->setValue("driver", cn->driver());
   settings->setValue("PWD", cn->pwd());
   settings->setValue("UID", cn->uid());
   settings->setValue("description",cn->description());
   settings->setValue("driver", cn->driver());
   settings->setValue("serverType", cn->servertype());
   if(cn->host() != "")
    settings->setValue("hostname", cn->host());
   if(cn->port() > 0)
    settings->setValue("port", cn->port());
   settings->setValue("useDatabase",cn->useDatabase());
   settings->setValue("cityName", cn->cityName());
  }
  settings->endArray(); // connections

  settings->beginGroup("overlays");
  settings->remove("");
  settings->endGroup();
#if 0
  settings->beginWriteArray("overlays");
//  int oCount = c->overlayList.count();
  int j=-1;
  foreach(Overlay* ov, c->city_overlayMap->values())
  {
   if(ov->cityName != currCity->name)
    continue;
   j++;
   settings->setArrayIndex(j);
   settings->setValue("id", j);
   settings->setValue("name", ov->name);
   settings->setValue("description", ov->description);
   settings->setValue("opacity", ov->opacity);
   settings->setValue("minZoom", ov->minZoom);
   settings->setValue("maxZoom", ov->maxZoom);
   settings->setValue("bounds", ov->bounds().toString());
   settings->setValue("center", ov->sCenter);
   settings->setValue("source", ov->source);
   settings->setValue("urls", ov->urls);
   settings->setValue("isSelected", ov->isSelected);
   settings->setValue("WMTSUrl", ov->wmtsUrl);
   settings->setValue("city", c->name);
   settings->setValue("year", ov->year());
  }
  settings->endArray(); // overlays
#endif
  settings->setValue("currOverlay", c->curOverlayId);
 }
 settings->endArray(); // cities

 settings->beginGroup("query");
 settings->setValue("stop_query_on_error", q.b_stop_query_on_error);
 settings->setValue("sql_execute_after_loading", q.b_sql_execute_after_loading);
 settings->setValue("queryPath", q.s_query_path);
 settings->setValue("geometry", q.geometry);
 settings->endGroup();

 settings->setValue("currCity", currentCityId);
 settings->setValue("showDebugMessages", bDisplayWebDebug);
 settings->setValue("runInBrowser", bRunInBrowser);
}

void Configuration::getSettings()
{
 QSettings settings;
 qDebug() << settings.fileName();
 //settingsDb settings;
 int size = settings.beginReadArray("cities");
 if( size == 0)
 {
  //  create a default configuration
  City* newCity = new City();
  newCity->id = 0;
  newCity->setName("St. Louis, MO");
  double latitude = 38.65858545118455;
  double longitude = -90.34764714500002;
  newCity->center = LatLng(latitude, longitude);
  qint32 zoom = 10;
  QString maptype = "roadmap";
  newCity->center = LatLng(latitude, longitude);
  newCity->setBounds(Bounds(LatLng(38.558585451, -90.447647145), LatLng(38.758585451, -90.247647145)));
  newCity->zoom = zoom;
  newCity->mapType = maptype;
  newCity->curConnectionId = 0;
  newCity->companyKey=0;
  if(!cityList.values().contains(newCity))
   cityList.insert(newCity->name(), newCity);
  Connection* nc = new Connection();
  nc->setId(0);
  nc->setDatabase("Resources/databases/StLouis.sqlite3");
  nc->setDescription ("SQLITE3 connection");
  nc->setDriver("QSQLITE");
  nc->setDSN("");
  nc->setUID("");
  nc->setPWD("");
  nc->setServerType("Sqlite");
  nc->setHost("");
  nc->setPort(0);
  nc->setCityName(newCity->name());
  newCity->connections.insert(nc->description(), nc);
  currConnection = nc;

  newCity = new City();
  newCity->id = 1;
  newCity->setName("Cincinnati, OH");
  latitude = 39.10457;
  longitude = -84.51382;
  //qint32 zoom = settings.value("zoom").toInt();
  newCity->center = LatLng(latitude, longitude);
  newCity->setBounds(Bounds(LatLng(38.995919924, -84.788475037), LatLng(39.213049836, -84.23915863)));
  newCity->zoom = zoom;
  newCity->mapType = maptype;
  newCity->curConnectionId =0 ;
  newCity->companyKey=0;
  if(!cityList.values().contains(newCity))
   cityList.insert(newCity->name(), newCity);
  nc = new Connection();
  nc->setId(0);
  nc->setDatabase("Resources/databases/cincinnati.sqlite3");
  nc->setDescription ("SQLITE3 connection");
  nc->setDriver("QSQLITE");
  nc->setDSN("");
  nc->setUID("");
  nc->setPWD("");
  nc->setServerType("Sqlite");
  nc->setHost("");
  nc->setPort(0);
  newCity->connections.insert(nc->description(),nc);
  currConnection = nc;

  newCity = new City();
  newCity->id = 2;
  newCity->setName("Louisville, KY");
  latitude = 38.25228;
  longitude = -85.76115;
  //qint32 zoom = settings.value("zoom").toInt();
  newCity->center = LatLng(latitude, longitude);
  newCity->setBounds(Bounds(LatLng(38.15228, -85.86115), LatLng(38.35228, -85.66115)));
  newCity->zoom = zoom;
  newCity->mapType = maptype;
  newCity->curConnectionId = 0 ;
  newCity->companyKey=0;
  if(!cityList.values().contains(newCity))
   cityList.insert(newCity->name(), newCity);
  nc = new Connection();
  nc->setId(0);
  nc->setDatabase("Resources/databases/louisville.sqlite3");
  nc->setDescription ("SQLITE3 connection");
  nc->setDriver("QSQLITE");
  nc->setDSN("");
  nc->setUID("");
  nc->setPWD("");
  nc->setServerType("Sqlite");
  nc->setHost("");
  nc->setPort(0);
  nc->setCityName(newCity->name());
  newCity->connections.insert(nc->description(),nc);
  currConnection = nc;

  newCity = new City();
  newCity->id = 3;
  newCity->setName("Berlin. Germany");
  latitude = 52.5315;
  longitude = 13.00165;
  newCity->center = LatLng(latitude, longitude);
  newCity->setBounds(Bounds(LatLng(52.4315, 12.90165), LatLng(52.6315, 13.10165)));
  newCity->zoom = zoom;
  newCity->mapType = maptype;
  newCity->curConnectionId = 0 ;
  newCity->companyKey=0;
  if(!cityList.values().contains(newCity))
   cityList.insert(newCity->name(), newCity);
  nc = new Connection();
  nc->setId(0);
  nc->setDatabase("Resources/databases/berlinerstrassenbahn.sqlite3");
  nc->setDescription ("SQLITE3 connection");
  nc->setDriver("QSQLITE");
  nc->setDSN("");
  nc->setUID("");
  nc->setPWD("");
  nc->setServerType("Sqlite");
  nc->setHost("");
  nc->setPort(0);
  nc->setCityName(newCity->name());
  newCity->connections.insert(nc->description(),nc);
  currConnection = nc;

  newCity = cityList.values().at(0);
  Overlay* ov = new Overlay("St Louis, MO", "St_Louis_historical_topo");

  newCity->city_overlayMap->insert(ov->name, ov);

  newCity->curOverlayId = 0;
  newCity->bShowOverlay =true;
  currCity = cityList.values().at(0);
  currentCityId = 0;
  currConnection = currCity->connections.values().at(0);
  currConnection->setId(currCity->curConnectionId);
  if(Overlay::importXml("./overlays.xml"))
  {
   for(Overlay* ov : Overlay::overlayList)
   {
    overlayMap->insert(ov->cityName+"|"+ov->name, ov);
    City* city = cityList.value(ov->cityName);
    if(city)
    {
     city->city_overlayMap->insert(ov->name, ov);
    }
   }
  }

  saveSettings();

  return;
 } // end default configuration

 bool ok = Overlay::importXml("./overlays.xml");

//  settings.remove("center/latitude");
//  settings.remove("center/longitude");
//  settings.remove("zoom");
//  settings.remove("maptype");
//  newCity->connections.append(nc);


 // load cities
 for(int i= 0; i < size; i++)
 {
  settings.setArrayIndex(i);
  City* nc = new City();
  nc->id = settings.value("id").toInt();
  nc->setName(settings.value("name").toString());
  cityList.insert(nc->name(),nc);
  LatLng pt;
  qDebug() << "lat: " << settings.value("lat",35).toString();
  qDebug() << "lon: " << settings.value("lon",-90).toString();
  pt.setLat(settings.value("lat",35).toDouble());
  pt.setLon(settings.value("lon",-90).toDouble());
  nc->center = pt;
  nc->setBounds(Bounds(settings.value("bounds").toString()));
  if(!nc->bounds().isValid())
  {
   nc->setBounds(Bounds(LatLng(pt.lat()-.1, pt.lon()-.1), LatLng(pt.lat()+.1, pt.lon()+.1)));
  }
  if(nc->bounds().isValid()){
   cityBounds.insert(nc->name(), nc->bounds());
   nc->center = nc->bounds().center();
  }
  nc->mapType = settings.value("maptype","roadmap").toString();
  nc->zoom = settings.value("zoom",12).toInt();
  nc->curConnectionId = settings.value("currConnection",0).toInt();
  nc->curExportConnId = settings.value("currExportConnId",0).toInt();
  nc->bAlphaRoutes = settings.value("AlphaRoutes").toBool();
  nc->bNoPanOpt = settings.value("noPanOpt").toBool();
  nc->bGeocoderRequest = settings.value("GeocoderRequest").toBool();
  nc->lastRoute = settings.value("lastRoute",0).toInt();
  nc->lastRouteName = settings.value("lastRouteName").toString();
  nc->lastRouteEndDate = settings.value("lastRouteEndDate").toString();
  nc->bDisplayStationMarkers = settings.value("displayStationMarkers",false).toBool();
  nc->bDisplayRouteComments = settings.value("displayRouteComments",false).toBool();
  nc->bDisplayTerminalMarkers = settings.value("displayTerminalMarkers",false).toBool();
  nc->bShowOverlay = settings.value("showOverlay").toBool();
  nc->companyKey = settings.value("companyKey").toInt();
  nc->routeSortType = settings.value("routeSortType").toInt();
  nc->savedClipboard = settings.value("savedClipboard").toString();
  nc->curOverlayId = settings.value("currOverlay", -1).toInt();
  nc->bUserMap = settings.value("userMap", false).toBool();
  int sizec = settings.beginReadArray("connections");
  for(int j = 0; j < sizec; j++)
  {
   settings.setArrayIndex(j);
   Connection* ncn = new Connection();
   ncn->setId(settings.value("id").toInt());
   ncn->setDatabase(settings.value("database").toString());
   ncn->setDescription(settings.value("description").toString());
   ncn->setDriver(settings.value("driver").toString());
   ncn->setDSN(settings.value("DSN").toString());
   ncn->setPWD(settings.value("PWD").toString());
   ncn->setUID(settings.value("UID").toString());
   ncn->setHost(settings.value("hostname").toString());
   ncn->setPort(settings.value("port").toInt());
   ncn->setServerType(settings.value("serverType").toString());
   ncn->setUseDatabase(settings.value("useDatabase", "default").toString());
   if(ncn->servertype() == "")
    ncn->setServerType("MySql");
   if(ncn->servertype() == "Sqlite")
   {
    ncn->setPWD("");
    ncn->setUID("");
    ncn->setPort(0);
    ncn->setHost("");
    ncn->setUseDatabase("");
    ncn->setCityName(settings.value("cityName", nc->name()).toString());
#ifdef WIN32
    QString ext = ncn->database().mid(ncn->database().lastIndexOf("."));
    if(ncn->database().toLower().endsWith(".sqlite3") && ext != ".sqlite3")
    {
     ncn->setDatabase(ncn->database().replace(ext, ".sqlite3"));
    }
#endif
   }
   nc->connections.insert(ncn->description(), ncn);
  }
  settings.endArray();

  if(currentCityId < 0)
  {
   currCity = cityList.values().at(0);
  }

  //qDebug() << "city bounds:" << cityBounds;
  if(nc->city_overlayMap->isEmpty())
   nc->bShowOverlay = false;
  nc->curOverlayId = settings.value("currOverlay").toInt();
  if(nc->city_overlayMap->count()== 0)
   nc->curOverlayId = -1;
 } // end cities

#if 0 // for testing purposes, create a MySql connection
  City* currCity = cityList.values().at(3);

  Connection* nc = new Connection();
  nc->setId(1);
  nc->setDatabase("StLouisMySql");
  nc->setDescription ("Test MySql connection");
  nc->setDriver("QMYSQL");
  nc->setDSN("");
  nc->setUID("allen");
  nc->setPWD("iic723");
  nc->setServerType("MySql");
  nc->setHost("192.168.1.101");
  nc->setPort(3306);
  currCity->connections.insert(nc->description(),nc);
  currConnection = nc;
#endif

 settings.endArray();

// for(Overlay* ov : overlayMap->values())
// {
//  QString  n = lookupCityName(ov->bounds());
//  if(!n.isEmpty())
//   ov->cityName = n;
//  else
//   qDebug() << "invalid city " << ov->cityName << ov->name;
// }
 if(Overlay::importXml("./overlays.xml"))
 {
  for(Overlay* ov : Overlay::overlayList)
  {
   QString cityName = ov->cityName;
   City* city = cityList.value(ov->cityName);
   if(city && !ov->bounds().isValid())
   {
    ov->setBounds(Bounds(LatLng(city->center.lat()-.3, city->center.lon()-.3), LatLng(city->center.lat()+.3, city->center.lon()+.3)));
   }
   overlayMap->insert(ov->cityName+"|"+ov->name, ov);
   if(city && city->name() == ov->cityName)
   {
    if(ov->isSelected)
    {
     city->city_overlayMap->insert(ov->name, ov);
     qDebug() << "add overlay " << ov->name << " for city:" << city->name();
    }
   }
  }
 }
 //settings.beginGroup("General");
 currentCityId = settings.value("currCity",0).toInt();
 if(currentCityId < 0 || currentCityId >= cityList.count())
  currentCityId = 0;
 currCity = cityList.values().at(currentCityId);
 if(currCity->curConnectionId < 0 && currCity->connections.size() == 1)
  currCity->curConnectionId =0;
 currConnection =   currCity->connections.values().at(currCity->curConnectionId);
 bDisplayWebDebug = settings.value("showDebugMessages", false).toBool();
 bRunInBrowser = settings.value("runInBrowser", false).toBool();
 //settings.endGroup();

 settings.beginGroup("query");
 q.b_stop_query_on_error = settings.value("stop_query_on_error").toBool();
 q.b_sql_execute_after_loading = settings.value("sql_execute_after_loading").toBool();
 q.s_query_path = settings.value("queryPath", QDir::homePath()).toString();
 q.geometry = settings.value("geometry").toByteArray();

 settings.endGroup();

 for(Overlay* ov : Overlay::overlayList)
 {
  overlayMap->insert(ov->cityName+"|"+ov->name, ov);
  City* city = cityList.value(ov->cityName);
  if(city)
  {
   city->city_overlayMap->insert(ov->name, ov);
  }
 }

}

void Configuration::setOverlay(Overlay* ov)
{
 for(int i=0; i < currCity->city_overlayMap->count(); i++)
 {
  Overlay* o  =  currCity->city_overlayMap->values().at(i);
  if(ov->name == o->name)
  {
   o->opacity = ov->opacity;
   //currCity->overlayMap->replace(i, o);
   currCity->city_overlayMap->insert(ov->name, ov);
  }
 }
}

Configuration* Configuration::_instance = NULL;

Configuration* Configuration::instance()
{
 if(_instance == NULL)
  _instance = new Configuration();
 return _instance;
}



QSqlDatabase Connection::configure(QString cName)
{
 qDebug() << "Connection: CWD = " << QDir::currentPath();
 config = Configuration::instance();
 sql = SQL::instance();
  db = QSqlDatabase::addDatabase(config->currConnection->driver(),cName);
 //db.setHostName("10.0.1.100");
 if(config->currConnection->host() != "")
  db.setHostName(config->currConnection->host());
 if(config->currConnection->port() > 0)
  db.setPort(config->currConnection->port());
 if(config->currConnection->driver() == "QODBC" || config->currConnection->driver() == "QODBC3")
  db.setDatabaseName(config->currConnection->dsn());
 else
 {
  QString dbName = config->currConnection->database();
  QFileInfo info(dbName);
  if(!info.isAbsolute() && config->currConnection->driver() == "QSQLITE")
  {
   if(!dbName.startsWith("Resources/databases/"))
    dbName = "Resources/databases/" + dbName;
   if(!dbName.endsWith(".sqlite3"))
    dbName.append(".sqlite3");
   //dbName = QDir(config->path + QDir::separator() + QDir::separator()+ dbName).path();
  }
  db.setDatabaseName(dbName);
 }
 db.setUserName(config->currConnection->uid());
 db.setPassword(config->currConnection->pwd());
 bOpen = db.open();
 // check for presence of Parameters table.
 QStringList tableList;
// if(ok)
// {
 if(bOpen)
 {
  if(config->currConnection->useDatabase() != "default" && config->currConnection->useDatabase() != "")
  {
   QSqlQuery query = QSqlQuery(db);
   QString cmd = QString("use [%1]").arg(config->currConnection->useDatabase());
   if(!query.exec(cmd))
   {
    SQLERROR(query);
    db.close();
    bOpen = false;
    return db;
   }
   sql->checkTables(db);
   tableList = sql->getTableList(db, config->currConnection->servertype());

   if(!tableList.contains("Parameters",Qt::CaseInsensitive))
   {
    bOpen = false;
    return db;
   }
   else
   {
    QSqlError err = db.lastError();
    QString msg;
    msg = err.text() + "\n"
           + db.driverName() + "\n"
           + "User:" + db.userName() + "\n"
           + "Host:" + db.hostName()+ "\n"
           + "Connection name: " + db.connectionName() + "\n"
           + "DSN:" + db.databaseName() + "\n"
    + "driver: " + config->currConnection->driver() + "\n";
    if(config->currConnection->driver()=="QSQLITE")
    {
     QFileInfo info(db.databaseName());
     msg.append(info.absoluteFilePath());
    }
    QMessageBox::warning(NULL, "Warning", msg);
   }
   }
  if(sql->loadSqlite3Functions())
  {
   sql->checkTables(db);
  }
 }
 else
 {
  qDebug() << "unable to open " << db.databaseName() << " " << db.lastError().text();
  QFileInfo info(db.databaseName());
  if(info.exists())
   qDebug() << "file exists.";
  else
   qDebug() << "file not found.";
 }
  return db;
}

QStringList Configuration::cityNames()
{
 QStringList result;
 for(City* city : cityList)
  result.append(city->name());
 return result;
}

QString Configuration::lookupCityName(Bounds b)
{
 QMapIterator<QString, Bounds> iter(cityBounds);
 while(iter.hasNext())
 {
  iter.next();
  if(iter.value().contains(b))
     return iter.key();
 }
 return "";
}
