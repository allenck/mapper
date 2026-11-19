#include "routeinfo.h"
#ifndef ROUTENAMEWIDGETPLUGIN
#include "segmentdata.h"
#endif
RouteInfo::RouteInfo()
{
    _route= -1;
    length = 0;
    segmentDataList = QList<SegmentData*>();
}

// RouteInfo::RouteInfo(const RouteData& rd)
// {
//     //this->rd = rd;
//     this->_route = rd.route();
//     this->_routeName = rd.routeName();
//     this->_routeId = rd.routeId();
//     length = 0;
//     segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route(), rd.routeName(),
//                                                                rd.companyKey(), rd.endDate());
// }
#ifndef ROUTENAMEWIDGETPLUGIN
RouteInfo::RouteInfo(const SegmentData& sd)
{
    //this->rd = rd;
    this->_route = sd.route();
    this->_routeName = sd.routeName();
    this->_startDate = sd.startDate();
    this->_endDate = sd.endDate();
    this->_alphaRoute = sd.alphaRoute();
    this->_companyKey = sd.companyKey();
    this->_routeId = sd.routeId();
    length = 0;
    // segmentDataList = SQL::instance()->getRouteSegmentsInOrder(rd.route(), rd.routeName(),
    //                                                            rd.companyKey(), rd.endDate());
}
#endif
RouteInfo::RouteInfo(qint32 route, QString name, QDate startDate, QDate endDate, int companyKey,
                     QString alphaRoute, int routeId)
{
    length = 0;
    this->_route = route;
    this->_routeName = name;
    this->_startDate = startDate;
    this->_endDate = endDate;
    this->_companyKey = companyKey;
    this->_alphaRoute = alphaRoute;
    this->_routeId = routeId;
    //segmentDataList = SQL::instance()->getRouteSegmentsInOrder(route, name, 0, endDate);
}


RouteInfo::RouteInfo(const RouteInfo &o)
{
    _route=o._route;
    _routeName= o._routeName;
    _canonicalName = o._canonicalName;
    _tractionType=o._tractionType;
    length = o.length;
    _startDate = o._startDate;
    _endDate = o._endDate;
    _companyKey=o._companyKey;
    _alphaRoute = o._alphaRoute;
    _baseRoute= o._baseRoute;
    _routePrefix = o._routePrefix;
    _companyMnemonic = o._companyMnemonic;
    _routeId = o._routeId;
    _description = o._description;
}

QString RouteInfo::description(){
    return _description;
}
void RouteInfo::setDescription(QString description)
{
    _description = description;
}

const QString RouteInfo::toString()
{
    bool isNumeric;
    int num = _alphaRoute.toInt(&isNumeric);
    QString str;
    if(isNumeric && _routePrefix.trimmed().isEmpty())
        str = _companyMnemonic + " " + _alphaRoute + " " + _routeName + " " + _startDate.toString("yyyy-MM-dd")+ "->"+ _endDate.toString("yyyy-MM-dd");
    else
        str = _companyMnemonic + " " + _alphaRoute+"(" +QString::number(_route)+ ") " + _routeName + " " + _startDate.toString("yyyy/MM/dd")+ "->"+ _endDate.toString("yyyy-MM-dd");

    return str;
}
QList<QPair<int, QString> > RouteInfo::setSeqList(QString txt)
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

QString RouteInfo::seqToString()
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

