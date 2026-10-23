#include "exportsql.h"
#include "qsqlerror.h"
#include <QMessageBox>
#include "sql.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

ExportSql::ExportSql( bool bDropTable, QObject *parent) :
    QObject(parent)
{
 config = Configuration::instance();
 bOverride = false;
 m_parent = parent;
 this->bDropTables = bDropTable;
 srcConn = config->currConnection;
 tgtConn = NULL;
}

ExportSql::~ExportSql()
{
 if(logfile)
 {
  logfile->flush();
  logfile->close();
  logfile = nullptr;
  stream = nullptr;
 }
}

void ExportSql::setTargetConn(Connection* tgtConn)
{
 this->tgtConn = tgtConn;
}

bool ExportSql::setIdentityInsert(QString table, bool b )
{
 if(identityInsertTable == table)
  return true;
 if(!identityInsertTable.isEmpty() && !SQL::instance()->executeCommand(QString("SET IDENTITY_INSERT %1 %2")
                                    .arg(identityInsertTable, "OFF"), _targetDb))
  throw IllegalArgumentException("Error setting identity_insert off");
 bool ret = SQL::instance()->executeCommand(QString("SET IDENTITY_INSERT %1 %2").arg(table, b?"on":"off"), _targetDb);
 if(ret)
  identityInsertTable = table;
 return ret;
}

void ExportSql::setOverride(QDateTime ovrTs)
{
 bOverride = true;
 strOverrideTs = "'" + ovrTs.toString("yyyy/MM/dd hh:mm:ss") + "'";
 overrideTs = ovrTs;
}
void ExportSql::setNoDelete(bool bFlag)
{
 bNoDeletes = bFlag;
}

bool ExportSql::openDb()
{
 if(config->currCity->curExportConnId == config->currCity->curConnectionId)
 {
  qDebug()<< "Source and target are the same";
  QMessageBox::warning(nullptr, tr("Warning"), tr("Source and target are the same!"));
  return false;
 }
 {
  //srcDb =
     srcDb = QSqlDatabase::database(SQL::_connectionName);
  qInfo() << "source database. " << srcDb.connectionName() << " " << srcDb.driverName() << " " << srcDb.databaseName();

  srcConn = config->currCity->connections.at(config->currCity->curConnectionId);
  tgtConn = config->currCity->connections.at(config->currCity->curExportConnId);

  qDebug()<< "Export from '" +srcConn->description() + "' --> '" + tgtConn->description() + "'" ;
  if(!_targetDb.connectionName().contains("export"))
  {
#if 1
    if(tgtConn->servertype()=="MimerSql")
    {
        QString server = tgtConn->host();
        QMap<QString,QString> map = Connection::decodeConnectionString(tgtConn->connectString());
        server = map.value("Node");
        Connection::connectToCityMimer(map.value("Database"), server,"export");
        _targetDb = QSqlDatabase::database("export");
        // === FORCE HARDWARE AUTO-COMMIT AT DRIVER LAYER ===
        // This removes the need to manually track _targetDb.commit() balances
        //_targetDb.setConnectOptions("AUTOCOMMIT=ON");
        tgtConn->setConnectionName(_targetDb.connectionName());
        //tgtConn->setDb(_targetDb);
    }
    else
    {
        if (QSqlDatabase::contains("export"))
        {
            QSqlDatabase oldDb = QSqlDatabase::database("export");
            if (oldDb.isOpen()) oldDb.close();
        }
        QSqlDatabase::removeDatabase("export");

       _targetDb = QSqlDatabase::addDatabase(tgtConn->driver(), "export");

        tgtConn->configureDb(_targetDb, tgtConn);
    }
    tgtDbType = tgtConn->servertype();
    if(config->currConnection->connectString().isEmpty() && tgtConn->connectionType() != "ODBC")
    {
       _targetDb.setUserName(tgtConn->userId());
       _targetDb.setPassword(tgtConn->pwd());
    }
    tgtConn->setConnectionName("export");

   if(! _targetDb.open())
   {
    qDebug()<< _targetDb.lastError().text();
    QMessageBox::critical(nullptr, tr("Error"), tr("An error occured opening connection '%1'. "
                          "The server returned  %2 %3").arg(tgtConn->description()).arg(_targetDb.lastError().driverText()).arg(_targetDb.lastError().databaseText()));
    return false;
   }
   else
   {
    if(tgtConn->servertype() != "Local" )
    {
     if(tgtConn->database() != "default" && tgtConn->database() != tgtConn->defaultSqlDatabase())
     {
      QSqlQuery* query = new QSqlQuery(_targetDb);
      query->setForwardOnly(true);
      QString cmd;
      if(tgtConn->servertype() == "PostgreSQL") // PostgreSQL doesn't have a 'use' command.
          return true;
      if(tgtConn->servertype() == "MySql")
        cmd = QString("use %1").arg(tgtConn->database());
      else
       cmd = QString("use [%1]").arg(tgtConn->database());

      if(!query->exec(cmd))
      {
       SQLERROR_E(std::move(query));
       _targetDb.close();
       return false;
      }
      if(tgtConn->servertype() == "MsSql")
       cmd = "select db_name()";
      else
       cmd = "select Database()";
      if(!query->exec(cmd))
      {
       SQLERROR_E(std::move(query));
       _targetDb.close();
       return false;
      }
      while(query->next())
      {
       QString dbName = query->value(0).toString();
       //if(dbName != tgtConn->useDatabase())
       if(dbName != tgtConn->defaultSqlDatabase())
        return false;
      }
     }
    }
   }
   QStringList tableList;
   {
    SQL* sql = SQL::instance();
    sql->setConfig(config);

    tableList = _targetDb.tables();
    if(!sql->doesColumnExist("Segments", "pointArray"))
    {
     sql->addColumn("Segments", "pointArray", "text");
    }
    if(!sql->doesColumnExist("Segments", "street"))
    {
     sql->addColumn("Segments", "street", "text");
    }
   }
#else
  targetDb = tgtConn->configure();

#endif
  }
 }

 beginTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

 return true;
}
bool ExportSql::exportAll()
{
 if(!openDb()) return false;
 exportTable("Parameters");
 exportTable("Companies");
 exportTable("TractionTypes");
 exportTable("AltRoute");
 //exportTable("Intersections");
 //exportLineSegments();
 exportTable("StreetDef");
 exportTable("Segments");
 exportTable("RouteName");
 exportTable("Routes");
 exportTable("Terminals");
 exportTable("Comments");
 exportTable("RouteComments");
 exportTable("Stations");
 exportTable("RouteSeq");
 return true;
}
 bool ExportSql::exportRoutes(RouteData rd)
 {
  SQL* sql = SQL::instance();
      if(!openDb()) return false;

  updateTimestamp("Routes");

  added=0;
  updated=0;
  deleted=0;
  errors=0;
  notUpdated=0;

  //getCount("Routes");
  QString commandText;

  sql->beginTransaction("Route");

  commandText = "delete from Routes where route=" +  QString("%1").arg(rd.route()) +" and name='"+ rd.routeName()
    + "' and startDate='" + rd.startDate().toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate().toString("yyyy/MM/dd") +"'";
  QSqlQuery* query = new QSqlQuery(_targetDb);
  query->prepare(commandText);
  bool bQuery = query->exec();
  if(!bQuery)
  {
   QSqlError err = query->lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   //db.close();
   if(!Retry(&srcDb, query, ""))
       exit(EXIT_FAILURE);
  }
  int rowsDeleted = query->numRowsAffected();


  if(srcConn->servertype() == "MsSql")
   commandText = "select route, LTRIM(RTRIM(name)), startDate, endDate, lineKey, companyKey,"
                 "tractionType, Direction, next, prev, normalEnter, normalLeave,"
                 " reverseEnter, ReverseLeave, sequence, reverseSeq, lastUpdate"
                 " from Routes where route = :route and startDate = :startDate and endDate = :endDate";
  else
   commandText = "select route, TRIM(name), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where route = :route and startDate = :startDate and endDate = :endDate";

  query = new QSqlQuery(srcDb);
  query->prepare(commandText);
  query->bindValue(":route", QString::number(rd.route()));
  query->bindValue(":startDate", rd.startDate().toString("yyyy/MM/dd"));
  query->bindValue(":endDate",rd.endDate().toString("yyyy/MM/dd"));
  bQuery = query->exec();
  if(!bQuery)
  {
      QSqlError err = query->lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      //db.close();
      if(!Retry(&srcDb, query, ""))
          exit(EXIT_FAILURE);
  }

  qint32 route=0, route2=0, lineKey=0, lineKey2=0, companyKey, companyKey2, tractionType, tractionType2,
    next=0, next2=0, prev=0, prev2=0, normalEnter=0, normalEnter2=0, normalLeave=0, normalLeave2=0,
    reverseEnter=0, reverseEnter2=0, reverseLeave=0, reverseLeave2 =0, sequence = -1, sequence2 = -1,
    reverseSeq = -1, reverseSeq2 = -1;
  QString name="", name2="", direction="", direction2="";
  QDateTime lastUpdate, lastUpdate2;
  QDateTime startDate, startDate2, endDate, endDate2;
  while(query->next())
  {
      route = query->value(0).toInt();
      name=query->value(1).toString();
      startDate = query->value(2).toDateTime();
      endDate = query->value(3).toDateTime();
      lineKey = query->value(4).toInt();
      companyKey = query->value(5).toInt();
      tractionType= query->value(6).toInt();
      direction = query->value(7).toString();
      next = query->value(8).toInt();
      prev = query->value(9).toInt();
      normalEnter = query->value(10).toInt();
      normalLeave = query->value(11).toInt();
      reverseEnter = query->value(12).toInt();
      reverseLeave = query->value(13).toInt();
      sequence = query->value(14).toInt();
      reverseSeq = query->value(15).toInt();
      lastUpdate = query->value(16).toDateTime();

      route2 = -1;
      name2 = "??";
      lineKey2 = -1;
      commandText = "select route, name, startDate, endDate, lineKey, companyKey,tractionType,"
                    " Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave,"
                    " sequence, reverseSeq, lastUpdate"
                    " from Routes where route = "+QString("%1").arg(route)+" and name='"+name+ "' and startDate='"+startDate.toString("yyyy/MM/dd")+ "' and endDate='"+endDate.toString("yyyy/MM/dd")+"' and lineKey="+ QString("%1").arg(lineKey);
      QSqlQuery* query2 = new QSqlQuery(_targetDb);
      bQuery = query2->exec(commandText);
      if(!bQuery)
      {
          QSqlError err = query2->lastError();
          qDebug()<< "Error#: " +QString("%1").arg(err.nativeErrorCode());
          qDebug() << err.text() + "\n";
          qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
          //db.close();
          if(!Retry(&_targetDb, query2, commandText))
              exit(EXIT_FAILURE);

      }
      bool bFound = false;
      while(query2->next())
      {
          bFound = true;
          route2 = query2->value(0).toInt();
          name2=query2->value(1).toString();
          startDate2 = query2->value(2).toDateTime();
          endDate2 = query2->value(3).toDateTime();
          lineKey2 = query2->value(4).toInt();
          companyKey2 = query2->value(5).toInt();
          tractionType2= query2->value(6).toInt();
          direction2 = query2->value(7).toString();
          next2 = query2->value(8).toInt();
          prev2 = query2->value(9).toInt();
          normalEnter2 = query2->value(10).toInt();
          normalLeave2 = query2->value(11).toInt();
          reverseEnter2 = query2->value(12).toInt();
          reverseLeave2 = query2->value(13).toInt();
          sequence2 = query->value(14).toInt();
          reverseSeq2 = query->value(15).toInt();
          lastUpdate2 = query->value(16).toDateTime();
      }
      if( bFound && route== route2 && name == name2 && startDate.date() == startDate2.date()
          && endDate.date() == endDate2.date() && lineKey == lineKey2 && companyKey == companyKey2
          && tractionType == tractionType2 && direction == direction2 && next == next2 && prev==prev2
          && normalEnter == normalEnter2 && normalLeave == normalLeave2 && reverseEnter == reverseEnter2
          && reverseLeave == reverseLeave2 && sequence == sequence2 && reverseSeq == reverseSeq2
          && lastUpdate == lastUpdate2)
      {
          notUpdated++;
          sendProgress();
          continue;
      }
      //if(route == route2 && name == name2 && startDate.date().toString("yyyy/MM/dd") == startDate2.date() .toString("yyyy/MM/dd")&& endDate.date().toString("yyyy/MM/dd") == endDate2.date().toString("yyyy/MM/dd") && lineKey == lineKey2)
      if(bFound)
      {
          if(tgtConn->servertype() != "MsSql")
              commandText = "update Routes set companyKey=" + QString("%1").arg(companyKey)
                + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction
                + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev)
                + ",normalEnter=" + QString("%1").arg(normalEnter)
                +",normalLeave=" + QString("%1").arg(normalLeave)
                +",reverseEnter="+ QString("%1").arg(reverseEnter )
                + ",reverseLeave=" + QString("%1").arg(reverseLeave)
                + ",sequence=" + QString("%1").arg(sequence)
                + ",reverseSeq=" + QString("%1").arg(reverseSeq)
                + ",lastUpdate =:lastUpdate where route=" + QString("%1").arg(route) +" and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "' and endDate='" + endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(lineKey);
          else
              commandText = "update Routes set companyKey=" + QString("%1").arg(companyKey)
                + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction
                + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev)
                + ",normalEnter=" + QString("%1").arg(normalEnter)
                +",normalLeave=" + QString("%1").arg(normalLeave)
                +",reverseEnter="+ QString("%1").arg(reverseEnter )
                + ",reverseLeave=" + QString("%1").arg(reverseLeave)
                + ",sequence=" + QString("%1").arg(sequence)
                + ",reverseSeq=" + QString("%1").arg(reverseSeq)
                + ",lastUpdate=:lastUpdate where route=" + QString("%1").arg(route)
                + " and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "'"
                " and endDate='" + endDate.toString("yyyy/MM/dd") + "'"
                " and lineKey=" + QString("%1").arg(lineKey);
          query2->prepare(commandText);
          query2->bindValue(":lastUpdate", lastUpdate);
          bQuery = query2->exec();
          if(!bQuery)
          {
              QSqlError err = query2->lastError();
              qDebug()<< "Error#: " +QString("%1").arg(err.nativeErrorCode());
              qDebug() << err.text() + "\n";
              qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";

              //db.close();
              //exit(EXIT_FAILURE);
              if(!Retry(&_targetDb, query2, ""))
                  errors++;
          }
          else
              updated++;
      }
      else
      {
          if(tgtConn->servertype() != "MsSql")
              commandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,"
                            "tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, "
                            "ReverseLeave, Sequence, Reve4seSeq,"
                             " lastUpdate) "
                             "values("+QString("%1").arg(route) + ",'"+name+ "','"
                             +startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")
                             +"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)
                             +","+QString("%1").arg(tractionType)+",'"+direction+"',"+QString("%1").arg(next)
                             +","+QString("%1").arg(prev)+","+QString("%1").arg(normalEnter)
                             +","+QString("%1").arg(normalLeave)+","+QString("%1").arg(reverseEnter)
                             +","+QString("%1").arg(reverseLeave)
                             +","+QString("%1").arg(sequence)
                             +","+QString("%1").arg(reverseSeq)
                             + ",:lastUpdate)";
          else
              commandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,"
                            "tractionType, Direction, next, prev, normalEnter, normalLeave,"
                            " reverseEnter, ReverseLeave, sequence, ReverseSeq,"
                            " lastUpdate) values("+QString("%1").arg(route) + ",'"+name+ "','"
                            +startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")
                            +"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)
                            +","+QString("%1").arg(tractionType)+",'"+direction+"',"
                            +QString("%1").arg(next)+","+QString("%1").arg(prev)
                            +","+QString("%1").arg(normalEnter)+","+QString("%1").arg(normalLeave)
                            +","+QString("%1").arg(reverseEnter)+","+QString("%1").arg(reverseLeave)
                            +","+QString("%1").arg(sequence) +","+QString("%1").arg(reverseSeq)
                            + ",:lastUpdate)";
          query2->prepare(commandText);
          query2->bindValue(":lastUpdate", lastUpdate);
          bQuery = query2->exec();
          if(!bQuery)
          {
              QSqlError err = query2->lastError();
              qDebug()<< "Error#: " +QString("%1").arg(err.nativeErrorCode());
              qDebug() << err.text() + "\n";
              qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
              qDebug() <<QString("%1").arg(route)+" " +name+" " +startDate.toString("yyyy/MM/dd")+" " +endDate.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey);
              qDebug()<<QString("%1").arg(route2)+" " +name2+" " +startDate2.toString("yyyy/MM/dd")+" " +endDate2.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey2);

              //db.close();
              //exit(EXIT_FAILURE);
              if(!Retry(&_targetDb, query2, ""))
                  errors++;
          }
          else
              added++;
      }
      sendProgress();

  }
 sql->commitTransaction("Routes");
 qDebug()<<"Routes: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
 emit uncheck("chkRoutes");

 return true;

}
bool ExportSql::getCount(QString table, bool bDropTable)
{
 emit(progressMsg("Processing "+ table + " inserts/updates"));
 lastUpdated = QDateTime::fromString("1800/01/01 00:00:01","yyyy/MM/dd hh:mm:ss");
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database(tgtConn->connectionName());
 QSqlQuery* query2 = new QSqlQuery(db);

 if(!bDropTable)
 {
  // we aren't going to drop the table and it exist's
   if(tgtConn->servertype() == "MsSql")
   {
//    commandText = "SELECT 1 "
//            "FROM INFORMATION_SCHEMA.TABLES "
//            "WHERE TABLE_TYPE='BASE TABLE' "
//            "AND TABLE_NAME='" +table +"'";
//    if(!query2->exec(commandText))
//    {
//        SQLERROR_E(std::move(query2));
//    }
//    if(!query2->isValid())
//    {
//       rowsCompleted = 0;
//       return false;
//    }

    commandText ="select max(lastUpdate) from [dbo].[" + table + "]";
   }
   else
    commandText ="select max(lastUpdate) from " + table;
   bool bQuery = query2->exec(commandText);
   if(!bQuery)
   {
    SQLERROR_E(std::move(query2));
    //db.close();
    //if(!Retry(&targetDb, query2, commandText))
     //exit(EXIT_FAILURE);
     //return;
   }
   while(query2->next())
   {
    lastUpdated = query2->value(0).toDateTime();
    strLastUpdated = "'" + lastUpdated.toString("yyyy/MM/dd hh:mm:ss") +"'";
    //strLastUpdated = "'" + query2->value(0).toString() +"'";
    if(!lastUpdated.isValid())
    {
     lastUpdated = QDateTime::fromString("1800/01/01 00:00:01", "yyyy/MM/dd hh:mm:ss");
     qDebug() << QString("lastUpdated read = %1").arg(lastUpdated.toString("yyyy/MM/dd hh:mm:ss"));
     strLastUpdated = "'1800/01/01 00:00:01'";
    }
   }
   if(bOverride)
   {
    strLastUpdated = strOverrideTs;
    lastUpdated = overrideTs;
   }

   qDebug()<< "Table: " + table + " lastUpdate =" + strLastUpdated;
 }
 // get count of rows in source table that have been updated.
 if(bDropTable)
  commandText = "select count(*) from " + table;
 else
  commandText = "select count(*) from " + table + " where lastUpdate > :lastUpdated";
 QSqlDatabase srcDb = QSqlDatabase::database(srcConn->connectionName());
 QSqlQuery* query = new QSqlQuery(srcDb);
 query->prepare(commandText);
 if(!bDropTable)
  query->bindValue(":lastUpdated", lastUpdated);
 bool bQuery = query->exec();
 if(!bQuery)
 {
  QSqlError err = query->lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  //db.close();
  if(!Retry(&srcDb, query, ""))
   exit(EXIT_FAILURE);
 }
 while(query->next())
 {
  rowCount = query->value(0).toInt();
  qDebug()<< QString("Row count = %1 lastUpdate = '%2'").arg(rowCount).arg(lastUpdated.toString("yyyy/MM/dd hh:mm:ss"));
 }
 rowsCompleted = 0;
 return true;
}

