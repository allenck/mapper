#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QtCore/qmath.h>
#include <QStringList>
#include <QPersistentModelIndex>
#include <QRectF>
#include "latlng.h"
#include <iostream>

class LatLng;
class SegmentInfo;
#if 0
class segmentChangedEventArgs
{
public:
    segmentChangedEventArgs(qint32 chgSeg, qint32 newSeg)
    {
        qint32 chgSegment = chgSeg;
        qint32 newSegment = newSeg;
    }
    qint32 chgSegment;
    qint32 newSegment;
};
#endif
enum RouteType { Surface, SurfacePRW, RapidTransit, Subway, Rail, Incline, Other };
static bool to_enum(int n)
{
 switch (n)
 {
 case Surface:
 case SurfacePRW:
 case RapidTransit:
 case Subway:
 case Rail:
 case Incline:
 case Other:
  return true;
 default:
  return false;
 }
}


class Bearing
{
 private:
  double R; // RADIUS OF THE EARTH IN KM

  double dToRad;
  double lat1;
  double lon1;
  double lat2;
  double lon2;
  double dLat;
  double dLon;
  double brng;
  QStringList strNormal;
  QStringList strReverse;
  QString _strDirection;
  QList<qint32> reverseDirection;
  qint32 direction;
  double d; // distance
 public:
 Bearing();

 Bearing(LatLng start, LatLng end);
 /// <summary>
 /// Constructor
 /// </summary>
 /// <param name="startLat"></param>
 /// <param name="startLon"></param>
 /// <param name="endLat"></param>
 /// <param name="endLon"></param>
 Bearing(double startLat, double startLon, double endLat, double endLon);
private:
 void common();
 void calculate();
public:
 qint32 getDirection()
 {
     return direction;
 }
 qint32 getReverseDirection()
 {
     return reverseDirection[direction];
 }
 double getBearing()
 {
  return brng;
 }
 /// <summary>
 /// gets distance in km
 /// </summary>
 double Distance() {return d;}
 /// <summary>
 /// gets the direction string, e.g. "N", "NE", etc.
 /// </summary>
 QString strDirection()
 {
  if(direction < 0 || direction > 7)
   return "?";
  return strNormal.at(direction);
 }
 /// <summary>
 ///  gets the reverse direction string, e.g. "S", "SW", etc.
 /// </summary>
 QString strReverseDirection()
 {
  if(direction < 0 || direction > 7)
   return "?";
  return strReverse.at(direction);
 }
 /// <summary>
 /// set a direction string, e.g. "N", "NE", etc.
 /// </summary>
 void DirectionString(QString value)
 {
     _strDirection = value;
 }
 /// <summary>
 /// return the reverse direction for DirectionString
 /// </summary>
 QString ReverseDirectionString()
 {
     int i;
     for (i = 0; i < strNormal.size(); i++)
     {
         if (_strDirection == strNormal[i])
             return strReverse[i];
     }
     return "";
 }
};

class Bounds : public QObject
{
  Q_OBJECT
 public:
 Bounds();
 Bounds(LatLng sw, LatLng ne);
 Bounds(QString bnds);
 ~Bounds();
 Bounds(const Bounds& other);
 static Bounds fromPointList(QList<LatLng>);
 bool isValid();
 bool checkValid();
 bool updateBounds(LatLng pt);
 bool updateBounds(Bounds bnds);
 void setBottomLeft(LatLng pt) {_swPt = pt;}
 void setTopRight(LatLng pt){_nePt = pt;};
 void operator=(const Bounds& other){
  _swPt = other._swPt;
  _nePt = other._nePt;
  _neLat = other._neLat;
  _neLon = other._neLon;
  _swLat = other._swLat;
  _swLon = other._swLon;
  bBoundsValid = other.bBoundsValid;
 }

 LatLng swPt() const;
 LatLng nePt() const;
 QString toString();
 bool contains(const LatLng &p) const;
 bool contains(const Bounds &b) const;

