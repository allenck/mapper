#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QObject>
#include <QtGui>
#include <QMainWindow>
//#include "mapview.h"
#include "ui_mainwindow.h"
#ifndef USE_WEBENGINE
#include <QWebView>
#else
#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include <QWebEnginePage>
#endif
#include "data.h"
#include "routeview.h"
#include <QSettings>
//#include "ccombobox.h"
#include <QMenu>
#include "routeviewsortproxymodel.h"
#include "routeviewtablemodel.h"
//#include "configuration.h"
//#include "sql.h"
#include "routedlg.h"
//#include "segmentviewsortproxymodel.h"
//#include "segmentviewtablemodel.h"
#include "filedownloader.h"
//#include "splitroute.h"
//#include "editstation.h"
#include "routecommentsdlg.h"
#include "querydialog.h"
#include <QToolTip>
#include <QPair>


class RouteView;
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
 MyWebEnginePage(QObject* parent = 0);
 bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool ) override
 {
  if(type == NavigationTypeLinkClicked)
  {
   QDesktopServices::openUrl(url);
   return false;
  }
  return true;
 }
 public slots:
 void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel /*level*/, const QString &message, int lineNumber,
                          const QString &sourceID)override
 {
  qDebug() << "javaScriptConsoleMessage:" << message << " at"<<lineNumber<<" source:"<<sourceID;

 }
 void loadProgress(int progress)
 {
  qDebug() << "progress "<< progress;
 }
};

#endif
class SystemConsoleAction;
class WebViewAction;
class QAction;
class QMenu;
class WebViewBridge;
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
class ExportDlg;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char * argv[],QWidget *parent = 0);
    Ui::MainWindow *ui;
    QSqlDatabase db;
    SQL* sql;
    void setModel(QAbstractItemModel *model);
    //MapView *centralWidget;
    WebViewBridge *m_bridge;
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
    QAction *discardChangesAct;

    double m_latitude, m_longitude;
    qint32 m_zoom;
    QString m_maptype;
    //QList<SegmentInfo> segmentInfoList;
    QList<SegmentData> segmentDataList;

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
    void displaySegment(qint32 segmentId, QString segmentName, QString color, QString trackUsage, bool bClearFirst);
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
    int selectedSegment() {return m_SegmentId;}
    QDir htmlDir;
    static MainWindow* _instance;
    static MainWindow* instance();

public slots:
    void copyRouteInfo_Click();
    void setDebug(QString str);
    void refreshRoutes();
    void mapInit();
    void refreshCompanies();
    void segmentStatus(QString str, QString color);
    void saveChanges();
    void On_displayRoute(RouteData);
    void addModeToggled(bool isChecked);
    void showGoogleMapFeatures(bool);

private slots:
    void about();
    void quit();
    void cbRoute_customContextMenu( const QPoint& );
    void txtSegment_customContextMenu(const QPoint&);
    void webView_customContextMenu(const QPoint&);

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
    void cbSegmentsSelectedValueChanged(SegmentInfo sd);
    void cbSegmentsTextChanged(QString text);
    void cbRoutesTextChanged(QString text);
//    void cbSegments_Leave();
    void cbRoutes_Leave();
    //void tblRouteView_selectionChanged(QTableWidgetItem * item);
    //void cbRoutes_contextMenu_requested(const QPoint& );
    //void updateRouteView();
    void txtSegment_TextChanged(QString text);
    void txtStreetName_TextChanged(QString text);
    void txtStreetName_Leave();
    void txtSegment_Leave();
    void newCity(QAction* act );
    void newOverlay(QAction* act);
    void splitRoute_Click();
    void combineRoutes();
    void renameRoute_Click();
    void modifyRouteDate();
    void addSegment();
    void deleteRoute();
    void updateRoute(SegmentData* sd = nullptr);
    void updateTerminals();
    void segmentChanged(qint32 changedSegment, qint32 newSegment);
    void segmentDlg_routeChanged(RouteChangedEventArgs);
    void RouteChanged(RouteChangedEventArgs args);
    void btnDeleteSegment_Click();
    //void btnDisplay_Click();
    //void chkShowWindow_CheckedChanged();
    void btnSplit_Clicked();
