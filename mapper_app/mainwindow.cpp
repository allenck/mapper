#include <QtGui>
#ifndef USE_WEBENGINE
#include <QtWebKit>
#include <QWebFrame>
#include <QWebElementCollection>
#else
#include <QWebEngineHistory>
#include "websocketclientwrapper.h"
#include "websockettransport.h"
#include <QWebEnginePage>
#include <QWebSocketServer>
#endif
#include "mainwindow.h"
#include "webviewbridge.h"
#include "sql.h"
 #include <QResizeEvent>
#include <qfile.h>
#include <qtextstream.h>
#include "dialogcopyroute.h"
#include "dialogrenameroute.h"
#include <QMessageBox>
#include "segmentdlg.h"
#include "modifyroutedatedlg.h"
//#include "splitroute.h"
#include "exportdlg.h"
#include "editconnectionsdlg.h"
#include "locatestreetdlg.h"
#include "combineroutesdlg.h"
#include "reroutingdlg.h"
#include "createsqlitedatabasedialog.h"
#include "kml.h"
#include <QFileDialog>
#include "exportroutedialog.h"
#include "editcitydialog.h"
#include "../console/systemconsoleaction.h"
#include "../console/systemconsole.h"
//#include "webviewaction.h"
//#include "webviewer.h"
#include <QStatusBar>
#include <QWidgetAction>
#include <QMenuBar>
#include "editsegmentdialog.h"
#include <QUrl>
#include <QDesktopServices>
#include "addgeoreferenceddialog.h"
#include <QFileInfo>
#include <QApplication>
#include <QProcess>
#include "sql.h"

QString mainWindow::pwd = "";
QString mainWindow::pgmDir = "";

mainWindow::mainWindow(int argc, char * argv[], QWidget *parent) :  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
 ui->setupUi(this);
 cityMenu = nullptr;
 path = argv[0];
 {
  QFileInfo info(path);
  wikiRoot = info.absolutePath();
  info = QFileInfo(wikiRoot);
  if(wikiRoot.endsWith("mapper_app"))
   wikiRoot = wikiRoot.remove("/mapper_app");
  wikiRoot = wikiRoot +  "/wiki";
 }
 QCoreApplication::setOrganizationName("ACK Software");
 QCoreApplication::setApplicationName("Mapper");
 config = Configuration::instance();
 config->getSettings();
 QFileInfo pathinfo(path);
 config->path = pathinfo.absolutePath();
 while(!config->currConnection->isOpen())
 {
  this->setWindowTitle("Mapper - "+ config->currCity->name + " ("+config->currConnection->description()+")");

  db =config->currConnection->configure();
  if(config->currConnection->isOpen())
   break;
  editConnections();
 }
 m_latitude = config->currCity->center.lat();
 m_longitude = config->currCity->center.lon();
 m_zoom = config->currCity->zoom;
 m_maptype = config->currCity->mapType;
 sql = SQL::instance();

 createBridge();  // create the webViewBridge
 zoomIndicator = new QLabel();
 statusBar()->addPermanentWidget( zoomIndicator);
 geocoderRslt= new QLabel();
 statusBar()->addPermanentWidget(geocoderRslt);

#ifndef USE_WEBENGINE
 config->bRunInBrowser = false;
#endif

 if(!config->bRunInBrowser)
 {
#ifndef USE_WEBENGINE
  webView = new QWebView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webView"));
  webView->setContextMenuPolicy(Qt::NoContextMenu);
  webView->setUrl(QUrl(QStringLiteral("qrc:/GoogleMaps.htm")));
#else
  webView = new QWebEngineView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webEngineView"));
  webView->setContextMenuPolicy(Qt::NoContextMenu);
  webView->setPage(new MyWebEnginePage());
  webView->setUrl(QUrl(QStringLiteral("qrc:/GoogleMaps2.htm")));
#endif
 }
 else
 {
  webView = NULL;
  ui->groupBox_2->setHidden(true);
  openWebWindow();
  ui->saveImage->setEnabled(false);
 }
#ifdef USE_WEBENGINE
 // setup the QWebSocketServer
 m_server = new QWebSocketServer(QStringLiteral("WebViewBridge"), QWebSocketServer::NonSecureMode);
 if (!m_server->listen(QHostAddress::LocalHost, 12345))
 {
  //qFatal("Failed to open web socket server.");
  return;
 }
 qDebug() << "listening on localhost:12345";
 // wrap WebSocket clients in QWebChannelAbstractTransport objects
 m_clientWrapper = new WebSocketClientWrapper (m_server);

 // setup the channel
 channel = new QWebChannel();
 QObject::connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,
                  channel, &QWebChannel::connectTo);
 qDebug() << "registering webViewBridge";
 channel->registerObject("webViewBridge", m_bridge);

 if(!config->bRunInBrowser)
  webView->page()->setWebChannel(channel);
 connect(m_clientWrapper, &WebSocketClientWrapper::clientClosed, this, &mainWindow::onWebSocketClosed);

#endif

 if(!config->bRunInBrowser)
  ui->verticalLayout_2->addWidget(webView);

 pwd = QDir::currentPath();
 QFileInfo info(argv[0]);
 pgmDir = info.absolutePath();
 //sql->setConfig(config);
 //ui->setupUi(this);
 webViewAction = NULL;
 systemConsoleAction = NULL;


 qDebug()<<QApplication::style();

 QUrl dataUrl("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/overlay.lst");
 m_dataCtrl = new FileDownloader(dataUrl, this);
 connect (m_dataCtrl, SIGNAL(downloaded()), this, SLOT(loadAcksoftData()));
//#ifdef WIN32
// m_overlays = new FileDownloader(QUrl("http://localhost/map_tiles/"),this);
//#else
// m_overlays = new FileDownloader(QUrl("http://localhost/tileserver.php?/tms"),this);
//#endif
// connect(m_overlays, SIGNAL(downloaded()), this, SLOT(loadOverlayData()));
 // get list of localhost's mbtiles overlays
 m_overlays = new FileDownloader(QUrl("http://localhost/map_tiles/mbtiles.php"),this);
 //m_overlays = new FileDownloader(QUrl("http://localhost/tileserver/"),this);connect(m_overlays, SIGNAL(downloaded()), this, SLOT(loadMbtilesData()));
 connect(m_overlays, SIGNAL(downloaded()), this, SLOT(loadMbtilesData()));

 createActions();
 createMenus();

// centralWidget = new MapView(this);
// setCentralWidget(centralWidget);
 m_bAddMode=false;
 b_cbSegments_TextChanged=false;
 b_cbRoutes_TextChanged=false;
 bStreetChanged=false;
 bSegmentChanged=false;
 m_SegmentId = -1;

 routeDlg=0;
 queryDlg = 0;

 QSettings settings;
 //settingsDb settings;
 restoreGeometry(settings.value("geometry").toByteArray());
 restoreState(settings.value("windowState").toByteArray());
 ui->splitter->restoreState(settings.value("splitter").toByteArray());
 ui->tblRouteView->horizontalHeader()->restoreState(settings.value("routeView").toByteArray());
 ui->tblOtherRouteView->horizontalHeader()->restoreState(settings.value("otherRouteView").toByteArray());
 ui->tblStationView->horizontalHeader()->restoreState(settings.value("stationView").toByteArray());
 ui->tblCompanyView->horizontalHeader()->restoreState(settings.value("companyView").toByteArray());
 ui->tblTractionTypes->horizontalHeader()->restoreState(settings.value("tractionTypeView").toByteArray());

 QDir resource("Resources");
 m_resourcePath =  resource.absoluteFilePath("");
 if(!resource.exists())
  resource.mkdir(m_resourcePath);

 connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(chkOneWay_Leave(bool)));
 connect(ui->saveImage, SIGNAL(clicked(bool)), this, SLOT(On_saveImage_clicked()));

 config->saveSettings();

  // Fix for problem where Google thinks webkit is on a mobile device. As a result, you can't drag the
  // map canvas or click on a map. class myWebPage fools it to thinking the browser is Chrome.
  //http://developer.qt.nokia.com/forums/viewthread/1643/P15
#ifndef USE_WEBENGINE
  webView->setPage(new myWebPage());
  // Signal is emitted before frame loads any web content:
  QObject::connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                    this, SLOT(addJSObject()));
  QUrl startURL = QUrl("qrc:/GoogleMaps.htm");
#else
  QUrl startURL = QUrl(QStringLiteral("qrc:/GoogleMaps2.htm"));
#endif

  routeView = new RouteView(this);
  connect(routeView, SIGNAL(refreshRoutes()), this, SLOT(refreshRoutes()));
  connect(routeView->model(), SIGNAL(refreshRoutes()), this, SLOT(refreshRoutes()));
  segmentView = new SegmentView(config, this);
  otherRouteView =  OtherRouteView::instance(this);
  connect(otherRouteView, SIGNAL(displayRoute(RouteData)), this, SLOT(On_displayRoute(RouteData)));
  stationView = new StationView(config, this);
  companyView = new CompanyView(config, this);
  tractionTypeView = new TractionTypeView(config, this);
  dupSegmentView = new DupSegmentView(config, this);
  connect(routeView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  connect(dupSegmentView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  ui->tabWidget->removeTab(6);

  // setup routeDlg
  routeDlg = new RouteDlg(config, this);
  //routeDlg->Configuration ( config);
  //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
  connect(routeDlg, SIGNAL(SegmentChangedEvent(qint32, qint32)),this, SLOT(segmentChanged(qint32,qint32)));
  //routeDlg->routeChanged += new routeChangedEventHandler(RouteChanged);
  connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs )), this, SLOT(RouteChanged(RouteChangedEventArgs )));
  //QMetaObject::connectSlotsByName(this);
  //connect(        ui->cbRoute, SIGNAL(currentIndexChanged(int)), SLOT(displayQuery(int)));

  connect(ui->btnDisplayRoute, SIGNAL(clicked()), this, SLOT(btnDisplayRouteClicked()));
  connect(ui->btnFirst, SIGNAL(clicked()), this, SLOT(btnFirstClicked()));
  connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(btnNextClicked()));
  connect(ui->btnPrev, SIGNAL(clicked()), this, SLOT(btnPrevClicked()));
  connect(ui->btnLast, SIGNAL(clicked()), this, SLOT(btnLastClicked()));
  connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(btnClearClicked()));
  connect(ui->btnDeletePt, SIGNAL(clicked()), this, SLOT(btnDeletePtClicked()));
  connect(ui->cbRoute, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbRouteIndexChanged(int)));
  connect(ui->cbRoute, SIGNAL(editTextChanged(QString)), this, SLOT(cbRoutesTextChanged(QString)));
  connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
  connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(textChanged(QString)), this, SLOT(txtStreetName_TextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(editingFinished()), this, SLOT(txtStreetName_Leave()));
  ui->txtSegment->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->txtSegment, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(txtSegment_customContextMenu(const QPoint&)));
  connect(ui->txtSegment, SIGNAL(textChanged(QString)), this, SLOT(txtSegment_TextChanged(QString)));
  connect(ui->txtSegment, SIGNAL(editingFinished()), this, SLOT(txtSegment_Leave()));
  connect(ui->btnSplit, SIGNAL(clicked()),this, SLOT(btnSplit_Click()));
  connect(ui->chkShowOverlay, SIGNAL(clicked(bool)),this, SLOT(chkShowOverlayChanged(bool)));
  if(!config->bRunInBrowser)
   connect(webView, SIGNAL(loadStarted()), this, SLOT(linkActivated()));
  connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(pageBack()));
  connect(ui->chkOneWay, SIGNAL(clicked(bool)), this, SLOT(chkOneWay_Leave(bool)));
  connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompanySelectionChanged(int)));
  connect(ui->sbRoute, SIGNAL(actionTriggered(int)), this,  SLOT(sbRouteTriggered(int)));
  connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbrLeave()) );
  connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(sbTracks_valueChanged(int)));

  // Context menus
  ui->cbRoute->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->cbRoute, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(cbRoute_customContextMenu( const QPoint& )));
  connect(ui->tab, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tab1CustomContextMenu(QPoint)));
  ui->cbRoute->addAction(addSegmentAct);
  ui->cbRoute->addAction(copyRouteAct);
  ui->cbRoute->addAction(displayAct);
  ui->cbRoute->addAction(modifyRouteDateAct);
  ui->cbRoute->addAction(renameRouteAct);
  ui->cbRoute->addAction(routeCommentsAct);
  ui->cbRoute->addAction(splitRouteAct);
  ui->cbRoute->addAction(updateRouteAct);
  ui->cbRoute->addAction(updateTerminalsAct);

  ui->cbSegments->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui->cbSegments->addAction(addSegmentToRouteAct);
  ui->cbSegments->addAction(deleteSegmentAct);
  ui->cbSegments->addAction(findDupSegmentsAct);
  ui->cbSegments->addAction(findDormantSegmentsAct);
  ui->cbSegments->addAction(selectSegmentAct);
  ui->cbSegments->addAction(editSegmentAct);

  connect(ui->cbSegments, SIGNAL(signalFocusOut()), this, SLOT( cbSegments_Leave()));
  connect(ui->cbRoute, SIGNAL(signalFocusOut()), this, SLOT(cbRoutes_Leave()));
  connect(companyView, SIGNAL(dataChanged()), this, SLOT(refreshCompanies()));

  //routeView = new RouteView(this);
  refreshSegmentCB();
  refreshCompanies();
  ui->cbCompany->setCurrentIndex(config->currCity->companyKey);
  refreshRoutes();
  stationView->showStations();
  ui->chkNoPan->setChecked(config->currCity->bNoPanOpt);
  for(int i = 0; i < routeList.count(); i ++ )
  {
   RouteData rd = routeList.at(i);
   if(rd.route ==config->currCity->lastRoute && rd.name == config->currCity->lastRouteName && rd.endDate.toString("yyyy/MM/dd") == config->currCity->lastRouteEndDate)
   {
    bCbRouteRefreshing = true;
    bNoDisplay = true;
    ui->cbRoute->setCurrentIndex(i);
    bNoDisplay = false;
    ui->sbRoute->setValue(i);
    bCbRouteRefreshing = false;
    break;
   }
  }
  bDisplayStationMarkers = config->currCity->bDisplayStationMarkers;
  displayStationMarkersAct->setChecked(bDisplayStationMarkers);

  tractionTypeList = sql->getTractionTypes();

  //http://developer.qt.nokia.com/forums/viewthread/1643/P15

#ifndef USE_WEBENGINE
  webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  webView->load(startURL);
  connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(on_linkClicked(QUrl)));
#endif
  ui->btnBack->setVisible(false);

  bDisplayStationMarkers = config->currCity->bDisplayStationMarkers;
  displayStationMarkersAct->setChecked(bDisplayStationMarkers);
  bDisplayTerminalMarkers = config->currCity->bDisplayTerminalMarkers;
  displayTerminalMarkersAct->setChecked(bDisplayTerminalMarkers);
  displayTerminalMarkersToggeled(bDisplayTerminalMarkers);

  displayRouteCommentsAct->setChecked(config->currCity->bDisplayRouteComments);
  geocoderRequestAct->setChecked(config->currCity->bGeocoderRequest);
  m_bridge->processScript("setGeocoderRequest", config->currCity->bGeocoderRequest?"true":"false");

  if(config->currCity->overlayList().count()> 0)
  {
   //QTimer::singleShot(10000, this, SLOT(mapInit()));
   //chkShowOverlayChanged(config->currCity->bShowOverlay);
   ui->chkShowOverlay->setChecked(config->currCity->bShowOverlay);
  }
  else
   ui->chkShowOverlay->setEnabled(false);


}
void mainWindow::createBridge()
{
 //! The object we will expose to JavaScript engine:
 m_bridge = new webViewBridge(LatLng(m_latitude, m_longitude), m_zoom, "roadmap", this);
// m_bridge->_lat = m_latitude;
// m_bridge->_lon = m_longitude;
// m_bridge->_zoom = m_zoom;
// m_bridge->maptype="roadmap";
// m_bridge->browseWindowHeight = webView->height();
// m_bridge->browseWindowWidth = webView->width();

 connect( m_bridge, SIGNAL(movePointSignal(qint32, qint32,double,double)), this, SLOT(movePoint(qint32, qint32,double,double)));
 connect(m_bridge, SIGNAL(addPointSignal(int, double, double)), this, SLOT(addPoint(int, double, double)));
 connect (m_bridge, SIGNAL(insertPointSignal(int,qint32,double,double)), this, SLOT(insertPoint(int,qint32,double,double)));
 connect(m_bridge, SIGNAL(segmentSelected(qint32,qint32)), this, SLOT(segmentSelected(qint32, qint32)));
 connect(m_bridge, SIGNAL(outputSetDebug(QString)), this, SLOT(setDebug(QString)));
 connect(m_bridge, SIGNAL(segmentStatusSignal(QString,QString)), this, SLOT(segmentStatus(QString,QString)));
 connect(m_bridge, SIGNAL(queryOverlaySignal()), this, SLOT(queryOverlay()));
}

void mainWindow::mapInit() // map initialization completed
{
 chkShowOverlayChanged(config->currCity->bShowOverlay);
//ui->chkShowOverlay->setChecked(config->currCity->bShowOverlay);

 if(config->currCity->bUserMap)
  m_bridge->processScript("setOptions");
 else
  m_bridge->processScript("setDefaultOptions");

}

Configuration* mainWindow::getConfiguration()
{
 return config;
}