void ExportSql::sendProgress()
{
    rowsCompleted++;
    if(rowCount > 0)
        emit progress((rowsCompleted*100)/rowCount);
    //SleeperThread::msleep(10);
    qApp->processEvents(QEventLoop::AllEvents);

}
//void ExportSql::setIdentityInsert(QString table, bool bOn)
//{
//    QString commandText ="SET IDENTITY_INSERT [dbo].[" + table + "] " +(bOn?"ON"?"OFF");
//    QSqlQuery* query2 = new QSqlQuery(targetDb);
//    bool bQuery = query2->exec(commandText);
//    if(!bQuery)
//    {
//        QSqlError err = query2->lastError();
//        qDebug() << err.text() + "\n";
//        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//        //db.close();
//        exit(EXIT_FAILURE);
//    }
//    else
//    {
//        //qDebug() << query2->lastError();
//        qDebug() << commandText;
//    }
//}
void ExportSql::updateTimestamp(QString table)
{
    if(srcConn->servertype() != "MySql")
        return;
    QSqlDatabase  srcDb = QSqlDatabase::database(SQL::_connectionName);

    QString commandText = "update " + table + " set lastupdate = '2000-01-01 00:00:00' where lastUpdate = '0000-00-00 00:00:00'";
    QSqlQuery* query = new QSqlQuery(srcDb);
    bool bQuery = query->exec(commandText);
    if(!bQuery)
    {
        QSqlError err = query->lastError();
        qDebug() << err.text() + "\n";
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        //exit(EXIT_FAILURE);
    }

}
bool ExportSql::Retry(QSqlDatabase *db, QSqlQuery *query, QString commandText)
{
    QSqlError err = query->lastError();
    if(err.type() == QSqlError::NoError)
        return true;
    if(err.type() != QSqlError::ConnectionError)
        return false;
    int retrys = 5;
    while(retrys)
    {
        retrys--;
        qDebug()<<"retrying";

        if(!db->isOpen())
        {
            db->open();
            if(db->isOpenError())
                continue;
        }
        bool bQuery;
        if(commandText.length() == 0)
            bQuery = query->exec();
        else
            bQuery = query->exec(commandText);
        if(bQuery)
            return true;
        err = query->lastError();
        qDebug() << err.text() + "\n";

    }
    qDebug()<< "no more retries.";
    return false;
}
bool ExportSql::createSegmentsTable(QSqlDatabase db, QString dbType)
{

 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Segments", db, dbType);

 if(dbType == "Sqlite")
 {
  commandText = "CREATE TABLE `Segments` (  "
                "`SegmentId` integer NOT NULL primary key AUTOINCREMENT,"
                " `Description` varchar(100) NOT NULL,"
                " `FormatOK` int(1) NOT NULL DEFAULT FALSE"
                " `Tracks` int(2) check(`tracks` in (1,2)) NOT NULL DEFAULT 1,"
                " `Street` varchar(100) NOT NULL DEFAULT '',"
                " `StreetId` int(11) DEFAULT NULL,"
                " `StreetSeq` integer DEFAULT 0,"
                " `NewerName` varchar(60) NOT NULL DEFAULT '',"
                " `Location` varchar(30) NOT NULL DEFAULT '',"
                " `Type` int(11) NOT NULL DEFAULT 0,"
                " `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0,"
                " `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0,"
                " `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0,"
                " `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0,"
                " `Length` decimal(15,5) NOT NULL DEFAULT 0,"
                " `StartDate` date NOT NULL DEFAULT '1800-01-01',"
                " `DoubleDate` date ,"
                " `EndDate` date NOT NULL DEFAULT '1800-01-01',"
                " `Direction` varchar(6) ,"
                " `OneWay` char(1) NOT NULL DEFAULT 'N',"
                " `Points` int(11) NOT NULL default 0,"
                " `PointArray` text,"
                " `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP)"
                " FOREIGN KEY (`StreetId`, `StreetSeq`) REFERENCES `StreetDef` (`StreetId`,`Seq`) ON UPDATE CASCADE)";
 }
 else if(dbType == "MySql")
  commandText = "CREATE TABLE IF NOT EXISTS `Segments` ("
                "`SegmentId` int(11) NOT NULL AUTO_INCREMENT,"
                " `Description` varchar(100) NOT NULL,"
                " `FormatOK` int(1) NOT NULL DEFAULT FALSE,"
                " `OneWay` char(1) ,"
                " `Tracks` int(2) NOT NULL DEFAULT 2,"
                " `Street` varchar(100),"
                " `StreetId` int(11) DEFAULT NULL,"
                " `StreetSeq` integer DEFAULT 0,"
                " `NewerName` varchar(100) NOT NULL,"
                " `Location` varchar(30),"
                " `Type` int NOT NULL Default 0,"
                " `StartLat` decimal(15,13) NOT NULL,"
                " `StartLon` decimal(15,13) NOT NULL,"
                " `EndLat` decimal(15,13) NOT NULL,"
                " `EndLon` decimal(15,13) NOT NULL,"
                " `Length` decimal(15,5) NOT NULL,"
                " `Points` int(11) NOT NULL default 0,"
                " `StartDate` date NOT NULL,"
                " `DoubleDate` date,"
                " `EndDate` date NOT NULL, "
                " `Direction` varchar(6),"
                " `pointArray` varchar(21844) NOT NULL,"
                " `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,PRIMARY KEY (`SegmentId`)) ENGINE=InnoDB AUTO_INCREMENT=1116 DEFAULT CHARSET=latin1";
 else  if(dbType == "PostgreSQL")
 {
     commandText = "CREATE TABLE Segments (  \
                   SegmentId integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
                   Description varchar(100) NOT NULL, \
                   FormatOK smallint NOT NULL DEFAULT 0, \
                   Tracks smallint check(tracks in (1,2)) NOT NULL DEFAULT 1, \
                   Street  varchar(100) NOT NULL DEFAULT '', \
                   StreetId  integer DEFAULT NULL, \
                   StreetSeq integer DEFAULT 0, \
                   NewerName  varchar(60) NOT NULL DEFAULT '', \
                   Location  varchar(30) NOT NULL DEFAULT '', \
                   Type  integer NOT NULL DEFAULT 0, \
                   StartLat decimal(15,13) NOT NULL DEFAULT 0.0, \
                   StartLon decimal(15,13) NOT NULL DEFAULT 0.0, \
                   EndLat decimal(15,13) NOT NULL DEFAULT 0.0, \
                   EndLon decimal(15,13) NOT NULL DEFAULT 0.0, \
                   Length decimal(15,5) NOT NULL DEFAULT 0, \
                   StartDate date NOT NULL DEFAULT '1800-01-01', \
                   DoubleDate date , \
                   EndDate date NOT NULL DEFAULT '1800-01-01', \
                   Direction varchar(6) , \
                   OneWay char(1) NOT NULL DEFAULT 'N', \
                   Points  integer NOT NULL default 0, \
                   PointArray  text, \
                   lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP)";
                   // CONSTRAINT Segments_ibfk_1 FOREIGN KEY (StreetId) REFERENCES StreetDef (StreetId))";
 }
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_PADDING ON;";
  commandText.append("CREATE TABLE [dbo].[Segments]("\
    "[SegmentId] [int] IDENTITY(1,1) NOT NULL,"\
    "[Description] [varchar](100) NOT NULL,"\
    "[FormatOK] int NOT NULL DEFAULT 0," \
    "[OneWay] [char](1),"\
    "[Tracks] [int] NOT NULL,"\
    "[Street] [varchar](100) NULL,"\
    "[StreetId] [int] DEFAULT NULL," \
    "[StreetSeq] [int] DEFAULT 0," \
    "[NewerName] [varchar](60)," \
    "[Location] [varchar](30)," \
    "[Type] [int] NOT NULL,"\
    "[StartLat] [decimal](15, 13) NOT NULL,"\
    "[StartLon] [decimal](15, 13) NOT NULL,"\
    "[EndLat] [decimal](15, 13) NOT NULL,"\
    "[EndLon] [decimal](15, 13) NOT NULL,"\
    "[Length] [decimal](15, 5) NOT NULL,"\
    "[Points] [int] NOT NULL,"\
    "[StartDate] [date] NOT NULL,"\
    "[DoubleDate] [date],"\
    "[EndDate] [date] NOT NULL,"\
    "[Direction] [varchar](6),"\
    "[PointArray] [varchar](max) NULL,"\
    "[lastUpdate] [datetime] NOT NULL,"\
 "CONSTRAINT [pk_Segments] PRIMARY KEY CLUSTERED"\
"("\
    "[SegmentId] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY];");
  commandText.append("SET ANSI_PADDING OFF;");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('N') FOR [OneWay];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT (2) FOR [Tracks];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [Type];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('') FOR [Description];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('') FOR [Street];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('') FOR [Location];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [StartLat];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [StartLon];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [EndLat];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [EndLon];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_dbo_Segments_Length]  DEFAULT ((0)) FOR [Length];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [Points];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_StartDate]  DEFAULT ('1899-01-01') FOR [StartDate];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_EndDate]  DEFAULT ('1899-01-01') FOR [endDate];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_Direction]  DEFAULT ('') FOR [Direction];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_PointArray]  DEFAULT ('') FOR [pointArray];");
  commandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }  else  if(dbType == "MimerSql")
 {
     // 1. Create a dedicated sequence for the Segments table autoincrement
     QSqlQuery countQuery(srcDb);
         int nextId = 1;
         if (countQuery.exec("SELECT MAX(SegmentId) FROM Segments") && countQuery.next()) {
             nextId = countQuery.value(0).toInt() + 1;
             if (nextId < 1) nextId = 1;
         }

         QString createSeq = QString("CREATE SEQUENCE segments_seq START WITH %1 INCREMENT BY 1;").arg(nextId);
         QSqlQuery seqQuery(db);
         seqQuery.exec(createSeq);

     // 2. Define the main table statement utilizing the sequence default
     commandText = "CREATE TABLE Segments (  \
                   SegmentId integer DEFAULT NEXT VALUE FOR segments_seq PRIMARY KEY,\
                   Description nvarchar(100) NOT NULL, \
                   FormatOK smallint NOT NULL DEFAULT 0, \
                   Tracks smallint check(tracks in (1,2)) NOT NULL DEFAULT 1, \
                   Street  nvarchar(100) NOT NULL DEFAULT '', \
                   StreetId  integer DEFAULT NULL, \
                   StreetSeq integer DEFAULT 0, \
                   NewerName  nvarchar(60) NOT NULL DEFAULT '', \
                   Location  nvarchar(30) NOT NULL DEFAULT '', \
                   Type  integer NOT NULL DEFAULT 0, \
                   StartLat decimal(15,13) NOT NULL DEFAULT 0.0, \
                   StartLon decimal(15,13) NOT NULL DEFAULT 0.0, \
                   EndLat decimal(15,13) NOT NULL DEFAULT 0.0, \
                   EndLon decimal(15,13) NOT NULL DEFAULT 0.0, \
                   Length decimal(15,5) NOT NULL DEFAULT 0, \
                   StartDate date NOT NULL DEFAULT DATE'1800-01-01', \
                   DoubleDate date , \
                   EndDate date NOT NULL DEFAULT DATE'1800-01-01', \
                   Direction varchar(6) , \
                   OneWay char(1) NOT NULL DEFAULT 'N', \
                   Points  integer NOT NULL default 0, \
                   PointArray  nclob, \
                   lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP)";
                   // CONSTRAINT Segments_ibfk_1 FOREIGN KEY (StreetId) REFERENCES StreetDef (StreetId))";
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating Segments: %1").arg(query->lastError().text()));
 }
 // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
 if (dbType == "MimerSql") {
     qDebug() << "Permanently securing schema geometry layout to disk.";
     db.commit();
 }
 delete query;
 return true;
}

