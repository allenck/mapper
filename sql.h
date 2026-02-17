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
#include "../functions/sqlite3.h"

class NotifyRouteChange;
class QSqldatabase;
#define SQLERROR(query) \
do \
{ \
 QSqlError err = query.lastError(); \
 qCritical() << "Sql error:" << err.text(); \
 qCritical() << query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n"; \
 (QMessageBox::StandardButton)SQL::instance()->displaySqlError(query, QMessageBox::NoButton,"",__FUNCTION__, __FILE__, __LINE__); \
} while (0)

#define SQLERROR1(query, buttons, text) \
do \
{ \
 QSqlError err = query.lastError(); \
 qCritical() << "Sql error:" << err.text(); \
 qCritical() << query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n"; \
 SQL::instance()->displaySqlError(query, buttons,text,__FUNCTION__, __FILE__, __LINE__); \
} while (0)


class SQL : public QObject
{
    Q_OBJECT
public:
 static SQL* instance();
    bool dbOpen();
    enum CHANGETYPE {
     ADDSEG,
     DELETESEG,
     MODIFYSEG
    };
    bool isTransactionActive();
    QList<RouteData> routeList;
    QList<RouteData> getRoutesByEndDate();
    QList<RouteData> getRoutesByEndDate(qint32 companyKey);
    QList<RouteData> getRoutesByEndDate(QList<int> compayList);
    //QT_DEPRECATED RouteInfo getRoutePoints(qint32 route, QString name, QString date);
    TerminalInfo getTerminalInfo(qint32 route, QString name, QDate endDate);
    QList<TerminalInfo> getTerminalInfoUsingSegment(int segmentId);
    QString getAlphaRoute(qint32 route, QString routePrefix);
    QMap<int, TractionTypeInfo> getTractionTypes();
    //QT_DEPRECATED QList<SegmentInfo> getSegmentInfo();
    QMap<int, SegmentInfo> getSegmentInfoList(QString locality = " ");
    QStringList getLocations();
    //QT_DEPRECATED SegmentInfo getSegmentInfo(qint32 segmentId);
    SegmentInfo getSegmentInfo(qint32 SegmentId);
    SegmentInfo getSegmentIdForDescription(QString);
    QList<SegmentData *> getRouteSegmentsInOrder(qint32 route, QString name, int companyKey, QDate date);
    QList<SegmentData*> getRouteSegmentsForDate(QDate date, int companyKey);
    QList<RouteData> getRoutes(qint32 segmentid, QString date );
//    Q_DECL_DEPRECATED QList<LatLng>  GetSegmentPoints(qint32 SegmentId);
    bool canConnect(SegmentData sd1, QString matchedTo, SegmentData sd2);
    qint32 sequenceRouteSegments(qint32 segmentId, QList<SegmentData*> segmentList,
                                 RouteData* rd, QString whichEnd);
    double angleDiff(double A1, double A2);
    double intersectingAngle(SegmentData sd, SegmentData sd2);
//    Q_DECL_DEPRECATED bool addPoint( qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon,  double EndLat, double EndLon,  QString StreetName);
//    Q_DECL_DEPRECATED bool movePoint(qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon);
    void beginTransaction (QString name);
    void commitTransaction (QString name);
    void rollbackTransaction (QString name);
    Q_DECL_DEPRECATED bool updateSegment(qint32 SegmentId);
    LatLng getPointOnSegment(qint32 pt, qint32 segmentId);
    QList<StationInfo> getStationAtPoint(LatLng pt);
    bool updateStation(qint32 stationKey, qint32 infoKey);
    bool updateStation(StationInfo sti);
    QString displayDbInfo(QSqlDatabase db);
    QT_DEPRECATED QString getSegmentOneWay(qint32 SegmentId);
    bool doesSegmentExist(QString descr, QString oneWay, QString location = "");
    QString getSegmentDescription(qint32 SegmentId);
    QList<SegmentInfo> getIntersectingSegments(double lat, double lon, double radius);
    QList<SegmentInfo> getIntersectingSegments(double lat, double lon, double radius, RouteType type);
    bool updateRecord(SegmentInfo sd);
    QList<CompanyData *> getCompanies();
    QList<CompanyData*> getCompaniesInDateRange(QDate startDate, QDate endDate);
    bool updateCompany(CompanyData* cd);
    CompanyData *getCompany(qint32 companyKey);
    //QT_DEPRECATED QList<RouteData> getRouteDatasForDate(qint32 route, QString name, int companyKey, QString date);
    QList<SegmentData*> getSegmentDatasForDate(qint32 route, QString name, int companyKey, QDate date);
    QList<SegmentData *> getRouteDatasForDate(int segmentId, QDate date);
    QT_DEPRECATED QList<SegmentData> getRouteDatasForDate(qint32 segmentId, QString date);
    bool saveRouteSequence(RouteData rd, int firstSegment, QString whichEnd);
    bool doesAltRouteExist(int route, QString alphaRoute);
    qint32 addAltRoute(QString routeAlpha, QString routePrefix);
    bool addAltRoute(int routeNum, QString routeAlpha, QString routePrefix);
    bool updateAltRoute(int route, QString routeAlpha);
    bool deleteRouteSegment(SegmentData sd, bool bNotify= false);
    bool deleteRouteSegment(qint32 route, int routeid, qint32 SegmentId, QString startDate, QString endDate, QString routeStartDate, QString routeEndDate);
    bool deleteRouteSegment(qint32 route, int routeId, qint32 SegmentId, QString startDate, QString endDate);
    bool addSegmentToRoute(SegmentData *sd, bool notify = true);
    //bool addSegmentToRoute(RouteData rd);
    // QT_DEPRECATED bool addSegmentToRoute(qint32 routeNbr, QString routeName, QDate startDate, QDate endDate,
    //                        qint32 SegmentId, qint32 companyKey, qint32 tractionType, QString direction,
    //                        qint32 next, qint32 prev,
    //                        qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave,
    //                        qint32 sequence, qint32 reverseSeq,
    //                        QString oneWay, QString trackUsage,
    //                        QDate doubleDate);
    bool addSegmentToRoutes(int _newSegmentId, int _segmentId);

    qint32 getNumericRoute(QString routeAlpha, QString * newAlphaRoute, bool * bAlphaRoute, QString routePrefix);
    int findNextRouteInRange(QString txt);
    bool updateTerminals(TerminalInfo ti);
    bool updateTerminals(qint32 route, QString name, QDate startDate, QDate endDate, qint32 startSegment, QString startWhichEnd, qint32 endSegment, QString endWhichEnd);
    QList<RouteData> getRouteInfo(qint32 route);
    RouteSeq getRouteSeq(RouteData rd);
    bool deleteRouteSeq(RouteSeq rs);
    bool addRouteSeq(RouteSeq rs);
    bool updateCompany(qint32 companyKey, qint32 route);
    Q_DECL_DEPRECATED void setConfig(Configuration *config);
    QList<QString> getRouteNames(qint32 route);
    qint32 getRouteCompany(qint32 route);
    Parameters getParameters(QSqlDatabase db = QSqlDatabase::database());
    bool insertParameters(Parameters, QSqlDatabase db);
    bool updateParameters(Parameters parms, QSqlDatabase db= QSqlDatabase::database());
    QList<SegmentData *> getRouteSegmentsBySegment(qint32 segmentId);
    QList<SegmentData> getRouteSegmentsBySegment(int route, qint32 segmentId);
    QList<SegmentData> getRouteSegmentsForRouteNbr(QString route);
    QList<RouteData> getRouteDataForRouteName(qint32 route, QString name);
    QList<RouteData> checkRouteName(QString name, QDate startDate, QDate endDate);
    double Distance(LatLng latLng1, LatLng latLng2);
    double Distance(double Lat1, double Lon1, double Lat2, double Lon2);
    QT_DEPRECATED QList<SegmentData *> getIntersectingRouteSegmentsAtPoint(int ignore, double lat, double lon, double radius, qint32 route, QString routeName, QString date);
    QList<SegmentData*> getIntersectingRouteSegmentsAtPoint(SegmentData* sd, double radius,
                                                            QString date,
                                                            QMap<int, SegmentData *> segMap,
                                                            int firstSegment, bool enableTurnCheck=true);
    QDate getRoutesEarliestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    //QDate getRoutesLatestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    QDate getRoutesNextDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date);
    bool recalculateSegmentDates(SegmentInfo* si);
    bool doesRouteSegmentExist(SegmentData sd);
    bool doesRouteSegmentExist(qint32 route, QString name, qint32 segmentId, QDate startDate, QDate endDate);
    QList<SegmentInfo> getSegmentsInSameDirection(SegmentInfo siIn, bool reverse = false);
    bool deleteSegment(qint32 SegmentId);
    qint32 getDefaultCompany(qint32 route, QString date);
    LatLng getPointInfo(qint32 pt, qint32 SegmentId);
    qint32 addCompany(QString name, qint32 route, QString startDate, QString endDate);
    QT_DEPRECATED qint32 addSegment(QString Description, QString OneWay, int tracks, RouteType routeType, const QList<LatLng> pointList, QString Location, bool * bAlreadyExists, bool forceInsert= false);
    qint32 addSegment(SegmentInfo sd, bool *bAlreadyExists, bool forceInsert);
    qint32 splitSegment(qint32 pt, qint32 SegmentId, QString oldDesc, QString oldOneWay, QString newDesc, QString newOneWay, RouteType routeType, RouteType newRouteType, int oldTracks, int newTracks, QString oldStreet, QString newStreet);
    SegmentData *getSegmentData(qint32 route, qint32 SegmentId, QString startDate, QString endDate);
    QList<SegmentData *> getSegmentDataList(RouteData rd);
    QList<SegmentData*> getSegmentDataList(int route);
    SegmentData* getConflictingSegmentDataForRoute(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate);
    RouteData getRouteDataForRouteDates(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate);
    bool deleteRoute(qint32 route, int routeId, QString startDate, QString endDate);
    bool modifyRouteDate(RouteData *rd, bool bStartDate, QDate dt);
    //bool modifyCurrentRoute(RouteData *rd, bool bStartDate, QDate dt, QString name1, QString name2);
    bool modifyCurrentRoute(RouteData* rd, bool bStartDate, QDate dt);
    QList<SegmentData *> getConflictingRouteSegments(qint32 route, QString name,
                                                     QDate startDate, QDate endDate, int companyKey,
                                                     qint32 segmentId);
    QList<SegmentData *> getRoutes(qint32 segmentid);
    QList<RouteIntersects> updateLikeRoutes(qint32 segmentid, qint32 route, QString name, QString date, bool bAllRoutes = false);
    QList<SegmentInfo> getDupSegments(SegmentInfo si);
    QList<SegmentInfo> getSegmentsInOppositeDirection(SegmentInfo siIn);
    bool isRouteUsedOnDate(qint32 route, qint32 segmentId,  QString date);
    CommentInfo getComments(qint32 infoKey);
    QList<CommentInfo>* getOrphanComments();
    QList<StationInfo> getStations(QString alphaRoute, QDate date);
    QList<CommentInfo>* commentByText(QString htmlText);
    QList<CommentInfo>* getComments();
    StationInfo getStationInfo(qint32 stationKey);
    QList<StationInfo> getStationsOnSegment(qint32 segmentId);
    StationInfo getStationInfo(QString name);
    int addComment(QString comments, QString tags, QList<int> routesUsed);
    bool addRouteComment(RouteComments rc);
    bool updateComment(qint32 infoKey, QString comments, QString tags = "", QList<int> table = QList<int>());
    bool deleteComment(qint32 infoKey);
    qint32 addStation(StationInfo sti);
    bool deleteStation(qint32 stationKey);
    QList<StationInfo> getStations();
    QList<StationInfo> getStationsLikeName(QString);
    //QT_DEPRECATED bool updateRoute(qint32 route, QString name, QString endDate, qint32 segmentId, qint32 next, qint32 prev, QString trackUsage);
    bool updateRoute(SegmentData osd, SegmentData sd, bool notify = true, bool ignoreErr=false);
    //QT_DEPRECATED int updateRouteDate(int segmentId, QString startDate, QString endDate);
    int updateRouteSegment(int segmentId, QString startDate, QString endDate, int newSegment);
    QDate getFirstCommentDate(qint32 route, QDate date, qint32 companyKey);
    RouteComments getRouteComment(qint32 route, QDate date, qint32 commentKey);
    QList<RouteComments*>getRouteComments(qint32 commentKey);
    QList<RouteComments*> listRouteComments();
    QList<RouteComments*> listInvalidRouteComments();
    bool updateRouteComment(RouteComments rc);
    int countCommentUsers(int commentKey);
    bool deleteRouteComment(RouteComments rc);
    RouteComments getNextRouteComment(qint32 route, QDate date, qint32 commentKey,qint32 companyKey);
    RouteComments getPrevRouteComment(qint32 route, QDate date, qint32 commentKey, qint32 companyKey);
    CommentInfo getComment(qint32 commentKey, int pos);

    QStringList showDatabases(QString Connection, QString servertype);
    QStringList getAlphaRoutes(QString text);
    bool deleteAlphaRoute(QString routeAlpha);
    bool loadSqlite3Functions(QSqlDatabase db);
    bool checkConnectingSegments(QList<SegmentData> segmentDataList);

//    QStringList getTableList(QSqlDatabase db, QString dbType);
    bool checkSegments();
    bool doesColumnExist(QString table, QString column);
    bool doesConstraintExist(QString tbName, QString name);
    bool addColumn(QString table, QString name, QString type, QString after = "");
    bool updateSegmentsTable();
    bool updateSegment(SegmentInfo *sd, bool bNotify = true);
    bool updateSegment(SegmentData* sd);
    //QT_DEPRECATED bool updateSegment(SegmentInfo* );
    bool updateTractionType(qint32 tractionType, QString description, QString displayColor, int routeType, QSqlDatabase db = QSqlDatabase());
    void checkTables(QSqlDatabase db);
    bool executeScript(QString path, QSqlDatabase db = QSqlDatabase());
    bool executeCommand(QString commandString, QSqlDatabase db  = QSqlDatabase(), QList<QVariant> *pList=nullptr);
    QString getPrevRouteName(QDate dt);
    QString getNextRouteName(QDate dt);
    bool testAltRoute();
    double static distance(LatLng latlng1, LatLng latlng2);
    int getCountOfRoutesUsingSegment(int segmentId);
    int getCountOfStationsUsingSegment(int segmentId);
    bool deleteAndReplaceSegmentWith(int segmentId1, int segmentId2);
    bool updateSegmentDates(SegmentInfo *si);
    QT_DEPRECATED QList<SegmentInfo> getUnusedSegments();
    bool replaceSegmentsInRoutes(QStringList oldSegments, QStringList newSegments, QDate ignoreDate);
    QT_DEPRECATED void updateSegmentDates(int segmentId);
    QPair<QDate,QDate> getStartAndEndDates(int segmentId);
    QList<FKInfo> getForeignKeyInfo(QSqlDatabase db, Connection *c, QString table="");
    QMap<int, RouteName*> *routeNameList();
    bool createSqlDatabase(QString dbName, QSqlDatabase db, QString dbType);
    QString getDatabase(QString serverType, QSqlDatabase db = QSqlDatabase());
    bool useDatabase(QString dbName, QString serverType, QSqlDatabase db);
    QStringList showMySqlDatabases(QSqlDatabase db);
    QStringList showMsSqlDatabases(QSqlDatabase db);
    QStringList showPostgreSQLDatabases(QSqlDatabase db);
    int sqlErrorMessage(QSqlQuery query, QMessageBox::StandardButtons buttons);
    SegmentInfo convertSegment(int segmentId, int tracks);
    int nextRouteNumberInRange(int lowRange, int highRange);
    bool renumberRoute(QString oldAlphaRoute, int newRoute, QString routePrefix);
    QList<RouteComments> commentsForRoute(int route);
    QList<TerminalInfo> terminalsForRoute(int route);
    bool updateRouteForStations(int oldRoute, int newRoute);
    bool deleteTerminalInfo(int route);
    bool deleteRoute(SegmentData rd);
    bool deleteRoute(RouteData rd);
    QString currentTransaction;
    bool doesFunctionExist(QString name, QString serverType, QSqlDatabase db);
    QStringList listViews();
    QList<SegmentData *> segmentDataListFromView(QString where);
    QStringList listColumns(QString table, QString serverType, QSqlDatabase db = QSqlDatabase(), QStringList *types=nullptr);
    QStringList listPkColumns(QString table, QString serverType, QSqlDatabase db = QSqlDatabase(), QStringList *types=nullptr);
    bool getForeignKeyCheck();
    void setForeignKeyCheck(bool b);
    int displaySqlError(QSqlQuery query, QMessageBox::StandardButtons buttons, QString text, QString func, QString file, int line);
    //static /*static*/ void distanceFunc(sqlite3_context, int argc, sqlite3_value **argv);
    bool isCompanyValid(SegmentData sd);
    QString list2String(const QList<int> &list);
    QList<QPair<SegmentInfo, SegmentInfo>> getDupSegmentsInList(QList<SegmentInfo> list);
    QDate getNextStartOrEndDate(int route, QDate dt, int segmentId, bool bStart=true);
    QList<SegmentData*> getConflictingRouteSegments(RouteData rd);
    QList<SegmentInfo> getSegmentsForStreet(QString street, QString location);
    //bool splitSegmentDataForCompany(SegmentData sd);
    bool scanRoutes(QList<RouteData> routes);
    bool populateRouteId();
    qint32 addRouteName(RouteInfo ri, bool *bAlreadyExists);
    RouteInfo getRouteName(int routeId);
    int getRouteId(QString routeName);
    bool updateRouteName(RouteInfo ri);
    bool insertRouteName(RouteInfo ri);
    bool updateIdentitySequence(QString table, QString column);
    bool processStream(QTextStream* in, QSqlDatabase db);
    QSqlQuery* getQuery() {return query;}
    bool isFunctionInstalled(QString function, QString dbType, QString dbName, QSqlDatabase db);
    bool createMissingStreetDef(QSqlDatabase db);
    QList<CommentInfo *> *commentsList();
    bool updateComment(CommentInfo info);


signals:
    void details(QString);
    void segmentChanged(const SegmentInfo si, CHANGETYPE t);
    void routeChange(NotifyRouteChange rc);

private:
    SQL();
    static SQL* _instance;
    void myExceptionHandler(Exception e);
    Configuration *config =nullptr;
   // bool compareSegmentData(const segmentData & sd1, const segmentData &sd2);
//    QT_DEPRECATED void populatePointList(SegmentData sd);
    bool insertRouteSegment(SegmentData sd, bool bNotify = true);
    //bool insertRouteSegment(RouteData sd);
    bool processFile(QTextStream* in, QSqlDatabase db, bool bIsInclude);
    QString scriptName;
    void setDefaultCompanyMnemonic(CompanyData* cd);
    QMessageBox::StandardButton errReturn;
    QString _delimiter;
    QStack<QString> delimiters;
    int linesRead =0;
    QSqlQuery* query = nullptr;
};
class NotifyRouteChange
{
 public:
  NotifyRouteChange(SQL::CHANGETYPE t, SegmentData* sd)
  {
   _t = t;
   _sd = sd;
  }
  SQL::CHANGETYPE type() {return _t;}
  SegmentData* sd() {return _sd;}
 private:
  SQL::CHANGETYPE _t;
  SegmentData* _sd;

};

#endif // SQL_H