void mainWindow::reloadMap()
{
#ifndef USE_WEBENGINE
 QUrl startURL = QUrl("qrc:/GoogleMaps.htm");
#else
 QUrl startURL = QUrl("qrc:/GoogleMaps2.htm");
#endif
 if(webView == NULL)
 {
#ifndef USE_WEBENGINE
  webView = new QWebView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webView"));
  webView->setContextMenuPolicy(Qt::NoContextMenu);
  webView->setUrl(QUrl(QStringLiteral("qrc:/GoogleMaps.htm")));
#else
  webView = new QWebEngineView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webEngineView"));
  webView->setContextMenuPolicy(Qt::NoContextMenu);
  webView->setPage(new MyWebEnginePage());
  webView->setUrl(QUrl(QStringLiteral("qrc:/GoogleMaps2.htm")));
#endif

 }
 webView->load(startURL);
 QVariantList objArray;
 objArray << m_latitude << m_longitude;
 m_bridge->processScript("setCenter", objArray);
 //m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 objArray.clear();
 objArray << m_zoom;
 m_bridge->processScript("setZoom", objArray);
 if(config->currCity->bUserMap)
  m_bridge->processScript("setOptions");
 else
  m_bridge->processScript("setDefaultOptions");
 objArray.clear();
 objArray << m_maptype;
 m_bridge->processScript("setMapType", objArray);
}
void mainWindow::linkActivated()
{
 ui->btnBack->setVisible(true);
}
void mainWindow::pageBack()
{
 webView->back();
#ifndef USE_WEBENGINE
 QWebHistory *hist = webView->history();
#else
 QWebEngineHistory *hist = webView->history();
#endif
 if(!hist->canGoBack())
  ui->btnBack->setVisible(false);
}

//process list of overlays served by http://acksoft.dyndns.biz
void mainWindow::loadAcksoftData()
{
 QString data;
 data = m_dataCtrl->downloadedData();
 if(data.startsWith("<!DOCTYPE HTML PUBLIC")) return;
 loadData(data, "acksoft");
}

//process list of mbtiles overlays on localhost.
void mainWindow::loadMbtilesData()
{
 QString data;
 data = m_overlays->downloadedData();
 if(data.startsWith("<!DOCTYPE HTML PUBLIC")) return;
 //if(data.startsWith("<!DOCTYPE html")) return;
 if(!data.startsWith("Exception"))
  loadData(data, "mbtiles");
}

void mainWindow::loadData(QString data, QString source)
{
 QStringList overlays = data.split('\n');

// overlaySignalMapper = new QSignalMapper(this);
// overlayActionGroup = new QActionGroup(this);
 for(int i=0; i < overlays.count(); i++)
 {
  QString ov = overlays.at(i);
  if(ov.startsWith("#"))
   continue;
  QStringList sl = ov.split("|");
  if(sl.count() < 1 || sl.at(0) == "")
   continue;

  Overlay* overlay;
  config->overlayList.insert(sl.at(0), overlay = new Overlay(sl.at(0))); // name
  bool bOk;
  overlay->description = sl.at(1);
  overlay->minZoom= sl.at(2).toInt(&bOk);
  if(!bOk)
   overlay->minZoom = 10;
  overlay->maxZoom= sl.at(3).toInt(&bOk);
  if(!bOk)
   overlay->maxZoom = 16;
  overlay->opacity= sl.at(4).toInt(&bOk);
  if(!bOk)
   overlay->opacity = 65;
  if(sl.count() > 5)
   overlay->bounds = Bounds(sl.at(5));
  if(sl.count() > 6)
   overlay->sCenter = sl.at(6);
  overlay->source = source;
  if(source == "mbtiles")
   overlay->urls.append("http://localhost/map_tiles/mbtiles.php");
  else
  if(source == "acksoft")
  {
   overlay->urls.append("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/");
//   if(!overlay->bounds.isValid())
//   {
//    QEventLoop loop;
//    m_tilemapresource = new FileDownloader("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/" + overlay->name + "/tilemapresource.xml");
//    m_tilemapresource->setOverlay(overlay);
//    connect(m_tilemapresource, SIGNAL(downloaded()), this, SLOT(processTileMapResource()));
//    loop.exec();
//   }
  }
  continue;
 }
}

void mainWindow::processTileMapResource()
{
 QString str = m_tilemapresource->downloadedData();
 if(str != "")
 {
  QDomDocument doc;
  QString title;
  Bounds bounds;
  doc.setContent(str);
  QDomElement root = doc.documentElement();
  if(root.tagName() == "TileMap")
  {
   QDomElement elem = root.firstChildElement("Title");
   if(!elem.isNull())
    title = elem.text();
   elem = root.firstChildElement("BoundingBox");
   if(!elem.isNull())
   {
    double minx, miny, maxx, maxy;
    minx = elem.attribute("minx").toDouble();
    miny = elem.attribute("miny").toDouble();
    maxx = elem.attribute("maxx").toDouble();
    maxy = elem.attribute("maxy").toDouble();
    bounds = Bounds(LatLng(miny, minx), LatLng(maxy, maxx));
    if(bounds.isValid())
    {
     Overlay* ov = m_tilemapresource->overlay();
     if(ov != nullptr)
     {
      ov->bounds = bounds;
      if(ov->description == "")
       ov->description = title;
      elem = doc.firstChildElement("TileSets");
      if(!elem.isNull())
      {
       QDomNodeList nl = elem.elementsByTagName("TileSet");
       int minZoom = 30;
       int maxZoom = 0;
       for(int i=0; i < nl.count(); i++)
       {
        bool bok;
        int zoom = nl.at(i).toElement().attribute("href").toInt(&bok);
        if(bok)
        {
         if(zoom < minZoom)
          minZoom = zoom;
         if(zoom > maxZoom)
          maxZoom = zoom;
        }
       }
       if(minZoom != 30)
        ov->minZoom = minZoom;
       if(maxZoom > 0)
        ov->maxZoom = maxZoom;
      }
      qDebug() <<"xml processed: " << ov->name << "descr: " << ov->description << " bounds: " << ov->bounds.toString() << " minZoom: " << ov->minZoom << " maxZoom: " << ov->maxZoom;
     }
    }
   }
  }
 }
}
#ifdef WIN32
void mainWindow::loadOverlayData()
{
 QString data;
 data = m_overlays->downloadedData();
 int ix = 0;
 while(ix > -1)
 {
  ix = data.indexOf("HREF=\"/map_tiles/", ix);
  if(ix > 0)
  {
   int ix2 = data.indexOf("\"", ix + 17);
   if(ix2 > 0)
   {
    QString name = data.mid(ix+17,ix2-ix-17);
    if(name.endsWith(".mbtiles"))
    {
     name = data.mid(ix+17,ix2-ix-17 - 8);
     config->localOverlayList.append(name);
     if(!config->overlayList.contains(name))
     {
      Overlay* ov = new Overlay(name);
      ov->source = "mbtiles";
      config->overlayList.insert(name, ov);
      ov->bLocal = true;
     }
     else
     {
      config->overlayList.value(name)->bLocal = true;
     }
    }
    ix = ix2;
   }
  }
 }
}
#else
void mainWindow::loadOverlayData()
{
 QString data;
 data = m_overlays->downloadedData();
 QDomDocument doc;
 doc.setContent(m_overlays->downloadedData());
 QDomElement root = doc.firstChild().toElement();
 //qDebug() << root.tagName();
 if(!root.isNull() && root.tagName() == "TileMapService")
 {
  QDomElement tileMap = root.firstChildElement("TileMaps");
  if(!tileMap.isNull())
  {
   QDomNodeList nl = tileMap.elementsByTagName("TileMap");
   for(int i = 0; i < nl.count(); i++)
   {
    QDomElement elem = nl.at(i).toElement();
    QString name;
    config->localOverlayList.append(name = elem.attribute("title"));
    if(!config->overlayList.contains(name))
    {
     Overlay* ov = new Overlay(name);
     ov->source = "mbtiles";
     ov->urls.append("http://localhost/mbtiles.php");

     ov->bLocal = true;
     config->overlayList.insert(name, ov);
    }
   }
  }
 }
}
#endif

void mainWindow::loadOverlay(Overlay* ov)
{
 currentOverlay = ov->name;
 QVariantList objArray;
 objArray << currentOverlay<< ov->opacity << ov->minZoom << ov->maxZoom << ov->source << ov->bounds.toString()<< ov->urls;
 m_bridge->processScript("loadOverlay", objArray);
}

void mainWindow::fillOverlayMenu()
{
 overlayMenu->clear();
 overlaySignalMapper = new QSignalMapper(this);
 overlayActionGroup = new QActionGroup(this);
 for(int i = 0; i < config->currCity->overlayList().count(); i++)
 {
  QString ov = config->currCity->overlayList().at(i)->name;
  if(ov == "")
      continue;
  QAction *act = new QAction(ov, this);
  act->setData(i);
  act->setCheckable(true);
  act->setToolTip(config->currCity->overlayList().at(i)->description);
  overlayActions.append(act);
  actionGroup->addAction(act);
  connect(act, SIGNAL(triggered()), overlaySignalMapper, SLOT(map()));
  overlaySignalMapper->setMapping(act, i);
  overlayMenu->addAction(act);
  //if(config->currCity->curOverlayId == i)
  if(config->currCity->curOverlayId >=0 && ov == config->currCity->overlayList().at(config->currCity->curOverlayId)->name)
   act->setChecked(true);
  //act->setToolTip(config->currCity->overlays.at(config->currCity->curOverlayId)->description);
  if(!ui->chkShowOverlay->isEnabled())
   ui->chkShowOverlay->setEnabled(true);
 }
 connect(overlaySignalMapper,SIGNAL(mapped(int)),this, SLOT(newOverlay(int)));
}

//MainWindow::~MainWindow()
//{
//    delete ui;
//}
void mainWindow::addJSObject() {
    // Add m_bridge to JavaScript Frame as member "webViewBridge".
#ifndef USE_WEBENGINE
    webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("webViewBridge"), m_bridge);
#else
 //webView->page()->setWebChannel(channel);
#endif
}
void mainWindow::NotYetInplemented()
{
    //QMessageBox::StandardButton reply;
    QMessageBox::critical(this, tr("Not Yet Implemented"),tr("This feature has not been implemented yet."), QMessageBox::NoButton);
}

void mainWindow::createActions()
{
 aboutAct = new QAction(tr("&About"), this);
 aboutAct->setStatusTip(tr("Show the application's About box"));
 connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

 quitAct = new QAction(tr("&Quit"), this);
 quitAct->setStatusTip(tr("Exit mapper"));
 connect(quitAct, SIGNAL(triggered()), this, SLOT(quit()));

 displayAct = new QAction(tr("Display route"), this);
 displayAct->setStatusTip(tr("Display the current route"));
 connect(displayAct, SIGNAL(triggered()), this, SLOT(btnDisplayRouteClicked()));

 copyRouteAct = new QAction(tr("Copy route"), this);
 copyRouteAct->setStatusTip(tr("Create a new route from this one"));
 connect(copyRouteAct, SIGNAL(triggered()), this, SLOT(copyRouteInfo_Click()));

 renameRouteAct = new QAction(tr("Rename Route"), this);
 renameRouteAct->setStatusTip(tr("Rename the current route"));
 connect(renameRouteAct, SIGNAL(triggered()), this, SLOT(renameRoute_Click()));

 modifyRouteDateAct = new QAction(tr("Modify route dates"), this);
 modifyRouteDateAct->setStatusTip(tr("Modify the begin or end date for a route"));
 connect(modifyRouteDateAct, SIGNAL(triggered()), this, SLOT(modifyRouteDate()));

 routeCommentsAct = new QAction(tr("Route Comment"), this);
 routeCommentsAct->setStatusTip(tr("Update route comment"));
 connect(routeCommentsAct, SIGNAL(triggered()), this, SLOT(updateRouteComment()));

 splitRouteAct = new QAction(tr("Split route at date"), this);
 splitRouteAct->setStatusTip(tr("Split a route into two at a specified date"));
 connect(splitRouteAct, SIGNAL(triggered()), this, SLOT(splitRoute_Click()));

 addSegmentAct = new QAction (tr("Add segment"), this);
 addSegmentAct->setStatusTip(tr("Create a new segment and add to route"));
 connect(addSegmentAct, SIGNAL(triggered()), this, SLOT(addSegment()));

 deleteRouteAct = new QAction(tr("Delete route"), this);
 deleteRouteAct->setStatusTip(tr("Delete the entire route"));
 connect(deleteRouteAct, SIGNAL(triggered()), this, SLOT(deleteRoute()));

 updateRouteAct = new QAction(tr("Update route"),this);
 updateRouteAct->setStatusTip(tr("Update route information "));
 connect(updateRouteAct, SIGNAL(triggered()), this, SLOT(updateRoute()));

 exportRouteAct = new QAction(tr("Export route"),this);
 exportRouteAct->setStatusTip(tr("Export specific route information "));
 connect(exportRouteAct, SIGNAL(triggered()), this, SLOT(exportRoute()));

 updateTerminalsAct = new QAction(tr("Update Terminals"),this);
 updateTerminalsAct->setStatusTip(tr("Update start and end terminal segements"));
 connect(updateTerminalsAct, SIGNAL(triggered()), this, SLOT(updateTerminals()));

//    displaySegmentAct = new QAction(tr("Display"),this);
//    displaySegmentAct->setStatusTip(tr("Display this segment"));
//    connect(displaySegmentAct,SIGNAL(triggered()), this, SLOT());

 deleteSegmentAct= new QAction(tr("DeleteSegment"),this);
 deleteSegmentAct->setToolTip(tr("Delete this segment if no longer used"));
 connect(deleteSegmentAct,SIGNAL(triggered()), this, SLOT(btnDeleteSegment_Click()));

 selectSegmentAct=new QAction(tr("Select segment"),this);
 selectSegmentAct->setToolTip(tr("Select and display segment"));
 connect(selectSegmentAct,SIGNAL(triggered()), this, SLOT(selectSegment()));
 editSegmentAct = new QAction("Edit Segment", this);
 connect(editSegmentAct, SIGNAL(triggered()), this, SLOT(On_editSegment_triggered()));

 findDupSegmentsAct=new QAction(tr("Display duplicate segments"),this);
 findDupSegmentsAct->setToolTip(tr("Display a list of duplicate segments"));
 connect(findDupSegmentsAct, SIGNAL(triggered()),this, SLOT(findDupSegments()));

 addSegmentToRouteAct = new QAction(tr("Add segment to route"), this);
 connect(addSegmentToRouteAct, &QAction::triggered, [=]{
     int ix = ui->cbSegments->currentIndex();
     int segmentId = ui->cbSegments->itemData(ix).toInt();
     SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId, true);

     int row =         ui->cbRoute->currentIndex();
     if(row < 0) return;
     RouteData rd = ((RouteData)routeList.at(row));
     bool b = SQL::instance()->addSegmentToRoute(rd.route, rd.name, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd"),
                                        si.segmentId, rd.companyKey,
                                        rd.tractionType, "?", -1, -1, 0, 0, 0, 0);
    if(b)
    {
        m_bridge->processScript("clearPolyline", QString("%1").arg(segmentId));
        SegmentInfo si = sql->getSegmentInfo(segmentId);
        displaySegment(segmentId, si.description, si.oneWay, /*ttColors[e.tractionType]*/getColor(rd.tractionType), true);

    }
 });

 findDormantSegmentsAct = new QAction(tr("Find dormant segments"),this);
 findDupSegmentsAct->setToolTip(tr("Display a lists of segments that are dormat, i.e. not in service"));
