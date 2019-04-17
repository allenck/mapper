#include "configuration.h"
#include "sql.h"

Configuration::Configuration(QObject *parent) :
    QObject(parent)
{
 currentCityId = -1;
}
void Configuration::saveSettings()
{
 if(currConnection->id() != currCity->curConnectionId)
 {
  qDebug() << "connection id mismatch: currConnection->id = " +QString("%1").arg(currConnection->id()) + " vs. currCity->curConnectionId = " + QString("%1").arg(currCity->curConnectionId)+"\n";
  exit(EXIT_FAILURE);
 }
 currCity->connections.replace(currConnection->id(), currConnection);
 // Save any changes to currentCity
 if(currentCityId >= 0)
  cityList.replace(currentCityId, currCity);

 QSettings* settings = new QSettings();
 //settingsDb settings;
 settings->beginWriteArray("cities");
 for(int i=0; i< cityList.count(); i++)
 {
  settings->setArrayIndex(i);
  City* c = cityList.at(i);
  settings->setValue("id", c->id);
  settings->setValue("name", c->name);
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

  //settings.setValue("connection",c->curConnectionId);
  settings->beginWriteArray("connections");
  for(int j=0; j < c->connections.count(); j++)
  {
   Connection* cn = c->connections.at(j);
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
  }
  settings->endArray(); // connections

  settings->beginGroup("overlays");
  settings->remove("");
  settings->endGroup();
  settings->beginWriteArray("overlays");
  int oCount = c->overlayList().count();
  for(int j=0; j < c->overlayList().count(); j++)
  {
   Overlay* ov = c->overlayList().at(j);
   settings->setArrayIndex(j);
   settings->setValue("id", j);
   settings->setValue("name", ov->name);
   settings->setValue("description", ov->description);
   settings->setValue("opacity", ov->opacity);
   settings->setValue("minZoom", ov->minZoom);
   settings->setValue("maxZoom", ov->maxZoom);
   settings->setValue("bounds", ov->bounds.toString());
   settings->setValue("center", ov->sCenter);
   settings->setValue("source", ov->source);
   settings->setValue("urls", ov->urls);
  }
  settings->endArray(); // overlays
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
  // temporarily create a default configuration
  //TODO replace this with a dialog to ask for data to setup a city and a connection.
  City* newCity = new City();
  newCity->id = 0;
  newCity->name = "St. Louis, MO";
  double latitude = settings.value("center/latitude").toDouble();
  double longitude = settings.value("center/longitude").toDouble();
  qint32 zoom = settings.value("zoom").toInt();
  QString maptype = settings.value("maptype").toString();
  newCity->center = LatLng(latitude, longitude);
  newCity->zoom = zoom;
  newCity->mapType = maptype;
  newCity->curConnectionId = newCity->id ;
  newCity->companyKey=0;

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

  Overlay* ov = new Overlay("St_Louis_historical_topo");
  newCity->overlayList().append(ov);

  newCity->curOverlayId = 0;
  newCity->bShowOverlay =true;

  settings.remove("center/latitude");
  settings.remove("center/longitude");
  settings.remove("zoom");
  settings.remove("maptype");
  newCity->connections.append(nc);
  cityList.append(newCity);
  currConnection = nc;
  currCity = newCity;
  currentCityId = 0;

  saveSettings();

  return;
 }

 // load cities
 for(int i= 0; i < size; i++)
 {
  settings.setArrayIndex(i);
  City* nc = new City();
  nc->id = settings.value("id").toInt();
  nc->name = settings.value("name").toString();
  LatLng pt;
  qDebug() << "lat: " << settings.value("lat",35).toString();
  qDebug() << "lon: " << settings.value("lon",-90).toString();
  pt.setLat(settings.value("lat",35).toDouble());
  pt.setLon(settings.value("lon",-90).toDouble());
  nc->center = pt;
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
  nc->bDisplayStationMarkers = settings.value("displayStationMarkers").toBool();
  nc->bDisplayRouteComments = settings.value("displayRouteComments").toBool();
  nc->bDisplayTerminalMarkers = settings.value("displayTerminalMarkers").toBool();
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
#ifdef WIN32
    QString ext = ncn->database().mid(ncn->database().lastIndexOf("."));
    if(ncn->database().toLower().endsWith(".sqlite3") && ext != ".sqlite3")
    {
     ncn->setDatabase(ncn->database().replace(ext, ".sqlite3"));
    }
#endif
    QFile file;
    file.setFileName(ncn->database());
//    if(!file.exists())
//    {
//     if(!ncn->database().toLower().endsWith(".sqlite3"))
//     {
//      ncn->setDatabase(ncn->database().append(".sqlite3"));
//     }
//    }
//    else
//    {
//     if(!ncn->database().toLower().endsWith(".sqlite3"))
//     {
//      if(file.rename(ncn->database(), ncn->database().append(".sqlite3")))
//       ncn->setDatabase(ncn->database().append(".sqlite3"));
//     }
//    }
   }
   nc->connections.append(ncn);
  }
  settings.endArray();

  int sizeo = settings.beginReadArray("overlays");
  for(int j =0; j < sizeo; j++)
  {
   settings.setArrayIndex(j);
   Overlay* no = new Overlay();
   //no->id = settings.value("id").toInt();
   no->name = settings.value("name").toString();
   no->description = settings.value("description").toString();
   no->opacity = settings.value("opacity").toInt();
   no->minZoom = settings.value("minZoom", 10).toInt();
   no->maxZoom = settings.value("maxZoom", 16).toInt();
   no->bounds = Bounds(settings.value("bounds").toString());
   no->sCenter = settings.value("center").toString();
   no->source = settings.value("source", "acksoft").toString();
   no->urls = settings.value("urls","http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/").toStringList();
   if(no->source == "acksoft" && no->urls.isEmpty())
    no->urls.append("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/");
   if(no->source == "mbtiles"&& no->urls.isEmpty()) // Windows
    no->urls.append("http://localhost/map_tiles/mbtiles.php");
   if(no->source == "tileserver" && no->urls.isEmpty()) // Linux
    no->urls.append("http://localhost/tileserver.php");
   bool bFound = false;
   if(no->source == "georeferencer") // add to global list
   {
    if(!overlayList.keys().contains(no->name))
     overlayList.insert(no->name, no);
   }
   if(!bFound)
    nc->addOverlay(no);
  }
  // TODO: create a dialog to add overlays like this
  if(nc->name == "St. Louis, MO")
  {
   Overlay* ov = new  Overlay("St Louis Worlds Fair 1904");
   ov->bounds =  Bounds(LatLng(38.623972, -90.330807), LatLng(38.658606, -90.273631));
   ov->source = "georeferencer";
   ov->maxZoom = 17;
   ov->minZoom = 0;
   ov->urls << "http://georeferencer-0.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-1.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-2.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-3.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png";
   bool bFound = false;
   foreach (Overlay* o, nc->overlayList())
   {
    if(o->name == ov->name)
    {
     bFound = true;
     o->urls = ov->urls;
     break;
    }
   }
   if(!bFound)
    nc->overlayList().append(ov);
  }
  if(nc->name == "Louisville, KY")
  {
   Overlay* ov = new  Overlay("Louisville, KY pilot2");
   ov->bounds =  Bounds(LatLng(38.1412, -85.91273), LatLng(38.351303, -85.626234));
   ov->source = "georeferencer";
   ov->maxZoom = 17;
   ov->minZoom = 0;
   ov->urls << "http://georeferencer-0.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/SQOqJ3TkkQzNnQyf8X5k4n/201502111947-kh1nwh/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-1.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/SQOqJ3TkkQzNnQyf8X5k4n/201502111947-kh1nwh/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-2.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/SQOqJ3TkkQzNnQyf8X5k4n/201502111947-kh1nwh/polynomial/{z}/{x}/{y}.png" << "http://georeferencer-3.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/SQOqJ3TkkQzNnQyf8X5k4n/201502111947-kh1nwh/polynomial/{z}/{x}/{y}.png";
   bool bFound = false;
   foreach (Overlay* o, nc->overlayList())
   {
    if(o->name == ov->name)
    {
     bFound = true;
     o->urls = ov->urls;
     break;
    }
   }
   if(!bFound)
    nc->overlayList().append(ov);
  }
