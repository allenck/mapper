#include "mainwindow.h"
#include "qcompleter.h"
#include "removecitydialog.h"
#include <QWebEngineHistory>
#include "streetstablemodel.h"
#include "websocketclientwrapper.h"
#include <QWebSocketServer>
#include "webviewbridge.h"
#include "sql.h"
#include <qfile.h>
#include <qtextstream.h>
#include "dialogcopyroute.h"
#include "modifyroutedialog.h"
#include <QMessageBox>
#include "segmentdlg.h"
#include "modifyroutedatedlg.h"
#include "editconnectionsdlg.h"
#include "combineroutesdlg.h"
#include "reroutingdlg.h"
#include "createsqlitedatabasedialog.h"
#include "kml.h"
#include <QFileDialog>
#include "exportroutedialog.h"
#include "editcitydialog.h"
#ifdef HAVE_CONSOLE
#include "systemconsole2.h"
#endif
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
#include "routeview.h"
#include "splitsegmentdlg.h"
#include "overlay.h"
#include <QClipboard>
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
#include <QWebEngineSettings>
#include <QFontDialog>
#include "dialogtextedit.h"
#include <QWindow>
#include "dialogeditparameters.h"
#include <QTimer>
#include "splitcompanyroutesdialog.h"
#include <QSystemTrayIcon>
#include "clipboard.h"
#include <QWebEngineCertificateError>
#include "dialogeditstreets.h"
#include "streetview.h"
#include "dialogupdatestreets.h"
#include "dialogpreferences.h"

QString MainWindow::pwd = "";
QString MainWindow::pgmDir = "";

MainWindow::MainWindow(int argc, char * argv[], QWidget *parent) :  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
 ui->setupUi(this);
    //ui->setupUi(this);
    QSystemTrayIcon * sys = new QSystemTrayIcon(this);
    sys->setIcon(QIcon(":/gui/tram-icon.ico"));
    sys->show();
 _instance = this;
 ui->groupBox_2->setVisible(false);
 cityMenu = nullptr;
 QCoreApplication::setOrganizationName("ACK Software");
 QCoreApplication::setApplicationName("Mapper");

 config = Configuration::instance();
 config->getSettings();

 // see if there are any command line overrides
 if(argc >= 3)
 {
     int i = 1;
     while(i < argc)
     {
         QString str = argv[i];
         QString str2;
         if(str.startsWith("-"))
         {
             if(str.at(1)== 'b')
             {
                 if(i+1 < argc)
                 {
                     str2 = argv[i+1];
                     bool bOK;
                     int v = str2.toInt(&bOK);
                     if(bOK)
                         config->bRunInBrowser = v;
                     else
                         config->bRunInBrowser = false;
                     qInfo() << "bRunInBrowser override - set to: " << (config->bRunInBrowser?"true":"false");
                 }
             }
         }
         i++;
     }
}
 enableControls(false);

 cwd = QDir::currentPath();
// config = Configuration::instance();
// config->getSettings();
 changeFonts(config->font);

 wikiRoot = cwd+ QDir::separator()+ "Resources/wiki";
 QIcon icon(":/tram-icon.ico");
 setWindowIcon(icon);
 QFileInfo info = QFileInfo(wikiRoot);
 if(!info.exists())
// {
//  QFileInfo info2 = QFileInfo(cwd+ QDir::separator()+ "../wiki");
//  if(info2.exists())
//      wikiRoot = info2.absoluteFilePath();
//  else
   qWarning() << "cannot find wiki pages!";
// }

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

 cwd = QDir::currentPath();
 qDebug() << "starting with CWD = '" << cwd;

 if(!config->bRunInBrowser)
 {
//  webView = new QWebEngineView(this);
//  ui->horizontalLayout->addWidget(webView);
  openWebViewPanel();
 }
 else // run in browser
 {
  qInfo() << "preparing to run in browser";
  webView = NULL;
  ui->groupBox_2->setHidden(true);
  openBrowserWindow();
  //ui->saveImage->setEnabled(false);
 }

#ifdef USE_WEBENGINE

#endif

// if(!config->bRunInBrowser)
//  ui->verticalLayout_2->addWidget(webView);

 pwd = QDir::currentPath();
 QFileInfo info3(argv[0]);
 pgmDir = info3.absolutePath();
 //sql->setConfig(config);
 //ui->setupUi(this);
 webViewAction = NULL;
 #ifdef HAVE_CONSOLE
 systemConsoleAction = NULL;
#endif

 //QUrl dataUrl("http://ubuntu-2:80/public/map_tiles/overlay.lst");
 QUrl dataUrl(config->tileServerUrl + "overlay.lst");

 m_dataCtrl = new FileDownloader(dataUrl, this);
 connect (m_dataCtrl, SIGNAL(downloaded(QString)), this, SLOT(loadAcksoftData(QString)));

 createActions();
 createMenus();


// centralWidget = new MapView(this);
// setCentralWidget(centralWidget);
 m_bAddMode=false;
 b_cbSegments_TextChanged=false;
 b_cbRoutes_TextChanged=false;
 bStreetChanged=false;
 //bSegmentChanged=false;
 m_segmentId = -1;

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
 ui->tblDupSegments->horizontalHeader()->restoreState(settings.value("dupSegmentsView").toByteArray());
 ui->tblStreetView->horizontalHeader()->restoreState(settings.value("streetView").toByteArray());

 QDir resource("Resources");
 m_resourcePath =  resource.absoluteFilePath("");
 if(!resource.exists())
  resource.mkdir(m_resourcePath);

 //connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(chkOneWay_Leave(bool)));
 connect(ui->saveImage, SIGNAL(clicked(bool)), this, SLOT(On_saveImage_clicked()));

  QUrl startURL = QUrl(QStringLiteral("qrc:/GoogleMaps2b.htm"));


  routeView = new RouteView(this);
  connect(routeView, SIGNAL(refreshRoutes()), this, SLOT(refreshRoutes()));
  //connect(routeView->model(), SIGNAL(refreshRoutes()), this, SLOT(refreshRoutes()));
  segmentView = new SegmentView(config, this);
  connect(segmentView, SIGNAL(selectSegment(int)), this, SLOT(on_selectSegment(int)));
  otherRouteView =  OtherRouteView::instance();
  connect(otherRouteView, SIGNAL(displayRoute(RouteData)), this, SLOT(On_displayRoute(RouteData)));
  stationView = new StationView(config, this);
  companyView = new CompanyView(this);
  connect(companyView->model(), SIGNAL(companySelectionsChanged()), this, SLOT(refreshCompanies()));

  tractionTypeView = new TractionTypeView(this);
  dupSegmentView = new DupSegmentView(this);
  connect(routeView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  connect(dupSegmentView, SIGNAL(selectSegment(int)), this, SLOT(selectSegment(int)));
  connect(m_bridge, &WebViewBridge::on_rightClicked, this,[=](LatLng latLng){
      segmentView->sourceModel->setList(sql->getIntersectingSegments(latLng.lat(), latLng.lon(), .020));
  });

  connect(m_bridge, &WebViewBridge::on_connection_closed, this, [=]{
      qDebug() << "WebViewBridge connection closed.";
  });
  streetView = new StreetView(this);

  // setup routeDlg
  routeDlg = new RouteDlg(this);
  //routeDlg->Configuration ( config);
  //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
  connect(routeDlg, SIGNAL(SegmentChangedEvent(qint32,qint32)),this, SLOT(segmentChanged(qint32,qint32)));
  //routeDlg->routeChanged += new routeChangedEventHandler(RouteChanged);
  connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(RouteChanged(RouteChangedEventArgs)));
  connect(ui->btnDisplayRoute, SIGNAL(clicked()), this, SLOT(btnDisplayRouteClicked()));
  connect(ui->btnFirst, SIGNAL(clicked()), this, SLOT(btnFirstClicked()));
  connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(btnNextClicked()));
  connect(ui->btnPrev, SIGNAL(clicked()), this, SLOT(btnPrevClicked()));
  connect(ui->btnLast, SIGNAL(clicked()), this, SLOT(btnLastClicked()));
  connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(btnClearClicked()));
  connect(ui->btnDeletePt, SIGNAL(clicked()), this, SLOT(btnDeletePtClicked()));
  connect(ui->cbRoute, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbRouteIndexChanged(int)));
  connect(ui->cbRoute, SIGNAL(editTextChanged(QString)), this, SLOT(cbRoutesTextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(textChanged(QString)), this, SLOT(txtStreetName_TextChanged(QString)));
  connect(ui->txtStreet, SIGNAL(editingFinished()), this, SLOT(txtStreetName_Leave()));
  ui->txtSegment->setContextMenu(txtSegment_customContextMenu());
  connect(ui->txtSegment, &EditSegmentDescr::descrUpdated, [=](QString descr, QString street){
      processDescriptionChange(descr, street);
  });
  currentStreetNames = StreetsTableModel::instance()->getStreetnamesList(ui->txtLocation->text());
  QCompleter* nnC = new QCompleter(currentStreetNames, this);
  nnC->setCaseSensitivity(Qt::CaseInsensitive);
  ui->txtNewerName->setCompleter(nnC);
  connect(StreetsTableModel::instance(), &StreetsTableModel::streetInfoChanged,this,[=]{
      currentStreetNames = StreetsTableModel::instance()->getStreetnamesList(ui->txtLocation->text());
  });

  connect(ui->txtSegment, SIGNAL(editingFinished()), this, SLOT(txtSegment_Leave()));
  connect(ui->txtNewerName, &QLineEdit::editingFinished, this,[=]{
      SegmentInfo si = sql->getSegmentInfo(m_segmentId);
      if(ui->txtNewerName->text() != si.newerName() )
      {
          if(!ui->txtNewerName->text().isEmpty())
          {
              ui->txtNewerName->setText(SegmentDescription::updateToken(ui->txtNewerName->text()));
          }
          si.setNewerName(ui->txtNewerName->text());
          sql->updateSegment(&si);

      }
  });
  Clipboard::instance()->setContextMenu(ui->txtNewerName);
  connect(ui->txtLocation, &QLineEdit::editingFinished, this,[=]{
      SegmentInfo si = sql->getSegmentInfo(m_segmentId);
      if(ui->txtLocation->text() != si.location() )
      {
          si.setLocation(ui->txtLocation->text());
          sql->updateSegment(&si);
      }
  });
  connect(m_bridge, &WebViewBridge::on_connection_closed,this,[=]{
  });

  connect(ui->btnSplit, SIGNAL(clicked()),this, SLOT(btnSplit_Clicked()));
  connect(ui->chkShowOverlay, SIGNAL(clicked(bool)),this, SLOT(chkShowOverlayChanged(bool)));
  if(!config->bRunInBrowser)
   connect(webView, SIGNAL(loadStarted()), this, SLOT(linkActivated()));
  connect(ui->btnBack, SIGNAL(clicked()), this, SLOT(pageBack()));
  //connect(ui->chkOneWay, SIGNAL(clicked(bool)), this, SLOT(chkOneWay_Leave(bool)));
  connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompanySelectionChanged(int)));
  connect(ui->sbRoute, SIGNAL(actionTriggered(int)), this,  SLOT(sbRouteTriggered(int)));
  //connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbrLeave()) );
  connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(sbTracks_valueChanged(int)));
  connect(ui->ssw, SIGNAL(segmentSelected(SegmentInfo)), this, SLOT(cbSegmentsSelectedValueChanged(SegmentInfo)));
  // Context menus
  ui->cbRoute->lineEdit()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->cbRoute->lineEdit(), SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(cbRoute_customContextMenu( const QPoint& )));
  connect(ui->cbRoute, &QComboBox::currentIndexChanged, this,[=](int index){
      QColor bkColor = Qt::white;
      if(index >=0)
      {
          const QModelIndex idx = ui->cbRoute->model()->index(index,0);
          bkColor = ui->cbRoute->model()->data(idx, Qt::BackgroundRole).value<QColor>();
      }
      if(bkColor != Qt::red)
      {
          bkColor = Qt::white;
      }
      QPalette p = ui->cbRoute->lineEdit()->palette();
      p.setColor(ui->cbRoute->lineEdit()->backgroundRole(),bkColor);
      ui->cbRoute->lineEdit()->setPalette(p);
  });
  connect(ui->cbRoute, &QComboBox::currentTextChanged, this,[=](QString  text){
      int index = ui->cbRoute->currentIndex();
      QColor bkColor = Qt::white;
      if(index >=0)
      {
          const QModelIndex idx = ui->cbRoute->model()->index(index,0);
          bkColor = ui->cbRoute->model()->data(idx, Qt::BackgroundRole).value<QColor>();
      }
      if(bkColor == Qt::black)
      {
          bkColor = Qt::white;
      }
      QPalette p = ui->cbRoute->lineEdit()->palette();
      p.setColor(ui->cbRoute->lineEdit()->backgroundRole(),bkColor);
      ui->cbRoute->lineEdit()->setPalette(p);
  });

  //connect(ui->tab, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tab1CustomContextMenu(QPoint)));
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
  ui->cbRoute->addAction(describeRouteAct);
  ui->cbCompany->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->cbCompany, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(cbCompany_customContextMenu( const QPoint& )));
  ui->ssw->cbSegments()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->ssw->cbSegments(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCbSegmentsCustomContextMenu(QPoint)));
  connect(ui->ssw->cbSegments()->lineEdit(), &QLineEdit::textEdited, this, [=](QString txt){
      ui->ssw->cbSegments()->lineEdit()->setText(SegmentDescription::updateToken(txt));

  });
  connect(SQL::instance(), &SQL::segmentChanged, [=](const SegmentInfo si){
   cbSegmentInfoList = sql->getSegmentInfoList();
  });

  connect(SQL::instance(), &SQL::routeChange, [=](NotifyRouteChange rc){
   SegmentData* sd = rc.sd();
   if(sd->route()==m_routeNbr  && rc.type()!= SQL::DELETESEG)
    displaySegment(sd->segmentId(),sd->description(), getColor(sd->tractionType()),sd->trackUsage(),true);
  });
//  connect(ui->cbSegments, SIGNAL(signalFocusOut()), this, SLOT( cbSegments_Leave()));
  connect(ui->cbRoute, SIGNAL(signalFocusOut()), this, SLOT(cbRoutes_Leave()));
  //connect(companyView, SIGNAL(dataChanged()), this, SLOT(refreshCompanies()));

  QPalette pal = ui->txtSegment->palette();
  txtSegment_color = pal.color(ui->txtSegment->backgroundRole());

  //routeView = new RouteView(this);
//  refreshSegmentCB();
  refreshCompanies();
  ui->cbCompany->setCurrentIndex( ui->cbCompany->findData(config->currCity->companyKey));
  refreshRoutes();
  //stationView->showStations();
  ui->chkNoPan->setChecked(config->currCity->bNoPanOpt);
  for(int i = 0; i < routeList.count(); i ++ )
  {
   RouteData rd = routeList.at(i);
   if(rd.route() ==config->currCity->lastRoute && rd.routeName() == config->currCity->lastRouteName
      && rd.endDate().toString("yyyy/MM/dd") == config->currCity->lastRouteEndDate)
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
#if 0
  displayTerminalMarkersToggeled(bDisplayTerminalMarkers);

  displayRouteCommentsAct->setChecked(config->currCity->bDisplayRouteComments);
  geocoderRequestAct->setChecked(config->currCity->bGeocoderRequest);
  m_bridge->processScript("setGeocoderRequest", config->currCity->bGeocoderRequest?"true":"false");
#endif
  if(config->currCity->city_overlayMap->count()> 0)
  {
   //QTimer::singleShot(10000, this, SLOT(mapInit()));
   //chkShowOverlayChanged(config->currCity->bShowOverlay);
   ui->chkShowOverlay->setChecked(config->currCity->bShowOverlay);
  }
  else
   ui->chkShowOverlay->setEnabled(false);
  ui->tabWidget->setCurrentIndex(0);
  config->saveSettings();
}

void MainWindow::onCbSegmentsCustomContextMenu(const QPoint &pos)
{
 //ui->ssw->cbSegments()->addAction(addSegmentToRouteAct);
 SegmentInfo si = sql->getSegmentInfo(ui->ssw->cbSegments()->currentData().toInt());
 if(si.segmentId()< 1)
     return;
 SegmentData* sd = new SegmentData(si);
 sd->setRoute(_rd.route());
 sd->setAlphaRoute(_rd.alphaRoute());
 sd->setRouteName(_rd.routeName());
 sd->setStartDate(_rd.startDate());
 sd->setEndDate(_rd.endDate());
 sd->setTractionType(_rd.tractionType());
 sd->setCompanyKey(_rd.companyKey());
 if(sd->startDate() > sd->endDate()) // new route?
  sd->setEndDate(sd->startDate().addDays(1));

 QMenu* menu = ui->ssw->cbSegments()->lineEdit()->createStandardContextMenu();
 if(!SQL::instance()->doesRouteSegmentExist(*sd))
 {
  QMenu* actMenu = addSegmentMenu(sd);
  menu->addMenu(actMenu);
 }
 menu->addAction(addSegmentViaUpdateRouteAct);
 menu->addAction(selectSegmentAct);
 menu->addAction(deleteSegmentAct);
 menu->addAction(sswEditSegmentAct);
 menu->addAction(newSegmentAct);
 menu->addAction(splitSegmentAct);
 menu->addSeparator();
 menu->addAction(findDupSegmentsAct);
 menu->addAction(queryRouteUsageAct);
 menu->addAction(findDormantSegmentsAct);
 menu->addAction(checkSegmentsAct);
 menu->exec(QCursor::pos());
}

