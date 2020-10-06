#ifndef WEBVIEWBRIDGE_H
#define WEBVIEWBRIDGE_H
#include <QFutureWatcher>
#include <QtGui>
#include "data.h"

class webViewBridge : public QObject
{
    Q_OBJECT
public:
    explicit webViewBridge(QObject *parent = 0);
    webViewBridge(LatLng latLng, int zoom, QString maptype, QObject *parent = 0);
    QObject* m_parent = nullptr;
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
    Q_PROPERTY(float lat READ curLat)
    Q_PROPERTY(float lng READ curLon)
    Q_PROPERTY(int zoom READ curZoom)
    Q_PROPERTY(QString maptype READ curMaptype NOTIFY onMapTypeChanged)
    Q_PROPERTY(LatLng latlng MEMBER _latLng WRITE setLatLng NOTIFY latlngChanged)
    void processScript(QString func, QString parms);
    void processScript(QString func);
    void processScript(QString func, QString parms, QString name, QString value);
    void processScript(QString func, QList<QVariant>objArray);

    //QVariant rslt;
    QVariant myRslt;
    QVariantList myList;
    QVariant getRslt();
    //Q_PROPERTY(QVariant rslt READ getRslt)
    static webViewBridge* instance();
    void setLatLng(LatLng latlng);
    bool isResultReceived();

    ~webViewBridge();

signals:
    void executeScript(QString func, QString parms);
    void executeScript2(QString func, QString parms, QString name, QString value);
    void executeScript3(QString func, QVariantList objArray, qint32 count);
    void movePointSignal(qint32 segmentId, qint32 i, double newLat, double newLon);
    void addPointSignal(int pt, double lat, double lon);
    void insertPointSignal(int SegmentId, qint32 i, double newLat, double newLon);
    void segmentSelected(qint32, qint32);
    void outputSetDebug(QString);
    void onMapTypeChanged(QString);
    void latlngChanged(LatLng latLng);
    void segmentStatusSignal(QString txt, QString color);
    void queryOverlaySignal();
    void on_scriptResult(QVariant);
    void on_scriptArrayResult(QVariantList);

public slots:
    void selectSegment(qint32 i, qint32 SegmentId); //19
    void scriptResult(QVariant rtn); //20
    void scriptArrayResult(QVariantList list);
    void setPoint(qint32 i, double lat, double lon);
    void setLat(double lat);
    void setLon(double lon);
    void setDebug(QString str); //25
    void setLen(qint32 len);
    void setCenter(double lat, double lon, int zoom, QString maptype);
    void getGeocoderResults(QString text);
    void addPoint(int pt, double lat, double lon); //29
    void moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i);
    void moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i);
    QString getImagePath(qint32);
    void movePoint(qint32 segmentId, qint32 i, double lat, double lng);
    void insertPoint(int SegmentId, qint32 i, double newLat, double newLon);
    void updateIntersection(qint32 i, double newLat, double newLon);
    void displayZoom(int zoom);
    void showSegmentsAtPoint(double lat, double lon, qint32 segmentId);
    void queryOverlay();
    void opacityChanged(QString name, qint32 opacity);
    void setStation(double lat, double lon, qint32 SegmentId, qint32 i);
    void updateStation( qint32 stationKey, qint32 segmentId);
    void moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng);
    void segmentStatus(QString txt, QString color);
    void getInfoWindowComments(double lat, double lon, int route, QString date, int func);
    void mapInit();

private slots:

private:

    static webViewBridge* _instance;
    float _lat;
    float _lon;
    int _zoom;
    LatLng _latLng;
    QString maptype;
    bool bResultReceived;
};

#endif // WEBVIEWBRIDGE_H
