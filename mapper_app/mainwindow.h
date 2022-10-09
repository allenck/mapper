#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QObject>
#include <QtGui>
#include <QMainWindow>
#include "mapview.h"
#include "ui_mainwindow.h"
#ifndef USE_WEBENGINE
#include <QWebView>
#else
#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
//#include <WebSocketClientWrapper>
#endif
#include "data.h"
#include "routeview.h"
#include <QSettings>
#include "ccombobox.h"
#include <QMenu>
#include "routeviewsortproxymodel.h"
#include "routeviewtablemodel.h"
#include "configuration.h"
//#include "sql.h"
#include "routedlg.h"
#include "segmentview.h"
#include "segmentviewsortproxymodel.h"
#include "segmentviewtablemodel.h"
#include "otherrouteview.h"
#include "filedownloader.h"
#include "splitroute.h"
#include "editstation.h"
#include "stationview.h"
#include "companyview.h"
#include "tractiontypeview.h"
#include "dupsegmentview.h"
#include "routecommentsdlg.h"
#include "querydialog.h"
#include <QToolTip>
#include "splitsegmentdlg.h"

class SQL;
class QWidgetAction;
class RouteViewTableModel;
class RouteViewSortProxyModel;

#ifndef USE_WEBENGINE
class myWebPage : public QWebPage
{
 virtual QString userAgentForUrl(const QUrl& url) const {
 Q_UNUSED(url)
 //return "Chrome/1.0";
 //QString userAgent = "Mozilla/5.0 (%Platform%%Security%%Subplatform%) AppleWebKit/%WebKitVersion% (KHTML, like Gecko) %AppVersion Safari/%WebKitVersion%";
 QString userAgent = "Mozilla/5.0 (X11)";
 //qDebug() << userAgent;
 return userAgent;
 }
};
#else
class WebSocketClientWrapper;
class MyWebEnginePage : public QWebEnginePage
{
 Q_OBJECT
public:
 MyWebEnginePage(QObject* parent = 0) : QWebEnginePage(parent){}
 bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool )
 {
  if(type == NavigationTypeLinkClicked)
  {
   QDesktopServices::openUrl(url);
   return false;
  }
  return true;
 }
};

#endif
class SystemConsoleAction;
class WebViewAction;
class QAction;
class QMenu;
class webViewBridge;
class SQL;
class QLabel;
class QSortFilterProxyModel;
class QItemSelectionModel;
class RouteView;
class SegmentView;
class OtherRouteView;
class StationView;
class CompanyView;
class TractionTypeView;
class DupSegmentView;

namespace Ui {
class MainWindow;
}

//class webviewer;
class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    mainWindow(int argc, char * argv[],QWidget *parent = 0);
    Ui::MainWindow *ui;
    QSqlDatabase db;
    SQL* sql;
    void setModel(QAbstractItemModel *model);
    //MapView *centralWidget;
    webViewBridge *m_bridge;
    RouteView *routeView;
    SegmentView *segmentView;
    OtherRouteView *otherRouteView;
    DupSegmentView *dupSegmentView;
    StationView * stationView;
    CompanyView * companyView;
    TractionTypeView * tractionTypeView;
    void getInfoWindowComments(double lat, double lon, int route, QString date, int func);
    void SetPoint(qint32 i, double lat, double lon);
    void getArray();
//    void setLat(double lat);
//    void setLon(double lon);
    void setLen(qint32 len);
    QString ProcessScript(QString func, QString params);
    QString getRouteMarkerImagePath(QString route, bool isStart);

    QList<RouteData> routeList;
    QAction *saveChangesAct;

    double m_latitude, m_longitude;
    qint32 m_zoom;
    QString m_maptype;
    QList<SegmentInfo> segmentInfoList;
    qint32 m_routeNbr;
    QString m_alphaRoute;
    QString m_currRouteStartDate, m_currRouteEndDate, m_routeName;
    QString m_resourcePath;
    QString m_segmentStatus, m_segmentColor;

    RouteDlg *routeDlg;
    RouteCommentsDlg *routeCommentsDlg;
    QueryDialog* queryDlg;

    //QMenu cbRouteContextMenu;
    //QMenu menu;
    RouteViewSortProxyModel *proxyModel;
    //segmentViewSortProxyModel *segmentProxyModel; // these need to be in thier own class, not in main window
    void updateIntersection(qint32 i, double newLat, double newLon);
    QLabel* zoomIndicator;
    QLabel* geocoderRslt;
    void getZoom(int zoom);
    void getGeocoderResults(QString array);
    void opacityChanged(QString name, qint32 opacity);
    void moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i);
    void moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i);
    void setStation(double lat, double lon, qint32 segmentId, qint32 ptIndex);
    Q_DECL_DEPRECATED Configuration* getConfiguration();
    void updateStation(qint32 stationKey, qint32 segmentId);
    void moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lon);
    void moveRouteComment(int route, QString date, double latitude, double longitude, int companyKey);

    RouteViewTableModel *sourceModel;
    void displaySegment(qint32 segmentId, QString segmentName, QString oneWay, QString color, QString trackUsage, bool bClearFirst);
    static QString pwd;
    static QString pgmDir;
