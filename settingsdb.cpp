#include "settingsdb.h"

//namespace QGeomCollection
//{
 settingsDb::settingsDb(QObject *parent) :
    QObject(parent)
 {
  QSettings *settings = new QSettings();
  bAutoClose = true;
  initialize(settings);
 }
 settingsDb::settingsDb(QSettings *settings, QObject *parent)
 {
  Q_UNUSED(parent);
  bAutoClose = false;
  initialize(settings);
 }
 settingsDb::settingsDb(const QString &organization, const QString &application, QObject *parent)
 {
  QSettings *settings = new QSettings(organization, application, parent);
  bAutoClose = true;
  initialize(settings);
  // delete settings; // avoid memory leaks [causes crash]
 }
 settingsDb::~settingsDb()
 {
//  if(bAutoClose)
//   db.close();
 }

 void settingsDb::initialize(QSettings *settings)
 {
  if(!QSqlDatabase::contains("settings"))
   db = QSqlDatabase::addDatabase("QSQLITE", "settings");
  else
   db = QSqlDatabase::database("settings");
  QString fileName = settings->fileName();
  int index = fileName.lastIndexOf(".conf");
  s_fileName = fileName;
  s_fileName.replace(index, 5, ".db.conf");// old naming convention
  QFile *this_file = new QFile(s_fileName);
  if(this_file->exists())
  {
   // check if a filename with the old convention exists. If so, rename it.
   QString s_old_Filename = s_fileName;
   s_fileName = fileName;
   s_fileName.append(".db"); // new name
   // rename from xxx.db.conf to xxx.conf.db
   this_file->rename(s_old_Filename, s_fileName );
   db.setDatabaseName(s_fileName);
   db.open();
  }
  else
  {
   s_fileName = fileName.append(".db"); // new naming convention
   this_file = new QFile(s_fileName);
   if(this_file->exists())
   {
    db.setDatabaseName(s_fileName);
    db.open();
   }
   else
   {
    if(!this_file->open(QIODevice::WriteOnly))
    {
     qDebug()<<"-E-> settingsDb:";
     qDebug()<< this_file->errorString() << s_fileName;
     return;
    }

    this_file->close();
    db.setDatabaseName(s_fileName);
    if(!db.open())
    {
     qDebug()<< db.lastError().text();
     return;
    }
    QSqlQuery query(db);
    QString s_Command_String = "create table if not exists master( groups text PRIMARY Key not null)";
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return;
    }
   }
  }
  s_current_Group = "";
  arrayIndex = 0;
  checkMaster();
 }
 QString settingsDb::fileName()
 {
  return s_fileName;
 }
 void settingsDb::openDb()
 {
  db.setDatabaseName(s_fileName);
  db.open();
 }

 // beginGroup
 void settingsDb::beginGroup(QString s_group)
 {
  QString s_new_Group;
  if (s_current_Group == "")
   s_new_Group = s_group;
  else
   s_new_Group= QString("%1/%2").arg(s_current_Group).arg(s_group);
  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String = "select count(*) from master where groups = '" + s_new_Group + "'";
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return;
  }
  int count = 0;
  while(query.next())
  {
   count = query.value(0).toInt();
  }
  if(count == 0)
  {
   s_Command_String = "insert into master (groups) values('" + s_new_Group + "')";
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
   s_Command_String = QString("create table if not exists `%1` (`keyVal` text not null, sequence int  not null, value  BLOB, `type` text, CONSTRAINT pk PRIMARY KEY (`keyVal`, sequence))").arg(s_new_Group);
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
  }
  // add type column if it doesn't exist
  s_Command_String = QString("alter table `%1` add column `type`  text default ' ' ").arg(s_new_Group);
  query.exec(s_Command_String);
  // add flags column if it doesn't exist
  s_Command_String = QString("alter table `%1` add column `flags`  int default 0 ").arg(s_new_Group);
  query.exec(s_Command_String);
  s_current_Group = s_new_Group;
  arrayIndex = 0;
 }
 // endGroup
 void settingsDb::endGroup()
 {
  int index = s_current_Group.lastIndexOf("/");
  if(index == -1)
   s_current_Group = "";
  else
   s_current_Group = s_current_Group.mid(0,index);
 }
 // setValue
 void settingsDb::setValue(const QString &key, const QVariant &value)
 {
  setValue(key, value, 0);
 }
 void settingsDb::setValue(const QString &key, const QVariant &value, int flags)
 {
  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String;
  QString groupTable = s_current_Group.isEmpty()?"General":s_current_Group;
  if(s_current_Group.isEmpty())
  {
   s_Command_String = "select count(*) from `Master` where groups = 'General'";
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
   int count = 0;
   while(query.next())
   {
    count = query.value(0).toInt();
   }
   if(count == 0)
   {
    s_Command_String = "insert into master (groups) values('General')";
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return;
    }
    s_Command_String = "create table if not exists `General` (`keyVal` text not null, sequence int  not null, value  BLOB, `type` text, `flags` int default 0, CONSTRAINT pk PRIMARY KEY (`keyVal`, sequence))";
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return;
    }
   }
  }
  if(value == QVariant())
  {
   s_Command_String = QString("delete from `%1` where `keyVal` = '%2' and sequence = %3").arg(groupTable).arg(key).arg(arrayIndex);
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
   checkGroup();
  }
  else
  {
   s_Command_String = QString("select count(*), `value`, `type`, `flags` from `%1` where `keyVal` = '%2' and `sequence` = %3").arg(groupTable).arg(key).arg(arrayIndex);
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
   int count = 0;
   QVariant currValue;
   QString currType;
   int currFlags;
   while(query.next())
   {
    count = query.value(0).toInt();
    currValue = query.value(1);
    currType = query.value(2).toString();
    currFlags = query.value(3).toInt();
   }
   if(count == 0)
    s_Command_String = QString("insert into `%1` (`keyVal`, `sequence`, `value`, `type`, `flags`) VALUES (:keyVal, :sequence, :value, :type, :flags)").arg(groupTable);
   else
   {
    QVariant newValue = value;
    QString newType = value.typeName();
    int newFlags = flags;
    if(compareQVariants(newValue, newType, newFlags, currValue, currType, currFlags))
     return;
    s_Command_String = QString("update `%1` set `value` = :value, `type` = :type, `flags` = :flags where `keyVal` = :keyVal and `sequence` = :sequence").arg(groupTable);
   }
   QString type = value.typeName();
   query.prepare(s_Command_String);
   query.bindValue(":keyVal", key);
   query.bindValue(":sequence", arrayIndex);
   if(mustBeSerialized(value))
    query.bindValue(":value", QVariantToByteArray(value));
   else
    query.bindValue(":value", value);
   query.bindValue(":type", QString(type));
   query.bindValue(":flags", flags);
   if (!query.exec())
   {
    qDebug()<< QString("-E-> settingsdb: error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
  }
 }
 // value
 QVariant settingsDb::value(const QString &key, const QVariant &defaultValue) const
 {
  QString groupTable = s_current_Group.isEmpty()?"General":s_current_Group;

  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String = QString("select `value`, `type` from `%1` where `keyVal` = '%2' and `sequence` = %3").arg(groupTable).arg(key).arg(arrayIndex);
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return QVariant();
  }
  while(query.next())
  {
    QString type = query.value(1).toString();
    if(mustBeSerialized(type))
     return ByteArrayToQVariant(query.value(0).toByteArray(),type);
    else
     return query.value(0);
  }
  return defaultValue;
 }
 QString settingsDb::valueType(const QString &key) const
 {
  QString groupTable = s_current_Group.isEmpty()?"General":s_current_Group;

  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String = QString("select `value`, `type` from `%1` where `keyVal` = '%2' and `sequence` = %3").arg(groupTable).arg(key).arg(arrayIndex);
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return "";
  }
  QString type;
  while(query.next())
  {
   type = query.value(1).toString();
//    if(mustBeSerialized(type))
//     return ByteArrayToQVariant(query.value(0).toByteArray(),type);
//    else
//     return query.value(0);
  }
  return type;
 }
 // group : return current group
 QString settingsDb::group()
 { return s_current_Group;}
 // childGroups : return list of groups
 QStringList settingsDb::childGroups()
 {
  QStringList list;
  if(s_current_Group.isEmpty())
  {
   QSqlQuery query = QSqlQuery(db);
   QString s_Command_String = QString("select groups from `master`");
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return list;
   }
   while(query.next())
   {
    list.append(query.value(0).toString());
   }
  }
  return list;
 }

 // remove entry from master if count is 0.
 void settingsDb::checkGroup()
 {
  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String = "select count(*) from master where groups = '" + s_current_Group + "'";
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return;
  }
  int count = 0;
  while(query.next())
  {
   count = query.value(0).toInt();
  }
  if(count == 0)
  {
   s_Command_String = QString("delete from `master` where groups = '%1'").arg(s_current_Group);
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
  }
  s_Command_String = QString("drop table `%1`").arg(s_current_Group);
  s_Command_String = QString("delete from `master` where groups = '%1'").arg(s_current_Group);
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return;
  }
 }
 // remove : remove a key from a group.
 void settingsDb::remove(const QString &key)
 {
  setValue(key, QVariant());
 }
 // remove : remove an index level from an array and renumber remaining indexes
 void settingsDb::remove(const int index)
 {
  if(!isArray)
  {
   qDebug()<< "-W-> remove: no active array";
   return;
  }
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  s_Command_String = QString("delete from `%1` where `sequence` = %2").arg(s_current_Group).arg(index);
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return;
  }
  s_Command_String = QString("update `%1` set `sequence` = `sequence` -1 where `sequence` > %1");
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return;
  }
  checkGroup();
 }

 // childKeys : return all keys in a group
 QStringList settingsDb::childKeys()
 {
  QStringList list;
  if(s_current_Group.isEmpty())
   return list;
  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String = QString("select `keyVal` from `%1`").arg(s_current_Group );
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return list;
  }
  while(query.next())
  {
   list.append(query.value(0).toString());
  }
  return list;
 }
 // contains : returns true if key exists. If group is set using beginGroup, key is taken relative to that group;
 bool settingsDb::contains(const QString &key)
 {
  bool ret = false;
  QSqlQuery query = QSqlQuery(db);
  QString s_Command_String;
  if(s_current_Group.isEmpty())
  {
   QStringList list = childGroups();
   foreach(QString group, list)
   {
    s_Command_String = QString("select count(*) from `%1` where `keyVal` = '%2'").arg(group).arg(key);
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return false;
    }
    int count = 0;
    while(query.next())
    {
     count = query.value(0).toInt();
    }
    if(count > 0)
     return true;
   }
  }
  else
  {
   s_Command_String = QString("select count(*) from `%1` where `keyVal` = '%2'").arg(s_current_Group).arg(key);
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return false;
   }
   int count = 0;
   while(query.next())
   {
    count = query.value(0).toInt();
   }
   if(count > 0)
    return true;
  }
  return ret;
 }
 void settingsDb::setArrayIndex(int i)
 {
  if(isArray)
   arrayIndex = i+1;
 }
 void settingsDb::beginWriteArray(const QString &prefix, int size)
 {
  //Q_UNUSED(size);
  //beginGroup(prefix);
  int count =beginReadArray(prefix);
  isArray = true;
  if(size < count && size > 0)
  {
   QString s_Command_String;
   QSqlQuery query = QSqlQuery(db);
   s_Command_String = QString("Delete from `%1` where `sequence` > %2").arg(s_current_Group).arg(size);
   //int max = 0;
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return;
   }
  }
 }
 void settingsDb::endArray()
 {
  isArray = false;
  endGroup();
 }
 int settingsDb::beginReadArray(const QString &prefix)
 {
  beginGroup(prefix);
  isArray = true;
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  s_Command_String = QString("Select max (`sequence`) from `%1`").arg(s_current_Group);
  int max = 0;
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return -1;
  }
  while(query.next())
  {
    max = query.value(0).toInt();
  }
  return max;
 }
 bool settingsDb::compareQVariants(QVariant v1, QString t1, int flags1, QVariant v2, QString t2, int flags2)
 {

  QByteArray ba1 = v1.toByteArray();
  QByteArray ba2 = v2.toByteArray();
  if(flags1 > 0 && flags1 != flags2)
   return false;
  if(ba1.count() == ba2.count() && v1 == v2 && t1 == t2)
   return true;
  else
   return false;

 }
 bool settingsDb::mustBeSerialized(QVariant v)
 {
  QVariant::Type t = v.type();
  switch(t)
  {
  case QMetaType::QPoint:
  case QMetaType::QSize:
  case QMetaType::QRect:
  case QMetaType::QPointF:
  case QMetaType::QRectF:
  case QMetaType::QPen:
  case QMetaType::QStringList:
  case QMetaType::QVariantList:
  case QMetaType::QVariantMap:

   return true;
  default:
   break;
  }
  return false;
 }
 bool settingsDb::mustBeSerialized(const QString typeName) const
 {
  QVariant::Type t = QVariant::nameToType(typeName.toLatin1());
  switch(t)
  {
  case QMetaType::QPoint:
  case QMetaType::QSize:
  case QMetaType::QRect:
  case QMetaType::QPointF:
  case QMetaType::QRectF:
  case QMetaType::QPen:
  case QMetaType::QStringList:
  case QMetaType::QVariantList:
  case QMetaType::QVariantMap:
   return true;
  default:
   break;
  }
  return false;
 }

 // serialize QVariantTypes that must be serialized
 QByteArray settingsDb::QVariantToByteArray(QVariant v)
 {
  QPoint pt;
  QSize sz;
  QRect rect;
  QPointF ptF;
  QRectF  rectF;
  QStringList stringList;
  QVariantList qvList;
  QVariantMap qvMap;

  QByteArray ba;
  QBuffer buf(&ba);
  QDataStream stream(&buf);
  stream.setByteOrder(QDataStream::BigEndian);
  buf.open(QIODevice::WriteOnly);
  QString type = v.typeName();
  QVariant::Type t = v.type();
  switch (t)
  {
  case QMetaType::QPoint:
   pt = v.toPoint();
   stream << pt;
   break;
  case QMetaType::QSize:
   sz = v.toSize();
   stream << sz;
   break;
  case QMetaType::QRect:
   rect = v.toRect();
   stream << rect;
   break;
  case QMetaType::QPointF:
   ptF = v.toPointF();
   stream << ptF;
   break;
  case QMetaType::QRectF:
   rectF = v.toRectF();
   stream << rectF;
   break;
  case QMetaType::QStringList:
   stringList = v.toStringList();
   stream << stringList;
   break;
  case QMetaType::QVariantList:
   qvList = v.toList();
   stream << qvList;
   break;
  case QMetaType::QVariantMap:
   qvMap = v.toMap();
   stream << qvMap;
   break;
  default:
   break;
  }
  return ba;
 }
 QVariant settingsDb::ByteArrayToQVariant(QByteArray v, QString type) const
 {
  QPoint pt;
  QSize sz;
  QRect rect;
  QPointF ptF;
  QRectF rectF;
  QStringList list;
  QVariantList qvList;
  QVariantMap qvMap;

  QVariant::Type t = QVariant::nameToType(type.toLatin1());

  QByteArray ba = v;
  QDataStream stream(ba);
  stream.setByteOrder(QDataStream::BigEndian);
  switch(t)
  {
  case QMetaType::QPoint:
   stream >> pt;
   return (QVariant)pt;
  case QMetaType::QSize:
   stream >> sz;
   return (QVariant)sz;
  case QMetaType::QRect:
   stream >>rect;
   return (QVariant)rect;
  case QMetaType::QPointF:
   stream >>ptF;
   return (QVariant)ptF;
  case QMetaType::QRectF:
   stream >> rectF;
   return (QVariant)rectF;
  case QMetaType::QStringList:
   stream >> list;
   return (QVariant)list;
  case QMetaType::QVariantList:
   stream >> qvList;
   return (QVariant)qvList;
  case QMetaType::QVariantMap:
   stream >> qvMap;
   return (QVariant)qvMap;
  default:
   break;
  }
  return QVariant();
 }
 QSqlDatabase settingsDb::getDb()
 { return db;}
 void settingsDb::closeDb()
 {
  db.close();
 }
