#include <QtGui>
#include "mainwindow.h"
#include "removecitydialog.h"
#ifndef USE_WEBENGINE
#include <QtWebKit>
#include <QWebFrame>
#include <QWebElementCollection>
#else
#include <QWebEngineHistory>
#include "websocketclientwrapper.h"
#include "websockettransport.h"
//#include <QWebEnginePage>
#include <QWebSocketServer>
#endif
#include "webviewbridge.h"
#include "sql.h"
#include <qfile.h>
#include <qtextstream.h>
#include "dialogcopyroute.h"
#include "dialogrenameroute.h"
#include <QMessageBox>
#include "segmentdlg.h"
#include "modifyroutedatedlg.h"
//#include "exportdlg.h"
#include "editconnectionsdlg.h"
#include "locatestreetdlg.h"
#include "combineroutesdlg.h"
#include "reroutingdlg.h"
#include "createsqlitedatabasedialog.h"
#include "kml.h"
#include <QFileDialog>
#include "exportroutedialog.h"
#include "editcitydialog.h"
#include "systemconsoleaction.h"
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
#include "replacesegmentdialog.h"
#include "segmentselectionwidget.h"
#include "browsecommentsdialog.h"
//#include "exceptions.h"
//#include "logger.h"
#include "routeview.h"
#include "splitsegmentdlg.h"
#include "overlay.h"
#include <QClipboard>
//#include "exceptions.h"
#include "segmentview.h"
#include "otherrouteview.h"
#include "stationview.h"
#include "companyview.h"
#include "tractiontypeview.h"
#include "dupsegmentview.h"
#include "exportdlg.h"
#include "vptr.h"
#include "splitroute.h"
#include "editstation.h"
#include <QPair>
#include "newcitydialog.h"
#include "removecitydialog.h"
#include "modifyroutetractiontypedlg.h"
#include "dialogchangeroute.h"
#include "systemconsole.h"

QString MainWindow::pwd = "";
QString MainWindow::pgmDir = "";

MainWindow::MainWindow(int argc, char * argv[], QWidget *parent) :  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
 ui->setupUi(this);
 _instance = this;
 cityMenu = nullptr;
 QCoreApplication::setOrganizationName("ACK Software");
 QCoreApplication::setApplicationName("Mapper");
 config = Configuration::instance();
 config->getSettings();
 QString cwd = QDir::currentPath();
 wikiRoot = cwd+ QDir::separator()+ "wiki";
 QIcon icon(":/tram-icon.ico");
 setWindowIcon(icon);
 QFileInfo info = QFileInfo(wikiRoot);
 if(!info.exists())
 {
  QFileInfo info2 = QFileInfo(cwd+ QDir::separator()+ "../wiki");
  if(info2.exists())
      wikiRoot = info2.absoluteFilePath();
  else
   qWarning() << "cannot find wiki pages!";
 }

 while(!config->currConnection->isOpen())
 {
  this->setWindowTitle("Mapper - "+ config->currCity->name() + " ("+config->currConnection->description()+")");

  db =config->currConnection->configure();
  if(config->currConnection->isOpen())
  {
   qInfo() << "database is open";
   break;
  }
  editConnections();
 }
 m_latitude = config->currCity->center.lat();
 m_longitude = config->currCity->center.lon();
 m_zoom = config->currCity->zoom;
 m_maptype = config->currCity->mapType;
 m_companyKey = config->currCity->companyKey;

 sql = SQL::instance();

 createBridge();  // create the webViewBridge
 zoomIndicator = new QLabel();
 statusBar()->addPermanentWidget( zoomIndicator);
 geocoderRslt= new QLabel();
 statusBar()->addPermanentWidget(geocoderRslt);

 ui->ssw->initialize();

#ifndef USE_WEBENGINE
 config->bRunInBrowser = false;
#endif

 tempDir = QDir::tempPath()+QDir::separator()+ "Mapper";
#ifdef  Q_OS_WIN
 tempDir.replace("/", QDir::separator());
#endif
#if 0
 QFile* keys = new QFile("Resources/api_keys.txt");
 if(!keys->open(QIODevice::ReadOnly))
  throw FileNotFoundException("keys file not found!");
 QTextStream stream(keys);
 QString keysText = stream.readLine();
 if(keysText.startsWith("//"))
  keysText = stream.readLine();
 keyTokens = keysText.split("|");
 qDebug() << "api_keys copied";
 keys->close();
#endif

 cwd = QDir::currentPath();
#ifndef Q_OS_WIN
 QDir htmlDir("/var/www/html");
 if(!htmlDir.exists())
     QDir().mkpath ("/var/www/html");
#else
 QDir htmlDir(cwd + QDir::separator() + "html");
#endif
 if(!config->bRunInBrowser)
 {
//#ifndef USE_WEBENGINE
//  webView = new QWebView(ui->groupBox_2);
//  webView->setObjectName(QStringLiteral("webView"));
//  webView->setContextMenuPolicy(Qt::NoContextMenu);
//  webView->setUrl(QUrl(QStringLiteral("qrc:/GoogleMaps.htm")));
//#else
  webView = new QWebEngineView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webEngineView"));
  webView->setContextMenuPolicy(Qt::CustomContextMenu);
  webView->setPage(myWebEnginePage = new MyWebEnginePage());
  webView->setMinimumWidth(400);
//  Q_ASSERT(keyTokens.size()>0);
#if 0
#ifdef Q_OS_WIN
  QFileInfo info(tempDir + QDir::separator()+"GoogleMaps2.htm");
#else
  QFileInfo info(htmlDir.path()+"GoogleMaps2.htm");
#endif
//   QList<QPair<QString,QString> > updates;
#ifdef Q_OS_WIN
//   updates = {QPair<QString,QString>("MYAPIKEY",keyTokens.at(0)),
//              QPair<QString,QString>("TEMPDIR",  tempDir)};
  if(verifyAPIKey("./Resources/GoogleMaps2.htm", keyTokens.at(0)))
    copyAndUpdate("./Resources/GoogleMaps2.htm", htmlDir.path(), "");
   else
    copyAndUpdate(":///GoogleMaps2.htm", htmlDir.path(), keyTokens.at(0));
#else
//   updates = {QPair<QString,QString>("MYAPIKEY",keyTokens.at(0)),
//              QPair<QString,QString>("TEMPDIR",  "http://localhost")};
   copyAndUpdate(":///GoogleMaps2.htm", htmlDir.path(), keyTokens.at(0));
#endif
   copyAndUpdate(":///GoogleMaps.js", htmlDir.path(), "");
   copyAndUpdate(":///scripts/ExtDraggableObject.js", htmlDir.path(),"" );
   copyAndUpdate(":///scripts/opacityControl.js", htmlDir.path(),"" );
   copyAndUpdate(":///scripts/qwebchannel.js", htmlDir.path(),"" );
   copyAndUpdate(":///scripts/WebChannel.js", htmlDir.path(),"" );
   //QFile::copy(":///scripts/opacity-slider2.png", htmlDir.path()+ QDir::separator()+"opacity-slider2.png");
   QFile slider(":///scripts/opacity-slider2.png");
   if(slider.exists())
   {
    QFile::remove( htmlDir.path()+ QDir::separator() + "opacity-slider2.png");
    if(!slider.copy( htmlDir.path()+ QDir::separator() + "opacity-slider2.png" ))
       qCritical() << "copy scripts/opacity-slider2.png failed" << slider.errorString();
   }
   else
       qCritical() << "cannot  find opacity-slider2.png";
#endif
#ifdef Q_OS_WIN
  fileUrl = QUrl::fromLocalFile(htmlDir.path() + QDir::separator()+"GoogleMaps2.htm");
#else
   fileUrl = QUrl::fromLocalFile(htmlDir.absolutePath() + QDir::separator()+"GoogleMaps2.htm");
  //fileUrl = QUrl("http://localhost/GoogleMaps2.htm");
#endif
  //if(verifyAPIKey(htmlDir.path() + QDir::separator()+"GoogleMaps2.htm", keyTokens.at(0)))
   webView->load(fileUrl);
//   QFile f = QFile(htmlDir.absolutePath() + QDir::separator()+"GoogleMaps2.htm");
//   if(f.open(QIODevice::ReadOnly))
//   {
//    QTextStream stream(&f);
//    QString htmlText = stream.readAll();
//    webView->setHtml(htmlText, QUrl("http://localHost:80"));
//    webView->show();
//   }
  //else throw Exception("API key invalid");

 }
 else // run in browser
 {
  qDebug() << "preparing to run in browser";
  webView = NULL;
  ui->groupBox_2->setHidden(true);
  openWebWindow();
  //ui->saveImage->setEnabled(false);
 }

#ifdef USE_WEBENGINE
 // setup the QWebSocketServer
 m_server = new QWebSocketServer(QStringLiteral("WebViewBridge"), QWebSocketServer::NonSecureMode);
 if (!m_server->listen(QHostAddress::LocalHost, 12345))
 {
  QString err = m_server->errorString();
  qCritical() <<tr("Failed to open web socket server(%1).").arg(err);
  return;
 }
 connect(m_server, &QWebSocketServer::newConnection, [=]{
  qInfo() << "new connection to browser";
 });
 connect(m_server, &QWebSocketServer::serverError, [=](QWebSocketProtocol::CloseCode closeCode){
  qDebug() << "server error" << m_server->errorString();
 });
 connect(m_server, &QWebSocketServer::acceptError, [=](QAbstractSocket::SocketError socketError){
  qDebug() << "server socket error" << socketError;
 });
 connect(m_server, &QWebSocketServer::closed, [=] {
  qDebug()  << "server closed";
 });
 if(m_server->isListening())
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
 connect(m_clientWrapper, &WebSocketClientWrapper::clientClosed, this, &MainWindow::onWebSocketClosed);

#endif

 if(!config->bRunInBrowser)
  ui->verticalLayout_2->addWidget(webView);

 pwd = QDir::currentPath();
 QFileInfo info3(argv[0]);
 pgmDir = info3.absolutePath();
 //sql->setConfig(config);
 //ui->setupUi(this);
 webViewAction = NULL;
 systemConsoleAction = NULL;

 QUrl dataUrl("http://ubuntu-2:80/public/map_tiles/overlay.lst");
 m_dataCtrl = new FileDownloader(dataUrl, this);
 connect (m_dataCtrl, SIGNAL(downloaded(QString)), this, SLOT(loadAcksoftData(QString)));
//#ifdef WIN32
// m_overlays = new FileDownloader(QUrl("http://localhost/map_tiles/"),this);
//#else
// m_overlays = new FileDownloader(QUrl("http://localhost/tileserver.php?/tms"),this);
//#endif

 // get list of localhost's mbtiles overlays
 m_overlays = new FileDownloader(QUrl("http://localhost/map_tiles/mbtiles.php"),this);
 //m_overlays = new FileDownloader(QUrl("http://localhost/tileserver/"),this);connect(m_overlays, SIGNAL(downloaded()), this, SLOT(loadMbtilesData()));
 connect(m_overlays, SIGNAL(downloaded(QString)), this, SLOT(loadMbtilesData()));

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

 //connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(chkOneWay_Leave(bool)));
 connect(ui->saveImage, SIGNAL(clicked(bool)), this, SLOT(On_saveImage_clicked()));

 //config->saveSettings();

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
  //connect(routeView->model(), SIGNAL(refreshRoutes()), this, SLOT(refreshRoutes()));
  segmentView = new SegmentView(config, this);
  connect(segmentView, SIGNAL(selectSegment(int)), this, SLOT(on_selectSegment(int)));
  otherRouteView =  OtherRouteView::instance(this);
  connect(otherRouteView, SIGNAL(displayRoute(displayRoute)), this, SLOT(On_displayRoute(displayRoute)));
  stationView = new StationView(config, this);
  companyView = new CompanyView(config, this);
  tractionTypeView = new TractionTypeView(config, this);
  dupSegmentView = new DupSegmentView(config, this);
  connect(routeView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  connect(dupSegmentView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  //ui->tabWidget->removeTab(6);

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
//  connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
//  connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(textChanged(QString)), this, SLOT(txtStreetName_TextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(editingFinished()), this, SLOT(txtStreetName_Leave()));
  ui->txtSegment->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->txtSegment, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(txtSegment_customContextMenu(const QPoint&)));
  connect(ui->txtSegment, SIGNAL(textChanged(QString)), this, SLOT(txtSegment_TextChanged(QString)));
  connect(ui->txtSegment, SIGNAL(editingFinished()), this, SLOT(txtSegment_Leave()));
  connect(ui->btnSplit, SIGNAL(clicked()),this, SLOT(btnSplit_Clicked()));
  connect(ui->chkShowOverlay, SIGNAL(clicked(bool)),this, SLOT(chkShowOverlayChanged(bool)));
  if(!config->bRunInBrowser)
   connect(webView, SIGNAL(loadStarted()), this, SLOT(linkActivated()));
  connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(pageBack()));
  //connect(ui->chkOneWay, SIGNAL(clicked(bool)), this, SLOT(chkOneWay_Leave(bool)));
  connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompanySelectionChanged(int)));
  connect(ui->sbRoute, SIGNAL(actionTriggered(int)), this,  SLOT(sbRouteTriggered(int)));
  connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbrLeave()) );
  connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(sbTracks_valueChanged(int)));
//  connect(ui->rbSingle, &QRadioButton::clicked, [=]{
//   saveStreet = ui->cbStreets->currentText();
//   refreshSegmentCB();
//  });
//  connect(ui->rbDouble, &QRadioButton::clicked, [=]{
//   saveStreet = ui->cbStreets->currentText();
//   refreshSegmentCB();
//  });
//  connect(ui->rbBoth, &QRadioButton::clicked, [=]{
//   saveStreet = ui->cbStreets->currentText();
//   refreshSegmentCB();
//  });

//  QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->cbStreets);
//  proxy->setSourceModel(ui->cbStreets->model());
//  // combo's current model must be reparented,
//  // otherwise QComboBox::setModel() will delete it
//  ui->cbStreets->model()->setParent(proxy);
//  ui->cbStreets->setModel(proxy);
//  connect(ui->cbStreets, &QComboBox::editTextChanged, [=]{
//   bCbStreets_text_changed = true;
//  });
//  connect(ui->cbStreets, SIGNAL(editingFinished()), this, SLOT(cbStreets_editingFinished()));
//  connect(ui->cbStreets, SIGNAL(currentIndexChanged(int)), this, SLOT(cbStreets_currentIndexChanged(int)));
  connect(ui->ssw, SIGNAL(segmentSelected(SegmentInfo)), this, SLOT(cbSegmentsSelectedValueChanged(SegmentInfo)));
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
  ui->cbRoute->addAction(refreshRoutesAct);
  ui->cbRoute->addAction(splitRouteAct);
  ui->cbRoute->addAction(updateRouteAct);
  ui->cbRoute->addAction(updateTerminalsAct);

  ui->ssw->cbSegments()->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui->ssw->cbSegments()->addAction(addSegmentToRouteAct);
  ui->ssw->cbSegments()->addAction(addSegmentToNewRouteAct);
  ui->ssw->cbSegments()->addAction(deleteSegmentAct);
  ui->ssw->cbSegments()->addAction(findDupSegmentsAct);
  ui->ssw->cbSegments()->addAction(queryRouteUsageAct);
  ui->ssw->cbSegments()->addAction(findDormantSegmentsAct);
  ui->ssw->cbSegments()->addAction(selectSegmentAct);
  ui->ssw->cbSegments()->addAction(editSegmentAct);
  ui->ssw->cbSegments()->addAction(splitSegmentAct);
  ui->ssw->cbSegments()->addAction(checkSegmentsAct);

  connect(SQL::instance(), &SQL::segmentsChanged, [=]{
   cbSegmentDataList = sql->getSegmentInfoList();
  });