 LatLng center();
// QString swLat() {s_swLat = QString::number(_swLat); return s_swLat;}
// QString swLon() {s_swLon = QString::number(_swLat); return s_swLon;}
// QString neLat() {s_neLat = QString::number(_neLat); return s_neLat;}
// QString neLon() {s_neLon = QString::number(_neLon); return s_neLon;}

// void set_swLat(double val) {s_swLat = QString::number(val); _swLat = val; }
// void set_swLon(double val) {s_swLon = QString::number(val); _swLon = val; }
// void set_neLat(double val) {s_neLat = QString::number(val); _neLat = val; }
// void set_neLon(double val) {s_neLon = QString::number(val); _neLon = val; }
// QString  s_swLat, s_swLon, s_neLat, s_neLon;


 private:
 LatLng _swPt;
 LatLng _nePt;
 bool bBoundsValid;
 //float _swLat, _swLon, _neLat, _neLon;
 double _neLat, _neLon, _swLat, _swLon;
 QStringList sl;
// friend QDataStream& operator<<(QDataStream& out, const Bounds& v) {
//     out << v.swLat() << v.swLon() << v.neLat() << v.neLon();
//     return out;
// }

// friend QDataStream& operator>>(QDataStream& in, Bounds& v) {
//     in >> v.s_swLat >> v.s_swLon >> v.s_neLat >> v.s_neLon;

//     return in;
// }

};
Q_DECLARE_METATYPE(Bounds)
//inline QDataStream& operator<<(QDataStream& out, const Bounds& v) {
//    out << v.swLat() << v.swLon() << v.neLat() << v.neLon();
//    return out;
//}

//inline QDataStream& operator>>(QDataStream& in, Bounds& v) {
//    in >> v.s_swLat >> v.s_swLon >> v.s_neLat >> v.s_neLon;

