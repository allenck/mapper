#ifndef ROUTEDATA_H
#define ROUTEDATA_H

#include <QObject>
//#include "segmentdata.h"
#include "routetype.h"
#include <QDate>
class SegmentData;
class RouteData
{
public:
    explicit RouteData();

    ~RouteData();
    RouteData(const RouteData&);
    RouteData(const SegmentData&);
    const QString toString();
    int route() {return _route;}
    void setRoute(int route) {_route = route;}
    void setRouteId(int routeId) {_routeId = routeId;}
    QString alphaRoute() const {return _alphaRoute;}
    void setAlphaRoute(QString alphaRoute) {_alphaRoute = alphaRoute;}
    QString routeName() const {return _name;}
    void setRouteName(QString name) {_name = name;}
    QDate startDate() const {return _dateBegin;}
    QDate doubleDate() const {return _doubleDate;}
    void setStartDate(QDate date) {_dateBegin = date;}
    QDate endDate() const {return _dateEnd;}
    void setEndDate(QDate date) {_dateEnd = date;}
    void setDoubleDate(QDate date) {_doubleDate = date;}
    qint32 companyKey() {return _companyKey;}
    void setCompanyKey(int key) {_companyKey = key;}
    QString companyMnemonic() {return _companyMnemonic;}
    void setCompanyMnemonic(QString mnemonic){_companyMnemonic = mnemonic;}
    qint32 tractionType() {return _tractionType;}
    void setTractionType(int t) {_tractionType = t;}
    qint32 lineKey() {return _lineKey;}
    qint32 segmentId() {return _lineKey;}
    QString direction() {return _direction;}
    qint32 normalEnter() {return _normalEnter;}
    qint32 normalLeave() {return _normalLeave;}
    qint32 reverseEnter() {return _reverseEnter;}        // Not defined for one Way
    qint32 reverseLeave() {return _reverseLeave;}        // Not defined for one Way
    QString oneWay() {return _oneWay;}
    QString trackUsage() {return _trackUsage;}
    int next() {return _next;}
    int prev() {return _prev;}
    int nextR() {return _nextR;}
    int prevR() {return _prevR;}
    int tracks() {return _tracks;}
    QList<QPair<int, QString>> seqList() {return _seqList;}
    void addSequence(QPair<int, QString> pair) {_seqList.append(pair);}
    QList<QPair<int, QString>> setSeqList(QString);
    QString seqToString();
    qint32 sequence() {return _sequence;}
    qint32 returnSeq() {return _returnSeq;}
    QString routePrefix() {return _routePrefix;}
    void setRoutePrefix(QString routePrefix) {_routePrefix = routePrefix;}
    int baseRoute() {return _baseRoute;}
    void setBaseRoute(int baseRoute){_baseRoute = baseRoute;}
    RouteType routeType() {return _routeType;}
    int routeId(){return _routeId;}
    bool operator==(const RouteData &o);
    QString companyName() {return _coName;}
    void setCompanyName(QString name) {_coName = name;}

private:
    qint32 _route = -1;
    qint32 _routeId = -1;
    qint32 _baseRoute=0;
    QString _alphaRoute;
    QString _routePrefix;
    QString _name;
    QString _coName;
    //QDate defaultDate;
    QDate _dateBegin;
    QDate _dateEnd;
    QDate _doubleDate;
    qint32 _companyKey;
    QString _companyMnemonic;
    qint32 _tractionType;
    qint32 _lineKey = -1;
    QString _direction;
    qint32 _normalEnter;
    qint32 _normalLeave;
    qint32 _reverseEnter;        // Not defined for one Way
    qint32 _reverseLeave;        // Not defined for one Way
    qint32 _sequence;
    qint32 _returnSeq;
    QString _oneWay;
    QString _trackUsage;
    int _next =-1, _prev=-1;
    int _nextR =-1, _prevR=-1;
    int _tracks;
    //    LatLng startLatLng;
    //    LatLng endLatLng;
    //    SegmentData sd;
    //    Bearing* bearing = nullptr;
    bool bNeedsUpdate = false;
    RouteType _routeType;
    QList<QPair<int, QString>> _seqList;
signals:

public slots:
    friend class SQL;
    friend class SegmentData;
};
Q_DECLARE_METATYPE(RouteData)
#endif // ROUTEDATA_H