// TODO: find dormant segments
 connect(findDormantSegmentsAct, SIGNAL(triggered()), this, SLOT(NotYetInplemented()));

 saveChangesAct = new QAction(tr("Save changes"), this);
 saveChangesAct->setStatusTip(tr("Commit changes to database"));
 connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(saveChanges()));

 addRouteAct = new QAction(tr("Add new Route"),this);
 addRouteAct->setToolTip(tr("Add a new route"));
 connect(addRouteAct, SIGNAL(triggered()), this, SLOT(AddRoute()));

 addPointModeAct = new QAction(tr("Add point mode"), this);
 addPointModeAct->setToolTip(tr("Toggle 'add point' mode. If on, points can be added to the currenly selected segment."));
 addPointModeAct->setCheckable(true);
 addPointModeAct->setChecked(false);
 connect(addPointModeAct, SIGNAL(triggered(bool)), this, SLOT(addModeToggled(bool)));

 reloadMapAct = new QAction(tr("Reload Google Maps"), this);
 reloadMapAct->setToolTip(tr("Reload the Google Maps window"));
 connect(reloadMapAct, SIGNAL(triggered()), this, SLOT(reloadMap()));

 displayStationMarkersAct = new QAction(tr("Display station markers"),this);
 displayStationMarkersAct->setToolTip(tr("Toggle display of station markers"));
 displayStationMarkersAct->setCheckable(true);
 connect(displayStationMarkersAct, SIGNAL(toggled(bool)), this, SLOT(displayStationMarkersToggeled(bool)));

 displayTerminalMarkersAct = new QAction(tr("Display terminal markers"),this);
 displayTerminalMarkersAct->setToolTip(tr("Toggle display of terminal markers"));
 displayTerminalMarkersAct->setCheckable(true);
 connect(displayTerminalMarkersAct, SIGNAL(toggled(bool)), this, SLOT(displayTerminalMarkersToggeled(bool)));

 createKmlAct = new QAction(tr("Create Kml file"), this);
 createKmlAct->setToolTip(tr("Create Kml file for current route. The KML can then be used to display the route on Google Maps or Google Earth."));
 connect(createKmlAct, SIGNAL(triggered()), this, SLOT(on_createKmlFile_triggered()));

 displayRouteCommentsAct= new QAction(tr("Display route comments"), this);
 displayRouteCommentsAct->setToolTip(tr("Toggle display of route comments"));
 displayRouteCommentsAct->setCheckable(true);
 connect(displayRouteCommentsAct, SIGNAL(toggled(bool)), this, SLOT(displayRouteCommentsToggled(bool)));

 geocoderRequestAct = new QAction(tr("Lookup geocoder requests"),this);
 geocoderRequestAct->setToolTip(tr("toggle whether to perform geocoder requests"));
 geocoderRequestAct->setCheckable(true);
 connect(geocoderRequestAct, SIGNAL(toggled(bool)), this, SLOT(geocoderRequestToggled(bool)));

 exportDbAct = new QAction(tr("Export database"), this);
 exportDbAct->setToolTip(tr("Export to another database"));
 connect(exportDbAct, SIGNAL(triggered()), this, SLOT(exportDb()));

 editConnectionsAct = new QAction(tr("Edit Connections..."), this);
 editConnectionsAct->setToolTip(tr("Select/edit city and connection"));
 connect(editConnectionsAct, SIGNAL(triggered()),this, SLOT(editConnections()));

 editCityAct = new QAction(tr("Edit City Info"), this);
 editCityAct->setToolTip(tr("Edit city info including selecting which available overlays can be displayed."));
 connect(editCityAct, SIGNAL(triggered()), this, SLOT(On_editCityInfo()));

 locateStreetAct = new QAction(tr("Locate Geodb Object"), this);
 locateStreetAct->setToolTip(tr("Locate, a street, bridge, park or bahanhof. For Berlin only."));
 connect(locateStreetAct, SIGNAL(triggered()), this, SLOT(locateStreet()));

 combineRoutesAct = new QAction(tr("Combine two routes"), this);
 combineRoutesAct->setToolTip(tr("Combine two routes into one"));
 connect(combineRoutesAct, SIGNAL(triggered()), this, SLOT(combineRoutes()));

 refreshRoutesAct = new QAction(tr("Refresh Routes"),this);
 refreshRoutesAct->setToolTip(tr("Refresh the routes combobox. Especially after executing manual queries to the database."));
 connect(refreshRoutesAct, SIGNAL(triggered()), this, SLOT(refreshRoutes()));

 cbSort = new QComboBox();
 cbSort->setVisible(true);
 cbSort->activateWindow();
 cbSort->addItem("Route, end date");
 cbSort->addItem("Route, start date");
 cbSort->addItem("Route, name, end date");
 cbSort->addItem("Route, name, start date");
 cbSort->addItem("Name, Route, start date");

 cbSort->setCurrentIndex(config->currCity->routeSortType);
 connect(cbSort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSortSelectionChanged(int)));

 sortTypeAct = new QWidgetAction(this);
 sortTypeAct->setDefaultWidget(cbSort);

 rerouteAct = new QAction(tr("Temp rerouting"),this);
 rerouteAct->setToolTip(tr("Temporarily reroute a route between two dates. After the end date, the route will revert back to the route before the start date"));
 connect(rerouteAct, SIGNAL(triggered()), this, SLOT(rerouteRoute()));

 newSqliteDbAct = new QAction(tr("Create new Sqlite db"),this);
 newSqliteDbAct->setToolTip(tr("Create a new Sqlite3 database."));
 connect(newSqliteDbAct, SIGNAL(triggered()), this, SLOT(newSqliteDbAct_triggered()));

 queryDialogAct = new QAction(tr("SQL Query dialog"), this);
 queryDialogAct->setToolTip(tr("Open a window to make SQL queries on the database."));
 connect(queryDialogAct, SIGNAL(triggered()), this, SLOT(QueryDialogAct_triggered()));

 showDebugMessages = new QAction(tr("Show Debug messages"),this);
 showDebugMessages->setCheckable(true);
 showDebugMessages->setToolTip(tr("If checked, WebViewer debug messages will be displayed. "));
 connect(showDebugMessages, SIGNAL(toggled(bool)), this, SLOT(on_showDebugMessages(bool)));
#ifdef USE_WEBENGINE
 runInBrowserAct = new QAction(tr("Display map in browser"), this);
 runInBrowserAct->setCheckable(true);
 runInBrowserAct->setToolTip(tr("If checked, map display will be in web browser. You must then restart Mapper"));
 connect(runInBrowserAct, SIGNAL(triggered(bool)), this, SLOT(on_runInBrowser(bool)));
 runInBrowserAct->setChecked(config->bRunInBrowser);
#endif
 addGeoreferencedOverlayAct = new QAction(tr("Edit Overlay list"), this);
 addGeoreferencedOverlayAct->setToolTip(tr("Open a dialog to edit list of available overlays."));
 connect(addGeoreferencedOverlayAct, SIGNAL(triggered(bool)), this, SLOT(on_addGeoreferenced(bool)));

 overlayHelp = new QAction(tr("Overlays"),this);
 overlayHelp->setToolTip(tr("Help on setting up a new overlay."));
 connect(overlayHelp, SIGNAL(triggered(bool)), this, SLOT(on_overlayHelp()));

 usingMapper = new QAction(tr("Using Mapper"), this);
 usingMapper->setToolTip(tr("User documentation"));
 connect(usingMapper, SIGNAL(triggered(bool)), this, SLOT(on_usingHelp()));
}

void mainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAct);
    QMenu* connectionsMenu = new Menu("City");
    menuBar()->addMenu(connectionsMenu);
    cityMenu = new Menu(tr("&Cities"));
    cityMenu->setToolTip(tr("Select which city and database connection to use."));
    connectionsMenu->addMenu(cityMenu);
    connect(cityMenu, SIGNAL(aboutToShow()), this, SLOT(createCityMenu()));
    connectionsMenu->addSeparator();
    createCityMenu();
    connectionsMenu->addAction(editConnectionsAct);
    connectionsMenu->addAction(editCityAct);

    toolsMenu = new Menu(tr("Tools"));
    toolsMenu->addAction(addRouteAct);
    toolsMenu->addAction(addSegmentAct);
    toolsMenu->addAction(addPointModeAct);
    toolsMenu->addAction(createKmlAct);
    toolsMenu->addAction(exportDbAct);
    toolsMenu->addAction(locateStreetAct);
    toolsMenu->addAction(reloadMapAct);
    toolsMenu->addAction(refreshRoutesAct);
    toolsMenu->addAction(newSqliteDbAct);
    toolsMenu->addAction(queryDialogAct);
    toolsMenu->addAction(addGeoreferencedOverlayAct);

    optionsMenu = new Menu(tr("Options"));
    overlayMenu = new Menu(tr("Overlays"));
    optionsMenu->addMenu(overlayMenu);
    connect(overlayMenu, SIGNAL(aboutToShow()), this, SLOT(fillOverlayMenu()));
    optionsMenu->addAction(displayRouteCommentsAct);
    optionsMenu->addAction(displayStationMarkersAct);
    optionsMenu->addAction(displayTerminalMarkersAct);
    optionsMenu->addAction(showDebugMessages);
    optionsMenu->addAction(geocoderRequestAct);
#ifdef USE_WEBENGINE
    optionsMenu->addAction(runInBrowserAct);
#endif
    sortMenu = new Menu(tr("Route Sort option"));
    optionsMenu->addMenu(sortMenu);
    sortMenu->addAction(sortTypeAct);

    menuBar()->addMenu(optionsMenu);
    menuBar()->addMenu(toolsMenu);

    helpMenu = menuBar()->addMenu(tr("&Help"));
//    helpMenu->addAction(webViewAction = new WebViewAction((QObject*)this));
#ifndef QT_DEBUG
    helpMenu->addAction(systemConsoleAction = new SystemConsoleAction());
#endif
    helpMenu->addAction(usingMapper);
    helpMenu->addAction(overlayHelp);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);

}
void mainWindow::createCityMenu()
{
 int k = 0;
 signalMapper = new QSignalMapper(this);
 actionGroup = new QActionGroup(this);
 cityActions.clear();

 for(int i=0; i < config->cityList.count(); i++)
 {
     City* c = config->cityList.at(i);
     for(int j =0; j<c->connections.count(); j++)
     {
         Connection* cn = c->connections.at(j);
         QAction* act = new QAction(cn->description() + "...", this);
         act->setCheckable(true);
         if(c->id == config->currentCityId && cn->id() == config->currCity->curConnectionId)
             act->setChecked(true);
         cityActions.append(act);
         actionGroup->addAction(act);
         connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
         signalMapper->setMapping(act, k);
         k++;
     }

 }
 connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(newCity(int)));


 //cityMenu = menuBar()->addMenu(tr("&City"));
 if(cityMenu == NULL)
  return;
 cityMenu->clear();
 k = 0;
 for(int i= 0; i< config->cityList.count(); i++)
 {
     City* c = config->cityList.at(i);
     connectMenu = cityMenu->addMenu(c->name);
     for(int j =0; j<c->connections.count(); j++)
     {
         connectMenu->addAction((QAction*)cityActions.at(k));
         k++;
     }
 }
}

void mainWindow::On_editCityInfo()
{
 EditCityDialog* dlg = new EditCityDialog(this);
 dlg->exec();
}

void mainWindow::cbRoute_customContextMenu( const QPoint& )
{
    cbRouteMenu.clear();
    cbRouteMenu.addAction(addSegmentAct);
    cbRouteMenu.addAction(combineRoutesAct);
    cbRouteMenu.addAction(copyRouteAct);
    cbRouteMenu.addAction(deleteRouteAct);
    cbRouteMenu.addAction(displayAct);
    cbRouteMenu.addAction(modifyRouteDateAct);
    cbRouteMenu.addAction(renameRouteAct);
    cbRouteMenu.addAction(rerouteAct);
    cbRouteMenu.addAction(routeCommentsAct);
    cbRouteMenu.addAction(splitRouteAct);
    cbRouteMenu.addAction(updateRouteAct);
    cbRouteMenu.addAction(exportRouteAct);
    cbRouteMenu.addAction(updateTerminalsAct);
    updateTerminalsAct->setEnabled(routeView->isSequenced());
    cbRouteMenu.exec(QCursor::pos());
}

void mainWindow::txtSegment_customContextMenu(const QPoint &)
{
 if(ui->txtSegment->text().isEmpty()) return;
 QMenu* menu = new QMenu();
 QAction* edit = new QAction(tr("Edit segment"), this);
 menu->addAction(edit);
 connect(edit, SIGNAL(triggered(bool)), this, SLOT(On_editSegment_triggered()));
 menu->addAction(updateRouteAct);
 menu->exec(QCursor::pos());

}

void mainWindow::tab1CustomContextMenu(const QPoint &)
{
    tab1Menu.addAction(saveChangesAct);
    if(!routeView->bUncomittedChanges())
       saveChangesAct->setEnabled(false);
    else
       saveChangesAct->setEnabled(true);
    tab1Menu.exec(QCursor::pos());
}

// New City and/or connection selected.
void mainWindow::newCity(int ix )
{
 qDebug() << "newCity " + QString("%1").arg(ix);
 int i=0, j=0, k = 0;
 for( i= 0; i< config->cityList.count(); i++)
 {
  City* c = config->cityList.at(i);
  //connectMenu = cityMenu->addMenu(c.name);
  for( j =0; j<c->connections.count(); j++)
  {
   Connection* cn = c->connections.at(j);
   if(k==ix)
   {
    this->setCursor(QCursor(Qt::WaitCursor));
    // first, save some settings for the current city
    config->currCity->center = LatLng(m_latitude, m_longitude);
    config->currCity->zoom = m_zoom;
    config->currCity->mapType = m_maptype;
    config->currCity->connections.replace(config->currCity->curConnectionId, config->currConnection);
    // Save any changes to currentCity
    config->cityList.replace(config->currentCityId, config->currCity);
    companyView->clear();
    tractionTypeView->clear();;

    // load the new city and configuration
    config->currCity = c;
    config->currentCityId = i;
    config->currCity->curConnectionId = j;
    config->currConnection = cn;
    config->currCity->lastRoute = m_routeNbr;
    config->currCity->lastRouteName = m_routeName;
    config->currCity->lastRouteEndDate = m_currRouteEndDate;
    config->currCity->bDisplayStationMarkers = bDisplayStationMarkers;
    config->currCity->bDisplayRouteComments = bDisplayRouteComments;
    config->currCity->bShowOverlay = ui->chkShowOverlay->isChecked();

    qDebug() << c->name + "/" + cn->description();
    // close the current database and open the new one
    db.close();
#if 0
    db = QSqlDatabase::addDatabase(config->currConnection->driver());

    if(config->currConnection->hostName != "")
           db.setHostName(config->currConnection->hostName);
    if(config->currConnection->port > 0)
       db.setPort(config->currConnection->port);

    if(config->currConnection->driver == "QODBC" || config->currConnection->driver == "QODBC3")
        db.setDatabaseName(config->currConnection->DSN);
    else
    {
     QString dbName = config->currConnection->database;
     QFileInfo info(dbName);
     if(!info.isAbsolute())
     {
      if(!dbName.startsWith("Resources/databases/"))
      dbName = "Resources/databases/" + dbName;
     }
     db.setDatabaseName(dbName);
    }
    //db.setDatabaseName("SQLSERVER_SAMPLE");
    db.setUserName(config->currConnection->UID);
    db.setPassword(config->currConnection->PWD);
    bool ok = db.open();
    if(!ok)
    {
     QSqlError err = db.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << db.driverName() + "\n";
     qDebug() << "User:" + db.userName() + "\n";
     qDebug() << "Host:" + db.hostName() + "\n";
     qDebug() << "Connection name: " + db.connectionName() + "\n";
     qDebug() << "DSN:" + db.databaseName() + "\n";
    }
    else
    {
     if(config->currConnection->driver == "QODBC" || config->currConnection->driver == "QODBC3")
     {
      if(config->currConnection->useDatabase != "default" && config->currConnection->useDatabase != "")
      {
       QSqlQuery query = QSqlQuery(db);
       if(!query.exec(tr("use [%1]").arg(config->currConnection->useDatabase)))
       {
        SQLERROR(query);
        db.close();
        return;
       }
      }
     }
    }
    sql->loadSqlite3Functions();
    if(ok)
    {
     sql->checkTables(db);
    }
#endif
    db = config->currConnection->configure();
    emit newCitySelected();

    this->setWindowTitle("Mapper - "+ config->currCity->name + " ("+config->currConnection->description()+")");
    config->saveSettings();

    refreshSegmentCB();
    ui->cbCompany->setCurrentIndex(config->currCity->companyKey);
    refreshCompanies();
    cbSort->setCurrentIndex(config->currCity->routeSortType);
    refreshRoutes();
    stationView->showStations();
    m_latitude = config->currCity->center.lat();
    m_longitude = config->currCity->center.lon();
    m_zoom = config->currCity->zoom;
    m_maptype = config->currCity->mapType;
    QVariantList objArray;
    objArray << m_latitude << m_longitude;
    m_bridge->processScript("setCenter", objArray);
    //m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
    objArray.clear();
    objArray << m_zoom;
    m_bridge->processScript("setZoom", objArray);
    if(config->currCity->bUserMap)
     m_bridge->processScript("setOptions");
    else
     m_bridge->processScript("setDefaultOptions");
    objArray.clear();
    objArray << m_maptype;
    m_bridge->processScript("setMapType", objArray);
    this->setCursor(QCursor(Qt::ArrowCursor));
    bDisplayStationMarkers = config->currCity->bDisplayStationMarkers;
    displayStationMarkersAct->setChecked(bDisplayStationMarkers);
    bDisplayTerminalMarkers = config->currCity->bDisplayTerminalMarkers;
    displayTerminalMarkersAct->setChecked(bDisplayTerminalMarkers);
    bDisplayRouteComments = config->currCity->bDisplayRouteComments;
    displayRouteCommentsAct->setChecked(bDisplayRouteComments);
    geocoderRequestAct->setChecked(config->currCity->bGeocoderRequest);
    m_bridge->processScript("setGeocoderRequest", config->currCity->bGeocoderRequest?"true":"false");

    for(int i=0; i< routeList.count(); i++)
    {
     RouteData rd = routeList.at(i);
     if(rd.route ==config->currCity->lastRoute && rd.name == config->currCity->lastRouteName && rd.endDate.toString("yyyy/MM/dd") == config->currCity->lastRouteEndDate)
     {
      ui->cbRoute->setCurrentIndex(i);
      break;
     }
    }
    companyView = new CompanyView(config, this);
    tractionTypeView = new TractionTypeView(config, this);
    return;
   }
   k++;
  }
 }
}
void mainWindow::newOverlay(int ix)
{
 Overlay* cOv = config->currCity->overlayList().at(ix);
 if(cOv->source == "")
 {
  if(config->localOverlayList.contains( cOv->name))
  {
#ifdef WIN32
   cOv->source = "mbtiles";
#else
   cOv->source = "tileserver";
#endif
  }
  else cOv->source = "acksoft";
 }

// currentOverlay = cOv->name;
// QVariantList objArray;
// objArray << currentOverlay<< cOv->opacity << cOv->minZoom << cOv->maxZoom << cOv->source << cOv->urls;
// m_bridge->processScript("loadOverlay", objArray);
 loadOverlay(cOv);
 bool bFound=false;
 for(int i = 0; i < config->currCity->overlayList().count(); i++)
 {
  Overlay* ov = config->currCity->overlayList().at(i);
  if(currentOverlay == ov->name)
  {
   bFound = true;
   config->currCity->curOverlayId = i;
   break;
  }
 }
// if(!bFound )
// {
//  Overlay* newOverlay = new ;
//  newOverlay->id = config->currCity->overlays.count();
//  newOverlay->name= currentOverlay;
//  newOverlay->opacity = 65;
//  config->currCity->overlays.append(newOverlay);
//  config->currCity->curOverlayId= newOverlay->id;
// }
// ui->chkShowOverlay->setEnabled(true);
// ui->chkShowOverlay->setChecked(true);
}

void mainWindow::about()
{
 QMessageBox::about(this, tr("About Mapper"),
     tr("The <b>Mapper</b> Written by Allen C. Kempe. "
        "maintain database of streetcar and transit routes.\r\nCompiled with QT version %1").arg(QT_VERSION_STR));
}

