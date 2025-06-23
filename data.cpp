#include "data.h"
#include "sql.h"
#include <QString>
#include "webviewbridge.h"
#include "sql.h"

//routeData::routeData(QObject *parent = 0)
//{
//route=-1;
//}
Bearing::Bearing()
{
  common();
}
void Bearing::common()
{
    strNormal <<"N"<< "NE"<< "E"<< "SE"<< "S"<< "SW"<< "W"<< "NW";
    strReverse <<"S"<< "SW"<< "W"<< "NW"<< "N"<< "NE"<< "E"<< "SE";
    reverseDirection <<4<<5<<6<<7<<0<<1<<2<<3;
    R = 6371.0;
    dToRad = 0.0174532925;
    direction = 0;
    d = 0;
    brng = 0;
    lat1=0;
    lat2=0;
    lon1=0;
    lon2=0;
    dLat=0;
    dLon=0;
}

Bearing::Bearing(LatLng start, LatLng end)
{
    common();

    lat1 = start.lat() * dToRad;
    lon1 = start.lon() * dToRad;
    lat2 = end.lat() * dToRad;
    lon2 = end.lon() * dToRad;
    dLat = dToRad * (end.lat() - start.lat());
    dLon = dToRad * (end.lon() - start.lon());

    calculate();
}
/// <summary>
/// Constructor
/// </summary>
/// <param name="startLat"></param>
/// <param name="startLon"></param>
/// <param name="endLat"></param>
/// <param name="endLon"></param>
Bearing::Bearing(double startLat, double startLon, double endLat, double endLon)
{
    common();

    lat1 = startLat * dToRad;
    lon1 = startLon * dToRad;
    lat2 = endLat * dToRad;
    lon2 = endLon * dToRad;
    dLat = dToRad * (endLat - startLat);
    dLon = dToRad * (endLon - startLon);
    calculate();
}
void Bearing::calculate()
{
    double y = qSin(dLon) * qCos(lat2);
    double x = qCos(lat1) * qSin(lat2) -
            qSin(lat1) * qCos(lat2) * qCos(dLon);
    brng = qAtan2(y, x) / dToRad;
    // save values for writing out later
    if (brng < 0)
        brng = 360.0 + brng;
    direction = (int)((brng + 22.5) / 45.0) ;
    if(direction >= 8)
        direction = 0;

    // calculate distance
    double a = qSin(dLat / 2) * qSin(dLat / 2)
        + qCos(lat1) * qCos(lat2)
        * qSin(dLon / 2) * qSin(dLon / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    d = R * c;
}

RouteData::RouteData()
{
 route=-1;
 companyKey = -1;
// lineKey = -1;
// trackUsage = " ";
// sd = SegmentInfo();

}

RouteData::~RouteData()
{
}

RouteData::RouteData(const RouteData& o)
{
 route = o.route;
 alphaRoute = o.alphaRoute;;
 //baseRoute = o.baseRoute;
 name = o.name;
 defaultDate = o.defaultDate;
 startDate = o.startDate;
 endDate = o.endDate;
 companyKey = o.companyKey;
#if 0
 lineKey = o.lineKey;
 tractionType = o.tractionType;
 direction = o.direction;
 normalEnter = o.normalEnter;
 normalLeave = o.normalLeave;
 reverseEnter = o.reverseEnter;        // Not defined for one Way
 reverseLeave = o.reverseLeave;        // Not defined for one Way
 oneWay = o.oneWay;
 next= o.next;
 prev= o.prev;
 trackUsage = o.trackUsage;
 sd = o.sd;
 bearing = o.bearing;
#endif
}


QString RouteData::toString()
{
 bool isNumeric;
 int num = alphaRoute.toInt(&isNumeric);
 QString str;
 if(isNumeric)
  str = alphaRoute + " " + name + " " + startDate.toString("yyyy/MM/dd")+ "->"+ endDate.toString("yyyy/MM/dd");
 else
  str = alphaRoute+"(" +QString::number(route)+ ") " + name + " " + startDate.toString("yyyy/MM/dd")+ "->"+ endDate.toString("yyyy/MM/dd");

 return str;
}
//enum routeType { Surface, SurfacePRW, RapidTransit, Subway, Rail, Other };
segmentGroup::segmentGroup()
{
 distance = 0;
}

segmentGroup::~segmentGroup()
{

}

//segmentData::segmentData(QObject *parent)
//{

//}
SegmentData::SegmentData()
{
 _segmentId = -1;
 _startLat = _startLon = _endLat = _endLon=0;
 _length = 0;
 _streetName = "";
 _bearing = _bearingStart = _bearingEnd =Bearing();
 _route = -1;

}

SegmentData::SegmentData(const SegmentData& o)
{
 _segmentId = o._segmentId;
 _tracks = o._tracks;
 _routeType = o._routeType;
 _startLat = o._startLat;
 _startLon = o._startLon;
 _endLat = o._endLat;
 _endLon = o._endLon;
 _length = o._length;;
 _points = o._points;
 _streetName = o._streetName;
 _description = o._description;
 _startDate = o._startDate;
 _endDate = o._endDate;
 _direction  = o._direction;
 _bearing = o._bearing;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _pointList = o._pointList;
 if(_points == 0 && pointList().count() > 0 )
  _points = pointList().count();
 _next = o._next;
 _prev = o._prev;
 _sequence = o._sequence;
 _returnSeq = o._returnSeq;
 _reverseEnter = o._reverseEnter;
 _reverseLeave = o._reverseLeave;
 _normalEnter = o._normalEnter;
 _normalLeave = o._normalLeave;
 _trackUsage = o._trackUsage;
 _tractionType = o._tractionType;
 _bNeedsUpdate = o._bNeedsUpdate;
 _oneWay = o._oneWay;
 _direction = o._direction;
 _bounds = o._bounds;
 _route = o._route;
 _companyKey = o._companyKey;
 _routeName = o._routeName;
 _location = o._location;
 _alphaRoute = o._alphaRoute;
 _whichEnd = o._whichEnd;
}

// create a new SegmentData from a SegmentInfo
SegmentData::SegmentData(const SegmentInfo& o)
{
 _segmentId = o._segmentId;
 _tracks = o._tracks;
 _routeType = o._routeType;
 _startLat = o._startLat;
 _startLon = o._startLon;
 _endLat = o._endLat;
 _endLon = o._endLon;
 _length = o._length;;
 _points = o._points;
 _streetName = o._streetName;
 _description = o._description;
 _startDate = o._startDate;
 _endDate = o._endDate;
 _direction  = o._direction;
 _bearing = o._bearing;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _pointList = o._pointList;
 if(_points == 0 && pointList().count() > 0 )
  _points = pointList().count();
 _location = o._location;
}

QString SegmentData::toString()
{
 QString str;

 if(_routeType < 0 || _routeType>= ROUTETYPES.count())
  _routeType = (RouteType)0;
 QString trackType = ROUTETYPES.at(_routeType);
 QString strSegment = QString("%1").arg(_segmentId);
  if (_tracks == 1)
      str = _description + QString("(single/%2) Seg=%1").arg(_segmentId).arg(trackType);
  else
      str = _description + QString(" (double/%2) Seg=%1").arg(_segmentId).arg(trackType);
 return str;
}

QString SegmentData::toString2()
{
 QString str = _description + QString(" Seg=%1").arg(_segmentId)
  + QString(" (%1-%2) ").arg(startDate().toString("yyyy/MM/dd")).arg(endDate().toString("yyyy/MM/dd"));
 if(_routeType < 0 || _routeType>= ROUTETYPES.count())
  _routeType = (RouteType)0;
 QString trackType = ROUTETYPES.at(_routeType);
 QString strSegment = QString("%1").arg(_segmentId);
  if (_tracks == 1)
      str = str + QString(" (single/%1)").arg(trackType);
  else
      str = str + QString(" (double/%1)").arg(trackType);
 return str;
}

/*static*/ QStringList SegmentData::ROUTETYPES = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";

void SegmentData::addPoint(LatLng pt)
{
 _pointList.append(pt);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentData::insertPoint(int ptNum, LatLng pt)
{
 // insert ptNum AFTER that point.
 _pointList.insert(ptNum +1, pt);
 //lineSegments = pointList.count()-1;
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentData::movePoint(int ptNum, LatLng pt)
{
 if(ptNum >= _pointList.count()) return;
 _pointList.replace(ptNum, pt);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentData::deletePoint(int ptNum)
{
 if(ptNum < _pointList.count())
  _pointList.removeAt(ptNum);
 else
  qDebug() << QString("delete point %1 from segment %2 failed").arg(ptNum).arg(_segmentId);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentData::setPoints(QString sPoints)
{
 if(_length > 15.0)
  _bNeedsUpdate = true;
 _length = 0;
 QStringList sl = sPoints.split(",");
 if(sl.count()== 0 || ((sl.count()& 0x01) == 1)) return;
 //bool bOk;
 for(int i=0; i < sl.count(); i+=2)
 {
  LatLng latLng = LatLng(sl.at(i).toDouble(), sl.at(i+1).toDouble());
  if(latLng.isValid())
  {
   _pointList.append(latLng);
   bounds().updateBounds(latLng);
  }
 }
 for(int i = 1; i < _pointList.count(); i++)
 {
  Bearing b = Bearing(_pointList.at(i-1), _pointList.at(i));
  _length += b.Distance();
  _endLat = _pointList.at(i).lat();
  _endLon = _pointList.at(i).lon();
 }
 if(_pointList.count() > 0)
 {
  _startLat = _pointList.at(0).lat();
  _startLon = _pointList.at(0).lon();
 }
 _points = _pointList.count();
}

QString SegmentData::pointsString()
{
 QString ps;
 for(int i=0; i < _pointList.count(); i++)
 {
  if(i > 0) ps.append(",");
  LatLng pt = _pointList.at(i);
  ps.append(pt.str());
 }
 return ps;
}

void SegmentData::checkTracks()
{
 if(_tracks < 1 || _tracks > 2)
 {
  _bNeedsUpdate = true;
  if(_oneWay == "N")
   _tracks = 2;
  else
   _tracks = 1;
 }
}

double SegmentData::getLength() {
 double len = 0.0;
 if(_points >= 2)
 {
  for(int i=0; i< _points-1; i++)
  {
   len += SQL::instance()->distance(_pointList.at(i),_pointList.at(i+1));
  }
 }
 _length = len;
 return len;
}

#if 0
RouteInfo::RouteInfo(QObject *parent)
{
 route= -1;
 length = 0;
 segmentDataList = QList<SegmentData>();

}

RouteInfo::RouteInfo(RouteData rd)
{
 this->rd = rd;
 this->route = rd.route;
 this->routeName = rd.name;
 length = 0;
 segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"));
}

RouteInfo::RouteInfo(qint32 route, QString name, QString date)
{
 length = 0;
 this->route = route;
 this->routeName = name;
 segmentDataList = SQL::instance()->getRouteSegmentsInOrder(route, name, date);
}

RouteInfo::~RouteInfo()
{

}
#endif
SegmentInfo::SegmentInfo(const SegmentInfo& o)
{
 _description = o._description;
 _segmentId = o._segmentId;
 lineSegments = o.lineSegments;
 _points = o._points;
 _length = o._length;
 _startDate = o._startDate;
 _endDate = o._endDate;
 _bearing = o._bearing;
 _startLat = o._startLat;
 _startLon = o._startLon;
 _endLat = o._endLat;
 _endLon = o._endLon;
 _direction = o._direction;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _whichEnd = o._whichEnd;
 _routeType = o._routeType;
 _tractionType = o._tractionType;
 _streetName = o._streetName;
 _pointList= o._pointList;
 _bounds = o._bounds;
 _tracks = o._tracks;
 _bNeedsUpdate = o._bNeedsUpdate;
 //_next = o._next;
 _trackType = o._trackType;
 _location = o._location;
}
SegmentInfo::SegmentInfo(const SegmentData& o)
{
 _description = o._description;
 _segmentId = o._segmentId;
 //lineSegments = o.lineSegments;
 _points = o._points;
 _length = o._length;
 _startDate = o._startDate;
 _endDate = o._endDate;
 _bearing = o._bearing;
 _startLat = o._startLat;
 _startLon = o._startLon;
 _endLat = o._endLat;
 _endLon = o._endLon;
 _direction = o._direction;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _whichEnd = o._whichEnd;
 _routeType = o._routeType;
 _tractionType = o._tractionType;
 _streetName = o._streetName;
 _pointList= o._pointList;
 _bounds = o._bounds;
 _tracks = o._tracks;
 _bNeedsUpdate = o._bNeedsUpdate;
 //_next = o._next;
 _trackType = o._trackType;
 _location = o._location;
}

#if 1
QString SegmentInfo::toString()
{
 QString str;
 //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";

 if(_routeType < 0 || _routeType>= SegmentData::ROUTETYPES.count())
  _routeType = (RouteType)0;
 QString trackType = SegmentData::ROUTETYPES.at(_routeType);
 QString strSegment = QString("%1").arg(_segmentId);
  if (_tracks == 1)
      str = _description + QString("(single/%2) Seg=%1").arg(_segmentId).arg(_trackType);
  else
      str = _description + QString(" (double/%2) Seg=%1").arg(_segmentId).arg(_trackType);
 return str;

}
#endif

void SegmentInfo::addPoint(LatLng pt)
{
 _pointList.append(pt);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentInfo::insertPoint(int ptNum, LatLng pt)
{
 // insert ptNum AFTER that point.
 _pointList.insert(ptNum +1, pt);
 lineSegments = _pointList.count()-1;
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentInfo::movePoint(int ptNum, LatLng pt)
{
 if(ptNum >= _pointList.count()) return;
 _pointList.replace(ptNum, pt);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentInfo::deletePoint(int ptNum)
{
 if(ptNum < _pointList.count())
  _pointList.removeAt(ptNum);
 else
  qDebug() << QString("delete point %1 from segment %2 failed").arg(ptNum).arg(_segmentId);
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

void SegmentInfo::setPoints(QString sPoints)
{
 if(_length > 15.0)
  _bNeedsUpdate = true;
 _length = 0;
 QStringList sl = sPoints.split(",");
 if(sl.count()== 0 || ((sl.count()& 0x01) == 1))
     return;
 //bool bOk;
 for(int i=0; i < sl.count(); i+=2)
 {
  LatLng latLng = LatLng(sl.at(i).toDouble(), sl.at(i+1).toDouble());
  if(latLng.isValid())
  {
   _pointList.append(latLng);
   _bounds.updateBounds(latLng);
  }
 }
 for(int i = 1; i < _pointList.count(); i++)
 {
  Bearing b = Bearing(_pointList.at(i-1), _pointList.at(i));
  _length += b.Distance();
  _endLat = _pointList.at(i).lat();
  _endLon = _pointList.at(i).lon();
 }
 if(_pointList.count() > 0)
 {
  _startLat = _pointList.at(0).lat();
  _startLon = _pointList.at(0).lon();
 }
 if(_points != _pointList.count())
 {
  _points = _pointList.count();
  _bNeedsUpdate = true;
 }
 lineSegments=_pointList.count()-1;
}

QString SegmentInfo::pointsString()
{
 QString ps;
 for(int i=0; i < _pointList.count(); i++)
 {
  if(i > 0) ps.append(",");
  LatLng pt = _pointList.at(i);
  ps.append(pt.str());
 }
 return ps;
}

void SegmentInfo::checkTracks()
{
 if(_tracks < 1 || _tracks > 2)
 {
  _bNeedsUpdate = true;
#if 0
  if(oneWay == "N")
   tracks = 2;
  else
   tracks = 1;
#endif
 }
}

void SegmentInfo::displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst)
{
 QVariantList points, objArray;
 QList<RouteData> myArray ;
 QString routeNames = "no route";
 if(date != "")
 {
  myArray = SQL::instance()->getRoutes(_segmentId, date);
  if (myArray.count()== 0)
  {
   int i = 0;
   routeNames = "";
   //foreach (routeData rd in myArray)
   for(i=0; i<myArray.count(); i++)
   {
    RouteData rd = (RouteData)myArray.at(i);
    routeNames += rd.name;
    //i++;
    if (i+1 < myArray.count())
     routeNames += ",";
   }
  }
 }
 if (bClearFirst)
 {
  objArray << _segmentId;
  WebViewBridge::instance()->processScript("clearPolyline",objArray);
 }

 for(int i=0; i < _pointList.count(); i++)
 {
  points.append(((LatLng)_pointList.at(i)).lat());
  points.append(((LatLng)_pointList.at(i)).lon());
 }
 int dash = 0;
 if(_routeType == Incline)
  dash = 1;
 else if(_routeType == SurfacePRW)
  dash = 2;
 else if(_routeType == Subway)
  dash = 3;
 objArray.clear();
 objArray << _segmentId << routeNames<<_description << " " <<color<< _tracks << dash << _routeType << trackUsage << _pointList.count()*2 << points;
 WebViewBridge::instance()->processScript("createSegment", objArray);
}


Bounds::Bounds()
{
 _swPt = LatLng(90,180);
 _nePt = LatLng(-90,-180);
bBoundsValid = false;
}

Bounds::Bounds(LatLng sw, LatLng ne) :  QObject()
{
 _swPt = sw;
 _swLat = sw.lat();
 _swLon = sw.lon();
 _nePt = ne;
 _neLat = ne.lat();
 _neLon = ne.lon();
 if(sw.lon() < ne.lon() && sw.lat() < ne.lat())
 {
  bBoundsValid = true;
 }
 else
  bBoundsValid = false;
}

Bounds::Bounds(QString bnds): QObject()
{
 _swPt = LatLng(90,180);
 _nePt = LatLng(-90,-180);
 bBoundsValid = false;
 QStringList sl = bnds.split(",");
 if(sl.count()>=4)
 {
  bool ok;
  _swLon = sl.at(0).toDouble(&ok);
  if(!(ok && _swLon >= -180.0 && _swLon <= 180.0))
   return;
  _swLat = sl.at(1).toDouble(&ok);
  if(!(ok && _swLat >= -90.0 && _swLat <= 90.0))
   return;
  _neLon = sl.at(2).toDouble(&ok);
  if(!(ok && _neLon >= -180.0 && _neLon <= 180.0))
   return;
  _neLat = sl.at(3).toDouble(&ok);
  if(!(ok && _neLat >= -90.0 && _neLat <= 90.0))
   return;
  _swPt = LatLng(_swLat, _swLon);
  _nePt = LatLng(_neLat, _neLon);
  setBottomLeft(_swPt);
  setTopRight(_nePt);
  bBoundsValid = isValid();
 }
}
Bounds::~Bounds(){}
Bounds::Bounds(const Bounds &other) : QObject()
{
 _swPt = other._swPt;
 _nePt = other._nePt;
 _neLat = other._neLat;
 _neLon = other._neLon;
 _swLat = other._swLat;
 _swLon = other._swLon;
 setBottomLeft(_swPt);
 setTopRight(_nePt);
 bBoundsValid = other.bBoundsValid;
}

Bounds Bounds::fromPointList(QList<LatLng> list)
{
 Bounds b = Bounds();
 double neLat = b.nePt().lat();
 double neLon = b.nePt().lon();
 double swLat = b.swPt().lat();
 double swLon = b.swPt().lon();
 foreach(LatLng latLng, list)
 {
  if(latLng.lat() > neLat)
   neLat = latLng.lat();
  if(latLng.lat() < swLat)
   swLat = latLng.lat();

  if(latLng.lon() > neLon)
   neLon = latLng.lon();
  if(latLng.lon() < swLon)
   swLon = latLng.lon();
  b.setBottomLeft(LatLng(swLat, swLon));
  b.setTopRight(LatLng(neLat, neLon));
  return b;
 }
}

bool Bounds::isValid() {bBoundsValid = checkValid(); return bBoundsValid;}
bool Bounds::checkValid()
{
// if(_nePt.lon() == 0)
//  return false;
 if(!_swPt.isValid())
  return false;
 if(!_nePt.isValid())
  return false;
 if(_swPt.lat() < _nePt.lat() && _swPt.lon() < _nePt.lon())
  return true;
 return false;
}
bool Bounds::updateBounds(LatLng pt)
{
 if (pt.lat() < _swPt.lat())
  _swPt.setLat(pt.lat());
 if (pt.lat() > _nePt.lat())
  _nePt.setLat( pt.lat());
 if (pt.lon() < _swPt.lon())
  _swPt.setLon( pt.lon());
 if (pt.lon() > _nePt.lon())
  _nePt.setLon( pt.lon());
 bBoundsValid = true;
 return bBoundsValid;
}
bool Bounds::updateBounds(Bounds bnds)
{
 updateBounds(bnds.nePt());
 updateBounds(bnds.swPt());
 return bBoundsValid;
}

LatLng Bounds::swPt() const { return _swPt;}
LatLng Bounds::nePt() const { return _nePt;}

QString Bounds::toString()
{
 return QString("%1,%2, %3,%4").arg(_swPt.lon(),14,'g',11).arg(_swPt.lat(),14,'g',11).arg(_nePt.lon(),14,'g',11).arg(_nePt.lat(),14,'g',11);
}

bool Bounds::contains(const LatLng &p) const
{
// if((p.lon() >= _swPt.lon()) && (p.lon() <= _nePt.lon())
//    && (p.lat() >= _swPt.lat()) && (p.lat()<= _nePt.lat())) return true;
 if(!(p.lon() >= _swPt.lon())) return false;
 if(!(p.lon() <= _nePt.lon())) return false;
 if(!(p.lat() >= _swPt.lat())) return false;
 if( (p.lat() <= _nePt.lat())) return true;

 return false;
}

bool Bounds::contains(const Bounds &b) const
{
 if(this->contains(b._swPt) || this->contains(b._nePt) || b.contains(this->_swPt) || b.contains(this->_nePt))
  return true;
 return false;
}

LatLng Bounds::center() {
 //QRectF rect(_swPt.lon(),_nePt.lat(),(_nePt.lon() - _swPt.lon()),(_nePt.lat()-_swPt.lat()));
 QRectF rect(QPointF(_swPt.lon(),_nePt.lat()), QPointF(_swPt.lon(), _nePt.lat())); // top left pt, bottom right pt
 QPointF ctr(rect.center());
 return LatLng(ctr.y(), ctr.x());
}

void SegmentData::displaySegment(QString date, QString color, QString trackUsage, bool bClearFirst)
{
 QVariantList points, objArray;
 QList<RouteData> myArray ;
 QString routeNames = "no route";
 if(date != "")
 {
  myArray = SQL::instance()->getRoutes(_segmentId, date);
  if (myArray.count()== 0)
  {
   int i = 0;
   routeNames = "";
   //foreach (routeData rd in myArray)
   for(i=0; i<myArray.count(); i++)
   {
    RouteData rd = (RouteData)myArray.at(i);
    routeNames += rd.name;
    //i++;
    if (i+1 < myArray.count())
     routeNames += ",";
   }
  }
 }
 if (bClearFirst)
 {
  objArray << _segmentId;
  WebViewBridge::instance()->processScript("clearPolyline",objArray);
 }

 for(int i=0; i < pointList().count(); i++)
 {
  points.append(((LatLng)pointList().at(i)).lat());
  points.append(((LatLng)pointList().at(i)).lon());
 }
 int dash = 0;
 if(routeType() == Incline)
  dash = 1;
 else if(routeType() == SurfacePRW)
  dash = 2;
 else if(routeType() == Subway)
  dash = 3;
 objArray.clear();
 objArray << _segmentId << routeNames<<_description<<_oneWay<<color<< _tracks << dash << _routeType << trackUsage << _pointList.count()*2 << points;
 WebViewBridge::instance()->processScript("createSegment", objArray);
}
