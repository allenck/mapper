#ifndef EXPORTSQL_H
#define EXPORTSQL_H

#include <QObject>
#include "configuration.h"
#include "qthread.h"
//#include "exportdlg.h"
#include <QSqlQuery>
#include <QFile>

class QSqldatabase;
#define SQLERROR_E(query) \
do \
{ \
 QSqlError err = query->lastError(); \
 qCritical() << "Sql error:" << err.text(); \
 qCritical() << query->lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n"; \
    QApplication::beep();\
    switch (errSqlMessage(query, __LINE__)) {\
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
    ~ExportSql();
    void setOverride(QDateTime strOvr);
    void setNoDelete(bool bFlag);
    void setTargetConn(Connection* tgtConn);
    bool setIdentityInsert(QString table, bool );
    void logError(QSqlQuery* query, bool ignored, int line);
    bool dropView(QString view);

    //bool exportAltRoute();
    bool exportAll();
    //bool exportComments();
    //bool exportCompanies();
    //bool exportIntersections();
    //bool exportTractionTypes();
    //bool exportParameters();
    bool exportLineSegments();
    //bool exportSegments();
    //bool exportRoutes();
//    bool exportStations();
//    bool exportTerminals();
//    bool exportRouteComments();
    //bool export_geodb_geometry();

    bool exportRoutes(RouteData rd);
    bool createSegmentsTable(QSqlDatabase db, QString dbType);
    bool dropTable(QString table, QSqlDatabase db, QString dbType);
    bool createRouteNameTable(QSqlDatabase db, QString dbType);
    bool createRouteTable(QSqlDatabase db, QString dbType);
    bool createRouteCommentsTable(QSqlDatabase db, QString dbType);
    bool createStationsTable(QSqlDatabase db, QString dbType);
    bool createTerminalsTable(QSqlDatabase db, QString dbType);
    bool createAltRouteTable(QSqlDatabase db, QString dbType);
    bool createParametersTable(QSqlDatabase db, QString dbType);
    bool createCompaniesTable(QSqlDatabase db, QString dbType);
    bool createIntersectionsTable(QSqlDatabase db, QString dbType);
    bool createRouteSeqTable(QSqlDatabase db, QString dbType);
    bool createCommentsTable(QSqlDatabase db, QString dbType);
    bool createTractionTypesTable(QSqlDatabase db, QString dbType);
    bool createStreetDefTable(QSqlDatabase db, QString dbType);
    bool dropRoutes();
    bool dropStations();
    bool dropRouteComments();
    bool createMySqlFunctions(QSqlDatabase db = QSqlDatabase());
    bool createMsSqlFunctions(QSqlDatabase db);
    QSqlDatabase targetDb() {return _targetDb;}
    bool exportTable(QString table);
    bool areTableDefsEqual(QString table, Connection* c1, Connection* c2, QSqlDatabase db1, QSqlDatabase db2);

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
    qint32 added, updated, deleted, errors, notUpdated, ignored;
    Connection* srcConn = nullptr;
    Connection* tgtConn = nullptr;
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
    int errSqlMessage(QSqlQuery* query, int line);
    int errReturn;
    QList<QString> ignoreList;
    QString identityInsertTable;
    QString displayQueryValues(QSqlQuery* query);
    QFile* logfile = nullptr;
    QTextStream* stream = nullptr;
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
