#include "segmentdata.h"
#include "exceptions.h"
#include "routedata.h"
#include "sql.h"
#include "webviewbridge.h"

SegmentData::SegmentData()
    : SegmentInfo()
{ _segmentId = -1;
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
    _newerStreetName = o._newerStreetName;
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
    _streetSeq = o._streetSeq;
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
    _newerStreetName = o._newerStreetName;
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
    _streetSeq = o._streetSeq;
    _segRowid = o._rowid;
}
// QString SegmentData::toString()
// {
//     QString str;

//     if(_routeType < 0 || _routeType>= ROUTETYPES.count())
//         _routeType = (RouteType)0;
//     QString trackType = ROUTETYPES.at(_routeType);
//     QString strSegment = QString("%1").arg(_segmentId);
//     if (_tracks == 1)
//         str = _description + QString("(single/%2) Seg=%1").arg(_segmentId).arg(trackType);
//     else
//         str = _description + QString(" (double/%2) Seg=%1").arg(_segmentId).arg(trackType);
//     return str;
// }

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

// void SegmentData::setPoints(QList<LatLng>points)
// {
//     _pointList = points;
// }

// void SegmentData::setPoints(QString sPoints)
// {
//     if(_length > 15.0)
//         _bNeedsUpdate = true;
//     _length = 0;
//     QStringList sl = sPoints.split(",");
//     if(sl.count()== 0 || ((sl.count()& 0x01) == 1)) return;
//     //bool bOk;
//     for(int i=0; i < sl.count(); i+=2)
//     {
//         LatLng latLng = LatLng(sl.at(i).toDouble(), sl.at(i+1).toDouble());
//         if(latLng.isValid())
//         {
//             _pointList.append(latLng);
//             bounds().updateBounds(latLng);
//         }
//     }
//     _points = _pointList.count();
//     calculate();
// }

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

// QString SegmentData::pointsString()
// {
//     QString ps;
//     for(int i=0; i < _pointList.count(); i++)
//     {
//         if(i > 0) ps.append(",");
//         LatLng pt = _pointList.at(i);
//         ps.append(pt.str());
//     }
//     return ps;
// }

// void SegmentData::checkTracks()
// {
//     if(_tracks < 1 || _tracks > 2)
//     {
//         _bNeedsUpdate = true;
//         if(_oneWay == "N")
//             _tracks = 2;
//         else
//             _tracks = 1;
//     }
// }

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
SegmentData::SegmentData(int route, QString name, int segmentId,
                         QDate startDate, QDate endDate)
{
    _route = route;
    _routeName = name;
    _segmentId = segmentId;
    _dateBegin = startDate;
    _dateEnd = endDate;
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
