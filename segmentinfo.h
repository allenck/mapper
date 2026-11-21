#ifndef SEGMENTINFO_H
#define SEGMENTINFO_H

#include <QObject>
#include "latlng.h"
#include "bearing.h"
#include "bounds.h"
//#include "segmentdata.h"
#include "routetype.h"
//enum RouteType;
class SegmentData;
class SegmentInfo
{
public:
    explicit SegmentInfo();
    enum CHANGETYPE {
        ADD,
        DELETE,
        MODIFY
    };
    QString _description;
    bool _formatOK = false;
    qint32 _segmentId = -1;
    qint32 _points =0;
    double _length = 0;
    QDate _dateBegin = QDate::fromString("1880-01-01", "yyyy-MM-dd");
    QDate _dateDoubled;
    QDate _dateEnd= QDate::fromString("2050-12-31", "yyyy-MM-dd");
    Bearing _bearing;
    double _startLat, _startLon, _endLat, _endLon;
    QString _direction = " ";
    Bearing _bearingStart;
    Bearing _bearingEnd;
    QString _whichEnd;
    RouteType _routeType;
    int _tractionType;
    QString _streetName;
    QList<LatLng> _pointList;
    Bounds _bounds;
    qint8 _tracks;
    bool _bNeedsUpdate;
    int _streetId =-1;
    int _streetSeq = 0;
    int _rowid = -1;
    //int routeCount;
    //QString trackUsage;
    int _next = -1; // needed for DupSegmentView
    // int _prev = -1;
    QString _trackType = " ";
    QString _location;
    QString _newerStreetName;
    SegmentInfo(const SegmentInfo& );
    SegmentInfo(const SegmentData&);
    void setPoints(QString sPoints);
    void setPoints(QList<LatLng> list) {_pointList = list;}
    QString pointsString();
    void addPoint(LatLng pt, bool updateDb=true);
    void insertPoint(int ptNum, LatLng pt);
    QList<LatLng> movePoint(int ptNum, LatLng pt);
    void deletePoint(int ptnum);
    void checkTracks();
    void clearList() {_pointList.clear();}
    void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
    QString toString() const;
    qint32 segmentId() const {return _segmentId;}
    void setSegmentId(int segmentId) {_segmentId = segmentId;}
    QDate startDate() const {return _dateBegin;}
    QDate endDate()const {return _dateEnd;}
    double startLat() {return _startLat;}
    double startLon() {return _startLon;}
    double endLat() {return _endLat;}
    double endLon() {return _endLon;}
    QString streetName() const {return _streetName;}
    QString newerName() {return _newerStreetName;}
    QString description() const {return _description;}
    RouteType routeType() {return _routeType;}
    int tracks() {return _tracks;}
    QList<LatLng> pointList() {return _pointList;}
    Bearing bearing()
    {
        _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
        _direction = _bearing.strDirection();
        _length = _bearing.Distance();
        return _bearing;
    }
    double length() { return _length;}
    void setStartLat(double lat) {_startLat = lat;}
    void setStartLon(double lon) {_startLon = lon;}
    void setEndLat(double lat) {_endLat = lat;}
    void setEndLon(double lon) {_endLon = lon;}
    QString getStreetName() {return _streetName;}
    QString whichEnd() {return _whichEnd;}
    void setEndDate(QDate date) {_dateEnd = date;}
    void setStartDate(QDate date) {_dateBegin = date;}
    void setRouteType(RouteType routeType) {_routeType = routeType;}
    void setDescription(QString description) {_description = description;}
    void setTracks(int tracks) {_tracks = tracks;}
    QString direction();
    void setStreetName(QString streetName) {_streetName = streetName;}
    void setNewerName(QString newerName){_newerStreetName = newerName;}
    bool needsUpdate() {return _bNeedsUpdate;}
    void setNeedsUpdate(bool b) {_bNeedsUpdate = b;}
    LatLng getStartLatLng() {return LatLng(_startLat, _startLon);}
    LatLng getEndLatLng() {return LatLng(_endLat, _endLon);}
    void setBounds(Bounds b){_bounds = b;}
    Bearing bearingStart() {
        if(_pointList.count() > 1)
            _bearingStart = Bearing(_startLat, _startLon, _pointList.at(1).lat(), _pointList.at(1).lon());
        return _bearingStart;
    }
    Bearing bearingEnd() {
        if(_pointList.count() > 1)
            _bearingEnd =  Bearing(_pointList.at(_points-2).lat(), _pointList.at(_points-2).lon(), _endLat, _endLon);
        return _bearingEnd;
    }
    static QStringList ROUTETYPES;

    void setWhichEnd(QString whichEnd) {_whichEnd = whichEnd;}
    Bounds bounds() {
        LatLng _sw = LatLng(_startLat < _endLat ? _startLat : _endLat, _startLon < _endLon ? _startLon : _endLon );
        LatLng _ne = LatLng(_startLat > _endLat ? _startLat : _endLat, _startLon > _endLon ? _startLon : _endLon );
        _bounds = Bounds(_sw, _ne);
        return _bounds;
    }
    QString location() {return _location;}
    void setLocation(QString loc) {_location = loc;}
    void setNext(int next){_next = next;}
    int next() const {return _next;}
    QString reverseDescription();
    QDate doubleDate() {return _dateDoubled;}
    void setDoubleDate(QDate date){_dateDoubled = date;}
    void setFormatOK(bool b){_formatOK = b;}
    bool formatOK(){return _formatOK;}
    int streetId(){return _streetId;}
    void setStreetId(int streetId){_streetId = streetId;}
    int streetSeq(){return _streetSeq;}
    void setStreetSeq(int streetSeq){_streetSeq = streetSeq;}

    bool operator==(const SegmentInfo& other) const{
        return(other._segmentId == _segmentId);

    }
private:
    QString _dtFormat;

    friend class SegmentData;
    friend class SQL;
    friend class StreetsTableModel;
};
#endif // SEGMENTINFO_H