#ifndef USE_WEBENGINE
    QWebView* webView;
#else
    QWebEngineView* webView;
    QWebChannel* channel;
    QWebSocketServer* m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebSocketServer* m_OverlayServer;
    WebSocketClientWrapper* m_overlayWrapper;
#endif

public slots:
    void copyRouteInfo_Click();
    void setDebug(QString str);
    void refreshRoutes();
    void mapInit();
    void refreshCompanies();
    void segmentStatus(QString str, QString color);
    void saveChanges();

private slots:
    void about();
    void quit();
    void cbRoute_customContextMenu( const QPoint& );
    void txtSegment_customContextMenu(const QPoint&);
    void tab1CustomContextMenu(const QPoint &);
    void btnDisplayRouteClicked();
    void btnClearClicked();
    void onCbRouteIndexChanged(int);
    void btnFirstClicked();
    void btnNextClicked();
    void btnPrevClicked();
    void btnLastClicked();
    void btnDeletePtClicked();
    //void onResize();
    void cbSegmentsSelectedValueChanged(int row);
    void cbSegmentsTextChanged(QString text);
    void cbRoutesTextChanged(QString text);
    void cbSegments_Leave();
    void cbRoutes_Leave();
    //void tblRouteView_selectionChanged(QTableWidgetItem * item);
    //void cbRoutes_contextMenu_requested(const QPoint& );
    //void updateRouteView();
    void txtSegment_TextChanged(QString text);
    void txtStreetName_TextChanged(QString text);
    void txtStreetName_Leave();
    void txtSegment_Leave();
    void newCity(int ix);
    void newOverlay(int ix);
    void splitRoute_Click();
    void combineRoutes();
    void renameRoute_Click();
    void modifyRouteDate();
    void addSegment();
    void deleteRoute();
    void updateRoute();
    void updateTerminals();
    void segmentChanged(qint32 changedSegment, qint32 newSegment);
    void segmentDlg_routeChanged(RouteChangedEventArgs);
    void RouteChanged(RouteChangedEventArgs args);
    void btnDeleteSegment_Click();
    //void btnDisplay_Click();
    //void chkShowWindow_CheckedChanged();
    void btnSplit_Click();
//    void selectSegment();
    void AddRoute();
    void addModeToggled(bool isChecked);
    void reloadMap();
    void displayStationMarkersToggeled(bool bChecked);
    void displayTerminalMarkersToggeled(bool bChecked);
    void displayRouteCommentsToggled(bool bChecked);
    void chkShowOverlayChanged(bool bChecked);
//    void chkOneWay_Leave(bool bChecked);
    void sbRouteTriggered(int sliderAction);
    void txtRouteNbrLeave();
    void On_saveImage_clicked();
    void on_createKmlFile_triggered();
    void fillOverlayMenu();
    void queryOverlay();
    void On_displayRoute(RouteData);

    // webViewBridge
    void addPoint(int pt, double lat, double lon);
    void movePoint(qint32 segmentId, qint32 i, double newLat, double newLon);
    void insertPoint(int SegmentId, qint32 i, double newLat, double newLon);
    void segmentSelected(qint32 pt, qint32 SegmentId);

    void addJSObject();
    void NotYetInplemented();
    void loadAcksoftData();
    void loadMbtilesData();
    void linkActivated();
    void pageBack();
    void exportDb();
    void editConnections();
    void cbCompanySelectionChanged(int sel);
    void locateStreet();
    void findDupSegments();
    void on_webView_statusBarMessage(QString text);
    void updateRouteComment();
    void geocoderRequestToggled(bool bChecked);
    void cbSortSelectionChanged(int sel);
    void rerouteRoute();
    void newSqliteDbAct_triggered();
    void QueryDialogAct_triggered();
    void exportRoute();
    void selectSegment(int );
    void On_editCityInfo();
//#ifndef USE_WEBENGINE
    void on_linkClicked(QUrl);
//#endif
    void on_addGeoreferenced(bool);
    void on_overlayHelp();
    void on_usingHelp();
    void processTileMapResource();