//    return in;
//}
class SegmentData
{
public:
    //explicit segmentData(QObject *parent = 0);
    SegmentData();
    ~SegmentData() {}
    SegmentData(const SegmentData&);
    SegmentData(const SegmentInfo&);
    void addPoint(LatLng pt);
    void insertPoint(int ptNum, LatLng pt);
    void movePoint(int ptNum, LatLng pt);
    void deletePoint(int ptNum);
    void setPoints(QString);
    QString toString();
    QString toString2();
    static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
    QString pointsString();
    int segmentId() const {return _segmentId;}
    void setSegmentId(int segmentId){_segmentId = segmentId;}
    LatLng startLatLng() { return LatLng(_startLat, _startLon);}
    void setStartLatLng(LatLng latLng) {_startLat = latLng.lat();
                                        _startLon = latLng.lon();
                                       }
    LatLng endLatLng() { return LatLng(_endLat, _endLon);}
    void setEndLatLng(LatLng latLng) {_endLat = latLng.lat();
                                      _endLon = latLng.lon();
                                     }
    QString streetName() { return _streetName;}
    QString description() const {return _description;}
    int tracks() {return _tracks;}
    void setTracks(int tracks){_tracks = tracks;}
    QList<LatLng> pointList() {return _pointList;}
    double startLat() {return _startLat;}
    double startLon() {return _startLon;}
    double endLat() {return _endLat;}
    double endLon() {return _endLon;}
    void setStartLat(double startLat) {this->_startLat = startLat;}
    void setStartLon(double startLon) {this->_startLon = startLon;}
    void setEndLat(double endLat) {this->_endLat = endLat;}
    void setEndLon(double endLon) {this->_endLon = endLon;}
    RouteType routeType() {return _routeType;}
    QString whichEnd() {return _whichEnd;}
    void setWhichEnd(QString whichEnd){_whichEnd = whichEnd;}
    void setStreetName(QString streetName){_streetName = streetName;}
    void setDescription(QString description){_description = description;}
    QString oneWay() const {return _oneWay;}
    void setOneWay(QString oneWay){_oneWay = oneWay;}
    QString direction() {
     _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
     if(tracks() == 1)
      _direction = _bearing.strDirection();
     else
      _direction = _bearing.strDirection() + "-" + _bearing.strReverseDirection();
     return _direction;
    }
    void setDirection(QString direction){_direction = direction;} // ??
    int next() const {return _next;}
    void setNext(int next){_next = next;}
    int prev() {return _prev;}
    void setPrev(int prev){_prev = prev;}
    Bearing bearing() {
     _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
     _direction = _bearing.strDirection();
     _length = _bearing.Distance();
     return _bearing;
    }
    void setBearing(Bearing bearing){_bearing = bearing;}
    Bearing bearingStart() {
     if(_pointList.count() < 2)
      return _bearing;
     _bearingStart = Bearing(_startLat, _startLon, _pointList.at(1).lat(), _pointList.at(1).lon());
     return _bearingStart;
    }
    Bearing bearingEnd() {
     if(_pointList.count() < 2)
      return _bearing;
     _bearingEnd =  Bearing(_pointList.at(_points-2).lat(), _pointList.at(_points-2).lon(), _endLat, _endLon);
     return _bearingEnd;
    }
    double getLength();
    QDate startDate() const {return _startDate;}
    QDate endDate() const {return _endDate;}
    void setEndDate(QDate endDate) {_endDate = endDate;}
    double length() {return _length;}
    void setRouteType(RouteType rt) {_routeType = rt;}
    void setStartDate(QDate dt) {_startDate = dt;}
    bool needsUpdate() {return _bNeedsUpdate;}
    void setNeedsUpdate(bool b){_bNeedsUpdate = b;}
    void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
    void checkTracks();
    int normalEnter() {return _normalEnter;}
    void setNormalEnter(int normalEnter) {_normalEnter = normalEnter;}
    int normalLeave() {return _normalLeave;}
    void setNormalLeave(int normalLeave) {_normalLeave = normalLeave;}
    void setSequence(int seq) {_sequence = seq;}
    void setReturnSeq(int seq) {_returnSeq = seq;}
    int sequence() const {return _sequence;}
    int returnSeq() const {return _returnSeq;}
    QString trackUsage() {return _trackUsage;}
    void setTrackUsage(QString s) {_trackUsage = s;}
    int tractionType() {return _tractionType;}
    void setTractionType(int t) {_tractionType = t;}
    int reverseEnter() {return _reverseEnter;}
    void setReverseEnter(int reverseEnter) {_reverseEnter = reverseEnter;}
    int reverseLeave() {return _reverseLeave;}
    void setReverseLeave(int reverseLeave) {_reverseLeave = reverseLeave;}
    int route() const {return _route;}
    void setRoute(int route){_route = route;}
    int companyKey() {return _companyKey;}
    void setCompanyKey(int key) {_companyKey = key;}
    QString routeName() const {return _routeName;}
    void setRouteName(QString name) {_routeName = name;}
    Bounds bounds() {
     LatLng _sw = LatLng(_startLat < _endLat ? _startLat : _endLat, _startLon < _endLon ? _startLon : _endLon );
     LatLng _ne = LatLng(_startLat > _endLat ? _startLat : _endLat, _startLon > _endLon ? _startLon : _endLon );
     _bounds = Bounds(_sw, _ne);
     return _bounds;
    }
    QString location() {return _location;}
    void setLocation(QString loc) {_location = loc;}
    void setAlphaRoute(QString alphaRoute) {_alphaRoute = alphaRoute;}
    QString alphaRoute() {return _alphaRoute;}

