#ifndef ROUTEINFO_H
#define ROUTEINFO_H

#include <qdatetime.h>
#include <qobject.h>
#include "routetype.h"

class SegmentData;
//class RouteData;
class RouteInfo
{
public:
    explicit RouteInfo();
    RouteInfo(qint32 route, QString name, QDate startDate, QDate endDate, int companyKey,
              QString alphaRoute, int routeId = -1);
    ~RouteInfo() {}
    //RouteInfo(const RouteData& rd);
    RouteInfo(const RouteInfo& o);
#ifndef ROUTENAMEWIDGETPLUGIN
    RouteInfo(const SegmentData &o);
#endif
    const QString toString();

    void addSequence(QPair<int, QString> pair) {_seqList.append(pair);}
    void setRouteId(int routeId){_routeId = routeId;}
    int routeId()const {return _routeId;}
    void setRouteName(QString name) {_routeName = name;}
    QString description();
    void setDescription(QString description);
    int route() {return _route;}
    void setRoute(int route){_route = route;}
    QDate startDate() const  {return _startDate;}
    void setStartDate(QDate startDate){_startDate = startDate;}
    QDate endDate()const {return _endDate;}
    void setEndDate(QDate endDate){_endDate = endDate;}
    int companyKey() {return _companyKey;}
    void setCompanyKey(int companyKey){_companyKey = companyKey;}
    QString companyName(){return _companyName;}
    void setCompanyName(QString companyName) {_companyName = companyName;}
    QString routeName()const {return _routeName;}
    QString alphaRoute() const {return _alphaRoute;}
    void setAlphaRoute(QString alphaRoute){_alphaRoute = alphaRoute;}
    QString routePrefix() {return _routePrefix;}
    void setRoutePrefix(QString routePrefix) {_routePrefix = routePrefix;}
    qint32 tractionType() {return _tractionType;}
    void setTractionType(qint32 tractionType){_tractionType = tractionType;}
    int baseRoute() {return _baseRoute;}
    void setBaseRoute(int baseRoute){_baseRoute = baseRoute;}
    RouteType routeType() {return _routeType;}
    QList<QPair<int, QString>> setSeqList(QString);
    QString seqToString();

private:
    qint32	_route=-1;
    QString _routeName;
    QString _description;
    QString _companyName;
    qint32	_tractionType;
    QList<SegmentData*> segmentDataList;
    double length;
    //RouteData& rd;
    QDate _startDate;
    QDate _endDate;
    int _companyKey=0;
    QString _alphaRoute;
    int _baseRoute=0;
    QString _routePrefix;
    QString _companyMnemonic;
    int _routeId = -1;
    RouteType _routeType;
    QList<QPair<int, QString>> _seqList;
    QString _canonicalName;
    friend class SQL;
};


#endif // ROUTEINFO_H