//  connect(ui->cbSegments, SIGNAL(signalFocusOut()), this, SLOT( cbSegments_Leave()));
  connect(ui->cbRoute, SIGNAL(signalFocusOut()), this, SLOT(cbRoutes_Leave()));
  connect(companyView, SIGNAL(dataChanged()), this, SLOT(refreshCompanies()));


  //routeView = new RouteView(this);
//  refreshSegmentCB();
  refreshCompanies();
  ui->cbCompany->setCurrentIndex( ui->cbCompany->findData(config->currCity->companyKey));
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

  ui->btnBack->setVisible(false);

  bDisplayStationMarkers = config->currCity->bDisplayStationMarkers;
  displayStationMarkersAct->setChecked(bDisplayStationMarkers);
  bDisplayTerminalMarkers = config->currCity->bDisplayTerminalMarkers;
  displayTerminalMarkersAct->setChecked(bDisplayTerminalMarkers);
  displayTerminalMarkersToggeled(bDisplayTerminalMarkers);

  displayRouteCommentsAct->setChecked(config->currCity->bDisplayRouteComments);
  geocoderRequestAct->setChecked(config->currCity->bGeocoderRequest);
  m_bridge->processScript("setGeocoderRequest", config->currCity->bGeocoderRequest?"true":"false");

  if(config->currCity->city_overlayMap->count()> 0)
  {
   //QTimer::singleShot(10000, this, SLOT(mapInit()));
   //chkShowOverlayChanged(config->currCity->bShowOverlay);
   ui->chkShowOverlay->setChecked(config->currCity->bShowOverlay);
  }
  else
   ui->chkShowOverlay->setEnabled(false);

//  ui->btnFirst->setEnabled(false);
//  ui->btnNext->setEnabled(false);
//  ui->btnPrev->setEnabled(false);
//  ui->btnLast->setEnabled(false);
//  ui->btnSplit->setEnabled(false);

//  qApp->processEvents();
//  showGoogleMapFeaturesAct->setChecked(config->bShowGMFeatures);
//  showGoogleMapFeatures(config->bShowGMFeatures);
config->saveSettings();
}

/*static*/ MainWindow* MainWindow::_instance = nullptr;
/*static*/ MainWindow* MainWindow::instance() {return _instance;}

void MainWindow::createBridge()
{
 //! The object we will expose to JavaScript engine:
 m_bridge = new WebViewBridge(LatLng(m_latitude, m_longitude), m_zoom, "roadmap", this);
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

void MainWindow::mapInit() // map initialization completed
{
 chkShowOverlayChanged(config->currCity->bShowOverlay);
//ui->chkShowOverlay->setChecked(config->currCity->bShowOverlay);

 if(config->currCity->bUserMap)
  m_bridge->processScript("setOptions");
 else
  m_bridge->processScript("setDefaultOptions");

}

Configuration* MainWindow::getConfiguration()
{
 return config;
}

void MainWindow::reloadMap()
{
 if(webView == NULL)
 {
  webView = new QWebEngineView(ui->groupBox_2);
  webView->setObjectName(QStringLiteral("webEngineView"));
  webView->setContextMenuPolicy(Qt::NoContextMenu);
  webView->setPage(new MyWebEnginePage());
  QUrl fileUrl = QUrl::fromLocalFile(htmlDir.path() + QDir::separator()+"GoogleMaps2.htm");
  //webView->setUrl(fileUrl);
 }
 webView->setUrl(fileUrl);
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

void MainWindow::linkActivated()
{
 ui->btnBack->setVisible(true);
}
void MainWindow::pageBack()
{
 webView->back();
 QWebEngineHistory *hist = webView->history();
 if(!hist->canGoBack())
  ui->btnBack->setVisible(false);
}

//process list of overlays served by http://acksoft.dyndns.biz
void MainWindow::loadAcksoftData(QString err)
{
 QString data;
 data = m_dataCtrl->downloadedData();
 if(data.startsWith("<!DOCTYPE HTML PUBLIC"))
  return;
 if(!err.isEmpty())
  return;
 loadData(data, "acksoft");
}

//process list of mbtiles overlays on localhost.
void MainWindow::loadMbtilesData()
{
 QString data;
 data = m_overlays->downloadedData();
 if(data.startsWith("<!DOCTYPE HTML PUBLIC")) return;
 //if(data.startsWith("<!DOCTYPE html")) return;
 if(!data.startsWith("Exception"))
  loadData(data, "mbtiles");
}

void MainWindow::loadData(QString data, QString source)
{
 QStringList overlays = data.split('\n');

 for(int i=0; i < overlays.count(); i++)
 {
  QString ov = overlays.at(i);
  if(ov.startsWith("#"))
   continue;
  QStringList sl = ov.split("|");
  if(sl.count() < 2 || sl.at(0) == "")
   continue;

  Overlay* overlay = new Overlay(sl.at(0), sl.at(1));
  bool bOk;
  overlay->description = sl.at(2);
  overlay->minZoom= sl.at(3).toInt(&bOk);
  if(!bOk)
   overlay->minZoom = 10;
  overlay->maxZoom= sl.at(4).toInt(&bOk);
  if(!bOk)
   overlay->maxZoom = 16;
  overlay->opacity= sl.at(5).toInt(&bOk);
  if(!bOk)
   overlay->opacity = 65;
  if(sl.count() > 6)
   overlay->setBounds(Bounds(sl.at(6)));
  if(config->currCity->bounds().contains(overlay->bounds()))
   overlay->cityName = config->currCity->name();
  if(sl.count() > 7)
   overlay->sCenter = sl.at(7);
  overlay->source = source;
  if(source == "mbtiles")
   overlay->urls.append("http://localhost/map_tiles/mbtiles.php");
  else
  if(source == "acksoft")
  {
   //overlay->urls.append("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/");
   overlay->urls.append("http://ubuntu-2:80/public/map_tiles/");

   if(!overlay->bounds().isValid())
   {
    QEventLoop loop;
    m_tilemapresource = new FileDownloader("http://ubuntu-2:80/public/map_tiles/" + overlay->name + "/tilemapresource.xml");
    m_tilemapresource->setOverlay(overlay);
    connect(m_tilemapresource, SIGNAL(downloaded(QString)), this, SLOT(processTileMapResource()));
    loop.exec();
   }
   if(config->currCity->name() == overlay->cityName)
    config->currCity->city_overlayMap->insert(overlay->name, overlay);
  }
  QString locatedName = config->lookupCityName(overlay->bounds());
  if(!locatedName.isEmpty())
   overlay->cityName = locatedName;
  config->overlayMap->insert(overlay->cityName+"|"+sl.at(0), overlay); // name

  continue;
 }
 config->saveSettings();
}

void MainWindow::processTileMapResource()
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
   Overlay* ov = m_tilemapresource->overlay();
   if(ov != nullptr)
   {
    QDomElement elem = root.firstChildElement("Title");
    if(!elem.isNull())
     title = elem.text();
    elem = root.firstChildElement("Abstract");
    if(!elem.isNull())
     ov->description = elem.text();
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
       ov->setBounds(bounds);
      }
     }
    }
    elem = root.firstChildElement("TileSets");
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
    qDebug() <<"xml processed: " << ov->name << "descr: " << ov->description << " bounds: " << ov->bounds().toString() << " minZoom: " << ov->minZoom << " maxZoom: " << ov->maxZoom;
   }
  }
 }
}

//#ifdef WIN32
//void MainWindow::loadOverlayData()
//{
// QString data;
// data = m_overlays->downloadedData();
// int ix = 0;
// while(ix > -1)
// {
//  ix = data.indexOf("HREF=\"/map_tiles/", ix);
//  if(ix > 0)
//  {
//   int ix2 = data.indexOf("\"", ix + 17);
//   if(ix2 > 0)
//   {
//    QString name = data.mid(ix+17,ix2-ix-17);
//    if(name.endsWith(".mbtiles"))
//    {
//     name = data.mid(ix+17,ix2-ix-17 - 8);
//     config->localOverlayList.append(name);
//     if(!config->overlayList.contains(name))
//     {
//      Overlay* ov = new Overlay(name);
//      ov->source = "mbtiles";
//      config->overlayList.insert(name, ov);
//      ov->bLocal = true;
//     }
//     else
//     {
//      config->overlayList.value(name)->bLocal = true;
//     }
//    }
//    ix = ix2;
//   }
//  }
// }
//}
////#else
//#endif
void MainWindow::loadOverlayData()
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
    if(!config->overlayMap->contains(name))
    {
     Overlay* ov = new Overlay(config->currCity->name(),name);
     ov->source = "mbtiles";
     ov->urls.append("http://localhost/mbtiles.php");

     ov->bLocal = true;
     config->overlayMap->insert(ov->cityName+"|"+name, ov);

     if(config->currCity->bounds().contains(ov->bounds()))
     {
      if(!config->currCity->city_overlayMap->contains(ov->name))
       config->currCity->city_overlayMap->insert(ov->cityName+"|"+ov->name, ov);
     }
    }
   }
   config->saveSettings();
  }
 }
}
//#endif

void MainWindow::loadOverlay(Overlay* ov)
{
 currentOverlay = ov->name;
 QVariantList objArray;
 objArray << currentOverlay<< ov->opacity << ov->minZoom << ov->maxZoom << ov->source << ov->bounds().toString()<< ov->urls;
 m_bridge->processScript("loadOverlay", objArray);
}

void MainWindow::fillOverlayMenu()
{
 overlayMenu->clear();
 QActionGroup *overlayActionGroup = new QActionGroup(this);
 qDebug() << "building overlayMenu for:" <<config->currCity->name();
 QMapIterator<QString, Overlay*> iter(*config->currCity->city_overlayMap);
 while(iter.hasNext())
 {
  iter.next();
  QString name = iter.key();
  Overlay* ov = iter.value();
  if(!ov->isSelected)
   continue;
  QAction *act = new QAction(name, this);
  act->setData(VPtr<Overlay>::asQVariant(ov));
  act->setCheckable(true);
  act->setStatusTip(ov->description);
  overlayActions.append(act);
  overlayActionGroup->addAction(act);
  overlayMenu->addAction(act);
  if(config->currCity->curOverlayId >= config->currCity->city_overlayMap->count())
   config->currCity->curOverlayId = 0;
  if(config->currCity->curOverlayId >=0 && name == config->currCity->city_overlayMap->values().at(config->currCity->curOverlayId)->name)
   act->setChecked(true);
  if(!ui->chkShowOverlay->isEnabled())
   ui->chkShowOverlay->setEnabled(true);
 }
 connect(overlayActionGroup,SIGNAL(triggered(QAction*)),this, SLOT(newOverlay(QAction*)));
}

//MainWindow::~MainWindow()
//{
//    delete ui;
//}
void MainWindow::addJSObject() {
    // Add m_bridge to JavaScript Frame as member "webViewBridge".
#ifndef USE_WEBENGINE
    webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("webViewBridge"), m_bridge);
#else
 //webView->page()->setWebChannel(channel);
#endif
}
void MainWindow::NotYetInplemented()
{
    //QMessageBox::StandardButton reply;
    QMessageBox::critical(this, tr("Not Yet Implemented"),tr("This feature has not been implemented yet."), QMessageBox::NoButton);
}

