#include "data.h"
#include "sql.h"
#include <QString>
#include "webviewbridge.h"
#include "sql.h"
#include <QPair>

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
 _route=-1;
 _companyKey = -1;
 _lineKey = -1;
 _trackUsage = " ";
// sd = SegmentInfo();

}

RouteData::~RouteData()
{
}

RouteData::RouteData(const RouteData& o)
{
 _route = o._route;
 _alphaRoute = o._alphaRoute;;
 //baseRoute = o.baseRoute;
 _name = o._name;
 //defaultDate = o.defaultDate;
 _dateBegin = o._dateBegin;
 _dateEnd = o._dateEnd;
 _companyKey = o._companyKey;
 _lineKey = o._lineKey;
 _tractionType = o._tractionType;
 _direction = o._direction;
 _normalEnter = o._normalEnter;
 _normalLeave = o._normalLeave;
 _reverseEnter = o._reverseEnter;        // Not defined for one Way
 _reverseLeave = o._reverseLeave;        // Not defined for one Way
 _oneWay = o._oneWay;
 _next= o._next;
 _prev= o._prev;
 _nextR= o._nextR;
 _prevR= o._prevR;
 _trackUsage = o._trackUsage;
 _tracks = o._tracks;
 _seqList = o._seqList;
 _baseRoute = o._baseRoute;
 _doubleDate = o._doubleDate;
 _companyMnemonic = o._companyMnemonic;
 _routeId = o._routeId;
}

RouteData::RouteData(const SegmentData& o)
{
 _route = o.route();
 _alphaRoute = o.alphaRoute();
 //baseRoute = o.baseRoute;
 _name = o.routeName();
 //defaultDate = o.defaultDate;
 _dateBegin = o.startDate();
 _dateEnd = o.endDate();
 _companyKey = o.companyKey();
 _lineKey = o.segmentId();
 _tractionType = o.tractionType();
 _baseRoute = o.baseRoute();
 _doubleDate = o.doubleDate();
 _routeId = o.routeId();
}


QList<QPair<int, QString> > RouteData::setSeqList(QString txt)
{
 _seqList.clear();
 QStringList sl = txt.split(",");
 for(int i=0; i< sl.count(); i+=2)
 {
  QPair<int, QString> pair(sl.at(i).toInt(), sl.at(i+1));
  _seqList.append(pair);
 }
 return _seqList;
}

QString RouteData::seqToString()
{
 QString text;
 for(QPair<int,QString> pair : _seqList)
 {
  if(!text.isEmpty())
   text.append(",");
  text.append(QString::number(pair.first));
  text.append(",");
  text.append(pair.second);
 }

 return text;
}

const QString RouteData::toString()
{
 bool isNumeric;
 int num = _alphaRoute.toInt(&isNumeric);
 QString str;
 if(isNumeric && _routePrefix.trimmed().isEmpty())
  str = _companyMnemonic + " " + _alphaRoute + " " + _name + " " + _dateBegin.toString("yyyy/MM/dd")+ "->"+ _dateEnd.toString("yyyy/MM/dd");
 else
  str = _companyMnemonic + " " + _alphaRoute+"(" +QString::number(_route)+ ") " + _name + " " + _dateBegin.toString("yyyy/MM/dd")+ "->"+ _dateEnd.toString("yyyy/MM/dd");

 return str;
}

bool RouteData::operator==(const RouteData& o )
{
 if(o._route == _route && o._name == _name && o._dateEnd == _dateEnd
    && o._dateBegin == _dateBegin)
  return true;
 return false;
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
 SegmentData::SegmentData(int route, QString name, int segmentId,
                            QDate startDate, QDate endDate)
{
 _route = route;
 _routeName = name;
 _segmentId = segmentId;
 _dateBegin = startDate;
 _dateEnd = endDate;
}

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
 _newerName = o._newerName;
 _description = o._description;
 _dateBegin = o._dateBegin;
 _dateEnd = o._dateEnd;
 _direction  = o._direction;
 _bearing = o._bearing;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _pointList = o._pointList;
 if(_points == 0 && pointList().count() > 0 )
  _points = pointList().count();
 _next = o._next;
 _prev = o._prev;
 _nextR = o._nextR;
 _prevR = o._prevR;
 _streetId = o._streetId;
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
 _rowid = o._rowid;
 _segRowid = o._segRowid;
 _routeId = o._routeId;
}