//        if(nc->id == currentCityId)
//            currConnection = nc->connections.at(nc->curConnectionId);
  settings.endArray();

  if(nc->overlayList().isEmpty())
   nc->bShowOverlay = false;
  nc->curOverlayId = settings.value("currOverlay").toInt();
  if(nc->overlayList().count()== 0)
   nc->curOverlayId = -1;
  cityList.append(nc);
 }
 settings.endArray();

 //settings.beginGroup("General");
 currentCityId = settings.value("currCity",0).toInt();
 if(currentCityId < 0 || currentCityId > cityList.count())
  currentCityId = 0;
 currCity = cityList.at(currentCityId);
 currConnection =   currCity->connections.at(currCity->curConnectionId);
 bDisplayWebDebug = settings.value("showDebugMessages", false).toBool();
 bRunInBrowser = settings.value("runInBrowser", false).toBool();
 //settings.endGroup();

 settings.beginGroup("query");
 q.b_stop_query_on_error = settings.value("stop_query_on_error").toBool();
 q.b_sql_execute_after_loading = settings.value("sql_execute_after_loading").toBool();
 q.s_query_path = settings.value("queryPath", QDir::homePath()).toString();
 q.geometry = settings.value("geometry").toByteArray();

 settings.endGroup();
}

void Configuration::setOverlay(Overlay* ov)
{
 for(int i=0; i < currCity->overlayList().count(); i++)
 {
  Overlay* o  =  currCity->overlayList().at(i);
  if(ov->name == o->name)
  {
   o->opacity = ov->opacity;
   currCity->overlayList().replace(i, o);
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

