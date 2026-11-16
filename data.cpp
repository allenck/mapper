#include "data.h"
#include "sql.h"
#include <QString>
#include "sql.h"
#include <QPair>
#include "exceptions.h"




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
    routeName = other.routeName;
    routeAlpha = other.routeAlpha;
    companyName = other.companyName;
    routeId = other.routeId;
}

CommentInfo::CommentInfo(const CommentInfo& other){
    commentKey = other.commentKey;
    tags = other.tags;
    comments = other.comments;
    usedByRoutes = other.usedByRoutes;
    //usedByStations = other.usedByStations;
    routesUsed = other.routesUsed;
    routeCount = other.routeCount;
    date = other.date;
    jRoutesListString = other.jRoutesListString;
    aRoutes = other.aRoutes;
    alphaRoute = other.alphaRoute;
    pos = other.pos;
    routeName = other.routeName;
    routeId = other.routeId;
    route = other.route;
    companyKey = other.companyKey;
    companyName = other.companyName;
    aRoutesList = other.aRoutesList;
}

QList<int> CommentInfo::toRoutesTable(QString routeList)
{
    QStringList sl = routeList.split(",");
    routesUsed.clear();
    foreach (QString s, sl) {
        int route = s.toInt();
        if(route > 0)
            routesUsed.append(route);
    }
    routeCount = routesUsed.count();
    return routesUsed;
}

QStringList CommentInfo::toAroutesList(QJsonArray aRoutes)
{
    QStringList list;
    QVariantList vl = aRoutes.toVariantList();
    foreach (QVariant v, vl) {
        list.append(v.toString());
    }
    return list;
}

void CommentInfo::populateARoutes(QString jRouteList)
{
    if(jRouteList.isEmpty() || !(jRouteList.startsWith('[') && jRouteList.endsWith("]")))
        return;
    QString listOfStrings = jRouteList.mid(1, jRouteList.length()-2); // strip off [ & ]
    QStringList sl = listOfStrings.split(",");
    foreach (QString s, sl) {
        if(s.startsWith("\"") && s.endsWith("\""))
        {
            QString s1 = s.mid(1, s.length()-2);
            aRoutes.append(s1);
            aRoutesList.append(s1);
        }
    }
}


/*static*/ QString CommentInfo::routesTableToString(QList<int> table)
{
    QString routeList;
    foreach (int route, table) {
        if(route < 1)
            continue;
        routeList.append(QString::number(route));
        routeList.append(",");
    }
    routeList.chop(1);
    return routeList;
}

QString CommentInfo::jRoutesTableToString(QList<QString> table){
    QString routeList = "[";
    foreach (QString aRoute, table) {
        routeList.append("\"" + aRoute + "\"");
        routeList.append(",");
        aRoutes.append(aRoute);
    }
    routeList.chop(1);
    routeList.append("]");
    aRoutes = QJsonArray::fromStringList(table);
    return routeList;
}

QString CommentInfo::aRoutesToString()
{
    QString routeList = "[";
    QVariantList vl = aRoutes.toVariantList();
    foreach (QVariant aRoute, vl) {
        routeList.append("\"" + aRoute.toString() + "\"");
        routeList.append(",");
    }
    routeList.chop(1);
    routeList.append("]");
    return routeList;
}

