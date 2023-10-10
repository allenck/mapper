#include "configuration.h"
#include "sql.h"
#include "overlay.h"
#include "data.h"
#include <QApplication>

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
// if(!currCity->connections.contains(currConnection))
// {
//  currCity->connections.append(currConnection);
//  currCity->connectionNames.append(currConnection->description());
// }
 // Save any changes to currentCity
// if(currentCityId >= 0)
// {
//  cityList.replace(currentCityId, currCity);

// }

 QSettings* settings = new QSettings();
 //settingsDb settings;
 settings->beginWriteArray("cities");
 settings->remove("");
 for(int i=0; i< cityList.count(); i++)
 {
  settings->setArrayIndex(i);
  City* c = cityList.at(i);
  //currCity->setName(c->name());
  settings->setValue("id", c->id);
  settings->setValue("name", c->name());
  settings->setValue("lat", c->center.lat());
  settings->setValue("lon", c->center.lon());
  settings->setValue("mapType", c->mapType);
  settings->setValue("zoom", c->zoom);
  settings->setValue("currConnection", c->curConnectionId);
  settings->setValue("currConnectionUuid", c->connectionUniqueId().toString());
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
  settings->remove("");
  for(int j=0; j < c->connections.count(); j++)
  {
   Connection* cn = c->connections.at(j);
   if(cn->uniqueId().isNull())
       qDebug() << "invalid unique id " << cn->description() << " ";
   settings->setArrayIndex(j);
   settings->remove("");
   settings->setValue("id", cn->id());
   settings->setValue("driver", cn->driver());
   settings->setValue("serverType", cn->servertype());
   settings->setValue("cityName", cn->cityName());
   settings->setValue("connectionType", cn->connectionType());
   settings->setValue("description",cn->description());
   settings->setValue("uniqueId", cn->uniqueId().toString());
   settings->setValue("defaultSqlDatabase", cn->defaultSqlDatabase());

   if(cn->connectionType()== "Local")
   {
       settings->setValue("sqliteFileName", cn->sqlite_fileName());
   }
   else if(cn->connectionType()== "ODBC")
   {
       settings->setValue("odbcConnector", cn->odbc_connectorName());
       settings->setValue("defaultSqlDatabase", cn->defaultSqlDatabase());
       //settings->setValue("mySqlDatabase", cn->mySqlDatabase());
   }
   else if(cn->connectionType()== "Direct")
   {
    settings->setValue("database", cn->database());
    settings->setValue("DSN", cn->dsn());
    settings->setValue("driver", cn->driver());
    settings->setValue("PWD", cn->pwd());
    settings->setValue("UID", cn->userId());
    if(cn->host() != "")
     settings->setValue("hostname", cn->host());
    if(cn->port() > 0)
     settings->setValue("port", cn->port());
    settings->setValue("defaultSqlDatabase", cn->defaultSqlDatabase());
    //settings->setValue("mySqlDatabase",cn->mySqlDatabase());
   }
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
 settings->setValue("saveImageDir", saveImageDir);
 settings->setValue("showGMFeatures", bShowGMFeatures);
}

void Configuration::getSettings()
{
  QSettings settings;
  qDebug() << settings.fileName();
  //settingsDb settings;
  int size = settings.beginReadArray("cities");
  if( size == 0)
  {
     createDefaultSettings();
  }

 bool ok = Overlay::importXml("./overlays.xml");

 // load cities
 for(int i= 0; i < size; i++)
 {
  settings.setArrayIndex(i);
  City* nc = new City();
  nc->id = settings.value("id").toInt();
  nc->setName(settings.value("name").toString());
  if(!cityList.contains(nc))
  {
   cityList.append(nc);
   cityMap.insert(nc->name(), nc);
  }
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
  nc->setConnectionUniqueId(QUuid::fromString(settings.value("currConnectionUuid").toString()));
  QString baseAddr = QDir::currentPath() +QDir::separator() + "Resources" + QDir::separator()+"databases" + QDir::separator();
  int sizec = settings.beginReadArray("connections");

  // connections
  for(int j = 0; j < sizec; j++)
  {
   settings.setArrayIndex(j);
   QString uuid_string = settings.value("uniqueId").toString();
   QUuid uuid;
   if(uuid_string.isEmpty())
       uuid = QUuid::createUuid();
   else
       uuid = QUuid::fromString(uuid_string);
   Connection* ncn = new Connection(uuid);
   ncn->setId(settings.value("id", -1).toInt());
   ncn->setDescription(settings.value("description").toString());
   ncn->setDriver(settings.value("driver").toString());
   ncn->setServerType(settings.value("serverType").toString());
   ncn->setCityName(settings.value("cityName", nc->name()).toString());
   ncn->setConnectionType(settings.value("connectionType", "Direct").toString());
   //ncn->setUniqueId(QUuid::fromString(settings.value("uniqueId", QUuid::createUuid().toString()).toString()));
   if(ncn->id() == nc->curConnectionId)
       nc->setConnectionUniqueId(uuid);
   if(ncn->connectionType() == "Local")
   {
    ncn->setConnectionType("Local");
    QString fileName;
     fileName = settings.value("sqliteFileName").toString();
     if(fileName.isEmpty())
         fileName = settings.value("database").toString();
     QFileInfo info(baseAddr + fileName);
     if(!info.exists())
         qDebug() << fileName << " not found";
//#ifdef WIN32
//     QString ext = fileName.mid(ncn->database().lastIndexOf("."));
//     if(fileName.toLower().endsWith(".sqlite3") && ext != ".sqlite3")
//     {
//      fileName.replace(ext, ".sqlite3");
//     }
//#endif
     ncn->setSqliteFileName(info.fileName());
   }
   else if(ncn->connectionType() == "Direct") {
       ncn->setDSN(settings.value("DSN").toString());
       ncn->setPWD(settings.value("PWD").toString());
       ncn->setUserId(settings.value("UID").toString());
       ncn->setHost(settings.value("hostname").toString());
       ncn->setPort(settings.value("port").toInt());
       ncn->setDefaultSqlDatabase(settings.value("defaultSqlDatabase").toString());
       //ncn->setMySqlDatabase(settings.value("mySqlDatabase").toString());
   }
   else if(ncn->connectionType() == "ODBC") {
    ncn->setOdbcConnectorName(settings.value("odbcConnector").toString());
    ncn->setDefaultSqlDatabase(settings.value("defaultSqlDatabase").toString());
    //ncn->setDefSqlDatabase(settings.value("mySqlDatabase").toString());
   }

   if(!nc->connections.contains(ncn))
   {
//    nc->connections.append( ncn);
//    nc->connectionNames.append(ncn->description());
//    nc->connectionMap.insert(ncn->uniqueId().toString(), ncn);
       nc->addConnection(ncn);
   }
  }
  settings.endArray();

  if(currentCityId < 0)
  {
   currCity = cityList.at(0);
  }

  //qDebug() << "city bounds:" << cityBounds;
  if(nc->city_overlayMap->isEmpty())
   nc->bShowOverlay = false;
  nc->curOverlayId = settings.value("currOverlay").toInt();
  if(nc->city_overlayMap->count()== 0)
   nc->curOverlayId = -1;
 } // end cities

#if 0 // for testing purposes, create a MySql connection
  City* currCity = cityList.at(3);

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

#if 0 // for testing purposes, create a MsSql connection
  City* currCity = cityList.at(3);

  Connection* nc = new Connection();
  nc->setId(1);
  nc->setDatabase("Mapper");
  nc->setDescription ("Test MsSql connection");
  nc->setDriver("QODBC");
  nc->setDSN("");
  nc->setUID("");
  nc->setPWD("");
  nc->setServerType("MsSql");
  nc->setHost("");
  nc->setPort(0);
  currCity->connections.insert(nc->description(),nc);
  //currConnection = nc;
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
   City* city = cityMap.value(ov->cityName);
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
 currCity = cityList.at(currentCityId);
 if(currCity->curConnectionId < 0 && currCity->connections.size() == 1)
  currCity->curConnectionId =0;
 currConnection =   currCity->connections.at(currCity->curConnectionId);
 bDisplayWebDebug = settings.value("showDebugMessages", false).toBool();
 bRunInBrowser = settings.value("runInBrowser", false).toBool();
 saveImageDir = settings.value("saveImageDir", "").toString();
 bShowGMFeatures = settings.value("showGMFeatures", true).toBool();
 //settings.endGroup();

 settings.beginGroup("query");
 q.b_stop_query_on_error = settings.value("stop_query_on_error").toBool();
 q.b_sql_execute_after_loading = settings.value("sql_execute_after_loading").toBool();
 q.s_query_path = settings.value("queryPath", QDir::homePath()+"/Resources/sql/").toString();
 q.geometry = settings.value("geometry").toByteArray();

 settings.endGroup();

 for(Overlay* ov : Overlay::overlayList)
 {
  overlayMap->insert(ov->cityName+"|"+ov->name, ov);
  City* city = cityMap.value(ov->cityName);
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





QStringList Configuration::cityNames()
{
 QStringList result;
 for(City* city : cityList)
  result.append(city->name());
 return result;
}

void Configuration::addCity(City* city)
{
    if(!cityMap.contains(city->name()))
    {
        cityList.append(city);
        cityMap.insert(city->name(), city);
        if(city->bounds().isValid())
         cityBounds.insert(city->name(), city->bounds());
        currCity = city;
        city->setId(cityList.count()-1);
        currentCityId = city->id;
        emit newCityCreated(city);
    }
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

void Configuration::createDefaultSettings()
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
    if(!cityList.contains(newCity))
    {
     cityList.append( newCity);
     cityMap.insert(newCity->name(), newCity);
    }
    //newCity->connectionNames.append(newCity->name());
    Connection* nc = new Connection();
    nc->setId(0);
    nc->setSqliteFileName("StLouis.sqlite3");
    nc->setDescription ("SQLITE3 connection");
    nc->setDriver("QSQLITE");
    nc->setServerType("Sqlite");
    nc->setConnectionType("Local");

    nc->setCityName(newCity->name());
    if(!newCity->connections.contains(nc))
    {
//     newCity->connections.append( nc);
//     newCity->connectionNames.append(nc->description());
        newCity->addConnection(nc);
    }
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
    if(!cityList.contains(newCity))
    {
     cityList.append(newCity);
     cityMap.insert(newCity->name(),newCity);
    }
    nc = new Connection();
    nc->setId(0);
    nc->setSqliteFileName("cincinnati.sqlite3");
    nc->setDescription ("SQLITE3 connection");
    nc->setDriver("QSQLITE");
    nc->setServerType("Sqlite");
    nc->setConnectionType("Local");
    if(!newCity->connections.contains(nc))
    {
//     newCity->connections.append( nc);
//     newCity->connectionNames.append(nc->description());
        newCity->addConnection(nc);
    }

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
    if(!cityList.contains(newCity))
    {
     cityList.append(newCity);
     cityMap.insert(newCity->name(), newCity);
    }
    nc = new Connection();
    nc->setId(0);
    nc->setSqliteFileName("louisville.sqlite3");
    nc->setDescription ("SQLITE3 connection");
    nc->setServerType("Sqlite");
    nc->setCityName(newCity->name());
    nc->setConnectionType("Local");
    if(!newCity->connections.contains(nc))
    {
//     newCity->connections.append( nc);
//     newCity->connectionNames.append(nc->description());
        newCity->addConnection(nc);
    }
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
    if(!cityList.contains(newCity))
    {
     cityList.append(newCity);
     cityMap.insert(newCity->name(), newCity);
    }
    nc = new Connection();
    nc->setId(0);
    nc->setSqliteFileName("berlinerstrassenbahn.sqlite3");
    nc->setDescription ("SQLITE3 connection");
    nc->setDriver("QSQLITE");
    nc->setServerType("Sqlite");
    nc->setCityName(newCity->name());
    nc->setConnectionType("Local");
    if(!newCity->connections.contains(nc))
    {
     newCity->connections.append( nc);
     newCity->connectionNames.append(nc->description());
    }
    currConnection = nc;

    Overlay* ov = new Overlay("St Louis, MO", "St_Louis_historical_topo");

    newCity->city_overlayMap->insert(ov->name, ov);

    newCity->curOverlayId = 0;
    newCity->bShowOverlay =true;

    currentCityId = 0;
    currConnection = newCity->connections.at(0);
    currConnection->setId(newCity->curConnectionId);
    if(Overlay::importXml("./overlays.xml"))
    {
     for(Overlay* ov : Overlay::overlayList)
     {
      overlayMap->insert(ov->cityName+"|"+ov->name, ov);
      currCity = cityList.at(0);
      City* city = cityMap.value(ov->cityName);
      if(city)
      {
       city->city_overlayMap->insert(ov->name, ov);
      }
     }
    }
    if(QGuiApplication::screens().count() > 1)
     bRunInBrowser = true;
    saveSettings();

    return;

} // end default configuration