void mainWindow::btnDeleteSegment_Click()   //SLOT
{
    //SQL sql;
    SegmentInfo sI;
    int ix = ui->cbSegments->currentIndex();
    //            sI = (segmentInfo)segmentInfoList[ix];
    sI = (SegmentInfo)cbSegmentInfoList.at(ix);
    if (sI.segmentId < 1)
    {
        //System.Media.SystemSounds.Beep.Play();
        QApplication::beep();
        //infoPanel.ForeColor = Color.Red;
        //infoPanel.Text = "select an item!";
        statusBar()->showMessage(tr("select an item"));
    }
    else
    {
        // Get all the segments intersecting both ends using the same point
        updateIntersection(0, sI.startLat, sI.startLon);
        updateIntersection(0, sI.endLat, sI.endLon);
        SegmentInfo siDup = sql->getSegmentInSameDirection(sI);

        // Get a list of all routes using this segment.
        QList<RouteData> segmentInfoList = sql->getRouteSegmentsBySegment(sI.segmentId);
        if ( segmentInfoList.count() > 0)
        {
            if ( siDup.segmentId >= 0)
            {
//                DialogResult rslt = MessageBox.Show(segmentInfoList.Count + " routes are using this segment. A duplicate segment is defined.\n Move these routes to that segment?",
//                    "Segment in use", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Hand);
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Segment in use"));
                msgBox.setInformativeText(QString("%1").arg(segmentInfoList.count()) + tr(" routes are using this segment. A duplicate segment is defined.\n Move these routes to that segment?"));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::No);
                int rslt = msgBox.exec();
                switch (rslt)
                {
                    case QMessageBox::Yes:
                        //foreach (routeData rd in segmentInfoList)
                    for(int i =0; i< segmentInfoList.count(); i++)
                        {
                        RouteData rd = (RouteData)segmentInfoList.at(i);
                        if (sql->deleteRouteSegment(rd.route, rd.name, rd.lineKey, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")) != true)
                            {
                                //infoPanel.Text = "Delete Error";
                                statusBar()->showMessage(tr("Delete failed"));
                                //infoPanel.ForeColor = Color.Red;
                                //System.Media.SystemSounds.Beep.Play();
                                QApplication::beep();
                                return;
                            }
                        if (sql->doesRouteSegmentExist(rd.route, rd.name, siDup.segmentId, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")))
                                continue;
                            if (!sql->addSegmentToRoute(rd.route, rd.name, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd"), siDup.segmentId, rd.companyKey, rd.tractionType, rd.direction, rd.normalEnter, rd.normalLeave, rd.reverseEnter, rd.reverseLeave))
                            {
                                //infoPanel.Text = "Update Error";
                                statusBar()->showMessage(tr("Update failed"));
                                //infoPanel.ForeColor = Color.Red;
                                //System.Media.SystemSounds.Beep.Play();
                                QApplication::beep();
                                return;
                            }
                        }
                        break;
                    case QMessageBox::No:
                        //QMessageBox::StandardButton reply;
                        //MessageBox.Show("The segment cannot be deleted until the routes using it are removed.", "Error");
                        QMessageBox::warning(this, tr("Error"), tr("The segment cannot be deleted until the routes using it are removed."), QMessageBox::NoButton);
                        return;

                    case QMessageBox::Cancel:
                        return;
                    default:
                        break;
                }
            }
            else
            {
                //QMessageBox::StandardButton reply;
                //MessageBox.Show("The segment cannot be deleted until the routes using it are removed.", "Error");
                QMessageBox::warning(this, tr("Error"), tr("The segment cannot be deleted until the routes using it are removed."), QMessageBox::NoButton);
                return;
            }
        }
        QMessageBox::StandardButton rslt;
        rslt = QMessageBox::warning(this, tr("Delete segment"), tr("Are you sure that you want to permanently delete this segment?"), QMessageBox::Yes | QMessageBox::No);
        switch (rslt)
        {
            case QMessageBox::No:
                return;
            case QMessageBox::Yes:
                break;
            default:
                return;
        }

        if (sql->deleteSegment(sI.segmentId))
        {
            refreshSegmentCB();
            // Display the new segment in Google Maps
//            Object[] objArray = new Object[1];
//            objArray[0] = sI.SegmentId;
//            webBrowser1.Document.InvokeScript("clearPolyline"); // clears the old line
            QVariantList objArray;
            objArray<<sI.segmentId;
            m_bridge->processScript("clearPolyline", objArray);
            //cbSegments.SelectedItem = 0;

        }
        else
        {
            //System.Media.SystemSounds.Beep.Play();
            QApplication::beep();
            //infoPanel.ForeColor = Color.Red;
            //infoPanel.Text = "deleteSegment failed";
            statusBar()->showMessage("deleteSegment failed",2000);
        }
    }
}




void mainWindow::refreshRoutes()
{
    //SQL sql;
    bCbRouteRefreshing = true;
    int ixCompany = ui->cbCompany->currentIndex();
    int companyKey = 0;
    if(ixCompany > 0)
    {
        //companyKey = ui->cbCompany->itemData(ixCompany).Int;
        companyKey = companyList.at(ixCompany-1).companyKey;

    }
    int ix = ui->cbRoute->currentIndex();
    QString currText = ui->cbRoute->currentText();
    ui->cbRoute->clear();

    routeList = sql->getRoutesByEndDate(companyKey);

    int len = routeList.count();
    for(int i=0; i<len; i++)
    {
         RouteData rd = (RouteData)routeList.at(i);
         QString rdStartDate = rd.startDate.toString("yyyy/MM/dd");
                 ui->cbRoute->addItem(rd.toString());
         if( rd.toString() == currText)
                     ui->cbRoute->setCurrentIndex(i);
    }
    ui->sbRoute->setRange(0, routeList.count());

    if( ui->cbRoute->currentIndex() <= 0 && ix >=0)
        ui->cbRoute->setCurrentIndex(ix);
    bCbRouteRefreshing = false;
}
void mainWindow::txtRouteNbrLeave()
{
    QString txt = ui->txtRouteNbr->text();
    QString txtAlpha = "";
    bool bIsAlpha = false;
    int ixCompany = ui->cbCompany->currentIndex();
    int companyKey = 0;
    if(ixCompany > 0)
    {
        //companyKey = ui->cbCompany->itemData(ixCompany).Int;
        companyKey = companyList.at(ixCompany-1).companyKey;

    }
    qint32 newRoute = sql->getNumericRoute(txt, &txtAlpha, &bIsAlpha, companyKey);
    routeList = sql->getRoutesByEndDate(companyKey);
    for(int i = routeList.count()-1; i >= 0; i--)
    {
        RouteData rd = (RouteData)routeList.at(i);
        if(rd.baseRoute != newRoute)
            routeList.removeAt(i);
    }
    int len = routeList.count();
    ui->cbRoute->clear();

    for(int i=0; i<len; i++)
    {
         RouteData rd = (RouteData)routeList.at(i);
         QString rdStartDate = rd.startDate.toString("yyyy/MM/dd");
                 ui->cbRoute->addItem(rd.toString());
    }
    ui->sbRoute->setRange(0, routeList.count());

    bCbRouteRefreshing = false;
}

void mainWindow::quit()
{
    this->close();
}
QString mainWindow::getColor(qint32 tractionType)
{
 QString color = "#DF01D7";
 //foreach (tractionTypeInfo tti in tractionTypeList)
 for(int i=0; i < tractionTypeList.count(); i++)
 {
  tractionTypeInfo tti = tractionTypeList.at(i);
  if (tractionType == tti.tractionType)
      return tti.displayColor;
 }
 return color;
}
void mainWindow::btnClearClicked()
{
    m_bridge->processScript("clearAll", "");
}

void mainWindow::on_createKmlFile_triggered()
{
 int row =         ui->cbRoute->currentIndex();
 RouteData rd = ((RouteData)routeList.at(row));
 RouteInfo ri = sql->getRoutePoints(rd.route,rd.name, ui->dateEdit->text());
 Kml* kml = new Kml(ri );
 QString fileName = QFileDialog::getOpenFileName(this,"Create Kml file", QDir::homePath(),"Kml files (*.kml");
 if(!fileName.isEmpty())
 kml->createKml(fileName, "ff0000ff");
}

void mainWindow::btnDisplayRouteClicked()
{
 int row =         ui->cbRoute->currentIndex();
 if(row < 0) return;
 RouteData rd = ((RouteData)routeList.at(row));

 On_displayRoute(rd);
}

void mainWindow::On_displayRoute(RouteData rd)
{
 if(!ui->chkNoClear->isChecked())
  btnClearClicked();

 RouteInfo ri = sql->getRoutePoints(rd.route,rd.name, ui->dateEdit->text());

// LatLng startPt =  LatLng();
// LatLng endPt =  LatLng();
// LatLng swPt = LatLng(90,180);
// LatLng nePt = LatLng(-90,-180);
 Bounds bounds = Bounds();
 bool bBoundsValid = false;
 double infoLat=0, infoLon = 0;
 bool bFirst = true;

 QVariantList objArray;
 if (ri.route < 1)
  return; // no data
    //string str = (m_routeNbr<10?"0":"")+ m_routeNbr;
 m_alphaRoute = sql->getAlphaRoute(m_routeNbr, rd.companyKey);
 if(bDisplayTerminalMarkers)
 {
  TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, m_currRouteEndDate);
  if (ti.route >= 1 && ti.startLatLng.lat() > 0 && ti.startLatLng.lon() )
  {
   objArray <<ti.startLatLng.lat() <<ti.startLatLng.lon() << getRouteMarkerImagePath(m_alphaRoute, true);
   m_bridge->processScript("addRouteStartMarker", objArray);
   infoLat = ti.startLatLng.lat();
   infoLon = ti.startLatLng.lon();
   bFirst = false;

   objArray.clear();
   objArray << ti.endLatLng.lat() << ti.endLatLng.lon()<<getRouteMarkerImagePath(m_alphaRoute, false);
   m_bridge->processScript("addRouteEndMarker", objArray);
  }
 }
 //foreach (segmentGroup sg in ri.segments)
 for(int i = 0; i< ri.segments.count(); i++)
 {
  SegmentInfo si = ri.segments.at(i);

  objArray.clear();
  objArray << si.segmentId;
  m_bridge->processScript("clearPolyline", objArray);
  QString color = getColor(si.tractionType);

  QVariantList points;
  for(int i=0; i < si.pointList.count(); i++)
  {
   LatLng pt = si.pointList.at(i);
   points.append(pt.lat());
   points.append(pt.lon());
  }
  bBoundsValid = bounds.updateBounds(si.bounds);

  int dash = 0;
  if(si.routeType == Incline)
   dash = 1;
  else if(si.routeType == SurfacePRW)
   dash = 2;
  else if(si.routeType == Subway)
   dash = 3;
  objArray.clear();
  objArray <<   si.segmentId<< ri.routeName <<  si.description << si.oneWay << color << si.tracks << dash << points.count();
  objArray.append(points);
  m_bridge->processScript("createSegment",objArray);

  statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(ri.length).arg(ri.length*0.621371192));

 }

 QString markerType = "green";
 QList< tractionTypeInfo> tractionTypes= sql->getTractionTypes();
 foreach(tractionTypeInfo tti,tractionTypeList)
 {
  //tractionTypeInfo tti = (tractionTypeInfo)tractionTypeList.at(i);
  if (tti.tractionType == ri.tractionType)
  {
   switch (tti.routeType)
   {
   case RapidTransit:
    if (tti.icon != "")
    {
     markerType = tti.icon;
    }
    else
     markerType = "green";
    break;
   case Subway:
    if (tti.icon != "")
    {
     markerType = tti.icon;
    }
    else
     markerType = "blue";
    break;
case Surface:
    if (tti.icon != "")
    {
     markerType = tti.icon;
    }
    else
     markerType = "red";
    break;
default:
    markerType = "red";
    break;
   }
  }
 }

 m_bridge->executeScript("removeStationMarkers", "");
 QList<StationInfo> stationList = sql->getStations(m_routeNbr, m_routeName, m_currRouteEndDate);
 if (!stationList.isEmpty())
 {
  //foreach (stationInfo sti in stationList)
  for(int i=0; i < stationList.count(); i ++)
  {
   StationInfo sti= stationList.at(i);
   if(sti.markerType == "")
    sti.markerType= markerType;
   QString str = sti.stationName;
   if (sti.infoKey > 0)
   {
    commentInfo ci = sql->getComments(sti.infoKey);
    //str = ci.comments;

    objArray.clear();
    objArray << sti.latitude << sti.longitude << (bDisplayStationMarkers?true:false) << sti.segmentId << sti.stationName << sti.stationKey << sti.infoKey << ci.comments << sti.markerType;
     m_bridge->processScript("addStationMarker", objArray);
    }
    else
    {
     objArray.clear();
     objArray << sti.latitude << sti.longitude << (bDisplayStationMarkers?true:false) << sti.segmentId << sti.stationName << sti.stationKey << sti.infoKey << "" << sti.markerType;
     m_bridge->processScript("addStationMarker", objArray);
    }
   }
   stationView->showStations();
  }

  if(!ui->chkNoPan->checkState() && bBoundsValid)
  {
   objArray.clear();
   objArray << bounds.swPt().lat() << bounds.swPt().lon() << bounds.nePt().lat() << bounds.nePt().lon();
   m_bridge->processScript("fitMapBounds", objArray);
  }
  if(!(infoLat == 0 && infoLon ==0))
  {
   QDate dt = QDate::fromString(m_currRouteStartDate, "yyyy/MM/dd");
   routeComments rc = sql->getRouteComment(m_routeNbr, dt, -1);
   if(rc.infoKey < 0)
   {
    rc = sql->getRouteComment(0, dt, -1);
   }
   if(rc.route >= 0 && rc.ci.comments != "")
   {
//       if(rc.ci.comments == "")
//           rc.ci.comments = "<body></body><";
    int i = rc.ci.comments.indexOf("</body>");
    if(i > 0)
    {
     rc.ci.comments.insert(i,"<input type='button' name='prev' value='<' onClick='prevRouteComment()'/><input type='button' name='next' value='>' onClick='nextRouteComment()'/>");
    }
    objArray.clear();
    objArray << infoLat <<infoLon << rc.ci.comments << rc.route << rc.date.toString("yyyy/MM/dd");
    if(bDisplayRouteComments)
    {
        m_bridge->processScript("displayRouteInfo", objArray);
        m_bridge->processScript("showRouteInfo", bDisplayRouteComments?"true": "false");
    }
   }
  }
}

void mainWindow::getInfoWindowComments(double lat, double lon, int route, QString date, int func)
{
 QDate dt = QDate::fromString(date, "yyyy/MM/dd");
 routeComments rc;

 if(func < 0)
 {
  rc = sql->getPrevRouteComment(route, dt, -1);
  if(rc.infoKey < 0)
  {
   rc = sql->getPrevRouteComment(0, dt, -1);
  }
 }
 else
 {
  rc = sql->getNextRouteComment(route, dt, -1);
  if(rc.infoKey < 0)
  {
   rc = sql->getNextRouteComment(0, dt, -1);
  }
 }
 if(rc.route >= 0)
 {
  int i = rc.ci.comments.indexOf("</body>");
  if(i > 0)
  {
   routeComments rcNext = sql->getNextRouteComment(route, rc.date, -1);
   QString sNext;
   if(rcNext.infoKey>=0)
    sNext = "<input type='button' name='next' value='>' onClick='nextRouteComment()'/>";
   routeComments rcPrev = sql->getPrevRouteComment(route, rc.date, -1);
   QString sPrev;
   if(rcPrev.infoKey >= 0)
     sPrev="<input type='button' name='prev' value='<' onClick='prevRouteComment()'/>";
   rc.ci.comments.insert(i,sPrev+sNext);
  }
  QVariantList objArray;

  objArray.clear();
  objArray << lat <<lon<< rc.ci.comments<<rc.route<< rc.date.toString("yyyy/MM/dd");
  m_bridge->processScript("displayRouteInfo", objArray);
  m_bridge->processScript("showRouteInfo", bDisplayRouteComments?"true": "false");
 }
}

void mainWindow::onCbRouteIndexChanged(int row)
{
 routeView->checkChanges();
 if(routeView->bUncomittedChanges())
 {
  return;
 }
 //SQL sql;
 if(row<0)
     return;
 _rd = ((RouteData)routeList.at(row));
 if(!bCbRouteRefreshing)
 {
  bNoDisplay = true;
  ui->sbRoute->setSliderPosition(row);
  bNoDisplay = false;
 }
 ui->txtRouteNbr->setText(sql->getAlphaRoute(_rd.route,_rd.companyKey));
 ui->dateEdit->setDate( _rd.endDate);
 m_currRouteStartDate = _rd.startDate.toString("yyyy/MM/dd");
 m_currRouteEndDate = _rd.endDate.toString("yyyy/MM/dd");
 m_routeNbr = _rd.route;
 m_routeName = _rd.name;
 routeDlg->setRouteData(_rd);

 routeView->updateRouteView();
}

//void MainWindow::onResize()
//{
//   m_bridge->processScript("resizeMap", "");
//}
bool compareSegmentInfoByName(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description < s2.description;
}

//void MainWindow::resizeEvent(QResizeEvent *event)
// {
//     if (width() > image.width() || height() > image.height()) {
//         int newWidth = qMax(width() + 128, image.width());
//         int newHeight = qMax(height() + 128, image.height());
//         resizeImage(&image, QSize(newWidth, newHeight));
//         update();
//     }
//     m_bridge->processScript("resizeMap", "");

 //    QWidget::resizeEvent(event);
 //}
void mainWindow::refreshSegmentCB()
{
    bRefreshingSegments = true;
    ui->cbSegments->clear();
    cbSegmentInfoList = sql->getSegmentInfo();
    qSort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfoByName);
    //foreach (segmentInfo sI in cbSegmentInfoList)
    for(int i=0; i < cbSegmentInfoList.count(); i++)
    {
     SegmentInfo sI = ((SegmentInfo)cbSegmentInfoList.at(i));
     ui->cbSegments->addItem(sI.ToString(), sI.segmentId);
    }
    m_bridge->processScript("addModeOff");
    addPointModeAct->setChecked(false);
    bRefreshingSegments = false;
}

void mainWindow::refreshCompanies()
{
    ui->cbCompany->clear();
    ui->cbCompany->addItem(tr("All companies"),-1);
    companyList = sql->getCompanies();
    for(int i=0; i < companyList.count(); i++)
    {
        CompanyData cd = companyList.at(i);
        ui->cbCompany->addItem(cd.name, cd.companyKey);
    }
    if(routeDlg != NULL)
     routeDlg->fillCompanies();
}
void mainWindow::cbCompanySelectionChanged(int sel)
{
    Q_UNUSED (sel);
    //qint32 companyKey = ui->cbCompany->itemData(sel).Int;
    refreshRoutes();
}

/// <summary>
/// Called by Google Maps when selecting a segment
/// </summary>
/// <param name="SegmentId"></param>
void mainWindow::segmentSelected(qint32 pt, qint32 SegmentId)
{
 //SQL sql;
 //webBrowser1.Document.InvokeScript("addModeOff");
 //m_bridge->processScript("addModeOff");
 m_bAddMode = false;
 addPointModeAct->setChecked(false);
 m_SegmentId = SegmentId;
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
 if (si.segmentId == -1)
 {
  qDebug() <<"segment " + QString("%1").arg(SegmentId) + " not found";
  return;
 }
 if(routeDlg)
 {
  routeDlg->setSegmentId(m_SegmentId);
  if (ui->cbRoute->currentIndex() != -1)
   routeDlg->setRouteData ( (RouteData)routeList.at(        ui->cbRoute->currentIndex()));
 }
 //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
 lookupStreetName(si);
 //txtSegment.Text = si.description;
 ui->txtSegment->setText(si.description);
 ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(si.pointList.count()));
 ui->cbSegments->findText(si.ToString(), Qt::MatchExactly);
 //txtOneWay.Text = si.oneWay;
 if(si.oneWay == "Y")
     ui->chkOneWay->setChecked(true);
 else
     ui->chkOneWay->setChecked(false);
 //getArray();
 m_points = si.pointList;
 m_nbrPoints = m_points.size();

 if (m_nbrPoints <= 0)
     return;
 if(pt < 0 || pt >= m_nbrPoints)
 {
  qDebug() << "pt invalid: " << QString::number(pt) << " array has " << QString::number(m_nbrPoints);
  m_currPoint = 0;
 }
 else
  m_currPoint = pt;
 ui->lblPoint->setText(QString::number(m_currPoint));
 m_latitude = ((LatLng)m_points.at(pt)).lat();
 m_longitude= ((LatLng)m_points.at(pt)).lon();
 //btnAddMarker.Enabled = true;
 ui->btnFirst->setEnabled(true);
 //btnPlus.Enabled = true;
 ui->btnNext->setEnabled(true);
 //btnMinus.Enabled = true;
 ui->btnPrev->setEnabled(true);
 //btnDelete.Enabled = true;
 ui->btnDeletePt->setEnabled(true);
 //btnEnd.Enabled = true;
 ui->btnLast->setEnabled(true);
 if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
 {
  ui->btnSplit->setEnabled(true);
 }
 else
 {
  ui->btnSplit->setDisabled(true);
 }

 if(config->currCity->bGeocoderRequest)
     m_bridge->processScript("geocoderRequest", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 //m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));

 qint32 ix =         ui->cbRoute->currentIndex();
 if (ix >= 0)
 {
  RouteData rd = RouteData();
  rd = (RouteData)routeList.at(ix);
  if(routeDlg)
   routeDlg->setRouteData(rd);

  otherRouteView->showRoutesUsingSegment(SegmentId);
 }
 if (!ui->chkNoPan->checkState())
 {
  m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 }
}

void mainWindow::SetPoint(qint32 i, double lat, double lon)
{
    LatLng pt = LatLng(lat, lon);
    if(i >= m_points.count())
        m_points.append(pt);
    m_nbrPoints = m_points.count();
}

//void mainWindow::setLat(double lat)
//{
// m_latitude = lat;
//}

//void mainWindow::setLon(double lon)
//{
// m_longitude = lon;
//}

void mainWindow::getArray()
{
    m_bridge->processScript("getPointArray");
    while(!m_bridge->isResultReceived())
    {
     qApp->processEvents(QEventLoop::AllEvents, 5);
    }
    QVariantList points;
    points = m_bridge->myList;
    m_nbrPoints = points.count()/2;
    m_points = QList<LatLng>();
    for(int i = 0; i < m_nbrPoints; i++)
    {
        m_points.append(LatLng(points[i*2].toDouble(), points[(i*2)+1].toDouble()));
    }

#if 0
    //m_nbrPoints = Convert.ToInt32(webBrowser1.Document.InvokeScript("setLen"));
   m_bridge->processScript("getLen");
   qint32 nbrPoints = (qint32)m_bridge->myRslt.toInt();

    if (m_nbrPoints == -1)
        return;
   // m_points = new double[(int)m_nbrPoints * 2];
    m_points = QList<LatLng>();
    for (int i = 0; i < nbrPoints; i++)
    {
//        Object[] objArray = new Object[1];
//        objArray[0] = i.ToString();
//        webBrowser1.Document.InvokeScript("setPointValues", objArray);
        m_bridge->processScript("getPointValues",QString("%1").arg(i) );
//        m_points[i * 2] = m_latitude;
//        m_points[(i * 2) + 1] = m_longitude;
        QStringList list;
        //list = m_bridge->getRslt().split(",");
        list = m_bridge->myRslt.toString().split(",");
        /*
                    //window.external.SetDebug("Set point " + i);
        //OK          window.external.SetPoint(i, element.lat, element.lon);
                    alert("getPointValues: " + i + " " + element.lat + " " + element.lon );
                    webViewBridge.setPoint(i, element.lat, element.lon);
        //OK          window.external.setLat(element.lat());
                    webViewBridge.setLat(element.lat());
        //OK          window.external.setLon(element.lng());
                    webViewBridge.setLon(element.lon()); */
        qint32 pi = list.at(0).toInt();
        m_latitude = list.at(1).toDouble();
        m_longitude = list.at(2).toDouble();
        SetPoint(pi, m_latitude, m_longitude);

        //LatLng pt = LatLng(m_latitude, m_longitude);
        //m_points.append(pt);
    }
#endif
    if (m_currPoint > ((int)m_nbrPoints - 1))
        m_currPoint = (int)m_nbrPoints - 1;
    ui->lblPoint->setText(QString::number(m_currPoint));


}

void mainWindow::setDebug(QString str)
{
 //infoPanel.Text = str;
 if(bDisplayWebDebug)
 {
  qDebug() << str;
  statusBar()->showMessage(str, 2000);
 }
}
void mainWindow::setLen(qint32 len)
{
    m_nbrPoints = len;
}
void mainWindow::btnFirstClicked()
{
    //SQL sql;
    getArray();
//    Object[] objArray = new Object[6];
//    objArray[0] = m_currPoint = 0;
    ui->btnSplit->setEnabled(false);
//    objArray[1] = m_points[(int)m_currPoint * 2].ToString();
//    objArray[2] = m_points[((int)m_currPoint * 2) + 1].ToString();
//    objArray[3] = "1";  // start marker
//    objArray[4] = "point "+ m_currPoint;
//    objArray[5] = m_SegmentId.ToString();
//    webBrowser1.Document.InvokeScript("addMarker", objArray);
    m_currPoint=0;
    ui->lblPoint->setText(QString::number(m_currPoint));

    if(m_points.isEmpty())
        return;
    //m_bridge->processScript("addMarker","0,"+QString("%1").arg(((LatLng)m_points.at(0)).lat(),0,'f',8)+","+QString("%1").arg(((LatLng)m_points.at(0)).lon(),0,'f',8)  +",1,'point0'," + QString("%1").arg(m_SegmentId));
    QVariantList objArray;
    objArray << 0 << ((LatLng)m_points.at(0)).lat()<<((LatLng)m_points.at(0)).lon()<<1<<"pointO"<<m_SegmentId;
     m_bridge->processScript("addMarker",objArray);

    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(false);
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(si);
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(0)).lat(), ((LatLng)m_points.at(0)).lon(),m_SegmentId);
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(0)).lat()<<((LatLng)m_points.at(0)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
}