SegmentData::SegmentData(const RouteData& o)
{
 _segmentId = o._lineKey;
 _tracks = o._tracks;
 _routeType = o._routeType;
// _startLat = o._startLat;
// _startLon = o._startLon;
// _endLat = o._endLat;
// _endLon = o._endLon;
// _length = o._length;;
// _points = o._points;
// _streetName = o._streetName;
 _description = o._name;
 _dateBegin = o._dateBegin;
 _dateEnd = o._dateEnd;
 _direction  = o._direction;
// _bearing = o._bearing;
// _bearingStart = o._bearingStart;
// _bearingEnd = o._bearingEnd;
// _pointList = o._pointList;
 if(_points == 0 && pointList().count() > 0 )
  _points = pointList().count();
 _next = o._next;
 _prev = o._prev;
 _nextR = o._nextR;
 _prevR = o._prevR;

 _sequence = o._sequence;
 _returnSeq = o._returnSeq;
 _reverseEnter = o._reverseEnter;
 _reverseLeave = o._reverseLeave;
 _normalEnter = o._normalEnter;
 _normalLeave = o._normalLeave;
 _trackUsage = o._trackUsage;
 _tractionType = o._tractionType;
 //_bNeedsUpdate = o._bNeedsUpdate;
 _oneWay = o._oneWay;
 _direction = o._direction;
 //_bounds = o._bounds;
 _route = o._route;
 _companyKey = o._companyKey;
 _routeName = o._name;
 //_location = o._location;
 _alphaRoute = o._alphaRoute;
 //_whichEnd = o._whichEnd;
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
 _newerName = o._newerStreetName;
 _description = o._description;
 //_startDate = o._startDate;
 //_endDate = o._endDate;
 _segmentDateStart = o._dateBegin;
 _segmentDateEnd = o._dateEnd;
 _dateDoubled = o._dateDoubled;
 _direction  = o._direction;
 _bearing = o._bearing;
 _bearingStart = o._bearingStart;
 _bearingEnd = o._bearingEnd;
 _pointList = o._pointList;
 if(_points == 0 && pointList().count() > 0 )
  _points = pointList().count();
 _location = o._location;
 _whichEnd = o._whichEnd;
 _streetId = o._streetId;
 _segRowid = o._rowid;
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
QString SegmentData::reverseDescription()
{
 QString desc = _description;
 QStringList sl1 = desc.split(",");
 QStringList sl2;
 QString separator;
 if(sl1.at(0).length()>0 && sl1.count()==2)
 {
  if(sl1.at(1).contains(" to "))
   separator=" to ";
  else if(sl1.at(1).contains("-"))
      separator = "-";
  else
   separator = " zur "; // German
  sl2 = sl1.at(1).split(separator);
  if(sl2.count()==2)
  {
   return sl1.at(0) +","+sl2.at(1).trimmed()+" " + separator+" " + sl2.at(0).trimmed();
  }
 }
 return "reverse: "+ _description;
}

void SegmentData::updateRouteInfo(RouteData rd){
 _route = rd.route();
 _routeId = rd.routeId();
 _routeName = rd.routeName();
 _dateBegin = rd.startDate();
 _dateEnd = rd.endDate();
 _companyKey = rd.companyKey();
 _tractionType = rd.tractionType();
 _baseRoute = rd.baseRoute();
}


/*static*/ QStringList SegmentData::ROUTETYPES =
  QStringList() << "Surface" << "Surface PRW" << "Rapid Transit"
                << "Subway" << "Rail"  << "Incline"<< "MagLev"
                << "Elevated" << "Other";

void SegmentData::addPoint(LatLng pt)
{
 if(_pointList.count()> 0)
 {
   if(_pointList.at(_pointList.count()-1)== pt)
    throw IllegalArgumentException(" duplicate point");
 }
 _pointList.append(pt);
 //SQL sql;
 calculate();
 SQL::instance()->updateSegment(this);
}

void SegmentData::insertPoint(int ptNum, LatLng pt)
{
 // insert ptNum AFTER that point.
 _pointList.insert(ptNum +1, pt);
 //lineSegments = pointList.count()-1;
 //SQL sql;
 calculate();
 SQL::instance()->updateSegment(this);
}

void SegmentData::movePoint(int ptNum, LatLng pt)
{
 if(ptNum >= _pointList.count()) return;
 _pointList.replace(ptNum, pt);
 //SQL sql;
 calculate();
 SQL::instance()->updateSegment(this);
}

void SegmentData::deletePoint(int ptNum)
{
 if(ptNum < _pointList.count())
  _pointList.removeAt(ptNum);
 else
  qDebug() << QString("delete point %1 from segment %2 failed").arg(ptNum).arg(_segmentId);
 //SQL sql;
 calculate();
 SQL::instance()->updateSegment(this);
}

void SegmentData::setPoints(QList<LatLng>points)
{
    _pointList = points;
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
 _points = _pointList.count();
 calculate();
}

// calculate Bounds. Bearings and length
void SegmentData::calculate()
{
 if(_points >= 2)
 {
  //_length += _bearing.Distance();
  _endLat = _pointList.at(_points-1).lat();
  _endLon = _pointList.at(_points-1).lon();
 }
 if(_pointList.count() > 0)
 {
  _startLat = _pointList.at(0).lat();
  _startLon = _pointList.at(0).lon();
 }
 if(_points >= 2)
 {
  _bearing = Bearing(_pointList.at(0), _pointList.at(_points-1));
  _bearingStart = Bearing(_startLat, _startLon, _pointList.at(1).lat(), _pointList.at(1).lon());
  _bearingEnd =  Bearing(_pointList.at(_points-2).lat(), _pointList.at(_points-2).lon(), _endLat, _endLon);
 _length = getLength();
  bounds();
  direction();
 }
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

// update common fields
void SegmentData::update(const SegmentInfo& si)
{
 if(_segmentId == si._segmentId)
 {
  _streetName = si._streetName;
  _description = si._description;
  _tracks = si._tracks;
  _location = si._location;
  _routeType = si._routeType;
  _dateDoubled = si._dateDoubled;
 }
}

RouteInfo::RouteInfo(QObject *parent)
{
 route= -1;
 length = 0;
 segmentDataList = QList<SegmentData*>();

}

RouteInfo::RouteInfo(RouteData rd)
{
 this->rd = rd;
 this->route = rd.route();
 this->routeName = rd.routeName();
 this->_routeId = rd.routeId();
 length = 0;
 segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route(), rd.routeName(),
                                                            rd.companyKey(), rd.endDate());
}
RouteInfo::RouteInfo(const SegmentData& sd)
{
    //this->rd = rd;
    this->route = sd.route();
    this->routeName = sd.routeName();
    this->startDate = sd.startDate();
    this->endDate = sd.endDate();
    this->alphaRoute = sd.alphaRoute();
    this->companyKey = sd.companyKey();
    this->_routeId = sd.routeId();
    length = 0;
    // segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route(), rd.routeName(),
    //                                                            rd.companyKey(), rd.endDate());
}

RouteInfo::RouteInfo(qint32 route, QString name, QDate startDate, QDate endDate, int companyKey, QString alphaRoute, int routeId)
{
 length = 0;
 this->route = route;
 this->routeName = name;
 this->startDate = startDate;
 this->endDate = endDate;
 this->companyKey = companyKey;
 this->alphaRoute = alphaRoute;
 this->_routeId = routeId;
 //segmentDataList = SQL::instance()->getRouteSegmentsInOrder(route, name, 0, endDate);
}


RouteInfo::RouteInfo(RouteInfo& o)
{
 route=o.route;
 routeName= o.routeName;
 tractionType=o.tractionType;
 length = o.length;
 startDate = o.startDate;
 endDate = o.endDate;
 companyKey=o.companyKey;
 alphaRoute = o.alphaRoute;
 baseRoute= o.baseRoute;
 routePrefix = o.routePrefix;
 companyMnemonic = o.companyMnemonic;
 _routeId = o._routeId;
}

SegmentInfo::SegmentInfo(const SegmentInfo& o)
{
 _description = o._description;
 _segmentId = o._segmentId;
 _points = o._points;
 _length = o._length;
 _dateBegin = o._dateBegin;
 _dateEnd = o._dateEnd;
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
 _newerStreetName = o._newerStreetName;
 _pointList= o._pointList;
 _bounds = o._bounds;
 _tracks = o._tracks;
 _bNeedsUpdate = o._bNeedsUpdate;
 _next = o._next;
 _trackType = o._trackType;
 _location = o._location;
 _dateDoubled = o._dateDoubled;
 _streetId = o._streetId;
 _rowid = o._rowid;
}

SegmentInfo::SegmentInfo(const SegmentData& o)
{
 _description = o._description;
 _segmentId = o._segmentId;
 //lineSegments = o.lineSegments;
 _points = o._points;
 _length = o._length;
 _dateBegin = o._segmentDateStart;
 _dateEnd = o._segmentDateEnd;
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
 _newerStreetName = o._newerName;
 _pointList= o._pointList;
 _bounds = o._bounds;
 _tracks = o._tracks;
 _bNeedsUpdate = o._bNeedsUpdate;
 _next = o._next;
 _routeType = o._routeType;
 _location = o._location;
 _dateDoubled = o._dateDoubled;
 _streetId = o._streetId;
}

#if 1
QString SegmentInfo::toString() const
{
 QString str;
 //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 RouteType rt = _routeType;
 if(rt < 0 || rt>= SegmentData::ROUTETYPES.count())
  rt = (RouteType)0;
 QString trackType = SegmentData::ROUTETYPES.at(rt);
 QString strSegment = QString("%1").arg(_segmentId);
  if (_tracks == 1)
      str = _description + QString("(single/%2) Seg=%1").arg(_segmentId).arg(_trackType);
  else
      str = _description + QString(" (double/%2) Seg=%1").arg(_segmentId).arg(_trackType);
 return str;

}
#endif

void SegmentInfo::addPoint(LatLng pt, bool updateDb)
{
 _pointList.append(pt);
 //SQL sql;
 if(updateDb)
  SQL::instance()->updateSegment(this);
}

void SegmentInfo::insertPoint(int ptNum, LatLng pt)
{
 // insert ptNum AFTER that point.
 _pointList.insert(ptNum +1, pt);
 //lineSegments = _pointList.count()-1;
 //SQL sql;
 SQL::instance()->updateSegment(this);
}

QList<LatLng> SegmentInfo::movePoint(int ptNum, LatLng pt)
{
  if(ptNum >= _pointList.count())
        return _pointList;
  _pointList.replace(ptNum, pt);
  //SQL sql;
  SQL::instance()->updateSegment(this);
  return _pointList;
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
 //lineSegments=_pointList.count()-1;
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

QString SegmentInfo::direction()
{

 if(_direction.isEmpty())
 {
  _direction = bearing().strDirection();
 }
 return _direction;
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
    routeNames += rd.routeName();
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
 int tracks = _tracks;
 //if(_tracks == 2 && _doubleDate.isValid() &&  QDate::fromString(date, "yyyy/MM/dd") < _doubleDate)
 if(tracks == 2 && _dateDoubled.isValid() && _dateBegin < _dateDoubled)
 {
  tracks = 1;
  trackUsage = ' ';
 }
 if(_routeType == Incline)
  dash = 1;
 else if(_routeType == SurfacePRW)
  dash = 2;
 else if(_routeType == Subway)
  dash = 3;
 if(trackUsage.isEmpty()) // fix for MySql not storing field correctly
  trackUsage =" ";
 objArray.clear();
 objArray << _segmentId << routeNames<<_description << "N" << true <<color<< tracks
          << dash << _routeType << trackUsage << points.count();
 objArray.append(points);
 WebViewBridge::instance()->processScript("createSegment", objArray);
}

QString SegmentInfo::reverseDescription()
{
 QString desc = _description;
 QStringList sl1 = desc.split(",");
 QStringList sl2;
 QString separator;
 if(sl1.at(0).length()>0 && sl1.count()==2)
 {
  if(sl1.at(1).contains(" to "))
   separator=" to";
  else
   separator = " zur "; // German
  sl2 = sl1.at(1).split(separator);
  if(sl2.count()==2)
  {
   return sl1.at(0) +","+sl2.at(1) + separator + sl2.at(0);
  }
 }
 return "reverse: "+ _description;
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
// QPoint topLeft(_swPt.lon(), _nePt.lat());
// QPoint bottomRight(_nePt.lon(), _swPt.lat());
 QRectF rect(QPointF(_swPt.lon(), _nePt.lat()),
             QPointF(_nePt.lon(), _swPt.lat())); // top left pt, bottom right pt
 QPointF ctr(rect.center());
 return LatLng(ctr.y(), ctr.x());
}

bool Bounds::intersects(Bounds o)
{
 if(!isValid() || !o.isValid())
  return false;
 QRectF thisRect(QPoint(_swPt.lon(), _nePt.lat()), QPoint(_nePt.lon(), _swPt.lat()));
 QRectF otherRect(QPoint(o._swPt.lon(), o._nePt.lat()), QPoint(o._nePt.lon(), o._swPt.lat()));
 return thisRect.intersects(otherRect);
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
    routeNames += rd.routeName();
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
 int tracks = _tracks;
 if(tracks == 2 && _dateDoubled.isValid() && _dateBegin < _dateDoubled)
 {
  tracks = 1;
  _trackUsage = " ";
 }

 int dash = 0;
 if(routeType() == Incline)
  dash = 1;
 else if(routeType() == SurfacePRW)
  dash = 2;
 else if(routeType() == Subway)
  dash = 3;
 objArray.clear();
 objArray << _segmentId << routeNames <<_description << _oneWay
          << Configuration::instance()->bDisplaySegmentArrows << color
          << tracks << dash
          << _routeType << trackUsage << points.count();
 objArray.append(points);
 WebViewBridge::instance()->processScript("createSegment", objArray);
}

void SegmentData::displaySegment( QString color,  bool bClearFirst)
{
    QVariantList points, objArray;
    QList<RouteData> myArray ;
    QString routeNames = "no route";
    if(_dateBegin.isValid())
    {
        myArray = SQL::instance()->getRoutes(_segmentId, _dateBegin.toString("yyyy/MM/dd"));
        if (myArray.count()== 0)
        {
            int i = 0;
            routeNames = "";
            //foreach (routeData rd in myArray)
            for(i=0; i<myArray.count(); i++)
            {
                RouteData rd = (RouteData)myArray.at(i);
                routeNames += rd.routeName();
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
    int tracks = _tracks;
    if(tracks == 2 && _dateDoubled.isValid() && _dateBegin < _dateDoubled)
    {
        tracks = 1;
        _trackUsage = " ";
    }

    int dash = 0;
    if(routeType() == Incline)
        dash = 1;
    else if(routeType() == SurfacePRW)
        dash = 2;
    else if(routeType() == Subway)
        dash = 3;
    objArray.clear();
    objArray << _segmentId << routeNames <<_description << _oneWay
             << Configuration::instance()->bDisplaySegmentArrows << color
             << tracks << dash
             << _routeType << _trackUsage << points.count();
    objArray.append(points);
    WebViewBridge::instance()->processScript("createSegment", objArray);
}

RouteSeq::RouteSeq(const RouteSeq& o){
 _seqList= o._seqList;
 _firstSegment = o._firstSegment;
 _whichEnd = o._whichEnd;
 _rd = o._rd;
 _lastUpdate = o._lastUpdate;
}
StationInfo::StationInfo(const StationInfo& o)
{
 routes = o.routes;
 stationName =o.stationName;
 stationSuffix = o.stationSuffix; // if multiple stops exist at say the intersection of two lines, each stop can be identified with a suffix code.
 latitude = o.latitude;       // latitude of the station
 longitude = o.longitude;      // longitude of the station.
 stationKey = o.stationKey;
 infoKey = o.infoKey;        // key to the comments table containing descritptive text for the station.
 startDate = o.startDate;
 endDate = o.endDate;
 routeType = o.routeType;
 markerType = o.markerType;
 segmentId = o.segmentId;
 segments = o.segments;
}

StreetInfo::StreetInfo(const StreetInfo& o)
{
    street = o.street;
    olderName = o.olderName;
    newerName = o.newerName;
    startLatLng = o.startLatLng;
    endLatLng = o.endLatLng;
    length = o.length;
    dateStart = o.dateStart;
    dateEnd= o.dateEnd;
    segments = o.segments;
    comment = o.comment;
    location = o.location;
    streetId = o.streetId;
    bounds = o.bounds;
    sequence = o.sequence;
    dateSort = o.dateSort;
    rowid = o.rowid;

}
QList<int> StreetInfo::setSegments(const QString text)
{
    QStringList sl = text.split(",");
    QList<int> list;
    foreach (QString s, sl) {
        if(!list.contains(s.toInt()))
            list.append(s.toInt());
    }
    return list;
}

QString StreetInfo::segmentsToString()
{
    QString result;
    foreach (int i, segments) {
        if(i > 0)
            result.append(QString::number(i)+",");
    }
    result.chop(1);
    return result;
}

void StreetInfo::updateSegmentInfo(SegmentInfo si)
{
    if(!dateStart.isValid() || si.startDate() < dateStart)
        dateStart = si.startDate();
    if(!dateEnd.isValid() || si.endDate() > dateEnd)
        dateEnd = si.endDate();

    bounds.updateBounds(LatLng(si.startLat(), si.startLon()));
    bounds.updateBounds(LatLng(si.endLat(), si.endLon()));
    if(bounds.isValid())
    {
        startLatLng = bounds.swPt();
        endLatLng = bounds.nePt();
        length = SQL::instance()->Distance(startLatLng.lat(), startLatLng.lon(),
                                           endLatLng.lat(), endLatLng.lon());
        //qDebug() << bounds.toString();
    }
}
void StreetInfo::updateBounds()
{
    if(startLatLng.isValid() && endLatLng.isValid())
    {
        bounds.updateBounds(startLatLng);
        bounds.updateBounds(endLatLng);
        if(bounds.isValid())
        {
            startLatLng = bounds.swPt();
            endLatLng = bounds.nePt();
            length = SQL::instance()->Distance(startLatLng.lat(), startLatLng.lon(),
                                               endLatLng.lat(), endLatLng.lon());
            //qDebug() << bounds.toString();
        }

    }
}


bool StreetInfo::inDateRange(QDate dt)
{
    if( dt.isNull())
        throw IllegalArgumentException("date is null!");
    if(dateStart.isNull()  || dateEnd.isNull())
        return false;
    if(dateEnd < dateStart)
        throw IllegalArgumentException("end date not after start!");

    if(dt >= dateStart && dt <= dateEnd)
        return true;
    return false;
}

bool StreetInfo::encompasses(StreetInfo sti)
{
    return sti.street == street && sti.location == location && inDateRange(sti.dateStart);
}

QString StreetInfo::toString()
{
    return street + " " + location + " streetId:" + QString::number(streetId);
}

RouteComments::RouteComments(const RouteComments& other)
{
    route = other.route;
    date = other.date;
    commentKey = other.commentKey;
    ci = other.ci;
    pos = other.pos;
    companyKey = other.companyKey;
    name = other.name;
    routeAlpha = other.routeAlpha;
}

CommentInfo::CommentInfo(const CommentInfo& other){
    commentKey = other.commentKey;
    tags = other.tags;
    comments = other.comments;
    usedByRoutes = other.usedByRoutes;
    usedByStations = other.usedByStations;
}