void MainWindow::createActions()
{

 aboutAct = new QAction(tr("&About"), this);
 aboutAct->setStatusTip(tr("Show the application's About box"));
 connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

 quitAct = new QAction(tr("&Quit"), this);
 quitAct->setStatusTip(tr("Exit mapper"));
 connect(quitAct, SIGNAL(triggered()), this, SLOT(quit()));

 newCityAct = new QAction(tr("New City"), this);
 newCityAct->setStatusTip(tr("Define a new city."));
 connect(newCityAct, &QAction::triggered, [=]{
     NewCityDialog* newCityDialog = new NewCityDialog(this);
     newCityDialog->show();
     connect(newCityDialog, &NewCityDialog::finished, [=](int result){
         if(result == QDialog::Rejected)
             return;
     });
 });
 removeCityAct = new QAction(tr("Remove city"),this);
 removeCityAct->setStatusTip(tr("Remove city and connections."));
 connect(removeCityAct, &QAction::triggered, [=]{
     RemoveCityDialog* dlg = new RemoveCityDialog();
     dlg->exec();
 });

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

 modifyRouteTractionTypeAct = new QAction(tr("Modify route traction type"), this);
 modifyRouteTractionTypeAct->setStatusTip(tr("Modify the tractionn type on a date"));
 connect(modifyRouteTractionTypeAct, SIGNAL(triggered()), this, SLOT(modifyRouteTractionType()));


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
 connect(updateRouteAct, SIGNAL(triggered()), this, SLOT(on_updateRoute()));

 replaceSegments = new QAction(tr("Replace segments"),this);
 replaceSegments->setStatusTip(tr("Replace list of segments with new list of segments."));
 connect(replaceSegments, &QAction::triggered, [=]{
  ReplaceSegmentDialog* dlg = new ReplaceSegmentDialog();
  dlg->exec();
 });

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
 deleteSegmentAct->setStatusTip(tr("Delete this segment if no longer used"));
 connect(deleteSegmentAct,SIGNAL(triggered()), this, SLOT(btnDeleteSegment_Click()));

 selectSegmentAct=new QAction(tr("Select segment"),this);
 selectSegmentAct->setStatusTip(tr("Select and display segment"));
 //connect(selectSegmentAct,SIGNAL(triggered()), this, SLOT(selectSegment()));
 connect(selectSegmentAct, &QAction::triggered, [=]{
  int segmentId = ui->ssw->cbSegments()->currentData().toInt();
  cbSegmentsSelectedValueChanged(sql->getSegmentInfo(segmentId));
});
 editSegmentAct = new QAction("Edit Segment", this);
 connect(editSegmentAct, SIGNAL(triggered()), this, SLOT(On_editSegment_triggered()));

 findDupSegmentsAct=new QAction(tr("Show duplicate segments view"),this);
 findDupSegmentsAct->setStatusTip(tr("Display a view of duplicate segments, i.e. with similar starting and ending points."));
 connect(findDupSegmentsAct, SIGNAL(triggered()),this, SLOT(findDupSegments()));

 queryRouteUsageAct=new QAction(tr("Query route usage"),this);
 queryRouteUsageAct->setStatusTip((tr("Show routes using this segment")));
 connect(queryRouteUsageAct, &QAction::triggered, [=] {
  if(!queryDlg)
  {
   queryDlg = new QueryDialog(config, this);
  }
  queryDlg->executeQuery(tr("select * from Routes where lineKey = %1").arg(ui->ssw->segmentSelected().segmentId()));
  queryDlg->show();
 });

 addSegmentToRouteAct = new QAction(tr("Add segment to route"), this);
 addSegmentToRouteAct->setStatusTip(tr("Add segment to current route"));
 connect(addSegmentToRouteAct, &QAction::triggered, [=]{
//     int ix = ui->cbSegments->currentIndex();
     SegmentData sd = ui->ssw->segmentSelected();
     sd.setCompanyKey(ui->cbCompany->currentData().toInt());

     int row =         ui->cbRoute->currentIndex();
     if(row < 0)
     {
      // No route selected. call updateRoute instead.
      updateRoute(&sd);
      return;
     }
     RouteData rd = ((RouteData)routeList.at(row));
     bool b = SQL::instance()->addSegmentToRoute(rd.route, rd.name, rd.startDate, rd.endDate,
                                        sd.segmentId(), rd.companyKey,
                                        sd.tractionType(), "?", -1, -1, 0, 0, 0, 0, sd.oneWay(), sd.trackUsage());
    if(b)
    {
        m_bridge->processScript("clearPolyline", QString("%1").arg(sd.segmentId()));
        //SegmentInfo si = sql->getSegmentInfo(segmentId);
        displaySegment(sd.segmentId(), sd.description(), /*sd.oneWay(),*/ /*ttColors[e.tractionType]*/getColor(rd.tractionType), " ", true);

    }
 });

 addSegmentToNewRouteAct = new QAction(tr("Add segment via UpdateRoute"),this);
 addSegmentToNewRouteAct->setStatusTip(tr("Add via UpdateRoute possibly creating new route."));
 connect(addSegmentToNewRouteAct, &QAction::triggered, [=]{
  SegmentData sd = ui->ssw->segmentSelected();
  sd.setRoute(m_routeNbr);
  sd.setRouteName(_rd.name);
  sd.setCompanyKey(ui->cbCompany->currentData().toInt());
  CompanyData* cd = sql->getCompany(sd.companyKey());
  sd.setStartDate(_rd.startDate);
  sd.setEndDate(_rd.endDate);
  if(sd.startDate() < cd->startDate)
   sd.setStartDate(cd->startDate);
  if(sd.endDate() > cd->endDate)
   sd.setEndDate(cd->endDate);

  updateRoute(&sd);
 });

 findDormantSegmentsAct = new QAction(tr("Find dormant segments"),this);
 findDormantSegmentsAct->setStatusTip(tr("Display a lists of segments that are dormant, i.e. not in service"));
// TODO: find dormant segments
 connect(findDormantSegmentsAct, SIGNAL(triggered()), this, SLOT(NotYetInplemented()));

 saveChangesAct = new QAction(tr("Commit changes"), this);
 saveChangesAct->setStatusTip(tr("Commit changes to database"));
 connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(saveChanges()));
 discardChangesAct = new QAction(tr("Discard changes"));
 discardChangesAct->setStatusTip(tr("Discard any changes"));
 connect(discardChangesAct, &QAction::triggered, [=] {
     routeView->model()->discardChanges();
 });

 addRouteAct = new QAction(tr("Add new Route"),this);
 addRouteAct->setStatusTip(tr("Add a new route"));
 connect(addRouteAct, SIGNAL(triggered()), this, SLOT(addRoute()));

 splitSegmentAct = new QAction(tr("Split segment at a date"),this);
 splitSegmentAct->setStatusTip(tr("Split a segment at a date."));
 connect(splitSegmentAct, &QAction::triggered, [=]{
  SplitSegmentDlg* splitSegment = new SplitSegmentDlg(m_SegmentId);
  int ret = splitSegment->exec();
  if(ret == QDialog::Accepted)
   //refreshSegmentCB();
   ui->ssw->refresh();
 });


 addPointModeAct = new QAction(tr("Add point mode"), this);
 addPointModeAct->setStatusTip(tr("Toggle 'add point' mode. If on, points can be added to the currenly selected segment."));
 addPointModeAct->setCheckable(true);
 addPointModeAct->setChecked(false);
 connect(addPointModeAct, SIGNAL(triggered(bool)), this, SLOT(addModeToggled(bool)));

 reloadMapAct = new QAction(tr("Reload Google Maps"), this);
 reloadMapAct->setStatusTip(tr("Reload the Google Maps window"));
 connect(reloadMapAct, SIGNAL(triggered()), this, SLOT(reloadMap()));

 displayStationMarkersAct = new QAction(tr("Display station markers"),this);
 displayStationMarkersAct->setStatusTip(tr("Toggle display of station markers"));
 displayStationMarkersAct->setCheckable(true);
 connect(displayStationMarkersAct, SIGNAL(toggled(bool)), this, SLOT(displayStationMarkersToggeled(bool)));

 displayTerminalMarkersAct = new QAction(tr("Display terminal markers"),this);
 displayTerminalMarkersAct->setStatusTip(tr("Toggle display of terminal markers"));
 displayTerminalMarkersAct->setCheckable(true);
 connect(displayTerminalMarkersAct, SIGNAL(toggled(bool)), this, SLOT(displayTerminalMarkersToggeled(bool)));

 createKmlAct = new QAction(tr("Create Kml file"), this);
 createKmlAct->setStatusTip(tr("Create Kml file for current route. The KML can then be used to display the route on Google Maps or Google Earth."));
 connect(createKmlAct, SIGNAL(triggered()), this, SLOT(on_createKmlFile_triggered()));

 displayRouteCommentsAct= new QAction(tr("Display route comments"), this);
 displayRouteCommentsAct->setStatusTip(tr("Toggle display of route comments"));
 displayRouteCommentsAct->setCheckable(true);
 connect(displayRouteCommentsAct, SIGNAL(toggled(bool)), this, SLOT(displayRouteCommentsToggled(bool)));

 geocoderRequestAct = new QAction(tr("Lookup geocoder requests"),this);
 geocoderRequestAct->setStatusTip(tr("toggle whether to perform geocoder requests"));
 geocoderRequestAct->setCheckable(true);
 connect(geocoderRequestAct, SIGNAL(toggled(bool)), this, SLOT(geocoderRequestToggled(bool)));

 exportDbAct = new QAction(tr("Export database"), this);
 exportDbAct->setStatusTip(tr("Export to another database"));
 connect(exportDbAct, SIGNAL(triggered()), this, SLOT(exportDb()));

 editConnectionsAct = new QAction(tr("Edit Connections..."), this);
 editConnectionsAct->setStatusTip(tr("Select/edit city and manage database connections."));
 connect(editConnectionsAct, SIGNAL(triggered()),this, SLOT(editConnections()));

 manageOverlaysAct = new QAction(tr("Manage Overlays"), this);
 manageOverlaysAct->setStatusTip(tr("Edit overlay info including selecting which available overlays can be displayed as well as defining additional overlays.."));
 connect(manageOverlaysAct, SIGNAL(triggered()), this, SLOT(On_editCityInfo()));

// locateStreetAct = new QAction(tr("Locate Geodb Object"), this);
// locateStreetAct->setStatusTip(tr("Locate, a street, bridge, park or bahanhof. For Berlin only."));
// connect(locateStreetAct, SIGNAL(triggered()), this, SLOT(locateStreet()));

 combineRoutesAct = new QAction(tr("Combine two routes"), this);
 combineRoutesAct->setStatusTip(tr("Combine two routes into one"));
 connect(combineRoutesAct, SIGNAL(triggered()), this, SLOT(combineRoutes()));

 refreshRoutesAct = new QAction(tr("Refresh Routes"),this);
 refreshRoutesAct->setStatusTip(tr("Refresh the routes combobox. Especially after executing manual queries to the database."));
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
 rerouteAct->setStatusTip(tr("Temporarily reroute a route between two dates. After the end date, the route will revert back to the route before the start date"));
 connect(rerouteAct, SIGNAL(triggered()), this, SLOT(rerouteRoute()));

 newSqliteDbAct = new QAction(tr("Create new Sqlite db"),this);
 newSqliteDbAct->setStatusTip(tr("Create a new Sqlite3 database."));
 connect(newSqliteDbAct, SIGNAL(triggered()), this, SLOT(newSqliteDbAct_triggered()));

 queryDialogAct = new QAction(tr("SQL Query dialog"), this);
 queryDialogAct->setStatusTip(tr("Open a window to make SQL queries on the database."));
 connect(queryDialogAct, SIGNAL(triggered()), this, SLOT(QueryDialogAct_triggered()));

 showDebugMessages = new QAction(tr("Show Debug messages"),this);
 showDebugMessages->setCheckable(true);
 showDebugMessages->setStatusTip(tr("If checked, WebViewer debug messages will be displayed. "));
 connect(showDebugMessages, SIGNAL(toggled(bool)), this, SLOT(on_showDebugMessages(bool)));
#ifdef USE_WEBENGINE
 runInBrowserAct = new QAction(tr("Display map in browser"), this);
 runInBrowserAct->setCheckable(true);
 runInBrowserAct->setStatusTip(tr("If checked, map display will be in web browser. You must then restart Mapper"));
 connect(runInBrowserAct, SIGNAL(triggered(bool)), this, SLOT(on_runInBrowser(bool)));
 runInBrowserAct->setChecked(config->bRunInBrowser);
#endif
 addGeoreferencedOverlayAct = new QAction(tr("Edit Overlay list"), this);
 addGeoreferencedOverlayAct->setStatusTip(tr("Open a dialog to edit list of available overlays."));
 connect(addGeoreferencedOverlayAct, SIGNAL(triggered(bool)), this, SLOT(on_addGeoreferenced(bool)));

 browseCommentsAct = new QAction(tr("Browse Comments"), this);
 connect(browseCommentsAct, &QAction::triggered, [=]{
  BrowseCommentsDialog* dlg = new BrowseCommentsDialog();
  dlg->exec();
 });

 exportOverlaysAct = new QAction("Export overlays", this);
 connect(exportOverlaysAct, &QAction::triggered, [=]{
  Overlay::exportXml("./overlays.xml", config->overlayMap->values());
 });

 overlayHelp = new QAction(tr("Overlays"),this);
 overlayHelp->setStatusTip(tr("Help on setting up a new overlay."));
 connect(overlayHelp, SIGNAL(triggered(bool)), this, SLOT(on_overlayHelp()));

 usingMapper = new QAction(tr("Using Mapper"), this);
 usingMapper->setStatusTip(tr("User documentation"));
 connect(usingMapper, SIGNAL(triggered(bool)), this, SLOT(on_usingHelp()));

 setCityBoundsAct = new QAction(tr("Set City Bounds"),this);
 setCityBoundsAct->setStatusTip(tr("Set the display bounds for thiis city/region"));
 connect(setCityBoundsAct, &QAction::triggered, [=]{config->currCity->setCityBounds(m_bridge);});

 setInspectedPageAct = new QAction(tr("Inspect page"),this);
 connect(setInspectedPageAct, &QAction::triggered, [=]{
     myWebEnginePage->setInspectedPage(myWebEnginePage);
 });
 setLoggingAct = new QAction(tr("Log messages to file"), this);
 setLoggingAct->setCheckable(true);
 setLoggingAct->setChecked(config->loggingOn());
 connect(setLoggingAct, &QAction::triggered, [=]{
  if(config->loggingOn())
   config->setLoggingOn(false);
  else
   config->setLoggingOn(true);
 });

 changeRouteNumberAct = new QAction(tr("Change route number"),this);
 changeRouteNumberAct->setStatusTip(tr("Change the route number for a route"));
 connect(changeRouteNumberAct, &QAction::triggered, [=]{
  int row =         ui->cbRoute->currentIndex();
  if(row < 0) return;
  RouteData rd = ((RouteData)routeList.at(row));
  DialogChangeRoute* dlg = new DialogChangeRoute();
  int rslt = dlg->exec();
  if(rslt == QDialog::DialogCode::Accepted )
  {
   int newRoute = dlg->getNumber();
   bool rslt = sql->renumberRoute(rd.alphaRoute, newRoute);
   if(rslt)
   {
    refreshRoutes();
   }
  }
 });

 checkSegmentsAct = new QAction(tr("Check segments"),this);
 checkSegmentsAct->setStatusTip(tr("update ditection, bounds, etc; Update routes usin segments"));
 connect(checkSegmentsAct, &QAction::triggered, [=]{
  sql->checkSegments();
 });

 showGoogleMapFeaturesAct = new QAction(tr("Show Google Map Features"), this);
 showGoogleMapFeaturesAct->setStatusTip(tr("Show or hide Google Maps features of interest."));
 showGoogleMapFeaturesAct->setCheckable(true);
 showGoogleMapFeaturesAct->setChecked(true);
 connect(showGoogleMapFeaturesAct, SIGNAL(toggled(bool)), this, SLOT(showGoogleMapFeatures(bool)));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setToolTipsVisible(true);
    fileMenu->setToolTip(tr(" select quit option tp exit program."));
    saveSettingsAct = new QAction(tr("Save current settings"), this);
    saveSettingsAct->setStatusTip(tr("save current selections an settings."));
    fileMenu->addAction(saveSettingsAct);
    connect(saveSettingsAct, &QAction::triggered, [=]{
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
     config->currCity->companyKey = ui->cbCompany->currentData().toInt();
     config->saveSettings();
    });
    fileMenu->addAction(quitAct);
    QMenu* connectionsMenu = new Menu("City");
    menuBar()->addMenu(connectionsMenu);
    cityMenu = new Menu(tr("&Cities"));
    cityMenu->setToolTipsVisible(true);
    //cityMenu->setToolTip(tr("Select city, connection or manage overlays"));
    cityMenu->setStatusTip(tr("Select which city and database connection to use."));

    connectionsMenu->addMenu(cityMenu);
    connect(cityMenu, SIGNAL(aboutToShow()), this, SLOT(createCityMenu()));
    connectionsMenu->addSeparator();
    //createCityMenu();
    connectionsMenu->addAction(editConnectionsAct);
    connectionsMenu->addAction(manageOverlaysAct);
    connectionsMenu->addAction(newCityAct);
    connectionsMenu->addAction(removeCityAct);

    toolsMenu = new Menu(tr("Tools"));
    toolsMenu->addAction(addRouteAct);
    toolsMenu->addAction(addSegmentAct);
    toolsMenu->addAction(addPointModeAct);
    toolsMenu->addAction(createKmlAct);
    toolsMenu->addAction(exportDbAct);
    //toolsMenu->addAction(locateStreetAct);
    toolsMenu->addAction(reloadMapAct);
    toolsMenu->addAction(refreshRoutesAct);
    toolsMenu->addAction(newSqliteDbAct);
    toolsMenu->addAction(queryDialogAct);
    toolsMenu->addAction(addGeoreferencedOverlayAct);
    toolsMenu->addAction(browseCommentsAct);
    toolsMenu->addAction(exportOverlaysAct);
    toolsMenu->addAction(setCityBoundsAct);
    toolsMenu->addAction(setLoggingAct);
    //toolsMenu->addAction(setInspectedPageAct);

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
    optionsMenu->addAction(showGoogleMapFeaturesAct);

    menuBar()->addMenu(optionsMenu);
    menuBar()->addMenu(toolsMenu);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setToolTipsVisible(true);