void mainWindow::btnNextClicked()
{
    //SQL sql;
    getArray();
    if(m_currPoint < 0 || m_nbrPoints <2)
        return;
    if ((m_currPoint + 1) < m_nbrPoints)
        m_currPoint++;
    ui->lblPoint->setText(QString::number(m_currPoint));

    QString marker;
    if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
    {
        marker = "0";  // dot marker
        ui->btnSplit->setEnabled(true);
        ui->btnPrev->setEnabled(true);
    }
    else
    {
        ui->btnSplit->setEnabled(false);
        ui->btnNext->setEnabled(false);
        ui->btnPrev->setEnabled(true);
        marker = "2";  // end marker
    }
//    objArray[0] = m_currPoint;
//    objArray[1] = m_points[(int)m_currPoint * 2].ToString();
//    objArray[2] = m_points[((int)m_currPoint * 2) + 1].ToString();
//    objArray[4] = "point " + m_currPoint;
//    objArray[5] = m_SegmentId.ToString();
//    webBrowser1.Document.InvokeScript("addMarker", objArray);
    if(m_points.isEmpty())
        return;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","+marker+",'point"+ QString("%1").arg(m_currPoint)+"',"+ QString("%1").arg(m_SegmentId));
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_SegmentId);

    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(si);
}

void mainWindow::lookupStreetName(SegmentInfo sd)
{
 updateSegmentInfoDisplay(sd);

#if 0 //TODO
        txtStreetName.ForeColor = Color.Black;
        string street = sql->getStreetNameBetweenTwoPoints(new LatLng(sd.startLat, sd.startLon), new LatLng(sd.endLat, sd.endLon), .040);
        if (street != "")
        {
            if (street != sd.streetName)
            {
                txtStreetName.Text = street;
                txtStreetName.ForeColor = Color.DarkOrange;
            }
        }
        else
        {
            if (chkHttp.Checked)
            {
                if (sql->findIntersection(new LatLng(sd.startLat, sd.startLon), .040) == null)
                {
                    //obj = new WorkObj(new LatLng(sd.startLat, sd.startLon));
                    //sList.Add(new intersectionQue(obj));
                    //processSingleItem(new intersectionQue(obj));
                    r.lookupIntersection(new LatLng(sd.startLat, sd.startLon));
                }

                if (sql->findIntersection(new LatLng(sd.endLat, sd.endLon), .040) == null)
                {
                    // obj = new WorkObj(new LatLng(sd.endLat, sd.endLon));
                    //sList.Add(new intersectionQue(obj));
                    //startBackgroundProcessing();
                    //processSingleItem(new intersectionQue(obj));
                    r.lookupIntersection(new LatLng(sd.endLat, sd.endLon));
                }
            }
        }
#endif
}

void mainWindow::btnLastClicked()
{
    //SQL sql;

    getArray();
    m_currPoint = m_nbrPoints - 1;
    ui->lblPoint->setText(QString::number(m_currPoint));

    if (m_nbrPoints < 1)
        return;

    ui->btnSplit->setEnabled(false);
    ui->btnPrev->setEnabled(true);
    ui->btnNext->setEnabled(false);
    QString marker = "2";  // end marker

//    objArray[0] = m_currPoint;
//    objArray[1] = m_points[(int)m_currPoint * 2].ToString();
//    objArray[2] = m_points[((int)m_currPoint * 2) + 1].ToString();
//    objArray[4] = "point " + m_currPoint;
//    objArray[5] = m_SegmentId.ToString();
//    webBrowser1.Document.InvokeScript("addMarker", objArray);
    if(m_points.isEmpty())
        return;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","+marker+",'point"+ QString("%1").arg(m_currPoint)+"',"+ QString("%1").arg(m_SegmentId));

    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_SegmentId);
//TODO            lookupStreetName(sd);
    ////if (sd != null)
    ////{
    ////    txtStreetName.Text = sd.streetName;
    ////    string street = mySql.getStreetNameBetweenTwoPoints(new LatLng(sd.startLat, sd.startLon), new LatLng(sd.endLat, sd.endLon));
    ////    if (street != "")
    ////    {
    ////        txtStreetName.Text = street;
    ////        txtStreetName.ForeColor = Color.DarkOrange;
    ////    }
    ////}
}

void mainWindow::btnPrevClicked()
{
    //SQL sql;
    getArray();
    if (m_currPoint > 0)
        m_currPoint--;
    ui->lblPoint->setText(QString::number(m_currPoint));

    QString marker;
    if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
    {
        marker = "0";  // dot marker
        ui->btnSplit->setEnabled(true);
        ui->btnNext->setEnabled(true);
    }
    else
    {
        marker = "1";  // start marker
        ui->btnSplit->setEnabled(false);
        ui->btnPrev->setEnabled(false);
        ui->btnNext->setEnabled(true);
    }
//    objArray[0] = m_currPoint;
//    objArray[1] = m_points[(int)m_currPoint * 2].ToString();
//    objArray[2] = m_points[((int)m_currPoint * 2) + 1].ToString();
//    objArray[4] = "point " + m_currPoint;
//    objArray[5] = m_SegmentId.ToString();
//    webBrowser1.Document.InvokeScript("addMarker", objArray);
    if(m_points.isEmpty())
        return;
    if(m_currPoint < 0)
     m_currPoint = 0;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","+marker+",'point"+ QString("%1").arg(m_currPoint)+"',"+ QString("%1").arg(m_SegmentId));
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_SegmentId);
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo si = sql->getSegmentInfo(m_SegmentId) ;
    lookupStreetName(si);
    //if (sd != null)
    //{
    //    txtStreetName.Text = sd.streetName;
    //    string street = mySql.getStreetNameBetweenTwoPoints(new LatLng(sd.startLat, sd.startLon), new LatLng(sd.endLat, sd.endLon));
    //    if (street != "")
    //    {
    //        if (street != sd.streetName)
    //        {
    //            txtStreetName.Text = street;
    //            txtStreetName.ForeColor = Color.DarkOrange;
    //        }
    //    }

    //}


}

void mainWindow::closeEvent(QCloseEvent *event)
{
 m_bridge->processScript("getCenter");
 qApp->processEvents(QEventLoop::AllEvents,50);
 if(ui->chkShowOverlay->isChecked())
 {
     m_bridge->processScript("isOverlayLoaded");
     qApp->processEvents(QEventLoop::AllEvents,50);
     QVariant rslt = m_bridge->getRslt();
     //qDebug() << "overlay loaded " << rslt.toString();
     if(config->currCity->overlayList().isEmpty())
         config->currCity->curOverlayId= -1;
     else
         if(config->currCity->curOverlayId >= config->currCity->overlayList().count())
             config->currCity->curOverlayId = 0;
     if( config->currCity->curConnectionId >= 0 && config->currCity->curOverlayId >= 0 && rslt.toString() == "true")
     {
         m_bridge->processScript("overlay.getOpacity");
         qApp->processEvents(QEventLoop::AllEvents,50);
         int opacity = m_bridge->myRslt.toInt();

         City* c = config->currCity;
         Overlay* ov = c->overlayList().at(config->currCity->curOverlayId);
         ov->opacity = opacity;
         config->setOverlay(ov);
     }
 }

 QSettings settings;
 //settingsDb settings;
 settings.setValue("geometry", saveGeometry());
 settings.setValue("windowState", saveState());
 settings.setValue("splitter", ui->splitter->saveState());
//    settings.setValue("center/latitude", m_latitude);
//    settings.setValue("center/longitude", m_longitude);
//    settings.setValue("zoom", m_zoom );
//    settings.setValue("maptype", m_maptype);
 config->currCity->center = LatLng(m_latitude, m_longitude);
 config->currCity->zoom = m_zoom;
 config->currCity->mapType = m_maptype;
 config->currCity->bNoPanOpt = ui->chkNoPan->isChecked();
 config->currCity->lastRoute = m_routeNbr;
 config->currCity->lastRouteName = m_routeName;
 config->currCity->lastRouteEndDate = m_currRouteEndDate;
 config->currCity->bDisplayStationMarkers = bDisplayStationMarkers;
 config->currCity->bDisplayTerminalMarkers = bDisplayTerminalMarkers;
 config->currCity->bDisplayRouteComments = bDisplayRouteComments;
 config->currCity->bShowOverlay = ui->chkShowOverlay->isChecked();
 config->currCity->companyKey = ui->cbCompany->currentIndex();
 config->saveSettings();

 settings.setValue("routeView", ui->tblRouteView->horizontalHeader()->saveState());
 settings.setValue("otherRouteView", ui->tblOtherRouteView->horizontalHeader()->saveState());
 settings.setValue("stationView", ui->tblStationView->horizontalHeader()->saveState());
 settings.setValue("companyView", ui->tblCompanyView->horizontalHeader()->saveState());
 settings.setValue("tractionTypeView", ui->tblTractionTypes->horizontalHeader()->saveState());
 QSqlDatabase db = QSqlDatabase::database();
 db.close();
#ifndef QT_DEBUG
 //systemConsoleAction->close();
 delete SystemConsole::getInstance();
#endif
// if(webViewAction != NULL)
//  webViewAction->closeWebView();
 //QMainWindow::closeEvent(event);
 event->accept();
}
void mainWindow::btnSplit_Click()    // SLOT
{
 //SQL sql;
 SegmentDlg segmentDlg(config, this);
 //segmentDlg.setConfiguration(config);
 segmentDlg.setSegmentId( m_SegmentId);
 segmentDlg.setPt(m_currPoint);
 if(ui->cbRoute->currentIndex() >=0)
  segmentDlg.setRouteData( routeList.at(ui->cbRoute->currentIndex()));
 connect(&segmentDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(segmentDlg_routeChanged(RouteChangedEventArgs)));
 //string RouteNames = "No route";

 if (segmentDlg.exec() == QDialog::Accepted)
 {
  m_SegmentId = segmentDlg.newSegmentId();

  if(routeDlg)
   routeDlg->setSegmentId( m_SegmentId);

  ui->txtSegment->setText( sql->getSegmentDescription(m_SegmentId));
  ui->chkOneWay->setChecked(sql->getSegmentOneWay(m_SegmentId) == "Y");


  // Refresh the Segments combobox
  refreshSegmentCB();
  refreshRoutes();

  // Display the new segment in Google Maps
  //Object[] objArray = new Object[1];

  // redisplay the original altered segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.SegmentId()));
  ui->chkOneWay->setChecked("Y"== sql->getSegmentOneWay(segmentDlg.SegmentId()));
  displaySegment(segmentDlg.SegmentId(), ui->txtSegment->text(), ui->chkOneWay->isChecked()?"Y":"N", "#b45f04", true);

  // display the new segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.newSegmentId()));
  ui->chkOneWay->setChecked("Y" == sql->getSegmentOneWay(segmentDlg.newSegmentId()));

  displaySegment(segmentDlg.newSegmentId(), ui->txtSegment->text(), ui->chkOneWay->isChecked()?"Y":"N", "#b45f04", false);

  ui->btnFirst->setEnabled(true);
  ui->btnNext->setEnabled(true);
  ui->btnPrev->setEnabled( true);
  ui->btnDeletePt->setEnabled(true);

  if (sql->updateSegment(segmentDlg.SegmentId()) != true)
  {
   //System.Media.SystemSounds.Beep.Play();
   QApplication::beep();
   //infoPanel.ForeColor = Color.Red;
   statusBar()->showMessage(tr("updateSegment failed"));
  }
  if (sql->updateSegment(segmentDlg.newSegmentId()) != true)
  {
   //System.Media.SystemSounds.Beep.Play();
   QApplication::beep();
   //infoPanel.ForeColor = Color.Red;
   statusBar()->showMessage(tr("updateSegment failed"));
  }
 }
}
/// <summary>
/// Display a segment's points with an option to clear the existing line
/// </summary>
/// <param name="segmentId"></param>
/// <param name="RouteNames"></param>
/// <param name="segmentName"></param>
/// <param name="oneWay"></param>
/// <param name="color"></param>
/// <param name="bClearFirst">true to clear the line first</param>
void mainWindow::displaySegment(qint32 segmentId, QString segmentName, QString oneWay, QString color, bool bClearFirst)
{
    SegmentInfo si = sql->getSegmentInfo(segmentId);
    si.displaySegment(ui->dateEdit->text(),color, bClearFirst);
    m_currPoint = 0;
    ui->lblPoint->setText(QString::number(m_currPoint));


    ui->btnFirst->setEnabled(true);
    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(false);
    ui->btnLast->setEnabled(true);
    ui->btnSplit->setEnabled(false);
    getArray();
    return;
}