private:
    //Webviewer *centralWidget;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *cityMenu;
    QMenu *connectMenu;
    QMenu *toolsMenu;
    QMenu* optionsMenu;
    QMenu *overlayMenu;
    QMenu cbRouteMenu;
    QMenu tab1Menu;
    QMenu *sortMenu;

    QAction *aboutAct;
    QAction *quitAct;
    QAction *displayAct;
    QAction *copyRouteAct;
    QAction *renameRouteAct;
    QAction *splitRouteAct;
    QAction *modifyRouteDateAct;
    QAction *addSegmentAct;
    QAction *deleteRouteAct;
    QAction *updateRouteAct;
    QAction *updateTerminalsAct;
    //QAction *displaySegmentAct;
    QAction* addSegmentToRouteAct;
    QAction *deleteSegmentAct;
    QAction *findDupSegmentsAct;
    QAction *findDormantSegmentsAct;
    QAction *selectSegmentAct;
    QAction* editSegmentAct;
    QAction *addRouteAct;
    QAction *addPointModeAct;
    QAction *reloadMapAct;
    QAction *displayStationMarkersAct;
    QAction *backAction;
    QAction *displayTerminalMarkersAct;
    QAction *displayRouteCommentsAct;
    QAction* createKmlAct;
    QAction *exportDbAct;
    QAction *editConnectionsAct;
    QAction* editCityAct;
    QAction *locateStreetAct;
    QAction *combineRoutesAct;
    QAction *refreshRoutesAct;
    QAction *routeCommentsAct;
    QAction *geocoderRequestAct;
    QAction *rerouteAct;
    QAction *newSqliteDbAct;
    QAction* queryDialogAct;
    QWidgetAction *sortTypeAct;
    QComboBox * cbSort;
    QAction* exportRouteAct;
    QAction* showDebugMessages;
    QAction* runInBrowserAct;
    QAction* addGeoreferencedOverlayAct;
    QAction* overlayHelp;
    QAction* usingMapper;
    QAction* splitSegmentAct;
    QList<QAction*> cityActions;
    QSignalMapper *signalMapper;
    QActionGroup  *actionGroup;

    QList<QAction*> overlayActions;
    QSignalMapper *overlaySignalMapper;
    QActionGroup  *overlayActionGroup;
    QString  currentOverlay;
    QList<tractionTypeInfo> tractionTypeList;
    QList<CompanyData*> companyList;

    TerminalInfo m_terminalInfo;
    RouteData _rd;
    bool m_bAddMode;
    bool b_cbSegments_TextChanged = false;
    bool b_cbRoutes_TextChanged = false;
    bool bStreetChanged = false;
    bool bSegmentChanged = false;
    bool bRefreshingSegments = false;
    bool bDisplayStationMarkers = false;
    bool bDisplayTerminalMarkers = false;
    bool bDisplayRouteComments = false;
    bool bCbRouteRefreshing = false;
    bool bNoDisplay = false;
    bool bDisplayWebDebug = false;
    QStringList overlays;

    void createActions();
    void createMenus();
    void lookupStreetName(SegmentInfo sd);
    QString getColor(qint32 tractionType);
    void closeEvent(QCloseEvent *event);
    FileDownloader *m_dataCtrl;
    FileDownloader *m_overlays;
    FileDownloader *m_tilemapresource;
    Configuration* config;

    qint32 m_SegmentId;
    QList<SegmentInfo> cbSegmentInfoList;  // list of segmentInfo items in cbSegments
    qint32 m_currPoint, m_nbrPoints;
    QList<LatLng> m_points;
    //segmentInfo si;
    SystemConsoleAction* systemConsoleAction;
    WebViewAction* webViewAction;
    void updateSegmentInfoDisplay(SegmentInfo si);
    void createBridge();
    bool openWebWindow();
    void loadOverlay(Overlay* ov);
    QString path, wikiRoot;
    void loadData(QString data, QString source);

private slots:
    void createCityMenu();
    void sbTracks_valueChanged(int);
    void on_showDebugMessages(bool);
    void chkOneWay_toggled(bool);
    void On_editSegment_triggered();
    void on_runInBrowser(bool);
    void onWebSocketClosed();
    void loadOverlayData();
    void refreshSegmentCB();

protected:
    //void resizeEvent(QResizeEvent *event);
signals:
    //void sendRows(int, int);
    void newCitySelected();
};
class Menu : public QMenu
{
    Q_OBJECT
public:
    Menu(){}
    Menu(QString str) : QMenu(str) {}
    bool event (QEvent * e)
    {
        const QHelpEvent *helpEvent = static_cast <QHelpEvent *>(e);
         if (helpEvent->type() == QEvent::ToolTip && activeAction() != 0)
         {
              QToolTip::showText(helpEvent->globalPos(), activeAction()->toolTip());
         } else
         {
              QToolTip::hideText();
         }
         return QMenu::event(e);
    }
};
#endif // MAINWINDOW_H