QMenu* MainWindow::addSegmentMenu(SegmentData *sd)
{
 QMenu* actMenu = new QMenu(tr("Add segment to route"));
 //menu->addMenu(actMenu);
 QActionGroup* ag = new QActionGroup(this);
 //ui->ssw->cbSegments()->addAction(addSegmentToNewRouteAct);
 if(sd->tracks()== 1)
 {
  QAction* act = new QAction(tr("OneWay"),this);
  act->setCheckable(true);
  act->setData(-1);
  ag->addAction(act);
  actMenu->addAction(act);

  act = new QAction(tr("TwoWay"),this);
  act->setCheckable(true);
  act->setData(0);
  ag->addAction(act);
  actMenu->addAction(act);
 }
 else
 {
  QAction* act = new QAction(tr("TwoWay"),this);
  ag->addAction(act);
  act->setData(0);
  actMenu->addAction(act);

  act = new QAction("OneWay: "+sd->description(),this);
  act->setData(1);
  ag->addAction(act);
  actMenu->addAction(act);

  act = new QAction("OneWay: "+sd->reverseDescription());
  act->setData(2);
  ag->addAction(act);
  actMenu->addAction(act);
  //menu->addMenu(actMenu);
 }
 connect(ag, &QActionGroup::triggered,[=](QAction* act){
     if(sd->segmentId() <=0)
         return;
  switch(act->data().toInt())
  {
  case 0:
   sd->setOneWay(" ");
   sd->setTrackUsage(" ");
   addSegmentToRoute(sd); // two tracks both used
   if(sd->tracks()==2)
   {
    SegmentInfo si = sql->getSegmentInfo(sd->segmentId());
    qDebug() << tr("sd whichend %1 si whichend %2").arg(sd->whichEnd()).arg(si.whichEnd());
    sd->setDoubleDate(si.doubleDate());

    if(sd->startDate() < si.startDate())
     si.setStartDate(sd->startDate() );
    if(sd->endDate() > si.endDate())
     si.setEndDate(sd->endDate());

    if((si.doubleDate() > sd->startDate()) || !si.doubleDate().isValid())
    {
     //sd->setDoubleDate(sd->startDate());
     si.setDoubleDate(si.startDate());
     sql->updateSegment(&si);
    }
   }
   break;
  case 1:
   sd->setOneWay("Y");
   sd->setTrackUsage("R");
   addSegmentToRoute(sd); // two tracks both used
   break;
  case 2:
   sd->setOneWay("Y");
   sd->setTrackUsage("L");
   addSegmentToRoute(sd); // two tracks both used
   break;
  case -1:
   sd->setOneWay(act->isChecked()?"Y":"N");
   addSegmentToRoute(sd); // single track oneway ot not
  }
 });
 m_segmentId = sd->segmentId();
 Q_ASSERT(m_segmentId > 0);
 selectSegment(sd->segmentId());
 return actMenu;
}

/*static*/ MainWindow* MainWindow::_instance = nullptr;
/*static*/ MainWindow* MainWindow::instance() {return _instance;}

void MainWindow::createBridge()
{
 //! The object we will expose to JavaScript engine:
 m_bridge = new WebViewBridge(LatLng(m_latitude, m_longitude), m_zoom, "roadmap", config->mapId, this);
 connect( m_bridge, SIGNAL(movePointSignalX(qint32,qint32,LatLng,QList<LatLng>)), this, SLOT(movePointX(qint32,qint32,LatLng,QList<LatLng>)));
 connect(m_bridge, SIGNAL(addPointSignal(int,double,double)), this, SLOT(addPoint(int,double,double)));
 connect (m_bridge, SIGNAL(insertPointSignal(int,qint32,double,double)), this, SLOT(insertPoint(int,qint32,double,double)));
 connect(m_bridge, SIGNAL(segmentSelectedX(qint32,qint32,QList<LatLng>)), this, SLOT(segmentSelectedX(qint32,qint32,QList<LatLng>)));
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
 {
  m_bridge->processScript("setDefaultOptions");
  if(config->bDisplayRouteOnReload)
    btnDisplayRouteClicked();
 }
 enableControls(true);
}

Configuration* MainWindow::getConfiguration()
{
 return config;
}

void MainWindow::reloadMap()
{
 disconnect(m_clientWrapper, SIGNAL(clientClosed()), this, SLOT(onWebSocketClosed()));
    if(config->bRunInBrowser)
    {
     if(!QDesktopServices::openUrl(fileUrl))
     {
         qCritical() << "open webbrowser failed " << fileUrl.toDisplayString();
         QMessageBox::critical(nullptr, tr("Error"), "open webbrowser failed ");
     }
     connect(m_clientWrapper, SIGNAL(clientClosed()), this, SLOT(onWebSocketClosed()));
     if(!channel)
         setupbridge();
    }
    else
    {
// #ifdef Q_OS_WINDOWS
//      fileUrl = QUrl::fromLocalFile(cwd + QDir::separator() + "Resources" + QDir::separator()+"GoogleMaps2b.htm");
// #else
     fileUrl = QUrl("qrc:/GoogleMaps2b.htm");
//#endif
    webView->setUrl(fileUrl);
    setupbridge();
    webView->page()->setWebChannel(channel);
  }

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

 //showGoogleMapFeatures(false);

}

void MainWindow::initializeGoogleMaps(QUrl url)
{
 qDebug() << "page loaded: " << url.toString();

 QVariantList objArray;
 objArray << "testing echo";
 m_bridge->processScript("echoConsole", objArray);
 m_bridge->processScript("initMap");
 objArray.clear();
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
 delete m_dataCtrl;
 m_dataCtrl = nullptr;
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
   overlay->urls.append("https://localhost/map_tiles/mbtiles.php");
  else
  if(source == "acksoft")
  {
   //overlay->urls.append("http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/");
   //overlay->urls.append("https://ubuntu-2:80/public/map_tiles/");
   overlay->urls.append(config->tileServerUrl);

   if(!overlay->bounds().isValid())
   {
    //QEventLoop loop;
    m_tilemapresource = new FileDownloader("http://ubuntu-2:80/public/map_tiles/" + overlay->name + "/tilemapresource.xml");
    m_tilemapresource->setOverlay(overlay);
    connect(m_tilemapresource, SIGNAL(downloaded(QString)), this, SLOT(processTileMapResource()));
    //loop.exec();
    while(m_tilemapresource)
        qApp->processEvents();
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
    //qDebug() <<"xml processed: " << ov->name << "descr: " << ov->description << " bounds: " << ov->bounds().toString() << " minZoom: " << ov->minZoom << " maxZoom: " << ov->maxZoom;
   }
  }
 }
 m_tilemapresource = nullptr;
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
     ov->urls.append("https://localhost/mbtiles.php");

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
 qDebug() << config->currCity->city_overlayMap;
 qDebug() << "building overlayMenu for:" <<config->currCity->name() << " overlays: " << config->currCity->city_overlayMap->count();
 qDebug() << "curr Overlay id = " << config->currCity->curOverlayId;
 //QList<Overlay* > oList = Overlay::getList(config->currCity);
 QMapIterator<QString, Overlay*> iter(*config->currCity->city_overlayMap);
 while(iter.hasNext())
 {
  iter.next();
  QString name = iter.key();
  Overlay* ov = iter.value();
// for(Overlay* ov : oList)
// {
//   QString name = ov->cityName;

  QAction *act = new QAction(name, this);
  act->setData(VPtr<Overlay>::asQVariant(ov));
  act->setCheckable(true);
  act->setStatusTip(ov->description);
  overlayActions.append(act);
  overlayActionGroup->addAction(act);
  overlayMenu->addAction(act);
  if(config->currCity->curOverlayId >= config->currCity->city_overlayMap->count())
  {
   config->currCity->curOverlayId = 0;
   qDebug() <<  " overlay name = " << config->currCity->city_overlayMap->values().at(config->currCity->curOverlayId)->name;
  }
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

 displayAllRoutesAct = new QAction(tr("Display all routes"),this);
 displayAllRoutesAct->setStatusTip(tr("Display all routes on date"));
 connect(displayAllRoutesAct, SIGNAL(triggered(bool)), this, SLOT(displayAll()));

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

 editRouteSqlAct = new QAction(tr("Edit Route Sql"),this);
 editRouteSqlAct->setStatusTip(tr("edit sql rot route in SQL Query dialog."));
 connect(editRouteSqlAct, &QAction::triggered, this,[=]{
     if(!queryDlg)
     {
         queryDlg = new QueryDialog(config, this);
     }
     queryDlg->raise();
     queryDlg->show();

     queryDlg->processSelect("Routes", tr("select * from Routes where route=%1"
                                          " and startdate = '%2'").arg(_rd.route()).arg(_rd.startDate().toString("yyyy/MM/dd")));
 });

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

 describeRouteAct = new QAction(tr("Describe route"),this);
 describeRouteAct->setStatusTip(tr("Display the route's description. If sequenced."));
 connect(describeRouteAct,SIGNAL(triggered()), this, SLOT(describeRoute()));

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
 editSegmentAct->setStatusTip(tr("Edit asegment's properties"));
 connect(editSegmentAct, SIGNAL(triggered()), this, SLOT(On_editSegment_triggered()));

 sswEditSegmentAct = new QAction("Edit Segment", this);
 sswEditSegmentAct->setStatusTip(tr("Edit asegment's properties"));
 connect(sswEditSegmentAct, &QAction::triggered, [=]{
  m_segmentId = ui->ssw->cbSegments()->currentData().toInt();
     Q_ASSERT(m_segmentId > 0);
  On_editSegment_triggered();
 });


 newSegmentAct = new QAction(tr("New Segment"), this);
 newSegmentAct->setStatusTip(tr("Create new segment not on a route"));
 connect(newSegmentAct, SIGNAL(triggered(bool)), this, SLOT(onNewSegment_triggered()));

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

 companyChangeRoutes = new QAction(tr("ChangeCompany for routes"),this);
 companyChangeRoutes->setStatusTip(tr("Change company for all routes at a date"));
 connect(companyChangeRoutes, &QAction::triggered, [=]{
    SplitCompanyRoutesDialog dlg;
    dlg.setOrginalCompany(ui->cbCompany->currentData().toInt());
    if(dlg.exec() == QDialog::Accepted)
        refreshRoutes();
 });

 addSegmentToRouteAct = new QAction(tr("Add segment to route"), this);
 addSegmentToRouteAct->setStatusTip(tr("Add segment to current route"));
 connect(addSegmentToRouteAct, &QAction::triggered, [=]{
//     int ix = ui->cbSegments->currentIndex();
     SegmentData *sd = new SegmentData(ui->ssw->segmentSelected());
     sd->setRoute(_rd.route());
     sd->setAlphaRoute(_rd.alphaRoute());
     sd->setRouteName(_rd.routeName());
     sd->setStartDate(_rd.startDate());
     sd->setEndDate(_rd.endDate());
     sd->setTractionType(_rd.tractionType());
     sd->setCompanyKey(_rd.companyKey());
     int selectedDegmentId = sd->segmentId(); // this is the pne we hope to add
     // QList<SegmentData*> conflicts
     //         = sql->getConflictingRouteSegments(_rd.route(), _rd.routeName(),
     //                                            _rd.startDate(),
     //                                            _rd.endDate(),
     //                                            _rd.companyKey(),sd->segmentId());
     QList<SegmentData*> conflicts;
     QList<SegmentInfo> dupSegments = sql->getDupSegments(*sd);
     foreach(SegmentInfo si, dupSegments) {
         sd->setSegmentId(si.segmentId());
         if(sql->doesRouteSegmentExist(*sd))
         {
            QMessageBox::critical(this, tr("Conflict"), tr("The segment is already present"
                               " or conflicts with the start or end date of an"
                               " existing segment. The segment will not be added!"));
            return;
         }
     }
     if(!sql->addSegmentToRoute(sd))
     {
      //updateRoute(sd);
      return;
     }
     m_bridge->processScript("clearPolyline", QString("%1").arg(sd->segmentId()));
        //SegmentInfo si = sql->getSegmentInfo(segmentId);
//     displaySegment(sd->segmentId(), sd->description(),
//                       getColor(sd->tractionType()),
//                       sd->trackUsage(), true);
     sd->displaySegment(sd->startDate().toString("yyyy/MM/dd"),getColor(sd->tractionType()),sd->trackUsage(),true);
     //selectSegment(sd->segmentId());
 });

 addSegmentViaUpdateRouteAct = new QAction(tr("Add segment via UpdateRoute"),this);
 addSegmentViaUpdateRouteAct->setStatusTip(tr("Add via UpdateRoute possibly creating new route."));
 connect(addSegmentViaUpdateRouteAct, &QAction::triggered, [=]{
  SegmentData* sd = new SegmentData(ui->ssw->segmentSelected());
  sd->setRoute(m_routeNbr);
  sd->setAlphaRoute((_rd.alphaRoute()));
  sd->setRouteName(_rd.routeName());
  sd->setCompanyKey(_rd.companyKey());
  sd->setTractionType(_rd.tractionType());
  CompanyData* cd = sql->getCompany(_rd.companyKey()>0?_rd.companyKey():ui->cbCompany->currentData().toInt());
  if(_rd.startDate().isValid())
  {
   sd->setStartDate(_rd.startDate());
   sd->setEndDate(_rd.endDate());
  }
  else
  {
   sd->setStartDate(cd->startDate);
   sd->setEndDate(cd->endDate);
  }
  if(sd->startDate() < cd->startDate)
   sd->setStartDate(cd->startDate);
  if(sd->endDate() > cd->endDate)
   sd->setEndDate(cd->endDate);
//  if(!sql->addSegmentToRoute(sd))
//  {
//   updateRoute(&sd);
//   return;
//  }
  connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs )), this, SLOT(routeChanged(RouteChangedEventArgs )));

  updateRoute(sd);
  ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(sd->companyKey()));
 });

 findDormantSegmentsAct = new QAction(tr("Find dormant segments"),this);
 findDormantSegmentsAct->setStatusTip(tr("Display a lists of segments that are dormant, i.e. not in service"));