QString ExportSql::getTableSeqName(QString table, QSqlDatabase db)
{
    QSqlQuery query(db);
    QString commandText = QString ("SELECT "
                                   "COLUMN_NAME,"
                                   /* Extracts the name following 'NEXT VALUE FOR '*/
                                   "SUBSTRING(COLUMN_DEFAULT FROM POSITION('NEXT VALUE FOR ' IN UPPER(COLUMN_DEFAULT)) + 15) AS SEQUENCE_NAME "
                               "FROM "
                                   "INFORMATION_SCHEMA.COLUMNS "
                               "WHERE "
                                   "TABLE_SCHEMA = CURRENT_USER "
                                   "AND TABLE_NAME = %1 " /* Remember: Mimer stores unquoted names in UPPERCASE*/
                                   "AND UPPER(COLUMN_DEFAULT) LIKE '%NEXT VALUE FOR%';").arg(table.toUpper());
    bool  bQuery = query.exec(commandText);
    if(!bQuery)
    {
     //SQLERROR_E(std::move(query));
        qDebug() << query.lastError().text();
     return "";
    }
    return query.value(0).toString();

}

bool ExportSql::dropSequence(QString sequence, QSqlDatabase db)
{
    QSqlQuery* query = new QSqlQuery(db);
    QString commandText = QString ("DROP SEQUENCE %1 CASCADE").arg(sequence);
    bool  bQuery = query->exec(commandText);
    if(!bQuery)
    {
     SQLERROR_E(std::move(query));
     return false;
    }
    return true;
}

bool ExportSql::viewExists(const QString &viewName, QSqlDatabase db)
{
    QSqlQuery query(db);
    // Mimer SQL is case-insensitive for unquoted names but stores them in UPPERCASE
    QString upperViewName = viewName.toUpper();

    query.prepare("SELECT 1 FROM INFORMATION_SCHEMA.VIEWS "
                  "WHERE VIEW_SCHEMA = CURRENT_USER "
                  "AND VIEW_NAME = :viewName;");
    query.bindValue(":viewName", upperViewName);

    if (query.exec() && query.next()) {
        return true; // The view exists
    }
    return false; // The view does not exist
}

bool ExportSql::dropTable(QString table, QSqlDatabase db, QString dbType)
{
 QStringList tables = db.tables();
 if(!tables.contains(table,Qt::CaseInsensitive)){
  qDebug() << "drop table ok as table " << table << " not present!";
  return true;
 }
 qWarning() << "dropping table: " << table;
 if(table == "Routes")
     qDebug() << "debug stop";
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 if(dbType == "Sqlite")
  commandText = QString("drop table if exists '%1'").arg(table);
 else if(dbType == "MySql")
  commandText = QString("drop table if exists `%1`").arg(table);
 else if(dbType == "PostgreSQL")
  commandText = QString("drop table if exists %1").arg(table.toLower());
 else if(dbType == "MimerSql")
     commandText = QString("drop table %1").arg(table.toLower());
 else
  commandText = QString("IF OBJECT_ID(N'dbo." + table +"', N'U') IS NOT NULL "\
                        "DROP TABLE [dbo].["+table +"];");
 bQuery = query->exec(commandText);
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  return false;
 }
 return true;
}

bool ExportSql::dropRoutes()
{
 QSqlDatabase  srcDb = QSqlDatabase::database(SQL::_connectionName);
 if(!openDb()) return false;

 return dropTable("Routes", _targetDb, tgtConn->servertype());
}

bool ExportSql::dropStations()
{
 QSqlDatabase  srcDb = QSqlDatabase::database(SQL::_connectionName);
 if(!openDb()) return false;

 return dropTable("Stations", _targetDb, tgtConn->servertype());
}

bool ExportSql::dropRouteComments()
{
 QSqlDatabase  srcDb = QSqlDatabase::database(SQL::_connectionName);
 if(!openDb()) return false;

 return dropTable("RouteComments", _targetDb, tgtConn->servertype());
}

bool ExportSql::createRouteNameTable(QSqlDatabase db, QString dbType)
{
    if(dbType == "Sqlite")
        return SQL::instance()->executeScript(":/sql/create_routeName.sql", db);
    if(dbType == "MySql")
        return SQL::instance()->executeScript(":/sql/MySql/mySql_create_routeName.sql", db);
    if(dbType == "MsSql")
        return SQL::instance()->executeScript(":/sql/MsSql/mssql_create_routeName.sql", db);
    if(dbType == "PostgreSQL")
    {
        QString commandString = "CREATE TABLE RouteName  (\
           RouteId  integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
           Name  varchar(140) NOT NULL,\
           lastUpdate  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
          UNIQUE (Name ));";

        return SQL::instance()->executeCommand(commandString, db);

    } else if(dbType == "MimerSql")
    {
        // 1. Create a dedicated sequence for the AltRoute table autoincrement
        QString createSeq = "CREATE SEQUENCE routename_seq START WITH 1 INCREMENT BY 1;";
        QSqlQuery seqQuery(db);
        seqQuery.exec(createSeq);

        // 2. Define the main table statement utilizing the sequence default
        QString commandString = "CREATE TABLE RouteName  (\
           RouteId  integer DEFAULT NEXT VALUE FOR routename_seq PRIMARY KEY,\
           Name  varchar(140) NOT NULL,\
           lastUpdate  timestamp NOT NULL DEFAULT LOCALTIMESTAMP,\
          UNIQUE (Name ));";

        if(SQL::instance()->executeCommand(commandString, db))
        {
            // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
            if (dbType == "MimerSql") {
                qDebug() << "Permanently securing schema geometry layout to disk.";
                db.commit();
            }
            return true;
        }
    }
    return false;
}
bool ExportSql::createRouteTable(QSqlDatabase db, QString dbType)
{
 if(dbType == "Sqlite")
  return SQL::instance()->executeScript(":/sql/SqLite3/sqlite3_create_routes.sql", db);
 if(dbType == "MySql")
  return SQL::instance()->executeScript(":/sql/MySql/mysql_create_routes.sql", db);
 if(dbType == "MsSql")
  return SQL::instance()->executeScript(":/sql/MsSql/mssql_create_routes.sql", db);
 if(dbType == "PostgreSQL")
 {
 QString commandString = "CREATE TABLE  Routes  (\
              Route   integer NOT NULL,\
              RouteId    integer,\
              StartDate date NOT NULL,\
              EndDate  date NOT NULL,\
              LineKey   integer NOT NULL,\
              OneWay  char(1) check( oneWay  in ('Y','N',' '))  NOT NULL default ' ',\
              TrackUsage  text check( TrackUsage  in ('B', 'L', 'R', ' ')) NOT NULL default ' ',\
              CompanyKey   integer NOT NULL DEFAULT 0,\
              TractionType   integer NOT NULL DEFAULT 0,\
              Direction  varchar(6) NOT NULL DEFAULT ' ',\
              Next   integer NOT NULL DEFAULT -1,\
              Prev   integer NOT NULL DEFAULT -1,\
              NormalEnter   integer NOT NULL DEFAULT 0,\
              NormalLeave   integer NOT NULL DEFAULT 0,\
              ReverseEnter   integer NOT NULL DEFAULT 0,\
              ReverseLeave   integer NOT NULL DEFAULT 0,\
              NextR   integer NOT NULL DEFAULT -1,\
              PrevR   integer NOT NULL DEFAULT -1,\
              Sequence   integer NOT NULL DEFAULT -1,\
              ReverseSeq   integer NOT NULL DEFAULT -1,\
              LastUpdate  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
              constraint pk PRIMARY key ( Route , RouteId , CompanyKey , StartDate , EndDate , LineKey ),\
              CONSTRAINT  Routes_ibfk_1  FOREIGN KEY ( LineKey ) REFERENCES  Segments  ( SegmentId ) ON UPDATE RESTRICT ON DELETE RESTRICT,\
              CONSTRAINT  Routes_ibfk_3  FOREIGN KEY ( CompanyKey ) REFERENCES  Companies  ( key ),\
              CONSTRAINT  Routes_ibfk_4  FOREIGN KEY ( tractionType ) REFERENCES  TractionTypes  ( tractionType ),\
              CONSTRAINT  Routes_ibfk_5  FOREIGN KEY ( Route ) REFERENCES  AltRoute  ( route ),\
              CONSTRAINT  Routes_ibfk_6  FOREIGN KEY ( RouteId ) REFERENCES  RouteName  ( routeId ));";
  return SQL::instance()->executeCommand(commandString, db);
 }
 if(dbType == "MimerSql")
 {
    QString commandString = "CREATE TABLE  Routes  (\
              Route   integer NOT NULL,\
              RouteId    integer,\
              StartDate date NOT NULL,\
              EndDate  date NOT NULL,\
              LineKey   integer NOT NULL,\
              OneWay  char(1) check( oneWay  in ('Y','N',' '))  NOT NULL default ' ',\
              TrackUsage  char(1) check( TrackUsage  in ('B', 'L', 'R', ' ')) NOT NULL default ' ',\
              CompanyKey   integer NOT NULL DEFAULT 0,\
              TractionType   integer NOT NULL DEFAULT 0,\
              Direction  varchar(6) NOT NULL DEFAULT ' ',\
              \"Next\"   integer NOT NULL DEFAULT -1,\
              \"Prev\"   integer NOT NULL DEFAULT -1,\
              NormalEnter   integer NOT NULL DEFAULT 0,\
              NormalLeave   integer NOT NULL DEFAULT 0,\
              ReverseEnter   integer NOT NULL DEFAULT 0,\
              ReverseLeave   integer NOT NULL DEFAULT 0,\
              NextR   integer NOT NULL DEFAULT -1,\
              PrevR   integer NOT NULL DEFAULT -1,\
              Sequence   integer NOT NULL DEFAULT -1,\
              ReverseSeq   integer NOT NULL DEFAULT -1,\
              LastUpdate  timestamp NOT NULL DEFAULT LOCALTIMESTAMP,\
              constraint pk PRIMARY key ( Route , RouteId , CompanyKey , StartDate , EndDate , LineKey ));";
              // CONSTRAINT  Routes_ibfk_1  FOREIGN KEY ( LineKey ) REFERENCES  Segments  ( SegmentId ) ON UPDATE RESTRICT ON DELETE RESTRICT,\
              // CONSTRAINT  Routes_ibfk_3  FOREIGN KEY ( CompanyKey ) REFERENCES  Companies  ( key ),\
              // CONSTRAINT  Routes_ibfk_4  FOREIGN KEY ( tractionType ) REFERENCES  TractionTypes  ( tractionType ),\
              // CONSTRAINT  Routes_ibfk_5  FOREIGN KEY ( Route ) REFERENCES  AltRoute  ( route ),\
              // CONSTRAINT  Routes_ibfk_6  FOREIGN KEY ( RouteId ) REFERENCES  RouteName  ( routeId ));";
  bool bSuccess =  SQL::instance()->executeCommand(commandString, db);
  // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
  // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
 // if (dbType == "MimerSql") {
 //     qDebug() << "Permanently securing schema geometry layout to disk.";
 //     db.commit();
 // }
 return bSuccess;
 }
 return false;
}

bool ExportSql::createRouteCommentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("RouteComments", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE `RouteComments` (  `route` int(6) NOT NULL,  "
                "`date` date NOT NULL,  "
                "`commentKey` integer NOT NULL,  "
                "`companyKey` integer NOT NULL,  "
                "`routeId` integer NOT NULL DEFAULT -1, "
                "`latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',"
                "`longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',"
                "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  "
                "constraint pk PRIMARY KEY (`route`,`date`, `commentKey`)) "
                "CONSTRAINT `RouteComments_ibfk_1` FOREIGN KEY (commentKey) REFERENCES Comments(commentKey) ON DELETE CASCADE)";

 else  if(dbType == "PostgreSQL")
     commandText = "CREATE TABLE RouteComments (\
    route integer NOT NULL,\
    date date NOT NULL,\
    commentKey integer NOT NULL,\
    companyKey integer NOT NULL,\
    routeId integer NOT NULL DEFAULT -1,\
    latitude decimal(15,5) NOT NULL DEFAULT '0.00000',\
    longitude decimal(15,5) NOT NULL DEFAULT '0.00000',\
    lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
    constraint RouteComments_pk PRIMARY KEY (route,date,commentKey))";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `RouteComments` (\
    `route` int(6) NOT NULL,\
    `date` date NOT NULL,\
    `commentKey` int(11) NOT NULL,\
    `companyKey` integer NOT NULL,\
    `routeId` integer NOT NULL DEFAULT -1,\
    `latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',\
    `longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',\
    `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
    PRIMARY KEY (`route`,`date`,`commentKey`)\
  ) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_german1_ci";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;";
  commandText.append("SET QUOTED_IDENTIFIER ON;");
  commandText.append("CREATE TABLE [dbo].[RouteComments]("\
        "[route] [int] NOT NULL,"\
        "[date] [date] NOT NULL,"\
        "[commentKey] [int] NOT NULL,"\
        "[companyKey] [int] NOT NULL,"\
        "[routeId] [int] NOT NULL DEFAULT -1,"\
        "[latitude] decimal(15,5) NOT NULL DEFAULT '0.00000',"\
        "[longitude] decimal(15,5) NOT NULL DEFAULT '0.00000',"\
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_RouteComments] PRIMARY KEY CLUSTERED"\
    "("\
        "[route] ASC,"\
        "[date] ASC, [commentKey] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];");
  commandText.append("ALTER TABLE [dbo].[RouteComments] ADD  CONSTRAINT [DF_RouteComments_companyKey]  DEFAULT ((0)) FOR [companyKey];");
  commandText.append("ALTER TABLE [dbo].[RouteComments] ADD  CONSTRAINT [DF_RouteComments_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  QSqlError err = query->lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  SQLERROR_E(std::move(query));
  //throw Exception(tr("SQL Error creating RouteComments: %1").arg(query->lastError().text()));
  return false;
 }
 // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
 if (dbType == "MimerSql") {
     qDebug() << "Permanently securing schema geometry layout to disk.";
     db.commit();
 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "RouteComments table created.";
 return true;
}