void mainWindow::selectSegment( )
{
 cbSegmentsSelectedValueChanged(ui->cbSegments->currentIndex());
}

void mainWindow::cbSegmentsSelectedValueChanged(qint32 row)
{
    if(bRefreshingSegments)
        return;
    SegmentInfo sI;
    if(row < 0)
        return;
    if(row >= cbSegmentInfoList.count())
        return;
    sI = (SegmentInfo)cbSegmentInfoList.at(row);
    //sI = (segmentInfo)(ui->cbSegments->itemData(row, Qt::UserRole));
    m_SegmentId = sI.segmentId;
    updateSegmentInfoDisplay(sI);

    routeDlg->setSegmentId( m_SegmentId);
    //webBrowser1.Document.InvokeScript("addModeOn");
    if(sI.startLat == 0)
    {
     m_bridge->processScript("addModeOn");
     m_bAddMode = true;
     //return;
    }
    if(ui->chkAddPt->isChecked())
    {
     m_bridge->processScript("addModeOn");
     m_bAddMode = true;
    }
    addPointModeAct->setChecked(ui->chkAddPt->isChecked());
    if(!ui->chkNoPan->isChecked())
    {
        QVariantList objArray;
        objArray << sI.startLat << sI.startLon;
        m_bridge->processScript("setCenter", objArray);
    }


    displaySegment(m_SegmentId, ui->txtSegment->text(), (ui->chkOneWay->checkState()?"Y":"N"), (!ui->chkOneWay->checkState() ? "#00FF00" : "#045fb4"), true);
#if 0
    // Display Start and end markers
    sI = sql->getSegmentInfo(m_SegmentId);
    m_bridge->processScript("addMarker", QString("%1").arg(0)+ ","+ QString("%1").arg(sI.startLat,0,'f',8) + "," + QString("%1").arg(sI.startLon,0,'f',8)+ ","+"1"+",'',"+QString("%1").arg(m_SegmentId));
    QVariantList objArray;
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<sI.startLat<<sI.startLon;
        m_bridge->processScript("setCenter", objArray);
    }
#endif
}

void mainWindow::txtSegment_TextChanged(QString text)
{
 bSegmentChanged = true;
 int ix = text.indexOf(",");
 QString street;
 if(ix > 0 )
  street = text.mid(0,ix);
 if(street != ui->txtStreet->text())
  ui->txtStreet->setText(street);
}

void mainWindow::txtSegment_Leave( )
{
 //SQL sql;
 if (bSegmentChanged)
 {
  SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
  sql->updateSegmentDescription(m_SegmentId, ui->txtSegment->text(), (ui->chkOneWay->checkState()?"Y":"N"), ui->sbTracks->value(), si.length);
  bSegmentChanged = false;
  int segmentId = m_SegmentId;
  refreshSegmentCB();
  for(int i=0; i < cbSegmentInfoList.count(); i++)
  {
   if(cbSegmentInfoList.at(i).segmentId == segmentId)
   {
    ui->cbSegments->setCurrentIndex(i);
    m_SegmentId = segmentId;
    break;
   }
  }
 }
}
void mainWindow::cbSegmentsTextChanged(QString )
{
 b_cbSegments_TextChanged = true;
}
void mainWindow::cbSegments_Leave()
{
 if(b_cbSegments_TextChanged ==true)
 {
  qint32 segmentId = -1;
  QString text = ui->cbSegments->currentText();

  bool bOk=false;
  segmentId = text.toInt(&bOk, 10);

  if (bOk)
  {
   //foreach (segmentInfo sI in segmentInfoList)
   for(int i=0; i< cbSegmentInfoList.count(); i++)
   {
    SegmentInfo sI = (SegmentInfo)cbSegmentInfoList.at(i);

    if (sI.segmentId == segmentId)
    {
     //cbSegments.SelectedItem = sI;
     ui->cbSegments->setCurrentIndex(i);
     break;
    }
   }
  }
 }
 b_cbSegments_TextChanged =false;

}

void mainWindow::cbRoutesTextChanged(QString text)
{
    Q_UNUSED(text)
    b_cbRoutes_TextChanged = true;
}
void mainWindow::cbRoutes_Leave()
{
 QString text =         ui->cbRoute->currentText();
 if(b_cbRoutes_TextChanged)
 {
  //foreach (routeData rd in cbRoutes.Items)
  if(routeList.count()==0)
      return;
  for(int i = 0; i<routeList.count(); i++)
  {
   RouteData rd = (RouteData)routeList.at(i);

   if (rd.alphaRoute.contains(text))
   {
    //cbRoutes.SelectedItem = rd;
    ui->cbRoute->setCurrentIndex(i);
    break;
   }
  }
  b_cbRoutes_TextChanged = false;
 }
}
QString mainWindow::getRouteMarkerImagePath(QString route, bool isStart)
{
    QString work = m_resourcePath;
    QString str = "";
    QString name = "";
    QString tmplt ="";
    QDir dir(m_resourcePath);
    if(!dir.exists("images"))
        dir.mkdir("images");

    QBrush brBkgnd = QBrush(Qt::SolidPattern);
    QFont f;
    if(isStart)
    {
        name = "green"+ route+".png";
        tmplt = "green00.png";
    }
    else
    {
        name = "red" + route + ".png";
        tmplt = "red00.png";
    }
    QFile file(dir.filePath(work+ "/images/" + name));
    str = "file://" + work + "/images/" + name;

    if (file.exists( ))
    {
        return str;
    }
    QFile temp(":/"+ tmplt);
    if(!temp.exists())
    {
        qDebug() <<":/"+ tmplt +" resource not found " ;
        return "";
    }


    // need to make a new one
    QImage image = QImage(":/"+ tmplt);
    QSize resultSize = QSize(image.size());
    QImage resultImage = QImage(resultSize,QImage::Format_ARGB32_Premultiplied);
    //image.save(work + "/images/" + name, "PNG",-1);  //temp
    QRect r = QRect(image.rect());

    QPainter painter(&resultImage);
    painter.drawImage(0, 0, image);
    QRgb color = image.pixel(10,10);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    brBkgnd = QBrush(QColor(color), Qt::SolidPattern);
//    if (isStart)
//        brBkgnd =  QBrush(Qt::green, Qt::SolidPattern);
//    else
//        brBkgnd = QBrush(Qt::red, Qt::SolidPattern);
    if(route.length() == 3)
        f =  QFont("Arial", 7, QFont::Bold, false);
    else
        f =  QFont("Arial", 6, QFont::Bold, false);
    painter.setFont(f);

    QRect eRect =painter.boundingRect(r, Qt::AlignCenter, "000");
    eRect.adjust(0, -3.0, 0, 0);
    painter.fillRect(eRect, brBkgnd);
    QRectF bRect = painter.boundingRect(r, Qt::AlignCenter, route); // bounding rectangle of text
    bRect.adjust(0, -3.0, 0, 0);
    painter.fillRect(bRect, brBkgnd);
    painter.setPen(Qt::white);
    painter.drawText(bRect, Qt::AlignCenter, route);
    painter.end();
    resultImage.save(work+ "/images/" + name, "PNG");


    return str;
}
QString mainWindow::ProcessScript(QString func, QString params)
{
    m_bridge->processScript(func, params);
    return "";
}
//void MainWindow::tblRouteView_selectionChanged(QTableWidgetItem *item)
//{
//    qint32 row = item->row();
//    QTableWidgetItem* segmentIdItem = ui->routeView->item(row, 1);
//    bool bOk=false;
//    qint32 segmentId = segmentIdItem->text().toInt(&bOk, 10);
//    if(bOk)
//        m_bridge->processScript("selectSegment", QString("%1").arg(segmentId));

//}
void mainWindow::addPoint(int pt, double lat, double lon)
{
    //SQL sql;
    //getArray(); // get points from display.
    SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
    m_points = si.pointList;
    m_points.append(LatLng(lat, lon));
    m_nbrPoints = m_points.size();
    //int begin = (int)m_nbrPoints - 2, End = (int)m_nbrPoints - 1;

    if(m_nbrPoints == 2 && si.startLat == 0 && si.startLon == 0)
    {
     si.addPoint(m_points.at(0));
     si.startLat = m_points.at(0).lat();
     si.startLat = m_points.at(0).lon();
    }
    if(si.segmentId == m_SegmentId && m_nbrPoints > 0)
    {
     si.addPoint(m_points.at(m_nbrPoints-1 ));
    }
    else
    {
     si.addPoint(m_points.at(0));
    }
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(si.pointList.count()));
#if 0
    if (m_nbrPoints > 0)
    {
//        sql->addPoint((int)m_nbrPoints - 2, m_SegmentId, m_points[(int)Begin * 2], m_points[((int)Begin * 2) + 1],
//            m_points[(int)End * 2], m_points[((int)End * 2) + 1], txtStreetName.Text);
        sql->addPoint(m_nbrPoints - 2, m_SegmentId, ((LatLng)m_points.at(Begin)).lat(), ((LatLng)m_points.at(Begin)).lon(),
            ((LatLng)m_points.at(End)).lat(), ((LatLng)m_points.at(End )).lon(), ui->txtStreet->text());
        routeView->updateRouteView();
        if (sql->updateSegment(m_SegmentId) != true)
        {
            //System.Media.SystemSounds.Beep.Play();
            //infoPanel.ForeColor = Color.Red;
            statusBar()->showMessage("updateSegment failed");
        }

    }
#endif
}

/// <summary>
/// Called by Google Maps when a point on a polyline is moved.
/// </summary>
/// <param name="i"></param>
/// <param name="newLat"></param>
/// <param name="newLon"></param>
void mainWindow::movePoint(qint32 segmentId, qint32 i, double newLat, double newLon)
{
 m_SegmentId = segmentId;
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);

 if(si.segmentId == m_SegmentId)
 {
  si.movePoint(i, LatLng(newLat, newLon));
 }
    //SQL sql;
    LatLng oldPoint = sql->getPointOnSegment((int)i, m_SegmentId);
#if 0
    if (sql->movePoint(i, m_SegmentId, newLat, newLon) != true)
    {
        //System.Media.SystemSounds.Beep.Play();
        QApplication::beep();
        //infoPanel.ForeColor = Color.Red;
        statusBar()->showMessage(tr("movePoint failed"));
    }
    if (sql->updateSegment(m_SegmentId) != true && m_nbrPoints > 0)
    {
     //System.Media.SystemSounds.Beep.Play();
     QApplication::beep();
     //infoPanel.ForeColor = Color.Red;
     statusBar()->showMessage(tr("updateSegment failed"));
    }
#endif
    //TODO what about multiple station records?
    StationInfo sti = sql->getStationAtPoint(oldPoint);
    if (sti.stationKey > 0)
    {
        sql->updateStation(sti.stationKey, LatLng(newLat, newLon));
    }
}

// called by webBrowser map initialization to see if an overlay should be loaded
void mainWindow::queryOverlay()
{
//    overlays ov = (overlays)cbOverlays.SelectedItem;
//    if(ov.overlayName != "")
//        webBrowser1.Document.InvokeScript("loadOverlay", new object[] { ov.overlayName, ov.opacity });
    if(!ui->chkShowOverlay->isChecked())
        return;
    if(config->currCity->curOverlayId >= 0)
    {
        Overlay* ov = config->currCity->overlayList().at(config->currCity->curOverlayId );
//        m_bridge->processScript("loadOverlay", "'"+ov->name+"',"+QString("%1").arg(ov->opacity)+ ","+ QString::number(ov->minZoom)+ ","+ QString::number(ov->maxZoom)+ ",'"+ ov->source+ "'");
        loadOverlay(ov);
    }
}

void mainWindow::getGeocoderResults(QString array)
{
    QStringList sa;
    sa = array.split(';');
//TODO    addressPanel.Text = sa[0];
    geocoderRslt->setText(sa.at(0));
}



/// <summary>
/// Called by Google Maps when an intersection is being updated. All segment end points will be set to the same latitude and longitude.
/// </summary>
/// <param name="i"></param>
/// <param name="newLat"></param>
/// <param name="newLon"></param>
void mainWindow::updateIntersection(qint32 i, double newLat, double newLon)
{
 Q_UNUSED(i)
 //SQL sql;
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
 // get all the points within 100 meters
 //QList<segmentData> myArray = sql->getIntersectingSegments(newLat, newLon, .020, si.routeType);
 QList<SegmentInfo> myArray = sql->getIntersectingSegments(newLat, newLon, .020, si.routeType);
 int currSegment = m_SegmentId;
 //foreach (segmentData sd in myArray)
 for(int i=0; i< myArray.count(); i++)
 {
  //segmentData sd= (segmentData)myArray.at(i);
  SegmentInfo si = myArray.at(i);
  //QString oneWay = sql->getSegmentOneWay(sd.SegmentId);
  m_SegmentId = si.segmentId;
  //m_nbrPoints = sql->getNbrPoints(m_SegmentId);
  m_nbrPoints = si.pointList.count();
  if(si.whichEnd == "S")
   movePoint(m_SegmentId, 0, newLat, newLon);
  else
   movePoint(m_SegmentId, m_nbrPoints -1, newLat, newLon);
  //displaySegment(sd.SegmentId, sql->getSegmentDescription(si.SegmentId), oneWay, oneWay == "N" ? "#00FF00" : "#045fb4", true);
  displaySegment(si.segmentId, si.description, si.oneWay, si.oneWay == "N" ? "#00FF00" : "#045fb4", true);
 }
 m_SegmentId = currSegment;
}


/// <summary>
/// Called by Google Maps when a point on a polyline is inserted
/// </summary>
/// <param name="i">point to insert after</param>
/// <param name="newLat">New latitude</param>
/// <param name="newLon">New longitude</param>
void mainWindow::insertPoint(int SegmentId, qint32 i, double newLat, double newLon)
{
 segmentSelected(i,SegmentId);
 SegmentInfo si = sql->getSegmentInfo((int)SegmentId);
 si.insertPoint(i, LatLng(newLat, newLon));
#if 0
    //SQL sql;
    sql->insertPoint(i, SegmentId, newLat, newLon);
    m_currPoint++;
#endif
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    si = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(si);
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(si.pointList.count()));
}
//void MainWindow::btnDisplay_Click() // SLOT
//{
//    SQL sql;
//    int ix= ui->cbSegments->currentIndex();


//    segmentInfo si = sql->getSegmentInfo(m_SegmentId);
//    displaySegment(m_SegmentId, si.description, si.oneWay, "#FF00FF", true);

//    ui->btnFirst->setEnabled(true);
//    btnPlus.Enabled = true;
//    btnMinus.Enabled = true;
//    btnDelete.Enabled = true;
//    btnSplit.Enabled = true;
//    getArray();
//}