// TODO: find dormant segments
 connect(findDormantSegmentsAct, SIGNAL(triggered()), this, SLOT(NotYetInplemented()));
 addRouteAct = new QAction(tr("Add new Route"),this);
 addRouteAct->setStatusTip(tr("Add a new route"));
 connect(addRouteAct, SIGNAL(triggered()), this, SLOT(addRoute()));

 splitSegmentAct = new QAction(tr("Split segment at a date"),this);
 splitSegmentAct->setStatusTip(tr("Split a segment at a date."));
 connect(splitSegmentAct, &QAction::triggered, [=]{
  SplitSegmentDlg* splitSegment = new SplitSegmentDlg(m_segmentId);
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
 updateParametersAct = new QAction(tr("Update Parameters"), this);
 updateParametersAct->setStatusTip(tr("Change city parameters"));
 connect(updateParametersAct, &QAction::triggered, [=]{
  DialogEditParameters dlg;
  if(dlg.exec() == QDialog::Accepted)
  {
    config->cityNames().removeOne(config->currCity->name());
    config->cityNames().append(dlg.parameters().city);
    config->cityBounds.remove(config->currCity->name());
    config->cityBounds.insert(dlg.parameters().city, config->currCity->bounds());
    config->currCity->setNameOverride(dlg.parameters().city);
  }
 });
 displayRoutesForSelectedCompaniesAct = new QAction(tr("Display routes for selected companies"),this);
 displayRoutesForSelectedCompaniesAct->setStatusTip(tr("If checked, routes will be displayed only for checked companies in the Companies tab."));
 displayRoutesForSelectedCompaniesAct->setCheckable(true);
 displayRoutesForSelectedCompaniesAct->setChecked(config->currCity->bDisplayRoutesForSelectedCompanies);
 connect(displayRoutesForSelectedCompaniesAct, &QAction::toggled, [=](bool checked){
     config->currCity->bDisplayRoutesForSelectedCompanies = checked;
     refreshCompanies();
 });


 testUrlAct = new QAction(tr("test url"));
 connect(testUrlAct, &QAction::triggered, [=]{
  if(webView)
  {
   DialogTextEdit* dlg = new DialogTextEdit("Enter Url", "Enter a url to test the webview.");
   if(dlg->exec() == QDialog::Accepted)
   {
    webView->load(QUrl(dlg->result()));
   }
  }
 });

 testScriptAct = new QAction(tr("test script"));
 connect(testScriptAct, &QAction::triggered, [=]{
//  if(webView)
//  {
   DialogTextEdit* dlg = new DialogTextEdit("Enter script", "Enter a script name to test the webview.");
   if(dlg->exec() == QDialog::Accepted)
   {
    m_bridge->processScript(dlg->result());
   }
//  }
 });

 testLoadAct = new QAction(tr("load file"),this);
 testLoadAct->setStatusTip(tr("load a file from the html dir"));
 connect(testLoadAct, &QAction::triggered, [=]{
//  if(webView)
//  {
   DialogTextEdit* dlg = new DialogTextEdit("Enter filename", "Enter a file name to load.");
   if(dlg->exec() == QDialog::Accepted)
   {
    QFileInfo info(QDir::currentPath() + QDir::separator() + "html"  + QDir::separator() + dlg->result());
    if(info.exists())
     fileUrl = QUrl::fromLocalFile(info.path());
    webView->load(fileUrl);
   }
//  }
 });

 testRunJavaScriptAct = new QAction(tr("run javascript"),this);
 testRunJavaScriptAct->setStatusTip("enter a javascript to run");
 connect(testRunJavaScriptAct, &QAction::triggered, [=]{
//  if(webView)
//  {
   DialogTextEdit* dlg = new DialogTextEdit("Enter script", "Enter the javascript snippet to test the webview.");
   if(dlg->exec() == QDialog::Accepted)
   {
    webView->page()->runJavaScript(dlg->result());
   }
//  }
 });

 populateRouteIdAct = new QAction(tr("populate Routeid"),this);
 connect(populateRouteIdAct, &QAction::triggered,this,[=]{
     sql->populateRouteId();
 });


 selAllCompaniesAct = new QAction(tr("All Companies"), this);
 selAllCompaniesAct->setStatusTip(tr("Show routes for all companies"));

 combineRoutesAct = new QAction(tr("Combine two routes"), this);
 combineRoutesAct->setStatusTip(tr("Combine two routes into one"));
 connect(combineRoutesAct, SIGNAL(triggered()), this, SLOT(combineRoutes()));

 refreshRoutesAct = new QAction(tr("Refresh Routes"),this);
 refreshRoutesAct->setStatusTip(tr("Refresh the routes combobox. Especially after executing manual queries to the database."));
 connect(refreshRoutesAct, SIGNAL(triggered()), this, SLOT(refreshRoutes()));

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
 showDebugMessages->setChecked(config->bDisplayDebugMsgs);
 showDebugMessages->setStatusTip(tr("If checked, javascript debug messages will be displayed in the infobar. "));
 connect(showDebugMessages, SIGNAL(toggled(bool)), this, SLOT(on_showDebugMessages(bool)));

 runInBrowserAct = new QAction(tr("Display map in browser"), this);
 runInBrowserAct->setCheckable(true);
 runInBrowserAct->setStatusTip(tr("If checked, map display will be in web browser. You must then restart Mapper"));
 runInBrowserAct->setChecked(config->bRunInBrowser);
 //connect(runInBrowserAct, SIGNAL(toggled(bool)), this, SLOT(on_runInBrowser(bool)));

 addGeoreferencedOverlayAct = new QAction(tr("Edit Overlay list"), this);
 addGeoreferencedOverlayAct->setStatusTip(tr("Open a dialog to edit list of available overlays."));
 connect(addGeoreferencedOverlayAct, SIGNAL(triggered(bool)), this, SLOT(on_addGeoreferenced(bool)));

 creditsAct = new QAction(tr("Credits"),this);
 creditsAct->setStatusTip(tr("Credits and information sources"));
 connect(creditsAct, &QAction::triggered,[=]{
     QDesktopServices::openUrl(QUrl::fromLocalFile(wikiRoot+"/Credits.html"));

 });
 browseCommentsAct = new QAction(tr("Browse Comments"), this);
 connect(browseCommentsAct, &QAction::triggered, [=]{
  BrowseCommentsDialog* dlg = new BrowseCommentsDialog();
  dlg->exec();
 });

 exportOverlaysAct = new QAction("Export overlays", this);
 connect(exportOverlaysAct, &QAction::triggered, [=]{
  Overlay::exportXml("./Resources/overlays.xml", config->overlayMap->values());
 });

 overlayHelp = new QAction(tr("Overlays"),this);
 overlayHelp->setStatusTip(tr("Help on setting up a new overlay."));
 connect(overlayHelp, SIGNAL(triggered(bool)), this, SLOT(on_overlayHelp()));

 usingMapper = new QAction(tr("Using Mapper"), this);
 usingMapper->setStatusTip(tr("User documentation"));
 connect(usingMapper, SIGNAL(triggered(bool)), this, SLOT(on_usingHelp()));

 setCityBoundsAct = new QAction(tr("Set City Bounds"),this);
 setCityBoundsAct->setStatusTip(tr("Set the display bounds for thiis city/region"));
 connect(setCityBoundsAct, &QAction::triggered, [=]{
     config->currCity->setCityBounds(m_bridge);
  });

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
  CompanyData* cd = sql->getCompany(rd.companyKey());
  DialogChangeRoute* dlg = new DialogChangeRoute();
  int rslt = dlg->exec();
  if(rslt == QDialog::DialogCode::Accepted )
  {
   int newRoute = dlg->getNumber();
   bool rslt = sql->renumberRoute(rd.alphaRoute(), newRoute, cd->routePrefix);
   if(rslt)
   {
    refreshRoutes();
   }
  }
 });

 checkSegmentsAct = new QAction(tr("Check segments"),this);
 checkSegmentsAct->setStatusTip(tr("update direction, bounds, etc; Update routes using segments"));
 connect(checkSegmentsAct, &QAction::triggered, [=]{
     setCursor(Qt::WaitCursor);
     try {
        sql->checkSegments();
     }
     catch (Exception e){
     }
     setCursor(Qt::ArrowCursor);
 });

 showGoogleMapFeaturesAct = new QAction(tr("Show Google Map Features"), this);
 showGoogleMapFeaturesAct->setStatusTip(tr("Show or hide Google Maps features of interest."));
 showGoogleMapFeaturesAct->setCheckable(true);
 showGoogleMapFeaturesAct->setChecked(true);
 connect(showGoogleMapFeaturesAct, SIGNAL(toggled(bool)), this, SLOT(showGoogleMapFeatures(bool)));

 foreignKeyCheckAct = new QAction(tr("Foreign key check"),this);
 foreignKeyCheckAct->setStatusTip(tr("Enable foreign key checking <I>Sqlite only!</I"));
 foreignKeyCheckAct->setCheckable(true);
 foreignKeyCheckAct->setChecked(config->foreignKeyCheck());
 connect(foreignKeyCheckAct, &QAction::toggled, [=](bool b){
  SQL::instance()->setForeignKeyCheck(b);
 });

  fontSizeChangeAct = new QAction(tr("Change font"), this);
  fontSizeChangeAct->setStatusTip(tr("Open a dialog to change the application's font."));
  connect(fontSizeChangeAct, &QAction::triggered, [=]{
     bool ok;
     QFont f = QFontDialog::getFont(&ok, config->font,this,tr("Select new font"));
     config->font = f;
     this->changeFonts(f);
     qDebug() << "new font " << this->font().toString();
  });
  displaySegmentArrows = new QAction(tr("Display segment arrows"), this);
  displaySegmentArrows->setCheckable(true);
  displaySegmentArrows->setStatusTip(tr("Display arrows on double track segments"));
  connect(displaySegmentArrows, &QAction::toggled, [=](bool b){
   config->bDisplaySegmentArrows = b;
  });

  editStreetsAct = new QAction("Edit streets tool",this);
  connect(editStreetsAct, &QAction::triggered, [=]{
      DialogEditStreets dlg = DialogEditStreets(this) ;
      dlg.exec();
  });

  updateStreetsAct = new QAction(tr("Update streets dialog"),this);
  connect(updateStreetsAct, &QAction::triggered, [=]{
      if(!dialogUpdateStreets)
        dialogUpdateStreets = new DialogUpdateStreets(this) ;
      dialogUpdateStreets->raise();
      dialogUpdateStreets->show();
      //dlg.exec();
  });

  displayRouteOnReloadAct = new QAction(tr("Display route on reload"),this);
  displayRouteOnReloadAct->setStatusTip(tr("Display current route when reloading map"));
  displayRouteOnReloadAct->setCheckable(true);
  displayRouteOnReloadAct->setChecked(config->bDisplayRouteOnReload);
  connect(displayRouteOnReloadAct, &QAction::toggled,this,[=](bool b){
      config->bDisplayRouteOnReload = b;
  });
}

QWidgetAction *MainWindow::createWidgetAction()
{
 cbSort = new QComboBox(this);

 initRouteSortCb(cbSort);

 sortTypeAct = new QWidgetAction(this);
 sortTypeAct->setDefaultWidget(cbSort);

 return sortTypeAct;
}

void MainWindow::initRouteSortCb(QComboBox* cbSort)
{
    cbSort->setVisible(true);
    cbSort->activateWindow();
    cbSort->addItem("Route, end date");
    cbSort->addItem("Route, start date");
    cbSort->addItem("Route, name, end date");
    cbSort->addItem("Route, name, start date");
    cbSort->addItem("Name, Route, start date");
    cbSort->addItem("start date, name, route");
    cbSort->addItem("Base Route, start date");
    cbSort->addItem("CompanyKey,Route, start date");

    cbSort->setCurrentIndex(config->currCity->routeSortType);

}

void MainWindow::addSegmentToRoute(SegmentData* sd)
{
    SegmentInfo si = sql->getSegmentInfo(sd->segmentId());
    QList<SegmentInfo> dupSegments = sql->getDupSegments(si);
    foreach(SegmentInfo si, dupSegments) {
        sd->setSegmentId(si.segmentId());
        if(sql->doesRouteSegmentExist(*sd))
        {
            QMessageBox::critical(this, tr("Conflict"), tr("The segment is already present"
                        " or conflicts with the start or end date of an"
                        " existing segment %1. The segment will not be added!").arg(si.toString()));
            return;
        }
    }

 if(sd->startDate() < sd->segmentStartDate())
     sd->setSegmentStartDate(sd->startDate());
 if(sd->endDate() > sd->segmentEndDate())
     sd->setSegmentEndDate(sd->endDate());
 if(!sd->doubleDate().isValid() && sd->tracks()==2)
  sd->setDoubleDate(sd->segmentStartDate());
 if(sd->streetId() == -1)
 {
     if(!sd->streetName().isEmpty())
     {
         QList<StreetInfo*> list = StreetsTableModel::instance()->getStreetName(sd->streetName());
         if(list.length() == 1)
         {
             StreetInfo* sti = list.at(0);
             sd->setStreetId(sti->streetId);
             sd->setNewerName(sti->newerName);
             if(!sti->startLatLng.isValid())
                 sti->startLatLng = sd->startLatLng();
             if(!sti->endLatLng.isValid())
                 sti->endLatLng = sd->endLatLng();
             if(!sti->segments.contains(sd->segmentId()))
             {
                 sti->segments.append(sd->segmentId());
                 sti->updateBounds();
                 StreetsTableModel::instance()->updateStreetName(*sti);
             }
             sti->updateSegmentInfo(SegmentInfo(*sd));
         }
     }
 }
 if(!sql->addSegmentToRoute(sd, true))
 {
  //updateRoute(sd);
  return;
 }

 m_segmentId = sd->segmentId();
 Q_ASSERT(m_segmentId > 0);
 m_nbrPoints = sd->pointList().size();
 m_points = sd->pointList();
 qDebug() << tr("adding segment %1 %2 to route %3 %4 connected at %5 end")
                 .arg(sd->segmentId()).arg(sd->description()).arg(sd->route()).arg(sd->routeName()).arg(sd->whichEnd());

    m_bridge->processScript("clearPolyline", QString("%1").arg(sd->segmentId()));
    //SegmentInfo si = sql->getSegmentInfo(segmentId);
    sd->displaySegment( getColor(sd->tractionType()), true);
    if(sd->whichEnd()== "E")
        btnFirstClicked();
    if(sd->whichEnd()== "S")
        btnLastClicked();
}

void MainWindow::changeFonts(QFont f)
{
    QObject *obj = this;
    QList<QWidget*> objlistChildren = obj->findChildren<QWidget*>();
    int iCount = objlistChildren.count();

    for(int iIndex = 0; iIndex < iCount; iIndex++)
    {
        QWidget *temp = objlistChildren[iIndex];
        temp->setFont(f);
    }
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

     m_bridge->processScript("getCurrBounds");
     settings.setValue("geometry", saveGeometry());
     settings.setValue("windowState", saveState());
     settings.setValue("splitter", ui->splitter->saveState());
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

    QAction* preferencesAct = new QAction(tr("Preferences"),this);
    fileMenu->addAction(preferencesAct);
    connect(preferencesAct, &QAction::triggered,this,[=]{
        DialogPreferences dlg(this);
        dlg.exec();
    });

    QAction* backupDbAct = new QAction(tr("Backup databases"),this);
    backupDbAct->setStatusTip(tr("Backup SQlite dabtabases to text files."));
    connect(backupDbAct, &QAction::triggered,this,[=]{
        backupDatabases();
    });
    fileMenu->addAction(backupDbAct);
    QAction* restoreDbAct = new QAction(tr("Restore databases"),this);
    backupDbAct->setStatusTip(tr("Restore SQlite dabtabases from text files."));
    connect(restoreDbAct, &QAction::triggered,this,[=]{
        restoreDatabases();
    });
    fileMenu->addAction(restoreDbAct);

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
    connectionsMenu->addAction(updateParametersAct);

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
    connect(toolsMenu, &QMenu::aboutToShow,this, [=]{
     addPointModeAct->setChecked(m_bAddMode);
    });
    //toolsMenu->addAction(companyChangeRoutes);
    //toolsMenu->addAction(editStreetsAct);
    toolsMenu->addAction((updateStreetsAct));
#ifdef MYPREFIX_DEBUG
    toolsMenu->addSeparator();
    toolsMenu->addSection("Debug");
    toolsMenu->addAction(testUrlAct);
    testUrlAct->setEnabled(!config->bRunInBrowser);
    toolsMenu->addAction(testScriptAct);
    toolsMenu->addAction(testLoadAct);
    toolsMenu->addAction(testRunJavaScriptAct);
    toolsMenu->addAction(populateRouteIdAct);
#endif
    optionsMenu = new Menu(tr("Options"));
    overlayMenu = new Menu(tr("Overlays"));

    // connect(optionsMenu, &Menu::aboutToShow, [=]{
    //  if(config->currConnection->servertype()== "Sqlite")
    //  {
      optionsMenu->clear();
      fillOverlayMenu();
      optionsMenu->addMenu(overlayMenu);
      connect(overlayMenu, SIGNAL(aboutToShow()), this, SLOT(fillOverlayMenu()));
      optionsMenu->addAction(displayRouteCommentsAct);
      optionsMenu->addAction(displayStationMarkersAct);
      optionsMenu->addAction(displayTerminalMarkersAct);
      optionsMenu->addAction(showDebugMessages);
#ifdef MYPREFIX_DEBUG
      optionsMenu->addAction(geocoderRequestAct);
#endif
      disconnect(runInBrowserAct, SIGNAL(toggled(bool)), this, SLOT(on_runInBrowser(bool)));
      optionsMenu->addAction(runInBrowserAct);
      runInBrowserAct->setChecked(config->bRunInBrowser);
      connect(runInBrowserAct, SIGNAL(toggled(bool)), this, SLOT(on_runInBrowser(bool)));

      sortMenu = new Menu(tr("Route Sort option"));
      sortMenu->addAction(createWidgetAction());
      optionsMenu->addMenu(sortMenu);
      optionsMenu->addAction(showGoogleMapFeaturesAct);
      optionsMenu->addAction(foreignKeyCheckAct);
      foreignKeyCheckAct->setChecked(SQL::instance()->getForeignKeyCheck());
      optionsMenu->addAction(fontSizeChangeAct);
      optionsMenu->addAction(displaySegmentArrows);
      displaySegmentArrows->setChecked(config->bDisplaySegmentArrows);
      optionsMenu->addAction(displayRoutesForSelectedCompaniesAct);
      optionsMenu->addAction(displayRouteOnReloadAct);
      displayRouteOnReloadAct->setCheckable(true);
      displayRouteOnReloadAct->setChecked(config->bDisplayRouteOnReload);
    menuBar()->addMenu(optionsMenu);
    menuBar()->addMenu(toolsMenu);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setToolTipsVisible(true);
//    helpMenu->addAction(webViewAction = new WebViewAction((QObject*)this));
#ifdef HAVE_CONSOLE
    //helpMenu->addAction(systemConsoleAction = new SystemConsoleAction());
    QAction* consoleAct = new QAction(tr("System Console"),this);
    helpMenu->addAction(consoleAct);

    //systemConsoleAction->setToolTip(tr("Display, info, error and debug messages"));
    consoleAct ->setToolTip(tr("Display, info, error and debug messages"));
    connect(consoleAct, &QAction::triggered, [=]{
        if(!consoleDlg )
            consoleDlg = new SystemConsole2(this);
        consoleDlg->raise();
        consoleDlg->show();
    });
#endif
    helpMenu->addAction(usingMapper);
    helpMenu->addAction(creditsAct);
    helpMenu->addAction(overlayHelp);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);
}

void MainWindow::createCityMenu()
{
 if(cityMenu == nullptr)
  return;
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
    //cbRouteMenu->clear();
    cbRouteMenu = ui->cbRoute->lineEdit()->createStandardContextMenu();
    cbRouteMenu->addSeparator();
    QMenu* extendMenu = new QMenu(tr("More ..."));
    cbRouteMenu->addAction(addSegmentAct);
    extendMenu->addAction(combineRoutesAct);
    cbRouteMenu->addAction(copyRouteAct);
    cbRouteMenu->addAction(deleteRouteAct);
    cbRouteMenu->addAction(displayAct);
    cbRouteMenu->addAction(modifyRouteDateAct);
    extendMenu->addAction(modifyRouteTractionTypeAct);
    cbRouteMenu->addAction(renameRouteAct);
    extendMenu->addAction(rerouteAct);
    cbRouteMenu->addAction(routeCommentsAct);
    cbRouteMenu->addAction(splitRouteAct);
    cbRouteMenu->addMenu(extendMenu);
    extendMenu->addAction(replaceSegments);
    extendMenu->addAction(exportRouteAct);
    extendMenu->addAction(updateTerminalsAct);
    cbRouteMenu->addAction(describeRouteAct);
    cbRouteMenu->addAction(displayAllRoutesAct);
    cbRouteMenu->addAction(editRouteSqlAct);
    updateTerminalsAct->setEnabled(routeView->isSequenced());
    cbRouteMenu->exec(QCursor::pos());
}

