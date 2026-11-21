#include "routedata.h"
#include "segmentdata.h"
#include "myapplication.h"
RouteData::RouteData()
{
    _route=-1;
    _companyKey = -1;
    _lineKey = -1;
    _trackUsage = " ";
    _dtFormat = ((MyApplication*)QApplication::instance())->dateFormat();

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
    _coName = o._coName;
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
        str = _companyMnemonic + " " + _alphaRoute + " " + _name + " " + _dateBegin.toString(_dtFormat)+ "->"+ _dateEnd.toString(_dtFormat);
    else
        str = _companyMnemonic + " " + _alphaRoute+"(" +QString::number(_route)+ ") " + _name + " " + _dateBegin.toString(_dtFormat)+ "->"+ _dateEnd.toString(_dtFormat);

    return str;
}

bool RouteData::operator==(const RouteData& o )
{
    if(o._route == _route && o._name == _name && o._dateEnd == _dateEnd
        && o._dateBegin == _dateBegin)
        return true;
    return false;
}

