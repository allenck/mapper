#ifndef WEBVIEWBRIDGE_H
#define WEBVIEWBRIDGE_H
#include <QFutureWatcher>
#include <QtGui>
#include "data.h"
#include "configuration.h"
#include "qwebchannel.h"
#include "websocketclientwrapper.h"

class MainWindow;
class WebViewBridge : public QObject
{
    Q_OBJECT

    // 1. Property Declarations
    Q_PROPERTY(QString name READ curName WRITE setName NOTIFY onNameChanged FINAL)
    Q_PROPERTY(float lat READ curLat WRITE setLat NOTIFY onLatChanged)
    Q_PROPERTY(float lng READ curLon WRITE setLon NOTIFY onLngChanged)
    Q_PROPERTY(int zoom READ curZoom WRITE setZoom NOTIFY onZoomChanged FINAL)
    Q_PROPERTY(QString mapType READ curMapType WRITE setMapType NOTIFY onMapTypeChanged FINAL)
    Q_PROPERTY(QString mapId READ curMapId WRITE setMapId NOTIFY onMapIdChanged)
    Q_PROPERTY(LatLng latLng MEMBER _latLng WRITE setLatLng NOTIFY onLatLngChanged)
    Q_PROPERTY(QString options READ options  NOTIFY  onOptionsChanged )
    Q_PROPERTY(QVariant myRslt  WRITE setMyRslt NOTIFY myRsltChanged FINAL)
    Q_PROPERTY(int opacitycurOpacity READ curOpacity WRITE setCurOpacity NOTIFY curOpacityChanged FINAL)

public:
    // 2. Constructor
    explicit WebViewBridge(QObject *parent = nullptr);
    WebViewBridge(LatLng latLng, int zoom, QString mapType, QString mapId, QString options, QObject *parent = 0);

    // 3. Getter Functions
    QString curName() const { return _name; }
    float curLat() const { return _lat; }
    float curLon() const { return _lng; } // Matches macro's READ curLon
    int curZoom() const { return _zoom; }
    QString curMapType() const { return _mapType; }
    QString curMapId() const { return _mapId; }
    QString options() const { return _options; }
    int curOpacity() const {return _opacity;}
    MainWindow* m_parent = nullptr;
    LatLng curLatLng();
    //bool runInBrowser();
    void processScript(QString func, QString parms);
    void processScript(QString func);
    void processScript(QString func, QString parms, QString name, QString value);
    void processScript(QString func, QList<QVariant>objArray);
    QVariant waitForScript(QString func, QVariantList objArray);

    bool isListening();
    void setMapType(QString &newType)
    {
        if(_mapType != newType)
        {
            _mapType = newType;
            emit onMapTypeChanged();
        }
    }
    //QVariant rslt;
    QVariant _myRslt; // can be a QvariantList
    QVariantList myList;
    QVariant getRslt(){return _myRslt;}
    void setMyRslt(QVariant myRslt) {
        if(_myRslt != myRslt)
        {
            _myRslt = myRslt;
            emit myRsltChanged();
        }
    }
    //Q_PROPERTY(QVariant rslt READ getRslt)
    static WebViewBridge* instance();
    bool isResultReceived();
    LatLng rightClick() {return _rightClickLoc;}
    //void setName(QString);
    ~WebViewBridge();
    int openConnections = 0;


public slots:
    // 4. Setter Functions (accessible to QML)
    void setName(const QString &name);
    void setZoom(int zoom);
    void setMapType(const QString &mapType);
    void setMapId(const QString &mapId);
    void setLatLng(const LatLng &latLng);
    void setCurOpacity(const int opacity);