void MainWindow::cbCompany_customContextMenu( const QPoint& )
{
    cbCompanyMenu = new QMenu();
    cbCompanyMenu->addAction(selAllCompaniesAct);
    cbCompanyMenu->addAction(clearAllCompaniesAct);
    cbCompanyMenu->exec(QCursor::pos());
}

QList<QAction *> MainWindow::txtSegment_customContextMenu()
{
    // Define sub menues and actions to be added to the context menu.
    QList<QAction*> list;
     //menu->addSection(tr("More"));
     QAction* edit = new QAction(tr("Edit segment"), this);
     edit->setStatusTip(tr("Edit segment details such as tracks, route type, etc."));
     list.append(edit);
     connect(edit, SIGNAL(triggered(bool)), this, SLOT(On_editSegment_triggered()));
     list.append(splitSegmentAct);
   return list;
}
#if 0
void MainWindow::tab1CustomContextMenu(const QPoint &)
{
    tab1Menu.addAction(saveChangesAct);
    tab1Menu.addAction(discardChangesAct);
    tab1Menu.addAction(sortNameAct);
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
#endif
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
    //companyView = new CompanyView(this);
    tractionTypeView = new TractionTypeView(this);
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

    qInfo() << city->name() + "/" + connection->description();
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
#ifndef NO_UDF
    sql->loadSqlite3Functions();
#endif
    if(ok)
    {
     sql->checkTables(db);
    }
#endif
    db = config->currConnection->configure();
    SQL::instance()->checkTables(db);
    emit newCitySelected();

    this->setWindowTitle("Mapper - "+ config->currCity->name() + " ("+config->currConnection->description()+")");
    config->saveSettings();

    tractionTypeList = sql->getTractionTypes();
    tractionTypeView = new TractionTypeView(this);

    //refreshSegmentCB();
    ui->ssw->refresh();
    refreshCompanies();
    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(config->currCity->companyKey));
    refreshRoutes();
    companyView->refresh();
    //stationView->showStations();
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

    // objArray.clear();
    // objArray << 0 << m_latitude<<m_longitude<<7<<"pointO"<<m_segmentId;
    //  m_bridge->processScript("addMarker",objArray);

    for(int i=0; i< routeList.count(); i++)
    {
     RouteData rd = routeList.at(i);
     if(rd.route() ==config->currCity->lastRoute && rd.routeName() == config->currCity->lastRouteName
        && rd.endDate().toString("yyyy/MM/dd") == config->currCity->lastRouteEndDate)
     {
      ui->cbRoute->setCurrentIndex(i);
      break;
     }
    }
    // companyView = new CompanyView(config, this);
    // tractionTypeView = new TractionTypeView(config, this);
    this->setCursor(QCursor(Qt::ArrowCursor));
    enableControls(true);
    routeView->clear();
    streetView->model()->setList(streetView->model()->getStreetInfoList());
    ui->ssw->initialize();
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

 loadOverlay(cOv);
 for(int i = 0; i < config->currCity->city_overlayMap->count(); i++)
 {
  Overlay* ov = config->currCity->city_overlayMap->values().at(i);
  if(currentOverlay == ov->name)
  {
   config->currCity->curOverlayId = i;
   break;
  }
 }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    config->saveSettings();
}

void MainWindow::about()
{
 QMessageBox::about(this, tr("About Mapper"),
     tr("The <b>Mapper</b> version %2. Written by Allen C. Kempe. "
        "maintain database of streetcar and transit routes.\r\n"
        "Compiled with QT version %1").arg(QT_VERSION_STR).arg(MY_VERSION));
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
        QList<SegmentInfo> siDups = sql->getSegmentsInSameDirection(sd);
        foreach(SegmentInfo siDup, siDups)
        {
           // Get a list of all routes using this segment.
           QList<SegmentData*> segmentDataList = sql->getRouteSegmentsBySegment(sd.segmentId());
           if ( segmentDataList.count() > 0)
           {
               if ( siDup.segmentId() >= 0)
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
                           SegmentData *sd = new SegmentData(*segmentDataList.at(i));
                           if (sql->deleteRouteSegment(sd->route(), sd->routeId(), sd->segmentId(),
                                                       sd->startDate().toString("yyyy/MM/dd"),
                                                       sd->endDate().toString("yyyy/MM/dd")) != true)
                               {
                                   //infoPanel.Text = "Delete Error";
                                   statusBar()->showMessage(tr("Delete failed"));
                                   //infoPanel.ForeColor = Color.Red;
                                   //System.Media.SystemSounds.Beep.Play();
                                   QApplication::beep();
                                   return;
                               }
                           if (sql->doesRouteSegmentExist(sd->route(), sd->routeName(), siDup.segmentId(),
                                                          sd->startDate(), sd->endDate()))
                                   continue;
   //                            if (!sql->addSegmentToRoute(rd.route(), rd.routeName(), rd.startDate(), rd.endDate(), sdDup.segmentId(),
   //                                                        rd.companyKey(),
   //                                                        rd.tractionType(), rd.direction(), rd.next(), rd.prev(), rd.normalEnter(),
   //                                                        rd.normalLeave(), rd.reverseEnter(), rd.reverseLeave(), rd.oneWay(), rd.trackUsage()))
                           SegmentData *sd1 = new SegmentData(*sd);
                           sd1->setSegmentId(siDup.segmentId());
                           if (!sql->addSegmentToRoute(sd))
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
    if(companyView->model()->isDirty())
    {
        refreshCompanies();
        companyView->model()->setDirty(false);
    }
    int companyKey = ui->cbCompany->currentData().toInt();
    int ix = ui->cbRoute->currentIndex();
    QString currText = ui->cbRoute->currentText();
    ui->cbRoute->clear();

    //routeList = sql->getRoutesByEndDate(companyKey);
    if(config->currCity->bDisplayRoutesForSelectedCompanies)
    {
        if(config->currCity->selectedCompaniesList.isEmpty() )
        {
            QMessageBox::information(nullptr, tr("No companies selected."),
                tr("No companies are checked in the Companies view tab.\n"
                    "Check one or more companies and turn on the <I>'Display routes for selected Companies'</I> option"));
            bCbRouteRefreshing= false;
            config->currCity->bDisplayRoutesForSelectedCompanies =false;
            return;
        }
        if(config->currCity->companyKey > 0 )
            config->currCity->selectedCompaniesList.append(config->currCity->companyKey);
        else
            config->currCity->selectedCompaniesList.append(1);

        routeList = sql->getRoutesByEndDate(config->currCity->selectedCompaniesList);
    }
    else
    {
     routeList = sql->getRoutesByEndDate(ui->cbCompany->currentData().toInt());
    }
    if(routeList.isEmpty())
     qDebug() << "no routes selected for company " << companyKey;

    QMap<QString, RouteData> map;
    QString rSort;

    for(RouteData rd : routeList)
    {
        QString companyKeyStr;
        if(rd.companyKey() < 10)
            companyKeyStr ="0";
        companyKeyStr.append(QString::number(rd.companyKey()));

        rSort = createSortString(rd.alphaRoute());
     switch(config->currCity->routeSortType)
     {
      case 0:
      map.insert(rSort+rd.endDate().toString("yyyy/MM/dd"),rd);
      break;
     case 1:
      map.insert(rSort+rd.startDate().toString("yyyy/MM/dd"),rd);
      break;
     case 2:
      map.insert(rSort+rd.routeName()+rd.endDate().toString("yyyy/MM/dd"),rd);
      break;
     case 3:
      map.insert(rSort+rd.routeName()+rd.startDate().toString("yyyy/MM/dd"),rd);
      break;
     case 4:
      map.insert(rd.routeName()+rSort+rd.endDate().toString("yyyy/MM/dd"),rd);
      break;
     case 5:
      map.insert(rd.startDate().toString("yyyy/MM/dd") + rd.routeName()+rSort,rd);
      break;
     case 6: // base route, start date
      map.insert(QStringLiteral("%1").arg(rd.baseRoute(),3,10,QLatin1Char('0'))+rd.startDate().toString("yyyy/MM/dd"),rd);
      break;
     case 7: // companykey, route, startdate
      map.insert(companyKeyStr+rSort+rd.startDate().toString("yyyy/MM/dd"),rd);
     break;
     }
    }
    //int len = routeList.count();
    routeList = map.values();

    for(int i=0; i<routeList.count(); i++)
    {
         RouteData rd = routeList.at(i);
        CompanyData* rcd = sql->getCompany(rd.companyKey());

         QString rdStartDate = rd.startDate().toString("yyyy/MM/dd");
         ui->cbRoute->addItem(rd.toString(),QVariant::fromValue(rd));
         if(!(rd.startDate()>=rcd->startDate && rd.endDate()<= rcd->endDate))
         {
             const QModelIndex idx = ui->cbRoute->model()->index(i, 0);
             ui->cbRoute->model()->setData(idx, QColor(255,0,0), Qt::BackgroundRole);
         }
         if( rd.toString() == currText)
            ui->cbRoute->setCurrentIndex(i);
    }
    ui->sbRoute->setRange(0, routeList.count());

    if( ui->cbRoute->currentIndex() <= 0 && ix >=0)
        ui->cbRoute->setCurrentIndex(ix);
    bCbRouteRefreshing = false;
}

void MainWindow::selectRoute(RouteData rd)
{
 //int ix = ui->cbRoute->findData(QVariant::fromValue(rd));
 int ix;
 for(ix=0; ix < ui->cbRoute->count(); ix++)
 {
  RouteData rd1 = ui->cbRoute->itemData(ix).value<RouteData>();
  if(rd1 == rd)
   break;
 }
 if(ix >= 0)
  ui->cbRoute->setCurrentIndex(ix);
}
#if 0
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
    CompanyData* cd = sql->getCompany(companyKey);
//    }
    qint32 newRoute = sql->getNumericRoute(txt, &txtAlpha, &bIsAlpha, cd->routePrefix);
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
         QString rdStartDate = rd.startDate().toString("yyyy/MM/dd");
         ui->cbRoute->addItem(rd.toString(), QVariant::fromValue(rd));
    }
    ui->sbRoute->setRange(0, routeList.count());

    bCbRouteRefreshing = false;
}
#endif
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
 Kml* kml = new Kml(rd.routeName(),  segmentDataList);
 QString fileName = QFileDialog::getOpenFileName(this,"Create Kml file", QDir::homePath(),"Kml files (*.kml");
 if(!fileName.isEmpty())
 kml->createKml(fileName, "ff0000ff");
}

void MainWindow::btnDisplayRouteClicked()
{
 addModeToggled(false);
 int row =         ui->cbRoute->currentIndex();
 if(row < 0) return;
 RouteData rd = routeList.at(row);
 //rd.setEndDate(ui->dateEdit->date());
 On_displayRoute(rd);
}

void MainWindow::On_displayRoute(RouteData rd)
{
 m_bridge->processScript("addModeOff");

 if(!ui->chkNoClear->isChecked())
  btnClearClicked();

 QList<SegmentData*> segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route(),
                                            rd.routeName(), rd.companyKey(), rd.endDate());
 Bounds bounds = Bounds();
 bool bBoundsValid = false;
 double infoLat=0, infoLon = 0;
 bool bFirst = true;

 QVariantList objArray;
 if (rd.route() < 1)
  return; // no data
    //string str = (m_routeNbr<10?"0":"")+ m_routeNbr;
 CompanyData* cd = sql->getCompany(rd.companyKey());

 m_alphaRoute = sql->getAlphaRoute(m_routeNbr, cd->routePrefix);
 if(bDisplayTerminalMarkers)
 {
  TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, rd.endDate());
  if (ti.route >= 1 && ti.startLatLng.lat() > 0 && ti.startLatLng.lon() )
  {
   objArray <<ti.startLatLng.lat() <<ti.startLatLng.lon() << //getRouteMarkerImagePath(m_alphaRoute, true);
          "./green00.png" << "'" + m_alphaRoute + "'";
   m_bridge->processScript("addRouteStartMarker", objArray);
   infoLat = ti.startLatLng.lat();
   infoLon = ti.startLatLng.lon();
   bFirst = false;

   objArray.clear();
   objArray << ti.endLatLng.lat() << ti.endLatLng.lon()<<//getRouteMarkerImagePath(m_alphaRoute, false);
       "./red00.png" << "'" + m_alphaRoute + "'";
   m_bridge->processScript("addRouteEndMarker", objArray);
  }
 }
 double length = 0.0;
 //foreach (segmentGroup sg in ri.segments)
 for(int i = 0; i< segmentDataList.count(); i++)
 {
  SegmentData* sd = segmentDataList.at(i);
  if(sd->segmentId() == 367)
   qDebug() << "halt";
#if 0
  objArray.clear();
  objArray << sd->segmentId();
  m_bridge->processScript("clearPolyline", objArray);
  QString color = getColor(sd->tractionType());

  QVariantList points;
  for(int i=0; i < sd->pointList().count(); i++)
  {
   LatLng pt = sd->pointList().at(i);
   points.append(pt.lat());
   points.append(pt.lon());
  }
  bBoundsValid = bounds.updateBounds(sd->bounds());
  int tracks = sd->tracks();
  if(tracks == 2 && sd->doubleDate().isValid() && rd.endDate() < sd->doubleDate())
  {
   tracks = 1;
   sd->setTrackUsage(" ");
  }

  int dash = 0;
  if(sd->routeType() == Incline)
   dash = 1;
  else if(sd->routeType() == SurfacePRW)
   dash = 2;
  else if(sd->routeType() == Subway)
   dash = 3;
  if(sd->trackUsage().isEmpty()) // fix for MySql not storing field correctly
   sd->setTrackUsage(" ");
  objArray.clear();
  objArray <<   sd->segmentId() << rd.routeName() <<  sd->description()
             << sd->oneWay() << config->bDisplaySegmentArrows
             << color << tracks
             << dash << sd->routeType() << sd->trackUsage() << points.count();
  objArray.append(points);
  m_bridge->processScript("createSegment",objArray);

  //statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(sd.length()).arg(sd.length()*0.621371192));
  if(sd->tracks() == 2)
  {
   if(sd->oneWay() == "Y")
    length += sd->length();
   else
    length += sd->length()*2;
  }
  else
   length += sd->length();
#else
  sd->displaySegment(ui->dateEdit->date().toString("yyyy/MM/dd"),getColor(sd->tractionType()), sd->trackUsage(),true);
  bBoundsValid = bounds.updateBounds(sd->bounds());
#endif
 }
 statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(length).arg(length*0.621371192));

 QString markerType = "green";
 QMap<int, TractionTypeInfo> tractionTypes= sql->getTractionTypes();
 foreach(TractionTypeInfo tti,tractionTypeList.values())
 {
  //tractionTypeInfo tti = (tractionTypeInfo)tractionTypeList.at(i);
  if (tti.tractionType == rd.tractionType())
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
 QList<StationInfo> stationList;
 if(bDisplayStationMarkers)
 {
  //QList<StationInfo> stationList = sql->getStations(m_alphaRoute, QDate::fromString(m_currRouteEndDate, "yyyy/MM/dd"));
  // QMap<int,StationInfo> stationMap;
  // for(SegmentData* sd : segmentDataList)
  // {
  //  QList<StationInfo> sList = sql->getStationsOnSegment(sd->segmentId());
  //  for(StationInfo sti : sList)
  //  {
  //   if(sti.routeType != sd->routeType())
  //   {
  //       sti.routeType = sd->routeType();
  //       sql->updateStation(sti);
  //   }
  //   if(sti.routes.at(0).isEmpty()||sti.routes.at(0) == " ")
  //   {
  //       sti.routes.removeAt(0);
  //       sql->updateStation(sti);
  //   }
  //   if(!sti.routes.contains(m_alphaRoute))
  //   {
  //       sti.routes.append(m_alphaRoute);
  //       sql->updateStation(sti);
  //   }
  //   if(sti.endDate < _rd.startDate() || sti.startDate> _rd.endDate())
  //    continue;
  //   stationMap.insert(sti.stationKey, sti);
  //  }
      stationList = getStations(segmentDataList);
  //}
  if (!stationList.isEmpty())
  {
   //foreach (stationInfo sti in stationList)
   for(int i=stationList.count()-1; i >= 0; i --)
   {
    StationInfo sti= stationList.at(i);
    // if(isStationOnSegment(&sti,segmentDataList))
    //  stationList.removeAt(i);
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

    stationView->showStations(stationList);
   }
  } // if(bDisplayStationMarkers)
  if(!ui->chkNoPan->checkState() && bBoundsValid)
  {
   objArray.clear();
   objArray << bounds.swPt().lat() << bounds.swPt().lon() << bounds.nePt().lat() << bounds.nePt().lon();
   m_bridge->processScript("fitMapBounds", objArray);
  }

  loadRouteComment();

  setCursor(Qt::ArrowCursor);
  //bFirstSegmentDisplayed=true;
}

void MainWindow::displayAll()
{
 QList<SegmentData*> segmentDataList = sql->getRouteSegmentsForDate(_rd.startDate(), ui->cbCompany->currentData().toInt());

 double length = 0.0;
 //foreach (segmentGroup sg in ri.segments)
 for(int i = 0; i< segmentDataList.count(); i++)
 {
  SegmentData* sd = segmentDataList.at(i);
  if(sd->segmentId() == 367)
   qDebug() << "halt";
  QVariantList objArray;
  objArray << sd->segmentId();
  m_bridge->processScript("clearPolyline", objArray);
  QString color = getColor(sd->tractionType());
  int tracks = sd->tracks();
  if(tracks == 2 && sd->doubleDate().isValid() && _rd.startDate() < sd->doubleDate())
   tracks = 1;

  QVariantList points;
  for(int i=0; i < sd->pointList().count(); i++)
  {
   LatLng pt = sd->pointList().at(i);
   points.append(pt.lat());
   points.append(pt.lon());
  }

  Bounds bounds = Bounds();
  bool bBoundsValid = bounds.updateBounds(sd->bounds());

  int dash = 0;
  if(sd->routeType() == Incline)
   dash = 1;
  else if(sd->routeType() == SurfacePRW)
   dash = 2;
  else if(sd->routeType() == Subway)
   dash = 3;
  if(sd->trackUsage().isEmpty()) // fix for MySql not storing field correctly
   sd->setTrackUsage(" ");
  objArray.clear();
  objArray <<   sd->segmentId() << sd->routeName() <<  sd->description()
             << sd->oneWay() << config->bDisplaySegmentArrows
             << color << tracks
             << dash << sd->routeType() << sd->trackUsage() << points.count();
  objArray.append(points);
  m_bridge->processScript("createSegment",objArray);

  //statusBar()->showMessage(tr("route length = %1 km, %2 miles").arg(sd.length()).arg(sd.length()*0.621371192));
  if(sd->tracks() == 2)
  {
   if(sd->oneWay() == "Y")
    length += sd->length();
   else
    length += sd->length()*2;
  }
  else
   length += sd->length();
 }
}

// this function is used to eliminate stations from the display that are
// not on s segment of a route. This can happen because route designations have
// changed from time to time.
bool MainWindow::isStationOnSegment(StationInfo* sti, QList<SegmentData*> segmentDataList)
{
 if(sti->segmentId <= 0)
 {
  for(SegmentData* sd1 : segmentDataList)
  {
   for(LatLng latlng : sd1->pointList())
   {
    if(sti->latitude == latlng.lat() && sti->longitude == latlng.lon())
    {
     sti->segmentId =sd1->segmentId();
     if(!SQL::instance()->updateStation(*sti))
     {
      qDebug() << "error updating station ";
     }
     return true;
    }
   }
  }
 }
 else
 {
  for(SegmentData* sd1 : segmentDataList)
  {
   if(sd1->segmentId() == sti->segmentId)
    return true;
  }
 }
 return false;
}

void MainWindow::loadRouteComment()
{
 double infoLat=0, infoLon = 0;
 QVariantList objArray;

 QDate dt = QDate::fromString(m_currRouteStartDate, "yyyy/MM/dd");
 dt = sql->getFirstCommentDate(m_routeNbr, dt, _rd.companyKey());
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
 //routeView->checkChanges();
// if(routeView->bUncomittedChanges())
// {
//  return;
// }
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
 CompanyData* cd = sql->getCompany(_rd.companyKey());
 disconnect(ui->cbRoute, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbRouteIndexChanged(int)));
 //ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(cd->companyKey));
 connect(ui->cbRoute, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbRouteIndexChanged(int)));

 //ui->txtRouteNbr->setText(sql->getAlphaRoute(_rd.route(),cd->routePrefix));
 ui->dateEdit->setDate( _rd.endDate());
 m_currRouteStartDate = _rd.startDate().toString("yyyy/MM/dd");
 m_currRouteEndDate = _rd.endDate().toString("yyyy/MM/dd");
 ui->dateEdit->setMinimumDate(_rd.startDate());
 ui->dateEdit->setMaximumDate(_rd.endDate());
 m_routeNbr = _rd.route();
 m_routeName = _rd.routeName();
 m_companyKey = _rd.companyKey();
 m_alphaRoute = _rd.alphaRoute();;
 //ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(m_companyKey));
 routeDlg->setSegmentData(_rd);

 routeView->updateRouteView();

 //stationView->model()->setStationList(sql->getStations(m_alphaRoute, QDate::fromString(m_currRouteEndDate, "yyyy/MM/dd")));
  QList<SegmentData*> rsList = sql->getRouteSegmentsInOrder(_rd.route(), _rd.routeName(),_rd.companyKey(),ui->dateEdit->date());
 // QMap<int, StationInfo> stationMap;
 // for(SegmentData* sd : rsList)
 // {
 //  QList<StationInfo> sList = sql->getStationsOnSegment(sd->segmentId());
 //  for(StationInfo sti : sList)
 //   stationMap.insert(sti.stationKey, sti);
 // }

 stationView->model()->setStationList(getStations(rsList));
 stationView->model()->reset();

}

