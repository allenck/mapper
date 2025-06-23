#ifndef SQL_H
#define SQL_H
#include <QtSql>

#include <QList>
#include <iostream>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtAlgorithms>
#include "mymessagebox.h"
#include "data.h"
#include "configuration.h"
#include <QMessageBox>
#include "exceptions.h"
#include "routeselector.h"
#include <QSqlQuery>

class QSqldatabase;
#define SQLERROR(query) \
do \
{ \
 QSqlError err = query.lastError(); \
 qCritical() << "Sql error:" << err.text(); \
 qCritical() << query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n"; \
 } while (0)


class SQL : public QObject
{
    Q_OBJECT
public:
 static SQL* instance();
    bool dbOpen();
    //QSqlDatabase db;
    // http://doc.qt.nokia.com/4.7/qlist.html
    bool isTransactionActive();
    QList<RouteData> routeList;
    QList<RouteData> getRoutesByEndDate();
    QList<RouteData> getRoutesByEndDate(qint32 companyKey);
    //QT_DEPRECATED RouteInfo getRoutePoints(qint32 route, QString name, QString date);
    TerminalInfo getTerminalInfo(qint32 route, QString name, QString endDate);
    QList<TerminalInfo> getTerminalInfoUsingSegment(int segmentId);
    QString getAlphaRoute(qint32 route, qint32 company);
    QMap<int, TractionTypeInfo> getTractionTypes();
    //QT_DEPRECATED QList<SegmentInfo> getSegmentInfo();
    QMap<int, SegmentData> getSegmentInfoList(QString locality = " ");
    QStringList getLocations();
    //QT_DEPRECATED SegmentInfo getSegmentInfo(qint32 segmentId);
    SegmentInfo getSegmentInfo(qint32 SegmentId);
    QList<SegmentData> getRouteSegmentsInOrder(qint32 route, QString name, QString date);
    //QT_DEPRECATED QList<SegmentInfo> getRouteSegmentsInOrder2(qint32 route, QString name, QString date);
    QList<RouteData> getRoutes(qint32 segmentid, QString date );
//    Q_DECL_DEPRECATED QList<LatLng>  GetSegmentPoints(qint32 SegmentId);
    bool canConnect(SegmentData sd1, QString matchedTo, SegmentData sd2);
    qint32 sequenceRouteSegments(qint32 segmentId, QList<SegmentData> segmentList,
                                 qint32 route, QString name, QString date, QString whichEnd);
    double angleDiff(double A1, double A2);
    double connectingAngle(SegmentData sd, QString enterAt, SegmentData sd2);
//    Q_DECL_DEPRECATED bool addPoint( qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon,  double EndLat, double EndLon,  QString StreetName);
//    Q_DECL_DEPRECATED bool movePoint(qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon);
    void beginTransaction (QString name);
    void commitTransaction (QString name);
    void rollbackTransaction (QString name);
    Q_DECL_DEPRECATED bool updateSegment(qint32 SegmentId);
    LatLng getPointOnSegment(qint32 pt, qint32 segmentId);
    StationInfo getStationAtPoint(LatLng pt);
    bool updateStation(qint32 stationKey, qint32 infoKey);
    bool updateStation(qint32 stationKey, LatLng latLng);
    bool updateStationRoute(qint32 stationKey, qint32 route);
//    Q_DECL_DEPRECATED bool insertPoint(qint32 pt, qint32 SegmentId, double newLat, double newLon);
//    Q_DECL_DEPRECATED bool insertPoint(qint32 pt, qint32 SegmentId, double newLat, double newLon/*,  qint32* lineSegmentKey*/);
//    Q_DECL_DEPRECATED bool deletePoint(qint32 pt, qint32 SegmentId, qint32 nbrPts);
    //QList<segmentData> getIntersectingSegments(double lat, double lon, double radius) __attribute_deprecated__;

    QT_DEPRECATED QString getSegmentOneWay(qint32 SegmentId);
    bool doesSegmentExist(QString descr, QString oneWay, QString location = "");
    QString getSegmentDescription(qint32 SegmentId);
    QList<SegmentData> getIntersectingSegments(double lat, double lon, double radius);
    QList<SegmentData> getIntersectingSegments(double lat, double lon, double radius, RouteType type);
    bool updateRecord(SegmentInfo sd);
    bool updateSegmentDetails(qint32 SegmentId, QString description, int tracks, double length, RouteType type);
    QList<CompanyData *> getCompanies();
    CompanyData *getCompany(qint32 companyKey);
    QList<SegmentData> getRouteSegmentsForDate(qint32 route, QString name, QString date);
    QList<SegmentData> getRouteSegmentsForDate(qint32 segmentId, QString date);
    bool doesAltRouteExist(int route, QString alphaRoute);
    qint32 addAltRoute(QString routeAlpha, QString routePrefix);
    bool addAltRoute(int routeNum, QString routeAlpha);
    bool updateAltRoute(int route, QString routeAlpha);
    bool deleteRouteSegment(SegmentData sd);
    bool deleteRouteSegment(qint32 route, QString name, qint32 SegmentId, QString startDate, QString endDate, QString routeStartDate, QString routeEndDate);
    bool deleteRouteSegment(qint32 route, QString name, qint32 SegmentId, QString startDate, QString endDate);
    bool addSegmentToRoute(SegmentData rd);
//    QT_DEPRECATED bool addSegmentToRoute(RouteData rd);
//                           qint32 SegmentId, qint32 companyKey, qint32 tractionType, QString direction,
//                           qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave,
//                           QString oneWay, QString trackUsage);
    bool addSegmentToRoute(qint32 routeNbr, QString routeName, QDate startDate, QDate endDate,
                           qint32 SegmentId, qint32 companyKey, qint32 tractionType, QString direction,
                           qint32 next, qint32 prev,
                           qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave,
                           QString oneWay, QString trackUsage);
    qint32 getNumericRoute(QString routeAlpha, QString * newAlphaRoute, bool * bAlphaRoute, int companyKey);
    int findNextRouteInRange(QString txt);
    bool updateTerminals(TerminalInfo ti);
    bool updateTerminals(qint32 route, QString name, QString startDate, QString endDate, qint32 startSegment, QString startWhichEnd, qint32 endSegment, QString endWhichEnd);
    QList<RouteData> getRouteInfo(qint32 route);
    bool updateCompany(qint32 companyKey, qint32 route);
    Q_DECL_DEPRECATED void setConfig(Configuration *config);
    QList<QString> getRouteNames(qint32 route);
    qint32 getRouteCompany(qint32 route);
    Parameters getParameters();
    bool insertParameters(Parameters, QSqlDatabase db);
    QList<SegmentData> getRouteSegmentsBySegment(qint32 segmentId);
    QList<SegmentData> getRouteSegmentsBySegment(int route, qint32 segmentId);
    QList<SegmentData> getRouteSegmentsForRouteNbr(QString route);
    QList<SegmentData> getRouteDataForRouteName(qint32 route, QString name);
    double Distance(double Lat1, double Lon1, double Lat2, double Lon2);
    QList<SegmentData> getIntersectingRouteSegmentsAtPoint(int ignore, double lat, double lon, double radius, qint32 route, QString routeName, QString date);
    QDate getRoutesEarliestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    QDate getRoutesLatestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    QDate getRoutesNextDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    bool doesRouteSegmentExist(SegmentData sd);
    bool doesRouteSegmentExist(qint32 route, QString name, qint32 segmentId, QDate startDate, QDate endDate);
    SegmentData getSegmentInSameDirection(SegmentData siIn);
    bool deleteSegment(qint32 SegmentId);
    qint32 getDefaultCompany(qint32 route, QString date);
    LatLng getPointInfo(qint32 pt, qint32 SegmentId);
    qint32 addCompany(QString name, qint32 route, QString startDate, QString endDate);
    qint32 addSegment(QString Description, QString OneWay, int tracks, RouteType routeType, const QList<LatLng> pointList, QString Location, bool * bAlreadyExists, bool forceInsert= false);
    qint32 addSegment(SegmentData sd, bool *bAlreadyExists, bool forceInsert);
    qint32 splitSegment(qint32 pt, qint32 SegmentId, QString oldDesc, QString oldOneWay, QString newDesc, QString newOneWay, RouteType routeType, RouteType newRouteType, int oldTracks, int newTracks, QString oldStreet, QString newStreet);
    SegmentData getSegmentData(qint32 route, qint32 SegmentId, QString startDate, QString endDate);
    bool updateSegmentToRoute(qint32 routeNbr, QString routeName, QString startDate, QString endDate, qint32 SegmentId, qint32 companyKey, qint32 tractionType, qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave, QString biDirectional);
    SegmentData getSegmentInfoForRouteDates(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate);
    bool deleteRoute(qint32 route, QString name, QString startDate, QString endDate);
    bool modifyRouteDate(RouteData *rd, bool bStartDate, QDate dt, QString name1, QString name2);
    bool modifyCurrentRoute(RouteData *rd, bool bStartDate, QDate dt, QString name1, QString name2);
    QList<SegmentData> getConflictingRouteSegments(qint32 route, QString name,
                                                   QString startDate, QString endDate,
                                                   qint32 segmentId);
    QList<SegmentData> getRoutes(qint32 segmentid);
    QList<RouteIntersects> updateLikeRoutes(qint32 segmentid, qint32 route, QString name, QString date, bool bAllRoutes = false);
    SegmentData getSegmentInOppositeDirection(SegmentData siIn);
    bool isRouteUsedOnDate(qint32 route, qint32 segmentId,  QString date);
    CommentInfo getComments(qint32 infoKey);
    QList<StationInfo> getStations(qint32 route, QString name, QString date);
    StationInfo getStationInfo(qint32 stationKey);
    StationInfo getStationInfo(QString name);
    int addComment(QString comments, QString tags);
    bool updateComment(qint32 infoKey, QString comments, QString tags = "");
    bool deleteComment(qint32 infoKey);
    //qint32 addStation(QString name, LatLng location, qint32 segmentId, RouteType type);
    //qint32 addStation(QString name, LatLng location, qint32 segmentId, QString startDate, QString endDate, qint32 geodb_loc_id, qint32 infoKey, RouteType routeType, qint32 route, int point);
    //qint32 addStation(QString name, LatLng location, qint32 segmentId, QString startDate, QString endDate, qint32 geodb_loc_id, RouteType routeType, QString markerType);
    qint32 addStation(QString name, LatLng location, qint32 segmentId, QString startDate, QString endDate, qint32 geodb_loc_id, qint32 infoKey, RouteType routeType, QString markerType, int point);
    bool deleteStation(qint32 stationKey);
    bool updateStation(qint32 stationKey, qint32 infoKey, QString name, qint32 segmentId, QString startDate, QString endDate, QString markerType);
    bool updateStationLineSegment(qint32 route, qint32 lineSegmentId, LatLng pt);
    bool updateStationLineSegment(qint32 geodb_loc_id, qint32 route, qint32 lineSegmentId, LatLng pt);
    bool updateStation(qint32 geodb_loc_id, qint32 stationKey, LatLng latLng);
    bool updateStation(qint32 stationKey, LatLng latLng, qint32 segmentId);
    QList<StationInfo> getStations();
    bool updateStation(qint32 stationKey,  qint32 route, qint32 lineSegmentId, qint32 segmentId, QString startDate, QString EndDate, qint32 *newStationId, int point);
//    QList<segmentData> getIntersectingSegmentsWithRoute(double lat, double lon, double radius, RouteType type) __attribute_deprecated__;
    QList<StationInfo> getStationsLikeName(QString);
    QT_DEPRECATED bool updateRoute(qint32 route, QString name, QString endDate, qint32 segmentId, qint32 next, qint32 prev, QString trackUsage);
    //bool updateRoute(RouteData rd);
    bool updateRoute(SegmentData osd, SegmentData sd);
    int updateRouteDate(int segmentId, QString startDate, QString endDate);
    int updateRouteSegment(int segmentId, QString startDate, QString endDate, int newSegment);
    QDate getFirstCommentDate(qint32 route, QDate date, qint32 companyKey);
    RouteComments getRouteComment(qint32 route, QDate date, qint32 companyKey);
    bool updateRouteComment(RouteComments rc);
    int countCommentUsers(int commentKey);
    bool deleteRouteComment(RouteComments rc);
    RouteComments getNextRouteComment(qint32 route, QDate date, qint32 companyKey);
    RouteComments getPrevRouteComment(qint32 route, QDate date, qint32 companyKey);
    CommentInfo getComment(qint32 commentKey, int pos);

    QStringList showDatabases(QString Connection, QString servertype);
    QStringList getAlphaRoutes(QString text);
    bool deleteAlphaRoute(QString routeAlpha);
    bool loadSqlite3Functions();
    bool checkConnectingSegments(QList<SegmentData> segmentDataList);

//    QStringList getTableList(QSqlDatabase db, QString dbType);
    bool checkSegments();
    bool doesColumnExist(QString table, QString column);
    bool doesConstraintExist(QString tbName, QString name);
    bool addColumn(QString table, QString name, QString type, QString after = "");
    bool updateSegmentsTable();
    bool updateSegment(SegmentInfo *sd);
    bool updateSegment(SegmentData* sd);
    //QT_DEPRECATED bool updateSegment(SegmentInfo* );
    bool updateTractionType(qint32 tractionType, QString description, QString displayColor, int routeType, QSqlDatabase db = QSqlDatabase());
    void checkTables(QSqlDatabase db);
    bool executeScript(QString path, QSqlDatabase db = QSqlDatabase());
//    QT_DEPRECATED bool getSegmentDates(SegmentInfo* is);
    QString getPrevRouteName(QDate dt);
    QString getNextRouteName(QDate dt);
    bool testAltRoute();
    double static distance(LatLng latlng1, LatLng latlng2);
    int getCountOfRoutesUsingSegment(int segmentId);
    int getCountOfStationsUsingSegment(int segmentId);
    bool deleteAndReplaceSegmentWith(int segmentId1, int segmentId2);
    bool updateSegmentDates(SegmentInfo *sd);
    QT_DEPRECATED QList<SegmentData> getUnusedSegments();
    bool replaceSegmentsInRoutes(QStringList oldSegments, QStringList newSegments, QDate ignoreDate);
    bool updateSegmentDates(); // Global
    void updateSegmentDates(int segmentId);
    QPair<QDate,QDate> getStartAndEndDates(int segmentId);
    QList<FKInfo> getForeignKeyInfo();
    QMap<int, RouteName*> *routeNameList();
    bool createSqlDatabase(QString dbName, QSqlDatabase db, QString dbType);
    bool useDatabase(QString dbName, QSqlDatabase db);
    QStringList  showMySqlDatabases(QSqlDatabase db);
    QStringList showMsSqlDatabases(QSqlDatabase db);
    int sqlErrorMessage(QSqlQuery query, QMessageBox::StandardButtons buttons);
    SegmentInfo convertSegment(int segmentId, int tracks);
    int nextRouteNumberInRange(int lowRange, int highRange);
    bool renumberRoute(QString oldAlphaRoute, int newRoute);
    QList<RouteComments> commentsForRoute(int route);
    QList<TerminalInfo> terminalsForRoute(int route);
    bool updateRouteForStations(int oldRoute, int newRoute);
    bool deleteTerminalInfo(int route);
    bool deleteRoute(SegmentData rd);
    //bool deleteRoute(RouteData rd);
    QString currentTransaction;
    bool doesFunctionExist(QString name, QSqlDatabase db);

signals:
    void details(QString);
    void segmentsChanged(int segmentId);
private:
    SQL();
    static SQL* _instance;
    void myExceptionHandler(Exception e);
    Configuration *config =nullptr;
   // bool compareSegmentData(const segmentData & sd1, const segmentData &sd2);
//    QT_DEPRECATED void populatePointList(SegmentData sd);
    bool insertRouteSegment(SegmentData sd);

};

#endif // SQL_H