 private:
    qint32 _segmentId=-1;
    qint8 _tracks;
    int _route;
    RouteType _routeType;
    double _startLat, _startLon, _endLat, _endLon;
    double _length;
    qint32	_points;
    QString _streetName;
    QString _description;
    QDate _startDate = QDate::fromString("1880/01/01", "yyyy/MM/dd");
    QDate _endDate = QDate::fromString("2050/12/31", "yyyy/MM/dd");
    QString _direction = " ";
    Bearing _bearing;      // bearing from start to end
    Bearing _bearingStart; // bearing of first portion from point(first +1) to point(first)
    Bearing _bearingEnd;   // bearing of last portion from point(last-1) to point(last)
    Bounds _bounds;
    QList<LatLng> _pointList;
    QString _whichEnd;     //Not in db, used for sequencing
    QString _oneWay;       //Not in db, used for sequencing
    qint32 _next=-1, _prev=-1;      //Not in db, used for sequencing
    qint32 _sequence=-1, _returnSeq=-1;
    qint32 _normalEnter=0, _normalLeave=0, _reverseEnter=0, _reverseLeave=0;
    bool _bNeedsUpdate = false;
    int _tractionType;
    QString _trackUsage;
    LatLng _sw;
    LatLng _ne;
    QString _trackType = " ";

    void setBounds(Bounds bounds) {
     if(bounds.isValid())
      _bounds = bounds;
    }
    int _companyKey = -1;
    QString _routeName;
    QString _location;
    QString _alphaRoute;
    friend class SQL;
    friend class SegmentInfo;
};
Q_DECLARE_METATYPE(SegmentData)

class RouteData
{

public:
    //explicit routeData(QObject *parent = 0);
    RouteData();
    ~RouteData();
    RouteData(const RouteData&);
    qint32 route;
    QString alphaRoute;
    QString name;
    QDate defaultDate;
    QDate startDate;
    QDate endDate;
    qint32 companyKey;
    qint32 tractionType;

#if 0
    qint32 lineKey;
    QString direction;
    qint32 normalEnter;
    qint32 normalLeave;
    qint32 reverseEnter;        // Not defined for one Way
    qint32 reverseLeave;        // Not defined for one Way
    QString oneWay;
    QString trackUsage;
    int next =-1, prev=-1;
    LatLng startLatLng;
    LatLng endLatLng;
    SegmentData sd;
    Bearing* bearing = nullptr;
    bool bNeedsUpdate = false;
    RouteType routeType;
#endif
    QString toString();
signals:

public slots:

};
Q_DECLARE_METATYPE(RouteData)


class segmentGroup
{
    public:
        //explicit segmentGroup(QObject *parent = 0);
        Q_DECL_DEPRECATED segmentGroup();
        qint32	SegmentId;
        QString Description;
        QString OneWay;
        double distance;
        qint32	lineSegments;
        QString startDate;
        QString endDate;
        qint32	tractionType;
        RouteType routeType;
        double startLat, startLon, endLat, endLon;
        QList<SegmentData> data ;      // array of segmentdata objects
        QList<LatLng> points;
        ~segmentGroup();
};

#if 0
class RouteInfo
{
	public:
        explicit RouteInfo(QObject *parent = 0);
        RouteInfo(qint32 route, QString name, QString date);
        RouteInfo(RouteData rd);
        qint32	route;
        QString routeName;
        qint32	tractionType;
        //QList<segmentGroup> segments;  // array of segmentGroup objects
        QList<SegmentData> segmentDataList;
        ~RouteInfo();
        double length;
        RouteData rd;
};
#endif

class TerminalInfo
{
    public:
        TerminalInfo()
        {
            route = -1;
            startSegment = -1;
            endSegment = -1;
        }

        qint32 route;
        QString name;
        QDateTime startDate;
        QDateTime endDate;
        qint32 startSegment;
        QString startWhichEnd;
        qint32 endSegment;
        QString endWhichEnd;
        LatLng startLatLng;
        LatLng endLatLng;
};

class TractionTypeInfo
{
    public:
        TractionTypeInfo(){}
        qint32 tractionType;
        QString description;
        QString displayColor;
        RouteType routeType;
        QString icon;
        QString ToString()
        {
            return QString("%1").arg(tractionType) + " " + description;
        }
};

class SegmentInfo
{