QList<StationInfo> MainWindow::getStations(QList<SegmentData*> rsList)
{
    QMap<int,StationInfo> stationMap;
    for(SegmentData* sd : rsList)
    {
        QList<StationInfo> sList = sql->getStationsOnSegment(sd->segmentId());
        for(StationInfo sti : sList)
        {
            if(sti.routeType != sd->routeType())
            {
                sti.routeType = sd->routeType();
                sql->updateStation(sti);
            }
            if(sti.routes.at(0).isEmpty()||sti.routes.at(0) == " ")
            {
                sti.routes.removeAt(0);
                sql->updateStation(sti);
            }
            if(!sti.routes.contains(m_alphaRoute))
            {
                sti.routes.append(m_alphaRoute);
                sql->updateStation(sti);
            }
            if(sti.endDate < _rd.startDate() || sti.startDate> _rd.endDate())
                continue;
            stationMap.insert(sti.stationKey, sti);
        }
    }
    return stationMap.values();
}

void MainWindow::refreshCompanies()
{
  if(bRefreshingCompanies)
    return;
  bRefreshingCompanies = true;
    ui->cbCompany->clear();
    if(!config->currCity->bDisplayRoutesForSelectedCompanies)
        ui->cbCompany->addItem(tr("All companies"),0);
    companyList = sql->getCompanies();
    config->currCity->selectedCompaniesList.clear();
    for(int i=0; i < companyList.count(); i++)
    {
        CompanyData* cd = companyList.at(i);
        if(config->currCity->bDisplayRoutesForSelectedCompanies)
        {
            // if(config->currCity->bDisplayRoutesForGroup &&
            //     config->currCity->selectedCompaniesList.contains(cd->companyKey))
          if(cd->bSelected)
            {
                ui->cbCompany->addItem(cd->name, cd->companyKey);
                config->currCity->selectedCompaniesList.append(cd->companyKey);
            }
        }
        else
        {
            ui->cbCompany->addItem(cd->name, cd->companyKey);
        }
    }
    bRefreshingCompanies = false;
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
void MainWindow::segmentSelected(qint32 pt, qint32 segmentId)
{
 //SQL sql;
 //webBrowser1.Document.InvokeScript("addModeOff");
 //m_bridge->processScript("addModeOff");
 //m_bAddMode = false;
 //addPointModeAct->setChecked(false);
    if((m_segmentId == segmentId && m_currPoint == pt) || segmentId<1)
  return;
 m_segmentId = segmentId;
 Q_ASSERT(m_segmentId > 0);
 SegmentInfo si = sql->getSegmentInfo(m_segmentId);
 if (si.segmentId() == -1)
 {
  qDebug() <<"segment " + QString("%1").arg(segmentId) + " not found";
  return;
 }
 if(routeDlg)
 {
  //routeDlg->setSegmentId(m_SegmentId);
  if (ui->cbRoute->currentIndex() != -1)
   routeDlg->setSegmentData ( (RouteData)routeList.at(        ui->cbRoute->currentIndex()));
 }
 //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
 lookupStreetName(si);
 //  ui->txtSegment->setText(si.description());
 //  ui->txtLocation->setText(si.location());
 //  ui->txtStreet->setText(si.streetName());
 //  ui->txtNewerName->setText(si.newerName());
 //  ui->sbTracks->setValue(si.tracks());

 //  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
 //  //ui->cbSegments->findText(si.toString(), Qt::MatchExactly);
 // // int ix = ui->cbSegments->findData(SegmentId);
 // // ui->cbSegments->setCurrentIndex(ix);
 //  m_points = si.pointList();
 //  m_nbrPoints = m_points.size();
 updateSegmentInfoDisplay(si);
 //Q_ASSERT(m_points.count() == 0 || m_points.count() >1);

 if (m_nbrPoints <= 0)
     return;
 if(pt < 0 || pt >= m_nbrPoints)
 {
  qDebug() << "pt invalid: " << QString::number(pt) << " array has " << QString::number(m_nbrPoints);
  m_currPoint = 0;
  pt = 0;
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

 QString marker;
 if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
 {
  marker = "-1";  // 0= blank red marker
  // marker = getMarkerImagePath("redblank.png", QString("point%1.png").arg(m_currPoint),QString::number(m_currPoint), 2.0);
  // marker = "'" +marker.mid(marker.indexOf("images"))+"'";
 }
 else if(m_currPoint == 0)
  marker = "1";
 else
  marker = "2";
 QVariantList objArray;
 m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","
                         +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","
                         +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","
                         +marker + ","
                         +QString("'%1'").arg(m_currPoint)+","
                         + QString("%1").arg(m_segmentId));

 if(config->currCity->bGeocoderRequest)
     m_bridge->processScript("geocoderRequest", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 //m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));

 qint32 irx =         ui->cbRoute->currentIndex();
 if (irx >= 0)
 {
  RouteData rd = RouteData();
  rd = (RouteData)routeList.at(irx);
  if(routeDlg)
   routeDlg->setSegmentData(rd);

  otherRouteView->showRoutesUsingSegment(segmentId);
 }
 if (!ui->chkNoPan->checkState())
 {
  m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 }
}

/// <summary>
/// Called by Google Maps when selecting a segment
/// </summary>
/// <param name="SegmentId"></param>
void MainWindow::segmentSelectedX(qint32 pt, qint32 segmentId, QList<LatLng> pointArray)
{
    if(segmentId < 1)
        return;
 m_segmentId = segmentId;
 Q_ASSERT(m_segmentId > 0);
 m_currPoint = pt;
 m_nbrPoints = pointArray.count()/2;
 SegmentInfo si = sql->getSegmentInfo(m_segmentId);
 if (si.segmentId() == -1)
 {
  qDebug() <<"segment " + QString("%1").arg(segmentId) + " not found";
  return;
 }
 // update SegmenInfo with new points
 if(routeDlg)
 {
  //routeDlg->setSegmentId(m_SegmentId);
  if (ui->cbRoute->currentIndex() != -1)
   routeDlg->setSegmentData ( (RouteData)routeList.at(        ui->cbRoute->currentIndex()));
 }
 //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
 lookupStreetName(si);
 //  //txtSegment.Text = si.description;
 //  ui->txtSegment->setText(si.description());
 //  ui->txtLocation->setText(si.location());
 //  ui->txtStreet->setText(si.streetName());
 //  ui->txtNewerName->setText(si.newerName());
 //  ui->sbTracks->setValue(si.tracks());
 //  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
 //  //ui->cbSegments->findText(si.toString(), Qt::MatchExactly);
 // // int ix = ui->cbSegments->findData(SegmentId);
 // // ui->cbSegments->setCurrentIndex(ix);
 //  m_points = si.pointList();
 //  m_nbrPoints = m_points.size();
 updateSegmentInfoDisplay(si);
 //Q_ASSERT(m_points.count() == 0 || m_points.count() >1);

 if (m_nbrPoints <= 0)
     return;
 if(pt < 0 || pt >= m_nbrPoints)
 {
  qDebug() << "pt invalid: " << QString::number(pt) << " array has " << QString::number(m_nbrPoints);
  m_currPoint = 0;
  pt = 0;
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

 QString marker;
 if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
 {
  marker = "-1";  // 0= blank red marker
  //marker = getMarkerImagePath("redblank.png", QString("point%1.png").arg(m_currPoint),QString::number(m_currPoint), 2.0);
  //marker = "'" +marker.mid(marker.indexOf("images"))+"'";
 }
 else if(m_currPoint == 0)
  marker = "1";
 else
  marker = "2";
 QVariantList objArray;
 m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","
                         +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","
                         +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","
                         +marker + ","
                         +QString("'%1'").arg(m_currPoint)+","
                         + QString("%1").arg(m_segmentId));

 if(config->currCity->bGeocoderRequest)
     m_bridge->processScript("geocoderRequest", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));
 //m_bridge->processScript("setCenter", QString("%1").arg(m_latitude,0,'f',8)+ "," + QString("%1").arg(m_longitude,0,'f',8));

 qint32 irx =         ui->cbRoute->currentIndex();
 if (irx >= 0)
 {
  RouteData rd = RouteData();
  rd = (RouteData)routeList.at(irx);
  if(routeDlg)
   routeDlg->setSegmentData(rd);

  otherRouteView->showRoutesUsingSegment(segmentId);
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
    QVariantList objArray;
    objArray << m_segmentId;
    m_bridge->processScript("getPointArray", objArray);
    connect(m_bridge, SIGNAL(on_scriptResult(QVariant)), this, SLOT(getArrayResult(QVariant)));
}
//    while(!m_bridge->isResultReceived())
//    {
//     qApp->processEvents(QEventLoop::AllEvents, 100);
//    }
void MainWindow::getArrayResult(QVariant v)
{
    disconnect(m_bridge, SIGNAL(on_scriptResult(QVariant)), this, SLOT(getArrayResult(QVariant)));

    QVariantList points;
    points = m_bridge->myList;
    m_nbrPoints = points.count()/2;
    m_points = QList<LatLng>();
    for(int i = 0; i < points.count(); i+=2)
    {
        m_points.append(LatLng(points[i].toDouble(), points[(i)+1].toDouble()));
    }
    if(m_points.count() == 1)
    {
     // 1 point is invalid, retrieve segment and use it's points
     SegmentInfo si = sql->getSegmentInfo(m_segmentId);
     if(si.pointList().count() > 1)
     {
      m_points = si.pointList();
      m_nbrPoints = m_points.count();
     }
     //else throw IllegalArgumentException("Bad array");
     else qWarning() << "bad Array";
    }

    //Q_ASSERT(m_points.count() == 0 || m_points.count() >1);
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
    if(m_segmentId < 1)
    {
        ui->btnFirst->setEnabled(false);
        ui->btnNext->setEnabled(false);
        ui->btnPrev->setEnabled(false);
        ui->btnLast->setEnabled(false);
        return;
    }
    // getArray();
    ui->btnSplit->setEnabled(false);
    m_currPoint=0;
    ui->lblPoint->setText(QString::number(m_currPoint));

    if(m_points.isEmpty())
        return;
    QVariantList objArray;
    objArray << 0 << ((LatLng)m_points.at(0)).lat()<<((LatLng)m_points.at(0)).lon()<<1<<"pointO"<<m_segmentId;
     m_bridge->processScript("addMarker",objArray);

    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(false);
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo si = sql->getSegmentInfo(m_segmentId);
    lookupStreetName(si);
    updateSegmentInfoDisplay(si);
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(0)).lat(), ((LatLng)m_points.at(0)).lon(),m_segmentId);
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(0)).lat()<<((LatLng)m_points.at(0)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
}