bool ExportSql::createStationsTable(QSqlDatabase db, QString dbType)
{
 if(dbType == "MsSql")
 {
  return SQL::instance()->executeScript(":/sql/MsSql/mssql_create_stations.sql",db);
 }
 else if(dbType == "MySql")
 {
     return SQL::instance()->executeScript(":/sql/MySql/mysql_create_stations.sql",db);
 }
 else if(dbType == "Sqlite")
 {
  return SQL::instance()->executeScript(":/sql/create_stations.sql",db);
 }
 else if(dbType == "PostgreSQL")
 {
   QString commandString = "CREATE TABLE Stations (\
  stationKey integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
  routes varchar(250) NOT NULL DEFAULT '',\
  name varchar(75) NOT NULL,\
  latitude decimal(15,13) NOT NULL DEFAULT 0.0,\
  longitude decimal(15,13) NOT NULL DEFAULT 0.0,\
  startDate date DEFAULT NULL,\
  endDate date DEFAULT NULL,\
  segmentId integer NOT NULL,\
  segments varchar(100) NOT NULL DEFAULT '',\
  infoKey integer NOT NULL DEFAULT -1,\
  markerType varchar(15) default '',\
  routeType integer NOT NULL DEFAULT -1,\
  lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
  constraint main unique (name,startDate,endDate),\
  constraint stationKey UNIQUE (stationKey)\
);";
     return SQL::instance()->executeCommand(commandString,db);
 } else  if(dbType == "MimerSql")
 {
     // 1. Create a dedicated sequence for the Stations table autoincrement
     QSqlQuery countQuery(srcDb);
         int nextId = 1;
         if (countQuery.exec("SELECT MAX(stationKey) FROM Stations") && countQuery.next()) {
             nextId = countQuery.value(0).toInt() + 1;
             if (nextId < 1) nextId = 1;
         }

         QString createSeq = QString("CREATE SEQUENCE stationkey_stations_seq START WITH %1 INCREMENT BY 1;").arg(nextId);
         QSqlQuery seqQuery(db);
         seqQuery.exec(createSeq);

     // 2. Define the main table statement utilizing the sequence default
   QString commandString = "CREATE TABLE Stations (\
      stationKey integer DEFAULT NEXT VALUE FOR stationkey_stations_seq PRIMARY KEY,\
      routes nvarchar(250) NOT NULL DEFAULT '',\
      name nvarchar(75) NOT NULL,\
      latitude decimal(15,13) NOT NULL DEFAULT 0.0,\
      longitude decimal(15,13) NOT NULL DEFAULT 0.0,\
      startDate date DEFAULT NULL,\
      endDate date DEFAULT NULL,\
      segmentId integer NOT NULL,\
      segments varchar(100) NOT NULL DEFAULT '',\
      infoKey integer NOT NULL DEFAULT -1,\
      markerType varchar(15) default '',\
      routeType integer NOT NULL DEFAULT -1,\
      lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP,\
      constraint main unique (name,startDate,endDate)\
    );";
     return SQL::instance()->executeCommand(commandString,db);
 }
 // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
 if (dbType == "MimerSql") {
     qDebug() << "Permanently securing schema geometry layout to disk.";
     db.commit();
 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
 return false;
}

bool ExportSql::createTerminalsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Terminals", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE `Terminals` (  `Route` int(11) NOT NULL,  `Name` varchar(125) NOT NULL,  `StartDate` date NOT NULL,  `EndDate` date NOT NULL,  `StartSegment` int(11) NOT NULL,  `StartWhichEnd` char(1) NOT NULL,  `EndSegment` int(11) NOT NULL,  `EndWhichEnd` char(1) NOT NULL,  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  constraint pk PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`))";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `Terminals` ("\
    "`Route` int(11) NOT NULL,"\
    "`Name` varchar(140) CHARACTER SET latin1 COLLATE latin1_german1_ci NOT NULL,"\
    "`StartDate` date NOT NULL,"\
    "`EndDate` date NOT NULL,"\
    "`StartSegment` int(11) NOT NULL,"\
    "`StartWhichEnd` char(1) NOT NULL,"\
    "`EndSegment` int(11) NOT NULL,"\
    "`EndWhichEnd` char(1) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`)"\
    ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;";
  commandText.append("SET QUOTED_IDENTIFIER ON;");
  commandText.append("SET ANSI_PADDING ON;");
  commandText.append("CREATE TABLE [dbo].[Terminals]("\
        "[Route] [int] NOT NULL,"\
        "[StartDate] [date] NOT NULL,"\
        "[EndDate] [date] NOT NULL,"\
        "[StartSegment] [int] NOT NULL,"\
        "[StartWhichEnd] [char](1) NOT NULL,"\
        "[EndSegment] [int] NOT NULL,"\
        "[EndWhichEnd] [char](1) NOT NULL,"\
        "[name] [varchar](125) NULL,"\
        "[lastUpdate] [datetime] NOT NULL);");
  commandText.append("ALTER TABLE [dbo].[Terminals] ADD  CONSTRAINT [DF_Terminals_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 else  if(dbType == "PostgreSQL")
 {
     commandText = "CREATE TABLE Terminals (\
             Route integer NOT NULL,\
             Name varchar(125) NOT NULL,\
             StartDate date NOT NULL,\
             EndDate date NOT NULL,\
             StartSegment integer NOT NULL,\
             StartWhichEnd char(1) NOT NULL,\
             EndSegment integer NOT NULL,\
             EndWhichEnd char(1) NOT NULL,\
             lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
             constraint Terminals_pk PRIMARY KEY (Route,Name,StartDate,EndDate))";
} else  if(dbType == "MimerSql")
 {
     commandText = "CREATE TABLE Terminals (\
             Route integer NOT NULL,\
             Name varchar(125) NOT NULL,\
             StartDate date NOT NULL,\
             EndDate date NOT NULL,\
             StartSegment integer NOT NULL,\
             StartWhichEnd char(1) NOT NULL,\
             EndSegment integer NOT NULL,\
             EndWhichEnd char(1) NOT NULL,\
             lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP,\
             constraint Terminals_pk PRIMARY KEY (Route,Name,StartDate,EndDate))";
}
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  QSqlError err = query->lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  //throw Exception(tr("SQL Error creating Terminals: %1").arg(query->lastError().text()));
  SQLERROR_E(std::move(query));
 }
 // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
  if (dbType == "MimerSql") {
      qDebug() << "Permanently securing schema geometry layout to disk.";
      db.commit();
  }
  // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "Terminals table created.";
 return true;
}


bool ExportSql::createAltRouteTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("AltRoute", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE `AltRoute` (" \
    "`route` integer NOT NULL primary key AUTOINCREMENT, "\
    "`routePrefix` varchar(10) NOT NULL default '', " \
    "`routeAlpha` varchar(10) NOT NULL, "\
    "`baseRoute` integer NOT NULL DEFAULT 0, " \
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  "
                "CONSTRAINT `index` unique ( routePrefix, routeAlpha) )";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `AltRoute` (" \
    "`route` integer NOT NULL AUTO_INCREMENT, " \
    "`routePrefix` varchar(10) NOT NULL DEFAULT '', "\
    "`routeAlpha` varchar(10) NOT NULL, "\
    "`baseRoute` integer NOT NULL, "\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP," \
    "UNIQUE KEY (`routePrefix`,`routeAlpha`)," \
    "PRIMARY KEY `route` (`route`) "\
    ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
 else  if(dbType == "PostgreSQL")
     commandText = "CREATE TABLE AltRoute (" \
         "route integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY, "\
         "routePrefix varchar(10) NOT NULL default '', " \
         "routeAlpha varchar(10) NOT NULL, "\
         "baseRoute int NOT NULL DEFAULT 0, " \
         "lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  "
         "CONSTRAINT index unique ( routePrefix, routeAlpha) )";
 else if(dbType == "MsSql")
 {

  /****** Object:  Table [dbo].[AltRoute]    Script Date: 03/21/2016 13:22:36 ******/
  commandText =
  "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[AltRoute]("\
   "[route] [int] IDENTITY(1,1) NOT NULL,"\
   "[routePrefix] [varchar](10) NOT NULL,"\
   "[routeAlpha] [varchar](10) NOT NULL,"\
   "[baseRoute] [int] NOT NULL,"\
   "[lastUpdate] [datetime] NOT NULL,"\
   "CONSTRAINT [PK_AltRoute] PRIMARY KEY CLUSTERED "\
  "("\
   "[route] ASC"\
  ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
  ") ON [PRIMARY];"\
  "SET ANSI_PADDING OFF;"\
  "ALTER TABLE [dbo].[AltRoute] ADD  CONSTRAINT [DF_AltRoute_routePrefix]  DEFAULT ('') FOR [routePrefix];"\
  "ALTER TABLE [dbo].[AltRoute] ADD  CONSTRAINT [DF_AltRoute_baseRoute]  DEFAULT ((0)) FOR [baseRoute];"\
  "ALTER TABLE [dbo].[AltRoute] ADD  CONSTRAINT [DF_AltRoute_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";

 }
 else if(dbType == "MimerSql")
  {
      // 1. Create a dedicated sequence for the AltRoute table autoincrement
        QSqlQuery countQuery(srcDb);
        int nextId = 1;
        if (countQuery.exec("SELECT MAX(route) FROM AltRoute") && countQuery.next()) {
            nextId = countQuery.value(0).toInt() + 1;
            if (nextId < 1) nextId = 1; // Fallback if table was empty
        }

        // Stitch the correct starting value into the sequence declaration
        QString createSeq = QString("CREATE SEQUENCE altroute_route_seq START WITH %1 INCREMENT BY 1;").arg(nextId);
        QSqlQuery seqQuery(db);
        seqQuery.exec(createSeq);

      // 2. Define the main table statement utilizing the sequence default
      // Note: Removed backticks, AUTOINCREMENT, and changed reserved keyword 'index' to 'uq_route_prefix_alpha'
      commandText = "CREATE TABLE AltRoute (" \
          "route integer DEFAULT NEXT VALUE FOR altroute_route_seq PRIMARY KEY, "\
          "routePrefix varchar(10) DEFAULT '' NOT NULL, " \
          "routeAlpha varchar(10) NOT NULL, "\
          "baseRoute integer DEFAULT 0 NOT NULL, " \
          "lastUpdate timestamp DEFAULT LOCALTIMESTAMP NOT NULL, " \
          "CONSTRAINT uq_route_prefix_alpha unique (routePrefix, routeAlpha) )";
  }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating altRoute: %1").arg(query->lastError().text()));
 }
 // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
 if (dbType == "MimerSql") {
     qDebug() << "Permanently securing schema geometry layout to disk.";
     db.commit();
 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "AltRoute table created.";
 return true;
}

bool ExportSql::createParametersTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Parameters", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `Parameters` ("\
                "`key` integer NOT NULL primary key asc AUTOINCREMENT,"\
                "`lat` decimal(18,15) NOT NULL default (0),"\
                "`lon` decimal(18,15) NOT NULL default (0),"\
                "`title` varchar(50) NOT NULL,"\
                "`city` varchar(50) NOT NULL,"\
                "`minDate` date NOT NULL,"\
                "`maxDate` date NOT NULL,"\
                "`alphaRoutes` char(1) NOT NULL default ('Y'),"\
                "`abbreviationsList` VARCHAR(200) NOT NULL DEFAULT '',"
                "`lastUpdate` timestamp NOT NULL DEFAULT (CURRENT_TIMESTAMP));";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `Parameters` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`lat` decimal(18,15) NOT NULL,"\
    "`lon` decimal(18,15) NOT NULL,"\
    "`title` varchar(50) NOT NULL,"\
    "`city` varchar(50) NOT NULL,"\
    "`minDate` date NOT NULL,"\
    "`maxDate` date NOT NULL,"\
    "`alphaRoutes` char(1) NOT NULL,"\
    "`abbreviationsList` VARCHAR(200) NOT NULL DEFAULT '',"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "UNIQUE KEY `key` (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[Parameters]("\
        "[key] [int] IDENTITY(1,1) NOT NULL,"\
        "[lat] [decimal](18, 15) NOT NULL,"\
        "[lon] [decimal](18, 15) NOT NULL,"\
        "[title] [varchar](50) NOT NULL,"\
        "[city] [varchar](50) NOT NULL,"\
        "[minDate] [date] NOT NULL,"\
        "[maxDate] [date] NOT NULL,"\
        "[alphaRoutes] [char](1) NOT NULL,"\
        "[abbreviationsList] VARCHAR(200) NOT NULL DEFAULT '',"
        "[lastUpdate] [datetime] NOT NULL"\
    ") ON [PRIMARY];"\
   "SET ANSI_PADDING OFF;"\
   "ALTER TABLE [dbo].[Parameters]  WITH CHECK ADD  CONSTRAINT [CK_parameters] CHECK  (([alphaRoutes]='Y' OR [alphaRoutes]='N'));"\
   "ALTER TABLE [dbo].[Parameters] CHECK CONSTRAINT [CK_parameters];"\
   "ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_minDate]  DEFAULT ('1899-01-01') FOR [minDate];"\
   "ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_maxDate]  DEFAULT ('1966-5-21') FOR [maxDate];"\
   "ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_alphaRoutes]  DEFAULT ('N') FOR [alphaRoutes];"\
   "ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 }
 else if(dbType == "PostgreSQL")
 {
     commandText = "CREATE TABLE parameters (\
          key integer NOT NULL primary key  GENERATED BY DEFAULT AS IDENTITY,\
          lat decimal(18,15) NOT NULL default (0),\
          lon decimal(18,15) NOT NULL default (0),\
          title varchar(50) NOT NULL,\
          city varchar(50) NOT NULL,\
          minDate date NOT NULL,\
          maxDate date NOT NULL,\
          alphaRoutes char(1) NOT NULL default ('Y'),\
          abbreviationsList VARCHAR(200) NOT NULL DEFAULT '',\
          lastUpdate timestamp NOT NULL DEFAULT (CURRENT_TIMESTAMP));";
 }else if(dbType == "MimerSql")
 {
  /*
    What was changed for Mimer SQL:

    • Unique Constraint Name: Changed the unique constraint name from `index` to uq_route_prefix_alpha. INDEX is a strictly reserved word in Mimer SQL and will throw a syntax parser error if used as a constraint name.
    • Autoincrement Integration: Replaced the SQLite-specific NOT NULL primary key AUTOINCREMENT with the DEFAULT NEXT VALUE FOR altroute_route_seq PRIMARY KEY sequence configuration.
    • Identifiers & Quotes: Removed the backticks (`) entirely. Mimer SQL uses standard SQL double quotes (") for case-sensitive identifiers, but unquoted identifiers work best here to match standard case-insensitive tables.
    • Constraint Positioning: Placed the DEFAULT definitions cleanly before the NOT NULL constraints to keep Mimer's strict parser happy.*/

  // 1. Mimer SQL requires an independent sequence for autoincrement fields
  QString createSeq = "CREATE SEQUENCE parameters_key_seq START WITH 1 INCREMENT BY 1;";
  QSqlQuery seqQuery(db);

  // We ignore errors if the sequence already exists (or handle it in dropTable)
  if(!seqQuery.exec(createSeq))
  {
      QSqlError err = seqQuery.lastError();
        //SQLERROR_E(std::move(&seqQuery));
  }
  // 2. Define the main table statement utilizing the sequence default
  commandText = "CREATE TABLE parameters ("\
        "key integer DEFAULT NEXT VALUE FOR parameters_key_seq PRIMARY KEY,"\
        "lat decimal(18,15) DEFAULT 0 NOT NULL,"\
        "lon decimal(18,15) DEFAULT 0 NOT NULL,"\
        "title varchar(50) NOT NULL,"\
        "city varchar(50) NOT NULL,"\
        "minDate date NOT NULL,"\
        "maxDate date NOT NULL,"\
        "alphaRoutes char(1) DEFAULT 'Y' NOT NULL,"\
        "abbreviationsList VARCHAR(200) DEFAULT '' NOT NULL,"\
        "lastUpdate timestamp DEFAULT LOCALTIMESTAMP NOT NULL);";
 }
 //query->prepare(commandText);
 bQuery = query->exec(commandText);
 if(!bQuery)
 {
  QSqlError err = query->lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  throw Exception(tr("SQL Error creating Parameters: %1").arg(query->lastError().text()));

 }
  // === FIX 1: SECURE THE TABLE SCHEMA TO DISK IMMEDIATELY ===
  if (dbType == "MimerSql") {
      qDebug() << "Permanently securing schema geometry layout to disk.";
      db.commit();
  }
  // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "Parameters table created.";
 return true;
}
bool ExportSql::createTractionTypesTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("TractionTypes", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `TractionTypes` (\
          `tractionType` integer NOT NULL primary key AUTOINCREMENT,\
          `description` varchar(50) NOT NULL DEFAULT '',\
          `displayColor` char(7) NOT NULL DEFAULT '#000000',\
          `routeType` int(11) NOT NULL DEFAULT 0,\
          `icon` varchar(10) NOT NULL DEFAULT '',\
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP \
        );";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `TractionTypes` ("\
    "`tractionType` int(11) NOT NULL AUTO_INCREMENT,"\
    "`description` varchar(50) NOT NULL DEFAULT '',"\
    "`displayColor` char(7) NOT NULL DEFAULT '#000000',"\
    "`routeType` int(11) NOT NULL DEFAULT 0,"\
    "`icon` varchar(10) NOT NULL DEFAULT '',"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`tractionType`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=latin1";
 if(dbType == "PostgreSQL")
  commandText = "CREATE TABLE if not exists TractionTypes (\
          tractionType integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
          description varchar(50) NOT NULL DEFAULT '',\
          displayColor char(7) NOT NULL DEFAULT '#000000',\
          routeType INTEGER NOT NULL DEFAULT 0,\
          icon varchar(10) NOT NULL DEFAULT '',\
          lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP \
        );";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[TractionTypes]("\
        "[tractionType] [int] IDENTITY(1,1) NOT NULL,"\
        "[description] [varchar](50) NOT NULL,"\
        "[displayColor] [char](7) NOT NULL,"\
        "[routeType] [int] NOT NULL,"\
        "[icon] [varchar](10) NOT NULL,"\
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_TractionTypes] PRIMARY KEY CLUSTERED"\
    "("\
        "[tractionType] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];"\
  "SET ANSI_PADDING OFF;"\
  "ALTER TABLE [dbo].[TractionTypes] ADD  CONSTRAINT [DF_TractionTypes_description]  DEFAULT ('') FOR [description];"\
  "ALTER TABLE [dbo].[TractionTypes] ADD  CONSTRAINT [DF_TractionTypes_displayColor]  DEFAULT ('#000000') FOR [displayColor];"\
  "ALTER TABLE [dbo].[TractionTypes] ADD  CONSTRAINT [DF_TractionTypes_routeType]  DEFAULT ((0)) FOR [routeType];"\
  "ALTER TABLE [dbo].[TractionTypes] ADD  CONSTRAINT [DF_TractionTypes_icon]  DEFAULT ('') FOR [icon];"\
  "ALTER TABLE [dbo].[TractionTypes] ADD  CONSTRAINT [DF_TractionTypes_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate]";
 } else  if(dbType == "MimerSql")
 {
     // 1. Create a dedicated sequence for the AltRoute table autoincrement
     QString createSeq = "CREATE SEQUENCE tractiontypes_seq START WITH 1 INCREMENT BY 1;";
     QSqlQuery seqQuery(db);
     seqQuery.exec(createSeq);

     commandText = "CREATE TABLE TractionTypes (\
             tractionType integer DEFAULT NEXT VALUE FOR tractiontypes_seq PRIMARY KEY,\
             description varchar(50) NOT NULL DEFAULT '',\
             displayColor char(7) NOT NULL DEFAULT '#000000',\
             routeType INTEGER NOT NULL DEFAULT 0,\
             icon varchar(10) NOT NULL DEFAULT '',\
             lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP \
           );";
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating TractionTypes: %1").arg(query->lastError().text()));
  ;
 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "TractionTypes table created.";
 return true;
}
bool ExportSql::createCompaniesTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Companies", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `Companies` ( "\
          "`key` integer NOT NULL primary key AUTOINCREMENT,"\
          "`mnemonic` varchar(10) not null default '', "\
          "`Description` nvarchar(100) NOT NULL default '', "\
          "`routePrefix` varchar(10) not null default '', "\
          "`info` varchar(60) not null default '',"
          "`Url` varchar(150),"
          "`startDate` date DEFAULT NULL," \
          "`endDate` date DEFAULT NULL," \
          "`firstRoute` int(11) DEFAULT NULL," \
          "`lastRoute` int(11) DEFAULT NULL," \
          "`Selected` int(1),"\
          "`lastUpdate` timestamp DEFAULT CURRENT_TIMESTAMP" \
    ");";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `Companies` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`mnemonic` varchar(10) not null default '', "\
    "`Description` nvarchar(100) NOT NULL,"\
    "`routePrefix` varchar(10) DEFAULT '',"\
    "`info` varchar(60),"
    "`Url` varchar(150),"
    "`startDate` date DEFAULT NULL,"\
    "`endDate` date DEFAULT NULL,"\
    "`firstRoute` int(11) DEFAULT NULL,"\
    "`lastRoute` int(11) DEFAULT NULL,"\
    "`Selected` int(1),"\
    "`lastUpdate` timestamp DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "UNIQUE KEY `key` (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=latin1";
 else if(dbType == "PostgreSQL")
     commandText = "CREATE TABLE if not exists Companies ( "\
         "key integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,"\
         "mnemonic varchar(10) not null default '', "\
         "Description nvarchar(100) NOT NULL default '', "\
         "routePrefix varchar(10) not null default '', "\
         "info varchar(60) not null default '',"
         "Url varchar(150),"
         "startDate date DEFAULT NULL," \
         "endDate date DEFAULT NULL," \
         "firstRoute integer DEFAULT NULL," \
         "lastRoute integer DEFAULT NULL," \
         "Selected smallint,"\
         "lastUpdate timestamp DEFAULT CURRENT_TIMESTAMP" \
         ");";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "CREATE TABLE [dbo].[Companies]("\
        "[key] [int] IDENTITY(1,1) NOT NULL,"\
        "[mnemonic] varchar(10) not null default '', "\
        "[Description] [nvarchar](100) NOT NULL,"\
        "[info] varchar(60),"
        "[Url] varchar(150),"
        "[routePrefix] [varchar](10),"\
        "[startDate] [date] NULL,"\
        "[endDate] [date] NULL,"\
        "[firstRoute] [int] NULL,"\
        "[lastRoute] [int] NULL,"\
        "[lastUpdate] [datetime] NOT NULL, "\
        "[Selected] int,"\
    "CONSTRAINT [PK_Companies_1] PRIMARY KEY CLUSTERED"\
    "(" \
        "[key] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];"\
    "SET ANSI_PADDING OFF;"\
    "ALTER TABLE [dbo].[Companies] ADD  CONSTRAINT [DF_Companies_routePrefix]  DEFAULT ('') FOR [routePrefix];"\
    "ALTER TABLE [dbo].[Companies] ADD  CONSTRAINT [DF_Companies_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 } else if(dbType == "MimerSql")
 {
     // 1. Create a dedicated sequence for the Companies table autoincrement
     QSqlQuery countQuery(srcDb);
         int nextId = 1;
         if (countQuery.exec("SELECT MAX([key]) FROM Companies") && countQuery.next()) {
             nextId = countQuery.value(0).toInt() + 1;
             if (nextId < 1) nextId = 1;
         }

         QString createSeq = QString("CREATE SEQUENCE companies_seq START WITH %1 INCREMENT BY 1;").arg(nextId);
         QSqlQuery seqQuery(db);
         seqQuery.exec(createSeq);

     // 2. Define the main table statement utilizing the sequence default
     commandText = "CREATE TABLE Companies ( "\
         "key integer DEFAULT NEXT VALUE FOR companies_seq PRIMARY KEY," \
         "mnemonic varchar(10) not null default '', "\
         "Description nvarchar(100) NOT NULL default '', "\
         "routePrefix varchar(10) not null default '', "\
         "info varchar(60) not null default '',"
         "Url varchar(150),"
         "startDate date DEFAULT NULL," \
         "endDate date DEFAULT NULL," \
         "firstRoute integer DEFAULT NULL," \
         "lastRoute integer DEFAULT NULL," \
         "Selected smallint,"\
         "lastUpdate timestamp DEFAULT LOCALTIMESTAMP" \
         ");";
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating Companies: %1").arg(query->lastError().text()));

 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "Companies table created.";return true;
}

bool ExportSql::createIntersectionsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Intersections", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `Comments` ("\
          "`commentKey` integer NOT NULL primary key AUTOINCREMENT,"\
          "`tags` varchar(1000) NOT NULL,"\
          "`comments` mediumtext NOT NULL,"\
          "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP"\
        ");";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `Intersections` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`Street1` varchar(50) NOT NULL,"\
    "`Street2` varchar(50) NOT NULL,"\
    "`Latitude` decimal(15,13) NOT NULL,"\
    "`Longitude` decimal(15,13) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=507 DEFAULT CHARSET=latin1";
 else if(dbType == "PostgreSQL")
     commandText = "CREATE TABLE if not exists Intersections (\
                 key integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
                 Street1 varchar(50) NOT NULL,\
                 Street2 varchar(50) NOT NULL,\
                 Latitude decimal(15,13) NOT NULL,\
                 Longitude decimal(15,13) NOT NULL,\
                 lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP\
                 );";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;" \
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[Intersections]("\
        "[key] [int] IDENTITY(1,1) NOT NULL,"\
        "[Street1] [varchar](50) NOT NULL,"\
        "[Street2] [varchar](50) NOT NULL,"\
        "[Latitude] [decimal](15, 13) NOT NULL,"\
        "[Longitude] [decimal](15, 13) NOT NULL,"\
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_Intersections] PRIMARY KEY CLUSTERED"\
    "("\
        "[key] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];" \
    "SET ANSI_PADDING OFF;"\
    "ALTER TABLE [dbo].[Intersections] ADD  CONSTRAINT [DF_Intersections_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating Intersections: %1").arg(query->lastError().text()));

 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "Intersections table created.";return true;
}