/// <summary>
/// Delete the point at the current marker
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void mainWindow::btnDeletePtClicked()
{
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);

 if(si.segmentId == m_SegmentId)
 {
  if(m_nbrPoints < 3)
  {
   statusBar()->showMessage("only 2 points. use deleteSegment instead!");
   return;
  }
  si.deletePoint(m_currPoint);
  m_bridge->processScript("deletePoint", QString("%1").arg(m_currPoint));
  if (m_currPoint > 0)
      m_currPoint--;
  ui->lblPoint->setText(QString::number(m_currPoint));

  QString marker;
  getArray();
  if (m_currPoint == 0)
  {
      //objArray[3] = "1"; // start marker
      marker = "1";
  }
  else
  {
      if (m_currPoint == (m_nbrPoints - 1))
          //objArray[3] = "2"; // end marker
          marker = "2";
      else
          //objArray[3] = "0"; // intermediate marker
          marker = "0";
  }
  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(si.pointList.count()));
  //webBrowser1.Document.InvokeScript("addMarker", objArray);
  m_bridge->processScript("addMarker", QString("%1").arg(m_currPoint)+ ","+ QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8) + "," + QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8)+ ","+marker+",'',"+QString("%1").arg(m_SegmentId));
  QVariantList objArray;
  if(!ui->chkNoPan->isChecked())
  {
      objArray.clear();
      objArray<<((LatLng)m_points.at(m_currPoint)).lat()<< ((LatLng)m_points.at(m_currPoint)).lon();
      m_bridge->processScript("setCenter", objArray);
  }

 }
#if 0
    //SQL sql;
    if (sql->deletePoint(m_currPoint, m_SegmentId, m_nbrPoints))
    {
        if (m_nbrPoints < 3)
        {
            statusBar()->showMessage("only 2 points. use deleteSegment instead!");
            return;
        }
//        Object[] objArray = new Object[1];
//        objArray[0] = m_currPoint;
//        webBrowser1.Document.InvokeScript("deletePoint", objArray);
        m_bridge->processScript("deletePoint", QString("%1").arg(m_currPoint));

        if (m_currPoint > 0)
            m_currPoint--;
        QString marker;
        getArray();
//        objArray = new Object[6];
//        objArray[0] = m_currPoint;
//        objArray[1] = m_points[(int)m_currPoint * 2].ToString();
//        objArray[2] = m_points[((int)m_currPoint * 2) + 1].ToString();
//        objArray[4] = "";
//        objArray[5] = m_SegmentId.ToString();
        if (m_currPoint == 0)
        {
            //objArray[3] = "1"; // start marker
            marker = "1";
        }
        else
        {
            if (m_currPoint == (m_nbrPoints - 1))
                //objArray[3] = "2"; // end marker
                marker = "2";
            else
                //objArray[3] = "0"; // intermediate marker
                marker = "0";
        }
        //webBrowser1.Document.InvokeScript("addMarker", objArray);
        m_bridge->processScript("addMarker", QString("%1").arg(m_currPoint)+ ","+ QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8) + "," + QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8)+ ","+marker+",'',"+QString("%1").arg(m_SegmentId));
        QVariantList objArray;
        if(!ui->chkNoPan->isChecked())
        {
            objArray.clear();
            objArray<<((LatLng)m_points.at(m_currPoint)).lat()<< ((LatLng)m_points.at(m_currPoint)).lon();
            m_bridge->processScript("setCenter", objArray);
        }

    }
    else
    {
        //System.Media.SystemSounds.Beep.Play();
        QApplication::beep();
        //infoPanel.ForeColor = Color.Red;
        statusBar()->showMessage("deletePoint failed");
        return;
    }
    if (sql->updateSegment(m_SegmentId) != true)
    {
        //System.Media.SystemSounds.Beep.Play();
        QApplication::beep();
        //infoPanel.ForeColor = Color.Red;
        statusBar()->showMessage("updateSegment failed");
    }
    else
        statusBar()->showMessage( "");
#endif
}


void mainWindow::txtStreetName_TextChanged(QString )
{
 bStreetChanged = true;
}

void mainWindow::sbTracks_valueChanged(int)
{
 bStreetChanged = true;
 bSegmentChanged = true;
 txtStreetName_Leave();
}

void mainWindow::txtStreetName_Leave()
{
 if (bStreetChanged && (m_currPoint < m_nbrPoints) && m_SegmentId > -1)
 {
  //segmentData sd = segmentData(m_currPoint, m_SegmentId);
  SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
  if(si.pointList.count()<2) return;
  si.startLat = si.pointList.at(0).lat();
  si.startLon = si.pointList.at(0).lon();
  si.endLat = si.pointList.at(si.pointList.count()-1).lat();
  si.endLon = si.pointList.at(si.pointList.count()-1).lon();
  bool bUpdate = false;
  if( ui->txtStreet->text().trimmed() != si.streetName)\
  {
   si.streetName = ui->txtStreet->text().trimmed();
   bUpdate = true;
  }
  if(ui->sbTracks->value() != si.tracks)
  {
   si.tracks = ui->sbTracks->value();
   bUpdate = true;
  }
  if((ui->chkOneWay->isChecked()?"Y":"N") != si.oneWay)
  {
   si.oneWay = ui->chkOneWay->isChecked()?"Y":"N";
   bUpdate = true;
  }
  if(bUpdate)
  {
   //SQL mySql;
   sql->updateRecord(si);
   refreshSegmentCB();
  }
 }
 bStreetChanged = false;
}

void mainWindow::getZoom(int zoom)
{
    zoomIndicator->setText("Zoom: "+ QString("%1").arg(zoom));
    m_zoom = zoom;
    config->currCity->zoom = zoom;
}

void mainWindow::copyRouteInfo_Click()
{
 DialogCopyRoute form(config, this);
 //form.Configuration = config;
 form.setRouteData( (RouteData)routeList.at(ui->cbRoute->currentIndex()));
 int rslt =  form.exec();
 if (rslt == QDialog::Accepted)
 {
  refreshRoutes();
  RouteData rd = form.getRouteData();
  for(int i=0; i < routeList.count(); i++)
  {
   RouteData _rd = (RouteData)routeList.at(i);
   if (rd.route == _rd.route && rd.name == _rd.name && rd.endDate == _rd.endDate)
   {
    bNoDisplay = true;
    ui->cbRoute->setCurrentIndex(i);
    bNoDisplay = false;
   }
  }
 }
}
void mainWindow::splitRoute_Click()
{
    //SplitRoute();
    SplitRoute splitRouteDlg(config, this);
    //splitRouteDlg.setConfiguration (config);
    splitRouteDlg.setRouteData (routeList.at(ui->cbRoute->currentIndex()));
    if (splitRouteDlg.exec() == QDialog::Accepted)
    {
        RouteData newRoute = splitRouteDlg.getNewRoute();
        refreshRoutes();
        for(int i =0; i < routeList.count(); i++)
        {
            RouteData rd = routeList.at(i);
            if(rd.route == newRoute.route && rd.name == newRoute.name && rd.endDate == newRoute.endDate)
            {
             bNoDisplay = true;
             ui->cbRoute->setCurrentIndex(i);
             bNoDisplay = false;
             break;
            }
        }
    }
}
void mainWindow::rerouteRoute()
{
    ReroutingDlg rerouteDlg((RouteData)routeList.at(ui->cbRoute->currentIndex()), config, this);
    int rslt = rerouteDlg.exec();
    if (rslt == QDialog::Accepted)
    {
        refreshRoutes();
    }
}

void mainWindow::renameRoute_Click()
{
    DialogRenameRoute renameRouteDlg(config, this);
    //renameRouteDlg.setConfig(config);
    renameRouteDlg.routeData ( (RouteData)routeList.at(ui->cbRoute->currentIndex()));
    int rslt = renameRouteDlg.exec();
    if (rslt == QDialog::Accepted)
    {
        refreshRoutes();
#if 1 // TODO
        // set the combo box to the renamed route.
        for(int i=0; i < routeList.count();i++)
        {
            RouteData rd = routeList.at(i);
            if(rd.toString() == renameRouteDlg.getRouteData().toString())
            {
                        ui->cbRoute->setCurrentIndex(i);
                break;
            }
        }
        //cbRoutes.SelectedItem = renameRouteDlg.RouteData;
        //cbRoutes.SelectedIndex = cbRoutes.FindString(renameRouteDlg.RouteData.ToString());
#endif
    }
}
void mainWindow::modifyRouteDate()
{
#if 1 // TODO

     ModifyRouteDateDlg form;
     //ModifyRouteDateDlg((routeData)cbRoutes.SelectedItem);
    //form.Configuration = config;
    form.setConfiguration(config);
    //form.setRouteData(routeList.at(ui->cbRoute->currentIndex()));
    form.setRouteData(routeList, ui->cbRoute->currentIndex());

    qint32 rslt = form.exec();
    if (rslt == form.Accepted)
    {
     refreshRoutes();

     //cbRoutes.SelectedItem = form.RouteData;
     //cbRoutes.SelectedIndex = cbRoutes.FindString(form.RouteData.ToString());
     for(int i=0; i <routeList.count(); i++)
     {
      RouteData rd =routeList.at(i);
      if(rd.toString() == form.getRouteData()->toString())
      {
                  ui->cbRoute->setCurrentIndex(i);
          break;
      }
     }
     routeView->updateRouteView();
    }
#else
    NotYetInplemented();
#endif
}

void mainWindow::addSegment()
{
#if 1 // TODO
    //SQL sql;
    SegmentDlg segmentDlg(config, this);
//TODO    segmentDlg.routeChanged += new routeChangedEventHandler(segmentDlg_routeChanged);
    connect(&segmentDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(segmentDlg_routeChanged(RouteChangedEventArgs)));
    //segmentDlg.setConfiguration( config);
    segmentDlg.setSegmentId( -1);
    m_currPoint = 0;
    ui->lblPoint->setText(QString::number(m_currPoint));

    segmentDlg.setPt( m_currPoint);
    //int ix = cbRoutes.SelectedIndex;
    //if(ix > -1)
    //{
    //    segmentDlg.RouteData = (routeData)routeDataList[ix];
    //}
    RouteData rd = RouteData();
    if(ui->cbRoute->currentIndex() >= 0)
     rd = (RouteData)routeList.at(ui->cbRoute->currentIndex());
    segmentDlg.setRouteData (rd);
    if (segmentDlg.exec() == QDialog::Accepted)
    {
        refreshSegmentCB();
        m_SegmentId = segmentDlg.newSegmentId();
        m_nbrPoints = 0;
        //??routeDlg.SegmentId = m_SegmentId;
        //        ui->txtSegment->setText(sql->getSegmentDescription(m_SegmentId));

        ui->lblSegment->setText(tr("Segment %1:").arg(m_SegmentId));
        //        ui->chkOneWay->setChecked("Y"== sql->getSegmentOneWay(m_SegmentId));
        SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
        updateSegmentInfoDisplay(si);

        int dash = 0;
        if(si.routeType == Incline)
         dash = 1;
        else if(si.routeType == SurfacePRW)
         dash = 2;
        else if(si.routeType == Subway)
         dash = 3;

        QVariantList objArray;
        objArray << m_SegmentId << segmentDlg.routeName()<<ui->txtSegment->text()<<(ui->chkOneWay?"Y":"N")<<getColor(segmentDlg.tractionType())<<si.tracks << dash;
        m_bridge->processScript("createSegment",objArray);

        //webBrowser1.Document.InvokeScript("addModeOn");
        m_bridge->processScript("addModeOn");
        m_bAddMode = true;

        si = sql->getSegmentInfo(m_SegmentId);
        lookupStreetName(si);
        //refreshRoutes();
        //refreshSegmentCB();
    }
#else
    NotYetInplemented();
#endif
}
void mainWindow::deleteRoute()
{
    //SQL sql;
#if 1 // TODO
    RouteData rd = routeList.at(        ui->cbRoute->currentIndex());

    QMessageBox::StandardButton reply;
    reply=QMessageBox::warning(this, tr("Confirm Delete"),tr("Are you sure you want to delete route ") + rd.alphaRoute + " " + rd.name + "?", QMessageBox::Yes | QMessageBox::No);
    //DialogResult rslt =  MessageBox.Show("Are you sure you want to delete route " + rd.alphaRoute + " " + rd.name + "?",
        //"Confirm Delete", MessageBoxButtons.YesNo);
    if(reply== QMessageBox::Yes)
    {
        sql->deleteRoute(rd.route, rd.name, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd"));
    }
    refreshRoutes();
#endif
}
void mainWindow::updateRoute()
{
    //SQL sql;
        QList<SegmentInfo> segmentInfoList = sql->getSegmentInfo();

    if (!routeDlg)
    {
        routeDlg = new RouteDlg(config, this);
        //routeDlg->Configuration ( config);
        //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
        connect(routeDlg, SIGNAL(SegmentChangedEvent(qint32, qint32)),this, SLOT(segmentChanged(qint32,qint32)));
        //routeDlg->routeChanged += new routeChangedEventHandler(RouteChanged);
        connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs )), this, SLOT(RouteChanged(RouteChangedEventArgs )));
    }

    routeDlg->setRouteNbr( m_routeNbr);
    routeDlg->setSegmentId( m_SegmentId);
    routeDlg->setRouteData(_rd);
    //routeDlg.segmentInfoList = segmentInfoList;
    routeDlg->show();
    routeDlg->raise();
    routeDlg->activateWindow();

}
void mainWindow::updateTerminals()
{
#if 0 // TODO
    animationPath = buildAnimationPath();
    cbRoutes_SelectedIndexChanged(sender, e);
    showAnimation(animationPath);
    updateRouteView(sender, e);
    terminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, dateTimePicker1.Text);
    if (ti != null)
    {
        object[] objArray = new Object[] { ti.startLatLng.lat, ti.startLatLng.lon, getRouteMarkerImagePath(m_alphaRoute, true) };
        webBrowser1.Document.InvokeScript("addRouteStartMarker", objArray);

        objArray = new Object[] { ti.endLatLng.lat, ti.endLatLng.lon, getRouteMarkerImagePath(m_alphaRoute, false) };
        webBrowser1.Document.InvokeScript("addRouteEndMarker", objArray);

    }
#else
    //NotYetInplemented();
    routeView->updateTerminals();
#endif
}

void mainWindow::selectSegment(int seg)
{
 //segmentChanged(seg, seg);

 for(int i=0; i < cbSegmentInfoList.count(); i++)
 {
  if(cbSegmentInfoList.at(i).segmentId == seg)
  {
      ui->cbSegments->setCurrentIndex(i);
      m_SegmentId = seg;
      cbSegmentsSelectedValueChanged(i);
      otherRouteView->showRoutesUsingSegment(seg);
      break;
  }
 }
}

void mainWindow::segmentChanged(qint32 changedSegment, qint32 newSegment)
{
    //SQL sql;
    //Object[] objArray = new Object[1];
    //objArray[0] = e.ChangedSegment;
    //webBrowser1.Document.InvokeScript("isSegmentDisplayed", objArray); // check to see if it is displayed
    m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(changedSegment));
    if (m_segmentStatus == "Y")
    {
//        objArray = new Object[1];
//        objArray[0] = e.ChangedSegment;
//        webBrowser1.Document.InvokeScript("clearPolyline", objArray); // clears the old line
        m_bridge->processScript("clearPolyline", QString("%1").arg(changedSegment));
    }
    if (newSegment > 0)
    {
     SegmentInfo si = sql->getSegmentInfo(newSegment);

     displaySegment(newSegment, si.description, si.oneWay, m_segmentColor, true);
    }
}
void mainWindow::segmentStatus(QString str, QString color)
{
    m_segmentStatus = str;
    m_segmentColor = color;
}
//void MainWindow::segmentDlg_routeChanged(qint32 typeOfChange, qint32 route, QString name, QString endDate)
void mainWindow::segmentDlg_routeChanged(RouteChangedEventArgs args)
{
    refreshRoutes();
   if (args.typeOfChange == "Add")
    {
        //foreach (routeData rd in cbRoutes.Items)
        for(int i=0; i<routeList.count(); i++)
        {
            RouteData rd = (RouteData)routeList.at(i);
            if (rd.route == args.routeNbr && rd.name == args.RouteName && args.dateEnd == rd.endDate)
            {
                //cbRoutes.SelectedItem = rd;
                        ui->cbRoute->setCurrentIndex(i);
                break;
            }
        }
    }
    //else
//TODO        cbRoutes_SelectedIndexChanged(sender, e);
}
/// <summary>
/// Event handler for routeChanged
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void mainWindow::RouteChanged(RouteChangedEventArgs args)
{
 //SQL sql;
 refreshRoutes();
 for(int i=0; i < routeList.count(); i++)
 {
  RouteData rd = routeList.at(i);
  if(rd.route == args.routeNbr && args.RouteName == rd.name && rd.endDate == args.dateEnd)
  {
   bCbRouteRefreshing = true;
   ui->cbRoute->setCurrentIndex(i);
   bCbRouteRefreshing = false;
   break;
  }
 }
 if (args.typeOfChange == "Delete")
 {
//        object[] objArray = new Object[1];
//        objArray[0] = e.SegmentId;
//        webBrowser1.Document.InvokeScript("clearPolyline", objArray); // clears the old line
  m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(args.routeSegment));
//        webBrowser1.Document.InvokeScript("clearMarker");
  m_bridge->processScript("clearMarker");
 }
 else
 {
//        object[] objArray = new Object[1];
//        objArray[0] = e.SegmentId;
//        webBrowser1.Document.InvokeScript("clearPolyline", objArray); // clears the old line
  m_bridge->processScript("clearPolyline", QString("%1").arg(args.routeSegment));
  SegmentInfo si = sql->getSegmentInfo(args.routeSegment);
  displaySegment(args.routeSegment, si.description, si.oneWay, /*ttColors[e.tractionType]*/getColor(args.tractionType), true);
 }
 routeView->updateRouteView();
}