void MainWindow::btnNextClicked()
{
    // getArray();
    if(m_points.count() != m_nbrPoints)
        m_nbrPoints = m_points.count();
    if(m_currPoint < 0 || m_nbrPoints < 2)
        return;
    if ((m_currPoint + 1) < m_nbrPoints)
        m_currPoint++;
    ui->lblPoint->setText(QString::number(m_currPoint));

    QString marker;
    if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
    {
     //marker = "3";  // 0= blank red marker
     marker = "-1"; // default marker
     // marker = getMarkerImagePath("redblank.png", QString("point%1.png").arg(m_currPoint),QString::number(m_currPoint), 2.0);
     // marker = "'" +marker.mid(marker.indexOf("images"))+"'";
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
    if(m_points.isEmpty())
        return;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","
                            +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","
                            +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 )+","
                            +marker+","
                            +QString("'%1'").arg(m_currPoint)+","
                            + QString("%1").arg(m_segmentId));
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_segmentId);

    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo sd = sql->getSegmentInfo(m_segmentId);
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
// if(!bFirstSegmentDisplayed)
//  return;
    //SQL sql;

    //getArray();
    m_currPoint = m_nbrPoints - 1;
    ui->lblPoint->setText(QString::number(m_currPoint));

    if (m_nbrPoints < 1)
        return;

    ui->btnSplit->setEnabled(false);
    ui->btnPrev->setEnabled(true);
    ui->btnNext->setEnabled(false);
    QString marker = "2";  // end marker

    if(m_points.isEmpty())
        return;
    m_currPoint = m_points.count()-1;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","
                            +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)
                            +","+QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 )
                            +","+marker+",'point"+ QString("%1").arg(m_currPoint)+"',"+ QString("%1").arg(m_segmentId));

    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_segmentId);
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
    // getArray();
    if (m_currPoint > 0)
        m_currPoint--;
    ui->lblPoint->setText(QString::number(m_currPoint));

    QString marker;
    if (m_currPoint > 0 && (m_currPoint < (m_nbrPoints - 1)))
    {
    //marker = "3";  // red blank marker
        marker = "-1"; // default marker
     // marker = getMarkerImagePath("redblank.png", QString("point%1.png").arg(m_currPoint),QString::number(m_currPoint), 2.0);
     // marker = "'" +marker.mid(marker.indexOf("images"))+"'";
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
    if(m_points.isEmpty())
        return;
    if(m_currPoint < 0)
     m_currPoint = 0;
    QVariantList objArray;
    m_bridge->processScript("addMarker",QString("%1").arg(m_currPoint)+","
                            +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)+","
                            +QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8 ) +","
                            +marker + ","
                            +QString("'%1'").arg(m_currPoint)+","
                            + QString("%1").arg(m_segmentId));
    if(!ui->chkNoPan->isChecked())
    {
        objArray.clear();
        objArray<<((LatLng)m_points.at(m_currPoint)).lat()<<((LatLng)m_points.at(m_currPoint)).lon();
        m_bridge->processScript("setCenter", objArray);
    }
    segmentView->showSegmentsAtPoint(((LatLng)m_points.at(m_currPoint)).lat(), ((LatLng)m_points.at(m_currPoint)).lon(),m_segmentId);
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    SegmentInfo sd = sql->getSegmentInfo(m_segmentId) ;
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
 m_bridge->processScript("getCurrBounds");
 m_bridge->processScript("getCenter");

 QSettings settings;
 //settingsDb settings;
 settings.setValue("geometry", saveGeometry());
 settings.setValue("windowState", saveState());
 settings.setValue("splitter", ui->splitter->saveState());
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
 settings.setValue("dupSegmentsView", ui->tblDupSegments->horizontalHeader()->saveState());
 settings.setValue("streetsView",ui->tblStreetView->horizontalHeader()->saveState());

 QSqlDatabase db = QSqlDatabase::database();
 db.close();
//#ifndef QT_DEBUG
// //systemConsoleAction->close();
// delete SystemConsole2::getInstance();
//#endif
// if(webViewAction != NULL)
//  webViewAction->closeWebView();
 //QMainWindow::closeEvent(event);
 event->accept();
}
void MainWindow::btnSplit_Clicked()    // SLOT
{
// if(!bFirstSegmentDisplayed)
//  return;
 //SQL sql;
 SegmentDlg segmentDlg(this);
 //segmentDlg.setSegmentId( m_segmentId);
 //segmentDlg.setPt(m_currPoint);
// if(ui->cbRoute->currentIndex() >=0)
//  segmentDlg.setRouteData( routeList.at(ui->cbRoute->currentIndex()));
 segmentDlg.configure(new RouteData(routeList.at(ui->cbRoute->currentIndex())), m_segmentId, m_currPoint);
 connect(&segmentDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(segmentDlg_routeChanged(RouteChangedEventArgs)));
 //string RouteNames = "No route";

 if (segmentDlg.exec() == QDialog::Accepted)
 {
  m_segmentId = segmentDlg.newSegmentId();
     Q_ASSERT(m_segmentId > 0);

  if(routeDlg)
   routeDlg->setSegmentId( m_segmentId);

  // Refresh the Segments combobox
  //refreshSegmentCB();
  ui->ssw->refresh();
  refreshRoutes();

  // Display the new segment in Google Maps
  //Object[] objArray = new Object[1];

  // redisplay the original altered segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.SegmentId()));
  //ui->chkOneWay->setChecked("Y"== sql->getSegmentOneWay(segmentDlg.SegmentId()));
  displaySegment(segmentDlg.SegmentId(), ui->txtSegment->text(),
                 /*sql->getSegmentOneWay(segmentDlg.newSegmentId()), */"#b45f04",
                 " ", true);

  // display the new segment
  ui->txtSegment->setText(sql->getSegmentDescription(segmentDlg.newSegmentId()));
  displaySegment(segmentDlg.newSegmentId(), ui->txtSegment->text(),
                 /*sql->getSegmentOneWay(segmentDlg.newSegmentId()), */"#b45f04",
                 " ", false);

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
void MainWindow::displaySegment(qint32 segmentId, QString segmentName,
                                /*QString oneWay,*/ QString color, QString trackUsage,
                                bool bClearFirst)
{
    SegmentInfo si = sql->getSegmentInfo(segmentId);
    si.displaySegment(ui->dateEdit->text(),color, trackUsage, bClearFirst);
    m_currPoint = 0;
    //bFirstSegmentDisplayed = true;
    ui->lblPoint->setText(QString::number(m_currPoint));
    m_points = si.pointList();

    ui->btnFirst->setEnabled(true);
    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(false);
    ui->btnLast->setEnabled(true);
    ui->btnSplit->setEnabled(false);
    ui->btnDeletePt->setEnabled(true);
    // getArray();
    return;
}

//void mainWindow::selectSegment( )
//{
// cbSegmentsSelectedValueChanged(ui->cbSegments->currentData().toInt());
//}

void MainWindow::cbSegmentsSelectedValueChanged(SegmentInfo si)
{
    if(si.segmentId() < 0)
        return;
    m_segmentId = si.segmentId();
    Q_ASSERT(m_segmentId > 0);
    updateSegmentInfoDisplay(si);
    m_points = si.pointList();
    //Q_ASSERT(m_points.count() !=1);

    //routeDlg->setSegmentId( m_segmentId);
    SegmentData* sd = new SegmentData(ui->ssw->segmentSelected());
    sd->setRoute(_rd.route());
    sd->setAlphaRoute(_rd.alphaRoute());
    sd->setRouteName(_rd.routeName());
    sd->setStartDate(_rd.startDate());
    sd->setEndDate(_rd.endDate());
    sd->setTractionType(_rd.tractionType());
    sd->setCompanyKey(_rd.companyKey());

    routeDlg->setSegmentData(sd);

    //webBrowser1.Document.InvokeScript("addModeOn");
    if(si.startLat() == 0)
    {
     QVariantList objArray;
     objArray << m_segmentId;
     m_bridge->processScript("addModeOn", objArray);
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
        objArray << si.startLat() << si.startLon();
        m_bridge->processScript("setCenter", objArray);
    }

    displaySegment(m_segmentId, ui->txtSegment->text(),
                   /*sd.oneWay(),*/ /*sd.oneWay()=="Y" ? "#00FF00" :*/
                   "#045fb4", " ", true);

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
 //bSegmentChanged = true;
 int ix = text.indexOf(",");
 QString street;
 if(ix > 0 )
  street = text.mid(0,ix);
 if(street != ui->txtStreet->text())
  ui->txtStreet->setText(street);
}


void MainWindow::txtSegment_Leave( )
{
  if(ui->txtSegment->text().isEmpty())
     return;
  ui->txtSegment->on_editingFinished();
  SegmentInfo si = sql->getSegmentInfo(m_segmentId);
  si.setDescription(ui->txtSegment->replaceAbbreviations(ui->txtSegment->text()));
  ui->txtSegment->setText(si.description());
  si.setFormatOK(ui->txtSegment->isValidFormat());
  si.setTracks(ui->sbTracks->value());
  si.setStreetName(ui->txtSegment->segmentDescription()->street());
  ui->txtStreet->setText(ui->txtSegment->segmentDescription()->street());
  si.setDescription(ui->txtSegment->text());
  si.setTracks(ui->sbTracks->value());
  sql->updateSegment(&si);

  processDescriptionChange(ui->txtSegment->text(), ui->txtStreet->text());
}


void MainWindow::cbSegmentsTextChanged(QString )
{
 b_cbSegments_TextChanged = true;
}


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

   if (rd.alphaRoute().contains(text))
   {
    //cbRoutes.SelectedItem = rd;
    ui->cbRoute->setCurrentIndex(i);
    break;
   }
  }
  b_cbRoutes_TextChanged = false;
 }
}

#if 0
QString MainWindow::getRouteMarkerImagePath(QString route, bool isStart)
{
 QString tmplt ="";
 QString name = "";
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

 return getMarkerImagePath(tmplt, name, route, 0);
}

QString MainWindow::getMarkerImagePath(QString tmplt, QString name, QString text, double offset)
{
    QString work = m_resourcePath;
    QString str = "";
    //QString name = "";
    QDir dir(m_resourcePath);
    if(!dir.exists("images"))
        dir.mkdir("images");

    QBrush brBkgnd = QBrush(Qt::SolidPattern);
    QFont f;
    QFile file(dir.filePath(work+ "/images/" + name));
    str = "file://" + work + "/images/" + name;

    if (file.exists( ) && file.size() > 0)
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
    if(text.length() == 3)
        f =  QFont("Arial", 7, QFont::Bold, false);
    else
        f =  QFont("Arial", 6, QFont::Bold, false);
    painter.setFont(f);

//    QRect eRect =painter.boundingRect(r, Qt::AlignCenter, "000");
//    eRect.adjust(0, -3.0, 0, 0);
//    painter.fillRect(eRect, brBkgnd);
    QRectF bRect = painter.boundingRect(r, Qt::AlignCenter, text); // bounding rectangle of text
    bRect.adjust(0, -3.0, 0, 0);
    bRect.moveTop(offset);
    painter.fillRect(bRect, brBkgnd);
    painter.setPen(Qt::white);
    painter.drawText(bRect, Qt::AlignCenter, text);
    painter.end();
    resultImage.save(work+ "/images/" + name, "PNG");

    return str;
}
#endif
QString MainWindow::ProcessScript(QString func, QString params)
{
    m_bridge->processScript(func, params);
    return "";
}
#if 1
void MainWindow::addPoint(int pt, double lat, double lon)
{
    //SQL sql;
    //getArray(); // get points from display.
    SegmentInfo si = sql->getSegmentInfo(m_segmentId);
    m_points = si.pointList();
    m_points.append(LatLng(lat,lon));
    m_nbrPoints = m_points.size();
    //int begin = (int)m_nbrPoints - 2, End = (int)m_nbrPoints - 1;
#if 1
    if(m_nbrPoints == 2 && si.startLat() == 0 && si.startLon() == 0)
    {
     si.addPoint(m_points.at(0));
     si.setStartLat(m_points.at(0).lat());
     si.setStartLon(m_points.at(1).lon());
    }
    if(si.segmentId() == m_segmentId && m_nbrPoints > 0)
    {
     si.addPoint(m_points.at(m_nbrPoints-1 ));
    }
    else
    {
     si.addPoint(m_points.at(0));
    }
#endif
    // if(m_nbrPoints > 1)
    //     si.displaySegment(si.startDate().toString("yyyy/MM/dd"), getColor(_rd.tractionType()), " ",false);

    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
}
#else
void MainWindow::addPoint(int pt, double lat, double lon)
{
    //SQL sql;
    //getArray(); // get points from display.
    SegmentInfo si = sql->getSegmentInfo(m_segmentId);
    m_points = si.pointList();
    m_points.append(LatLng(lat,lon));
    m_nbrPoints = m_points.size();
    //int begin = (int)m_nbrPoints - 2, End = (int)m_nbrPoints - 1;

    if(m_nbrPoints == 2 && si.startLat() == 0 && si.startLon() == 0)
    {
        si.addPoint(m_points.at(0));
        si.setStartLat(m_points.at(0).lat());
        si.setStartLon(m_points.at(1).lon());
    }
    if(si.segmentId() == m_segmentId && m_nbrPoints > 0)
    {
        si.addPoint(m_points.at(m_nbrPoints-1 ));
    }
    else
    {
        si.addPoint(m_points.at(0));
    }
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
}
#endif
// void MainWindow::addPointX(int pt, QList<LatLng> points)
// {
//     //SQL sql;
//     //getArray(); // get points from display.
//     SegmentInfo si = sql->getSegmentInfo(m_segmentId);
//     m_points = si.pointList();
//     m_points.append(LatLng(points.at(pt)));
//     m_nbrPoints = m_points.size();
//     //int begin = (int)m_nbrPoints - 2, End = (int)m_nbrPoints - 1;

//     if(m_nbrPoints == 2 && si.startLat() == 0 && si.startLon() == 0)
//     {
//         si.addPoint(m_points.at(0));
//         si.setStartLat(m_points.at(0).lat());
//         si.setStartLon(m_points.at(1).lon());
//     }
//     if(si.segmentId() == m_segmentId && m_nbrPoints > 0)
//     {
//         si.addPoint(m_points.at(m_nbrPoints-1 ));
//     }
//     else
//     {
//         si.addPoint(m_points.at(0));
//     }
//     ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
// }

/// <summary>
/// Called by Google Maps when a point on a polyline is moved.
/// </summary>
/// <param name="i"></param>
/// <param name="newLat"></param>
/// <param name="newLon"></param>
void MainWindow::movePoint(qint32 segmentId, qint32 i, double newLat, double newLon)
{
  if(segmentId == 0)
    return;
  LatLng oldPoint; // = sql->getPointOnSegment((int)i, m_segmentId);
  SegmentInfo si = sql->getSegmentInfo(segmentId);

  if(si.segmentId() == m_segmentId)
  {
   oldPoint = si.pointList().at(i);
   m_points = si.movePoint(i, LatLng(newLat, newLon));
   m_nbrPoints = m_points.count();
   m_currPoint = i;
  }
  else
  {
      updateSegmentInfoDisplay(si);
      m_currPoint = i;
  }
  Q_ASSERT(si.segmentId() == m_segmentId);
  //SQL sql;

  //TODO what about multiple station records?
  QList<StationInfo> stiList = sql->getStationAtPoint(oldPoint);
  for(StationInfo sti : stiList)
  {
   if (sti.stationKey > 0)
   {
    sti.latitude = newLat;
    sti.longitude = newLon;
       sql->updateStation(sti);
   }
  }
}

/// <summary>
/// Called by Google Maps when a point on a polyline is moved.
/// </summary>
/// <param name="i"></param>
/// <param name="newLat"></param>
/// <param name="newLon"></param>
void MainWindow::movePointX(qint32 segmentId, qint32 i, LatLng newPt, QList<LatLng> pointlist)
{
    //m_segmentId = segmentId;
    if(segmentId < 1)
        return;
    SegmentInfo si = sql->getSegmentInfo(segmentId);
    LatLng oldPoint = si.pointList().at(i);

    if(si.segmentId() >0)
    {
        if(pointlist.count() != si.pointList().count())
            return;

        si.setPoints(pointlist);
        m_points = pointlist;
        sql->updateSegment(&si);
        updateSegmentInfoDisplay(si);
    }
    SegmentInfo newSi = sql->getSegmentInfo(segmentId);

    //SQL sql;
    //TODO what about multiple station records?
    QList<StationInfo> stiList = sql->getStationAtPoint(oldPoint);
    for(StationInfo sti : stiList)
    {
        if (sti.stationKey > 0)
        {
            sti.latitude = pointlist.at(i).lat();
            sti.longitude = pointlist.at(i).lon();
            sql->updateStation(sti);
        }
    }
}


