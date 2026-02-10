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
#include <QUrl>

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
enum RouteType { Surface, SurfacePRW, RapidTransit, Subway, Rail,
                 Incline, MagLev, Elevated, Other };
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
 case MagLev:
 case Elevated:
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
 double angle()
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
     bool intersects(Bounds o);
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

 private:
     LatLng _swPt;
     LatLng _nePt;
     bool bBoundsValid;
     //float _swLat, _swLon, _neLat, _neLon;
     double _neLat, _neLon, _swLat, _swLon;
     QStringList sl;
};
Q_DECLARE_METATYPE(Bounds)

class RouteData;
class SegmentData
{
 public:
    //explicit segmentData(QObject *parent = 0);
    SegmentData();
    SegmentData(int route, QString name, int segmentId, QDate startDate, QDate endDate);
    ~SegmentData() {}
    SegmentData(const SegmentData&);
    SegmentData(const SegmentInfo&);
    SegmentData(const RouteData&);

    void addPoint(LatLng pt);
    void insertPoint(int ptNum, LatLng pt);
    void movePoint(int ptNum, LatLng pt);
    void deletePoint(int ptNum);
    void setPoints(QList<LatLng>points);
    void setPoints(QString);
    QString toString();
    QString toString2();
    static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
    QString pointsString();
    int segmentId() const {return _segmentId;}
    void setSegmentId(int segmentId){_segmentId = segmentId;}
    LatLng startLatLng() { return _pointList.at(0);}
//    void setStartLatLng(LatLng latLng) {_startLat = latLng.lat();
//                                        _startLon = latLng.lon();
//                                       }
    LatLng endLatLng() {return _pointList.at(_points-1);}
//    void setEndLatLng(LatLng latLng) {_endLat = latLng.lat();
//                                      _endLon = latLng.lon();
//                                     }
    QString streetName() { return _streetName;}
    QString newerName() {return _newerName;}
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
    void setNewerName(QString newerName) {_newerName = newerName;}
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
    int nextR() const {return _nextR;}
    void setNextR(int next){_nextR = next;}
    int prevR() {return _prevR;}
    void setPrevR(int prev){_prevR = prev;}
    Bearing bearing() {
     _bearing = Bearing(_startLat, _startLon, _endLat, _endLon);
     _direction = _bearing.strDirection();
     _length = _bearing.Distance();
     return _bearing;
    }
    void setBearing(Bearing bearing){_bearing = bearing;}
    Bearing bearingStart() {
//     if(_pointList.count() < 2)
//      return _bearing;
//     _bearingStart = Bearing(_startLat, _startLon, _pointList.at(1).lat(), _pointList.at(1).lon());
     return _bearingStart;
    }
    Bearing bearingEnd() {
//     if(_pointList.count() < 2)
//      return _bearing;
//     _bearingEnd =  Bearing(_pointList.at(_points-2).lat(), _pointList.at(_points-2).lon(), _endLat, _endLon);
     return _bearingEnd;
    }
    double getLength();
    QDate startDate() const {return _dateBegin;}
    QDate segmentStartDate() const {return _segmentDateStart;}
    QDate endDate() const {return _dateEnd;}
    QDate segmentEndDate() const {return _segmentDateEnd;}
    void setEndDate(QDate endDate) {_dateEnd = endDate;}
    void setSegmentStartDate(QDate dt) {_segmentDateStart =dt;}
    void setSegmentEndDate(QDate dt) {_segmentDateEnd =dt;}
    double length() {return _length;}
    void setRouteType(RouteType rt) {_routeType = rt;}
    void setStartDate(QDate dt) {_dateBegin = dt;}
    bool needsUpdate() {return _bNeedsUpdate;}
    void setNeedsUpdate(bool b){_bNeedsUpdate = b;}
    QT_DEPRECATED void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
    void displaySegment( QString color,  bool bClearFirst);
    void checkTracks();
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
    Bounds bounds() {
     _sw = LatLng(_startLat < _endLat ? _startLat : _endLat, _startLon < _endLon ? _startLon : _endLon );
     _ne = LatLng(_startLat > _endLat ? _startLat : _endLat, _startLon > _endLon ? _startLon : _endLon );
     _bounds = Bounds(_sw, _ne);
     return _bounds;
    }
    QString location() {return _location;}
    void setLocation(QString loc) {_location = loc;}
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
    int streetId(){return _streetId;}
    void setStreetId(int streetId){_streetId = streetId;}

