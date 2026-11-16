#ifndef SEGMENTDATA_H
#define SEGMENTDATA_H

#include <QObject>
#include "segmentinfo.h"
//#include "routedata.h"

class RouteData;
class SegmentData : public SegmentInfo
{
public:
    explicit SegmentData();
    SegmentData(int route, QString name, int segmentId, QDate startDate, QDate endDate);
    ~SegmentData() {}
    SegmentData(const SegmentData&);
    SegmentData(const SegmentInfo&);
    SegmentData(const RouteData&);

    void addPoint(LatLng pt);
    void insertPoint(int ptNum, LatLng pt);
    void movePoint(int ptNum, LatLng pt);
    void deletePoint(int ptNum);
    //void setPoints(QList<LatLng>points);
    //void setPoints(QString);
    //QString toString();
    QString toString2();
    static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
    //QString pointsString();
    int segmentId() const {return _segmentId;}
    void setSegmentId(int segmentId){_segmentId = segmentId;}
    LatLng startLatLng() { return _pointList.at(0);}
       void setStartLatLng(LatLng latLng) {_startLat = latLng.lat();
                                           _startLon = latLng.lon();
                                          }
   LatLng endLatLng() {return _pointList.at(_points-1);}
       void setEndLatLng(LatLng latLng) {_endLat = latLng.lat();
                                         _endLon = latLng.lon();
                                        }
    //QString streetName() const { return _streetName;}
    // newerName() {return _newerName;}
    //QString description() const {return _description;}
    //int tracks() {return _tracks;}
    //void setTracks(int tracks){_tracks = tracks;}
    //QList<LatLng> pointList() {return _pointList;}
    // double startLat() {return _startLat;}
    // double startLon() {return _startLon;}
    // double endLat() {return _endLat;}
    // double endLon() {return _endLon;}
    // void setStartLat(double startLat) {this->_startLat = startLat;}
    // void setStartLon(double startLon) {this->_startLon = startLon;}
    // void setEndLat(double endLat) {this->_endLat = endLat;}
    // void setEndLon(double endLon) {this->_endLon = endLon;}
    //RouteType routeType() {return _routeType;}
    //QString whichEnd() {return _whichEnd;}
    //void setWhichEnd(QString whichEnd){_whichEnd = whichEnd;}
    // void setStreetName(QString streetName){_streetName = streetName;}
    // void setNewerName(QString newerName) {_newerName = newerName;}
    // void setDescription(QString description){_description = description;}
    QString oneWay() const {return _oneWay;}
    void setOneWay(QString oneWay){_oneWay = oneWay;}
    // QString direction() {
    //     _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
    //     if(tracks() == 1)
    //         _direction = _bearing.strDirection();
    //     else
    //         _direction = _bearing.strDirection() + "-" + _bearing.strReverseDirection();
    //     return _direction;
    // }
    void setDirection(QString direction){_direction = direction;} // ??
    // int next() const {return _next;}
    // void setNext(int next){_next = next;}
    int prev() {return _prev;}
    void setPrev(int prev){_prev = prev;}
    int nextR() const {return _nextR;}
    void setNextR(int next){_nextR = next;}
    int prevR() {return _prevR;}
    void setPrevR(int prev){_prevR = prev;}
    // Bearing bearing() {
    //     _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
    //     _direction = _bearing.strDirection();
    //     _length = _bearing.Distance();
    //     return _bearing;
    // }
    // void setBearing(Bearing bearing){_bearing = bearing;}
    // Bearing bearingStart() {
    //     //     if(_pointList.count() < 2)
    //     //      return _bearing;
    //     //     _bearingStart = Bearing(_startLat, _startLon, _pointList.at(1).lat(), _pointList.at(1).lon());
    //     return _bearingStart;
    // }
    // Bearing bearingEnd() {
    //     //     if(_pointList.count() < 2)
    //     //      return _bearing;
    //     //     _bearingEnd =  Bearing(_pointList.at(_points-2).lat(), _pointList.at(_points-2).lon(), _endLat, _endLon);
    //     return _bearingEnd;
    // }
    double getLength();
    //QDate startDate() const {return _dateBegin;}
    QDate segmentStartDate() const {return _segmentDateStart;}
    //QDate endDate() const {return _dateEnd;}
    QDate segmentEndDate() const {return _segmentDateEnd;}
    void setEndDate(QDate endDate) {_dateEnd = endDate;}
    void setSegmentStartDate(QDate dt) {_segmentDateStart =dt;}
    void setSegmentEndDate(QDate dt) {_segmentDateEnd =dt;}
    //double length() {return _length;}
    //void setRouteType(RouteType rt) {_routeType = rt;}
    //void setStartDate(QDate dt) {_dateBegin = dt;}
    // bool needsUpdate() {return _bNeedsUpdate;}
    // void setNeedsUpdate(bool b){_bNeedsUpdate = b;}
    QT_DEPRECATED void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
    void displaySegment( QString color,  bool bClearFirst);
    //void checkTracks();
    int normalEnter() {return _normalEnter;}
    void setNormalEnter(int normalEnter) {_normalEnter = normalEnter;}
    int normalLeave() {return _normalLeave;}
    void setNormalLeave(int normalLeave) {_normalLeave = normalLeave;}
    void setSequence(int seq) {_sequence = seq;}
    void setReturnSeq(int seq) {_returnSeq = seq;}
    int sequence() const {return _sequence;}
    int returnSeq() const {return _returnSeq;}
    QString trackUsage() const {return _trackUsage;}
    void setTrackUsage(QString s) {_trackUsage = s;}
    int tractionType() const {return _tractionType;}
    void setTractionType(int t) {_tractionType = t;}
    int reverseEnter() {return _reverseEnter;}
    void setReverseEnter(int reverseEnter) {_reverseEnter = reverseEnter;}
    int reverseLeave() {return _reverseLeave;}
    void setReverseLeave(int reverseLeave) {_reverseLeave = reverseLeave;}
    int route() const {return _route;}
    void setRoute(int route){_route = route;}
    int companyKey() const {return _companyKey;}
    void setCompanyKey(int key) {_companyKey = key;}
    QString routeName() const {return _routeName;}
    void setRouteName(QString name) {_routeName = name;}
    // Bounds bounds() {
    //     _sw = LatLng(_startLat < _endLat ? _startLat : _endLat, _startLon < _endLon ? _startLon : _endLon );
    //     _ne = LatLng(_startLat > _endLat ? _startLat : _endLat, _startLon > _endLon ? _startLon : _endLon );
    //     _bounds = Bounds(_sw, _ne);
    //     return _bounds;
    // }
    // QString location() {return _location;}
    // void setLocation(QString loc) {_location = loc;}
    void setAlphaRoute(QString alphaRoute) {_alphaRoute = alphaRoute;}
    QString alphaRoute() const {return _alphaRoute;}
    int routeId() const {return _routeId;}
    void setRouteId(int routeId){this->_routeId = routeId;}
    bool operator==(const SegmentData o)
    {
        return _route == o._route && _alphaRoute == o._alphaRoute && _routeName == o._routeName
               && _dateBegin == o.startDate() && _dateEnd == o._dateEnd && _segmentId == o._segmentId;
    }
    void markForDelete(bool b) {_markedForDelete = b;}
    bool markedForDelete() {return _markedForDelete;}
    QString reverseDescription();
    void updateRouteInfo(RouteData rd);
    quint32 baseRoute() const {return _baseRoute;}
    void update(const SegmentInfo& si);
    QDate doubleDate() const {return _dateDoubled;}
    void setDoubleDate(QDate date){_dateDoubled = date;}
    QString routePrefix() {return _routePrefix;}
    // int streetId(){return _streetId;}
    // void setStreetId(int streetId){_streetId = streetId;}
    // int streetSeq(){return _streetSeq;}
    // void setStreetSeq(int streetSeq){_streetSeq = streetSeq;}

private:
    //qint32 _segmentId=-1;
    //qint8 _tracks=0;
    qint32 _route=-1;
    qint32 _baseRoute=-1;
    //RouteType _routeType = Surface;
    // double _startLat, _startLon, _endLat, _endLon;
    // double _length=0;
    // qint32	_points=0;
    // QString _streetName;
    // int _streetId;
    // int _streetSeq;
    // QString _newerName;
    // QString _description;
    QString _routePrefix;
    // QDate _dateBegin = QDate::fromString("1880/01/01", "yyyy/MM/dd");
    // QDate _dateDoubled;
    // QDate _dateEnd = QDate::fromString("2050/12/31", "yyyy/MM/dd");
    QDate _segmentDateStart;
    QDate _segmentDateEnd;
    // QString _direction = " ";
    // Bearing _bearing;      // bearing from start to end
    // Bearing _bearingStart; // bearing of first portion from point(first +1) to point(first)
    // Bearing _bearingEnd;   // bearing of last portion from point(last-1) to point(last)
    // Bounds _bounds;
    // QList<LatLng> _pointList;
    // QString _whichEnd;     //Not in db, used for sequencing
    QString _oneWay = " ";       //Not in db, used for sequencing
    qint32 _next=-1, _prev=-1;
    qint32 _nextR=-1, _prevR=-1;
    qint32 _sequence=-1, _returnSeq=-1;
    qint32 _normalEnter=0, _normalLeave=0, _reverseEnter=0, _reverseLeave=0;
    bool _bNeedsUpdate = false;
    int _tractionType=0;
    QString _trackUsage = " ";
    LatLng _sw;
    LatLng _ne;
    //QString _trackType = " ";
    void calculate();
    bool _markedForDelete = false;

    //    void setBounds(Bounds bounds) {
    //     if(bounds.isValid())
    //      _bounds = bounds;
    //    }
    int _companyKey = -1;
    int _rowid = -1;
    int _segRowid = -1;
    QString _routeName;
    QString _location;
    QString _alphaRoute;
    int _routeId = -1;
    friend class SQL;
    friend class SegmentInfo;
};
Q_DECLARE_METATYPE(SegmentData)

#endif // SEGMENTDATA_H