// QByteArray settingsDb::IntListToByteArray(QList<int> list)
// {
//  QByteArray ba;
//  QBuffer buf(&ba);
//  QDataStream stream(&buf);
//  stream.setByteOrder(QDataStream::BigEndian);
//  buf.open(QIODevice::WriteOnly);
//  foreach(int i, list)
//   stream << i;
//  return ba;
// }

// QList<int> settingsDb::ByteArrayToIntList(QByteArray v, QString type) const
// {
//  QList<int> list;
//  QBuffer buf(&v);
//  QDataStream stream(&buf);
//  stream.setByteOrder(QDataStream::BigEndian);
//  buf.open(QIODevice::ReadOnly);
//  while(!stream.atEnd())
//  {
//   int i;
//   stream >>i;
//   list << i;
//  }
//  return list;
// }
 void settingsDb::setDb(QSqlDatabase db)
 {
  this->db = db;
 }
 QVariant settingsDb::getTableInfo(QString uuid, QString table, QString type)
 {
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  s_Command_String = QString("create table if not exists `tableInfo` (`uuid` text not null, `table` text  not null, `type` text not null, `value`  BLOB,  CONSTRAINT pk PRIMARY KEY (`uuid`, `table`, `type`))");
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return QVariantList();
  }
  QVariant data;
  s_Command_String = "select `value` from `tableInfo` where `uuid` = :uuid and `table` = :table and `type` = :type";
  query.prepare(s_Command_String);
  query.bindValue(":uuid", uuid);
  query.bindValue(":table", table);
  query.bindValue(":type", type);
  if (!query.exec())
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return QVariantList();
  }
  while(query.next())
  {
   if(type == "columns" || type == "mapped")
    data = ByteArrayToQVariant(query.value(0).toByteArray(),"QVariantList");
   else
    data = query.value(0);
  }
  return (QVariant)data;
 }
 bool settingsDb::saveTableInfo(QString uuid, QString table, QString type, QVariant data)
 {
  if(uuid.isEmpty())
  {
   qDebug()<<QString("-W-> settingsDb:saveTableInfo uuid is blank! Table= %1").arg(table);
   return false;
  }
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  s_Command_String = QString("create table if not exists `tableInfo` (`uuid` text not null, `table` text  not null, `type` text not null, `value`  BLOB,  CONSTRAINT pk PRIMARY KEY (`uuid`, `table`, `type`))");
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return false;
  }
  int count=0;
  s_Command_String = "select count(*) from `tableInfo` where `uuid` = :uuid and `table` = :table and `type` = :type";
  query.prepare(s_Command_String);
  query.bindValue(":uuid", uuid);
  query.bindValue(":table", table);
  query.bindValue(":type", type);
  if (!query.exec())
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return false;
  }
  while(query.next())
  {
   count = query.value(0).toInt();
  }
  if(count == 0)
  {
   s_Command_String = "insert into `tableInfo` (`uuid`, `table`, `type`, `value`) VALUES( :uuid, :table, :type, :value)";
   query.prepare(s_Command_String);
   query.bindValue(":uuid", uuid);
   query.bindValue(":table", table);
   query.bindValue(":type", type);
   if(type == "columns" || type == "mapped")
    query.bindValue(":value", QVariantToByteArray(data));
   else
    query.bindValue(":value", data);
   if (!query.exec())
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return false;
   }
   return true;
  }
  else
  {
   QByteArray currVal;
   s_Command_String = "select `value` from `tableInfo` where `uuid` = :uuid and `table` = :table and `type` = :type";
   query.prepare(s_Command_String);
   query.bindValue(":uuid", uuid);
   query.bindValue(":table", table);
   query.bindValue(":type", type);
   if (!query.exec())
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return false;
   }
   while(query.next())
   {
    currVal = query.value(0).toByteArray();
   }
   QByteArray newVal;
   if(type == "columns" || type == "mapped")
    newVal = QVariantToByteArray(data);
   else
    newVal = data.toByteArray();
   if(!compareByteArrays(currVal, newVal))
   {
    s_Command_String = "update `tableInfo` set `value` = :value where `uuid` = :uuid and `table` = :table and `type` = :type";
    query.prepare(s_Command_String);
    query.bindValue(":uuid", uuid);
    query.bindValue(":table", table);
    query.bindValue(":type", type);
    query.bindValue(":value", newVal);
    if (!query.exec())
    {
     qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return false;
    }
   }
  }
  return true;
 }
 // to be used when deleting a connection.
 bool settingsDb::deleteTableInfo(QString uuid)
 {
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  s_Command_String = QString("delete from `tableInfo` where `uuid` = %1").arg(uuid);
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return false;
  }
  return true;
 }

 bool settingsDb::compareByteArrays(QByteArray ba1, QByteArray ba2)
 {
  if(ba1.count() != ba2.count())
   return false;
  for(int i=0; i<ba1.count(); i++)
  {
   if(ba1[i] != ba2[i])
    return false;
  }
  return true;
 }
 bool settingsDb::checkMaster(int i_debug)
 {
  QString s_Command_String;
  QSqlQuery query = QSqlQuery(db);
  QStringList list;
  int count=0;
  s_Command_String = "select groups from `master`;";
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return false;
  }
  while(query.next())
  {
   list.append(query.value(0).toString());
  }
  foreach(QString table, list)
  {
   s_Command_String = QString("pragma table_info(`%1`)").arg(table);
   count=0;
   if (!query.exec(s_Command_String))
   {
    qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
    qDebug()<<s_Command_String;
    return false;
   }
   while(query.next())
   {
    count++;
    if(i_debug > 1)
     qDebug()<< QString("%1|%2|%3|%4|%5|%6")
               .arg(query.value(0).toString())
               .arg(query.value(1).toString())
               .arg(query.value(2).toString())
               .arg(query.value(3).isNull()?"":query.value(3).toString())
               .arg(query.value(4).isNull()?"":query.value(4).toString())
               .arg(query.value(5).isNull()?"":query.value(5).toString());
   }
   if(i_debug > 0)
    qDebug()<< QString("table %1 column count %2").arg(table).arg(count);
   if(count == 0)
   {
    s_Command_String = QString("delete from `master` where `groups` = '%1'").arg(table);
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return false;
    }
   }
  }
  QStringList tableList;
  s_Command_String = "SELECT name FROM " \
          "(SELECT * FROM sqlite_master UNION ALL " \
           "SELECT * FROM sqlite_temp_master) " \
          "WHERE type='table' " \
       "ORDER BY name";
  if (!query.exec(s_Command_String))
  {
   qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
   qDebug()<<s_Command_String;
   return false;
  }
  while(query.next())
  {
   tableList.append(query.value(0).toString());
  }
  foreach(QString table, tableList)
  {
   if(table == "master" || table == "tableInfo")
    continue;
   bool bFound = false;
   foreach(QString group, list)
   {
    if(table == group)
    {
     bFound = true;
     break;
    }
   }
   if(!bFound)
   {
    qDebug()<< QString("-W-> settingsdb: database %1 doesn't need table `%2`").arg(s_fileName).arg(table);
    s_Command_String = QString("Drop table `%1`").arg(table);
    if (!query.exec(s_Command_String))
    {
     qDebug()<< QString("-E-> error %1 line %2").arg(query.lastError().text()).arg(__LINE__);
     qDebug()<<s_Command_String;
     return false;
    }
   }
  }
  return true;
 }
//}