	public:
 QString _description;
 //QString oneWay;
 qint32 _segmentId = -1;
 qint32 lineSegments;
 qint32 _points;
 double _length;
 QDate _startDate = QDate::fromString("1880/01/01", "yyyy/MM/dd");
 QDate _endDate= QDate::fromString("2050/12/31", "yyyy/MM/dd");
 Bearing _bearing;
 double _startLat, _startLon, _endLat, _endLon;
 QString _direction = " ";
// qint32 sequence, returnSeq;
// qint32 prev, next;
// qint32 normalEnter;
// qint32 normalLeave;
// qint32 reverseEnter;        // Not defined for one Way
// qint32 reverseLeave;        // Not defined for one Way
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
 //int routeCount;
 //QString trackUsage;
// int _next = -1; // needed for DupSegmentView
// int _prev = -1;
 QString _trackType = " ";
 QString _location;

 static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";

 SegmentInfo()
 {
  //Q_UNUSED(parent)
  _segmentId = -1;
  _pointList = QList<LatLng>();
  _bounds = Bounds();
  _bNeedsUpdate = false;
 }
 SegmentInfo(const SegmentInfo& );
 SegmentInfo(const SegmentData&);
 void setPoints(QString sPoints);
 QString pointsString();
 void addPoint(LatLng pt);
 void insertPoint(int ptNum, LatLng pt);
 void movePoint(int ptNum, LatLng pt);
 void deletePoint(int ptnum);
 void checkTracks();
 void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
 QString toString();
 qint32 segmentId() const {return _segmentId;}
 QDate startDate() {return _startDate;}
 QDate endDate() {return _endDate;}
 double startLat() {return _startLat;}
 double startLon() {return _startLon;}
 double endLat() {return _endLat;}
 double endLon() {return _endLon;}
 LatLng getStartLat() {return LatLng(_startLat, _startLon);}
 LatLng getEndLat() {return LatLng(_endLat, _endLon);}
 QString streetName() {return _streetName;}
 QString description() const {return _description;}
 RouteType routeType() {return _routeType;}
 int tracks() {return _tracks;}
 QList<LatLng> pointList() {return _pointList;}
 Bearing bearing() {
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
 void setEndDate(QDate date) {_endDate = date;}
 void setStartDate(QDate date) {_startDate = date;}
 void setRouteType(RouteType routeType) {_routeType = routeType;}
 void setDescription(QString description) {_description = description;}
 void setTracks(int tracks) {_tracks = tracks;}
 QString direction() {return _direction;}
 void setStreetName(QString streetName) {_streetName = streetName;}
 bool needsUpdate() {return _bNeedsUpdate;}
 void setNeedsUpdate(bool b) {_bNeedsUpdate = b;}
 LatLng getStartLatLng() {return LatLng(_startLat, _startLon);}
 LatLng getEndLatLng() {return LatLng(_endLat, _endLon);}
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
 //int next() const {return _next;}
 void setWhichEnd(QString whichEnd) {_whichEnd = whichEnd;}
 Bounds bounds() {
  LatLng _sw = LatLng(_startLat < _endLat ? _startLat : _endLat, _startLon < _endLon ? _startLon : _endLon );
  LatLng _ne = LatLng(_startLat > _endLat ? _startLat : _endLat, _startLon > _endLon ? _startLon : _endLon );
  _bounds = Bounds(_sw, _ne);
  return _bounds;
 }
 QString location() {return _location;}
 void setLocation(QString loc) {_location = loc;}
 friend class SegmentData;
};

class RouteIntersects
{
    public:
    explicit RouteIntersects(QObject *parent = 0){Q_UNUSED(parent)}
        SegmentData sd;
        QList<SegmentData> startIntersectingSegments;
        QList<SegmentData> endIntersectingSegments;
};

// Stations are some type of transit stop whether it be a Rapid Transit or railroad station or a streetcar or bus stop.
// Stations are associated with a point on a segment of a specific route type. This enables us to
// differentiate say between a streetcar stop at the same location as a subway stop.
class StationInfo
{
    public:
        //explicit stationInfo(QObject *parent = 0);
        StationInfo(){stationKey = -1;}
        QString stationName;
        QString stationSuffix; // if multiple stops exist at say the intersection of two lines, each stop can be identified with a suffix code.
        double latitude;       // latitude of the station
        double longitude;      // longitude of the station.
        Q_DECL_DEPRECATED qint16 lineSegmentId;
        qint32 segmentId;
        qint32 point;
        qint32 stationKey;
        qint32 infoKey;        // key to the comments table containing descritptive text for the station.
        QDateTime startDate;
        QDateTime endDate;
        qint32 geodb_loc_id;   // link to another database containing street information.
        RouteType routeType;
        qint32 route;
        QString alphaRoute;
        QString routeName;
        QString markerType;
};

class CompanyData
{
    public:
    CompanyData(QObject *parent =0)
    {
     Q_UNUSED(parent)
     routePrefix = "";
     companyKey = -1;
    }
    QString toString()
    {
     return QString("%1").arg(companyKey) + " " + name;
    }