//    helpMenu->addAction(webViewAction = new WebViewAction((QObject*)this));
//#ifndef QT_DEBUG
    helpMenu->addAction(systemConsoleAction = new SystemConsoleAction());
    systemConsoleAction->setToolTip(tr("Display, info, error and debug messages"));
//#endif
    helpMenu->addAction(usingMapper);
    helpMenu->addAction(overlayHelp);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);

}
void MainWindow::createCityMenu()
{
 cityMenu->clear();

 for(int i=0; i < config->cityList.count(); i++)
 {
  City* city = config->cityList.at(i);
  QActionGroup *actionGroup = new QActionGroup(this);
  QMenu *connectMenu =new QMenu(city->name());

  for(int j =0; j < city->connections.count(); j++)
  {
   Connection* connection = city->connections.at(j);

   QAction* act = new QAction(connection->description() + "...", this);
   act->setCheckable(true);
   QPair<City*, Connection*>* pair = new QPair<City*, Connection*>(city, connection);
   act->setData(VPtr<QPair<City*, Connection*> >::asQVariant(pair));
   act->setCheckable(true);
   if(city->id == config->currentCityId && connection->id() == config->currCity->curConnectionId)
       act->setChecked(true);
   actionGroup->addAction(act);
   connectMenu->addAction(act);
   connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(newCity(QAction*)));
  }
  cityMenu->addMenu(connectMenu);
 }
}

void MainWindow::On_editCityInfo()
{
 EditCityDialog* dlg = new EditCityDialog(this);
 dlg->exec();
}

void MainWindow::cbRoute_customContextMenu( const QPoint& )
{
    cbRouteMenu.clear();
    cbRouteMenu.addAction(addSegmentAct);
    cbRouteMenu.addAction(combineRoutesAct);
    cbRouteMenu.addAction(copyRouteAct);
    cbRouteMenu.addAction(deleteRouteAct);
    cbRouteMenu.addAction(displayAct);
    cbRouteMenu.addAction(modifyRouteDateAct);
    cbRouteMenu.addAction(modifyRouteTractionTypeAct);
    cbRouteMenu.addAction(renameRouteAct);
    cbRouteMenu.addAction(rerouteAct);
    cbRouteMenu.addAction(routeCommentsAct);
    cbRouteMenu.addAction(splitRouteAct);
    cbRouteMenu.addAction(updateRouteAct);
    cbRouteMenu.addAction(replaceSegments);
    cbRouteMenu.addAction(exportRouteAct);
    cbRouteMenu.addAction(updateTerminalsAct);
    cbRouteMenu.addAction(changeRouteNumberAct);
    updateTerminalsAct->setEnabled(routeView->isSequenced());
    cbRouteMenu.exec(QCursor::pos());
}

void MainWindow::txtSegment_customContextMenu(const QPoint &)
{
 if(ui->txtSegment->text().isEmpty()) return;
 QMenu* menu = new QMenu();
 QAction* edit = new QAction(tr("Edit segment"), this);
 edit->setStatusTip(tr("Edit segment details such as tracks, route type, etc."));
 menu->addAction(edit);
 connect(edit, SIGNAL(triggered(bool)), this, SLOT(On_editSegment_triggered()));
 menu->addAction(splitSegmentAct);
 //menu->addAction(updateRouteAct);
 menu->exec(QCursor::pos());

}

void MainWindow::tab1CustomContextMenu(const QPoint &)
{
    tab1Menu.addAction(saveChangesAct);
    tab1Menu.addAction(discardChangesAct);
    if(!routeView->bUncomittedChanges())
    {
       saveChangesAct->setEnabled(false);
       discardChangesAct->setEnabled(false);
    }
    else
    {
       saveChangesAct->setEnabled(true);
       discardChangesAct->setEnabled(true);
    }
    tab1Menu.exec(QCursor::pos());
}

void MainWindow::webView_customContextMenu(const QPoint &)
{
    QMenu* menu = new QMenu();
    LatLng pos;
    WebViewBridge::instance()->processScript("getRightClick");
    pos = WebViewBridge::instance()->rightClick();
    QAction* currPoint = new QAction(tr("%1, %2").arg(pos.lat()).arg(pos.lon()), this);
    connect(currPoint, &QAction::triggered, [=]{
       QClipboard* clip = QApplication::clipboard();
       clip->setText(tr("%1, %2").arg(pos.lat()).arg(pos.lon()));
    });
    menu->addAction(currPoint);
    menu->exec(QCursor::pos());
}

// New City and/or connection selected.
void MainWindow::newCity(QAction* act )
{
  QPair<City*, Connection*>* pair = VPtr<QPair<City*, Connection*>>::asPtr(act->data());
  City* city = pair->first;
  Connection* connection = pair->second;
  this->setCursor(QCursor(Qt::WaitCursor));
  enableControls(false);
  qApp->processEvents();

  // first, save some settings for the current city
  config->currCity->center = LatLng(m_latitude, m_longitude);
  config->currCity->zoom = m_zoom;
  config->currCity->mapType = m_maptype;
  if(!config->currCity->connections.contains(config->currConnection))
  {
      //   config->currCity->connections.append(config->currConnection);
      //   config->currCity->connectionNames.append(config->currConnection->description());
      config->currCity->addConnection(config->currConnection);
  }

  // Save any changes to currentCity
//    config->cityList.replace(config->currentCityId, config->currCity);
    companyView->clear();
    tractionTypeView->clear();;
    config->currCity->lastRoute = m_routeNbr;
    config->currCity->lastRouteName = m_routeName;
    config->currCity->lastRouteEndDate = m_currRouteEndDate;
    config->currCity->bDisplayStationMarkers = bDisplayStationMarkers;
    config->currCity->bDisplayRouteComments = bDisplayRouteComments;
    config->currCity->bShowOverlay = ui->chkShowOverlay->isChecked();

    // load the new city and configuration
    config->currCity = city;
    config->currentCityId = city->id;
    config->currCity->curConnectionId =connection->id();
    config->currConnection = connection;

    qDebug() << city->name() + "/" + connection->description();
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

    this->setWindowTitle("Mapper - "+ config->currCity->name() + " ("+config->currConnection->description()+")");
    config->saveSettings();

    //refreshSegmentCB();
    ui->ssw->refresh();
    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(config->currCity->companyKey));
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
    this->setCursor(QCursor(Qt::ArrowCursor));
    enableControls(true);
}