//// Show Overview window in GoogleMaps
//void MainWindow::chkShowWindow_CheckedChanged()
//{
//    //webBrowser1.Document.InvokeScript("showWindowControl", new object[] { chkShowWindow.Checked });
//    m_bridge->processScript("showWindowControl", ui->chkShowWindow?"true":"false");
//}
void mainWindow::opacityChanged(QString name, qint32 opacity)
{
 Overlay* ov = new Overlay(name, opacity);
    //ov->name = name;
    //ov.opacity = opacity;
    config->setOverlay(ov);
    statusBar()->showMessage(tr("%2 opacity=%1").arg(opacity).arg(name));
    //m_bridge->processScript("setOverlayOpacity", QString::number(opacity));
}
void mainWindow::moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //SQL sql;
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, m_currRouteEndDate);
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate.toString("yyyy/MM/dd"), ti.endDate.toString("yyyy/MM/dd"), segmentId, i == 0 ? "S" : "E", ti.endSegment, ti.endWhichEnd);
}
void mainWindow::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //SQL sql;
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, m_currRouteEndDate);
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate.toString("yyyy/MM/dd"), ti.endDate.toString("yyyy/MM/dd"), ti.startSegment, ti.startWhichEnd, segmentId, i == 0 ? "S" : "E");

}
void mainWindow::AddRoute()
{
    if(routeDlg == 0)
        routeDlg = new RouteDlg(config, this);
    routeDlg->setAddMode(true);
    routeDlg->show();
    routeDlg->raise();
    routeDlg->activateWindow();
}
void mainWindow::addModeToggled(bool isChecked)
{
    if(!isChecked)
        m_bridge->processScript("addModeOff");
    else
        m_bridge->processScript("addModeOn");
}
void mainWindow::displayStationMarkersToggeled(bool bChecked)
{
    bDisplayStationMarkers = bChecked;
    //displayStationMarkersAct->setChecked(bDisplayStationMarkers);
    m_bridge->processScript("displayStationMarkers", bChecked?"true":"false");
}
void mainWindow::displayTerminalMarkersToggeled(bool bChecked)
{
    bDisplayTerminalMarkers = bChecked;
    m_bridge->processScript("displayTerminalMarkers", bChecked?"true":"false");
}
void mainWindow::displayRouteCommentsToggled(bool bChecked)
{
    bDisplayRouteComments = bChecked;
    m_bridge->processScript("showRouteInfo", bChecked?"true":"false");
}

void mainWindow::geocoderRequestToggled(bool bChecked)
{
    config->currCity->bGeocoderRequest = bChecked;
    m_bridge->processScript("setGeocoderRequest", bChecked?"true":"false");
}

void mainWindow::chkShowOverlayChanged(bool bChecked)
{
 if(config->currCity->overlayList().size() == 0) return;
 if(bChecked && config->currCity->curOverlayId >= 0)
 {
  Overlay* ov = config->currCity->overlayList().at(config->currCity->curOverlayId);
  //m_bridge->processScript("loadOverlay", "'" +ov->name+"',"+QString("%1").arg(ov->opacity));
//  m_bridge->processScript("loadOverlay", QString("'%1',%2,%3,%4,'%5'").arg(ov->name).arg(ov->opacity).arg(ov->minZoom).arg(ov->maxZoom).arg(ov->source));
  loadOverlay(ov);
 }
 else
  m_bridge->processScript("loadOverlay", "null,0,0,0,''");
}
/// <summary>
/// setStation. Called when user doubleclicks on a line segment to add a new station
/// </summary>
/// <param name="lat"></param>
/// <param name="lon"></param>
/// <param name="segmentId"></param>
void mainWindow::setStation(double lat, double lon, qint32 segmentId, qint32 ptIndex)
{
 //SQL sql;
 try
 {
  LatLng pt =  LatLng(lat, lon);
  SegmentInfo si = sql->getSegmentInfo(segmentId);
  editStation form(-1, bDisplayStationMarkers, this);
  //form.setConfiguration(config);
  form.setPoint(pt);
  form.setSegmentId(segmentId);
  form.setIndex( ptIndex);
  QDateTime dt;
  dt = QDateTime::fromString(si.startDate, "yyyy/MM/dd");
  form.setStartDate( dt);
  dt = QDateTime::fromString(si.endDate, "yyyy/MM/dd");
  form.setEndDate( dt);

  QString markerType = "green";
  switch (si.routeType)
  {
   case RapidTransit:
       markerType = "green";
       break;
   case Subway:
       markerType = "blue";
       break;
   case Surface:
       markerType = "red";
       break;
   default:
       markerType = "red";
       break;
  }
  form.setMarkerType(markerType);

  //if (form.ShowDialog() == DialogResult.OK)
  form.exec();
 }
 catch (std::exception e)
 {
//TODO       messsageBox.Show(e.Message, "error");
 }
}
void mainWindow::updateStation(qint32 stationKey, qint32 segmentId)
{
    StationInfo sti = sql->getStationInfo(stationKey);
    //segmentInfo si = sql->getSegmentInfo(segmentId);
    commentInfo ci =  commentInfo();
    QVariantList objArray;

    editStation form(sti.stationKey, bDisplayStationMarkers, this);
    //form.setConfiguration(config);
    //form.setStationId(sti.stationKey);
 form.exec();
}
void mainWindow::moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lon)
{
 SegmentInfo si = sql->getSegmentInfo(segmentId);
 if(si.segmentId < 0)
  qDebug() << tr("invalid segmentId=%1 stationKey=%2").arg(segmentId).arg(stationKey);

 StationInfo sti = sql->getStationInfo(stationKey);
 if(sti.segmentId != -1 && sti.segmentId != segmentId)
 {
  // add another station with the same name but different segment.
  int stationKey = sql->addStation(sti.stationName,LatLng(sti.latitude, sti.longitude),segmentId,si.startDate,si.endDate,sti.geodb_loc_id, sti.infoKey,si.routeType,sti.markerType,sti.point);
  commentInfo ci = sql->getComments(sti.infoKey);
  QVariantList objArray;
  objArray << lat<< lon << (bDisplayStationMarkers?true:false)<<segmentId<<sti.stationName<<stationKey<<sti.infoKey<<ci.comments<<sti.markerType;
  m_bridge->processScript("addStationMarker",objArray);
  // the Javascript will call this function again with the new stationKey
  stationView->changeStation("move", sti);
  return;
 }
 sti.segmentId = segmentId;
 LatLng latLng(lat, lon);
 if(latLng.isValid())
 {
     if(sti.geodb_loc_id > 0)
         sql->updateStation(sti.geodb_loc_id, stationKey,  LatLng(lat, lon));
     else
         sql->updateStation(stationKey,  LatLng(lat, lon), segmentId);
    }
    sti = sql->getStationInfo(stationKey);
    sti.latitude = lat;
    sti.longitude = lon;
    stationView->changeStation("move", sti);

    QString str = sti.stationName;
    QString markerType = "default";
    QVariantList objArray;

    sti = sql->getStationInfo(stationKey); // get station's updated route info
    commentInfo ci = sql->getComments(sti.infoKey);
    //str = ci.comments;
    m_bridge->processScript("removeStationMarker", QString("%1").arg(stationKey));
    //m_bridge->processScript("addStationMarker",QString("%1").arg(lat,0,'f',8) +","+QString("%1").arg(lon,0,'f',8) +","+(bDisplayStationMarkers?"true":"false")+","+QString("%1").arg(sti.segmentId)+",'"+sti.stationName+"',"+QString("%1").arg(stationKey)+","+QString("%1").arg(sti.infoKey)+",comments,'"+markerType+"'", "comments", ci.comments);
    objArray.clear();
    objArray << lat<< lon << (bDisplayStationMarkers?true:false)<<sti.segmentId<<sti.stationName<<stationKey<<sti.infoKey<<ci.comments<<sti.markerType;
    m_bridge->processScript("addStationMarker",objArray);
}

void mainWindow::chkOneWay_toggled(bool bChecked)
{
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
 if(bChecked)
 {
  ui->sbTracks->setEnabled(false);
  ui->sbTracks->setValue(1);
 }
 else
 {
  ui->sbTracks->setValue(si.tracks);
  ui->sbTracks->setEnabled(true);
 }
 bSegmentChanged = true;
 txtSegment_Leave();
}

void mainWindow::chkOneWay_Leave(bool bChecked)
{
 SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
 if(!bChecked)
  ui->sbTracks->setEnabled(true);

 sql->updateSegmentDescription(m_SegmentId, ui->txtSegment->text(), bChecked?"Y":"N", si.tracks, si.length);
 refreshSegmentCB();
}

void mainWindow::exportDb()
{
 ExportDlg form(config, this);
 form.exec();
}

void mainWindow::editConnections()
{
 //NotYetInplemented();
 editConnectionsDlg form( this);
 form.exec();

 createCityMenu();
 this->setWindowTitle("Mapper - "+ config->currCity->name + " ("+config->currConnection->description()+")");
}

void mainWindow::locateStreet()
{
    LocateStreetDlg form(this);
    form.exec();
}

void mainWindow::findDupSegments()
{
    QList<SegmentInfo> myArray;
    this->setCursor(QCursor(Qt::WaitCursor));
    //foreach(segmentInfo si in segmentInfoList)
    for(int i=0; i < cbSegmentInfoList.count(); i++)
    {
        SegmentInfo si = cbSegmentInfoList.at(i);
        SegmentInfo siDup = sql->getSegmentInSameDirection(si);
        siDup.next = si.segmentId;

        if(siDup.segmentId > -1)
            myArray.append(siDup);
        qApp->processEvents();

    }
    if(ui->tabWidget->count() < 7)
     ui->tabWidget->addTab(ui->tblDupSegments,"DuplicateSegments");
    dupSegmentView->showDupSegments(myArray);
    this->setCursor(QCursor(Qt::ArrowCursor));
    //NotYetInplemented();

}

void mainWindow::on_webView_statusBarMessage(QString text)
{
    setDebug(text);
}

void mainWindow::combineRoutes()
{
    CombineRoutesDlg dlg(ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt(), this);
    dlg.exec();
    refreshRoutes();
}

void mainWindow::updateRouteComment()
{
 RouteCommentsDlg routeCommentsDlg(config, this);

 int row =         ui->cbRoute->currentIndex();
 RouteData rd = ((RouteData)routeList.at(row));
 routeCommentsDlg.setCompanyKey(rd.companyKey);
 routeCommentsDlg.setRoute(rd.route);
 routeCommentsDlg.setDate(rd.startDate);
 routeCommentsDlg.exec();
}

void mainWindow::sbRouteTriggered(int sliderAction)
{
 Q_UNUSED (sliderAction);
 int pos = ui->sbRoute->sliderPosition();
 ui->cbRoute->setCurrentIndex(pos);
 if(!bNoDisplay)
  btnDisplayRouteClicked();
}

void mainWindow::cbSortSelectionChanged(int sel)
{
 config->currCity->routeSortType = sel;
 refreshRoutes();
 toolsMenu->close();
}

void mainWindow::newSqliteDbAct_triggered()
{
 m_bridge->processScript("getCenter");
 LatLng* latLng = new LatLng(m_latitude, m_longitude);
 CreateSqliteDatabaseDialog dlg(latLng, this);
 if(dlg.exec())
 {

 }
}
void mainWindow::QueryDialogAct_triggered()
{
 if(!queryDlg)
 {
  queryDlg = new QueryDialog(config, this);
 }
 queryDlg->show();
}

void mainWindow::On_saveImage_clicked()
{
 QString saveFilename = QFileDialog::getSaveFileName(this, "Save as", "Choose a filename", "PNG(*.png);; TIFF(*.tiff *.tif);; JPEG(*.jpg *.jpeg)");

 QString saveExtension = "PNG";
 int pos = saveFilename.lastIndexOf('.');
 if (pos >= 0)
     saveExtension = saveFilename.mid(pos + 1);
 QString ext = "." + saveExtension.toLower();
 if(!saveFilename.endsWith(ext))
  saveFilename.append(ext);

 if(!QPixmap::grabWidget(webView).save(saveFilename, qPrintable(saveExtension)))
 {
  QMessageBox::warning(this, "File could not be saved", "ok", QMessageBox::Ok);
 }
}

void mainWindow::exportRoute()
{
 RouteData rd = routeList.at(ui->cbRoute->currentIndex());
 ExportRouteDialog dlg(rd, config, this);
 int rslt = dlg.exec();
}

void mainWindow::on_showDebugMessages(bool b)
{
 bDisplayWebDebug = b;
}

void mainWindow::on_runInBrowser(bool b)
{
 config->bRunInBrowser = b;
 config->saveSettings();

 if(QMessageBox::information(this, tr("Restart required!"), tr("This option requires that Mapper be restarted.\nDo you wish to restart now?"),QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
 {
  // restart:
  qApp->quit();
  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());}
}

void mainWindow::updateSegmentInfoDisplay(SegmentInfo si)
{
 if (si.segmentId > 0)
 {
  ui->txtStreet->setText( si.streetName);
  ui->txtSegment->setText( si.description);
  if(si.oneWay == "Y")
  {
   ui->chkOneWay->setChecked(true);
   ui->sbTracks->setValue(1);
   ui->sbTracks->setEnabled(false);
  }
  else
  {
   ui->sbTracks->setEnabled(true);
   ui->sbTracks->setValue(si.tracks);
  }
  //txtOneWay.Text = sI.oneWay;
 }
 ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(si.pointList.count()));
}

void mainWindow::On_editSegment_triggered()
{
 SegmentInfo si = cbSegmentInfoList.at(ui->cbSegments->currentIndex());
 EditSegmentDialog dlg(si.segmentId,this);
 dlg.exec();
}
//#ifdef USE_WEBENGINE
void mainWindow::on_linkClicked(QUrl url)
{
 if(QDesktopServices::openUrl(url))
 {
  qDebug() << "browser open of URL " << url.toString() << " successful";
 }
 else
 {
  qDebug() << "browser open of URL " << url.toString() << " failed";
 }
}
//#endif

bool mainWindow::openWebWindow()
{
 // open map display in browser window
#ifndef USE_WEBENGINE
 QUrl startURL = QUrl("qrc:/GoogleMaps.htm");
#else
 QUrl startURL = QUrl("qrc:/GoogleMaps2.htm");
#endif
 QString tempDir = QDir::tempPath();
 qDebug() << "tempPath =" << tempDir;
 QFile* gFile = new QFile(":///GoogleMaps2b.htm");
 if(gFile->open(QIODevice::ReadOnly))
 {
  QTextStream* inStream = new QTextStream(gFile);
  QString text = inStream->readAll();
  text.replace("TEMPDIR", tempDir);
  QFile* tgFile = new QFile(tempDir+QDir::separator()+"GoogleMaps2b.htm");
  if(tgFile->open(QIODevice::WriteOnly))
  {
   QTextStream * outStream = new QTextStream(tgFile);
   *outStream << text;
   tgFile->close();
   gFile->close();

   if(!QFile(tempDir+ QDir::separator() + "qwebchannel.js").exists())
   {
    if(!QFile::copy(":///scripts/qwebchannel.js", tempDir+ QDir::separator() + "qwebchannel.js"))
    {
     qDebug() << "copy failed:" << ":///scripts/qwebchannel.js" << " to " << tempDir+ QDir::separator() + "qwebchannel.js";
     return false;
    }
   }
   QFile file(tempDir+ QDir::separator() + "GoogleMaps.js");
   if(file.exists())
   {
    file.setPermissions(QFile::ReadOther | QFile::WriteOther);
    if(!file.remove())
     qDebug() << "error deleting " << file.fileName() << file.errorString();
   }
   if(!QFile::copy(":///GoogleMaps.js", tempDir+ QDir::separator() + "GoogleMaps.js" ))
   {

    qDebug() << "copy failed:" << ":///GoogleMaps.js" << " to " << tempDir+ QDir::separator() + "GoogleMaps.js" << file.errorString();
    //return false;
   }
   if(!QFile(tempDir+ QDir::separator() + "WebChannel.js").exists())
   {
    if(!QFile::copy(":///scripts/WebChannel.js", tempDir+ QDir::separator() + "WebChannel.js" ))
    {
     qDebug() << "copy failed:" << ":///scripts/WebChannel.js" << " to " << tempDir+ QDir::separator() + "WebChannel.js";
     //return false;
    }
   }
   if(!QFile(tempDir+ QDir::separator() + "ExtDraggableObject.js").exists())
   {
    if(!QFile::copy(":///scripts/ExtDraggableObject.js", tempDir+ QDir::separator() + "ExtDraggableObject.js" ))
    {
     qDebug() << "copy failed:" << ":///scripts/ExtDraggableObject.js" << " to " << tempDir+ QDir::separator() + "ExtDraggableObject.js";
     //return false;
    }
   }
   if(!QFile(tempDir+ QDir::separator() + "opacityControl.js").exists())
   {
    if(!QFile::copy(":///scripts/opacityControl.js", tempDir+ QDir::separator() + "opacityControl.js" ))
    {
     qDebug() << "copy failed:" << ":///scripts/opacityControl.js" << " to " << tempDir+ QDir::separator() + "opacityControl.js";
     //return false;
    }
   }
   if(!QDesktopServices::openUrl(QUrl(tempDir+"/"+"GoogleMaps2b.htm")))
    qDebug() << "open webbrowser failed " << startURL;
   return true;
  }
  else
  {
   qDebug() << "cannot open " << tgFile->fileName();
  }
 }
 else
 {
  qDebug() << "cannot open " << gFile->fileName();
 }
 return false;
}

void mainWindow::onWebSocketClosed()
{
 QMessageBox::critical(this, tr("Browser closed"), tr("The browser window has closed"));
 quit();
}

void mainWindow::on_addGeoreferenced(bool)
{
 AddGeoreferencedDialog* dlg = new AddGeoreferencedDialog(this);
 dlg->exec();
}

void mainWindow::on_overlayHelp()
{
 QDir dir(wikiRoot);
 if(dir.exists())
 {
  QDesktopServices::openUrl(wikiRoot+"/Overlays.html");
 }
}
void mainWindow::on_usingHelp()
{
 QDir dir(wikiRoot);
 if(dir.exists())
 {
  QDesktopServices::openUrl(wikiRoot+"/Documentation.htm");
 }
}

void mainWindow::saveChanges()
{
 routeView->model()->commitChanges();
}
