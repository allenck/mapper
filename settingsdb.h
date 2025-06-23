#ifndef SETTINGSDB_H
#define SETTINGSDB_H

#include <QObject>
#include <QSettings>
#include <QtCore>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define CONFIG_NOT_NULL      0x000001
#define CONFIG_DONT_EDIT     0x000002
#define CONFIG_ENUM          0x000004
#define CONFIG_PATH          0x000010
#define CONFIG_FILE          0x000020
#define CONFIG_CONNECTION    0x000100    // a connection property
#define CONFIG_DB_DRIVER     0x000200
#define CONFIG_DB_TYPE       0x000400
#define CONFIG_PASSWORD      0x000800
#define CONFIG_SRID          0x001000
#define CONFIG_TILES_DIR_FALLBACK 0x008000
#define CONFIG_GDAL_TILES_DIRECTORY 0x010000
#define CONFIG_GDAL_RASTER_DIRECTORY 0x020000
#define CONFIG_WMSSERVER     0x040000
#define CONFIG_TMSSERVER     0x080000


//namespace QGeomCollection
//{
 class settingsDb : public QObject
 {
  Q_OBJECT
 public:
  explicit settingsDb(QObject *parent = 0);
  settingsDb(QSettings *settings, QObject *parent = 0);
  settingsDb( const QString & organization, const QString & application = QString(), QObject * parent = 0 );
  ~settingsDb();
  QString fileName();
  void beginGroup(QString group);
  void endGroup();
  void setValue(const QString & key, const QVariant & value);
  void setValue(const QString & key, const QVariant & value, int flags);
  QVariant value(const QString & key, const QVariant & defaultValue = QVariant()) const;
  QString valueType(const QString & key) const;
  QString group();
  QStringList childGroups();
  void remove(const QString & key);
  void remove(const int arrayIndex);
  QStringList childKeys();
  bool contains(const QString & key);
  int beginReadArray ( const QString & prefix );
  void beginWriteArray ( const QString & prefix, int size = -1 );
  void endArray ();
  void setArrayIndex ( int i );
  QSqlDatabase getDb();
  bool mustBeSerialized(QVariant v);
  bool mustBeSerialized(const QString typeName) const;
  QByteArray QVariantToByteArray(QVariant v);
  QVariant ByteArrayToQVariant(QByteArray v, QString type) const;
//  QByteArray IntListToByteArray(QList<int> list);
//  QList<int> ByteArrayToIntList(QByteArray v, QString type) const;
  void closeDb();
  void setDb(QSqlDatabase db);
  void openDb();
  QVariant getTableInfo(QString uuid, QString table, QString type);
  bool saveTableInfo(QString uuid, QString table, QString type, QVariant data);
  bool deleteTableInfo(QString uuid);
  bool checkMaster(int i_debug=0);

    
 signals:
    
 public slots:
 private:
  QString s_fileName;
  QSqlDatabase db;
  QString s_current_Group;
  int arrayIndex;
  void checkGroup();
  bool isArray;
  void initialize(QSettings *settings);
  bool compareQVariants(QVariant v1, QString t1, int flags1, QVariant v2, QString t2, int flags2);
  bool bAutoClose;
  bool compareByteArrays(QByteArray ba1, QByteArray ba2);

    
 };
//}
#endif // SETTINGSDB_H