void MainWindow::newOverlay(QAction* act)
{
 Overlay* cOv = VPtr<Overlay>::asPtr(act->data());
 cOv->isSelected = true;

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
 for(int i = 0; i < config->currCity->city_overlayMap->count(); i++)
 {
  Overlay* ov = config->currCity->city_overlayMap->values().at(i);
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

void MainWindow::about()
{
 QMessageBox::about(this, tr("About Mapper"),
     tr("The <b>Mapper</b> Written by Allen C. Kempe. "
        "maintain database of streetcar and transit routes.\r\nCompiled with QT version %1").arg(QT_VERSION_STR));
}

void MainWindow::btnDeleteSegment_Click()   //SLOT
{
    //SQL sql;
    SegmentData sd;
//    int ix = ui->cbSegments->currentIndex();
    //            sI = (segmentInfo)segmentInfoList[ix];
    //sI = (SegmentInfo)cbSegmentInfoList.at(ix);
    sd =  ui->ssw->segmentSelected();
    if (sd.segmentId() < 1)
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
        updateIntersection(0, sd.startLat(), sd.startLon());
        updateIntersection(0, sd.endLat(), sd.endLon());
        SegmentData sdDup = sql->getSegmentInSameDirection(sd);

        // Get a list of all routes using this segment.
        QList<SegmentData> segmentDataList = sql->getRouteSegmentsBySegment(sd.segmentId());
        if ( segmentDataList.count() > 0)
        {
            if ( sdDup.segmentId() >= 0)
            {
//                DialogResult rslt = MessageBox.Show(segmentInfoList.Count + " routes are using this segment. A duplicate segment is defined.\n Move these routes to that segment?",
//                    "Segment in use", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Hand);
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Segment in use"));
                msgBox.setInformativeText(QString("%1").arg(segmentDataList.count()) + tr(" routes are using this segment. A duplicate segment is defined.\n Move these routes to that segment?"));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::No);
                int rslt = msgBox.exec();
                switch (rslt)
                {
                    case QMessageBox::Yes:
                        //foreach (routeData rd in segmentInfoList)
                    for(int i =0; i< segmentDataList.count(); i++)
                        {
                        SegmentData rd = segmentDataList.at(i);
                        if (sql->deleteRouteSegment(rd.route(), rd.routeName(), rd.segmentId(),
                                                    rd.startDate().toString("yyyy/MM/dd"),
                                                    rd.endDate().toString("yyyy/MM/dd")) != true)
                            {
                                //infoPanel.Text = "Delete Error";
                                statusBar()->showMessage(tr("Delete failed"));
                                //infoPanel.ForeColor = Color.Red;
                                //System.Media.SystemSounds.Beep.Play();
                                QApplication::beep();
                                return;
                            }
                        if (sql->doesRouteSegmentExist(rd.route(), rd.routeName(), sdDup.segmentId(),
                                                       rd.startDate(), rd.endDate()))
                                continue;
                            if (!sql->addSegmentToRoute(rd.route(), rd.routeName(), rd.startDate(), rd.endDate(), sdDup.segmentId(),
                                                        rd.companyKey(),
                                                        rd.tractionType(), rd.direction(), rd.next(), rd.prev(), rd.normalEnter(),
                                                        rd.normalLeave(), rd.reverseEnter(), rd.reverseLeave(), rd.oneWay(), rd.trackUsage()))
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

        if (sql->deleteSegment(sd.segmentId()))
        {
            //refreshSegmentCB();
         ui->ssw->refresh();
            // Display the new segment in Google Maps
//            Object[] objArray = new Object[1];
//            objArray[0] = sI.SegmentId;
//            webBrowser1.Document.InvokeScript("clearPolyline"); // clears the old line
            QVariantList objArray;
            objArray<<sd.segmentId();
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




void MainWindow::refreshRoutes()
{
    //SQL sql;
    bCbRouteRefreshing = true;
//    int ixCompany = ui->cbCompany->currentIndex();
    int companyKey = 0;
//    if(ixCompany > 0)
//    {
//        //companyKey = ui->cbCompany->itemData(ixCompany).Int;
        companyKey = ui->cbCompany->currentData().toInt();

//    }
    int ix = ui->cbRoute->currentIndex();
    QString currText = ui->cbRoute->currentText();
    ui->cbRoute->clear();

    routeList = sql->getRoutesByEndDate(companyKey);
    if(routeList.isEmpty())
     qDebug() << "no routes selected for company " << companyKey;

    int len = routeList.count();
    for(int i=0; i<len; i++)
    {
         RouteData rd = (RouteData)routeList.at(i);
         QString rdStartDate = rd.startDate.toString("yyyy/MM/dd");
//         if(ui->cbRoute->findText(rd.toString())< 0)
            ui->cbRoute->addItem(rd.toString(),QVariant::fromValue(rd));
         if( rd.toString() == currText)
            ui->cbRoute->setCurrentIndex(i);
    }
    ui->sbRoute->setRange(0, routeList.count());

    if( ui->cbRoute->currentIndex() <= 0 && ix >=0)
        ui->cbRoute->setCurrentIndex(ix);
    bCbRouteRefreshing = false;
}

void MainWindow::txtRouteNbrLeave()
{
    QString txt = ui->txtRouteNbr->text();
    QString txtAlpha = "";
    bool bIsAlpha = false;
//    int ixCompany = ui->cbCompany->currentIndex();
    int companyKey = 0;
//    if(ixCompany > 0)
//    {
//        //companyKey = ui->cbCompany->itemData(ixCompany).Int;
//        companyKey = companyList.at(ixCompany-1)->companyKey;
    companyKey = ui->cbCompany->currentData().toInt();
//    }
    qint32 newRoute = sql->getNumericRoute(txt, &txtAlpha, &bIsAlpha, companyKey);
    routeList = sql->getRoutesByEndDate(companyKey);
    for(int i = routeList.count()-1; i >= 0; i--)
    {
        RouteData rd = (RouteData)routeList.at(i);
//        if(rd.baseRoute != newRoute)
            routeList.removeAt(i);
    }
    int len = routeList.count();
    ui->cbRoute->clear();

    for(int i=0; i<len; i++)
    {
         RouteData rd = (RouteData)routeList.at(i);
         QString rdStartDate = rd.startDate.toString("yyyy/MM/dd");
         ui->cbRoute->addItem(rd.toString(), QVariant::fromValue(rd));
    }
    ui->sbRoute->setRange(0, routeList.count());

    bCbRouteRefreshing = false;
}

void MainWindow::quit()
{
    this->close();
}

QString MainWindow::getColor(qint32 tractionType)
{
 QString color = "#DF01D7";
 //foreach (tractionTypeInfo tti in tractionTypeList)
 for(int i=0; i < tractionTypeList.count(); i++)
 {
  TractionTypeInfo tti = tractionTypeList.values().at(i);
  if (tractionType == tti.tractionType)
      return tti.displayColor;
 }
 return color;
}

void MainWindow::btnClearClicked()
{
    m_bridge->processScript("clearAll", "");
}

void MainWindow::on_createKmlFile_triggered()
{
 int row =         ui->cbRoute->currentIndex();
 RouteData rd = ((RouteData)routeList.at(row));
 //RouteInfo ri = sql->getRoutePoints(rd.route,rd.name, ui->dateEdit->text());
 //RouteInfo ri(rd.route,rd.name, ui->dateEdit->text());
 Kml* kml = new Kml(rd.name,  segmentDataList);
 QString fileName = QFileDialog::getOpenFileName(this,"Create Kml file", QDir::homePath(),"Kml files (*.kml");
 if(!fileName.isEmpty())
 kml->createKml(fileName, "ff0000ff");
}

void MainWindow::btnDisplayRouteClicked()
{
 addModeToggled(false);
 int row =         ui->cbRoute->currentIndex();
 if(row < 0) return;
 RouteData rd = ((RouteData)routeList.at(row));

 On_displayRoute(rd);
}

void MainWindow::On_displayRoute(RouteData rd)
{
 if(!ui->chkNoClear->isChecked())
  btnClearClicked();

 //RouteInfo ri = sql->getRoutePoints(rd.route,rd.name, ui->dateEdit->text());
 //RouteInfo ri = RouteInfo(rd.route,rd.name, ui->dateEdit->text());
 QList<SegmentData> segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route, rd.name, ui->dateEdit->text());
// LatLng startPt =  LatLng();
// LatLng endPt =  LatLng();
// LatLng swPt = LatLng(90,180);
// LatLng nePt = LatLng(-90,-180);
 Bounds bounds = Bounds();
 bool bBoundsValid = false;
 double infoLat=0, infoLon = 0;
 bool bFirst = true;

 QVariantList objArray;
 if (rd.route < 1)
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
 double length = 0.0;
 //foreach (segmentGroup sg in ri.segments)
 for(int i = 0; i< segmentDataList.count(); i++)
 {
  SegmentData sd = segmentDataList.at(i);
  if(sd.segmentId() == 367)
   qDebug() << "halt";
  objArray.clear();
  objArray << sd.segmentId();
  m_bridge->processScript("clearPolyline", objArray);
  QString color = getColor(sd.tractionType());

  QVariantList points;
  for(int i=0; i < sd.pointList().count(); i++)
  {
   LatLng pt = sd.pointList().at(i);
   points.append(pt.lat());
   points.append(pt.lon());
  }
  bBoundsValid = bounds.updateBounds(sd.bounds());

  int dash = 0;
  if(sd.routeType() == Incline)
   dash = 1;
  else if(sd.routeType() == SurfacePRW)
   dash = 2;
  else if(sd.routeType() == Subway)
   dash = 3;
  if(sd.trackUsage().isEmpty()) // fix for MySql not storing field correctly
   sd.setTrackUsage(" ");
  objArray.clear();
  objArray <<   sd.segmentId() << rd.name <<  sd.description() << sd.oneWay() << color << sd.tracks()
             << dash << sd.routeType() << sd.trackUsage() << points.count();
  objArray.append(points);
  m_bridge->processScript("createSegment",objArray);

  //statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(sd.length()).arg(sd.length()*0.621371192));
  if(sd.tracks() == 2)
  {
   if(sd.oneWay() == "Y")
    length += sd.length();
   else
    length += sd.length()*2;
  }
  else
   length += sd.length();
 }
 statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(length).arg(length*0.621371192));

 QString markerType = "green";
 QMap<int, TractionTypeInfo> tractionTypes= sql->getTractionTypes();
 foreach(TractionTypeInfo tti,tractionTypeList.values())
 {
  //tractionTypeInfo tti = (tractionTypeInfo)tractionTypeList.at(i);
  if (tti.tractionType == rd.tractionType)
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
 if(config->currCity->bDisplayStationMarkers)
 {
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
     CommentInfo ci = sql->getComments(sti.infoKey);
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
  }
  if(!ui->chkNoPan->checkState() && bBoundsValid)
  {
   objArray.clear();
   objArray << bounds.swPt().lat() << bounds.swPt().lon() << bounds.nePt().lat() << bounds.nePt().lon();
   m_bridge->processScript("fitMapBounds", objArray);
  }


  //if(!(infoLat == 0 && infoLon ==0))
  {
   QDate dt = QDate::fromString(m_currRouteStartDate, "yyyy/MM/dd");
   dt = sql->getFirstCommentDate(m_routeNbr, dt, rd.companyKey);
   RouteComments rc = sql->getRouteComment(m_routeNbr, dt, -1);
   if(rc.commentKey < 0)
   {
    rc = sql->getRouteComment(0, dt, -1);
   }
   if(rc.ci.comments.isEmpty())
   {
    rc = sql->getNextRouteComment(m_routeNbr, dt, -1);
    if(rc.commentKey < 0)
    {
     rc = sql->getNextRouteComment(0, dt, -1);
    }

   }
   if(rc.pos.lat() && rc.pos.lon())
   {
    infoLat = rc.pos.lat();
    infoLon = rc.pos.lon();
   }
   else {
    m_bridge->processScript("getCenter");
    infoLat = m_latitude;
    infoLon = m_longitude;
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
    int ix = rc.ci.comments.indexOf("text-indent:0px;\">");
    if(ix > 0)
    {
     rc.ci.comments.insert(ix+18, "<b>" + rc.date.toString("yyyy/MM/dd")+ "</b><p><h1>" + rc.routeAlpha + " " + rc.name + "</h1>");
    }
    objArray.clear();
    objArray << infoLat << infoLon << rc.ci.comments << rc.route << rc.date.toString("yyyy/MM/dd") << rc.companyKey ;
    if(bDisplayRouteComments)
    {
        m_bridge->processScript("displayRouteComment", objArray);
        m_bridge->processScript("showRouteComment", bDisplayRouteComments?"true": "false");
    }
   }
  }
  setCursor(Qt::ArrowCursor);
  bFirstSegmentDisplayed=true;
}

void MainWindow::getInfoWindowComments(double lat, double lon, int route, QString date, int func)
{
 QDate dt = QDate::fromString(date, "yyyy/MM/dd");
 RouteComments rc;
 double latitude = lat;
 double longitude = lon;

 if(func < 0)
 {
  rc = sql->getPrevRouteComment(route, dt, -1);
  if(rc.commentKey < 0)
  {
   rc = sql->getPrevRouteComment(0, dt, -1);
  }
 }
 else
 {
  rc = sql->getNextRouteComment(route, dt, -1);
  if(rc.commentKey < 0)
  {
   rc = sql->getNextRouteComment(0, dt, -1);
  }
 }
 if(rc.route >= 0)
 {
  int i = rc.ci.comments.indexOf("</body>");
  if(i > 0)
  {
   RouteComments rcNext = sql->getNextRouteComment(route, rc.date, -1);
   QString sNext;
   if(rcNext.commentKey>=0)
    sNext = "<input type='button' name='next' value='>' onClick='nextRouteComment()'/>";
   RouteComments rcPrev = sql->getPrevRouteComment(route, rc.date, -1);
   QString sPrev;
   if(rcPrev.commentKey >= 0)
     sPrev="<input type='button' name='prev' value='<' onClick='prevRouteComment()'/>";
   rc.ci.comments.insert(i,sPrev+sNext);
  }
  QVariantList objArray;

  objArray.clear();
  if(rc.pos.lat() != 0 && rc.pos.lon() != 0)
  {
   latitude = rc.pos.lat();
   longitude = rc.pos.lon();
  }
  int ix = rc.ci.comments.indexOf("text-indent:0px;\">");
  if(ix > 0)
  {
   rc.ci.comments.insert(ix+18, "<b>" + rc.date.toString("yyyy/MM/dd")+ "</b><p><h1>" + rc.routeAlpha + " " + rc.name + "</h1>");
  }
  objArray << latitude << longitude << rc.ci.comments<< rc.route << rc.date.toString("yyyy/MM/dd");
  m_bridge->processScript("displayRouteComment", objArray);
  m_bridge->processScript("showRouteComment", bDisplayRouteComments?"true": "false");
 }
}

void MainWindow::onCbRouteIndexChanged(int row)
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
 ui->dateEdit->setMinimumDate(_rd.startDate);
 ui->dateEdit->setMaximumDate(_rd.endDate);
 m_routeNbr = _rd.route;
 m_routeName = _rd.name;
 routeDlg->setRouteData(_rd);

 routeView->updateRouteView();

 stationView->model()->setStationList(sql->getStations(m_routeNbr, m_routeName, m_currRouteEndDate));
 stationView->model()->reset();

}

//void MainWindow::onResize()
//{
//   m_bridge->processScript("resizeMap", "");
//}
//bool compareSegmentInfoByName(const SegmentInfo & s1, const SegmentInfo & s2)
//{
// return s1.description < s2.description;
//}

//void MainWindow::refreshSegmentCB()
//{
// QStringList streets;
// QStringList tokens;
// QStringList tokens2;
// QString description;
// QString selectedStreet =ui->cbStreets->currentText();

// bRefreshingSegments = true;
// if(!bCbStreetsRefreshing)
//  refreshStreetsCb();
// ui->cbSegments->clear();
// cbSegmentInfoList = sql->getSegmentInfo();
// cbSegmentDataList = sql->getSegmentDataList();
// qSort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfoByName);
// //foreach (segmentInfo sI in cbSegmentInfoList)
// for(int i=0; i < cbSegmentInfoList.count(); i++)
// {
//  SegmentInfo sI = cbSegmentInfoList.at(i);
//  description = sI.description;
//  tokens = description.split(",");
//  if(tokens.count() > 1)
//  {
//   QString street = tokens.at(0).trimmed();
//   if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
//   if(street == selectedStreet)
//   {
//    // populate streets
//    if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
//       (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
//       ui->rbBoth->isChecked())
//     ui->cbSegments->addItem(sI.toString(), sI.segmentId);
//    continue;
//   }
//   else
//   {
//    if(selectedStreet.trimmed().isEmpty())
//    {
//     // populate streets
//     if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
//        (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
//        ui->rbBoth->isChecked())
//      ui->cbSegments->addItem(sI.toString(), sI.segmentId);
//     continue;
//    }
//   }
//   tokens2 = tokens.at(1).split("to");
//   {
//    for(int i=0; i < tokens2.count(); i++)
//    {
//     QString street = tokens2.at(i);
//     street = tokens.at(0).trimmed();
//     if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
//     //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
//     if(street == selectedStreet)
//     {
//      // populate streets
//      if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
//         (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
//         ui->rbBoth->isChecked())
//       ui->cbSegments->addItem(sI.toString(), sI.segmentId);
//      continue;
//     }
//     else
//     {
//      if(selectedStreet.trimmed().isEmpty())
//      {
//       // populate streets
//       if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
//          (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
//          ui->rbBoth->isChecked())
//        ui->cbSegments->addItem(sI.toString(), sI.segmentId);
//       continue;
//      }
//     }
//    }
//   }
//  }
// }
// if(m_SegmentId >0)
//  ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(m_SegmentId));
// m_bridge->processScript("addModeOff");
// addPointModeAct->setChecked(false);
// bRefreshingSegments = false;
//}

//void MainWindow::refreshStreetsCb()
//{
// QStringList streets;
// QStringList tokens;
// QStringList tokens2;
// QString description;
// QString selectedStreet = ui->cbStreets->currentText();
// bCbStreetsRefreshing = true;

// ui->cbStreets->clear();
// ui->cbStreets->addItem("");
// cbSegmentInfoList = sql->getSegmentInfo();
// for(int i=0; i < cbSegmentInfoList.count(); i++)
// {
//  SegmentInfo sI = cbSegmentInfoList.at(i);
//  description = sI.description;
//  tokens = description.split(",");
//  if(tokens.count() > 1)
//  {
//   QString street = tokens.at(0).trimmed();
//   if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
//   if(!streets.contains(street))
//   {
//    streets.append(street);
//   }
//   tokens2 = tokens.at(1).split("to");
//   {
//    for(int i=0; i < tokens2.count(); i++)
//    {
//     QString street2 = tokens2.at(i);
//     street2 = tokens.at(0).trimmed();
//     if(street2.indexOf("(")) street2= street.mid(0, street2.indexOf("("));
//     //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
//     if(!streets.contains(street2))
//     {
//      streets.append(street2);
//     }
//    }
//   }
//  }
//  else
//  {
//   tokens = sI.description.split(" ");
//  }
// } // end for
// streets.sort();
// ui->cbStreets->addItems(streets);
// ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(selectedStreet));
// bCbStreetsRefreshing = false;
//}

//void MainWindow::cbStreets_editingFinished()
//{
// QString txt = ui->cbStreets->currentText();
// //ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(txt));
// if(bCbStreets_text_changed)
// {
//  ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(txt));
// }
// bCbStreets_text_changed = false;
//}

//void MainWindow::cbStreets_currentIndexChanged(int)
//{
// saveStreet = ui->cbStreets->currentText();
// if(!bRefreshingSegments)
// refreshSegmentCB();
//}

void MainWindow::refreshCompanies()
{
    ui->cbCompany->clear();
    ui->cbCompany->addItem(tr("All companies"),-1);
    companyList = sql->getCompanies();
    for(int i=0; i < companyList.count(); i++)
    {
        CompanyData* cd = companyList.at(i);
        ui->cbCompany->addItem(cd->name, cd->companyKey);
    }
    if(routeDlg != NULL)
     routeDlg->fillCompanies();
    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(m_companyKey));
}

void MainWindow::cbCompanySelectionChanged(int sel)
{
    Q_UNUSED (sel);
    //qint32 companyKey = ui->cbCompany->itemData(sel).Int;
    m_companyKey = ui->cbCompany->currentData().toInt();
    refreshRoutes();
}

/// <summary>
/// Called by Google Maps when selecting a segment
/// </summary>
/// <param name="SegmentId"></param>
void MainWindow::segmentSelected(qint32 pt, qint32 SegmentId)
{
 //SQL sql;
 //webBrowser1.Document.InvokeScript("addModeOff");
 //m_bridge->processScript("addModeOff");
 m_bAddMode = false;
 addPointModeAct->setChecked(false);
 m_SegmentId = SegmentId;
 SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
 if (sd.segmentId() == -1)
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
 lookupStreetName(sd);
 //txtSegment.Text = si.description;
 ui->txtSegment->setText(sd.description());
 ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(sd.pointList().count()));
 //ui->cbSegments->findText(si.toString(), Qt::MatchExactly);
// int ix = ui->cbSegments->findData(SegmentId);
// ui->cbSegments->setCurrentIndex(ix);
 m_points = sd.pointList();
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

 qint32 irx =         ui->cbRoute->currentIndex();
 if (irx >= 0)
 {
  RouteData rd = RouteData();
  rd = (RouteData)routeList.at(irx);
  if(routeDlg)
   routeDlg->setRouteData(rd);

  otherRouteView->showRoutesUsingSegment(SegmentId);
 }
 if (!ui->chkNoPan->checkState())
 {
  m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 }
}

void MainWindow::SetPoint(qint32 i, double lat, double lon)
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

void MainWindow::getArray()
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

void MainWindow::setDebug(QString str)
{
 //infoPanel.Text = str;
 if(bDisplayWebDebug)
 {
  qDebug() << str;
  statusBar()->showMessage(str, 2000);
 }
}

void MainWindow::setLen(qint32 len)
{
    m_nbrPoints = len;
}
void MainWindow::btnFirstClicked()
{
 if(!bFirstSegmentDisplayed)
  return;

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
    SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(sd);
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(0)).lat(), ((LatLng)m_points.at(0)).lon(),m_SegmentId);
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(0)).lat()<<((LatLng)m_points.at(0)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
}

void MainWindow::btnNextClicked()
{
 if(!bFirstSegmentDisplayed)
  return;
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
    SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(sd);
}

void MainWindow::lookupStreetName(SegmentInfo sd)
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

void MainWindow::btnLastClicked()
{
 if(!bFirstSegmentDisplayed)
  return;
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

void MainWindow::btnPrevClicked()
{
 if(!bFirstSegmentDisplayed)
  return;
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
    SegmentInfo sd = sql->getSegmentInfo(m_SegmentId) ;
    lookupStreetName(sd);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
 m_bridge->processScript("getCenter");
 qApp->processEvents(QEventLoop::AllEvents,50);
 if(ui->chkShowOverlay->isChecked())
 {
     m_bridge->processScript("isOverlayLoaded");
     qApp->processEvents(QEventLoop::AllEvents,50);
     QVariant rslt = m_bridge->getRslt();
     //qDebug() << "overlay loaded " << rslt.toString();
     if(config->currCity->city_overlayMap->isEmpty())
         config->currCity->curOverlayId= -1;
     else
         if(config->currCity->curOverlayId >= config->currCity->city_overlayMap->count())
             config->currCity->curOverlayId = 0;
     if( config->currCity->curConnectionId >= 0 && config->currCity->curOverlayId >= 0 && rslt.toString() == "true")
     {
         m_bridge->processScript("overlay.getOpacity");
         qApp->processEvents(QEventLoop::AllEvents,50);
         int opacity = m_bridge->myRslt.toInt();

         City* c = config->currCity;
         Overlay* ov = c->city_overlayMap->values().at(config->currCity->curOverlayId);
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
 config->currCity->companyKey = ui->cbCompany->currentData().toInt();
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
void MainWindow::btnSplit_Clicked()    // SLOT
{
 if(!bFirstSegmentDisplayed)
  return;
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
  //ui->chkOneWay->setChecked(sql->getSegmentOneWay(m_SegmentId) == "Y");


  // Refresh the Segments combobox
  //refreshSegmentCB();
  ui->ssw->refresh();
  refreshRoutes();

  // Display the new segment in Google Maps
  //Object[] objArray = new Object[1];

  // redisplay the original altered segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.SegmentId()));
  //ui->chkOneWay->setChecked("Y"== sql->getSegmentOneWay(segmentDlg.SegmentId()));
  displaySegment(segmentDlg.SegmentId(), ui->txtSegment->text(), /*sql->getSegmentOneWay(segmentDlg.newSegmentId()), */"#b45f04", " ", true);

  // display the new segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.newSegmentId()));
  //ui->chkOneWay->setChecked("Y" == sql->getSegmentOneWay(segmentDlg.newSegmentId()));

  displaySegment(segmentDlg.newSegmentId(), ui->txtSegment->text(), /*sql->getSegmentOneWay(segmentDlg.newSegmentId()), */"#b45f04", " ", false);

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
void MainWindow::displaySegment(qint32 segmentId, QString segmentName, /*QString oneWay,*/ QString color, QString trackUsage, bool bClearFirst)
{
    SegmentInfo sd = sql->getSegmentInfo(segmentId);
    sd.displaySegment(ui->dateEdit->text(),color, trackUsage, bClearFirst);
    m_currPoint = 0;
    bFirstSegmentDisplayed = true;
    ui->lblPoint->setText(QString::number(m_currPoint));


    ui->btnFirst->setEnabled(true);
    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(false);
    ui->btnLast->setEnabled(true);
    ui->btnSplit->setEnabled(false);
    ui->btnDeletePt->setEnabled(true);
    getArray();
    return;
}

//void mainWindow::selectSegment( )
//{
// cbSegmentsSelectedValueChanged(ui->cbSegments->currentData().toInt());
//}

void MainWindow::cbSegmentsSelectedValueChanged(SegmentInfo sd)
{
    if(sd.segmentId() < 0)
        return;
    m_SegmentId = sd.segmentId();
    updateSegmentInfoDisplay(sd);

    routeDlg->setSegmentId( m_SegmentId);
    //webBrowser1.Document.InvokeScript("addModeOn");
    if(sd.startLat() == 0)
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
        objArray << sd.startLat() << sd.startLon();
        m_bridge->processScript("setCenter", objArray);
    }


    displaySegment(m_SegmentId, ui->txtSegment->text(), /*sd.oneWay(),*/ /*sd.oneWay()=="Y" ? "#00FF00" :*/ "#045fb4", " ", true);

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

void MainWindow::txtSegment_TextChanged(QString text)
{
 bSegmentChanged = true;
 int ix = text.indexOf(",");
 QString street;
 if(ix > 0 )
  street = text.mid(0,ix);
 if(street != ui->txtStreet->text())
  ui->txtStreet->setText(street);
}

void MainWindow::txtSegment_Leave( )
{
 //SQL sql;
 if (bSegmentChanged)
 {
  SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
  sql->updateSegmentDetails(m_SegmentId, ui->txtSegment->text(), /*sd.oneWay(),*/ ui->sbTracks->value(), sd.length(), sd.routeType());
  bSegmentChanged = false;
  int segmentId = m_SegmentId;
//  refreshSegmentCB();
//  for(int i=0; i < cbSegmentInfoList.count(); i++)
//  {
//   if(cbSegmentInfoList.at(i).segmentId == segmentId)
//   {
//    ui->cbSegments->setCurrentIndex(i);
//    m_SegmentId = segmentId;
//    break;
//   }
//  }
//  ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(m_SegmentId));
 }
}
void MainWindow::cbSegmentsTextChanged(QString )
{
 b_cbSegments_TextChanged = true;
}

//void MainWindow::cbSegments_Leave()
//{
// if(b_cbSegments_TextChanged ==true)
// {
//  qint32 segmentId = -1;
//  QString text = ui->ssw->cbSegments()->currentText();

//  bool bOk=false;
//  segmentId = text.toInt(&bOk, 10);

////  if (bOk)
////  {
////   //foreach (segmentInfo sI in segmentInfoList)
////   for(int i=0; i< cbSegmentInfoList.count(); i++)
////   {
////    SegmentInfo sI = (SegmentInfo)cbSegmentInfoList.at(i);

////    if (sI.segmentId == segmentId)
////    {
////     //cbSegments.SelectedItem = sI;
////     ui->cbSegments->setCurrentIndex(i);
////     break;
////    }
////   }
////  }
//  ui->ssw->cbSegments()->setCurrentIndex(ui->ssw->cbSegments()->findData(segmentId));
// }
// b_cbSegments_TextChanged =false;

//}

void MainWindow::cbRoutesTextChanged(QString text)
{
    Q_UNUSED(text)
    b_cbRoutes_TextChanged = true;
}

void MainWindow::cbRoutes_Leave()
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

QString MainWindow::getRouteMarkerImagePath(QString route, bool isStart)
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

QString MainWindow::ProcessScript(QString func, QString params)
{
    m_bridge->processScript(func, params);
    return "";
}

void MainWindow::addPoint(int pt, double lat, double lon)
{
    //SQL sql;
    //getArray(); // get points from display.
    SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
    m_points = sd.pointList();
    m_points.append(LatLng(lat, lon));
    m_nbrPoints = m_points.size();
    //int begin = (int)m_nbrPoints - 2, End = (int)m_nbrPoints - 1;

    if(m_nbrPoints == 2 && sd.startLat() == 0 && sd.startLon() == 0)
    {
     sd.addPoint(m_points.at(0));
     sd.setStartLat(m_points.at(0).lat());
     sd.setStartLon(m_points.at(1).lon());
    }
    if(sd.segmentId() == m_SegmentId && m_nbrPoints > 0)
    {
     sd.addPoint(m_points.at(m_nbrPoints-1 ));
    }
    else
    {
     sd.addPoint(m_points.at(0));
    }
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(sd.pointList().count()));
}

/// <summary>
/// Called by Google Maps when a point on a polyline is moved.
/// </summary>
/// <param name="i"></param>
/// <param name="newLat"></param>
/// <param name="newLon"></param>
void MainWindow::movePoint(qint32 segmentId, qint32 i, double newLat, double newLon)
{
 m_SegmentId = segmentId;
 SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);

 if(sd.segmentId() == m_SegmentId)
 {
  sd.movePoint(i, LatLng(newLat, newLon));
 }
 //SQL sql;
 LatLng oldPoint = sql->getPointOnSegment((int)i, m_SegmentId);
 //TODO what about multiple station records?
 StationInfo sti = sql->getStationAtPoint(oldPoint);
 if (sti.stationKey > 0)
 {
     sql->updateStation(sti.stationKey, LatLng(newLat, newLon));
 }
}

// called by webBrowser map initialization to see if an overlay should be loaded
void MainWindow::queryOverlay()
{
//    overlays ov = (overlays)cbOverlays.SelectedItem;
//    if(ov.overlayName != "")
//        webBrowser1.Document.InvokeScript("loadOverlay", new object[] { ov.overlayName, ov.opacity });
    if(!ui->chkShowOverlay->isChecked())
        return;
    if(config->currCity->curOverlayId >= config->currCity->city_overlayMap->count())
     config->currCity->curOverlayId =0;
    if(config->currCity->curOverlayId >= 0)
    {
        Overlay* ov = config->currCity->city_overlayMap->values().at(config->currCity->curOverlayId );
//        m_bridge->processScript("loadOverlay", "'"+ov->name+"',"+QString("%1").arg(ov->opacity)+ ","+ QString::number(ov->minZoom)+ ","+ QString::number(ov->maxZoom)+ ",'"+ ov->source+ "'");
        loadOverlay(ov);
    }
}

void MainWindow::getGeocoderResults(QString array)
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
void MainWindow::updateIntersection(qint32 i, double newLat, double newLon)
{
 Q_UNUSED(i)
 //SQL sql;
 SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
 // get all the points within 100 meters
 //QList<segmentData> myArray = sql->getIntersectingSegments(newLat, newLon, .020, si.routeType);
 QList<SegmentData> myArray = sql->getIntersectingSegments(newLat, newLon, .020, sd.routeType());
 int currSegment = m_SegmentId;
 //foreach (segmentData sd in myArray)
 for(int i=0; i< myArray.count(); i++)
 {
  //segmentData sd= (segmentData)myArray.at(i);
  SegmentData sd = myArray.at(i);
  //QString oneWay = sql->getSegmentOneWay(sd.SegmentId);
  m_SegmentId = sd.segmentId();
  //m_nbrPoints = sql->getNbrPoints(m_SegmentId);
  m_nbrPoints = sd.pointList().count();
  if(sd.whichEnd() == "S")
   movePoint(m_SegmentId, 0, newLat, newLon);
  else
   movePoint(m_SegmentId, m_nbrPoints -1, newLat, newLon);
  //displaySegment(sd.SegmentId, sql->getSegmentDescription(si.SegmentId), oneWay, oneWay == "N" ? "#00FF00" : "#045fb4", true);
  displaySegment(sd.segmentId(), sd.description(), /*sd.oneWay(),*/ /*sd.oneWay() == "N" ? "#00FF00" : */"#045fb4", " ",  true);
 }
 m_SegmentId = currSegment;
}


/// <summary>
/// Called by Google Maps when a point on a polyline is inserted
/// </summary>
/// <param name="i">point to insert after</param>
/// <param name="newLat">New latitude</param>
/// <param name="newLon">New longitude</param>
void MainWindow::insertPoint(int SegmentId, qint32 i, double newLat, double newLon)
{
 segmentSelected(i,SegmentId);
 SegmentInfo sd = sql->getSegmentInfo((int)SegmentId);
 sd.insertPoint(i, LatLng(newLat, newLon));
#if 0
    //SQL sql;
    sql->insertPoint(i, SegmentId, newLat, newLon);
    m_currPoint++;
#endif
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    sd = sql->getSegmentInfo(m_SegmentId);
    lookupStreetName(sd);
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(sd._pointList.count()));
    ui->btnSplit->setEnabled(true);
}

/// <summary>
/// Delete the point at the current marker
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void MainWindow::btnDeletePtClicked()
{
 if(!bFirstSegmentDisplayed)
  return;
 SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);

 if(sd.segmentId() == m_SegmentId)
 {
  if(m_nbrPoints < 3)
  {
   statusBar()->showMessage("only 2 points. use deleteSegment instead!");
   return;
  }
  sd.deletePoint(m_currPoint);
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
  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(sd.pointList().count()));
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
}

void MainWindow::txtStreetName_TextChanged(QString )
{
 bStreetChanged = true;
}

void MainWindow::sbTracks_valueChanged(int)
{
 bStreetChanged = true;
 bSegmentChanged = true;
 txtStreetName_Leave();
}

void MainWindow::txtStreetName_Leave()
{
 if (bStreetChanged && (m_currPoint < m_nbrPoints) && m_SegmentId > -1)
 {
  //segmentData sd = segmentData(m_currPoint, m_SegmentId);
  SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
  if(sd.pointList().count()<2) return;
  sd.setStartLat(sd.pointList().at(0).lat());
  sd.setStartLon(sd.pointList().at(0).lon());
  sd.setEndLat(sd.pointList().at(sd.pointList().count()-1).lat());
  sd.setEndLon(sd.pointList().at(sd.pointList().count()-1).lon());
  bool bUpdate = false;
  if( ui->txtStreet->text().trimmed() != sd.getStreetName())\
  {
   sd.getStreetName() = ui->txtStreet->text().trimmed();
   bUpdate = true;
  }
  if(ui->sbTracks->value() != sd.tracks())
  {
   sd.setTracks(ui->sbTracks->value());
   bUpdate = true;
  }
  if(bUpdate)
  {
   //SQL mySql;
   sql->updateRecord(sd);
   //refreshSegmentCB();
   ui->ssw->refresh();
  }
 }
 bStreetChanged = false;
}

void MainWindow::getZoom(int zoom)
{
    zoomIndicator->setText("Zoom: "+ QString("%1").arg(zoom));
    m_zoom = zoom;
    config->currCity->zoom = zoom;
}

void MainWindow::copyRouteInfo_Click()
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
void MainWindow::splitRoute_Click()
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
void MainWindow::rerouteRoute()
{
    ReroutingDlg rerouteDlg((RouteData)routeList.at(ui->cbRoute->currentIndex()), config, this);
    int rslt = rerouteDlg.exec();
    if (rslt == QDialog::Accepted)
    {
        refreshRoutes();
    }
}

void MainWindow::renameRoute_Click()
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
void MainWindow::modifyRouteDate()
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
#if 1 // not really needed since historically routes could hane multiple types
void MainWindow::modifyRouteTractionType()
{
 ModifyRouteTractionTypeDlg* form = new ModifyRouteTractionTypeDlg();
 form->setConfiguration(config);
 form->setRouteData(routeList, ui->cbRoute->currentIndex());

 qint32 rslt = form->exec();
 if (rslt == form->Accepted)
 {
  refreshRoutes();

  //cbRoutes.SelectedItem = form.RouteData;
  //cbRoutes.SelectedIndex = cbRoutes.FindString(form.RouteData.ToString());
  for(int i=0; i <routeList.count(); i++)
  {
   RouteData rd =routeList.at(i);
   if(rd.toString() == form->getRouteData()->toString())
   {
    ui->cbRoute->setCurrentIndex(i);

    break;
   }
  }
  routeView->updateRouteView();
 }

}
#endif
void MainWindow::addSegment()
{
#if 1 // TODO
    //SQL sql;
    SegmentDlg segmentDlg(config, this);
//TODO    segmentDlg.routeChanged += new routeChangedEventHandler(segmentDlg_routeChanged);
    connect(&segmentDlg, &SegmentDlg::companySelectionChanged, [=](/*int companyKey*/){
     ui->cbCompany->setCurrentIndex( ui->cbCompany->findData(_rd.companyKey));
     refreshRoutes();
    } );
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
     rd = routeList.at(ui->cbRoute->currentIndex());
    segmentDlg.setRouteData (rd);
    if (segmentDlg.exec() == QDialog::Accepted)
    {
        //refreshSegmentCB();
       ui->ssw->refresh();
       m_SegmentId = segmentDlg.newSegmentId();
        m_nbrPoints = 0;
        //??routeDlg.SegmentId = m_SegmentId;
        //        ui->txtSegment->setText(sql->getSegmentDescription(m_SegmentId));

        ui->lblSegment->setText(tr("Segment %1:").arg(m_SegmentId));
        //        ui->chkOneWay->setChecked("Y"== sql->getSegmentOneWay(m_SegmentId));
        SegmentData sd = sql->getSegmentData(rd.route, m_SegmentId, rd.startDate.toString("yyyy/MM/dd"),
                                             rd.endDate.toString("yyyy/MM/dd"));
        updateSegmentInfoDisplay(sd);

        int dash = 0;
        if(sd.routeType() == Incline)
         dash = 1;
        else if(sd.routeType() == SurfacePRW)
         dash = 2;
        else if(sd.routeType() == Subway)
         dash = 3;

        QVariantList objArray;
        objArray << m_SegmentId << segmentDlg.routeName()<<ui->txtSegment->text()
                 <<sd.oneWay()<<getColor(segmentDlg.tractionType())<<sd.tracks() << dash
                << sd.routeType() << sd.trackUsage() << 0;
        m_bridge->processScript("createSegment",objArray);

        //webBrowser1.Document.InvokeScript("addModeOn");
        m_bridge->processScript("addModeOn");
        m_bAddMode = true;

        sd = sql->getSegmentInfo(m_SegmentId);
        lookupStreetName(sd);
        //refreshRoutes();
        //refreshSegmentCB();
    }
#else
    NotYetInplemented();
#endif
}
void MainWindow::deleteRoute()
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

void MainWindow::on_updateRoute()
{
  SegmentData sd = sql->getSegmentData(m_routeNbr, m_SegmentId, _rd.startDate.toString("yyyy/MM/dd"),
                                       _rd.endDate.toString("yyyy/MM/dd"));
  sd.setRouteName(_rd.name);
  updateRoute(&sd);
}

void MainWindow::updateRoute(SegmentData* sd )
{
    //SQL sql;
//        QList<SegmentInfo> segmentInfoList = sql->getSegmentInfo();

    if (!routeDlg)
    {
        routeDlg = new RouteDlg(config, this);
        //routeDlg->Configuration ( config);
        //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
        connect(routeDlg, SIGNAL(SegmentChangedEvent(qint32, qint32)),this, SLOT(segmentChanged(qint32,qint32)));
        //routeDlg->routeChanged += new routeChangedEventHandler(RouteChanged);
        connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs )), this, SLOT(RouteChanged(RouteChangedEventArgs )));
    }

    if(sd)
    {
     try{
     routeDlg->setRouteNbr( sd->route());
     routeDlg->setSegmentId( sd->segmentId());
     routeDlg->setRouteData(*sd);
     routeDlg->show();
     routeDlg->raise();
     routeDlg->activateWindow();
     return;
     }
     catch(IllegalArgumentException)
     {
      return;
     }
    }

    if(m_SegmentId >= 0)
    {
     routeDlg->setRouteNbr( m_routeNbr);
     routeDlg->setSegmentId( m_SegmentId);
     routeDlg->setRouteData(_rd);
     //routeDlg.segmentInfoList = segmentInfoList;
     routeDlg->show();
     routeDlg->raise();
     routeDlg->activateWindow();
    }

}
void MainWindow::updateTerminals()
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