// called by webBrowser map initialization to see if an overlay should be loaded
void MainWindow::queryOverlay()
{
    if(!ui->chkShowOverlay->isChecked())
        return;
    if(config->currCity->curOverlayId >= config->currCity->city_overlayMap->count())
     config->currCity->curOverlayId =0;
    if(config->currCity->curOverlayId >= 0)
    {
        Overlay* ov = config->currCity->city_overlayMap->values().at(config->currCity->curOverlayId );
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
 SegmentInfo sd = sql->getSegmentInfo(m_segmentId);
 // get all the points within 100 meters
 //QList<segmentData> myArray = sql->getIntersectingSegments(newLat, newLon, .020, si.routeType);
 QList<SegmentInfo> myArray = sql->getIntersectingSegments(newLat, newLon, .020, sd.routeType());
 int currSegment = m_segmentId;
 //foreach (segmentData sd in myArray)
 for(int i=0; i< myArray.count(); i++)
 {
  //segmentData sd= (segmentData)myArray.at(i);
  SegmentData sd = myArray.at(i);
  //QString oneWay = sql->getSegmentOneWay(sd.SegmentId);
  m_segmentId = sd.segmentId();
  //m_nbrPoints = sql->getNbrPoints(m_SegmentId);
  m_nbrPoints = sd.pointList().count();
  if(sd.whichEnd() == "S")
   movePoint(m_segmentId, 0, newLat, newLon);
  else
   movePoint(m_segmentId, m_nbrPoints -1, newLat, newLon);
  //displaySegment(sd.SegmentId, sql->getSegmentDescription(si.SegmentId), oneWay, oneWay == "N" ? "#00FF00" : "#045fb4", true);
  displaySegment(sd.segmentId(), sd.description(),
                 /*sd.oneWay(),*/ /*sd.oneWay() == "N" ? "#00FF00" : */
                 "#045fb4", sd.trackUsage(),  true);
 }
 m_segmentId = currSegment;
}


/// <summary>
/// Called by Google Maps when a point on a polyline is inserted
/// </summary>
/// <param name="i">point to insert after</param>
/// <param name="newLat">New latitude</param>
/// <param name="newLon">New longitude</param>
void MainWindow::insertPoint(int segmentId, qint32 i, double newLat, double newLon)
{
    if(segmentId < 1)
        return;
 segmentSelected(i,segmentId);
 SegmentInfo si = sql->getSegmentInfo((int)segmentId);
 si.insertPoint(i, LatLng(newLat, newLon));
#if 0
    //SQL sql;
    sql->insertPoint(i, SegmentId, newLat, newLon);
    m_currPoint++;
#endif
 m_currPoint = i+1;
    m_segmentId = segmentId;
    Q_ASSERT(m_segmentId > 0);
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    si = sql->getSegmentInfo(m_segmentId);
    lookupStreetName(si);
    //ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
    updateSegmentInfoDisplay(si);
    ui->btnSplit->setEnabled(true);
}
/// <summary>
/// Called by Google Maps when a point on a polyline is inserted
/// </summary>
/// <param name="i">point to insert after</param>
/// <param name="newLat">New latitude</param>
/// <param name="newLon">New longitude</param>
void MainWindow::insertPointX(int segmentId, qint32 i, QList<LatLng> points)
{
    segmentSelected(i,segmentId);
    SegmentInfo si = sql->getSegmentInfo((int)segmentId);
    //si.insertPoint(i, LatLng(newLat, newLon));
    si.setPoints(points);
    m_points = points;
#if 0
    //SQL sql;
    sql->insertPoint(i, SegmentId, newLat, newLon);
    m_currPoint++;
#endif
    m_currPoint = i+1;
    m_segmentId = segmentId;
    //segmentData sd = sql->getSegmentData(m_currPoint, m_SegmentId);
    si = sql->getSegmentInfo(m_segmentId);
    lookupStreetName(si);
    ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
    ui->btnSplit->setEnabled(true);
}



/// <summary>
/// Delete the point at the current marker
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void MainWindow::btnDeletePtClicked()
{
// if(!bFirstSegmentDisplayed)
//  return;
   SegmentInfo sd = sql->getSegmentInfo(m_segmentId);

   if(sd.segmentId() == m_segmentId)
   {
    if(m_nbrPoints < 3)
    {
     statusBar()->showMessage("only 2 points. use deleteSegment instead!");
     return;
    }
    sd.deletePoint(m_currPoint);
    m_points = sd.pointList();
    m_nbrPoints - m_points.count();

    QVariantList objArray;
    objArray << m_currPoint << m_segmentId;
    m_bridge->processScript("deletePoint", objArray);
    if (m_currPoint > 0)
        m_currPoint--;
    ui->lblPoint->setText(QString::number(m_currPoint));


    QString marker;
    // getArray();
    if(m_nbrPoints == 0)
    {
     m_points = sd.pointList();
     m_currPoint = sd.pointList().size()-1;
    }
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
  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(sd.pointList().count()));
  //webBrowser1.Document.InvokeScript("addMarker", objArray);
  m_bridge->processScript("addMarker", QString("%1").arg(m_currPoint)+ ","
                          + QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lat(),0,'f',8)
                          + "," + QString("%1").arg(((LatLng)m_points.at(m_currPoint)).lon(),0,'f',8)
                          + ","+marker+",'',"+QString("%1").arg(m_segmentId));
  objArray.clear();
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
 //bSegmentChanged = true;
 txtStreetName_Leave();
}

