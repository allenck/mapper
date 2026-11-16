#include "segmentinfo.h"
#include "segmentdata.h"
#ifndef SEGMENTSELECTIONPLUGIN
#include "sql.h"
#include "webviewbridge.h"
#endif
SegmentInfo::SegmentInfo()
{
    _segmentId = -1;
    _pointList = QList<LatLng>();
    _bounds = Bounds();
    _bNeedsUpdate = false;
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
    _streetSeq = o._streetSeq;
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
    _newerStreetName = o._newerStreetName;
    _pointList= o._pointList;
    _bounds = o._bounds;
    _tracks = o._tracks;
    _bNeedsUpdate = o._bNeedsUpdate;
    _next = o._next;
    _routeType = o._routeType;
    _location = o._location;
    _dateDoubled = o._dateDoubled;
    _streetId = o._streetId;
    _streetSeq = o._streetSeq;
}
/*static*/ QStringList SegmentInfo::ROUTETYPES =
    QStringList() << "Surface" << "Surface PRW" << "Rapid Transit"
                  << "Subway" << "Rail"  << "Incline"<< "MagLev"
                  << "Elevated" << "Other";

QString SegmentInfo::toString() const
{
    QString str;
    //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
    RouteType rt = _routeType;
    if(rt < 0 || rt>= SegmentInfo::ROUTETYPES.count())
        rt = (RouteType)0;
    QString trackType = SegmentInfo::ROUTETYPES.at(rt);
    QString strSegment = QString("%1").arg(_segmentId);
    if (_tracks == 1)
        str = _description + QString("(single/%2) Seg=%1").arg(_segmentId).arg(_trackType);
    else
        str = _description + QString(" (double/%2) Seg=%1").arg(_segmentId).arg(_trackType);
    return str;

}

void SegmentInfo::addPoint(LatLng pt, bool updateDb)
{
    _pointList.append(pt);
    //SQL sql;
#ifndef SEGMENTSELECTIONPLUGIN
    if(updateDb)
        SQL::instance()->updateSegment(this);
#endif
}

void SegmentInfo::insertPoint(int ptNum, LatLng pt)
{
    // insert ptNum AFTER that point.
    _pointList.insert(ptNum +1, pt);
    //lineSegments = _pointList.count()-1;
    //SQL sql;
#ifndef SEGMENTSELECTIONPLUGIN
    SQL::instance()->updateSegment(this);
#endif
}

QList<LatLng> SegmentInfo::movePoint(int ptNum, LatLng pt)
{
    if(ptNum >= _pointList.count())
        return _pointList;
    _pointList.replace(ptNum, pt);
    //SQL sql;
#ifndef SEGMENTSELECTIONPLUGIN
    SQL::instance()->updateSegment(this);
#endif
    return _pointList;
}

void SegmentInfo::deletePoint(int ptNum)
{
    if(ptNum < _pointList.count())
        _pointList.removeAt(ptNum);
    else
        qDebug() << QString("delete point %1 from segment %2 failed").arg(ptNum).arg(_segmentId);
    //SQL sql;
#ifndef SEGMENTSELECTIONPLUGIN
    SQL::instance()->updateSegment(this);
#endif
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

#ifndef SEGMENTSELECTIONPLUGIN

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
    objArray << _segmentId << routeNames<<_description << "N" << Configuration::instance()->bDisplaySegmentArrows <<color<< tracks
             << dash << _routeType << trackUsage << points.count();
    objArray.append(points);
    WebViewBridge::instance()->processScript("createSegment", objArray);
}
#endif

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