void MainWindow::selectSegment(int seg)
{
 //segmentChanged(seg, seg);

// for(int i=0; i < cbSegmentInfoList.count(); i++)
// {
//  if(cbSegmentInfoList.at(i).segmentId == seg)
//  {
//      ui->cbSegments->setCurrentIndex(i);
//      m_SegmentId = seg;
//      cbSegmentsSelectedValueChanged(i);
//      otherRouteView->showRoutesUsingSegment(seg);
//      break;
//  }
// }
 //ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(seg));
 ui->ssw->setCurrentSegment(seg);
}

void MainWindow::segmentChanged(qint32 changedSegment, qint32 newSegment)
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
     SegmentInfo sd = sql->getSegmentInfo(newSegment);

     displaySegment(newSegment, sd.description(), /*sd.oneWay(),*/ m_segmentColor, " ", true);
    }
}

void MainWindow::segmentStatus(QString str, QString color)
{
    m_segmentStatus = str;
    m_segmentColor = color;
}

//void MainWindow::segmentDlg_routeChanged(qint32 typeOfChange, qint32 route, QString name, QString endDate)
void MainWindow::segmentDlg_routeChanged(RouteChangedEventArgs args)
{
    refreshRoutes();
   if (args.typeOfChange == "Add")
    {
        //foreach (routeData rd in cbRoutes.Items)
        for(int i=0; i<routeList.count(); i++)
        {
            RouteData rd = (RouteData)routeList.at(i);
            if (rd.route == args.routeNbr && rd.name == args.routeName && args.dateEnd == rd.endDate)
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
void MainWindow::RouteChanged(RouteChangedEventArgs args)
{
 //SQL sql;
 refreshRoutes();
 RouteData rd = args.rd;
 for(int i=0; i < routeList.count(); i++)
 {
  RouteData rd1 = routeList.at(i);
  if(rd1.route == args.routeNbr && args.routeName == rd1.name && rd1.endDate == args.dateEnd)
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
  //SegmentInfo si = sql->getSegmentInfo(args.routeSegment);
  displaySegment(args.routeSegment, rd.name, /*rd.oneWay,*/ /*ttColors[e.tractionType]*/getColor(args.tractionType), " ", true);
 }
 routeView->updateRouteView();
}

//// Show Overview window in GoogleMaps
//void MainWindow::chkShowWindow_CheckedChanged()
//{
//    //webBrowser1.Document.InvokeScript("showWindowControl", new object[] { chkShowWindow.Checked });
//    m_bridge->processScript("showWindowControl", ui->chkShowWindow?"true":"false");
//}
void MainWindow::opacityChanged(QString name, qint32 opacity)
{
 Overlay* ov = new Overlay(config->currCity->name(), name, opacity);
    //ov->name = name;
    //ov.opacity = opacity;
    config->setOverlay(ov);
    statusBar()->showMessage(tr("%2 opacity=%1").arg(opacity).arg(name));
    //m_bridge->processScript("setOverlayOpacity", QString::number(opacity));
}
void MainWindow::moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //SQL sql;
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, m_currRouteEndDate);
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate.toString("yyyy/MM/dd"), ti.endDate.toString("yyyy/MM/dd"), segmentId, i == 0 ? "S" : "E", ti.endSegment, ti.endWhichEnd);
}
void MainWindow::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //SQL sql;
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, m_currRouteEndDate);
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate.toString("yyyy/MM/dd"), ti.endDate.toString("yyyy/MM/dd"), ti.startSegment, ti.startWhichEnd, segmentId, i == 0 ? "S" : "E");

}
void MainWindow::addRoute()
{
    if(routeDlg == 0)
        routeDlg = new RouteDlg(config, this);
    routeDlg->setAddMode(true);
    routeDlg->show();
    routeDlg->raise();
    routeDlg->activateWindow();
}
void MainWindow::addModeToggled(bool isChecked)
{
    if(!isChecked)
        m_bridge->processScript("addModeOff");
    else
        m_bridge->processScript("addModeOn");
}