void MainWindow::txtStreetName_Leave()
{
 if (bStreetChanged && (m_currPoint < m_nbrPoints) && m_segmentId > -1)
 {
  //segmentData sd = segmentData(m_currPoint, m_SegmentId);
  SegmentInfo si = sql->getSegmentInfo(m_segmentId);
  if(si.pointList().count()<2) return;
  si.setStartLat(si.pointList().at(0).lat());
  si.setStartLon(si.pointList().at(0).lon());
  si.setEndLat(si.pointList().at(si.pointList().count()-1).lat());
  si.setEndLon(si.pointList().at(si.pointList().count()-1).lon());
  si.setNewerName(ui->txtNewerName->text().trimmed());
  bool bUpdate = false;
  if( ui->txtStreet->text().trimmed() != si.getStreetName())\
  {
   si.getStreetName() = ui->txtStreet->text().trimmed();
   bUpdate = true;
  }
  if(ui->sbTracks->value() != si.tracks())
  {
   si.setTracks(ui->sbTracks->value());
   bUpdate = true;
  }
  if(bUpdate)
  {
   //SQL mySql;
   sql->updateRecord(si);
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
    setCursor(Qt::WaitCursor);
 DialogCopyRoute form(routeList.at(ui->cbRoute->currentIndex()), this);
 //form.Configuration = config;
 //form.setRouteData( (RouteData)routeList.at(ui->cbRoute->currentIndex()));

 int rslt =  form.exec();
 if (rslt == QDialog::Accepted)
 {
  refreshRoutes();
  RouteData rd = form.getRouteData();
  for(int i=0; i < routeList.count(); i++)
  {
   RouteData _rd = (RouteData)routeList.at(i);
   if (rd.route() == _rd.route() && rd.routeName() == _rd.routeName() && rd.endDate() == _rd.endDate())
   {
    bNoDisplay = true;
    ui->cbRoute->setCurrentIndex(i);
    bNoDisplay = false;
   }
  }
 }
 setCursor(Qt::ArrowCursor);
}

void MainWindow::splitRoute_Click()
{
    //SplitRoute();
    SplitRoute splitRouteDlg(this);
    //splitRouteDlg.setConfiguration (config);
    if(!splitRouteDlg.setRouteData (routeList.at(ui->cbRoute->currentIndex())))
        return;
    if (splitRouteDlg.exec() == QDialog::Accepted)
    {
        RouteData newRoute = splitRouteDlg.getNewRoute();
        refreshRoutes();
        for(int i =0; i < routeList.count(); i++)
        {
            RouteData rd = routeList.at(i);
            if(rd.route() == newRoute.route() && rd.routeName() == newRoute.routeName()
               && rd.endDate() == newRoute.endDate())
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
    ModifyRouteDialog renameRouteDlg(this);
    //renameRouteDlg.setConfig(config);
    renameRouteDlg.routeData((RouteData)routeList.at(ui->cbRoute->currentIndex()));
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
    //form.setConfiguration(config);
    //form.setRouteData(routeList.at(ui->cbRoute->currentIndex()));
    form.setRouteData(&_rd);

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
 ModifyRouteTractionTypeDlg* form = new ModifyRouteTractionTypeDlg(routeList.at(ui->cbRoute->currentIndex()));
 //form->setConfiguration(config);
 //form->setRouteData(routeList, ui->cbRoute->currentIndex());

 qint32 rslt = form->exec();
 if (rslt == form->Accepted)
 {
  refreshRoutes();

  //cbRoutes.SelectedItem = form.RouteData;
  //cbRoutes.SelectedIndex = cbRoutes.FindString(form.RouteData.ToString());
  for(int i=0; i <routeList.count(); i++)
  {
   RouteData rd =routeList.at(i);
   if(rd.toString() == form->getRouteData().toString())
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
    m_bridge->processScript("addModeOff");
    SegmentDlg*  segmentDlg = new SegmentDlg(this);
//TODO    segmentDlg.routeChanged += new routeChangedEventHandler(segmentDlg_routeChanged);
    connect(segmentDlg, &SegmentDlg::companySelectionChanged, [=](/*int companyKey*/){
     ui->cbCompany->setCurrentIndex( ui->cbCompany->findData(_rd.companyKey()));
     refreshRoutes();
    } );
    connect(segmentDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(segmentDlg_routeChanged(RouteChangedEventArgs)));
    //segmentDlg.setConfiguration( config);
    //segmentDlg.setSegmentId( -1);
    m_currPoint = 0;
    ui->lblPoint->setText(QString::number(m_currPoint));

    //segmentDlg.setPt( m_currPoint);
    //int ix = cbRoutes.SelectedIndex;
    //if(ix > -1)
    //{
    //    segmentDlg.RouteData = (routeData)routeDataList[ix];
    //}
    RouteData* rd = nullptr;
    if(ui->cbRoute->currentIndex() >= 0)
     rd = new RouteData(routeList.at(ui->cbRoute->currentIndex()));
    if(!rd)
    {
        rd = new RouteData();
        CompanyData* cd =sql->getCompany(ui->cbCompany->currentData().toInt());
        rd->setStartDate(cd->startDate);
        rd->setEndDate(cd->endDate);
    }
    //segmentDlg.setRouteData (rd);
    segmentDlg->configure(rd, -1, m_currPoint);
    if (segmentDlg->exec() == QDialog::Accepted)
    {
        //refreshSegmentCB();
       ui->ssw->refresh();
       m_segmentId = segmentDlg->newSegmentId();
       Q_ASSERT(m_segmentId > 0);
        m_nbrPoints = 0;

        ui->lblSegment->setText(tr("Segment %1:").arg(m_segmentId));
        SegmentData sd = segmentDlg->segment();
        updateSegmentInfoDisplay(sd);

        int dash = 0;
        if(sd.routeType() == Incline)
         dash = 1;
        else if(sd.routeType() == SurfacePRW)
         dash = 2;
        else if(sd.routeType() == Subway)
         dash = 3;

        QVariantList objArray;
        objArray << m_segmentId << segmentDlg->routeName()<<ui->txtSegment->text()
                 << sd.oneWay() <<true<<getColor(segmentDlg->tractionType())<<sd.tracks()
                 << dash << sd.routeType() << sd.trackUsage() << 0;
        m_bridge->processScript("createSegment",objArray);

        //webBrowser1.Document.InvokeScript("addModeOn");
        m_bridge->processScript("addModeOn");
        m_bAddMode = true;

        SegmentInfo si = sql->getSegmentInfo(m_segmentId);
        lookupStreetName(si);
        //refreshRoutes();
        //refreshSegmentCB();
    }
}
bool MainWindow::deleteRoute()
{
    //SQL sql;
    RouteData rd = routeList.at(ui->cbRoute->currentIndex());

    QMessageBox::StandardButton reply;
    reply=QMessageBox::warning(this, tr("Confirm Delete"),tr("Are you sure you want to delete route ")
                               + rd.alphaRoute() + " " + rd.routeName() + "?", QMessageBox::Yes | QMessageBox::No);
    if(reply== QMessageBox::Yes)
    {
        sql->beginTransaction("deleteRoute");
        if(sql->deleteRoute(rd.route(), rd.routeId(), rd.startDate().toString("yyyy/MM/dd"), rd.endDate().toString("yyyy/MM/dd")))
        {
            if(!sql->executeCommand(QString("delete from RouteName where routeId = %1").arg(rd.routeId())))
            {
                sql->rollbackTransaction("deleteRoute");
                return false;
            }
            sql->commitTransaction("deleteRoute");
        }
    }
    refreshRoutes();
    return true;
}

void MainWindow::on_updateRoute()
{
  SegmentData* sd = sql->getSegmentData(m_routeNbr, m_segmentId, _rd.startDate().toString("yyyy/MM/dd"),
                                       _rd.endDate().toString("yyyy/MM/dd"));
  if(!sd)
   return;
  sd->setRouteName(_rd.routeName());
  sd->setCompanyKey(_rd.companyKey());
  sd->setTractionType(_rd.tractionType());
  updateRoute(sd);
}

void MainWindow::updateRoute(SegmentData* sd )
{
    //SQL sql;
//        QList<SegmentInfo> segmentInfoList = sql->getSegmentInfo();

    if (!routeDlg)
    {
        routeDlg = new RouteDlg(this);
        //routeDlg->Configuration ( config);
        //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
        connect(routeDlg, SIGNAL(SegmentChangedEvent(qint32, qint32)),this, SLOT(segmentChanged(qint32,qint32)));
        //routeDlg->routeChanged += new routeChangedEventHandler(RouteChanged);
        connect(routeDlg, SIGNAL(routeChangedEvent(RouteChangedEventArgs)), this, SLOT(routeChanged(RouteChangedEventArgs)));
    }

    if(sd)
    {
     try{
//      routeDlg->setRouteNbr( sd->route());
//      //routeDlg->setSegmentId( sd->segmentId());
//      if(sd->alphaRoute().isEmpty())
//       sd->setAlphaRoute(_rd.alphaRoute());
      if(sd->companyKey()<0)
       sd->setCompanyKey(ui->cbCompany->currentData().toInt());
      routeDlg->setSegmentData(sd);
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

    if(m_segmentId >= 0)
    {
     routeDlg->setRouteNbr( m_routeNbr);
     routeDlg->setSegmentId( m_segmentId);
     routeDlg->setSegmentData(_rd);
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
    if(seg < 1)
        return;
 ui->ssw->setCurrentSegment(seg);
 m_segmentId = seg;
 Q_ASSERT(m_segmentId > 0);
 SegmentInfo si = sql->getSegmentInfo(seg);
 updateSegmentInfoDisplay(si);
 QList<SegmentInfo> dups = sql->getDupSegments(si);
 //ui->txtStreet->setText(si.streetName());
 ui->txtNewerName->setText(si.newerName());
 ui->txtSegment->setText(si.description());
 ui->txtStreet->setText(ui->txtSegment->streetName());
}

void MainWindow::segmentChanged(qint32 changedSegment, qint32 newSegment)
{
    m_bridge->processScript("clearPolyline", QString("%1").arg(changedSegment));
    m_bridge->processScript("clearMarker");

    if (newSegment > 0)
    {
     SegmentInfo si = sql->getSegmentInfo(newSegment);
     QList<SegmentData> segments = sql->getRouteSegmentsBySegment(m_routeNbr, newSegment);

     // displaySegment(newSegment, si.description(), /*sd.oneWay(),*/
     //                m_segmentColor, " ", true);
     if(!segments.isEmpty())
         si.displaySegment(segments.at(0).startDate().toString("yyyy/MM/dd"),m_segmentColor,segments.at(0).trackUsage(),true);
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
            if (rd.route() == args.routeNbr && rd.routeName() == args.routeName && args.dateEnd == rd.endDate())
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
void MainWindow::routeChanged(RouteChangedEventArgs args)
{
 //SQL sql;
 refreshRoutes();
 RouteData rd = args.rd;
 for(int i=0; i < routeList.count(); i++)
 {
  RouteData rd1 = routeList.at(i);
  if(rd1.route() == args.routeNbr && args.routeName == rd1.routeName() && rd1.endDate() == args.dateEnd)
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
  displaySegment(args.routeSegment, rd.routeName(),
                 /*rd.oneWay,*/ /*ttColors[e.tractionType]*/getColor(args.tractionType), rd.trackUsage(), true);
 }
 routeView->updateRouteView();
 ui->tabWidget->setCurrentIndex(0);
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
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, QDate::fromString(m_currRouteEndDate,"yyyy/MM/dd"));
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate, ti.endDate, segmentId,
                         i == 0 ? "S" : "E", ti.endSegment, ti.endWhichEnd);
}
void MainWindow::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //SQL sql;
    TerminalInfo ti = sql->getTerminalInfo(m_routeNbr, m_routeName, QDate::fromString(m_currRouteEndDate,"yyyy/MM/dd"));
    sql->updateTerminals(m_routeNbr, m_routeName, ti.startDate, ti.endDate, ti.startSegment,
                         ti.startWhichEnd, segmentId, i == 0 ? "S" : "E");
}

void MainWindow::addRoute()
{
    if(routeDlg == 0)
        routeDlg = new RouteDlg(this);
    if(!ui->ssw->cbSegments()->currentIndex()>0)
    {
     QMessageBox::warning(this, tr("No Segment"), tr("In order to create a newRoute, an existing segment must be selected."));
     return;
    }
    else
    {
     SegmentData* sd = new SegmentData(ui->ssw->segmentSelected());
     routeDlg->setSegmentData(sd);
    }
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
    config->currCity->bDisplayStationMarkers = bChecked;
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
    loadRouteComment();
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
  QList<StationInfo> stiList = sql->getStationAtPoint(pt);
  SegmentInfo si = sql->getSegmentInfo(segmentId);
  //EditStation form(-1, bDisplayStationMarkers, this);

  // if multiple segments meet here, get a list of them
  QList<SegmentInfo> sList = sql->getIntersectingSegments(lat, lon, .020, _rd.routeType());

  StationInfo sti;
  for(StationInfo sti1 : stiList)
  {
   sti1.latitude = lat;
   sti1.longitude = lon;
   sti1.segmentId = segmentId;
   QString aSegmentId = QString::number(segmentId);
   if(!sti1.segments.contains(aSegmentId))
    sti1.segments.append(aSegmentId);
   if(ui->dateEdit->date()>= sti1.startDate && ui->dateEdit->date() <= sti.endDate)
    sti = StationInfo(sti1);
   for(SegmentInfo si : sList)
   {
    QString sTxt = QString::number(si.segmentId());
    if(!sti1.segments.contains(sTxt))
      sti1.segments.append(sTxt);
   }
   sql->updateStation(sti1);
  }
  QDate dt;
  if(si.segmentId() >0)
  {
   dt = si.startDate();
   sti.startDate = dt;
   dt = si.endDate();
   sti.endDate = dt;
  }

  QString markerType = "green";
  switch (si.routeType())
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
  sti.markerType = markerType;
  EditStation* form = new EditStation(sti);
  form->exec();
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

    EditStation form(sti, this);
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
//  int stationKey = sql->addStation(sti.stationName,LatLng(sti.latitude, sti.longitude),segmentId,sd.startDate().toString("yyyy/MM/dd"),
//                                   sd.endDate().toString("yyyy/MM/dd"),sti.geodb_loc_id, sti.infoKey,sd.routeType(),sti.markerType,sti.point);
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
//     if(sti.geodb_loc_id > 0)
//         sql->updateStation(sti.geodb_loc_id, stationKey,  LatLng(lat, lon));
//     else
         sti.latitude = lat;
         sti.longitude = lon;
         sql->updateStation(sti);
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
 SegmentInfo sd = sql->getSegmentInfo(m_segmentId);
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
 //bSegmentChanged = true;
 //txtSegment_Leave();
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
    QList<SegmentInfo> myArray;
    this->setCursor(QCursor(Qt::WaitCursor));
    //foreach(segmentInfo si in segmentInfoList)
    foreach(SegmentInfo si, cbSegmentInfoList)
    {
//        SegmentInfo si = cbSegmentInfoList.at(i);
        QList<SegmentInfo> siDups = sql->getSegmentsInSameDirection(si);
        foreach(SegmentInfo siDup, siDups)
        {
          if(siDup.segmentId() > -1)
              myArray.append(siDup);
          qApp->processEvents();
        }
    }
//    if(ui->tabWidget->count() < 7)
//     ui->tabWidget->addTab(ui->tblDupSegments,"DuplicateSegments");
    // dupSegmentView->showDupSegments(myArray);
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
  SegmentInfo si = sql->getSegmentInfo(segmentId);
  displaySegment(si.segmentId(), si.description(), getColor(_rd.tractionType()), "B", true);
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
 RouteCommentsDlg routeCommentsDlg(this);

 int row =         ui->cbRoute->currentIndex();
 RouteData rd = ((RouteData)routeList.at(row));
 routeCommentsDlg.setCompanyKey(rd.companyKey());
 routeCommentsDlg.setRoute(rd.route());
 routeCommentsDlg.setDate(rd.startDate());
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
 queryDlg->raise();
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

  //if(!QWidget::grab(ui->webView->rect()).save(saveFilename, qPrintable(saveExtension)))
  if(!ui->webView->grab().save(saveFilename, qPrintable(saveExtension)))
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
 config->bDisplayDebugMsgs = b;
}

void MainWindow::on_runInBrowser(bool bRunInBrowser)
{
 if(!bRunInBrowser)
 {
  m_bridge->processScript("alertClose");
 }
 m_server->close();
 delete m_bridge;
 createBridge();  // create the webViewBridge
 if(bRunInBrowser)
 {
  ui->groupBox_2->setVisible(false);
  openBrowserWindow();
 }
 else
 {
  ui->groupBox_2->setVisible(true);
  openWebViewPanel();
 }
 config->bRunInBrowser = bRunInBrowser;
 config->saveSettings();

// if(QMessageBox::information(this, tr("Restart required!"), tr("This option requires that Mapper be restarted.\nDo you wish to restart now?"),QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
// {
//  // restart:
//  qApp->quit();
//  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());}
}

void MainWindow::updateSegmentInfoDisplay(SegmentInfo si)
{
 if (si.segmentId() > 0)
 {
  ui->txtStreet->setText( si.streetName());
  ui->txtSegment->setText(si.description());
  ui->txtLocation->setText(si.location());
  ui->txtNewerName->setText(si.newerName());
  ui->sbTracks->setValue(si.tracks());
  m_segmentId = si.segmentId();
  m_points = si.pointList();
  m_nbrPoints = si.pointList().count();
  ui->lblSegment->setText(tr("Segment %1: (points: %2)").arg(m_segmentId).arg(si.pointList().count()));
 }
     return;
}

void MainWindow::On_editSegment_triggered()
{
 SegmentInfo si = sql->getSegmentInfo(m_segmentId);
 SegmentData* sd = sql->getSegmentData(m_routeNbr,m_segmentId,m_currRouteStartDate, m_currRouteEndDate);

 EditSegmentDialog* dlg;
 if(sd)
  dlg = new EditSegmentDialog(sd,this);
 else
  dlg = new EditSegmentDialog(si,this);
 int ret = dlg->exec();
 if(ret == QDialog::Accepted)
 {
  //refreshSegmentCB();
  ui->ssw->refresh();
  refreshRoutes();
 }
}

void MainWindow::onNewSegment_triggered()
{
    EditSegmentDialog* dlg = new EditSegmentDialog(this);
    int ret = dlg->exec();
    if(ret == QDialog::Accepted)
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

bool MainWindow::openBrowserWindow()
{
 // open map display in browser window
    ui->groupBox->setMaximumWidth(16777215);
     QString cwd = QDir::currentPath();
#ifdef Q_OS_WIN
    cwd.replace("/", QDir::separator());
#endif
    fileUrl = QUrl::fromLocalFile(cwd + QDir::separator() + "Resources"  + QDir::separator() + "GoogleMaps2b.htm");

    qInfo() << "open " << fileUrl.toString();
    if(!QDesktopServices::openUrl(fileUrl))
    {
        qCritical() << "open webbrowser failed " << fileUrl.toDisplayString();
        QMessageBox::critical(nullptr, tr("Error"), "open webbrowser failed ");
    }

    setupbridge();

   return true;
}

bool MainWindow::openWebViewPanel()
{
     webView = ui->webView;

     ui->groupBox_2->setVisible(true);
     ui->groupBox->setMaximumWidth(1000);
     QWebEngineSettings *settings = webView->page()->settings();
     settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
     //webView->setObjectName(QStringLiteral("webEngineView"));
     webView->setContextMenuPolicy(Qt::CustomContextMenu);
     webView->setPage(myWebEnginePage = new MyWebEnginePage());
     webView->setMinimumWidth(400);
     connect(myWebEnginePage, &QWebEnginePage::selectClientCertificate,
             [=](QWebEngineClientCertificateSelection selection){
         QList<QSslCertificate> list = selection.certificates();

         qDebug() << list.count() << " certificates in list";
     });
     connect(myWebEnginePage, &QWebEnginePage::certificateError,
             [=](QWebEngineCertificateError error){
        QList<QSslCertificate> chain = error.certificateChain();
         qDebug() << error.description() << tr(" there are %1 certificates in chain. From %2").arg(chain.count()).arg(error.url().toDisplayString());
        foreach (QSslCertificate cert, chain) {
            qDebug() << cert.toText(); // not implemented
            qDebug() << tr("name: %1 expires:%2 isSelf-signed: %3")
                             .arg(cert.subjectDisplayName(),cert.expiryDate().toString(),cert.isSelfSigned()?"yes":"no");
        }
        if(error.type() == QWebEngineCertificateError::CertificateAuthorityInvalid)
        {
            qDebug() << "accepting certificate";
            return error.acceptCertificate();
        }

        return error.defer();
     });
     fileUrl = QUrl("qrc:/GoogleMaps2b.htm");
    webView->setUrl(fileUrl);
    setupbridge();
    webView->page()->setWebChannel(channel);
    return true;
}

bool MainWindow::setupbridge()
{
    // setup the QWebSocketServer
    m_server = new QWebSocketServer(QStringLiteral("WebViewBridge"), QWebSocketServer::NonSecureMode);
    if (!m_server->listen(QHostAddress::LocalHost, 12345))
    {
        QString err = m_server->errorString();
        qCritical() <<tr("Failed to connect to web socket server(%1).").arg(err);
        return false;
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
        qInfo() << "listening on localhost:12345";

    // wrap WebSocket clients in QWebChannelAbstractTransport objects
    m_clientWrapper = new WebSocketClientWrapper (m_server);

    if(!webView)
        connect(m_clientWrapper, SIGNAL(clientClosed()), this, SLOT(onWebSocketClosed()));

    // setup the channel
    channel = new QWebChannel();
    QObject::connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,
                  channel, &QWebChannel::connectTo);
    qInfo() << "registering webViewBridge";
    channel->registerObject("webViewBridge", m_bridge);
    connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,this, [=]{
        if(config->bDisplayRouteOnReload)
        {
            ui->btnDisplayRoute->click();
        }
    });
    connect(m_clientWrapper,  &WebSocketClientWrapper::clientClosed, this, [=]{
        enableControls(false);
    });
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
    enableControls(false);
  if(config->bRunInBrowser)
  {
     //QMessageBox::critical(this, tr("Browser closed"), tr("The browser window has closed"));
   QMessageBox *mbox = new QMessageBox;
   mbox->setWindowTitle(tr("Browser closed"));
   mbox->setText("The browser window has closed");
   mbox->show();
   QTimer::singleShot(2000, mbox, SLOT(hide()));
   channel = nullptr;
  }
  else
  {
      int rslt = QMessageBox::question(this, tr("Connection closed"), tr("The connection to the browser has closed."
                                                                         "Click Yes to reload Map,  Close to exit"), QMessageBox::Yes|QMessageBox::Close);
      if(rslt == QMessageBox::Close)
      {
          close();
          return;
      }
      qInfo() << "reload map initiated!";
      reloadMapAct->trigger();
  }
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
     QDesktopServices::openUrl(QUrl::fromLocalFile(wikiRoot+"/Overlays.html"));
 }
}
void MainWindow::on_usingHelp()
{
 QDir dir(wikiRoot);
 if(dir.exists())
 {
  QDesktopServices::openUrl(QUrl::fromLocalFile(wikiRoot+"/Documentation.htm"));
 }
}

void MainWindow::showGoogleMapFeatures( bool bShow)
{
 if(!bShow)
  //m_bridge->processScript("setOption", "{ styles: styles[\"hide\"] }");
     config->mapId = "99f6ba1d184ea0b6";
 else
  //m_bridge->processScript("setOption", "{ styles: styles[\"default\"] }");
     config->mapId = "DEMO_MAP_ID";
 m_bridge->setMapId(config->mapId);
 reloadMap();
 config->bShowGMFeatures = bShow;
}

MyWebEnginePage::MyWebEnginePage(QObject* parent) : QWebEnginePage(parent){
// connect(this, SIGNAL(QWebEnginePage::loadProgress(int)), this,
//                      SLOT(loadProgress(int)));

 connect(this, &QWebEnginePage::loadProgress, [=](int progress){
  qInfo() << "progress "<< progress << " loading " << requestedUrl().toString();
  //setVisible(true);
  if(progress == 100)
  {
   emit (pageLoaded(requestedUrl()));
   //runJavaScript("onLoad()");
  }

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
    if(webView)
        webView->setEnabled(b);
}

void MainWindow::describeRoute()
{
 RouteData rd = ui->cbRoute->currentData().value<RouteData>();
 //QList<QPair<int, QString>> seqList = sql->getRouteSeq(rd);
 RouteSeq rs = sql->getRouteSeq(rd);
 if(rs.seqList().isEmpty())
 {
  statusBar()->setStyleSheet("color: red");
  statusBar()->showMessage("route must be sequenced", 2000);
 QTimer::singleShot(2000, [=]{
  statusBar()->setStyleSheet("color: black");
 });
  return;
 }
 QString text;
 setCursor(Qt::WaitCursor);
 bool bFirst = true;

 for(QPair<int,QString> p : rs.seqList())
 {
  SegmentInfo si = sql->getSegmentInfo(p.first);
  QString from;
  QString to;
  QString street = si.streetName();
  if(si.description().contains(","))
  {
    to = si.description().mid(si.description().indexOf(" to ")+4);
    int i1 = si.description().indexOf(",");
    int i2 = si.description().indexOf(" to ");
    from = si.description().mid(i1+1, i2-i1);
  }
  else
  street = si.description();
  if(!text.isEmpty())
   text.append(", ");
  text.append("<B>"+si.streetName()+"</B>");
  if(bFirst)
  {
   if(p.second == "F")
   text.append(si.description().mid(si.description().indexOf(",")));
   else
    text.append(to + " to"+ from);
   bFirst=false;
  }
  else
  if(si.description().contains(" to "))
  {
   text.append(" to ");

   if(p.second == "F")
    text.append(to);
   else {
     text.append(to);
   }
  }
 }
 QMessageBox box(QMessageBox::Information,tr("Route description"),
                               tr(" %1-%2 from %3 to %4").arg(rd.alphaRoute(), rd.routeName(),
                                  rd.startDate().toString("yyyy/MM/dd"), rd.endDate().toString("yyyy/MM/dd")));
 box.setInformativeText(text);
 box.exec();

 setCursor(Qt::ArrowCursor);
}

// create string for sorting alpha routes
QString MainWindow::createSortString(QString alphaRoute)
{
 int len = alphaRoute.length();
 QString result;
 int index =0;
 while(index < len)
 {
  if(alphaRoute.at(index).isDigit())
  {
   int digits = countDigits(alphaRoute.mid(index));
   int diff = 3-digits;
   while(diff)
   {
    result.append(QChar('0'));
    diff--;
   }
   while(digits)
   {
    result.append(alphaRoute.at(index++));
    digits--;
   }
   index+=digits;
  }
  else
   result.append(alphaRoute.at(index++));
 }
 return result;
}

int MainWindow::countDigits(QString str)
{
 int len = str.length();
 int ix = 0;
 while(ix < len)
 {
  if(str.at(ix).isDigit())
   ix++;
  else
   break;
 }
 return ix;
}

void MainWindow::processDescriptionChange(QString descr, QString street)
{
    SegmentInfo si = sql->getSegmentInfo(m_segmentId);
    if(descr != si.description() || street != si.streetName())
    {
        si.setStreetName(street);
        si.setDescription(descr);
        si.setLocation(ui->txtLocation->text().trimmed());
        si.setNewerName(ui->txtNewerName->text());
        sql->updateSegment(&si);

        if(!ui->txtNewerName->text().isEmpty())
        {
            QList<StreetInfo*> list = StreetsTableModel::instance()->getStreetName(ui->txtNewerName->text(),
                                                                                    ui->txtLocation->text()  ) ;
            if(list.isEmpty())
            {
                StreetInfo sti;
                sti.street = ui->txtNewerName->text();
                sti.location = ui->txtLocation->text();
                sti.sequence = 0;
                sti.segments.append(si.segmentId());
                sti.updateSegmentInfo(si);
                int streetId =StreetsTableModel::instance()->newStreetDef(&sti);
                if(streetId > 0)
                {
                    sti.street = ui->txtStreet->text();
                    sti.sequence = 1;
                    sti.dateStart = si.startDate();
                    sti.dateEnd = si.endDate();
                    sti.newerName = ui->txtNewerName->text().trimmed();
                    if(StreetsTableModel::instance()->newStreetName(&sti))
                    {
                        si.setStreetId(streetId);
                        sql->updateSegment(&si);
                    }
                }
            }
            else
            {
                foreach (StreetInfo* sti, list) {
                    if(sti->sequence == 0)
                    {
                        if(!sti->segments.contains(si.segmentId()))
                            sti->updateSegmentInfo(si);
                        QStringList names;
                        QList<StreetInfo*>* list2 =StreetsTableModel::instance()->getStreetNames(sti->streetId, &names);
                        if(!names.contains(ui->txtStreet->text().trimmed()))
                        {
                            StreetInfo sti2 = StreetInfo(*sti);
                            sti2.street = ui->txtStreet->text().trimmed();
                            sti2.sequence = 1;
                            sti2.dateStart = si.startDate();
                            sti2.dateEnd = si.endDate();
                            sti2.segments.append(si.segmentId());
                            sti2.updateBounds();
                            if(StreetsTableModel::instance()->newStreetName(&sti2))
                            {
                                si.setStreetId(sti->streetId);
                                si.setNewerName(sti->street);
                                sql->updateSegment(&si);
                            }
                        }
                        else
                        {
                            if(names.contains(ui->txtStreet->text().trimmed()))
                            {
                                StreetInfo* sti2 = list2->at(names.indexOf(ui->txtStreet->text()));
                                sti->segments.append(si.segmentId());
                                if(StreetsTableModel::instance()->updateStreetName(*sti2))
                                {
                                    sti2->segments.append(si.segmentId());
                                    sti2->updateBounds();
                                    si.setStreetId(sti->streetId);
                                    si.setNewerName(sti->street);
                                    sql->updateSegment(&si);
                                }
                            }
                        }
                    }
                    StreetsTableModel::instance()->updateStreetDef(*sti);
                }
            }
        }
    }
}

bool MainWindow::backupDatabases()
{
    QProcess* process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    QFileInfo info("./Resources/dump_databases.bat");
#else
    QFileInfo info("./Resources/dump_databases.sh");
#endif
    if(!info.exists())
        qCritical() << "backup script not found! " << info.absoluteFilePath();
    process->setWorkingDirectory(info.absolutePath());
    qDebug() << "run:" << info.absoluteFilePath();
    connect(process, &QProcess::errorOccurred,this,[=](QProcess::ProcessError error){
        qDebug() << "process error " << error << " " << process->errorString();
        return false;
    });
    process->start(info.absoluteFilePath());
    if(process->waitForStarted())
    {
        process->waitForFinished();
        process->close();
        return true;
    }
    qDebug() << "process error " << process->errorString();

    return false;
}
bool MainWindow::restoreDatabases()
{
    QSqlDatabase db = QSqlDatabase::database();
    if(db.isOpen())
        db.close();
    QProcess* process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    QFileInfo info("./Resources/restore_databases.bat");
#else
    QFileInfo info("./Resources/restore_databases.sh");
#endif
    if(!info.exists())
        qCritical() << "restore script not found! " << info.absoluteFilePath();
    process->setWorkingDirectory(info.absolutePath());
    qDebug() << "run:" << info.absoluteFilePath();
    connect(process, &QProcess::errorOccurred,this,[=](QProcess::ProcessError error){
        qDebug() << "process error " << error << " " << process->errorString();
        return false;
    });
    process->start(info.absoluteFilePath());
    if(process->waitForStarted())
    {
        process->waitForFinished();
        process->close();
        sql->dbOpen();
        return true;
    }
    qDebug() << "process error " << process->errorString();

    return false;

}

