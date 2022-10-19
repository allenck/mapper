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

class Bounds : public QRectF
{
 public:
 Bounds();
 Bounds(LatLng sw, LatLng ne);
 Bounds(QString bnds);
 ~Bounds();
 Bounds(const Bounds& other);
 bool isValid();
 bool checkValid();
 bool updateBounds(LatLng pt);
 bool updateBounds(Bounds bnds);

 LatLng swPt();
 LatLng nePt();
 QString toString();
 bool contains(const QPointF &p) const;
 LatLng center();

 private:
 LatLng _swPt;
 LatLng _nePt;
 bool bBoundsValid;
};

class SegmentData
{
public:
    //explicit segmentData(QObject *parent = 0);
    SegmentData();
    ~SegmentData() {}
    SegmentData(const SegmentData&);
    void addPoint(LatLng pt);
    void insertPoint(int ptNum, LatLng pt);
    void movePoint(int ptNum, LatLng pt);
    void deletePoint(int ptNum);
    void setPoints(QString);
    QString toString();
    static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
    QString pointsString();
    int segmentId() const {return _segmentId;}
    void setSegmentId(int segmentId){_segmentId = segmentId;}
    LatLng getStartLatLng() { return LatLng(_startLat, _startLon);}
    LatLng getEndLatLng() { return LatLng(_endLat, _endLon);}
    QString getStreetName() { return _streetName;}
    QString getDescription() const {return _description;}
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
    QString streetName() {return _streetName;}
    void setStreetName(QString streetName){_streetName = streetName;}
    QString description() {return _description;}
    void setDescription(QString description){_description = description;}
    QString oneWay() {return _oneWay;}
    void setOneWay(QString oneWay){_oneWay = oneWay;}
    QString direction() {return _oneWay;}
    void setDirection(QString direction){_direction = direction;}
    int next() const {return _next;}
    void setNext(int next){_next = next;}
    int prev() {return _prev;}
    void setPrev(int prev){_prev = prev;}
    Bearing bearing() {return _bearing;}
    void setBearing(Bearing bearing){_bearing = bearing;}
    Bearing bearingStart() {return _bearingStart;}
    Bearing bearingEnd() {return _bearingEnd;}
    QDate startDate() {return _startDate;}
    QDate endDate() {return _endDate;}
    double length() {return _length;}
    void setRouteType(RouteType rt) {_routeType = rt;}
    void setStartDate(QDate dt) {_startDate = dt;}
    void setEndDate(QDate dt) {_endDate = dt;}
    bool needsUpdate() {return bNeedsUpdate;}
    void setNeedsUpdate(bool b){bNeedsUpdate = b;}
    void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);

 private:
    qint32 _segmentId;
    qint32 _tracks;
    RouteType _routeType;
    double _startLat, _startLon, _endLat, _endLon;
    double _length;
    qint32	points;
    QString _streetName;
    QString _description;
    QDate _startDate = QDate::fromString("1880/01/01", "yyyy/MM/dd");
    QDate _endDate = QDate::fromString("2050/12/31", "yyyy/MM/dd");
    QString _direction;
    Bearing _bearing;      // bearing from start to end
    Bearing _bearingStart; // bearing of first portion from point(first +1) to point(first)
    Bearing _bearingEnd;   // bearing of last portion from point(last-1) to point(last)
    Bounds bounds;
    QList<LatLng> _pointList;
    QString _whichEnd;     //Not in db, used for sequencing
    QString _oneWay;       //Not in db, used for sequencing
    int _next, _prev;      //Not in db, used for sequencing
    bool bNeedsUpdate = false;
    friend class SQL;
};

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
    qint32 lineKey;
    qint32 tractionType;
    QString direction;
    qint32 normalEnter;
    qint32 normalLeave;
    qint32 reverseEnter;        // Not defined for one Way
    qint32 reverseLeave;        // Not defined for one Way
    QString oneWay;
    QString trackUsage;
    int next, prev;
    LatLng startLatLng;
    LatLng endLatLng;
    SegmentData* sd = nullptr;
    Bearing* bearing = nullptr;

    QString toString();