//    void selectSegment();
    void addRoute();
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

    // webViewBridge
    void addPoint(int pt, double lat, double lon);
    void movePoint(qint32 segmentId, qint32 i, double newLat, double newLon);
    void insertPoint(int SegmentId, qint32 i, double newLat, double newLon);
    void segmentSelected(qint32 pt, qint32 SegmentId);

    void addJSObject();
    void NotYetInplemented();
    void loadAcksoftData(QString err);
    void loadMbtilesData();
    void linkActivated();
    void pageBack();
    void exportDb();
    void editConnections();
    void cbCompanySelectionChanged(int sel);
    //void locateStreet();
    void findDupSegments();
    void on_webView_statusBarMessage(QString text);
    void on_selectSegment(int segmentId);
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
    //QMenu *connectMenu;
    QMenu *toolsMenu;
    QMenu* optionsMenu;
    QMenu *overlayMenu = nullptr;
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
    QAction *modifyRouteTractionTypeAct;
    QAction *addSegmentAct;
    QAction *deleteRouteAct;
    QAction *updateRouteAct;
    QAction *replaceSegments;
    QAction *updateTerminalsAct;
    //QAction *displaySegmentAct;
    QAction* addSegmentToRouteAct;
    QAction* addSegmentToNewRouteAct;
    QAction *deleteSegmentAct;
    QAction *findDupSegmentsAct;
    QAction *queryRouteUsageAct;
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
    QAction* manageOverlaysAct;
    //QAction *locateStreetAct;
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
    QAction* browseCommentsAct;
    QAction* overlayHelp;
    QAction* usingMapper;
    QAction* splitSegmentAct;
    QAction* setInspectedPageAct = nullptr;
    QList<QAction*> cityActions;
    //QActionGroup  *actionGroup;
    QAction* exportOverlaysAct;
    QAction* setCityBoundsAct;
    QAction* setLoggingAct;
    QList<QAction*> overlayActions;
    QAction* newCityAct;
    QAction* removeCityAct;
    QAction* changeRouteNumberAct;
    QAction* checkSegmentsAct;
    QAction* saveSettingsAct;
    QAction* showGoogleMapFeaturesAct;

    //QSignalMapper *overlaySignalMapper;
    QActionGroup  *overlayActionGroup;
    QString  currentOverlay;
    QMap<int,TractionTypeInfo> tractionTypeList;
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
    bool bCbStreets_text_changed = false;
    QString saveStreet = "";
    bool bCbStreetsRefreshing = false;
    bool bFirstSegmentDisplayed = false;

    void createActions();
    void createMenus();
    void lookupStreetName(SegmentInfo sd);
    QString getColor(qint32 tractionType);
    void closeEvent(QCloseEvent *event);
    FileDownloader *m_dataCtrl;
    FileDownloader *m_overlays;
    FileDownloader *m_tilemapresource;
    Configuration* config;
    MyWebEnginePage* myWebEnginePage = nullptr;
    qint32 m_SegmentId;
    //QT_DEPRECATED QList<SegmentInfo> cbSegmentInfoList;  // list of segmentInfo items in cbSegments
    QMap<int, SegmentData> cbSegmentDataList;  // list of segmentInfo items in cbSegments

    qint32 m_currPoint, m_nbrPoints;
    int m_companyKey;
    QList<LatLng> m_points;
    //segmentInfo si;
    SystemConsoleAction* systemConsoleAction;
    WebViewAction* webViewAction;
    void updateSegmentInfoDisplay(SegmentInfo sd);
    void createBridge();
    bool openWebWindow();
    void loadOverlay(Overlay* ov);
    QString wikiRoot;
    void loadData(QString data, QString source);
//    void refreshStreetsCb();
    bool copyAndUpdate(QString inFile, QString outDir, QString apiKey);
    bool updateTarget(QString inDir, QString outDir);

    QString tempDir;
//    QStringList keyTokens;
    QUrl fileUrl;
    bool verifyAPIKey(QString path, QString apiKey);
    ExportDlg* form = nullptr;
    void enableControls( bool b);

private slots:
    void createCityMenu();
    void sbTracks_valueChanged(int);
    void on_showDebugMessages(bool);
    void chkOneWay_toggled(bool);
    void On_editSegment_triggered();
    void on_runInBrowser(bool);
    void onWebSocketClosed();
    void loadOverlayData();
//    void refreshSegmentCB();
    void modifyRouteTractionType();
    void on_updateRoute();
protected:
    //void resizeEvent(QResizeEvent *event);
signals:
    //void sendRows(int, int);
    void newCitySelected();
    friend class RouteView;
    friend class WebViewBridge;
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
