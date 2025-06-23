#ifndef EXPORTSQL_H
#define EXPORTSQL_H

#include <QObject>
#include "configuration.h"
#include "qthread.h"
//#include "exportdlg.h"
#include <QSqlQuery>

class QSqldatabase;
#define SQLERROR_E(query) \
do \
{ \
 QSqlError err = query.lastError(); \
 qCritical() << "Sql error:" << err.text(); \
 qCritical() << query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n"; \
    switch (errSqlMessage(query)) {\
    case QMessageBox::Abort:\
     emit ExportSql::requestStop();\
     return false;\
    case QMessageBox::Ignore:\
     continue;\
    default:\
     break;\
   }\
 } while (0)

class ExportSql : public QObject
{
    Q_OBJECT
public:
    ExportSql(Configuration* cfg, bool bDropTables, QObject *parent = 0);
    void setOverride(QDateTime strOvr);
    void setNoDelete(bool bFlag);

    bool exportAltRoute();
    bool exportAll();
    bool exportComments();
    bool exportCompanies();
    bool exportIntersections();
    bool exportTractionTypes();
    bool exportParameters();
    bool exportLineSegments();
    bool exportSegments();
    bool exportRoute();
    bool exportStations();
    bool exportTerminals();
    bool exportRouteComments();
    bool export_geodb_geometry();

    bool exportRoute(RouteData rd);
    bool createSegmentsTable(QSqlDatabase db, QString dbType);
    bool dropTable(QString table, QSqlDatabase db, QString dbType);
    bool createRouteTable(QSqlDatabase db, QString dbType);
    bool createRouteCommentsTable(QSqlDatabase db, QString dbType);
    bool createStationsTable(QSqlDatabase db, QString dbType);
    bool createTerminalsTable(QSqlDatabase db, QString dbType);
    bool createAltRouteTable(QSqlDatabase db, QString dbType);
    bool createParametersTable(QSqlDatabase db, QString dbType);
    bool createCompaniesTable(QSqlDatabase db, QString dbType);
    bool createIntersectionsTable(QSqlDatabase db, QString dbType);
    bool createCommentsTable(QSqlDatabase db, QString dbType);
    bool createTractionTypesTable(QSqlDatabase db, QString dbType);
    bool dropRoutes();
    bool createMySqlFunctions(QSqlDatabase db = QSqlDatabase());
    bool createMsSqlFunctions(QSqlDatabase db);
    QSqlDatabase targetDb() {return _targetDb;}

signals:
    void progress(int value);
    void progressMsg(QString value);
    void uncheck(QString control);
    void requestStop();

public slots:

private:
    QObject *m_parent;
    QSqlDatabase srcDb;
    QSqlDatabase _targetDb;
    Configuration *config;
    qint32 added, updated, deleted, errors, notUpdated;
    Connection* srcConn;
    Connection* tgtConn;
    qint32 rowCount;
    qint32 rowsCompleted;
    QDateTime lastUpdated;
    QString strLastUpdated;
    bool bOverride;
    bool bNoDeletes;
    QString strOverrideTs;
    QDateTime overrideTs;
    bool openDb();
    bool getCount(QString table, bool bDropTables = false);
    void sendProgress();
    //void setIdentityInsert(QString table, bool bOn);
    void updateTimestamp(QString table);
    QString beginTime;
    bool Retry(QSqlDatabase *db, QSqlQuery *query, QString CommandText);
    bool bDropTables;
    QString tgtDbType;
    int errSqlMessage(QSqlQuery query);
    int errReturn;

};

class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

#endif // EXPORTSQL_H