    void selectSegment(qint32 i, qint32 SegmentId); //19
    void selectSegmentX(qint32 i, qint32 SegmentId, QVariantList array); //19
    void scriptResult(QVariant rtn); //20
    void scriptFunctionResult(QVariant function, QVariant value);
    void scriptArrayResult(QVariantList list);
    void setPoint(qint32 i, double lat, double lon);
    void setLat(double lat);
    void setLon(double lon);
    void setDebug(QString str); //25
    //void setLen(qint32 len);
    //void reportMapType(QString mapType);
    //void setCenter(double lat, double lon, int zoom, QString mapType);
    void getGeocoderResults(QString text);
    void addPoint(int pt, double lat, double lon); //29
    void addPointX(int pt, QVariantList array); //29
    void moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i);
    void moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i);
    QString getImagePath(qint32);
    void clickPoint(double lat, double lng);
    void movePoint(qint32 segmentId, qint32 i, double lat, double lng);
    void movePointX(qint32 segmentId, qint32 i, double lat, double lon, QVariantList array);
    void insertPoint(int SegmentId, qint32 i, double newLat, double newLon);
    void insertPointX(int SegmentId, qint32 i, QVariantList array);
    void updateIntersection(qint32 i, double newLat, double newLon);
    // void displayZoom(int zoom);
    void showSegmentsAtPoint(double lat, double lon, qint32 segmentId);
    void queryOverlay();
    QT_DEPRECATED void opacityChanged(QString name, qint32 opacity);
    void setStation(double lat, double lon, qint32 SegmentId, qint32 i);
    void updateStation( qint32 stationKey, qint32 segmentId);
    void moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng);
    void moveRouteComment(qint32 route, QString date, double lat, double lng, int commentKey, int companyKey);
    void segmentStatus(QString txt, QString color);
    void getInfoWindowComments(double lat, double lon, int route, QString date, int commentKey, int companyKey, int func);
    void mapInit();
    void debug(QString text);
    void cityBounds(double neLat, double neLng, double swLat, double swLng);
    void rightClicked(double lat, double lon);
    void screenshot(QString base64image);
    //void initialized();
    void addPointMode(bool);
    void pinClicked(int, double lat, double lon, QString street, int streetId, QString location, int seq);
    void pinMarkerMoved(double lat, double lon);
    bool setupbridge();
    QString createIcon(QColor color);
    void mapTypeChanged();
    //void zoomChanged(int zoom);

signals:
    // 5. Notification Signals
    void onNameChanged();
    void onLatChanged();
    void onLngChanged();
    void onZoomChanged();
    void onMapTypeChanged();
    void onMapIdChanged();
    void onLatLngChanged();
    void onOptionsChanged();
    void myRsltChanged();
    void curOpacityChanged();

    void executeScript(QString func, QString parms);
    QT_DEPRECATED void executeScript2(QString func, QString parms, QString name, QString value);
    QT_DEPRECATED void executeScript3(QString func, QVariantList objArray, qint32 count);
    void movePointSignal(qint32 segmentId, qint32 i, double newLat, double newLon);
    void movePointSignalX(qint32 segmentId, qint32 i, LatLng pt, QList<LatLng> points);
    void addPointSignal(int pt, double lat, double lon);
    void addPointSignalX(int pt, QList<LatLng> points);
    void insertPointSignal(int SegmentId, qint32 i, double newLat, double newLon);
    void insertPointSignalX(int SegmentId, qint32 i, QList<LatLng> points);
    void segmentSelected(qint32, qint32);
    void segmentSelectedX(qint32, qint32, QList<LatLng>);
    void outputSetDebug(QString);
    //void onRunInBrowserChanged(bool);
    void segmentStatusSignal(QString txt, QString color);
    void queryOverlaySignal();
    void on_scriptResult(QVariant);
    void on_scriptFunctionResult(QVariant, QVariant);
    void on_scriptArrayResult(QVariantList);
    void on_rightClicked(LatLng);
    void on_cityBounds(Bounds bounds);
    void clickLatLng(LatLng);
    void on_pinClicked(int pinId, LatLng latLng, QString street, int streetid, QString location, int seq);
    void on_pinMarkerMoved(LatLng latLng);
    void on_connection_closed();

private slots:

private:
    // 6. Private Member Variables (Backing Fields)
    QString _name;
    float _lat;
    float _lng;
    int _zoom;
    QString _mapType;
    QString _mapId;
    LatLng _latLng; // Macro links directly to this via MEMBER
    QString _options;
    int _opacity;
    static WebViewBridge* _instance;
    bool bResultReceived;
    Configuration* config;
    LatLng _rightClickLoc;
    QList<LatLng> buildPoints(QVariantList array);

    QWebChannel* channel = nullptr;
    QWebSocketServer* m_server=nullptr;
    WebSocketClientWrapper* m_clientWrapper= nullptr;
    QWebSocketServer* m_OverlayServer=nullptr;
    WebSocketClientWrapper* m_overlayWrapper= nullptr;

    friend class MainWindow;
    friend class Configuration;
};

#endif // WEBVIEWBRIDGE_H