 private:
    qint32 _segmentId=-1;
    qint8 _tracks=0;
    qint32 _route=-1;
    qint32 _baseRoute=-1;
    RouteType _routeType = Surface;
    double _startLat, _startLon, _endLat, _endLon;
    double _length=0;
    qint32	_points=0;
    QString _streetName;
    int _streetId;
    QString _newerName;
    QString _description;
    QString _routePrefix;
    QDate _dateBegin = QDate::fromString("1880/01/01", "yyyy/MM/dd");
    QDate _dateDoubled;
    QDate _dateEnd = QDate::fromString("2050/12/31", "yyyy/MM/dd");
    QDate _segmentDateStart;
    QDate _segmentDateEnd;
    QString _direction = " ";
    Bearing _bearing;      // bearing from start to end
    Bearing _bearingStart; // bearing of first portion from point(first +1) to point(first)
    Bearing _bearingEnd;   // bearing of last portion from point(last-1) to point(last)
    Bounds _bounds;
    QList<LatLng> _pointList;
    QString _whichEnd;     //Not in db, used for sequencing
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

class RouteData
{
public:
    //explicit routeData(QObject *parent = 0);
    RouteData();
    ~RouteData();
    RouteData(const RouteData&);
    RouteData(const SegmentData&);
    const QString toString();
    int route() {return _route;}
    void setRoute(int route) {_route = route;}
    void setRouteId(int routeId) {_routeId = routeId;}
    QString alphaRoute() const {return _alphaRoute;}
    void setAlphaRoute(QString alphaRoute) {_alphaRoute = alphaRoute;}
    QString routeName() const {return _name;}
    void setRouteName(QString name) {_name = name;}
    QDate startDate() const {return _dateBegin;}
    QDate doubleDate() const {return _doubleDate;}
    void setStartDate(QDate date) {_dateBegin = date;}
    QDate endDate() const {return _dateEnd;}
    void setEndDate(QDate date) {_dateEnd = date;}
    void setDoubleDate(QDate date) {_doubleDate = date;}
    qint32 companyKey() {return _companyKey;}
    void setCompanyKey(int key) {_companyKey = key;}
    QString companyMnemonic() {return _companyMnemonic;}
    void setCompanyMnemonic(QString mnemonic){_companyMnemonic = mnemonic;}
    qint32 tractionType() {return _tractionType;}
    void setTractionType(int t) {_tractionType = t;}
    qint32 lineKey() {return _lineKey;}
    qint32 segmentId() {return _lineKey;}
    QString direction() {return _direction;}
    qint32 normalEnter() {return _normalEnter;}
    qint32 normalLeave() {return _normalLeave;}
    qint32 reverseEnter() {return _reverseEnter;}        // Not defined for one Way
    qint32 reverseLeave() {return _reverseLeave;}        // Not defined for one Way
    QString oneWay() {return _oneWay;}
    QString trackUsage() {return _trackUsage;}
    int next() {return _next;}
    int prev() {return _prev;}
    int nextR() {return _nextR;}
    int prevR() {return _prevR;}
    int tracks() {return _tracks;}
    QList<QPair<int, QString>> seqList() {return _seqList;}
    void addSequence(QPair<int, QString> pair) {_seqList.append(pair);}
    QList<QPair<int, QString>> setSeqList(QString);
    QString seqToString();
    qint32 sequence() {return _sequence;}
    qint32 returnSeq() {return _returnSeq;}
    QString routePrefix() {return _routePrefix;}
    void setRoutePrefix(QString routePrefix) {_routePrefix = routePrefix;}
    int baseRoute() {return _baseRoute;}
    void setBaseRoute(int baseRoute){_baseRoute = baseRoute;}
    RouteType routeType() {return _routeType;}
    int routeId(){return _routeId;}
    bool operator==(const RouteData &o);

