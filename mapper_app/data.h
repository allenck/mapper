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
class RouteChangedEventArgs
{
public:
    RouteChangedEventArgs()
    {

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
     RouteName = n;
     routeSegment = s;
     tractionType = tt;
     companyKey =ck;
     dateEnd = de;
     typeOfChange = type;
    }
    qint32 routeNbr;
    QString RouteName;
    qint32 routeSegment;
    qint32 tractionType;
    qint32 companyKey;
    QDate dateEnd;
    QString typeOfChange;
};
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
class RouteData
{

public:
    //explicit routeData(QObject *parent = 0);
    RouteData();
    qint32 route;
    QString alphaRoute;
    //qint32 baseRoute;
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
    int next, prev;

    QString toString();
    ~RouteData();
    RouteData(const RouteData&);
signals:

public slots:

};

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

class SegmentData
{
    public:
        //explicit segmentData(QObject *parent = 0);
        SegmentData();
        SegmentData(qint32 Pt, qint32 SId);
        qint32 key;
        qint32 SegmentId;
        qint32 sequence;
        double startLat, startLon, endLat, endLon;
        double distance;
        QString streetName;
        QString description;
        QDateTime startDate;
        QDateTime endDate;
        RouteType routeType;
        QString whichEnd;
        qint32 endSegment;
        qint32 route;
        QString alphaRoute;
        bool oneWay;
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
        Q_DECL_DEPRECATED QList<SegmentData> data ;      // array of segmentdata objects
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


 QString toString()
 {
  QString str;
  QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";

  if(routeType < 0 || routeType>= routeTypes.count())
   routeType = (RouteType)0;
  QString trackType = routeTypes.at(routeType);
  QString strSegment = QString("%1").arg(segmentId);
   if (tracks == 1)
       str = description + QString("(single/%2) Seg=%1").arg(segmentId).arg(trackType);
   else
       str = description + QString(" (double/%2) Seg=%1").arg(segmentId).arg(trackType);
  return str;
 }

 void setPoints(QString sPoints);

 QString pointsString();

 void addPoint(LatLng pt);
 void insertPoint(int ptNum, LatLng pt);
 void movePoint(int ptNum, LatLng pt);
 void deletePoint(int ptnum);
 void checkTracks();
 void displaySegment(QString date, QString color, bool bClearFirst);

};

class routeIntersects
{
    public:
    explicit routeIntersects(QObject *parent = 0){Q_UNUSED(parent)}
        RouteData rd;
        QList<SegmentInfo> startIntersectingSegments;
        QList<SegmentInfo> endIntersectingSegments;
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
Q_DECLARE_METATYPE(RouteType)
#endif // DATA_H