void MainWindow::displayStationMarkersToggeled(bool bChecked)
{
    bDisplayStationMarkers = bChecked;
    //displayStationMarkersAct->setChecked(bDisplayStationMarkers);
    m_bridge->processScript("displayStationMarkers", bChecked?"true":"false");
}
void MainWindow::displayTerminalMarkersToggeled(bool bChecked)
{
    bDisplayTerminalMarkers = bChecked;
    m_bridge->processScript("displayTerminalMarkers", bChecked?"true":"false");
}
void MainWindow::displayRouteCommentsToggled(bool bChecked)
{
    bDisplayRouteComments = bChecked;
    m_bridge->processScript("showRouteComment", bChecked?"true":"false");
}

void MainWindow::geocoderRequestToggled(bool bChecked)
{
    config->currCity->bGeocoderRequest = bChecked;
    m_bridge->processScript("setGeocoderRequest", bChecked?"true":"false");
}

void MainWindow::chkShowOverlayChanged(bool bChecked)
{
 if(config->currCity->city_overlayMap->size() == 0) return;
 if(bChecked && config->currCity->curOverlayId >= 0)
 {
  Overlay* ov = config->currCity->city_overlayMap->values().at(config->currCity->curOverlayId);
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
void MainWindow::setStation(double lat, double lon, qint32 segmentId, qint32 ptIndex)
{
 //SQL sql;
 try
 {
  LatLng pt =  LatLng(lat, lon);
  SegmentInfo sd = sql->getSegmentInfo(segmentId);
  EditStation form(-1, bDisplayStationMarkers, this);
  //form.setConfiguration(config);
  form.setPoint(pt);
  form.setSegmentId(segmentId);
  form.setIndex( ptIndex);
  QDate dt;
  dt = sd.startDate();
  form.setStartDate( dt);
  dt = sd.endDate();
  form.setEndDate( dt);

  QString markerType = "green";
  switch (sd.routeType())
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
  case SurfacePRW:
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
void MainWindow::updateStation(qint32 stationKey, qint32 segmentId)
{
    StationInfo sti = sql->getStationInfo(stationKey);
    //segmentInfo si = sql->getSegmentInfo(segmentId);
    CommentInfo ci =  CommentInfo();
    QVariantList objArray;

    EditStation form(sti.stationKey, bDisplayStationMarkers, this);
    //form.setConfiguration(config);
    //form.setStationId(sti.stationKey);
 form.exec();
}
void MainWindow::moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lon)
{
 SegmentInfo sd = sql->getSegmentInfo(segmentId);
 if(sd.segmentId() < 0)
  qDebug() << tr("invalid segmentId=%1 stationKey=%2").arg(segmentId).arg(stationKey);

 StationInfo sti = sql->getStationInfo(stationKey);
 if(sti.segmentId != -1 && sti.segmentId != segmentId)
 {
  // add another station with the same name but different segment.
  int stationKey = sql->addStation(sti.stationName,LatLng(sti.latitude, sti.longitude),segmentId,sd.startDate().toString("yyyy/MM/dd"),
                                   sd.endDate().toString("yyyy/MM/dd"),sti.geodb_loc_id, sti.infoKey,sd.routeType(),sti.markerType,sti.point);
  CommentInfo ci = sql->getComments(sti.infoKey);
  QVariantList objArray;
  objArray << lat<< lon << (bDisplayStationMarkers?true:false)<<segmentId<<sti.stationName
           <<stationKey<<sti.infoKey<<ci.comments<<sti.markerType;
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
    CommentInfo ci = sql->getComments(sti.infoKey);
    //str = ci.comments;
    m_bridge->processScript("removeStationMarker", QString("%1").arg(stationKey));
    //m_bridge->processScript("addStationMarker",QString("%1").arg(lat,0,'f',8) +","+QString("%1").arg(lon,0,'f',8) +","+(bDisplayStationMarkers?"true":"false")+","+QString("%1").arg(sti.segmentId)+",'"+sti.stationName+"',"+QString("%1").arg(stationKey)+","+QString("%1").arg(sti.infoKey)+",comments,'"+markerType+"'", "comments", ci.comments);
    objArray.clear();
    objArray << lat<< lon << (bDisplayStationMarkers?true:false)<<sti.segmentId<<sti.stationName<<stationKey<<sti.infoKey<<ci.comments<<sti.markerType;
    m_bridge->processScript("addStationMarker",objArray);
}

void MainWindow::moveRouteComment(int route, QString date, double latitude, double longitude, int companyKey)
{
 RouteComments rc = sql->getRouteComment(route, QDate::fromString(date, "yyyy/MM/dd"), companyKey);
 rc.pos = LatLng(latitude, longitude);
 sql->updateRouteComment(rc);
}

void MainWindow::chkOneWay_toggled(bool bChecked)
{
 SegmentInfo sd = sql->getSegmentInfo(m_SegmentId);
 if(bChecked)
 {
  //ui->sbTracks->setEnabled(false);
  ui->sbTracks->setValue(sd.tracks());
 }
 else
 {
  ui->sbTracks->setValue(sd.tracks());
  //ui->sbTracks->setEnabled(true);
 }
 bSegmentChanged = true;
 txtSegment_Leave();
}

//void mainWindow::chkOneWay_Leave(bool bChecked)
//{
// SegmentInfo si = sql->getSegmentInfo(m_SegmentId);
// if(!bChecked)
//  ui->sbTracks->setEnabled(true);

// sql->updateSegmentDescription(m_SegmentId, ui->txtSegment->text(), bChecked?"Y":"N", si.tracks, si.length);
// refreshSegmentCB();
//}

void MainWindow::exportDb()
{
 if(form == nullptr)
 {
  form = new ExportDlg(config, this);
 }
 form->show();
}

void MainWindow::editConnections()
{
 //NotYetInplemented();
 EditConnectionsDlg form( this);
 form.exec();

 createCityMenu();
 this->setWindowTitle("Mapper - "+ config->currCity->name() + " ("+config->currConnection->description()+")");
}

//void MainWindow::locateStreet()
//{
//    LocateStreetDlg form(this);
//    form.exec();
//}

void MainWindow::findDupSegments()
{
    QList<SegmentData> myArray;
    this->setCursor(QCursor(Qt::WaitCursor));
    //foreach(segmentInfo si in segmentInfoList)
    foreach(SegmentData sd, cbSegmentDataList)
    {
//        SegmentInfo si = cbSegmentInfoList.at(i);
        SegmentData sdDup = sql->getSegmentInSameDirection(sd);
//        sdDup.next = si.segmentId;

        if(sdDup.segmentId() > -1)
            myArray.append(sdDup);
        qApp->processEvents();

    }
//    if(ui->tabWidget->count() < 7)
//     ui->tabWidget->addTab(ui->tblDupSegments,"DuplicateSegments");
    dupSegmentView->showDupSegments(myArray);
    this->setCursor(QCursor(Qt::ArrowCursor));
    //NotYetInplemented();

}

void MainWindow::on_webView_statusBarMessage(QString text)
{
    setDebug(text);
}

void MainWindow::on_selectSegment(int segmentId)
{
 ProcessScript("isSegmentDisplayed", QString("%1").arg(segmentId));
 if(m_segmentStatus == "Y")
  ProcessScript("selectSegment", QString("%1").arg(segmentId));
 else
 {
  SegmentInfo sd = sql->getSegmentInfo(segmentId);
  displaySegment(sd.segmentId(), sd.description(), getColor(_rd.tractionType), "B", true);
  ProcessScript("selectSegment", QString("%1").arg(segmentId));
 }
}

void MainWindow::combineRoutes()
{
    CombineRoutesDlg dlg(ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt(), this);
    dlg.exec();
    refreshRoutes();
}

void MainWindow::updateRouteComment()
{
 RouteCommentsDlg routeCommentsDlg(config, this);

 int row =         ui->cbRoute->currentIndex();
 RouteData rd = ((RouteData)routeList.at(row));
 routeCommentsDlg.setCompanyKey(rd.companyKey);
 routeCommentsDlg.setRoute(rd.route);
 routeCommentsDlg.setDate(rd.startDate);
 routeCommentsDlg.exec();
}

void MainWindow::sbRouteTriggered(int sliderAction)
{
 Q_UNUSED (sliderAction);
 int pos = ui->sbRoute->sliderPosition();
 ui->cbRoute->setCurrentIndex(pos);
 if(!bNoDisplay)
  btnDisplayRouteClicked();
}

void MainWindow::cbSortSelectionChanged(int sel)
{
 config->currCity->routeSortType = sel;
 refreshRoutes();
 toolsMenu->close();
}

void MainWindow::newSqliteDbAct_triggered()
{
 m_bridge->processScript("getCenter");
 LatLng* latLng = new LatLng(m_latitude, m_longitude);
 CreateSqliteDatabaseDialog dlg(latLng, this);
 if(dlg.exec())
 {

 }
}
void MainWindow::QueryDialogAct_triggered()
{
 if(!queryDlg)
 {
  queryDlg = new QueryDialog(config, this);
 }
 queryDlg->show();
}

void MainWindow::On_saveImage_clicked()
{
 QString saveFilename;
 if(!config->bRunInBrowser)
 {
  saveFilename = QFileDialog::getSaveFileName(this, "Save as", config->saveImageDir, "PNG(*.png);; TIFF(*.tiff *.tif);; JPEG(*.jpg *.jpeg)");

  QString saveExtension = "PNG";
  int pos = saveFilename.lastIndexOf('.');
  if (pos >= 0)
      saveExtension = saveFilename.mid(pos + 1);
  QString ext = "." + saveExtension.toLower();
  if(!saveFilename.endsWith(ext))
   saveFilename.append(ext);

  if(!QWidget::grab(webView->rect()).save(saveFilename, qPrintable(saveExtension)))
  {
   QMessageBox::warning(this, "File could not be saved", "ok", QMessageBox::Ok);
  }
 }
 else {
    m_bridge->processScript("screenshot");
 }
 QFileInfo info(saveFilename);
 config->saveImageDir= info.absolutePath();
 config->saveSettings();
}

void MainWindow::exportRoute()
{
 RouteData rd = routeList.at(ui->cbRoute->currentIndex());
 ExportRouteDialog dlg(rd, config, this);
 int rslt = dlg.exec();
}

void MainWindow::on_showDebugMessages(bool b)
{
 bDisplayWebDebug = b;
}

void MainWindow::on_runInBrowser(bool b)
{
 config->bRunInBrowser = b;
 config->saveSettings();

 if(QMessageBox::information(this, tr("Restart required!"), tr("This option requires that Mapper be restarted.\nDo you wish to restart now?"),QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
 {
  // restart:
  qApp->quit();
  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());}
}

void MainWindow::updateSegmentInfoDisplay(SegmentInfo sd)
{
 if (sd.segmentId() > 0)
 {
  ui->txtStreet->setText( sd.streetName());
  ui->txtSegment->setText( sd.description());
  //  if(si.oneWay == "Y")
  //  {
  ////   ui->chkOneWay->setChecked(true);
  //   ui->sbTracks->setValue(1);
  //   ui->sbTracks->setEnabled(false);
  //  }
  //  else
  //  {
  //   ui->sbTracks->setEnabled(true);
  ui->sbTracks->setValue(sd.tracks());
  //  }
  //txtOneWay.Text = sI.oneWay;
 }
 ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_SegmentId).arg(sd.pointList().count()));
}

void MainWindow::On_editSegment_triggered()
{
 SegmentInfo si = sql->getSegmentInfo(ui->ssw->cbSegments()->currentData().toInt());
 SegmentData sd = sql->getSegmentData(m_routeNbr,si.segmentId(),m_currRouteStartDate, m_currRouteEndDate);

 EditSegmentDialog* dlg;
 if(sd.route() >= 0)
  dlg = new EditSegmentDialog(&sd, si,this);
 else
  dlg = new EditSegmentDialog(si,this);
 int ret = dlg->exec();
 if(ret == QDialog::Accepted)
  //refreshSegmentCB();
  ui->ssw->refresh();
}

//#ifdef USE_WEBENGINE
void MainWindow::on_linkClicked(QUrl url)
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

bool MainWindow::openWebWindow()
{
 // open map display in browser window
 QString startHTML = "./Resources/GoogleMaps2b.htm";
 QString startFn = "GoogleMaps2b.htm";

 QString cwd = QDir::currentPath();
#ifdef Q_OS_WIN
 cwd.replace("/", QDir::separator());
#endif
QString loadPath = tempDir;
#ifdef  Q_OS_WIN
 tempDir = cwd+ QDir::separator() + "html";
 tempDir = tempDir.replace("/", "\\");
#else
 tempDir = "/var/www/html/";
 loadPath = "http://localhost/";
#endif
 qDebug() << "openWebWindow: tempPath =" << tempDir;
// QFileInfo tempInfo(tempDir);
// if(!tempInfo.isWritable())
// {
//     QMessageBox::critical(this, tr("Error"), tr(" cannot write to temp directory %1").arg(tempDir));
//     abort();
// }

 qDebug() << "openWebWindow: copyAndUpdate GoogleMaps2b.htm" << " to " << tempDir;
#if 0
 QFileInfo info(tempDir+QDir::separator() + "GoogleMaps2b.htm");
 if(info.exists())
 {

    //#define FORCE_COPY
     if(verifyAPIKey("./Resources/GoogleMaps2b.htm", keyTokens.at(1)))
       copyAndUpdate("./Resources/GoogleMaps2b.htm", tempDir, "");
      else
       copyAndUpdate(startHTML, tempDir, keyTokens.at(1));

     qDebug() << "file verified";

      copyAndUpdate(":///scripts/qwebchannel.js", tempDir,"");
      copyAndUpdate(":///GoogleMaps.js", tempDir,"" );
      copyAndUpdate(":///scripts/WebChannel.js", tempDir,"" );
      copyAndUpdate(":///scripts/ExtDraggableObject.js", tempDir, "");
      copyAndUpdate(":///scripts/opacityControl.js", tempDir,"");
      QFile slider(":///scripts/opacity-slider2.png");
      if(slider.exists())
      {
       QFile::remove(tempDir+ QDir::separator() + "opacity-slider2.png" );
      if(!slider.copy( tempDir+ QDir::separator() + "opacity-slider2.png" ))
          qCritical() << "copy scripts/opacity-slider2.png failed" << slider.errorString();
      }
      else
          qCritical() << "cannot  find opacity-slider2.png";
 }
#endif
#ifdef Q_OS_LINUX
  //QFile::copy(":///GoogleMaps2b.htm", "/var/www/html/GoogleMaps2b.htm");
//  copyAndUpdate(cwd + QDir::separator() + "GoogleMaps2b.htm", tempDir, "");
//  copyAndUpdate(cwd + QDir::separator() + "GoogleMaps.js", tempDir, "");
 updateTarget("./html", tempDir);
 fileUrl = QUrl::fromLocalFile(cwd + QDir::separator() + "html"  + QDir::separator() + startFn);

 //fileUrl = QUrl("http://localhost:80/GoogleMaps2b.htm");
#else
    fileUrl = QUrl::fromLocalFile(tempDir  + QDir::separator() + startFn);
#endif

    qDebug() << "open " << fileUrl.toString();
   if(!QDesktopServices::openUrl(fileUrl))
   {
    qDebug() << "open webbrowser failed " << fileUrl.toDisplayString();
    QMessageBox::critical(nullptr, tr("Error"), "open webbrowser failed ");
   }
   return true;

}

bool MainWindow::copyAndUpdate(QString inFile, QString outDir, QString apiKey)
{
    qInfo() << "copyAndUpdate: " << tr(" copy %1 to %2").arg(inFile).arg( outDir);
//#ifdef Q_OS_WIN
// outDir.replace("\\\\", "\\");
//#endif

 QFileInfo in(inFile);
 if(!in.exists())
     qCritical() << "copyAndUpdate: input file not found!"<< inFile;
 QString baseName = inFile.mid(inFile.lastIndexOf("/")+1);
 QFileInfo out(outDir+QDir::separator()+baseName);
//#ifndef FORCE_COPY
 if(out.exists() &&  (out.fileTime(QFileDevice::FileModificationTime)) > in.fileTime(QFileDevice::FileModificationTime))
 {
//  qDebug() << "out file " << baseName << " newer; will not copy it:";
//  qDebug() << " infile path " << in.absoluteFilePath() << " time" << in.fileTime(QFileDevice::FileModificationTime).toString();
//  qDebug() << " outfile path" << out.absoluteFilePath() << " time" << out.fileTime(QFileDevice::FileModificationTime).toString();
  return true;
 }
//#endif
 qDebug() << "copyAndUpdate: infile = " << inFile << "outdir= " << outDir << apiKey;
 QFile* gFile = new QFile(in.absoluteFilePath());
 QFile* tgFile = new QFile(out.absoluteFilePath());
 QString text;
 if(gFile->open(QIODevice::ReadOnly))
 {
  QTextStream* inStream = new QTextStream(gFile);
  text = inStream->readAll();
  if(!apiKey.isEmpty())
   text = text.replace("MYAPIKEY", apiKey);

  if(!tgFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                         QFileDevice::ReadGroup |QFileDevice::WriteGroup |
                         QFileDevice::ReadOther))
  {
   qDebug() <<"error setting permissions on " << tgFile->fileName() << tgFile->errorString();
  }

  if(tgFile->open(QIODevice::WriteOnly))
  {
   QTextStream * outStream = new QTextStream(tgFile);
   if(!apiKey.isEmpty() && text.contains ("MYAPILEY")){
       qCritical() << "apiKey replacement failed";
   }
   *outStream << text;
   //Q_ASSERT(!text.contains("MYAPIKEY"));
   tgFile->flush();
   tgFile->close();
   gFile->close();
   qInfo() << "copyAndUpdate"<< "file " << baseName << " updated successfully line " << __LINE__;
   return true;
  }
  qDebug() << "copyAndUpdate failed writing " << tgFile->fileName() << tgFile->errorString();
 }
 else
 {
  qCritical() << "copyAndUpdate failed copying " << gFile->fileName() << " to " << outDir << " reason" << gFile->errorString();
 }
 return false;
}

bool MainWindow::updateTarget(QString inDir, QString outDir)
{
 QDir in(inDir);
 QStringList names = in.entryList(QDir::Files | QDir::NoDotAndDotDot);
 QFile* gFile;
 QFile* tgFile;

 for(QString inFile : names)
 {
  QFileInfo in(inFile);
  QString baseName = inFile.mid(inFile.lastIndexOf("/")+1);
  QFileInfo out(outDir+QDir::separator()+baseName);
  //#ifndef FORCE_COPY
  if(out.exists() &&  (out.fileTime(QFileDevice::FileModificationTime)) > in.fileTime(QFileDevice::FileModificationTime))
  {
 //  qDebug() << "out file " << baseName << " newer; will not copy it:";
 //  qDebug() << " infile path " << in.absoluteFilePath() << " time" << in.fileTime(QFileDevice::FileModificationTime).toString();
 //  qDebug() << " outfile path" << out.absoluteFilePath() << " time" << out.fileTime(QFileDevice::FileModificationTime).toString();
   return true;
  }
 //#endif
  qDebug() << "copyAndUpdate: infile = " << inFile << "outdir= " << outDir;
  gFile = new QFile(in.absoluteFilePath());
  tgFile = new QFile(out.absoluteFilePath());
  QString text;
  if(gFile->open(QIODevice::ReadOnly))
  {
   QTextStream* inStream = new QTextStream(gFile);
   text = inStream->readAll();

   if(!tgFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                          QFileDevice::ReadGroup |QFileDevice::WriteGroup |
                          QFileDevice::ReadOther))
   {
    qDebug() <<"error setting permissions on " << tgFile->fileName() << tgFile->errorString();
   }

   if(tgFile->open(QIODevice::WriteOnly))
   {
    QTextStream * outStream = new QTextStream(tgFile);
    *outStream << text;
    //Q_ASSERT(!text.contains("MYAPIKEY"));
    tgFile->flush();
    tgFile->close();
    gFile->close();
    qInfo() << "copyAndUpdate"<< "file " << baseName << " updated successfully line " << __LINE__;
    return true;
   }
   qDebug() << "copyAndUpdate failed writing " << tgFile->fileName() << tgFile->errorString();
  }
 }
  qCritical() << "copyAndUpdate failed copying " << gFile->fileName() << " to " << outDir << " reason" << gFile->errorString();
 return false;
}

bool MainWindow::verifyAPIKey(QString path, QString apiKey)
{
    QFile* gFile = new QFile(path);
    if(gFile->open(QIODevice::ReadOnly))
    {
         QTextStream* inStream = new QTextStream(gFile);
         QString text;
        text = inStream->readAll();
        if(text.contains("MYAPIKEY"))
        {
            qCritical() << "apikey not updated!";
            gFile->close();
            return false;
        }
        if(!text.contains(apiKey))
        {
            qCritical() << "apikey not found!";
            gFile->close();
            return false;
        }
        qInfo() << "apikey is correct!";
    }
    else
    {
        qCritical() << "verifyApi: " << path << " not found " << gFile->errorString();
    }
    return true;
}

void MainWindow::onWebSocketClosed()
{
 QMessageBox::critical(this, tr("Browser closed"), tr("The browser window has closed"));
 quit();
}

void MainWindow::on_addGeoreferenced(bool)
{
 AddGeoreferencedDialog* dlg = new AddGeoreferencedDialog(this);
 dlg->exec();
}

void MainWindow::on_overlayHelp()
{
 QDir dir(wikiRoot);
 if(dir.exists())
 {
  QDesktopServices::openUrl(wikiRoot+"/Overlays.html");
 }
}
void MainWindow::on_usingHelp()
{
 QDir dir(wikiRoot);
 if(dir.exists())
 {
  QDesktopServices::openUrl(wikiRoot+"/Documentation.htm");
 }
}

void MainWindow::saveChanges()
{
 routeView->model()->commitChanges();
}

void MainWindow::showGoogleMapFeatures( bool bShow)
{
 if(!bShow)
  m_bridge->processScript("setOption", "{ styles: styles[\"hide\"] }");
 else
  m_bridge->processScript("setOption", "{ styles: styles[\"default\"] }");
 config->bShowGMFeatures = bShow;
}

MyWebEnginePage::MyWebEnginePage(QObject* parent) : QWebEnginePage(parent){
// connect(this, SIGNAL(QWebEnginePage::loadProgress(int)), this,
//                      SLOT(loadProgress(int)));
 connect(this, &QWebEnginePage::loadProgress, [=](int progress){
  qDebug() << "progress "<< progress;
  //setVisible(true);
 });
}

void MainWindow::enableControls( bool b)
{
    ui->tabWidget->setEnabled(b);
    ui->ssw->setEnabled(b);
    ui->cbRoute->setEnabled(b);
    ui->cbCompany->setEnabled(b);
    ui->btnClear->setEnabled(b);
    ui->btnBack->setEnabled(b);
    ui->btnDisplayRoute->setEnabled(b);
    ui->sbRoute->setEnabled(b);
    ui->saveImage->setEnabled(b);
    ui->chkNoClear->setEnabled(b);
    ui->chkNoPan->setEnabled(b);
    ui->chkShowOverlay->setEnabled(b);
    ui->txtStreet->setEnabled(b);
    ui->txtSegment->setEnabled(b);
    ui->sbTracks->setEnabled(b);
    ui->chkAddPt->setEnabled(b);
}