    int companyKey;
    QString name;
    QDate startDate, endDate;
    int firstRoute, lastRoute;
    QString routePrefix;
};
class Parameters
{
public:
    Parameters(){
        bAlphaRoutes = false;
    }

    QString title;
    QString city;
    double lat, lon;
    QDateTime minDate, maxDate;
    bool bAlphaRoutes;
};

class CommentInfo
{
 public:
  CommentInfo(){commentKey = -1;}
  CommentInfo(int commentKey) {this->commentKey = commentKey;}
    qint32 commentKey = -1;
    QString tags;
    QString comments;
    QStringList usedByRoutes;
    QStringList usedByStations;
};

class RowChanged
{
public:
    qint32 row;
    QPersistentModelIndex index;
    bool bChanged = false;
    bool bDeleted = false;
    qint32 segmentId;
    SegmentData osd;
    SegmentData sd;
    RowChanged()
    {
     //si = SegmentInfo();
     segmentId = -1;
     row = -1;
     bChanged = false;
     bDeleted = false;
     index = QPersistentModelIndex();
    }
    RowChanged(int row,SegmentData sd )
    {
     this->row = row;
     this->sd = SegmentData(sd);
     bDeleted = true;
    }
    RowChanged(int row,bool bChanged, SegmentData osd, SegmentData sd)
    {
     this->row = row;
     this->bChanged = bChanged;
     this->sd = SegmentData(sd);
     this->osd =SegmentData(osd);
    }
};

struct RouteComments
{
    qint32 route;
    QDate date;
    qint32 commentKey;
    CommentInfo ci;
    LatLng pos;
    qint32 companyKey;
    QString name;
    QString routeAlpha;
};

class RouteChangedEventArgs
{
public:
    RouteChangedEventArgs()
    {

    }

    RouteChangedEventArgs(RouteData rd, int segmentId, QString type)
    {
     typeOfChange = type;
     this->rd = rd;
     routeNbr = rd.route;
     this->routeName = rd.name;
     this->routeSegment = segmentId;
     tractionType = rd.tractionType;
     companyKey =rd.companyKey;
     dateEnd = rd.endDate;
    }

    RouteChangedEventArgs(QString type, qint32 s, qint32 tt)
    {
     typeOfChange = type;
     routeSegment = s;
     tractionType =tt;

    }

    RouteChangedEventArgs( qint32 r, QString n, qint32 s, qint32 tt, qint32 ck, QDate de, QString type)
    {
     routeNbr = r;
     routeName = n;
     routeSegment = s;
     tractionType = tt;
     companyKey =ck;
     dateEnd = de;
     typeOfChange = type;
    }
    qint32 routeNbr =-1;
    QString routeName;
    qint32 routeSegment = -1;
    qint32 tractionType;
    qint32 companyKey;
    QDate dateEnd;
    QString typeOfChange;
    RouteData rd;
};

struct FKInfo
{
  QString name;
  int id;
  int seq;
  QString table;
  QString from;
  QString to;
  QString on_update;
  QString on_delete;
  QString match;
};

Q_DECLARE_METATYPE(RouteType)
#endif // DATA_H