 private:
    qint32 _route = -1;
    qint32 _routeId = -1;
    qint32 _baseRoute=0;
    QString _alphaRoute;
    QString _routePrefix;
    QString _name;
    //QDate defaultDate;
    QDate _dateBegin;
    QDate _dateEnd;
    QDate _doubleDate;
    qint32 _companyKey;
    QString _companyMnemonic;
    qint32 _tractionType;
    qint32 _lineKey = -1;
    QString _direction;
    qint32 _normalEnter;
    qint32 _normalLeave;
    qint32 _reverseEnter;        // Not defined for one Way
    qint32 _reverseLeave;        // Not defined for one Way
    qint32 _sequence;
    qint32 _returnSeq;
    QString _oneWay;
    QString _trackUsage;
    int _next =-1, _prev=-1;
    int _nextR =-1, _prevR=-1;
    int _tracks;
//    LatLng startLatLng;
//    LatLng endLatLng;
//    SegmentData sd;
//    Bearing* bearing = nullptr;
    bool bNeedsUpdate = false;
    RouteType _routeType;
    QList<QPair<int, QString>> _seqList;
signals:

public slots:
 friend class SQL;
 friend class SegmentData;
};
Q_DECLARE_METATYPE(RouteData)

class RouteSeq
{
 public:
  RouteSeq() {}
  ~RouteSeq() {}
  RouteSeq(const RouteSeq&);
  QList<QPair<int, QString>> seqList() {return _seqList;}
  QString routeName() {return _rd.routeName();}
  void setRouteName(QString name) {_rd.setRouteName(name);}
  int route() {return _rd.route();}
  QDate startDate() {return _rd.startDate();}
  QDate endDate() {return _rd.endDate();}
  QString whichEnd() {return _whichEnd;}
  int firstSegment() {return _firstSegment;}
  QString listString();

 private:
  QList<QPair<int, QString>> _seqList;
  int _firstSegment;
  QString _whichEnd;
  RouteData _rd =RouteData();
  QDateTime _lastUpdate;
  friend class SQL;
};


class segmentGroup
{
    public:
        //explicit segmentGroup(QObject *parent = 0);
        Q_DECL_DEPRECATED segmentGroup();
        qint32	SegmentId;
        QString Description;
        QString OneWay;
        double distance;
        //qint32	lineSegments;
        QString startDate;
        QString endDate;
        qint32	tractionType;
        RouteType routeType;
        double startLat, startLon, endLat, endLon;
        QList<SegmentData> data ;      // array of segmentdata objects
        QList<LatLng> points;
        ~segmentGroup();
};

class RouteInfo
{
	public:
        explicit RouteInfo(QObject *parent = 0);
        RouteInfo(qint32 route, QString name, QDate startDate, QDate endDate, int companyKey,
                  QString alphaRoute, int routeId = -1);
        ~RouteInfo() {}
        RouteInfo(RouteData rd);
        RouteInfo(RouteInfo& o);
        RouteInfo(const SegmentData &o);

        //QList<segmentGroup> segments;  // array of segmentGroup objects
        void setRouteId(int routeId){_routeId = routeId;}
        int routeId()const {return _routeId;}
        void setRouteName(QString name) {routeName = name;}

 private:
        qint32	route=-1;
        QString routeName;
        qint32	tractionType;
        QList<SegmentData*> segmentDataList;
        double length;
        RouteData rd;
        QDate startDate;
        QDate endDate;
        int companyKey=0;
        QString alphaRoute;
        int baseRoute=0;
        QString routePrefix;
        QString companyMnemonic;
        int _routeId = -1;
        friend class SQL;
};

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
        QDate startDate;
        QDate endDate;
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
 QString _description;
 bool _formatOK = false;
 qint32 _segmentId = -1;
 qint32 _points =0;
 double _length = 0;
 QDate _dateBegin = QDate::fromString("1880/01/01", "yyyy/MM/dd");
 QDate _dateDoubled;
 QDate _dateEnd= QDate::fromString("2050/12/31", "yyyy/MM/dd");
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
 int _rowid = -1;
 //int routeCount;
 //QString trackUsage;
 int _next = -1; // needed for DupSegmentView
// int _prev = -1;
 QString _trackType = " ";
 QString _location;
 QString _newerStreetName;

