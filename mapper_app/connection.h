#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QSqlDatabase>
#include <QUuid>

class Configuration;
class SQL;
class Connection : public QObject
{
 Q_OBJECT
 qint32 _id =-1;
 QString _driver = "QSQLITE";
 QString _description;
 QString _DSN;
 QString _userId;
 QString _PWD;
 QString _database;
 QString _hostName;
 qint32  _port =0;
 //QString _useDatabase; // MySql or MsSql
 QSqlDatabase db;
 SQL* sql;
 QString _servertype = "Sqlite"; // "MsSql (default), "MySql"
 Configuration* config;
 bool bOpen = false;
 QString _cityName;
 //const QString cName;
 QString _connectionName;
 QString _sqlite_fileName;    // Sqlite
 //QString _odbc_connectorName;
 QString _defaultSqlDatabase; // MySql or MsSql
 //QString _mySqlDatabase;
 QString _connectionType="Local";
 QUuid   _uuid;
 bool _dirty;
 bool _connectionValid = false;
 bool _sqlite_user_function_loaded = false; // true for Sqlite only!
 QStringList _tables;
 QString _connectString;

public:
 Connection(QObject* parent = nullptr);
 Connection(QUuid uud, QObject* parent = nullptr);

 Connection(const Connection&);
 bool isOpen() {return bOpen;}
 void setServerType(QString s) { _servertype = s;}
 QString servertype() {return _servertype;}
 QSqlDatabase configure(const QString cName = QLatin1String(QSqlDatabase::defaultConnection));
 QSqlDatabase getDb();
 void setDb(QSqlDatabase db){this->db = db;}
 qint32 id() {return _id;}
 void setId(qint32 i) {_id = i;}
 QString description() { return _description;}
 void setDescription(QString s) {_description = s;}
 QString driver() {return _driver;}
 void setDriver(QString d) {_driver = d;}
 QString dsn() {return _DSN;}
 void setDSN(QString d) {_DSN = d;}
 QString userId() {return _userId;}
 void setUserId(QString userId) {_userId = userId;}
 QString pwd() {return _PWD;}
 void setPWD(QString p) {_PWD = p;}
 QString database() {return _database;}
 void setDatabase(QString p) {_database = p;}
 QString host() {return _hostName;}
 void setHost(QString h) {_hostName = h;}
 qint32 port() {return _port;}
 void setPort(qint32 p) {_port = p;}
 void setConnectString(QString s){_connectString = s;}
 QString connectString() {return _connectString;}
// QString useDatabase() {return _useDatabase;}
// void setUseDatabase(QString u) {_useDatabase = u;}
 QString cityName() {return _cityName;}
 void setCityName(QString name) {_cityName = name;}
 //static QString dbType(QString name);
 QString connectionName(){return _connectionName;}
 void setConnectionName(QString name) {_connectionName =name; }
 QString sqlite_fileName() {return _sqlite_fileName;}
 void setSqliteFileName(QString fn){_sqlite_fileName = fn;}
// QString odbc_connectorName(){return _odbc_connectorName;}
// void setOdbcConnectorName(QString fn){_odbc_connectorName = fn;}
 QString defaultSqlDatabase() {return _defaultSqlDatabase;}
 void setDefaultSqlDatabase(QString defaultSqlDatabase) {_defaultSqlDatabase = defaultSqlDatabase;}
 static void configureDb(QSqlDatabase db, Connection *currConnection, Configuration *config);
 //QString mySqlDatabase(){return _mySqlDatabase;}
 //void setMySqlDatabase(QString name){_mySqlDatabase =name;}
 QString connectionType(){return _connectionType;}
 void setConnectionType(QString s ) {_connectionType = s;}
 bool tablesChecked = false;  // do not save in settings!
 QUuid uniqueId() {return _uuid;}
 void setUniqueId(QUuid uuid){_uuid = uuid;}
 bool isDirty() {return _dirty;}
 void setDirty(bool dirty){_dirty = dirty;}
 bool isValid() {return _connectionValid;}
 void setValid(bool b){_connectionValid = b;}
 bool isSqliteUserFunctionLoaded() {return _sqlite_user_function_loaded;}
 void setSqliteUserFunctionLoaded(bool b) {_sqlite_user_function_loaded = b;}
 QStringList tables(){return db.tables();}
 void setTables(QStringList tables){_tables = tables;}
};

#endif // CONNECTION_H