bool ExportSql::createRouteSeqTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("RouteSeq", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `RouteSeq` ("\
                " `Route` integer NOT NULL,"\
                " `Name` varchar(125) NOT NULL,"
                " `StartDate` date NOT NULL, "
                " `EndDate` date NOT NULL, "
                " `segmentList` text,"
                " `firstSegment` integer NOT NULL,"
                " `whichEnd` text,"
                " `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP, "\
                " constraint pk PRIMARY key (`Route`,`Name`,`StartDate`,`EndDate`)"
        ");";
 else if(dbType == "PostgreSQL")
  commandText = "CREATE TABLE if not exists RouteSeq ("\
                " Route integer NOT NULL,"\
                " Name varchar(125) NOT NULL,"
                " StartDate date NOT NULL, "
                " EndDate date NOT NULL, "
                " segmentList text,"
                " firstSegment integer NOT NULL,"
                " whichEnd text,"
                " lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP, "\
                " constraint RouteSeq_pk PRIMARY key (Route,Name,StartDate,EndDate)"
        ");";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `RouteSeq` ("\
    "`route` int(11) NOT NULL ,"\
    "`name` varchar(125) NOT NULL,"\
    "`startDate` date NOT NULL,"\
    "`endDate` date NOT NULL,"\
    "`segmentList` varchar(500) NOT NULL,"\
    " `firstSegment` int(11) NOT NULL,"
    " `whichEnd` text,"
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`)"\
    ") ENGINE=InnoDB AUTO_INCREMENT=507 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;" \
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[RouteSeq]("\
        "[route] [int]  NOT NULL,"\
        "[name] [varchar](125) NOT NULL,"\
        "[startDate] [date] NOT NULL,"\
        "[endDate] [date] NOT NULL,"\
        "[segmentList] [varchar](500) NOT NULL,"\
        "[firstSegment] [int] NOT NULL,"
        "[whichEnd] [varchar](1),"
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_RouteSeq] PRIMARY KEY CLUSTERED"\
    "("\
        "[route] ASC,"\
        "[name] ASC," \
        "[startDate] ASC," \
        "[endDate] ASC" \
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];" \
         "SET ANSI_PADDING OFF;";
    //"ALTER TABLE [dbo].[Intersections] ADD  CONSTRAINT [DF_Intersections_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 } else if(dbType == "MimerSql") {

     commandText = "CREATE TABLE RouteSeq ("\
                   " Route integer NOT NULL,"\
                   " Name varchar(125) NOT NULL,"
                   " StartDate date NOT NULL, "
                   " EndDate date NOT NULL, "
                   " segmentList clob,"
                   " firstSegment integer NOT NULL,"
                   " whichEnd varchar(10),"
                   " lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP, "\
                   " constraint RouteSeq_pk PRIMARY key (Route,Name,StartDate,EndDate)"
           ");";
 }
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating RouteSeq: %1").arg(query->lastError().text()));

 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "RouteSeq table created.";
  return true;
}

bool ExportSql::createCommentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery* query = new QSqlQuery(db);
 QString commandText;
 bool bQuery;
 dropTable("Comments", db, dbType);

 if(dbType == "Sqlite")
  commandText = "CREATE TABLE if not exists `Comments` (\
              `commentKey` INTEGER primary key AUTOINCREMENT,\
              `tags` varchar(1000) NOT NULL,\
              `routeList` varchar(100) NOT NULL, \
              `date` date ,\
              `jRouteList` JSON,\
              `comments` mediumtext NOT NULL,\
              `latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',\
              `longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',\
              `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP\
            );";
 else if(dbType == "MySql")
  commandText = "CREATE TABLE `Comments` ("\
    "`commentKey` int(11) NOT NULL AUTO_INCREMENT,"\
    "`tags` varchar(1000) NOT NULL,"\
    "`routeList` varchar(100) NOT NULL," \
    "`date` date ,"\
    "`jRouteList` JSON NOT NULL ,"\
    "`comments` text CHARACTER SET utf8 NOT NULL,"\
    "`latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',"\
    "`longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`commentKey`)"\
    ") ENGINE=InnoDB AUTO_INCREMENT=79 DEFAULT CHARSET=latin1";
 else if(dbType == "PostgreSQL")
     commandText = "CREATE TABLE if not exists Comments (\
             commentKey integer NOT NULL primary key GENERATED BY DEFAULT AS IDENTITY,\
             tags varchar(1000) NOT NULL,\
             routeList varchar(100) NOT NULL,\
             date date ,\
             jRouteList JSON NOT NULL DEFAULT '[]',\
             comments text NOT NULL,\
             latitude decimal(15,5) NOT NULL DEFAULT '0.00000',\
             longitude decimal(15,5) NOT NULL DEFAULT '0.00000',\
             lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP\
           );";
 else if(dbType == "MsSql")
 {
  commandText = "SET ANSI_NULLS ON;"\
   "SET QUOTED_IDENTIFIER ON;"\
                "SET ANSI_PADDING ON;" \
   "DROP TABLE IF EXISTS [Comments];" \
   "CREATE TABLE [dbo].[Comments]("\
        "[commentKey] [int] IDENTITY(1,1) NOT NULL,"\
        "[tags] [varchar](1000) NOT NULL,"\
        "[routeList] [varchar](100) NOT NULL,"\
        "[date] date ,"\
        "[jRouteList] NVARCHAR(MAX) NOT NULL DEFAULT '[]',"\
        "[comments] [varchar](max) NOT NULL,"\
        "[latitude] decimal(15,5) NOT NULL DEFAULT '0.00000',"\
        "[longitude] decimal(15,5) NOT NULL DEFAULT '0.00000',"\
        "[lastupdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_comments] PRIMARY KEY CLUSTERED"\
    "("\
        "[commentKey] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];" \
    "SET ANSI_PADDING OFF;"\
    "ALTER TABLE [dbo].[comments] ADD  CONSTRAINT [DF_comments_new_comments]  DEFAULT ('') FOR [comments];"\
    "ALTER TABLE [dbo].[comments] ADD  CONSTRAINT [DF_comments_lastupdate]  DEFAULT (getdate()) FOR [lastupdate];";
 } else if(dbType == "MimerSql")
 {
     // 1. Create a dedicated sequence for the Comments table autoincrement
     QString createSeq = "CREATE SEQUENCE comments_seq START WITH 1 INCREMENT BY 1;";
     QSqlQuery seqQuery(db);
     seqQuery.exec(createSeq);

     // 2. Define the main table statement utilizing the sequence default
     commandText = "CREATE TABLE Comments (\
             commentKey integer DEFAULT NEXT VALUE FOR comments_seq PRIMARY KEY,\
             tags varchar(1000) NOT NULL,\
             routeList varchar(100) NOT NULL,\
             date date ,\
             jRouteList varchar(100) NOT NULL DEFAULT '[]',\
             comments clob NOT NULL,\
             latitude decimal(15,5) NOT NULL DEFAULT 0.00000,\
             longitude decimal(15,5) NOT NULL DEFAULT 0.00000,\
             lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP\
           );";}
 query->prepare(commandText);
 bQuery = query->exec();
 if(!bQuery)
 {
  SQLERROR_E(std::move(query));
  throw Exception(tr("SQL Error creating Comments: %1").arg(query->lastError().text()));

 }
 // === FIX 2: RECLAIM QUERY OBJECT MEMORY TO RELEASE OPEN SERVER CURSORS ===
  delete query;

  qDebug() << "Comments table created.";return true;
}
bool ExportSql::createStreetDefTable(QSqlDatabase db, QString dbType)
{
    if(dbType == "MsSql")
    {
        return SQL::instance()->executeScript(":/sql/MsSql/mssql_create_streetdef.sql",db);
    }
    else if(dbType == "MySql")
    {
        return SQL::instance()->executeScript(":/sql/MySql/mysql_create_streetdef.sql",db);
    }
    else if(dbType == "Sqlite")
    {
        return SQL::instance()->executeScript(":/sql/Sqlite3/sqlite3_create_streetdef.sql",db);
    }
    else
    if(dbType == "PostgreSQL")
    {
        QString commandString = "CREATE TABLE IF NOT EXISTS StreetDef ( \
          StreetId INTEGER NOT NULL GENERATED BY DEFAULT AS IDENTITY,\
          Street varchar(60) NOT NULL DEFAULT '' , \
          Location varchar(30),\
          StartDate date NOT NULL DEFAULT '2050-01-01',\
          EndDate date, \
          StartLatLng text, \
          EndLatLng text, \
          Length decimal(15,5) NOT NULL DEFAULT 0, \
          Bounds text, \
          Segments text, \
          Comment text,\
          Seq smallint NOT NULL DEFAULT 1, \
          lastUpdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
                                         PRIMARY KEY(StreetId, Seq),\
                                         UNIQUE (streetID, Street, StartDate)\
            );";

        return SQL::instance()->executeCommand(commandString,db);
    } else if(dbType == "MimerSql")
    {
        // 1. Create a dedicated sequence for the AltRoute table autoincrement
        QString createSeq = "CREATE SEQUENCE streetdef_seq START WITH 1 INCREMENT BY 1;";
        QSqlQuery seqQuery(db);
        seqQuery.exec(createSeq);

        // 2. Define the main table statement utilizing the sequence default
        // === FIXED: Added DATE prefix to literal default & changed text columns to NVARCHAR ===
        QString commandString = "CREATE TABLE StreetDef ( \
          StreetId INTEGER DEFAULT NEXT VALUE FOR streetdef_seq PRIMARY KEY,\
          Street nvarchar(60) NOT NULL DEFAULT '' , \
          Location nvarchar(30),\
          StartDate date NOT NULL DEFAULT DATE'2050-01-01',\
          EndDate date, \
          StartLatLng varchar(30), \
          EndLatLng varchar(30), \
          Length decimal(15,5) NOT NULL DEFAULT 0, \
          Bounds varchar(100), \
          Segments varchar(200), \
          Comment nvarchar(1000),\
          Seq smallint NOT NULL DEFAULT 1, \
          lastUpdate timestamp NOT NULL DEFAULT LOCALTIMESTAMP,\
                                         UNIQUE (StreetId, Street, StartDate)\
            );";

        return SQL::instance()->executeCommand(commandString, db);
    }

    return false;

}
bool ExportSql::createMySqlFunctions(QSqlDatabase db)
{
 return false;
}

bool ExportSql::createMsSqlFunctions(QSqlDatabase db)
{
    QSqlQuery* query = new QSqlQuery(db);
    bool bQuery;

    QString commandText =
    "USE [master] "
    "GO "

    "/****** Object:  UserDefinedFunction [dbo].[distance]    Script Date: 10/12/2023 8:04:06 PM ******/ "
    "SET ANSI_NULLS ON "
    "GO "

    "SET QUOTED_IDENTIFIER ON "
    "GO "



    "CREATE FUNCTION [dbo].[distance](@lat1 FLOAT, @lat2 FLOAT, @long1 FLOAT, @long2 FLOAT ) "
    "RETURNS FLOAT(18) "
    "AS "
    "Begin "
    "      Declare @R Float(8); "
    "      Declare @dLat Float(18); "
    "      Declare @dLon Float(18); "
    "      Declare @a Float(18); "
    "      Declare @c Float(18); "
    "      Declare @d Float(18); "
    "      Set @R =  6367.45 "
    "            --Miles 3956.55  "
    "            --Kilometers 6367.45 "
    "            --Feet 20890584 "
    "            --Meters 6367450 "


    "      Set @dLat = Radians(@lat2 - @lat1); "
    "      Set @dLon = Radians(@long2 - @long1); "
    "      Set @a = Sin(@dLat / 2)  "
    "                 * Sin(@dLat / 2) "
    "                 + Cos(Radians(@lat1)) "
    "                 * Cos(Radians(@lat2)) "
    "                 * Sin(@dLon / 2) "
    "                 * Sin(@dLon / 2); "
    "      Set @c = 2 * Asin(Min(Sqrt(@a))); "

    "      Set @d = @R * @c; "
    "      Return @d; "

    "End "
    "GO ";
    query->prepare(commandText);
    bQuery = query->exec();
    if(!bQuery)
    {
     SQLERROR_E(std::move(query));
     throw Exception(tr("SQL Error creating Comments: %1").arg(query->lastError().text()));

    }
    return true;
}


int ExportSql::errSqlMessage(QSqlQuery* query, int line)
{
 if(ignoreList.contains(query->lastError().nativeErrorCode()))
 {
  ignored++;
  logError(query, true, line);
  return QMessageBox::Ignore;
 }
 int ret = QMessageBox::critical(nullptr, tr("Sql Error"), tr("An SqL error has occurred.<br>"
                                 "Sql error:%1<br><B>query:</B> %2 %3").arg(query->lastError().text(),query->lastQuery(),displayQueryValues(query)),
                                 QMessageBox::Ignore|QMessageBox::Abort);
 if(ret == QMessageBox::Abort)
    return ret;
 int ret2 = QMessageBox::question(nullptr, tr("Ignore"), tr("Should this error be ignored for subsequent transactions?"),
                                  QMessageBox::Yes|QMessageBox::No);
 if(ret2 == QMessageBox::Yes)
 {
  ignoreList.append(query->lastError().nativeErrorCode());
  ignored++;
 }
 logError(query, true, line);

 return ret;
}

bool ExportSql::dropView(QString view)
{
  if(!openDb())
    return false;
  QString commandText = QString("drop view if exists %1").arg(view);
  if(tgtConn->servertype()=="MimerSql")
  {
      if(viewExists(view, _targetDb))
        commandText = QString("drop view %1").arg(view.toUpper());
      else
          return true;
  }
  QSqlQuery* query = new QSqlQuery(_targetDb);

  bool bQuery = query->exec(commandText);
  if(!bQuery)
  {
      QSqlError err = query->lastError();
      SQLERROR_E(std::move(query));
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      //db.close();
      return false;
  }
  return true;
}

bool ExportSql::exportTable(QString inTable)
{
    if (_targetDb.driverName() == "QODBC" && tgtConn->servertype()=="MimerSql") {
            return exportMimerTable(inTable);
    }
    QString table = inTable;
    if(tgtConn->servertype() == "PostgreSQL")
        table = inTable.toLower();
     bool bDropTables = this->bDropTables;
     srcDb = QSqlDatabase::database(srcConn->connectionName());
     qInfo() << "database is" << (srcDb.isOpen()?"open":"not open") << srcDb.connectionName() << " "
             << srcDb.driverName() << " " << srcDb.databaseName();

     QString srcServerType = srcConn->servertype();
     if(srcConn->uniqueId() == tgtConn->uniqueId())
         throw IllegalArgumentException("source and destination the same");
     // prevent leaking connections
     // if(!openDb())
     //  return false;
       ignoreList.clear();

     if(tgtConn->servertype() == "MySql" || tgtConn->servertype() == "MsSql")
        SQL::instance()->useDatabase(tgtConn->database(), tgtConn->servertype(), _targetDb);

 QStringList tables = targetDb().tables();
 if(tables.contains(table,Qt::CaseInsensitive))
  updateTimestamp(table);

 QStringList types;
 QString resolvedSourceTable = table;

 // FIXED: Query the source database for its actual table name casing layout
 QStringList srcTables = srcDb.tables();
 int matchIndex = srcTables.indexOf(QRegularExpression(QString("^%1$").arg(table), QRegularExpression::CaseInsensitiveOption));

 if (matchIndex != -1) {
     resolvedSourceTable = srcTables.at(matchIndex);
     qDebug() << "Resolved source database case-sensitive table name match:" << resolvedSourceTable;
 }

 // Pass the correctly cased table name string straight to the schema extractor
 QStringList columns = SQL::instance()->listColumns(resolvedSourceTable, srcServerType, srcDb, &types);
 // FIXED: If case-sensitivity caused the schema dictionary to return empty,
  // fall back to evaluating a lowercase variation to extract the true field array width
  if (columns.isEmpty()) {
      qWarning() << "Table schema lookup came back blind for:" << table << "- Retrying with lowercase format.";
      columns = SQL::instance()->listColumns(table.toLower(), srcServerType, srcDb, &types);
  }
  if(tables.contains(table,Qt::CaseInsensitive))
 {
     if(!areTableDefsEqual(table, config->currConnection, tgtConn, srcDb, _targetDb))
      bDropTables = true;
 }
 else
     bDropTables = true; // if target table does not exist, they are obviously not equal

 added=0;
 updated=0;
 deleted=0;
 errors=0;
 notUpdated=0;
 ignored = 0;

 getCount(table, bDropTables || !tables.contains(table,Qt::CaseInsensitive));
 if(tables.contains(table,Qt::CaseInsensitive) && bDropTables)
 {
  if(!dropTable(table, targetDb(),tgtConn->servertype()))
     return false;
 }
 tables = targetDb().tables();
 if(bDropTables || !tables.contains(table,Qt::CaseInsensitive))
 {
  if(table.compare("altRoute", Qt::CaseInsensitive)==0)
   if(!createAltRouteTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Comments", Qt::CaseInsensitive)==0)
   if(!createCommentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Companies", Qt::CaseInsensitive)==0)
   if(!createCompaniesTable(_targetDb, tgtConn->servertype())) return false;
  // if(table.compare("Intersections", Qt::CaseInsensitive)==0)
  //  if(!createIntersectionsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Parameters", Qt::CaseInsensitive)==0)
   if(!createParametersTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteComments", Qt::CaseInsensitive)==0)
   if(!createRouteCommentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Routes", Qt::CaseInsensitive)==0)
   if(!createRouteTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteName", Qt::CaseInsensitive)==0)
    if(!createRouteNameTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteSeq", Qt::CaseInsensitive)==0)
   if(!createRouteSeqTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("StreetDef",Qt::CaseInsensitive)==0)
   if(!createStreetDefTable(_targetDb,tgtConn->servertype()))
       return false;
  if(table.compare("Segments", Qt::CaseInsensitive)==0)
   if(!createSegmentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Stations", Qt::CaseInsensitive)==0)
   if(!createStationsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Segments", Qt::CaseInsensitive)==0)
   if(!createSegmentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Terminals", Qt::CaseInsensitive)==0)
   if(!createTerminalsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("TractionTypes", Qt::CaseInsensitive)==0)
   if(!createTractionTypesTable(_targetDb, tgtConn->servertype())) return false;
  // === FIX 1: COMMIT THE STRUCTURAL SCHEMAS BEFORE INSERTING ROWS ===
  if (tgtConn->servertype() == "MimerSql") {
       qDebug() << "Committing table layout structural modifications for:" << table;
       _targetDb.commit();
   }
 }


 if(tgtDbType == "MsSql" && table != "Terminals" && table != "RouteComments"
    && table != "Routes" && table != "RouteSeq")
  setIdentityInsert(table, true);
 QString commandText;
 QString selectText = "Select ";
 for(int i=0; i< columns.count(); i++)
 {
  QString col = columns.at(i);
  if(tgtConn->servertype()== "MsSql")
   selectText.append("["+col+"]");
  else
   selectText.append("`"+col+"`");
  if(i < columns.count()-1)
    selectText.append(",");
 }
 selectText.append(" from ");
 selectText.append(table);

 if(bDropTables)
  commandText = selectText + " order by lastUpdate";
 else
  commandText = selectText + " where lastUpdate > :lastUpdated  order by lastUpdate";


 QSqlQuery* query = new QSqlQuery(srcDb);
 query->setForwardOnly(true);
 QSqlQuery* query2 = new QSqlQuery(_targetDb);

 // === STEP 1: INITIALIZE THE EXPLICIT WRITER BOUNDARY ===
  if (tgtConn->servertype() == "MimerSql") {
      qDebug() << "Initializing explicit transactional workspace boundary for table:" << table;
      _targetDb.transaction();
  }

  bool bPrepare =query->prepare(commandText);
 if(!bDropTables)
  query->bindValue(":lastUpdated", lastUpdated);
 bool bQuery = query->exec();
 if(!bQuery)
 {
     QSqlError err = query->lastError();
     SQLERROR_E(std::move(query));
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     //db.close();
     return false;
 }

 QStringList keys = SQL::instance()->listPkColumns(table, tgtDbType, _targetDb);
 while(query->next())
 {
  bool bFound = false; // This is your existing local variable declaration block
  if(!bDropTables)
  {
   // Strip backticks from the selection template string shell for Mimer SQL
   QString cleanSelect = selectText;
   if(tgtConn->servertype() == "MimerSql") {
       cleanSelect.replace("`", "");
   }

   commandText = cleanSelect + " where ";
   for(int i=0; i< keys.count(); i++)
   {
    QString col = keys.at(i);
    // === MIMER COMPATIBLE LOOKUP PLACEHOLDERS ===
    if(tgtConn->servertype() == "MimerSql")
        commandText.append(QString("%1 = ?").arg(col));
    else
        commandText.append(QString("%1=:%1").arg(col));

     if(i < keys.count()-1)
       commandText.append(" and ");
   }

   query2->prepare(commandText);
   for(int i=0; i< keys.count(); i++)
   {
    QString col = keys.at(i);
    // Find the correct primary key index location in the main columns list array
    QVariant pkVal = query->value(columns.indexOf(col));

    if(tgtConn->servertype() == "MimerSql")
        query2->bindValue(i, pkVal.toString()); // Force clean out-of-band wide string matching
    else
        query2->bindValue( ":"+ col, query->value(i) );
   }

   bQuery = query2->exec();
   if(!bQuery)
   {
       QSqlError err = query2->lastError();
       SQLERROR_E(std::move(query2));
       qDebug() << err.text() + "\n";
       qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
       return false;
   }
   bFound = query2->next();
  }

  if(bFound)
  {
   bool bEqual = true;

   // FIXED: Extract matching column fields straight from the in-memory array width,
    // bypassing the toxic query->record() background catalog loops completely.
   for(int i = 0; i < columns.count(); i++)
   {
        if(query->value(i) != query2->value(i))
        {
            bEqual = false;
        break;
        }
   }
   if(bEqual)
   {
    notUpdated++;
    sendProgress();
    continue;
   }

   //QStringList pkList = SQL::instance()->listPkColumns(table, tgtDbType, _targetDb);
   QStringList pkList = keys;
   QString commandString = "update " + table + " set ";

   // === FIX 1: STRIP BACKTICKS AND BUILD POSITIONAL TEMPLATES FOR MIMER SQL ===
   if (tgtConn->servertype() == "MimerSql")
   {
       bool first = true;
       for(int i=0; i< columns.count(); i++) {
           QString col = columns.at(i);
           if(!pkList.contains(col)) {
               if(!first) commandString.append(", ");
               commandString.append(QString("%1 = ?").arg(col));
               first = false;
           }
       }
       commandString.append(" where ");
       first = true;
       for(int i=0; i< pkList.count(); i++) {
           QString col = pkList.at(i);
           if(!first) commandString.append(" and ");
           commandString.append(QString("%1 = ?").arg(col));
           first = false;
       }
   }
   else
   {
       // Keep your original named parameter loop intact for MySQL / MS SQL:
       for(int i=0; i< columns.count(); i++) {
        QString col = columns.at(i);
        if(!pkList.contains(col)) {
         commandString.append(QString("`%1`=:%1").arg(col));
          if(i < columns.count()-1) commandString.append(",");
        }
       }
       commandString.append(" where ");
       for(int i=0; i< pkList.count(); i++) {
        QString col = pkList.at(i);
        commandString.append(QString("`%1`=:%1").arg(col));
         if(i < pkList.count()-1) commandString.append(" and ");
       }
   }

   query2->prepare(commandString);

   // // === FIX 2: ALIGN OUT-OF-BAND BINDINGS BY POSITION FOR MIMER SQL ===
   // if (tgtConn->servertype() == "MimerSql")
   // {
   //     int updateSlot = 0;

   //     // Pass 1: Bind Non-Primary Key values sequentially
   //     for(int i=0; i< columns.count(); i++) {
   //         if(!pkList.contains(columns.at(i))) {
   //             QVariant val = query->value(i);

   //             // Reuse our standardized data normalization pipeline
   //             if (val.userType() == QMetaType::QDateTime) {
   //                 query2->bindValue(updateSlot, val.toDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
   //             } else if (val.userType() == QMetaType::QDate) {
   //                 query2->bindValue(updateSlot, val.toDate().toString("yyyy-MM-dd"));
   //             } else {
   //                 query2->bindValue(updateSlot, val.isNull() ? QVariant(QMetaType::fromType<QString>()) : val.toString());
   //             }
   //             updateSlot++;
   //         }
   //     }

   //     // Pass 2: Bind Primary Key values sequentially for the WHERE clause
   //     for(int i=0; i< columns.count(); i++) {
   //         if(pkList.contains(columns.at(i))) {
   //             QVariant val = query->value(i);
   //             query2->bindValue(updateSlot, val.isNull() ? QVariant(QMetaType::fromType<QString>()) : val.toString());
   //             updateSlot++;
   //         }
   //     }
   // }
   // else
   // {
       // Keep original named binding loop for other backends
       for(int i=0; i< columns.count(); i++) {
        QString col = columns.at(i);
        query2->bindValue( ":"+ col, query->value(i) );
        // === FORCE WARNING TO BECOME FATAL ERROR ===
           if (_targetDb.lastError().isValid())
           {
               qCritical() << "FATAL: ODBC Sequence Collision Caught at binding slot" << i
                           << "-" << _targetDb.lastError().text();

               // Terminate immediately so your log isn't wiped out
               if (tgtConn->servertype() == "MimerSql") {
                   _targetDb.rollback();
               }
               return false;
           }
       }
   // }

   bQuery = query2->exec();
   if(!bQuery)
   {
    QSqlError err = query2->lastError();
    SQLERROR_E(std::move(query2));
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    errors++;
   }
   else
    updated++;
  }
  else
  {
i0:   QString commandString = "insert into " + table + " (";
   for(int i=0; i< columns.count(); i++)
   {
    QString col = columns.at(i);
    if(tgtConn->servertype()== "MsSql")
        commandString.append("[" + col + "]");
    else if(tgtConn->servertype()== "PostgreSQL")
        commandString.append( col );
    else if(tgtConn->servertype()== "MimerSql")
        commandString.append( col );
    else
        commandString.append("`" + col + "`");
    if(i < columns.count()-1)
      commandString.append(",");
   }
   commandString.append(") values (");

   for(int i=0; i< columns.count(); i++)
   {
    QString col = columns.at(i);
    QString outStr;
m0:    if (tgtConn->servertype() == "MimerSql")
    {
        outStr = "?";
    }
    else if(tgtConn->servertype() != "PostgreSQL")
    {
        // Check if the current column is the problematic JSON column
         if(col.compare("jRouteList", Qt::CaseInsensitive) == 0 && tgtConn->servertype() == "MySql")
         {
             commandString.append("CONVERT(:" + col + " USING utf8mb4)");
         }
         else
         {
             commandString.append(":" + col);
         }
    }
    else {
            // Your exact original type evaluation block continues underneath:
            if(types.at(i)== "")
            {
                // for some reason, the Sqlite startDate column in Segments had a blank type,
                // so see if it can be converted to a string and is a valid date.
                QString str =  query->value(i).toString();
                if(QDate::fromString(str,"yyyy/MM/dd").isValid())
                    types.replace(i,"date");
            }

            if(types.at(i)== "date")
            {
                QDate date = query->value(i).toDate();
                QString dateStr = date.toString("yyyy-MM-dd");
                if(dateStr.isEmpty())
                    qDebug() << "no date";
                if(date.isValid())
                    outStr = "DATE'" + dateStr + "'";
                else
                    outStr = "NULL";
            }
            else
            if(types.at(i)=="timestamp")
            {
                QDateTime dt = query->value(i).toDateTime();
                QString dtStr = dt.toString(Qt::ISODateWithMs);
                if(tgtConn->servertype() != "MimerSql")
                    outStr = "'"+dtStr + "'::timestamp";
                else
                {
                    dtStr.replace('T', ' ');
                    dtStr.replace('Z',"");
                    outStr = "TIMESTAMP'"+dtStr+"'";
                }
            }
            else
            if(types.at(i).startsWith("decimal",Qt::CaseInsensitive)!=0)
            {
                double dbl = query->value(i).toDouble();
                outStr = QString("%1").arg(dbl,0,'f',6);
            }
            else
            if(types.at(i).compare("integer",Qt::CaseInsensitive)==0 || types.at(i).startsWith("int",Qt::CaseInsensitive))
            {
                if(query->value(i).isNull())
                    outStr = "0";
                else
                    outStr = query->value(i).toString();
            }
            else
            if(types.at(i).startsWith("char",Qt::CaseInsensitive)!=0)
            {
                QString c = query->value(i).toString();
                bool b;

                int cn = c.toInt(&b);
                if(c.length() == 1)
                    outStr = "'"+c+"'";
                else
                {
                    if(b && cn == 89)
                        outStr = "'Y'";
                    else
                        outStr = "'N'";
                }
            }
            else
            if(types.at(i).compare("text",Qt::CaseInsensitive)==0
                        || types.at(i).compare("mediumtext",Qt::CaseInsensitive)==0
                        || types.at(i).startsWith("varchar",Qt::CaseInsensitive)
                        || types.at(i).startsWith("nvarchar",Qt::CaseInsensitive))
            {
                QString str = query->value(i).toString();
                str = str.replace("'","''");
                outStr = "'" + str +"'";
            }
            else if(types.at(i).compare("JSON",Qt::CaseInsensitive)==0)
            {
                QString str = query->value(i).toString();
                outStr = "'" + str +"'";
            }
            else {
                qDebug() << "not handled: " << types.at(i);
            }
        } // End of non-Mimer else block

        // FIXED: Force outStr to contain a placeholder for Mimer SQL right before it appends
        if (tgtConn->servertype() == "MimerSql") {
            outStr = "?";
        }

        if(outStr.isEmpty())
            qDebug() << "error: no value!";

        commandString.append(outStr);


     if(i < columns.count()-1)
      commandString.append(",");
   }
   commandString.append(")");

p0:   if(tgtConn->servertype() != "PostgreSQL")
   {
       query2->prepare(commandString);

       for(int i=0; i< columns.count(); i++)
       {
        QString col = columns.at(i);
        QVariant val;

        if(col.compare("lastUpdate",Qt::CaseInsensitive)==0)
        {
         QDateTime dt = query->value(i).toDateTime();
         if(!dt.isValid())
          qDebug() << "date is invalid '" <<query->value(i).toString() << "'";
         if(dt.date().month()==1 && dt.date().day()>28)
         {
          dt = QDateTime(QDate(dt.date().year(), 1, 28), dt.time());
         }
         val = dt;
        }
        else
        {
            if(types.at(i)== "date" )
            {
               QDate dt = query->value(i).toDate();
               if(!dt.isValid())
               {
                   QString rawDateStr = query->value(i).toString();
                   dt = QDate::fromString(rawDateStr, "yyyy/MM/dd");
               }

               if(!dt.isValid())
               {
                   val = QVariant(QMetaType::fromType<QString>());
               }
               else if (tgtConn->servertype() == "MimerSql")
               {
                   val = dt.toString("yyyy-MM-dd");
               }
               else
               {
                   val = dt;
               }
            }
            else
            if (tgtConn->servertype() == "MimerSql")
            {
                QVariant rawVal = query->value(i);

                if (rawVal.isNull()) {
                    val = QVariant(QMetaType::fromType<QString>());
                }
                else if (rawVal.userType() == QMetaType::QDateTime) {
                    val = rawVal.toDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                }
                else if (rawVal.userType() == QMetaType::QDate) {
                    val = rawVal.toDate().toString("yyyy-MM-dd");
                }
                else if (rawVal.userType() == QMetaType::Double) {
                    val = QString("%1").arg(rawVal.toDouble(), 0, 'f', 6);
                }
                else {
                    QString cleanStr = rawVal.toString();
                    val = QVariant(QMetaType::fromType<QString>(), &cleanStr);
                }
            }
            else
                val = query->value(i);
        }

        if (tgtConn->servertype() == "MimerSql")
        {
            query2->bindValue(i, val);
        }
        else
        {
            query2->bindValue(":" + col, val);
        }
       }
       // FIXED: Capture return status to satisfy downstream error checkers cleanly
       bQuery = query2->exec();
   }
   else
       bQuery=query2->exec(commandString); // PostgreSQL path (Line ~2746)


   if(!bQuery)
   {
    QSqlError err = query2->lastError();
    SQLERROR_E(std::move(query2));
    qDebug() << "error code:" << err.nativeErrorCode() << " " << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    qDebug() << "values: " << displayQueryValues(query);
    errors++;
   }
   else
    added++;
  }

  sendProgress();
 }

 qDebug()<<"Comments: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
 emit uncheck("chk"+table);

 // === FIX: Explicitly delete the query handles to free active ODBC statements ===
  if(query)  { delete query;  query = nullptr;  }
  if(query2) { delete query2; query2 = nullptr; }

  if (tgtConn->servertype() == "MimerSql") {
     qDebug() << "Enforcing explicit transaction commit for table: " << table;
     _targetDb.commit();
 }
 return true;
}

bool ExportSql::exportMimerTable(const QString &inTable)
{

    QString table = inTable;
    // if(tgtConn->servertype() == "PostgreSQL")
    //     table = inTable.toLower();
     bool bDropTables = this->bDropTables;
     srcDb = QSqlDatabase::database(srcConn->connectionName());
     qInfo() << "database is" << (srcDb.isOpen()?"open":"not open") << srcDb.connectionName() << " "
             << srcDb.driverName() << " " << srcDb.databaseName();

     QString srcServerType = srcConn->servertype();
     if(srcConn->uniqueId() == tgtConn->uniqueId())
         throw IllegalArgumentException("source and destination the same");
     // prevent leaking connections
     // if(!openDb())
     //  return false;
       ignoreList.clear();

     // if(tgtConn->servertype() == "MySql" || tgtConn->servertype() == "MsSql")
     //    SQL::instance()->useDatabase(tgtConn->database(), tgtConn->servertype(), _targetDb);

 QStringList tables = targetDb().tables();
 if(tables.contains(table,Qt::CaseInsensitive))
  updateTimestamp(table);

 QStringList types;
 QString resolvedSourceTable = table;

 // FIXED: Query the source database for its actual table name casing layout
 QStringList srcTables = srcDb.tables();
 int matchIndex = srcTables.indexOf(QRegularExpression(QString("^%1$").arg(table), QRegularExpression::CaseInsensitiveOption));

 if (matchIndex != -1) {
     resolvedSourceTable = srcTables.at(matchIndex);
     qDebug() << "Resolved source database case-sensitive table name match:" << resolvedSourceTable;
 }

 // Pass the correctly cased table name string straight to the schema extractor
 QStringList columns = SQL::instance()->listColumns(resolvedSourceTable, srcServerType, srcDb, &types);
 // FIXED: If case-sensitivity caused the schema dictionary to return empty,
  // fall back to evaluating a lowercase variation to extract the true field array width
  if (columns.isEmpty()) {
      qWarning() << "Table schema lookup came back blind for:" << table << "- Retrying with lowercase format.";
      columns = SQL::instance()->listColumns(table.toLower(), srcServerType, srcDb, &types);
  }
  if(tables.contains(table,Qt::CaseInsensitive))
 {
     if(!areTableDefsEqual(table, config->currConnection, tgtConn, srcDb, _targetDb))
      bDropTables = true;
 }
 else
     bDropTables = true; // if target table does not exist, they are obviously not equal

 added=0;
 updated=0;
 deleted=0;
 errors=0;
 notUpdated=0;
 ignored = 0;

 getCount(table, bDropTables || !tables.contains(table,Qt::CaseInsensitive));
 if(tables.contains(table,Qt::CaseInsensitive) && bDropTables)
 {
  if(!dropTable(table, targetDb(),tgtConn->servertype()))
     return false;
 }
 tables = targetDb().tables();
 if(bDropTables || !tables.contains(table,Qt::CaseInsensitive))
 {
  if(table.compare("altRoute", Qt::CaseInsensitive)==0)
   if(!createAltRouteTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Comments", Qt::CaseInsensitive)==0)
   if(!createCommentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Companies", Qt::CaseInsensitive)==0)
   if(!createCompaniesTable(_targetDb, tgtConn->servertype())) return false;
  // if(table.compare("Intersections", Qt::CaseInsensitive)==0)
  //  if(!createIntersectionsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Parameters", Qt::CaseInsensitive)==0)
   if(!createParametersTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteComments", Qt::CaseInsensitive)==0)
   if(!createRouteCommentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Routes", Qt::CaseInsensitive)==0)
   if(!createRouteTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteName", Qt::CaseInsensitive)==0)
    if(!createRouteNameTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("RouteSeq", Qt::CaseInsensitive)==0)
   if(!createRouteSeqTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("StreetDef",Qt::CaseInsensitive)==0)
   if(!createStreetDefTable(_targetDb,tgtConn->servertype()))
       return false;
  if(table.compare("Segments", Qt::CaseInsensitive)==0)
   if(!createSegmentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Stations", Qt::CaseInsensitive)==0)
   if(!createStationsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Segments", Qt::CaseInsensitive)==0)
   if(!createSegmentsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("Terminals", Qt::CaseInsensitive)==0)
   if(!createTerminalsTable(_targetDb, tgtConn->servertype())) return false;
  if(table.compare("TractionTypes", Qt::CaseInsensitive)==0)
   if(!createTractionTypesTable(_targetDb, tgtConn->servertype())) return false;
  // === FIX 1: COMMIT THE STRUCTURAL SCHEMAS BEFORE INSERTING ROWS ===
  if (tgtConn->servertype() == "MimerSql") {
       qDebug() << "Committing table layout structural modifications for:" << table;
       _targetDb.commit();
   }
 }


 if(tgtDbType == "MsSql" && table != "Terminals" && table != "RouteComments"
    && table != "Routes" && table != "RouteSeq")
  setIdentityInsert(table, true);
 QString commandText;
 QString selectText = "Select ";
 for(int i=0; i< columns.count(); i++)
 {
  QString col = columns.at(i);
  // if(tgtConn->servertype()== "MsSql")
  //  selectText.append("["+col+"]");
  // else
   selectText.append("`"+col+"`");
  if(i < columns.count()-1)
    selectText.append(",");
 }
 selectText.append(" from ");
 selectText.append(table);

 if(bDropTables)
  commandText = selectText + " order by lastUpdate";
 else
  commandText = selectText + " where lastUpdate > :lastUpdated  order by lastUpdate";

 QStringList keys = SQL::instance()->listPkColumns(table, tgtDbType, _targetDb);

 QSqlQuery* query = new QSqlQuery(srcDb);
 query->setForwardOnly(true);
 QSqlQuery* query2 = new QSqlQuery(_targetDb);

 _targetDb.transaction();

  bool bPrepare =query->prepare(commandText);
 if(!bDropTables)
  query->bindValue(":lastUpdated", lastUpdated);
 bool bQuery = query->exec();
 if(!bQuery)
 {
     QSqlError err = query->lastError();
     SQLERROR_E(std::move(query));
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     return false;
 }

 while(query->next())
 {
  bool bFound = false; // This is your existing local variable declaration block
  if(!bDropTables)
  {
   // Strip backticks from the selection template string shell for Mimer SQL
   QString cleanSelect = selectText;
   //if(tgtConn->servertype() == "MimerSql") {
       cleanSelect.replace("`", "");
   //}

   commandText = cleanSelect + " where ";
   for(int i=0; i< keys.count(); i++)
   {
    QString col = keys.at(i);
    commandText.append(QString("%1 = ?").arg(col));

     if(i < keys.count()-1)
       commandText.append(" and ");
   }

   query2->prepare(commandText);
   for(int i=0; i< keys.count(); i++)
   {
    QString col = keys.at(i);
    // Find the correct primary key index location in the main columns list array
    QVariant pkVal = query->value(columns.indexOf(col));

    query2->bindValue(i, pkVal.toString()); // Force clean out-of-band wide string matching
   }

   bQuery = query2->exec();
   if(!bQuery)
   {
       QSqlError err = query2->lastError();
       SQLERROR_E(std::move(query2));
       qDebug() << err.text() + "\n";
       qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
       return false;
   }
   bFound = query2->next();
   // === CRITICAL REGISTRATION UNLOCK ===
   // Clear the active composite key selection cursor out of the driver manager stack
   // before reusing the object handle for an update or insert prepare statement.
   query2->finish();
  } // end if(!bDropTables) block

  if(bFound)
  {
   bool bEqual = true;

   // FIXED: Extract matching column fields straight from the in-memory array width,
    // bypassing the toxic query->record() background catalog loops completely.
   for(int i = 0; i < columns.count(); i++)
   {
        if(query->value(i) != query2->value(i))
        {
            bEqual = false;
        break;
        }
   }
   if(bEqual)
   {
    notUpdated++;
    sendProgress();
    continue;
   }

   QStringList pkList = keys;
   QString commandString = "update " + table + " set ";

   // === FIX 1: STRIP BACKTICKS AND BUILD POSITIONAL TEMPLATES FOR MIMER SQL ===
   bool first = true;
   for(int i=0; i< columns.count(); i++) {
       QString col = columns.at(i);
       if(!pkList.contains(col)) {
           if(!first) commandString.append(", ");
           commandString.append(QString("%1 = ?").arg(col));
           first = false;
       }
   }
   commandString.append(" where ");
   first = true;
   for(int i=0; i< pkList.count(); i++) {
       QString col = pkList.at(i);
       if(!first) commandString.append(" and ");
       commandString.append(QString("%1 = ?").arg(col));
       first = false;
   }

   query2->prepare(commandString);

   // Keep original named binding loop for other backends
   for(int i=0; i< columns.count(); i++) {
        QString col = columns.at(i);
        query2->bindValue( ":"+ col, query->value(i) );
        // === FORCE WARNING TO BECOME FATAL ERROR ===
       if (_targetDb.lastError().isValid())
       {
           qCritical() << "FATAL: ODBC Sequence Collision Caught at binding slot" << i
                       << "-" << _targetDb.lastError().text();

           // Terminate immediately so your log isn't wiped out
           if (tgtConn->servertype() == "MimerSql") {
               _targetDb.rollback();
           }
           return false;
       }
   }

   bQuery = query2->exec();
   if(!bQuery)
   {
    QSqlError err = query2->lastError();
    SQLERROR_E(std::move(query2));
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    errors++;
   }
   else
    updated++;
  }
  else
  {
i0:   QString commandString = "insert into " + table + " (";
   for(int i=0; i< columns.count(); i++)
   {
        QString col = columns.at(i);
        // === REPAIRED: WRAP IDENTIFIERS IN ANSI DOUBLE QUOTES ===
        // This allows keywords like "Next" and "Prev" to be parsed safely as columns
        commandString.append("\"" + col + "\"");
        if(i < columns.count()-1)
            commandString.append(",");
   }
   commandString.append(") values (");

   for(int i=0; i< columns.count(); i++)
   {
        QString col = columns.at(i);
        QString outStr;

        // FIXED: Force outStr to contain a placeholder for Mimer SQL right before it appends
        if (tgtConn->servertype() == "MimerSql") {
            outStr = "?";
        }

        if(outStr.isEmpty())
            qDebug() << "error: no value!";

        commandString.append(outStr);


     if(i < columns.count()-1)
      commandString.append(",");
   }
   commandString.append(")");

       query2->prepare(commandString);

       for(int i=0; i< columns.count(); i++)
       {
        QString col = columns.at(i);
        QVariant val;

        // === DEDICATED MIMER SQL DATA PRIMITIVE BINDING TRANSLATION ===
        // === DEDICATED MIMER SQL DATA PRIMITIVE BINDING TRANSLATION ===
        QVariant rawVal = query->value(i);
        QString colType = types.at(i).toLower();

        // 1. EXTRACT RAW STRING CONTENTS IMMEDIATELY FOR TEXT FIELDS
        QString cleanStr = "";
        bool isStringPrimitive = (rawVal.userType() == QMetaType::QString);
        if (isStringPrimitive) {
            cleanStr = rawVal.toString().trimmed();
        }

        // 2. DECISIVE BLANK OR MISSING VALUES DETERMINATION FILTER
        bool isValueBlank = rawVal.isNull() || (isStringPrimitive && cleanStr.isEmpty());

        // 3. CHROMATIC ROUTING ENGINE
        if (isValueBlank)
        {
            if (colType == "date")
            {
                // Fallback to explicit baseline string to satisfy NOT NULL constraints on date columns
                val = QString("1800-01-01");
            }
            else if (colType == "timestamp" || colType == "datetime")
            {
                // Fallback to explicit baseline string to satisfy NOT NULL constraints on timestamp columns
                val = QString("1800-01-01 00:00:00.000");
            }
            else if (colType.startsWith("int") || colType == "integer" || colType.startsWith("decimal"))
            {
                // Force true database NULL for missing numeric entries
                val = QVariant(QMetaType::fromType<QString>());
            }
            else
            {
                // Allow regular character columns to hold a safe blank value ("") to satisfy text NOT NULL rules
                val = QByteArray("");
            }
        }
        else if (rawVal.userType() == QMetaType::QDateTime)
        {
            QString dtStr = rawVal.toDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            val = dtStr;
        }
        else if (rawVal.userType() == QMetaType::QDate)
        {
            val = rawVal.toDate().toString("yyyy-MM-dd");
        }
        else if (isStringPrimitive)
        {
            // === CLEAN TOXIC ZERO-DATE ARRAYS ===
            if (cleanStr == "0000-00-00" || cleanStr == "0000/00/00")
            {
                cleanStr = "1800-01-01";
            }

            if (cleanStr.contains('/') && cleanStr.length() == 10)
            {
                cleanStr.replace('/', '-');
            }
            else if (cleanStr.contains('T') && cleanStr.endsWith('Z'))
            {
                cleanStr.replace('T', ' ');
                cleanStr.replace('Z', "");
            }

            // Guard against text-encoded date strings mapping to raw TIMESTAMP columns
            if (colType == "timestamp" || colType == "datetime")
            {
                if (cleanStr.length() == 10 && cleanStr.count('-') == 2)
                {
                    cleanStr.append(" 00:00:00.000");
                }
                val = cleanStr;
            }
            else if (colType == "date")
            {
                val = cleanStr;
            }
            else
            {
                val = cleanStr.toUtf8();
            }
        }
        else
        {
            // Handle integers, flags, and other basic numeric primitives safely
            val = rawVal;
        }

        // Bind by index position cleanly
        query2->bindValue(i, val);

       }
       // FIXED: Capture return status to satisfy downstream error checkers cleanly
       bQuery = query2->exec();


   if(!bQuery)
   {
    QSqlError err = query2->lastError();
    SQLERROR_E(std::move(query2));
    qDebug() << "error code:" << err.nativeErrorCode() << " " << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    qDebug() << "values: " << displayQueryValues(query);
    errors++;
   }
   else
   {
    added++;
   }

  } // end while(query->next() loop
  if (query2) {
         query2->finish();
     }
  sendProgress();
 }
 // === DECISIVE ACTION: DESTROY ACTIVE HEAP CURSORS IMMEDIATELY ===
 if (query) {
       query->finish();  // Halts the source reader data scan engine explicitly
       delete query;     // Deallocates the object to unlock the ODBC register lanes
       query = nullptr;
   }

 if (query2) {
        query2->finish(); // Halts the writer engine explicitly [2]
        delete query2;    // Deallocates memory to free the open statement registers [2]
        query2 = nullptr;
  }

  qDebug() << "Mimer Comments: " + QString("%1").arg(added) + " added, " + QString("%1").arg(updated) + " updated.";
  emit uncheck("chk" + table);

  if (tgtConn->servertype() == "MimerSql") {
     qDebug() << "Enforcing explicit transaction commit for table: " << table;
     _targetDb.commit();
 }
 return true;
}

void ExportSql::closeLogFile(){
    if(logfile)
    {
     logfile->flush();
     logfile->close();
     logfile = nullptr;
     stream = nullptr;
     logfile = nullptr;
    }
}

bool ExportSql::areTableDefsEqual(QString table, Connection* c1, Connection* c2, QSqlDatabase db1, QSqlDatabase db2)
{
 QStringList columns1 = SQL::instance()->listColumns(table, c1->servertype(), db1);
 QStringList columns2 = SQL::instance()->listColumns(table, c2->servertype(), db2);
 if(columns1.count() != columns2.count())
 {
  qDebug() << table << " column counts differ! " << "src: " << columns1.count() << " dst: " << columns2.count();
  return false;
 }
 for(int i=0; i < columns1.count(); i++)
 {
  if(columns1.at(i).compare(columns2.at(i),Qt::CaseInsensitive)!=0)
  {
   qDebug() << table << " column names or sequence differ";
   return false;
  }
 }
 QStringList pk1 = SQL::instance()->listPkColumns(table, c1->servertype(), db1);
 QStringList pk2 = SQL::instance()->listPkColumns(table, c2->servertype(), db2);
 if(pk1.count() != pk2.count())
 {
  qDebug() << table << " primary key counts differ";
  return false;
 }

 QList<FKInfo> info1 = SQL::instance()->getForeignKeyInfo(db1,config->currConnection, table);
 QList<FKInfo> info2 = SQL::instance()->getForeignKeyInfo(db2,tgtConn, table);
 if(info1.count() != info2.count())
 {
  qDebug() << table << " foreign key counts differ";
  return false;
 }
 return true;
}

QString ExportSql::displayQueryValues(QSqlQuery* query)
{
 QString str;
 for(int i=0; i < query->boundValues().count();i++) {
  if(i > 0)
   str.append(",");
#if QT_VERSION >= 0x060600
  str.append(query->boundValueName(i) + "=" + query->boundValue(i).toString());
#else
  str.append(query->boundValue(i).toString());
#endif
 }
 return str;
}

void ExportSql::logError(QSqlQuery* query, bool ignored, int line)
{
 QDir logDir;
 if(!logDir.exists("logs"))
  logDir.mkdir("logs");
 if(logfile == nullptr)
 {
  logfile = new QFile(QDir::currentPath()+"/logs/exportLog_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm")+".log");
  if(!logfile->open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
   QString msg = tr("error opening %1 %2").arg(logfile->fileName(), logfile->errorString());
   qDebug() << msg;
   throw IOException(msg);
  }
  stream = new QTextStream(logfile);
 }
 *stream << QDateTime::currentDateTime().toString() << " at line " << QString::number(line) << (ignored?" IGNORED":" ..") << "\n";
 *stream << query->lastError().text() << " " << query->lastError().nativeErrorCode() << "\n";
 *stream << query->executedQuery()<< "\n";
 *stream << displayQueryValues(query) << "\n\n";
 stream->flush();
}