 public:
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
 QDate startDate() {return _dateBegin;}
 QDate endDate() {return _dateEnd;}
 double startLat() {return _startLat;}
 double startLon() {return _startLon;}
 double endLat() {return _endLat;}
 double endLon() {return _endLon;}
 QString streetName() {return _streetName;}
 QString newerName() {return _newerStreetName;}
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
 void setNext(int next){_next = next;}
 int next() const {return _next;}
 QString reverseDescription();
 QDate doubleDate() {return _dateDoubled;}
 void setDoubleDate(QDate date){_dateDoubled = date;}
 void setFormatOK(bool b){_formatOK = b;}
 bool formatOK(){return _formatOK;}
 int streetId(){return _streetId;}
 void setStreetId(int streetId){_streetId = streetId;}
 bool operator==(const SegmentInfo& other) const{
     return(other._segmentId == _segmentId);

 }

 friend class SegmentData;
 friend class SQL;
 friend class StreetsTableModel;
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
        ~StationInfo() {}
        StationInfo(const StationInfo&);
        QStringList routes;
        QString stationName;
        QString stationSuffix; // if multiple stops exist at say the intersection of two lines, each stop can be identified with a suffix code.
        double latitude;       // latitude of the station
        double longitude;      // longitude of the station.
        qint32 stationKey;
        qint32 infoKey;        // key to the comments table containing descritptive text for the station.
        QDate startDate;
        QDate endDate;
        RouteType routeType;
        QString markerType;
QT_DEPRECATED  int segmentId = -1;
        QStringList segments; // list of segments
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
    CompanyData(const CompanyData& o)
    {
        companyKey = o.companyKey;
        name = o.name;
        mnemonic = o.mnemonic;
        routePrefix = o.routePrefix;
        startDate = o.startDate;
        endDate = o.endDate;
        firstRoute =o.firstRoute;
        lastRoute = o.lastRoute;
        info = o.info;
        url = o.url;
        bSelected =o.bSelected;
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
    QString mnemonic;
    QString info;
    QUrl url;
    QDateTime lastUpdated;
    bool bSelected;
};

class Parameters
{
public:
    Parameters(){
    }
    QList<QPair<QString, QString>> abbreviationsList;
    QString title;
    QString city;
    double lat, lon;
    QDate minDate, maxDate;
    bool bAlphaRoutes= false;
};

class CommentInfo
{
public:
    CommentInfo(){commentKey = -1;}
    CommentInfo(int commentKey) {this->commentKey = commentKey;}
    ~CommentInfo() {}
    CommentInfo(const CommentInfo& other);
    qint32 commentKey = -1;
    QString tags;
    QString comments;
    QStringList usedByRoutes;
    QStringList usedByStations;
    int routeCount;
    int stationCount;
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

class RouteComments
{
public:
    qint32 route;
    QDate date;
    qint32 commentKey;
    CommentInfo ci;
    LatLng pos;
    qint32 companyKey;
    QString name;
    QString routeAlpha;
    RouteComments(){}
    ~RouteComments() {}
    RouteComments(const RouteComments&other);
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
     routeNbr = rd.route();
     this->routeName = rd.routeName();
     this->routeSegment = segmentId;
     tractionType = rd.tractionType();
     companyKey =rd.companyKey();
     dateEnd = rd.endDate();
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
  int id = 0;
  int seq = 0;
  QString table;
  QString from;
  QString to;
  QString on_update;
  QString on_delete;
  QString match;
};

Q_DECLARE_METATYPE(RouteType)

class StreetInfo
{
public:
    QString street;
    QString olderName;
    QString newerName;
    LatLng startLatLng;
    LatLng endLatLng;
    double length = 0;
    QDate dateStart;
    QDate dateEnd;
    QDate dateSort;
    QList<int> segments;
    QString comment;
    Bounds bounds;
    int streetId = -1;
    QString location;
    int sequence = 0;
    int rowid =-1;
    StreetInfo() {}
    ~StreetInfo() {}
    StreetInfo(const StreetInfo&);
    QList<int> setSegments(const QString);
    QString segmentsToString();
    void updateSegmentInfo(SegmentInfo si);
    void updateBounds();
    bool inDateRange(QDate dt);
    bool encompasses(StreetInfo sti);
    QString toString();
};
Q_DECLARE_METATYPE(StreetInfo)
#endif // DATA_H
