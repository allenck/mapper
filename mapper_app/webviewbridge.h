#ifndef WEBVIEWBRIDGE_H
#define WEBVIEWBRIDGE_H
#include <QFutureWatcher>
#include <QtGui>
#include "data.h"
#include "configuration.h"

class MainWindow;
class WebViewBridge : public QObject
{
    Q_OBJECT
public:
    WebViewBridge(MainWindow *parent = 0);
    WebViewBridge(LatLng latLng, int zoom, QString maptype, QString mapId, MainWindow *parent = 0);
    MainWindow* m_parent = nullptr;
//    int browseWindowWidth;
//    int browseWindowHeight;
//    int curBrowseWindowWidth();
//    int curBrowseWindowHeight();
//    Q_PROPERTY(int browseWindowWidth READ curBrowseWindowWidth NOTIFY isCurBrowseWindowWidthChanged)
//    Q_PROPERTY(int browseWindowHeight READ curBrowseWindowHeight )
    float curLat() const;
    float curLon();
    LatLng curLatLng();
    int curZoom();
    QString curMaptype();
    QString curMapId();
    Q_PROPERTY(float lat READ curLat NOTIFY onLatChanged)
    Q_PROPERTY(float lng READ curLon NOTIFY onLngChanged)
    Q_PROPERTY(int zoom READ curZoom NOTIFY onZoomChanged)
    Q_PROPERTY(QString maptype READ curMaptype NOTIFY onMapTypeChanged)
    Q_PROPERTY(QString mapId READ curMapId WRITE setMapId NOTIFY onMapIdChanged)
    Q_PROPERTY(LatLng latlng MEMBER _latLng WRITE setLatLng NOTIFY latlngChanged)
    void processScript(QString func, QString parms);
    void processScript(QString func);
    QT_DEPRECATED void processScript(QString func, QString parms, QString name, QString value);
    void processScript(QString func, QList<QVariant>objArray);

    //QVariant rslt;
    QVariant myRslt;
    QVariantList myList;
    QVariant getRslt();
    //Q_PROPERTY(QVariant rslt READ getRslt)
    static WebViewBridge* instance();
    void setLatLng(LatLng latlng);
    bool isResultReceived();
    LatLng rightClick() {return _rightClickLoc;}
    void setMapId(QString);

    ~WebViewBridge();

signals:
    void executeScript(QString func, QString parms);
    void executeScript2(QString func, QString parms, QString name, QString value);
    void executeScript3(QString func, QVariantList objArray, qint32 count);
    void movePointSignal(qint32 segmentId, qint32 i, double newLat, double newLon);
    void movePointSignalX(qint32 segmentId, qint32 i, LatLng pt, QList<LatLng> points);
    void addPointSignal(int pt, double lat, double lon);
    void addPointSignalX(int pt, QList<LatLng> points);
    void insertPointSignal(int SegmentId, qint32 i, double newLat, double newLon);
    void insertPointSignalX(int SegmentId, qint32 i, QList<LatLng> points);
    void segmentSelected(qint32, qint32);
    void segmentSelectedX(qint32, qint32, QList<LatLng>);
    void outputSetDebug(QString);
    void onLatChanged(QString);
    void onLngChanged(QString);
    void onZoomChanged(QString);
    void onMapTypeChanged(QString);
    void latlngChanged(LatLng latLng);
    void segmentStatusSignal(QString txt, QString color);
    void queryOverlaySignal();
    void on_scriptResult(QVariant);
    void on_scriptFunctionResult(QVariant, QVariant);
    void on_scriptArrayResult(QVariantList);
    void on_rightClicked(LatLng);
    void on_cityBounds(Bounds bounds);
    void onMapIdChanged(QString mapId);
    void clickLatLng(LatLng);
    void on_pinClicked(int pinId, LatLng latLng, QString street, int streetid, QString location, int seq);
    void on_pinMarkerMoved(LatLng latLng);

public slots:
    void selectSegment(qint32 i, qint32 SegmentId); //19
    void selectSegmentX(qint32 i, qint32 SegmentId, QVariantList array); //19
    void scriptResult(QVariant rtn); //20
    void scriptFunctionResult(QVariant function, QVariant value);
    void scriptArrayResult(QVariantList list);
    void setPoint(qint32 i, double lat, double lon);
    void setLat(double lat);
    void setLon(double lon);
    void setDebug(QString str); //25
    void setLen(qint32 len);
    void setCenter(double lat, double lon, int zoom, QString maptype);
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
    void displayZoom(int zoom);
    void showSegmentsAtPoint(double lat, double lon, qint32 segmentId);
    void queryOverlay();
    void opacityChanged(QString name, qint32 opacity);
    void setStation(double lat, double lon, qint32 SegmentId, qint32 i);
    void updateStation( qint32 stationKey, qint32 segmentId);
    void moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng);
    void moveRouteComment(qint32 route, QString date, double lat, double lng, int companyKey);
    void segmentStatus(QString txt, QString color);
    void getInfoWindowComments(double lat, double lon, int route, QString date, int func);
    void mapInit();
    void debug(QString text);
    void cityBounds(double neLat, double neLng, double swLat, double swLng);
    void rightClicked(double lat, double lon);
    void screenshot(QString base64image);
    void initialized();
    void addPointMode(bool);
    void pinClicked(int, double lat, double lon, QString street, int streetId, QString location, int seq);
    void pinMarkerMoved(double lat, double lon);

private slots:

private:

    static WebViewBridge* _instance;
    float _lat;
    float _lon;
    int _zoom;
    LatLng _latLng;
    QString maptype;
    QString mapId;
    bool bResultReceived;
    Configuration* config;
    LatLng _rightClickLoc;
    QList<LatLng> buildPoints(QVariantList array);
};

#endif // WEBVIEWBRIDGE_H