signals:

public slots:

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

class RouteInfo
{
	public:
        //explicit routeInfo(QObject *parent = 0);
        RouteInfo();
        qint32	route;
        QString routeName;
        qint32	tractionType;
        //QList<segmentGroup> segments;  // array of segmentGroup objects
        QList<SegmentInfo> segments;
        ~RouteInfo();
        double length;
};


Q_DECLARE_METATYPE(Bounds)

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

class tractionTypeInfo
{
    public:
        tractionTypeInfo(){}
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
 QString description;
 QString oneWay;
 qint32 segmentId = -1;
 qint32 lineSegments;
 qint32 points;
 double length;
 QString startDate;
 QString endDate;
 Bearing bearing;
 double startLat, startLon, endLat, endLon;
 QString direction;
 qint32 sequence, returnSeq;
 qint32 prev, next;
 qint32 normalEnter;
 qint32 normalLeave;
 qint32 reverseEnter;        // Not defined for one Way
 qint32 reverseLeave;        // Not defined for one Way
 Bearing bearingStart;
 Bearing bearingEnd;
 QString whichEnd;
 RouteType routeType;
 int tractionType;
 QString streetName;
 QList<LatLng> pointList;
 Bounds bounds;
 qint8 tracks;
 bool bNeedsUpdate;
 int routeCount;
 QString trackUsage;

 static QStringList ROUTETYPES;// = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";

 SegmentInfo()
 {
  //Q_UNUSED(parent)
  segmentId = -1;
  description = "";
  oneWay = "";
  direction = "  ";
  sequence = -1;
  returnSeq = -1;
  prev = -1;
  next = -1;
  bearing = bearingStart = bearingEnd =Bearing();
  pointList = QList<LatLng>();
  bounds = Bounds();
  bNeedsUpdate = false;
  startDate = "1880/01/01";
  endDate = "2050/12/31";
 }
 void setPoints(QString sPoints);
 QString pointsString();
 void addPoint(LatLng pt);
 void insertPoint(int ptNum, LatLng pt);
 void movePoint(int ptNum, LatLng pt);
 void deletePoint(int ptnum);
 void checkTracks();
 void displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst);
 QString toString();
};

class routeIntersects
{
    public:
    explicit routeIntersects(QObject *parent = 0){Q_UNUSED(parent)}
        RouteData rd;
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
class parameters
{
public:
    parameters(){
        bAlphaRoutes = false;
    }

    QString title;
    QString city;
    double lat, lon;
    QDateTime minDate, maxDate;
    bool bAlphaRoutes;
};
struct commentInfo
{
    qint32 infoKey;
    QString tags;
    QString comments;
};

class RowChanged
{
public:
    qint32 row;
    QPersistentModelIndex index;
    bool bChanged;
    bool bDeleted;
    qint32 segmentId;
    SegmentInfo si;
    QString startDate;
    QString endDate;
    RowChanged()
    {
     //si = SegmentInfo();
     segmentId = -1;
     row = -1;
     bChanged = false;
     bDeleted = false;
     index = QPersistentModelIndex();
    }
};

struct routeComments
{
    qint32 route;
    QDate date;
    qint32 infoKey;
    commentInfo ci;
    LatLng pos;
    qint32 companyKey;
};

class RouteChangedEventArgs
{
public:
    RouteChangedEventArgs()
    {

    }

    RouteChangedEventArgs(RouteData rd, QString type)
    {
     typeOfChange = type;
     this->rd = rd;
     routeNbr = rd.route;
     this->routeName = rd.name;
     this->routeSegment = rd.lineKey;
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
    qint32 routeNbr;
    QString routeName;
    qint32 routeSegment;
    qint32 tractionType;
    qint32 companyKey;
    QDate dateEnd;
    QString typeOfChange;
    RouteData rd;
};

Q_DECLARE_METATYPE(RouteType)
#endif // DATA_H
