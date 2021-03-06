#include "exportsql.h"

ExportSql::ExportSql(Configuration *cfg, bool bDropTable, QObject *parent) :
    QObject(parent)
{
 config = cfg;
 bOverride = false;
 m_parent = parent;
 this->bDropTables = bDropTable;
 tgtConn = NULL;
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
  return false;
 }
 {
  srcDb = QSqlDatabase();
  srcConn = config->currCity->connections.at(config->currCity->curConnectionId);
  tgtConn = config->currCity->connections.at(config->currCity->curExportConnId);

  qDebug()<< "Export from '" +srcConn->description() + "' --> '" + tgtConn->description() + "'";
  if(!targetDb.connectionName().contains("export"))
  {
#if 1
   targetDb = QSqlDatabase::addDatabase(tgtConn->driver(), "export");
   if(tgtConn->driver() == "QODBC" || tgtConn->driver() == "QODBC3")
   {
    targetDb.setDatabaseName(tgtConn->dsn());
    tgtDbType = "MsSql";
   }
   else
   {
    targetDb.setHostName(tgtConn->host());
    targetDb.setDatabaseName(tgtConn->database());
    if(tgtConn->driver() == "QSQLITE")
     tgtDbType= "Sqlite";
    else
     tgtDbType = "MySql";
   }

   targetDb.setUserName(tgtConn->uid());
   targetDb.setPassword(tgtConn->pwd());

   if(! targetDb.open())
   {
    qDebug()<< targetDb.lastError();
    return false;
   }
   else
   {
    if(targetDb.driverName() == "QODBC" || targetDb.driverName() == "QODBC3" )
    {
     if(tgtConn->useDatabase() != "default" && tgtConn->useDatabase() != "")
     {
      QSqlQuery query = QSqlQuery(targetDb);
      QString cmd = QString("use [%1]").arg(tgtConn->useDatabase());
      if(!query.exec(cmd))
      {
       SQLERROR(query);
       targetDb.close();
       return false;
      }
      cmd = "select db_name()";
      if(!query.exec(cmd))
      {
       SQLERROR(query);
       targetDb.close();
       return false;
      }
      while(query.next())
      {
       QString dbName = query.value(0).toString();
       if(dbName != tgtConn->useDatabase())
        return false;
      }
     }
    }
   }
   QStringList tableList;
   {
    SQL* sql = SQL::instance();
    sql->setConfig(config);

    tableList = sql->getTableList(targetDb, config->currConnection->servertype());
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
 exportParameters();
 exportCompanies();
 exportTractionTypes();
 exportAltRoute();
 exportComments();
 exportIntersections();
 exportStations();
 exportLineSegments();
 exportSegments();
 exportRoute();

 exportTerminals();
 exportRouteComments();
 return true;
}

bool ExportSql::exportAltRoute()
{
    srcDb = QSqlDatabase::database();
    if(!openDb()) return false;

    updateTimestamp("altRoute");
    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bQuery;
    bool bFound = false;

    getCount("altRoute");
    if(bDropTables)
    {
     dropTable("Routes",targetDb, tgtDbType); // delete because of foriegnKey constraints.
     if(!createAltRouteTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
     CommandText = "select route, routePrefix, routeAlpha, baseRoute, lastUpdate from altRoute  order by lastUpdate";
    else
     CommandText = "select route, routePrefox, routeAlpha, baseRoute, lastUpdate from altRoute where lastUpdate > :lastUpdated order by lastUpdate";
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bQuery = query.exec();
    if(!bQuery)
    {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 route=0, route2=0, baseRoute, baseRoute2;
    QString altRoute="", altRoute2="", routePrefix="", routePrefix2="";
    QDateTime lastUpdate, lastUpdate2;
    while(query.next())
    {
        route = query.value(0).toInt();
        routePrefix = query.value(1).toString();
        altRoute=query.value(2).toString();
        baseRoute = query.value(3).toInt();
        lastUpdate = query.value(4).toDateTime();

        if(!bDropTables)
        {
        CommandText = "select route, routePrefix, routeAlpha, baseRoute, lastUpdate from altRoute where route="+QString("%1").arg(route);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            SQLERROR(query2);
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            route2 = query2.value(0).toInt();
            routePrefix2 = query2.value(1).toString();
            altRoute2=query2.value(2).toString();
            baseRoute2=query2.value(3).toInt();
            lastUpdate2=query2.value(4).toDateTime();
            bFound = true;
        }
        }
        if(route== route2 && altRoute == altRoute2 && baseRoute == baseRoute2 && lastUpdate == lastUpdate2 && routePrefix == routePrefix2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(route2 == route)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update altRoute set routeAlpha='"+altRoute+"', baseRoute = :baseRoute, lastUpdate=:lastUpdate where route = "+QString("%1").arg(route);
            else
                CommandText = "update altRoute set routeAlpha='"+altRoute+"', baseRoute = :baseRoute, lastUpdate=:lastUpdate where route = "+QString("%1").arg(route);
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            query2.bindValue(":baseRoute", baseRoute);
            bQuery = query2.exec();
            if(!bQuery)
            {
                SQLERROR(query2);
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into altRoute (route, routePrefix, routeAlpha, baseRoute, lastUpdate) values("+QString("%1").arg(route) + ",'"+routePrefix+ "','"+ altRoute+ "', :baseRoute, :lastUpdate);";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[altRoute] ON;insert into altRoute (route, routePrefix, routeAlpha, baseRoute, lastUpdate) values("+QString("%1").arg(route) + ",'"+routePrefix+ "','"+altRoute+ "', :baseRoute, :lastUpdate);SET IDENTITY_INSERT [dbo].[altRoute] OFF;";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            query2.bindValue(":baseRoute", baseRoute);
            bQuery = query2.exec();
            if(!bQuery)
            {
             SQLERROR(query);
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();

    }
    qDebug()<<"altRoute: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkAltRoute");
    return true;
}

bool ExportSql::exportComments()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("Comments");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;
    bool bQuery;

//#ifdef WIN32
//    QString up = QString::fromUtf8("?");
//    QString down = QString::fromUtf8("?");
//#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
//#endif

    getCount("Comments");
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Comments",true);
    if(bDropTables)
    {
     if(!createCommentsTable(targetDb, tgtDbType))
      return false;
    }
     if(bDropTables)
      CommandText = "select commentKey, tags, comments, lastUpdate from Comments  order by lastUpdate";
     else
      CommandText = "select commentKey, tags, comments, lastUpdate from Comments where lastUpdate > :lastUpdated  order by lastUpdate";
     QSqlQuery query = QSqlQuery(srcDb);
     query.prepare(CommandText);
     if(!bDropTables)
      query.bindValue(":lastUpdated", lastUpdated);
      bQuery = query.exec();
     if(!bQuery)
     {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 key=0, key2=0;
    QString tags="", tags2="", comments="", comments2="";
    QDateTime lastUpdate, lastUpdate2;
    while(query.next())
    {
        key = query.value(0).toInt();
        tags=query.value(1).toString();
        comments = query.value(2).toString();
        lastUpdate = query.value(3).toDateTime();
        //if(srcConn->servertype() != "MySql")
        {
            comments.replace("&up", up);
            comments.replace("&down", up);
        }
    if(!bDropTables)
    {
        CommandText = "select commentKey, tags, comments, lastUpdate from Comments where commentKey="+QString("%1").arg(key);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            SQLERROR(query2);
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            bFound = true;
            key2 = query2.value(0).toInt();
            tags2=query2.value(1).toString();
            comments2=query2.value(2).toString();
            lastUpdate2 = query.value(3).toDateTime();
            //if(tgtConn->servertype() != "MySql")
            {
                comments2.replace("&up", up);
                comments2.replace("&down", down);
            }
            bFound = true;
        }
    }
        if(bFound && key== key2 && tags == tags2 && comments == comments2 && lastUpdate == lastUpdate2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        comments.replace(up, "&up");
        comments.replace(down, "&down");
        tags.replace("\n", " ");

        if(key == key2)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update  Comments set comments=:comments, tags =:tags, lastUpdate =:lastUpdate where commentKey = " + QString("%1").arg(key);
            else
            {
                CommandText = "update  Comments set comments=:comments, tags =:tags,lastUpdate=:lastUpdate where commentKey = " + QString("%1").arg(key);
            }
            query2.prepare(CommandText);
            query2.bindValue(":comments", comments);
            query2.bindValue(":tags", tags);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into Comments (commentKey, comments, tags, lastUpdate ) values(:commentKey, :comments,:tags, :lastUpdate)";
            else
            {
                CommandText = "SET IDENTITY_INSERT [dbo].[Comments] ON; insert into Comments (commentKey, comments, tags, lastUpdate ) values(:commentKey, :comments,:tags,:lastUpdate); SET IDENTITY_INSERT [dbo].[Comments] OFF";
            }
            query2.prepare(CommandText);
            query2.bindValue(":commentKey", key);
            query2.bindValue(":comments", comments);
            query2.bindValue(":tags", tags);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();
    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Comments",false);

    qDebug()<<"Comments: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkComments");

    return true;
}

bool ExportSql::exportCompanies()
{
    srcDb = QSqlDatabase::database();
    if(!openDb()) return false;
    updateTimestamp("Companies");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;
    bool bQuery;

    getCount("Companies");
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Companies",true);

    if(bDropTables)
    {
     if(!createCompaniesTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
    {
     if(srcConn->servertype() == "MySql")
        CommandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute, lastUpdate from Companies   order by lastUpdate";
     else
        CommandText = "select [key], description, startDate, endDate, firstRoute, lastRoute, lastUpdate from Companies   order by lastUpdate";
    }
    else
    {
        if(srcConn->servertype() == "MySql")
            CommandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute, lastUpdate from Companies where lastUpdate > :lastUpdated  order by lastUpdate";
        else
            CommandText = "select [key], description, startDate, endDate, firstRoute, lastRoute, lastUpdate from Companies where lastUpdate > :lastUpdated  order by lastUpdate";

    }
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bQuery = query.exec();
    if(!bQuery)
    {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 key=0, key2=0;
    QString description="", description2="";
    QDateTime lastUpdate;
    QDateTime startDate, startDate2, endDate, endDate2;
    qint32 firstRoute=0, firstRoute2=0, lastRoute=0, lastRoute2=0;
    while(query.next())
    {
        key = query.value(0).toInt();
        description=query.value(1).toString();
        startDate = query.value(2).toDateTime();
        endDate = query.value(3).toDateTime();
        firstRoute = query.value(4).toInt();
        lastRoute = query.value(5).toInt();
        lastUpdate = query.value(6).toDateTime();
        if(lastUpdate.isValid() || lastUpdate.isNull())
            lastUpdate = QDateTime::currentDateTimeUtc();

     if(!bDropTables)
     {
        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute from Companies where `key`="+QString("%1").arg(key);
        else
            CommandText = "select [key], description, startDate, endDate, firstRoute, lastRoute from Companies where [key]="+QString("%1").arg(key);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            SQLERROR(query2);
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            key2 = query2.value(0).toInt();
            description2=query2.value(1).toString();
            startDate2 = query2.value(2).toDateTime();
            endDate2 = query2.value(3).toDateTime();
            firstRoute2 = query2.value(4).toInt();
            lastRoute2 = query2.value(5).toInt();
            bFound = true;
        }
    }
        if(key== key2 && description == description2 && startDate.date() == startDate2.date() && endDate.date() == endDate2.date() && firstRoute == firstRoute2 && lastRoute == lastRoute2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(key == key2)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update Companies set description='"+description+"', startDate='"+startDate.toString("yyyy/MM/dd")+"', endDate = '"+endDate.toString("yyyy/MM/dd")+"', firstRoute="+QString("%1").arg(firstRoute)+",lastroute="+QString("%1").arg(lastRoute)+ ",lastUpdate=:lastUpdate where `key` = "+QString("%1").arg(key);
            else
                CommandText = "update Companies set description='"+description+"', startDate='"+startDate.toString("yyyy/MM/dd")+"', endDate = '"+endDate.toString("yyyy/MM/dd")+"', firstRoute="+QString("%1").arg(firstRoute)+",lastroute="+QString("%1").arg(lastRoute)+ ",lastUpdate=:lastUpdate where [key] = "+QString("%1").arg(key);
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into Companies (`key`, description, startDate, endDate, firstRoute, lastRoute, lastUpdate) values("+QString("%1").arg(key) + ",'" + description + "','"+startDate.toString("yyyy/MM/dd")+"','" + endDate.toString("yyyy/MM/dd")+"'," + QString("%1").arg(firstRoute) + "," + QString("%1").arg(lastRoute) +",:lastUpdate)";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[Companies] ON; insert into Companies ([key], description, startDate, endDate, firstRoute, lastRoute,lastUpdate) values("+QString("%1").arg(key) + ",'" + description + "','"+startDate.toString("yyyy/MM/dd")+"','" + endDate.toString("yyyy/MM/dd")+"'," + QString("%1").arg(firstRoute) + "," + QString("%1").arg(lastRoute) + ",:lastUpdate); SET IDENTITY_INSERT [dbo].[Companies] OFF";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();

    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Companies",false);

    if(!bNoDeletes)
    {
        // Process Companies deletes
        emit(progressMsg("Processing Companies deletes"));
        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key` from Companies";
        else
            CommandText = "select [key] from Companies ";
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            exit(EXIT_FAILURE);
        }
        QList<qint32> CompaniesList;
        while(query2.next())
        {
            key = query2.value(0).toInt();
            CompaniesList.append(key);
        }
        rowCount = CompaniesList.count();
        rowsCompleted = 0;
        for(int i =0; i < CompaniesList.count(); i++)
        {
            key = CompaniesList.at(i);
            if(srcConn->servertype() == "MySql")
                CommandText = "select count(*) from Companies  where `key` = "+QString("%1").arg(key);
            else
                CommandText = "select count(*) from Companies  where [key] = "+QString("%1").arg(key);
            QSqlQuery query = QSqlQuery(srcDb);
            bool bQuery = query.exec(CommandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                exit(EXIT_FAILURE);
            }
            int srcCount=0;
            while(query.next())
            {
                srcCount = query.value(0).toInt();
            }
            if(srcCount > 0)
            {
                sendProgress();
                continue;   // record exists, continue
            }

            if(tgtConn->servertype() != "MsSql")
                CommandText = "delete from Companies  where `key` = "+QString("%1").arg(key);
            else
                CommandText = "delete from Companies  where [key] = "+QString("%1").arg(key);
            QSqlQuery query2 = QSqlQuery(targetDb);
            bQuery = query2.exec(CommandText);
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            deleted += query2.numRowsAffected();

            sendProgress();
        }
    }

    qDebug()<<"Companies: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkCompanies");

    return true;
}

bool ExportSql::exportIntersections()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("Intersections");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;
    bool bQuery;

    getCount("Intersections");
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Intersections",true);

    if(bDropTables)
    {
     if(!createIntersectionsTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
    {
     if(srcConn->servertype() == "MySql")
        CommandText = "select `key`, street1, street2, latitude, longitude,lastUpdate from Intersections  order by lastUpdate";
    else
        CommandText = "select [key], street1, street2, latitude, longitude, lastUpdate from Intersections   order by lastUpdate";
    }
    else
    {
        if(srcConn->servertype() == "MySql")
            CommandText = "select `key`, street1, street2, latitude, longitude,lastUpdate from Intersections where lastUpdate > :lastUpdated  order by lastUpdate";
        else
            CommandText = "select [key], street1, street2, latitude, longitude, lastUpdate from Intersections where lastUpdate > :lastUpdated  order by lastUpdate";
    }
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bQuery = query.exec();
    if(!bQuery)
    {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 key=0, key2=0;
    QString street1="", street12="", street2="", street22="";
    QDateTime lastUpdate;
    double latitude=0, latitude2=0, longitude=0, longitude2=0;
    while(query.next())
    {
        key = query.value(0).toInt();
        street1=query.value(1).toString();
        street2 = query.value(2).toString();
        latitude = query.value(3).toDouble();
        longitude = query.value(4).toDouble();
        lastUpdate = query.value(5).toDateTime();
     if(!bDropTables)
     {

        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key`, street1, street2, latitude, longitude from Intersections where `key`="+QString("%1").arg(key);
        else
            CommandText = "select [key], street1, street2, latitude, longitude from Intersections where [key]="+QString("%1").arg(key);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            key2 = query.value(0).toInt();
            street12=query.value(1).toString();
            street22 = query.value(2).toString();
            latitude2 = query.value(3).toDouble();
            longitude2 = query.value(4).toDouble();
            bFound = true;
        }
        }
        if(key== key2 && street1 == street12 && street2 == street22 && latitude == latitude2 && longitude == longitude2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(key == key2)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update Intersections set street1='"+street1+"', street2='"+street2+"',latitude="+QString("%1").arg(latitude, 0,'f',8)+",longitude="+QString("%1").arg(longitude,0,'f',8)+",lastUpdate=:lastUpdate where `key` = "+QString("%1").arg(key);
            else
                CommandText = "update Intersections set street1='"+street1+"', street2='"+street2+"',latitude="+QString("%1").arg(latitude, 0,'f',8)+",longitude="+QString("%1").arg(longitude,0,'f',8)+",:lastUpdate where [key] = "+QString("%1").arg(key);
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into Intersections (`key`, street1, street2, latitude, longitude, lastUpdate) values(" + QString("%1").arg(key) + ",'" + street1 + "','" +street2 + "',"+QString("%1").arg(latitude, 0,'f',8)+","+QString("%1").arg(longitude,0,'f',8)+",:lastUpdate)";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[Intersections] ON; insert into Intersections ([key], street1, street2, latitude, longitude,lastUpdate) values(" + QString("%1").arg(key) + ",'" + street1 + "','" +street2 + "',"+QString("%1").arg(latitude, 0,'f',8)+","+QString("%1").arg(longitude,0,'f',8)+",:lastUpdate); SET IDENTITY_INSERT [dbo].[Intersections] OFF";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();

    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Intersections",false);

    qDebug()<<"Intersections: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkIntersections");

    return true;
}

bool ExportSql::exportTractionTypes()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("TractionTypes");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;

    getCount("TractionTypes");
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("TractionTypes",true);

    if(bDropTables)
    {
     if(!createTractionTypesTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
     CommandText = "select tractionType, description, displayColor, routeType, icon, lastUpdate from TractionTypes   order by lastUpdate";
    else
     CommandText = "select tractionType, description, displayColor, routeType, icon, lastUpdate from TractionTypes where lastUpdate > :lastUpdated  order by lastUpdate";

    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bool bQuery = query.exec();
    if(!bQuery)
    {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 tractionType=0, tractionType2=0, routeType=0, routeType2=0;
    QString description="", description2="", displayColor="", displayColor2="", icon="", icon2="";
    QDateTime lastUpdate;

    while(query.next())
    {
        tractionType = query.value(0).toInt();
        description=query.value(1).toString();
        displayColor=query.value(2).toString();
        routeType=query.value(3).toInt();
        icon=query.value(4).toString();
        lastUpdate = query.value(5).toDateTime();

     if(!bDropTables)
     {
        CommandText = "select tractionType, description, displayColor, routeType, icon from TractionTypes where tractionType="+QString("%1").arg(tractionType);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            exit(EXIT_FAILURE);
        }
        bool bFound = false;
        while(query2.next())
        {
            tractionType2 = query2.value(0).toInt();
            description2=query2.value(1).toString();
            displayColor2=query2.value(2).toString();
            routeType2=query2.value(3).toInt();
            icon2=query2.value(4).toString();
            bFound = true;
        }
        }
        if(tractionType== tractionType2 && description == description2 && displayColor == displayColor2 && routeType==routeType2 && icon==icon2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(bFound)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update TractionTypes set description='"+description+"',displayColor='"+displayColor+"',routeType="+QString("%1").arg(routeType)+",icon='"+icon+"', lastUpdate=:lastUpdate where tractionType = "+QString("%1").arg(tractionType);
            else
                CommandText = "update TractionTypes set description='"+description+"',displayColor='"+displayColor+"',routeType="+QString("%1").arg(routeType)+",icon='"+icon+"', lastUpdate=:lastUpdate where tractionType = "+QString("%1").arg(tractionType);
            query2.prepare(CommandText);
            query2.bindValue(":LastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into TractionTypes (tractionType, description, displayColor, routeType, icon, lastUpdate) values("+QString("%1").arg(tractionType) + ",'"+description+ "','"+displayColor+"',"+QString("%1").arg(routeType)+",'"+icon+"',:lastUpdate)";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[TractionTypes] ON; insert into TractionTypes (tractionType, description, displayColor, routeType, icon, lastUpdate) values("+QString("%1").arg(tractionType) + ",'"+description+ "','"+displayColor+"',"+QString("%1").arg(routeType)+",'"+icon+"',:lastUpdate);SET IDENTITY_INSERT [dbo].[TractionTypes] OFF ";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();
    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("TractionTypes",false);

    qDebug()<<"TractionType: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkTractionTypes");

    return true;
}

bool ExportSql::exportParameters()
{
    srcDb = QSqlDatabase::database();
    if(!openDb())
       return false;
    updateTimestamp("Parameters");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;
    bool bQuery;

    getCount("Parameters");
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Parameters",true);
    if(bDropTables)
    {
     if(!createParametersTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
    {
    if(srcConn->servertype() == "MySql")
        CommandText = "select `key`, lat, lon, title, city, minDate, maxDate, alphaRoutes, lastUpdate from Parameters   order by lastUpdate";
    else
        CommandText = "select [key], lat, lon, title, city, minDate, maxDate, alphaRoutes,lastUpdate from Parameters   order by lastUpdate";
    }
    else
    {
        if(srcConn->servertype() == "MySql")
            CommandText = "select `key`, lat, lon, title, city, minDate, maxDate, alphaRoutes, lastUpdate from Parameters where lastUpdate > :lastUpdated  order by lastUpdate";
        else
            CommandText = "select [key], lat, lon, title, city, minDate, maxDate, alphaRoutes,lastUpdate from Parameters where lastUpdate > :lastUpdated  order by lastUpdate";

    }
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bQuery = query.exec();
    if(!bQuery)
    {
        SQLERROR(query);
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 key=0, key2=0;
    QString title="", title2="", city="", city2="", alphaRoutes="", alphaRoutes2="";
    QDateTime lastUpdate;
    double lat=0, lat2=0, lon=0, lon2=0;
    QDate minDate, maxDate, minDate2, maxDate2;
    while(query.next())
    {
        key = query.value(0).toInt();
        lat= query.value(1).toDouble();
        lon=query.value(2).toDouble();
        title=query.value(3).toString();
        city=query.value(4).toString();
        minDate = query.value(5).toDate();
        maxDate = query.value(6).toDate();
        alphaRoutes=query.value(7).toString();
        lastUpdate = query.value(8).toDateTime();
        if(!bDropTables)
        {
        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key`, lat, lon, title, city, minDate, maxDate, alphaRoutes from Parameters where `key`="+QString("%1").arg(key);
        else
            CommandText = "select [key], lat, lon, title, city, minDate, maxDate, alphaRoutes from Parameters where [key]="+QString("%1").arg(key);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            key2 = query2.value(0).toInt();
            lat2= query2.value(1).toDouble();
            lon2=query2.value(2).toDouble();
            title2=query2.value(3).toString();
            city2=query2.value(4).toString();
            minDate2 = query.value(5).toDate();
            maxDate2 = query.value(6).toDate();
            alphaRoutes2=query.value(7).toString();
            bFound = true;
        }
        }
        if(key== key2 && lat == lat2 && lon == lon2 && title == title2 && city == city2 && alphaRoutes == alphaRoutes2 && minDate == minDate2 && maxDate == maxDate2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(key== key2)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update Parameters set lat="+QString("%1").arg(lat, 0, 'f',8) +",lon="+QString("%1").arg(lon, 0, 'f',8)+", title='"+title+"',city='"+city+"', minDate = '"+minDate.toString("yyyy/MM/dd") + "', maxDate = '"+maxDate.toString("yyyy/MM/dd")+"',alphaRoutes='"+alphaRoutes+"', lastUpdate =:lastUpdate where `key` = "+QString("%1").arg(key);
            else
                CommandText = "update Parameters set lat="+QString("%1").arg(lat, 0, 'f',8) +",lon="+QString("%1").arg(lon, 0, 'f',8)+", title='"+title+"',city='"+city+"',maxDate = '"+maxDate.toString("yyyy/MM/dd")+"', alphaRoutes='"+alphaRoutes+"',lastUpdate=:lastUpdate where [key] = "+QString("%1").arg(key);
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into Parameters (`key`, lat, lon, title, city, minDate, maxDate, alphaRoutes, lastUpdate) values("+QString("%1").arg(key) + ","+QString("%1").arg(lat, 0, 'f',8) +","+QString("%1").arg(lon, 0, 'f',8)+", '"+title+"','"+city+"', '"+minDate.toString("yyyy/MM/dd") + "','" + maxDate.toString("yyyy/MM/dd") + "','"+alphaRoutes+"',:lastUpdate)";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[Parameters] ON; insert into Parameters ([key], lat, lon, title, city, minDate, maxDate, alphaRoutes, lastUpdate) values("+QString("%1").arg(key) + ","+QString("%1").arg(lat, 0, 'f',8) +","+QString("%1").arg(lon, 0, 'f',8)+", '"+title+"','"+city+"', '"+minDate.toString("yyyy/MM/dd") + "','" + maxDate.toString("yyyy/MM/dd") + "','"+alphaRoutes+"',:lastUpdate); SET IDENTITY_INSERT [dbo].[Parameters] OFF";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();
    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Parameters",false);

    qDebug()<<"Parameters: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkParameters");

    return true;
}


bool ExportSql::exportLineSegments()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("LineSegment");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    int newSegments=0;

    getCount("LineSegment");
    QString CommandText;
    if(srcConn->servertype() == "MySql")
        CommandText = "select `key`, a.startLat, a.startLon, a.endLat, a.endLon, streetName, a.segmentId, sequence, a.length, s.description, s.oneWay, a.lastUpdate, s.lastUpdate from LineSegment a join Segments s on a.segmentId = s.segmentId where a.lastUpdate > :lastUpdated  order by a.lastUpdate";
    else
        CommandText = "select [key], a.startLat, a.startLon, a.endLat, a.endLon, streetName, a.segmentId, sequence, a.length, s.description, s.oneWay, a.lastUpdate, s.lastUpdate from LineSegment a join Segments s on a.segmentId = s.segmentId where a.lastUpdate > :lastUpdated  order by a.lastUpdate";
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    query.bindValue(":lastUpdated",lastUpdated);
    bool bQuery = query.exec();
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 key=0, key2=0, segmentId=0, segmentId2=0, sequence=0, sequence2=0;
    QString streetName="", streetName2="", description="", description2="", oneWay="", oneWay2="";
    QDateTime lastUpdate, lastUpdateS, lastUpdate2;
    double startLat=0, startLat2=0, startLon=0, startLon2=0;
    double endLat=0, endLat2=0, endLon=0, endLon2=0;
    double length = 0, length2=0;
    while(query.next())
    {
        key = query.value(0).toInt();
        startLat= query.value(1).toDouble();
        startLon = query.value(2).toDouble();
        endLat = query.value(3).toDouble();
        endLon = query.value(4).toDouble();
        streetName = query.value(5).toString();
        segmentId = query.value(6).toInt();
        sequence = query.value(7).toInt();
        length = query.value(8).toDouble();

        description = query.value(9).toString();
        oneWay = query.value(10).toString();
        lastUpdate = query.value(11).toDateTime();
        lastUpdateS = query.value(12).toDateTime();

        CommandText = "select segmentId, description, oneWay from Segments where segmentId="+QString("%1").arg(segmentId);
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            if(!Retry(&targetDb, &query2, CommandText))
                exit(EXIT_FAILURE);

        }
        while(query2.next())
        {
            segmentId2 = query2.value(0).toInt();
            description2 = query2.value(1).toString();
            oneWay = query2.value(2).toString();
        }
        if(segmentId != segmentId2)
        {
            // segment doesn't exist, insert a minimal record
           if(tgtConn->servertype() != "MsSql")
               CommandText = "insert into Segments (segmentId, description, oneWay, lastUpdate) values("+QString("%1").arg(segmentId)+",'"+description+"','"+oneWay+"', :lastUpdateS)";
           else
           {
               //setIdentityInsert("Segments", true);
               CommandText = "SET IDENTITY_INSERT [dbo].[Segments] ON; insert into Segments (segmentId, description, oneWay, lastUpdate) values("+QString("%1").arg(segmentId)+",'"+description+"','"+oneWay+"',:lastUpdateS); SET IDENTITY_INSERT [dbo].[Segments] OFF";
           }
           query2.prepare(CommandText);
           query2.bindValue(":lastUpdateS", lastUpdateS);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                if(!Retry(&targetDb, &query2, ""))
                    exit(EXIT_FAILURE);

                errors++;
                //exit(EXIT_FAILURE);
            }
            else
                newSegments++;
//            if(tgtConn->servertype() == "MsSql")
//                setIdentityInsert("Segments", false);
        }

        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key`, startLat, startLon, endLat, endLon, streetName, segmentId, sequence, length, lastUpdate from LineSegment where `key`="+QString("%1").arg(key);
        else
            CommandText = "select [key], startLat, startLon, endLat, endLon, streetName, segmentId, sequence, length, lastUpdate from LineSegment where [key]="+QString("%1").arg(key);
        query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            if(!Retry(&targetDb, &query2, CommandText))
                exit(EXIT_FAILURE);
        }
        bool bFound = false;
        while(query2.next())
        {
            bFound = true;
            key2 = query2.value(0).toInt();
            startLat2= query2.value(1).toDouble();
            startLon2 = query2.value(2).toDouble();
            endLat2 = query2.value(3).toDouble();
            endLon2 = query2.value(4).toDouble();
            streetName2 = query2.value(5).toString();
            segmentId2 = query2.value(6).toInt();
            sequence2 = query2.value(7).toInt();
            length2 = query2.value(8).toDouble();
            lastUpdate2 = query2.value(9).toDateTime();
        }
        if(bFound && key== key2 && startLat == startLat2 && startLon == startLon2 && endLat == endLat2 && endLon== endLon2 && streetName == streetName2 && segmentId == segmentId2 && sequence == sequence2 && length == length2 && lastUpdate == lastUpdate2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(bFound)
        {
            if(tgtConn->servertype() != "MsSql")
                //CommandText = "update LineSegment set startLat="+QString("%1").arg(startLat, 0, 'f',8) +",startLon="+QString("%1").arg(startLon, 0, 'f',8)+", endLat="+QString("%1").arg(endLat, 0, 'f',8) +",endLon="+QString("%1").arg(endLon, 0, 'f',8)+",streetName='"+streetName+"',segmentId="+QString("%1").arg(segmentId)+", sequence="+QString("%1").arg(sequence)+",length="+QString("%1").arg(length, 0, 'f', 8)+",lastUpdate=:lastUpdate where `key` = "+QString("%1").arg(key);
                CommandText = "update LineSegment set startLat="+QString("%1").arg(startLat, 0, 'f',8) +",startLon="+QString("%1").arg(startLon, 0, 'f',8)+", endLat="+QString("%1").arg(endLat, 0, 'f',8) +",endLon="+QString("%1").arg(endLon, 0, 'f',8)+",streetName='"+streetName+"',length="+QString("%1").arg(length, 0, 'f', 8)+",lastUpdate=:lastUpdate where `key` = "+QString("%1").arg(key);

            else
                CommandText = "update LineSegment set startLat="+QString("%1").arg(startLat, 0, 'f',8) +",startLon="+QString("%1").arg(startLon, 0, 'f',8)+", endLat="+QString("%1").arg(endLat, 0, 'f',8) +",endLon="+QString("%1").arg(endLon, 0, 'f',8)+",streetName='"+streetName+"',segmentId="+QString("%1").arg(segmentId)+", sequence="+QString("%1").arg(sequence)+",length="+QString("%1").arg(length, 0, 'f', 8)+",lastUpdate=:lastUpdate where [key] = "+QString("%1").arg(key);

            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                if(!Retry(&targetDb, &query2, ""))
                    exit(EXIT_FAILURE);

                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into LineSegment (`key`, startLat, startLon, endLat, endLon, streetName, segmentId, sequence, length, lastUpdate) values("+QString("%1").arg(key) + ","+QString("%1").arg(startLat, 0, 'f',8) +","+QString("%1").arg(startLon, 0, 'f',8)+", "+QString("%1").arg(endLat, 0, 'f',8) +","+QString("%1").arg(endLon, 0, 'f',8)+",'"+streetName+"',"+QString("%1").arg(segmentId)+","+QString("%1").arg(sequence)+","+QString("%1").arg(length, 0, 'f', 8)+",:lastUpdate)";
            else
            {
                //setIdentityInsert("LineSegment", true);
                CommandText = "SET IDENTITY_INSERT [dbo].[LineSegment] ON; insert into LineSegment ([key], startLat, startLon, endLat, endLon, streetName, segmentId, sequence, length, lastUpdate) values("+QString("%1").arg(key) + ","+QString("%1").arg(startLat, 0, 'f',8) +","+QString("%1").arg(startLon, 0, 'f',8)+", "+QString("%1").arg(endLat, 0, 'f',8) +","+QString("%1").arg(endLon, 0, 'f',8)+",'"+streetName+"',"+QString("%1").arg(segmentId)+","+QString("%1").arg(sequence)+","+QString("%1").arg(length, 0, 'f', 8)+",:lastUpdate); SET IDENTITY_INSERT [dbo].[LineSegment] OFF";

            }
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
//                if(!Retry(&targetDb, &query2, ""))
//                    exit(EXIT_FAILURE);

                errors++;
            }
            else
                added++;
//            if(tgtConn->servertype() == "MsSql")
//                setIdentityInsert("LineSegment", false);
        }
        sendProgress();

    }
    if(!bNoDeletes)
    {
        // Process LineSegment deletes
        emit(progressMsg("Processing LineSegment deletes"));
        if(tgtConn->servertype() != "MsSql")
            CommandText = "select `key` from LineSegment where lastUpdate <'"+ beginTime + "'";
        else
            CommandText = "select [key] from LineSegment where lastUpdate <'"+ beginTime + "'";
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            if(!Retry(&targetDb, &query2, CommandText))
                exit(EXIT_FAILURE);

        }
        QList<qint32> lineSegmentList;
        while(query2.next())
        {
            key = query2.value(0).toInt();
            lineSegmentList.append(key);
            qApp->processEvents(QEventLoop::AllEvents);
        }
        rowCount = lineSegmentList.count();
        rowsCompleted = 0;
        for(int i =0; i < lineSegmentList.count(); i++)
        {
            key = lineSegmentList.at(i);
            if(srcConn->servertype() == "MySql")
                CommandText = "select count(*) from LineSegment  where `key` = "+QString("%1").arg(key);
            else
                CommandText = "select count(*) from LineSegment  where [key] = "+QString("%1").arg(key);
            QSqlQuery query = QSqlQuery(srcDb);
            bool bQuery = query.exec(CommandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                exit(EXIT_FAILURE);
            }
            int srcCount=0;
            while(query.next())
            {
                srcCount = query.value(0).toInt();
            }
            if(srcCount > 0)
            {
                sendProgress();
                continue;   // record exists, continue
            }

            if(tgtConn->servertype() != "MsSql")
                CommandText = "delete from LineSegment  where `key` = "+QString("%1").arg(key);
            else
                CommandText = "delete from LineSegment  where [key] = "+QString("%1").arg(key);
            QSqlQuery query2 = QSqlQuery(targetDb);
            bQuery = query2.exec(CommandText);
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                if(!Retry(&targetDb, &query2, CommandText))
                    exit(EXIT_FAILURE);

                errors++;
            }
            if(query2.numRowsAffected()>0)
                deleted += query2.numRowsAffected();
            else
                qDebug()<< "fail? "+ CommandText;

            sendProgress();
        }
    }

    qDebug()<<"LineSegments: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors";
    qDebug()<< "  " + QString("%1").arg(newSegments)+ " new segments added\n";
    emit uncheck("chkLineSegments");

    return true;
}

bool ExportSql::exportSegments()
{
  QSqlDatabase srcDb = QSqlDatabase::database();
      if(!openDb()) return false;

  updateTimestamp("Segments");

  added=0;
  updated=0;
  deleted=0;
  errors=0;
  notUpdated=0;
  int lineSegmentsDeleted=0;
  int routesDeleted=0;
  QString CommandText;

  getCount("Segments", bDropTables);

  if(bDropTables)
  {
   dropTable("Routes",targetDb, tgtDbType); // delete because of foriegnKey constraints.
   if(!createSegmentsTable(targetDb, tgtDbType))
      return false;
  }

//    if(tgtConn->servertype() != "MySql")
//        setIdentityInsert("Segments", true);
  if(!bDropTables)
  {
   CommandText = "select segmentId, description, oneWay, type, startLat, startLon, endLat, endLon, length, points, startDate, endDate, direction, lastUpdate, street, pointArray, tracks from Segments where lastUpdate > :lastUpdated  order by lastUpdate";
  }
  else
  {
   CommandText = "select segmentId, description, oneWay, type, startLat, startLon, endLat, endLon, length, points, startDate, endDate, direction, lastUpdate, street, pointArray, tracks from Segments";
  }

  QSqlQuery query = QSqlQuery(srcDb);
  QSqlQuery query2 = QSqlQuery(targetDb);

  query.prepare(CommandText);
  if(!bDropTables)
   query.bindValue(":lastUpdated", lastUpdated);
  bool bQuery = query.exec();
  if(!bQuery)
  {
   SQLERROR(query);
      //db.close();
      exit(EXIT_FAILURE);
  }
  qint32 segmentId=0, segmentId2=0, type=0, type2=0, points=0, points2=0;
  qint8 tracks = 0, tracks2 = 0;
  QString description="", description2="", oneWay="", oneWay2=0, direction="", direction2="", street="", street2="", pointArray ="", pointArray2="";
  QDateTime lastUpdate, lastUpdate2;
  double startLat=0, startLat2=0, startLon = 0, startLon2=0, endLat=0, endLat2=0, endLon=0, endLon2=0, length=0, length2=0;
  QDateTime startDate, startDate2, endDate, endDate2;
  while(query.next())
  {
   segmentId = query.value(0).toInt();
   description = query.value(1).toString();
   oneWay = query.value(2).toString();
   type = query.value(3).toInt();
   startLat = query.value(4).toDouble();
   startLon = query.value(5).toDouble();
   endLat = query.value(6).toDouble();
   endLon = query.value(7).toDouble();
   length = query.value(8).toDouble();
   points = query.value(9).toInt();
   startDate = query.value(10).toDateTime();
   endDate = query.value(11).toDateTime();
   direction=query.value(12).toString();
   lastUpdate = query.value(13).toDateTime();
   street = query.value(14).toString();
   pointArray = query.value(15).toString();
   tracks = query.value(16).toInt();
   bool bFound = false;

   if(!bDropTables)
   {
    CommandText = "select segmentId, description, oneWay, type, startLat, startLon, endLat, endLon, length, points, startDate, endDate, direction,lastUpdate, street, pointArray, tracks from Segments where segmentId="+QString("%1").arg(segmentId);
    bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
     SQLERROR(query2);
     //db.close();
     if(!Retry(&targetDb, &query2, CommandText))
         exit(EXIT_FAILURE);
    }
    bool bFound = false;

    while(query2.next())
    {
     bFound = true;
     segmentId2 = query2.value(0).toInt();
     description2 = query2.value(1).toString();
     oneWay2 = query2.value(2).toString();
     type2 = query2.value(3).toInt();
     startLat2 = query2.value(4).toDouble();
     startLon2 = query2.value(5).toDouble();
     endLat2 = query2.value(6).toDouble();
     endLon2 = query2.value(7).toDouble();
     length2 = query2.value(8).toDouble();
     points2 = query2.value(9).toInt();
     startDate2 = query2.value(10).toDateTime();
     endDate2 = query2.value(11).toDateTime();
     direction2=query2.value(12).toString();
     lastUpdate2 = query2.value(13).toDateTime();
     street2 = query.value(14).toString();
     pointArray2 = query.value(15).toString();
     tracks2 = query.value(16).toInt();
    }
    if(bFound && segmentId== segmentId2 && description == description2 && oneWay == oneWay2 && type == type2 && startLat == startLat2 && startLon == startLon2 && endLat == endLat2 && endLon == endLon2 && length == length2 && points==points2 && startDate.date() == startDate2.date() && endDate.date() == endDate2.date() && direction == direction2 && lastUpdate == lastUpdate2 && street == street2 && pointArray == pointArray2 && tracks == tracks2)
    {
     notUpdated++;
     sendProgress();
     continue;
    }
   }
   if(bDropTables)
    bFound = false;
   if(bFound)
   {
    if(tgtConn->servertype() != "MsSql")
     CommandText = "update Segments set description='"+description+"', oneWay= '"+oneWay+ "', type="+QString("%1").arg(type)+", startLat="+QString("%1").arg(startLat, 0, 'f', 8)+",startLon="+QString("%1").arg(startLon, 0, 'f', 8)+",endLat="+QString("%1").arg(endLat, 0, 'f',8)+",endLon="+QString("%1").arg(endLon, 0, 'f',8)+",length="+QString("%1").arg(length,0,'f',8)+",points="+QString("%1").arg(length,0,'f',8)+",startDate='"+startDate.toString("yyyy/MM/dd")+"',endDate='"+endDate.toString("yyyy/MM/dd")+"',direction='"+direction+"',lastUpdate =:lastUpdate, street='"+ street +"', pointArray ='"+ pointArray +"', tracks = " + QString::number(tracks) + " where segmentId = "+QString("%1").arg(segmentId);
    else
     CommandText = "update Segments set description='"+description+"', oneWay= '"+oneWay+ "', type="+QString("%1").arg(type)+", startLat="+QString("%1").arg(startLat, 0, 'f', 8)+",startLon="+QString("%1").arg(startLon, 0, 'f', 8)+",endLat="+QString("%1").arg(endLat, 0, 'f',8)+",endLon="+QString("%1").arg(endLon, 0, 'f',8)+",length="+QString("%1").arg(length,0,'f',8)+",points="+QString("%1").arg(length,0,'f',8)+",startDate='"+startDate.toString("yyyy/MM/dd")+"',endDate='"+endDate.toString("yyyy/MM/dd")+"',direction='"+direction+"',lastUpdate=:lastUpdate, street='"+ street +"', pointArray ='"+ pointArray +"', tracks = " + QString::number(tracks) + " where segmentId = "+QString("%1").arg(segmentId);
    query2.prepare(CommandText);
    query2.bindValue(":lastUpdate", lastUpdate);
    bQuery = query2.exec();
    if(!bQuery)
    {
     SQLERROR(query2);
     //db.close();
     //exit(EXIT_FAILURE);
     if(!Retry(&targetDb, &query2, ""))
         exit(EXIT_FAILURE);
     errors++;
    }
    else
     updated++;

    // If the new segment has less points than the old, Delete any LineSegments left over
    if(points2 > points)
    {
     CommandText = "delete from LineSegment where segmentId = " + QString("%1").arg(segmentId)+ " and sequence >= "+ QString("%1").arg(points);
     bQuery = query2.exec(CommandText);
     if(!bQuery)
     {
      SQLERROR(query2);
      //db.close();
      //exit(EXIT_FAILURE);
//                    if(!Retry(&targetDb, &query2, CommandText))
//                        exit(EXIT_FAILURE);
      errors++;
     }
     if(query2.numRowsAffected()>0)
      deleted += query2.numRowsAffected();
    }
   }
   else
   {
    // Insert the new segment
    if(tgtConn->servertype() != "MsSql")
     CommandText = "insert into Segments (segmentId, description, oneWay, type, startLat, startLon, endLat, endLon, length, points, startDate, endDate, direction, street, pointArray, tracks, lastUpdate) values("+QString("%1").arg(segmentId) + ",'"+description+"', '"+oneWay+ "', "+QString("%1").arg(type)+", "+QString("%1").arg(startLat, 0,'f',8)+","+QString("%1").arg(startLon, 0, 'f', 8)+","+QString("%1").arg(endLat, 0, 'f',8)+","+QString("%1").arg(endLon, 0,'f',8)+","+QString("%1").arg(length,0,'f',8)+","+QString("%1").arg(points)+",'"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd")+"','"+direction + "','" + street + "', '" + pointArray +"', "+QString::number(tracks) + ", :lastUpdate)";
    else
    {
     //setIdentityInsert("Segments", true);
     CommandText = "SET IDENTITY_INSERT [dbo].[Segments] ON; insert into Segments (segmentId, description, oneWay, type, startLat, startLon, endLat, endLon, length, points, startDate, endDate, direction, street, pointArray, tracks, lastUpdate) values("+QString("%1").arg(segmentId) + ",'"+description+"', '"+oneWay+ "', "+QString("%1").arg(type)+", "+QString("%1").arg(startLat, 0,'f',8)+","+QString("%1").arg(startLon, 0, 'f', 8)+","+QString("%1").arg(endLat, 0, 'f',8)+","+QString("%1").arg(endLon, 0,'f',8)+","+QString("%1").arg(length,0,'f',8)+","+QString("%1").arg(points)+",'"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd")+"','"+direction + "','" + street + "', '" + pointArray +"', "+QString::number(tracks) + ",:lastUpdate); SET IDENTITY_INSERT [dbo].[Segments] OFF; ";
    }
    bQuery = query2.prepare(CommandText);
    if(!bQuery)
    {
     SQLERROR(query2);
    }
    query2.bindValue(":lastUpdate",lastUpdate);
    bQuery = query2.exec();
    if(!bQuery)
    {
     SQLERROR(query2);
     QSqlError err = query2.lastError();
     qDebug() << err.databaseText() + "\n";
     qDebug() << err.driverText() + "\n";
     //db.close();
     //exit(EXIT_FAILURE);
     if(!Retry(&targetDb, &query2, ""))
         exit(EXIT_FAILURE);

     errors++;
    }
    else
     added++;
   }
   sendProgress();
  }

//    if(tgtConn->servertype() != "MySql")
//        setIdentityInsert("Segments", false);

  if(!bNoDeletes && !bDropTables)
  {
   // Process Segment deletes
   emit(progressMsg("Processing segment deletes"));
   CommandText = "select segmentId from Segments where lastUpdate>'" + beginTime + "'";
   QSqlQuery query2 = QSqlQuery(targetDb);
   bQuery = query2.exec(CommandText);
   if(!bQuery)
   {
    SQLERROR(query2);
       //db.close();
       if(!Retry(&targetDb, &query2, CommandText))
           exit(EXIT_FAILURE);

   }
   QList<qint32> tgtSegmentList;
   while(query2.next())
   {
       tgtSegmentList.append(query2.value(0).toInt());
       qApp->processEvents(QEventLoop::AllEvents);
   }
   rowCount = tgtSegmentList.count();
   rowsCompleted = 0;
   for(int i =0; i < tgtSegmentList.count(); i++)
   {
    qint32 segment = tgtSegmentList.at(i);
    CommandText = "select count(*) from Segments  where segmentId = "+QString("%1").arg(segment);
    QSqlQuery query = QSqlQuery(srcDb);
    bool bQuery = query.exec(CommandText);
    if(!bQuery)
    {
     SQLERROR(query);
     //db.close();
     exit(EXIT_FAILURE);
    }
    int srcCount=0;
    while(query.next())
    {
     srcCount = query.value(0).toInt();
    }
    if(srcCount > 0)
    {
     sendProgress();
     continue;   // record exists, continue
    }

    CommandText = "delete from Routes where lineKey = "+QString("%1").arg(segment);
    QSqlQuery query2 = QSqlQuery(targetDb);
    bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
     SQLERROR(query2);
     //db.close();
     if(!Retry(&targetDb, &query2, CommandText))
         exit(EXIT_FAILURE);

    }
    if(query2.numRowsAffected()>0)
     routesDeleted += query2.numRowsAffected();
    else
     qDebug()<< "fail? "+ CommandText;

    CommandText = "delete from Segments where segmentId = "+QString("%1").arg(segment);
    query2 = QSqlQuery(targetDb);
    bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
     SQLERROR(query2);
     //db.close();
     if(!Retry(&targetDb, &query2, CommandText))
         exit(EXIT_FAILURE);

    }
    if(query2.numRowsAffected()>0)
     deleted += query2.numRowsAffected();
    else
     qDebug()<< "fail? "+ CommandText;


    // delete any orphaned line segments
    CommandText = "delete from LineSegment where segmentId = "+QString("%1").arg(segment);
    query2 = QSqlQuery(targetDb);
    bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
     SQLERROR(query2);
     //db.close();
     if(!Retry(&targetDb, &query2, CommandText))
         exit(EXIT_FAILURE);

    }
    if(query2.numRowsAffected()>0)
     lineSegmentsDeleted += query2.numRowsAffected();
    else
     qDebug()<< "fail? "+ CommandText;


    sendProgress();
   }
  }
  qDebug()<<"Segments: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors";
  qDebug()<< QString("%1").arg(lineSegmentsDeleted) + " lineSegments deleted, " + QString("%1").arg(routesDeleted)+ " routesDeleted\n";
  emit uncheck("chkSegments");

  return true;
 }

 bool ExportSql::exportRoute()
 {
  srcDb = QSqlDatabase::database();
      if(!openDb()) return false;

  updateTimestamp("Routes");

  added=0;
  updated=0;
  deleted=0;
  errors=0;
  notUpdated=0;

  getCount("Routes", bDropTables);
  if(bDropTables)
  {
   if(!createRouteTable(targetDb, tgtConn->servertype()))
    return false;
  }
  QString CommandText;
  if(bDropTables)
  {
   if(srcConn->servertype() == "MsSql")
    CommandText = "select route, LTRIM(RTRIM(name)), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes  order by lastUpdate";
   else
    CommandText = "select route, TRIM(name), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes order by lastUpdate";
  }
  else
  {
   if(srcConn->servertype() == "MsSql")
    CommandText = "select route, LTRIM(RTRIM(name)), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where lastUpdate > :lastUpdated  order by lastUpdate";
   else
    CommandText = "select route, TRIM(name), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where lastUpdate > :lastUpdated  order by lastUpdate";

  }

  QSqlQuery query = QSqlQuery(srcDb);
  QSqlQuery query2 = QSqlQuery(targetDb);

  query.prepare(CommandText);
  if(!bDropTables)
   query.bindValue(":lastUpdated", lastUpdated);
  bool bQuery = query.exec();
  if(!bQuery)
  {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      //db.close();
      if(!Retry(&srcDb, &query, ""))
          exit(EXIT_FAILURE);
  }
  qint32 route=0, route2=0, lineKey=0, lineKey2=0, companyKey, companyKey2, tractionType, tractionType2, next=0, next2=0, prev=0, prev2=0, normalEnter=0, normalEnter2=0, normalLeave=0, normalLeave2=0, reverseEnter=0, reverseEnter2=0, reverseLeave=0, reverseLeave2 =0;
  QString name="", name2="", direction="", direction2="";
  QDateTime lastUpdate, lastUpdate2;
  QDateTime startDate, startDate2, endDate, endDate2;
  while(query.next())
  {
   route = query.value(0).toInt();
   name=query.value(1).toString();
   startDate = query.value(2).toDateTime();
   endDate = query.value(3).toDateTime();
   lineKey = query.value(4).toInt();
   companyKey = query.value(5).toInt();
   tractionType= query.value(6).toInt();
   direction = query.value(7).toString();
   next = query.value(8).toInt();
   prev = query.value(9).toInt();
   normalEnter = query.value(10).toInt();
   normalLeave = query.value(11).toInt();
   reverseEnter = query.value(12).toInt();
   reverseLeave = query.value(13).toInt();
   lastUpdate = query.value(14).toDateTime();


   route2 = -1;
   name2 = "??";
   lineKey2 = -1;

   bool bFound = false;

   if(!bDropTables)
   {
    CommandText = "select route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where route = "+QString("%1").arg(route)+" and name='"+name+ "' and startDate='"+startDate.toString("yyyy/MM/dd")+ "' and endDate='"+endDate.toString("yyyy/MM/dd")+"' and lineKey="+ QString("%1").arg(lineKey);
    bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
        QSqlError err = query2.lastError();
        qDebug()<< "Error#: " +QString("%1").arg(err.number());
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        if(!Retry(&targetDb, &query2, CommandText))
            exit(EXIT_FAILURE);

    }
    while(query2.next())
    {
        bFound = true;
        route2 = query2.value(0).toInt();
        name2=query2.value(1).toString();
        startDate2 = query2.value(2).toDateTime();
        endDate2 = query2.value(3).toDateTime();
        lineKey2 = query2.value(4).toInt();
        companyKey2 = query2.value(5).toInt();
        tractionType2= query2.value(6).toInt();
        direction2 = query2.value(7).toString();
        next2 = query2.value(8).toInt();
        prev2 = query2.value(9).toInt();
        normalEnter2 = query2.value(10).toInt();
        normalLeave2 = query2.value(11).toInt();
        reverseEnter2 = query2.value(12).toInt();
        reverseLeave2 = query2.value(13).toInt();
        lastUpdate2 = query.value(14).toDateTime();
    }
   }
   if( bFound && route== route2 && name == name2 && startDate.date() == startDate2.date() && endDate.date() == endDate2.date() && lineKey == lineKey2 && companyKey == companyKey2 && tractionType == tractionType2 && direction == direction2 && next == next2 && prev==prev2 && normalEnter == normalEnter2 && normalLeave == normalLeave2 && reverseEnter == reverseEnter2 && reverseLeave == reverseLeave2 && lastUpdate == lastUpdate2)
   {
       notUpdated++;
       sendProgress();
       continue;
   }
   //if(route == route2 && name == name2 && startDate.date().toString("yyyy/MM/dd") == startDate2.date() .toString("yyyy/MM/dd")&& endDate.date().toString("yyyy/MM/dd") == endDate2.date().toString("yyyy/MM/dd") && lineKey == lineKey2)
   if(bFound)
   {
    if(tgtConn->servertype() != "MsSql")
        CommandText = "update Routes set companyKey=" + QString("%1").arg(companyKey) + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev) + ",normalEnter=" + QString("%1").arg(normalEnter) +",normalLeave=" + QString("%1").arg(normalLeave) +",reverseEnter="+ QString("%1").arg(reverseEnter ) + ",reverseLeave=" + QString("%1").arg(reverseLeave) + ",lastUpdate =:lastUpdate where route=" + QString("%1").arg(route) +" and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "' and endDate='" + endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(lineKey);
    else
        CommandText = "update Routes set companyKey=" + QString("%1").arg(companyKey) + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev) + ",normalEnter=" + QString("%1").arg(normalEnter) +",normalLeave=" + QString("%1").arg(normalLeave) +",reverseEnter="+ QString("%1").arg(reverseEnter ) + ",reverseLeave=" + QString("%1").arg(reverseLeave) + ",lastUpdate=:lastUpdate where route=" + QString("%1").arg(route) +" and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "' and endDate='" + endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(lineKey);
    query2.prepare(CommandText);
    query2.bindValue(":lastUpdate", lastUpdate);
    bQuery = query2.exec();
    if(!bQuery)
    {
        QSqlError err = query2.lastError();
        qDebug()<< "Error#: " +QString("%1").arg(err.number());
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";

        //db.close();
        //exit(EXIT_FAILURE);
        if(!Retry(&targetDb, &query2, ""))
            errors++;
    }
    else
        updated++;
   }
   else
   {
    if(tgtConn->servertype() != "MsSql")
        CommandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate) values("+QString("%1").arg(route) + ",'"+name+ "','"+startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)+","+QString("%1").arg(tractionType)+",'"+direction+"',"+QString("%1").arg(next)+","+QString("%1").arg(prev)+","+QString("%1").arg(normalEnter)+","+QString("%1").arg(normalLeave)+","+QString("%1").arg(reverseEnter)+","+QString("%1").arg(reverseLeave)+ ",:lastUpdate)";
    else
        CommandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate) values("+QString("%1").arg(route) + ",'"+name+ "','"+startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)+","+QString("%1").arg(tractionType)+",'"+direction+"',"+QString("%1").arg(next)+","+QString("%1").arg(prev)+","+QString("%1").arg(normalEnter)+","+QString("%1").arg(normalLeave)+","+QString("%1").arg(reverseEnter)+","+QString("%1").arg(reverseLeave)+ ",:lastUpdate)";
    query2.prepare(CommandText);
    query2.bindValue(":lastUpdate", lastUpdate);
    bQuery = query2.exec();
    if(!bQuery)
    {
        QSqlError err = query2.lastError();
        qDebug()<< "Error#: " +QString("%1").arg(err.number());
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() <<QString("%1").arg(route)+" " +name+" " +startDate.toString("yyyy/MM/dd")+" " +endDate.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey);
        qDebug()<<QString("%1").arg(route2)+" " +name2+" " +startDate2.toString("yyyy/MM/dd")+" " +endDate2.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey2);

        //db.close();
        //exit(EXIT_FAILURE);
        if(!Retry(&targetDb, &query2, ""))
            errors++;
    }
    else
        added++;
   }
   sendProgress();
  }

  if(!bNoDeletes && !bDropTables)
  {
      // Check for deleted entries
      emit(progressMsg("Processing Route deletes"));
      QList<RouteData> tgtRouteList;
      CommandText = "select route, name, startDate, endDate, lineKey from Routes where lastUpdate <'"+beginTime+"'";
      QSqlQuery query2 = QSqlQuery(targetDb);
      bQuery = query2.exec(CommandText);
      if(!bQuery)
      {
          QSqlError err = query2.lastError();
          qDebug() << err.text() + "\n";
          qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
          //db.close();
          exit(EXIT_FAILURE);
      }
      while(query2.next())
      {
          RouteData rd;
          rd.route = query2.value(0).toInt();
          rd.name=query2.value(1).toString();
          rd.startDate = query2.value(2).toDate();
          rd.endDate = query2.value(3).toDate();
          rd.lineKey = query2.value(4).toInt();

          tgtRouteList.append(rd);
          qApp->processEvents(QEventLoop::AllEvents);

      }
      rowCount = tgtRouteList.count();
      rowsCompleted = 0;
      for(int i=0; i < tgtRouteList.count(); i++)
      {
          RouteData rd = tgtRouteList.at(i);
          CommandText = "select count(*) from Routes  where route=" + QString("%1").arg(rd.route) +" and name='"+ rd.name + "' and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);

          QSqlQuery query = QSqlQuery(srcDb);
          bool bQuery = query.exec(CommandText);
          if(!bQuery)
          {
              QSqlError err = query.lastError();
              qDebug() << err.text() + "\n";
              qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
              //db.close();
              exit(EXIT_FAILURE);
          }
          int srcCount=0;
          while(query.next())
          {
              srcCount = query.value(0).toInt();
          }
          if(srcCount > 0)
          {
              sendProgress();
              continue;   // record exists, continue
          }

          CommandText = "delete from Routes where route=" + QString("%1").arg(rd.route) +" and name='"+ rd.name + "' and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);
          //CommandText = "delete from Routes where route=" + QString("%1").arg(rd.route) +" and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);

          QSqlQuery query2 = QSqlQuery(targetDb);
          bQuery = query2.exec(CommandText);
          if(!bQuery)
          {
              QSqlError err = query2.lastError();
              qDebug() << err.text() + "\n";
              qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
              //db.close();
              exit(EXIT_FAILURE);
          }
          if(query2.numRowsAffected()>0)
              deleted += query2.numRowsAffected();
          else
              qDebug()<< "fail? " + CommandText;

          sendProgress();
      }
  }

  qDebug()<<"Routes: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
  emit uncheck("chkRoutes");

  return true;
 }

 bool ExportSql::exportRoute(RouteData rd)
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
  QString CommandText;

  sql->BeginTransaction("Route");

  CommandText = "delete from Routes where route=" +  QString("%1").arg(rd.route) +" and name='"+ rd.name + "' and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") +"'";
  QSqlQuery query = QSqlQuery(targetDb);
  query.prepare(CommandText);
  bool bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   //db.close();
   if(!Retry(&srcDb, &query, ""))
       exit(EXIT_FAILURE);
  }
  int rowsDeleted = query.numRowsAffected();


  if(srcConn->servertype() == "MsSql")
   CommandText = "select route, LTRIM(RTRIM(name)), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where route = :route and startDate = :startDate and endDate = :endDate";
  else
   CommandText = "select route, TRIM(name), startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where route = :route and startDate = :startDate and endDate = :endDate";

  query = QSqlQuery(srcDb);
  query.prepare(CommandText);
  query.bindValue(":route", QString::number(rd.route));
  query.bindValue(":startDate", rd.startDate.toString("yyyy/MM/dd"));
  query.bindValue(":endDate",rd.endDate.toString("yyyy/MM/dd"));
  bQuery = query.exec();
  if(!bQuery)
  {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      //db.close();
      if(!Retry(&srcDb, &query, ""))
          exit(EXIT_FAILURE);
  }

  qint32 route=0, route2=0, lineKey=0, lineKey2=0, companyKey, companyKey2, tractionType, tractionType2, next=0, next2=0, prev=0, prev2=0, normalEnter=0, normalEnter2=0, normalLeave=0, normalLeave2=0, reverseEnter=0, reverseEnter2=0, reverseLeave=0, reverseLeave2 =0;
  QString name="", name2="", direction="", direction2="";
  QDateTime lastUpdate, lastUpdate2;
  QDateTime startDate, startDate2, endDate, endDate2;
  while(query.next())
  {
      route = query.value(0).toInt();
      name=query.value(1).toString();
      startDate = query.value(2).toDateTime();
      endDate = query.value(3).toDateTime();
      lineKey = query.value(4).toInt();
      companyKey = query.value(5).toInt();
      tractionType= query.value(6).toInt();
      direction = query.value(7).toString();
      next = query.value(8).toInt();
      prev = query.value(9).toInt();
      normalEnter = query.value(10).toInt();
      normalLeave = query.value(11).toInt();
      reverseEnter = query.value(12).toInt();
      reverseLeave = query.value(13).toInt();
      lastUpdate = query.value(14).toDateTime();

      route2 = -1;
      name2 = "??";
      lineKey2 = -1;
      CommandText = "select route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate from Routes where route = "+QString("%1").arg(route)+" and name='"+name+ "' and startDate='"+startDate.toString("yyyy/MM/dd")+ "' and endDate='"+endDate.toString("yyyy/MM/dd")+"' and lineKey="+ QString("%1").arg(lineKey);
      QSqlQuery query2 = QSqlQuery(targetDb);
      bQuery = query2.exec(CommandText);
      if(!bQuery)
      {
          QSqlError err = query2.lastError();
          qDebug()<< "Error#: " +QString("%1").arg(err.number());
          qDebug() << err.text() + "\n";
          qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
          //db.close();
          if(!Retry(&targetDb, &query2, CommandText))
              exit(EXIT_FAILURE);

      }
      bool bFound = false;
      while(query2.next())
      {
          bFound = true;
          route2 = query2.value(0).toInt();
          name2=query2.value(1).toString();
          startDate2 = query2.value(2).toDateTime();
          endDate2 = query2.value(3).toDateTime();
          lineKey2 = query2.value(4).toInt();
          companyKey2 = query2.value(5).toInt();
          tractionType2= query2.value(6).toInt();
          direction2 = query2.value(7).toString();
          next2 = query2.value(8).toInt();
          prev2 = query2.value(9).toInt();
          normalEnter2 = query2.value(10).toInt();
          normalLeave2 = query2.value(11).toInt();
          reverseEnter2 = query2.value(12).toInt();
          reverseLeave2 = query2.value(13).toInt();
          lastUpdate2 = query.value(14).toDateTime();
      }
      if( bFound && route== route2 && name == name2 && startDate.date() == startDate2.date() && endDate.date() == endDate2.date() && lineKey == lineKey2 && companyKey == companyKey2 && tractionType == tractionType2 && direction == direction2 && next == next2 && prev==prev2 && normalEnter == normalEnter2 && normalLeave == normalLeave2 && reverseEnter == reverseEnter2 && reverseLeave == reverseLeave2 && lastUpdate == lastUpdate2)
      {
          notUpdated++;
          sendProgress();
          continue;
      }
      //if(route == route2 && name == name2 && startDate.date().toString("yyyy/MM/dd") == startDate2.date() .toString("yyyy/MM/dd")&& endDate.date().toString("yyyy/MM/dd") == endDate2.date().toString("yyyy/MM/dd") && lineKey == lineKey2)
      if(bFound)
      {
          if(tgtConn->servertype() != "MsSql")
              CommandText = "update Routes set companyKey=" + QString("%1").arg(companyKey) + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev) + ",normalEnter=" + QString("%1").arg(normalEnter) +",normalLeave=" + QString("%1").arg(normalLeave) +",reverseEnter="+ QString("%1").arg(reverseEnter ) + ",reverseLeave=" + QString("%1").arg(reverseLeave) + ",lastUpdate =:lastUpdate where route=" + QString("%1").arg(route) +" and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "' and endDate='" + endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(lineKey);
          else
              CommandText = "update Routes set companyKey=" + QString("%1").arg(companyKey) + ",tractionType=" + QString("%1").arg(tractionType) + ", direction='"+direction + "',next=" + QString("%1").arg(next) + ",prev=" + QString("%1").arg(prev) + ",normalEnter=" + QString("%1").arg(normalEnter) +",normalLeave=" + QString("%1").arg(normalLeave) +",reverseEnter="+ QString("%1").arg(reverseEnter ) + ",reverseLeave=" + QString("%1").arg(reverseLeave) + ",lastUpdate=:lastUpdate where route=" + QString("%1").arg(route) +" and name='"+ name + "' and startDate='" + startDate.toString("yyyy/MM/dd") + "' and endDate='" + endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(lineKey);
          query2.prepare(CommandText);
          query2.bindValue(":lastUpdate", lastUpdate);
          bQuery = query2.exec();
          if(!bQuery)
          {
              QSqlError err = query2.lastError();
              qDebug()<< "Error#: " +QString("%1").arg(err.number());
              qDebug() << err.text() + "\n";
              qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";

              //db.close();
              //exit(EXIT_FAILURE);
              if(!Retry(&targetDb, &query2, ""))
                  errors++;
          }
          else
              updated++;
      }
      else
      {
          if(tgtConn->servertype() != "MsSql")
              CommandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate) values("+QString("%1").arg(route) + ",'"+name+ "','"+startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)+","+QString("%1").arg(tractionType)+",'"+direction+"',"+QString("%1").arg(next)+","+QString("%1").arg(prev)+","+QString("%1").arg(normalEnter)+","+QString("%1").arg(normalLeave)+","+QString("%1").arg(reverseEnter)+","+QString("%1").arg(reverseLeave)+ ",:lastUpdate)";
          else
              CommandText = "insert into Routes (route, name, startDate, endDate, lineKey, companyKey,tractionType, Direction, next, prev, normalEnter, normalLeave, reverseEnter, ReverseLeave, lastUpdate) values("+QString("%1").arg(route) + ",'"+name+ "','"+startDate.toString("yyyy/MM/dd")+ "','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(lineKey)+","+QString("%1").arg(companyKey)+","+QString("%1").arg(tractionType)+",'"+direction+"',"+QString("%1").arg(next)+","+QString("%1").arg(prev)+","+QString("%1").arg(normalEnter)+","+QString("%1").arg(normalLeave)+","+QString("%1").arg(reverseEnter)+","+QString("%1").arg(reverseLeave)+ ",:lastUpdate)";
          query2.prepare(CommandText);
          query2.bindValue(":lastUpdate", lastUpdate);
          bQuery = query2.exec();
          if(!bQuery)
          {
              QSqlError err = query2.lastError();
              qDebug()<< "Error#: " +QString("%1").arg(err.number());
              qDebug() << err.text() + "\n";
              qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
              qDebug() <<QString("%1").arg(route)+" " +name+" " +startDate.toString("yyyy/MM/dd")+" " +endDate.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey);
              qDebug()<<QString("%1").arg(route2)+" " +name2+" " +startDate2.toString("yyyy/MM/dd")+" " +endDate2.toString("yyyy/MM/dd")+" " +QString("%1").arg(lineKey2);

              //db.close();
              //exit(EXIT_FAILURE);
              if(!Retry(&targetDb, &query2, ""))
                  errors++;
          }
          else
              added++;
      }
      sendProgress();

  }
 #if 0
  if(!bNoDeletes)
  {
      // Check for deleted entries
      emit(progressMsg("Processing Route deletes"));
      QList<routeData> tgtRouteList;
      CommandText = "select route, name, startDate, endDate, lineKey from Routes where lastUpdate <'"+beginTime+"'";
qlQuery query2 = QSqlQuery(targetDb);
     bQuery = query2.exec(CommandText);
     if(!bQuery)
     {
         QSqlError err = query2.lastError();
         qDebug() << err.text() + "\n";
         qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
         //db.close();
         exit(EXIT_FAILURE);
     }
     while(query2.next())
     {
         routeData rd;
         rd.route = query2.value(0).toInt();
         rd.name=query2.value(1).toString();
         rd.startDate = query2.value(2).toDateTime();
         rd.endDate = query2.value(3).toDateTime();
         rd.lineKey = query2.value(4).toInt();

         tgtRouteList.append(rd);
         qApp->processEvents(QEventLoop::AllEvents);

     }
     rowCount = tgtRouteList.count();
     rowsCompleted = 0;
     for(int i=0; i < tgtRouteList.count(); i++)
     {
         routeData rd = tgtRouteList.at(i);
         CommandText = "select count(*) from Routes  where route=" + QString("%1").arg(rd.route) +" and name='"+ rd.name + "' and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);

         QSqlQuery query = QSqlQuery(srcDb);
         bool bQuery = query.exec(CommandText);
         if(!bQuery)
         {
             QSqlError err = query.lastError();
             qDebug() << err.text() + "\n";
             qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
             //db.close();
             exit(EXIT_FAILURE);
         }
         int srcCount=0;
         while(query.next())
         {
             srcCount = query.value(0).toInt();
         }
         if(srcCount > 0)
         {
             sendProgress();
             continue;   // record exists, continue
         }

         CommandText = "delete from Routes where route=" + QString("%1").arg(rd.route) +" and name='"+ rd.name + "' and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);
         //CommandText = "delete from Routes where route=" + QString("%1").arg(rd.route) +" and startDate='" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate='" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey=" + QString("%1").arg(rd.lineKey);

         QSqlQuery query2 = QSqlQuery(targetDb);
         bQuery = query2.exec(CommandText);
         if(!bQuery)
         {
             QSqlError err = query2.lastError();
             qDebug() << err.text() + "\n";
             qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
             //db.close();
             exit(EXIT_FAILURE);
         }
         if(query2.numRowsAffected()>0)
             deleted += query2.numRowsAffected();
         else
             qDebug()<< "fail? " + CommandText;

         sendProgress();
     }
 }
#endif
 sql->CommitTransaction("Routes");
 qDebug()<<"Routes: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
 emit uncheck("chkRoutes");

 return true;

}

bool ExportSql::exportStations()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("Stations");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;

    getCount("Stations", bDropTables);
    if(bDropTables)
    {
     if(!createStationsTable(targetDb, tgtDbType))
      return false;
    }

//    if(tgtConn->servertype() != "MySql")
//        setIdentityInsert("Stations", true);
    if(bDropTables)
     CommandText = "select stationKey, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, SegmentId, point, lastUpdate from Stations ";
    else
     CommandText = "select stationKey, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, SegmentId, point, lastUpdate from Stations where lastUpdate >:lastUpdated";
    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bool bQuery = query.exec();
    if(!bQuery)
    {
     SQLERROR(query);
     //db.close();
     exit(EXIT_FAILURE);
    }
    qint32 stationKey=0, stationKey2=0, infoKey=0, infoKey2=0, geodb_loc_id=0, geodb_loc_id2=0, routeType=0, routeType2=0, point=0, point2 =0, segmentId=0, segmentId2 = 0;
    QString name="", name2="", suffix = "", suffix2 = "";
    QDateTime lastUpdate;
    double latitude=0, latitude2=0, longitude=0, longitude2=0;
    QDateTime startDate, startDate2, endDate, endDate2;
    while(query.next())
    {
        stationKey = query.value(0).toInt();
        name=query.value(1).toString();
        suffix=query.value(2).toString();
        latitude = query.value(3).toDouble();
        longitude = query.value(4).toDouble();
        startDate = query.value(5).toDateTime();
        endDate = query.value(6).toDateTime();
        if(query.value(7).isNull())
            infoKey = -1;
        else
            query.value(7).toInt();
        geodb_loc_id = query.value(8).toInt();
        routeType = query.value(9).toInt();
        segmentId = query.value(10).toInt();
        point = query.value(11).toInt();
        lastUpdate = query.value(12).toDateTime();
        if(lastUpdate.isValid() || lastUpdate.isNull())
            lastUpdate = QDateTime::currentDateTimeUtc();
        if(!bDropTables)
        {
         CommandText = "select stationKey, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, SegmentId, point, lastUpdate from Stations ";
         bQuery = query2.exec(CommandText);
         if(!bQuery)
         {
             QSqlError err = query2.lastError();
             qDebug() << err.text() + "\n";
             qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
             //db.close();
             exit(EXIT_FAILURE);
         }
         while(query2.next())
         {
             stationKey2 = query2.value(0).toInt();
             name2=query2.value(1).toString();
             suffix2 = query2.value(2).toString();
             latitude2 = query2.value(3).toDouble();
             longitude2 = query2.value(4).toDouble();
             startDate2 = query2.value(5).toDateTime();
             endDate2 = query2.value(6).toDateTime();
             if(query2.value(7).isNull())
                 infoKey2 = -1;
             else
                 infoKey2 = query2.value(7).toInt();
             geodb_loc_id2 = query2.value(8).toInt();
             routeType2 = query2.value(9).toInt();
             segmentId2 = query.value(10).toInt();
             point2 = query.value(11).toInt();
            bFound = true;
         }
        }
        if(bFound && stationKey== stationKey2 && name == name2 && latitude == latitude2 && longitude == longitude2 && startDate.date() == startDate2.date() && endDate.date() == endDate2.date() && infoKey == infoKey2 && geodb_loc_id == geodb_loc_id2 && routeType == routeType2 && suffix == suffix2 && segmentId == segmentId2 && point == point2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(stationKey== stationKey2)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update Stations set name='"+name+"',latitude="+QString("%1").arg(latitude, 0,'f',8)+",longitude="+QString("%1").arg(longitude,0,'f',8)+",startDate='"+startDate.toString("yyyy/MM/dd")+"',endDate='"+endDate.toString("yyyy/MM/dd")+"',infoKey="+QString("%1").arg(infoKey)+",geodb_loc_id="+QString("%1").arg(geodb_loc_id)+",lastUpdate=:lastUpdate where stationKey = "+QString("%1").arg(stationKey);
            else
                CommandText = "update Stations set name='"+name+"',latitude="+QString("%1").arg(latitude, 0,'f',8)+",longitude="+QString("%1").arg(longitude,0,'f',8)+",startDate='"+startDate.toString("yyyy/MM/dd")+"',endDate='"+endDate.toString("yyyy/MM/dd")+"',infoKey="+QString("%1").arg(infoKey)+",geodb_loc_id="+QString("%1").arg(geodb_loc_id)+ ",lastUpdate=:lastUpdate where stationKey = "+QString("%1").arg(stationKey);
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate",lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
             CommandText = "insert into Stations (stationKey, name, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, suffix, segmentId, point, lastUpdate) values("+QString("%1").arg(stationKey) + ",'"+name+"',"+QString("%1").arg(latitude, 0,'f',8)+","+QString("%1").arg(longitude,0,'f',8)+",'"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(infoKey)+","+QString("%1").arg(geodb_loc_id) +","+QString("%1").arg(routeType)+ ",'" + suffix + "'," +QString::number(segmentId) + "," +QString::number(point) + ",:lastUpdate)";
            else
                CommandText = "SET IDENTITY_INSERT [dbo].[Stations] ON; insert into [dbo].[Stations] (stationKey, name, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, suffix, segmentId, point, lastUpdate) values("+QString("%1").arg(stationKey) + ",'"+name+"',"+QString("%1").arg(latitude, 0,'f',8)+","+QString("%1").arg(longitude,0,'f',8)+",'"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd")+"',"+QString("%1").arg(infoKey)+","+QString("%1").arg(geodb_loc_id) +","+ QString("%1").arg(routeType)+ ",'" + suffix + "'," +QString::number(segmentId) + "," +QString::number(point) + ",:lastUpdate); SET IDENTITY_INSERT [dbo].[Stations] OFF";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();

    }

//    if(tgtConn->servertype() != "MySql")
//        setIdentityInsert("Stations", false);

    qDebug()<<"Stations: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkStations");

    return true;
}

bool ExportSql::exportTerminals()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("Terminals");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QString CommandText;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;

    getCount("Terminals", bDropTables);
    if(bDropTables)
    {
     if(!createTerminalsTable(targetDb, tgtDbType))
      return false;
    }
    if(bDropTables)
     CommandText = "select route, name, startDate, endDate, startSegment, startWhichEnd, endSegment,  endWhichEnd, lastUpdate from Terminals order by lastUpdate";
    else
     CommandText = "select route, name, startDate, endDate, startSegment, startWhichEnd, endSegment,  endWhichEnd, lastUpdate from Terminals where lastUpdate > :lastUpdated  order by lastUpdate";

    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bool bQuery = query.exec();
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 route=0, route2=0, startSegment=0, startSegment2=0, endSegment=0, endSegment2=0;
    QString name="", name2="", startWhichEnd="", startWhichEnd2="", endWhichEnd="", endWhichEnd2="";
    QDateTime lastUpdate;
    QDateTime startDate, startDate2, endDate, endDate2;
    while(query.next())
    {
        route = query.value(0).toInt();
        name=query.value(1).toString();
        startDate = query.value(2).toDateTime();
        endDate = query.value(3).toDateTime();
        startSegment = query.value(4).toInt();
        startWhichEnd = query.value(5).toString();
        endSegment = query.value(6).toInt();
        endWhichEnd = query.value(7).toString();
        lastUpdate =query.value(8).toDateTime();

        if(!bDropTables)
        {
        CommandText = "select route, name, startDate, endDate, startSegment, startWhichEnd, endSegment,  endWhichEnd from Terminals where route="+QString("%1").arg(route)+" and name='"+name+"' and startDate='"+startDate.toString("yyyy/MM/dd")+"' and endDate='"+endDate.toString("yyyy/MM/dd")+"'";
        QSqlQuery query2 = QSqlQuery(targetDb);
        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            SQLERROR(query2);
            //db.close();
            exit(EXIT_FAILURE);
        }
        while(query2.next())
        {
            route2 = query2.value(0).toInt();
            name2=query2.value(1).toString();
            startDate2 = query2.value(2).toDateTime();
            endDate2 = query2.value(3).toDateTime();
            startSegment2 = query2.value(4).toInt();
            startWhichEnd2 = query2.value(5).toString();
            endSegment2 = query2.value(6).toInt();
            endWhichEnd2 = query2.value(7).toString();
            bFound = true;
        }
        }
        if(route== route2 && name == name2 && startDate.date() == startDate2.date() && endDate.date()== endDate2.date() && startSegment == startSegment2 && endSegment == endSegment2 && startWhichEnd == startWhichEnd2 && endWhichEnd == endWhichEnd2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        if(route== route2 && name == name2 && startDate.date() == startDate2.date() && endDate.date()== endDate2.date())
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update Terminals set startSegment=" + QString("%1").arg(startSegment)+",startWhichEnd='"+startWhichEnd+"',endSegment="+QString("%1").arg(endSegment)+",endWhichEnd='"+endWhichEnd+ "',lastUpdate=:lastUpdate  where route="+QString("%1").arg(route)+" and name='"+name+"' and startDate='"+startDate.toString("yyyy/MM/dd")+"' and endDate='"+endDate.toString("yyyy/MM/dd")+"'";
            else
                CommandText = "update Terminals set startSegment=" + QString("%1").arg(startSegment)+",startWhichEnd='"+startWhichEnd+"',endSegment="+QString("%1").arg(endSegment)+",endWhichEnd='"+endWhichEnd+ "',lastUpdate=:lastUpdate where route="+QString("%1").arg(route)+" and name='"+name+"' and startDate='"+startDate.toString("yyyy/MM/dd")+"' and endDate='"+endDate.toString("yyyy/MM/dd")+"'";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                SQLERROR(query2);
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into Terminals (route, name, startDate, endDate, startSegment, startWhichEnd, endSegment,  endWhichEnd,lastUpdate) values("+QString("%1").arg(route)  +",'"+name+"','"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd") + "'," + QString("%1").arg(startSegment)+",'"+startWhichEnd+"',"+QString("%1").arg(endSegment)+",'"+endWhichEnd+ "',:lastUpdate)";
            else
                CommandText = "insert into Terminals (route, name, startDate, endDate, startSegment, startWhichEnd, endSegment,  endWhichEnd,lastUpdate) values("+QString("%1").arg(route)  +",'"+name+"','"+startDate.toString("yyyy/MM/dd")+"','"+endDate.toString("yyyy/MM/dd") + "'," + QString("%1").arg(startSegment)+",'"+startWhichEnd+"',"+QString("%1").arg(endSegment)+",'"+endWhichEnd+ "',:lastUpdate)";
            query2.prepare(CommandText);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
               SQLERROR(query2);
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();
    }

    qDebug()<<"Terminals: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkTerminals");

    return true;
}

void ExportSql::getCount(QString table, bool bDropTable)
{
 emit(progressMsg("Processing "+ table + " inserts/updates"));
 lastUpdated = QDateTime::fromString("1800/01/01 00:00:01","yyyy/MM/dd hh:mm:ss");
 QString CommandText;
 if(tgtConn->servertype() == "MsSql")
  CommandText ="select max(lastUpdate) from [dbo].[" + table + "]";
 else
  CommandText ="select max(lastUpdate) from " + table;
 QSqlQuery query2 = QSqlQuery(targetDb);
 bool bQuery = query2.exec(CommandText);
 if(!bQuery)
 {
  SQLERROR(query2);
  //db.close();
  //if(!Retry(&targetDb, &query2, CommandText))
   //exit(EXIT_FAILURE);
   //return;
 }
 while(query2.next())
 {
  lastUpdated = query2.value(0).toDateTime();
  strLastUpdated = "'" + lastUpdated.toString("yyyy/MM/dd hh:mm:ss") +"'";
  //strLastUpdated = "'" + query2.value(0).toString() +"'";
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

 // get count of rows in source table that have been updated.
 if(bDropTable)
  CommandText = "select count(*) from " + table;
 else
  CommandText = "select count(*) from " + table + " where lastUpdate > :lastUpdated";
 QSqlQuery query = QSqlQuery(srcDb);
 query.prepare(CommandText);
 if(!bDropTable)
  query.bindValue(":lastUpdated", lastUpdated);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  //db.close();
  if(!Retry(&srcDb, &query, ""))
   exit(EXIT_FAILURE);
 }
 while(query.next())
 {
  rowCount = query.value(0).toInt();
  qDebug()<< QString("Row count = %1 lastUpdate = '%2'").arg(rowCount).arg(lastUpdated.toString("yyyy/MM/dd hh:mm:ss"));
 }
 rowsCompleted = 0;
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
//    QString CommandText ="SET IDENTITY_INSERT [dbo].[" + table + "] " +(bOn?"ON":"OFF");
//    QSqlQuery query2 = QSqlQuery(targetDb);
//    bool bQuery = query2.exec(CommandText);
//    if(!bQuery)
//    {
//        QSqlError err = query2.lastError();
//        qDebug() << err.text() + "\n";
//        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//        //db.close();
//        exit(EXIT_FAILURE);
//    }
//    else
//    {
//        //qDebug() << query2.lastError();
//        qDebug() << CommandText;
//    }
//}
bool ExportSql::exportRouteComments()
{
    srcDb = QSqlDatabase::database();
        if(!openDb()) return false;

    updateTimestamp("RouteComments");

    added=0;
    updated=0;
    deleted=0;
    errors=0;
    notUpdated=0;
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bFound = false;
    QString CommandText;

    getCount("RouteComments", bDropTables);
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Comments",true);
    if(bDropTables)
    {
     if(!createRouteCommentsTable(targetDb, tgtDbType ))
      return false;
    }

    if(bDropTables)
     CommandText = "select route, date, commentKey, companyKey, lastUpdate from RouteComments order by lastUpdate";
    else
     CommandText = "select route, date, commentKey, companyKey, lastUpdate from RouteComments where lastUpdate > :lastUpdated  order by lastUpdate";

    QSqlQuery query = QSqlQuery(srcDb);
    query.prepare(CommandText);
    if(!bDropTables)
     query.bindValue(":lastUpdated", lastUpdated);
    bool bQuery = query.exec();
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug()<< "Error#: " +QString("%1").arg(err.number());
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        exit(EXIT_FAILURE);
    }
    qint32 route=0, route2=0, commentKey=0, commentKey2=0, companyKey=0, companyKey2=0;
    QDateTime lastUpdate, lastUpdate2;
    QDate date, date2;
    while(query.next())
    {
        route = query.value(0).toInt();
        date=query.value(1).toDate();
        commentKey = query.value(2).toInt();
        companyKey = query.value(3).toInt();
        lastUpdate =query.value(4).toDateTime();

        if(!bDropTables)
        {
         CommandText = "select route, date, commentKey, companyKey, lastUpdate from RouteComments where route="+QString("%1").arg(route) + " and date ='"+date.toString("yyyy/MM/dd")+"'";
         bQuery = query2.exec(CommandText);
         if(!bQuery)
         {
             QSqlError err = query2.lastError();
             qDebug()<< "Error#: " +QString("%1").arg(err.number());
             qDebug() << err.text() + "\n";
             qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
             //db.close();
             exit(EXIT_FAILURE);
         }
         while(query2.next())
         {
             route2 = query2.value(0).toInt();
             date2=query2.value(1).toDate();
             commentKey2 = query2.value(2).toInt();
             companyKey2 = query2.value(3).toInt();
             lastUpdate2 = query.value(4).toDateTime();
             bFound = true;
         }
        }
        if(bFound && route== route2 && date == date2 && commentKey == commentKey2 && companyKey == companyKey2 && lastUpdate == lastUpdate2)
        {
            notUpdated++;
            sendProgress();
            continue;
        }
        //if(route== route2 && date == date2)
        if(bFound)
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "update  RouteComments set commentKey=:commentKey, companyKey=:companyKey,lastUpdate=:lastUpdate where route="+QString("%1").arg(route) + " and date ='"+date.toString("yyyy/MM/dd")+"'";
            else
                CommandText = "update  RouteComments set commentKey=:comments, companyKey=:companyKey,lastUpdate=:lastUpdate where route="+QString("%1").arg(route) + " and date ='"+date.toString("yyyy/MM/dd")+"'";
            query2.prepare(CommandText);
            query2.bindValue(":commentKey", commentKey);
            query2.bindValue(":companyKey", companyKey);
            query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug()<< "Error#: " +QString("%1").arg(err.number());
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                updated++;
        }
        else
        {
            if(tgtConn->servertype() != "MsSql")
                CommandText = "insert into RouteComments (route, date, commentKey, companyKey,lastUpdate ) values (:route, :date, :commentKey, :companyKey, :lastUpdate)";
            else
                CommandText = "insert into RouteComments (route, date, commentKey, companyKey, lastUpdate ) values (:route, :date, :commentKey, :companyKey, :lastUpdate)";

            query2.prepare(CommandText);
            query2.bindValue(":route", route);
            query2.bindValue(":date", date.toString("yyyy/MM/dd"));
            query2.bindValue(":commentKey", commentKey);
            query2.bindValue(":companyKey", companyKey);
            //if(tgtConn->servertype() == "MsSql")
                query2.bindValue(":lastUpdate", lastUpdate);
            bQuery = query2.exec();
            if(!bQuery)
            {
                QSqlError err = query2.lastError();
                qDebug()<< "Error#: " +QString("%1").arg(err.number());
                qDebug() << err.text() + "\n";
                qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                //db.close();
                //exit(EXIT_FAILURE);
                errors++;
            }
            else
                added++;
        }
        sendProgress();
    }
//    if(tgtConn->servertype() == "MsSql")
//        setIdentityInsert("Comments",false);

    qDebug()<<"RouteComments: "+ QString("%1").arg(added)+ " added, "+ QString("%1").arg(updated) + " updated, "+ QString("%1").arg(deleted)+ " deleted, "+ QString("%1").arg(notUpdated)+ " not updated, "+QString("%1").arg(errors)+ " errors\n";
    emit uncheck("chkRouteComments");

    return true;
}

void ExportSql::updateTimestamp(QString table)
{
    if(srcConn->servertype() != "MySql")
        return;
    srcDb = QSqlDatabase::database();

    QString CommandText = "update " + table + " set lastupdate = '2000-01-01 00:00:00' where lastUpdate = '0000-00-00 00:00:00'";
    QSqlQuery query = QSqlQuery(srcDb);
    bool bQuery = query.exec(CommandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        //exit(EXIT_FAILURE);
    }

}
bool ExportSql::Retry(QSqlDatabase *db, QSqlQuery *query, QString CommandText)
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
        if(CommandText.length() == 0)
            bQuery = query->exec();
        else
            bQuery = query->exec(CommandText);
        if(bQuery)
            return true;
        err = query->lastError();
        qDebug() << err.text() + "\n";

    }
    qDebug()<< "no more retries.";
    return false;
}
bool ExportSql::export_geodb_geometry()
{
    if(srcConn->servertype() != "MySql")
        return false;
    srcDb = QSqlDatabase::database();

    QString CommandText = "delete from geodb_berlin.dbo.geodb_geometry";
    QSqlQuery query2 = QSqlQuery(targetDb);
    bool bQuery = query2.exec(CommandText);
    if(!bQuery)
    {
        QSqlError err = query2.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        //exit(EXIT_FAILURE);
    }
    CommandText = "select loc_id, text_val, coord_type, AsText(coord_geometry), coord_subType, admin_level, belongs_to_01, belongs_to_02, valid_since, date_type_since, valid_until, date_type_until from geodb_berlin.geodb_geometry";
    QSqlQuery query = QSqlQuery(srcDb);
    bQuery = query.exec(CommandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug()<< "Error#: " +QString("%1").arg(err.number());
        qDebug() << err.text() + "\n";
        qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        //db.close();
        exit(EXIT_FAILURE);
    }
    int addCount=0;
    int loc_id;
    QString text_val;
    int coord_type;
    QString coord_geometry;
    int coord_subType;
    int admin_level;
    int belongs_to_01;
    int belongs_to_02;
    QDate valid_since;
    int date_type_since;
    QDate valid_until;
    int date_type_until;
    while(query.next())
    {
        loc_id = query.value(0).toInt();
        text_val = query.value(1).toString();
        coord_type = query.value(2).toInt();
        coord_geometry = query.value(3).toString();
        coord_subType = query.value(4).toInt();
        admin_level =query.value(5).toInt();
        belongs_to_01=query.value(6).toInt();
        belongs_to_02=query.value(7).toInt();
        valid_since = query.value(8).toDate();
        date_type_since = query.value(9).toInt();
        valid_until = query.value(10).toDate();
        date_type_until = query.value(11).toInt();


        CommandText = "insert into geodb_berlin.dbo.geodb_geometry (loc_id, text_val, coord_type, coord_geometry, coord_subType, admin_level, belongs_to_01, belongs_to_02, valid_since, date_type_since, valid_until, date_type_until) values ("+QString("%1").arg(loc_id)+", '"+ text_val+ "', "+QString("%1").arg(coord_type)+", geometry::STGeomFromText('"+coord_geometry+"', 3068), "+QString("%1").arg(coord_subType)+", "+QString("%1").arg(admin_level)+", "+QString("%1").arg(belongs_to_01)+", "+QString("%1").arg(belongs_to_02)+", '"+ valid_since.toString("yyyy/MM/dd")+"', "+QString("%1").arg(date_type_since)+", '"+valid_until.toString("yyyy/MM/dd")+"', "+QString("%1").arg(date_type_until)+")";
        query2 = QSqlQuery(targetDb);

        bQuery = query2.exec(CommandText);
        if(!bQuery)
        {
            QSqlError err = query2.lastError();
            qDebug()<< "Error#: " +QString("%1").arg(err.number());
            qDebug() << err.text() + "\n";
            qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            //db.close();
            exit(EXIT_FAILURE);
        }
        addCount++;
    }
    qDebug()<<"records added: "+ QString("%1").arg(addCount);
    return true;
}

bool ExportSql::createSegmentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Segments", db, dbType);

 if(dbType == "Sqlite")
 {
  CommandText = "CREATE TABLE `Segments` (  `SegmentId` integer NOT NULL primary key AUTOINCREMENT,  `Description` varchar(100) NOT NULL,  `OneWay` char(1) NOT NULL DEFAULT 'N', `Tracks` int(2) NOT NULL DEFAULT 2, street 'text', `Type` int(11) NOT NULL DEFAULT 0,  `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0,  `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0,  `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0,  `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0,  `Length` decimal(15,5) NOT NULL DEFAULT 0,  `points` int(11) NOT NULL default 0,  `StartDate` date NOT NULL DEFAULT '0000-00-00',  `endDate` date NOT NULL DEFAULT '0000-00-00',  `Direction` varchar(6) NOT NULL DEFAULT ' ', pointArray 'text',  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP)";
 }
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Segments` (`SegmentId` int(11) NOT NULL AUTO_INCREMENT,  `Description` varchar(100) NOT NULL,    `OneWay` char(1) NOT NULL,`Tracks` int(2) NOT NULL DEFAULT 2,`street` text, `Type` int NOT NULL Default 0, `StartLat` decimal(15,13) NOT NULL, `StartLon` decimal(15,13) NOT NULL, `EndLat` decimal(15,13) NOT NULL, `EndLon` decimal(15,13) NOT NULL,`Length` decimal(15,5) NOT NULL,  `points` int(11) NOT NULL default 0,  `StartDate` date NOT NULL,`endDate` date NOT NULL, `Direction` varchar(6) NOT NULL, `pointArray` text NOT NULL,`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,PRIMARY KEY (`SegmentId`)) ENGINE=InnoDB AUTO_INCREMENT=1116 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_PADDING ON;";
  CommandText.append("CREATE TABLE [dbo].[Segments]("\
    "[SegmentId] [int] IDENTITY(1,1) NOT NULL,"\
    "[Description] [varchar](100) NOT NULL,"\
    "[OneWay] [char](1) NOT NULL,"\
    "[Tracks] [int] NOT NULL,"\
    "[street] [text] NULL,"\
    "[Type] [int] NOT NULL,"\
    "[StartLat] [decimal](15, 13) NOT NULL,"\
    "[StartLon] [decimal](15, 13) NOT NULL,"\
    "[EndLat] [decimal](15, 13) NOT NULL,"\
    "[EndLon] [decimal](15, 13) NOT NULL,"\
    "[Length] [decimal](15, 5) NOT NULL,"\
    "[points] [int] NOT NULL,"\
    "[StartDate] [date] NOT NULL,"\
    "[endDate] [date] NOT NULL,"\
    "[Direction] [varchar](6) NOT NULL,"\
    "[pointArray] [text] NULL,"\
    "[lastUpdate] [datetime] NOT NULL,"\
 "CONSTRAINT [pk_Segments] PRIMARY KEY CLUSTERED"\
"("\
    "[SegmentId] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY] TEXTIMAGE_ON [PRIMARY];");
  CommandText.append("SET ANSI_PADDING OFF;");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('N') FOR [OneWay];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT (2) FOR [Tracks];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [Type];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('') FOR [Description];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ('') FOR [street];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [StartLat];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [StartLon];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [EndLat];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [EndLon];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_dbo_Segments_Length]  DEFAULT ((0)) FOR [Length];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  DEFAULT ((0)) FOR [points];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_StartDate]  DEFAULT ('1899-01-01') FOR [StartDate];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_endDate]  DEFAULT ('1899-01-01') FOR [endDate];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_Direction]  DEFAULT ('') FOR [Direction];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_pointArray]  DEFAULT ('') FOR [pointArray];");
  CommandText.append("ALTER TABLE [dbo].[Segments] ADD  CONSTRAINT [DF_Segments_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}

bool ExportSql::dropTable(QString table, QSqlDatabase db, QString dbType)
{
 qWarning() << "dropping table: " << table;
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 if(dbType == "Sqlite")
  CommandText = QString("drop table if exists '%1'").arg(table);
 else if(dbType == "MySql")
  CommandText = QString("drop table if exists `%1`").arg(table);
 else
  CommandText = QString("IF OBJECT_ID(N'dbo.%1', N'U') IS NOT NULL "\
                        "BEGIN;"\
                        "DROP TABLE dbo.%1;End;").arg(table);
 bQuery = query.exec(CommandText);
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}

bool ExportSql::dropRoutes()
{
 srcDb = QSqlDatabase::database();
 if(!openDb()) return false;

 return dropTable("Routes", targetDb, tgtConn->servertype());
}

bool ExportSql::createRouteTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Routes", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE `Routes` (  `Route` int(11) NOT NULL,  `Name` varchar(125) NOT NULL,  `StartDate` date NOT NULL,  `EndDate` date NOT NULL,  `LineKey` int(11) NOT NULL,  `CompanyKey` int(11) NOT NULL DEFAULT 0,  `tractionType` int(11) NOT NULL DEFAULT 0,  `Direction` varchar(6) NOT NULL DEFAULT ' ',  `next` int(11) NOT NULL DEFAULT -1,  `prev` int(11) NOT NULL DEFAULT -1,  `normalEnter` int(11) NOT NULL DEFAULT 0,  `normalLeave` int(11) NOT NULL DEFAULT 0,  `reverseEnter` int(11) NOT NULL DEFAULT 0,  `reverseLeave` int(11) NOT NULL DEFAULT 0,  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  constraint pk PRIMARY key (`Route`,`Name`,`StartDate`,`EndDate`,`LineKey`),  CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),  CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),  CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),  CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`))";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Routes` ("\
    "`Route` int(11) NOT NULL,"\
    "`Name` varchar(125) CHARACTER SET latin1 COLLATE latin1_german1_ci NOT NULL,"\
    "`StartDate` date NOT NULL,"\
    "`EndDate` date NOT NULL,"\
    "`LineKey` int(11) NOT NULL,"\
    "`CompanyKey` int(11) NOT NULL,"\
    "`tractionType` int(11) NOT NULL,"\
    "`Direction` varchar(6) NOT NULL,"\
    "`next` int(11) NOT NULL,"\
    "`prev` int(11) NOT NULL,"\
    "`normalEnter` int(11) NOT NULL,"\
    "`normalLeave` int(11) NOT NULL,"\
    "`reverseEnter` int(11) NOT NULL,"\
    "`reverseLeave` int(11) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`,`LineKey`),"\
    "KEY `LineKey` (`LineKey`),"\
    "KEY `companyKey` (`CompanyKey`),"\
    "KEY `tractionType` (`tractionType`),"\
    "CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),"\
    "CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),"\
    "CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),"\
    "CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`)"\
    ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_PADDING ON;";
  CommandText.append("CREATE TABLE [dbo].[Routes]("\
      "[Route] [int] NOT NULL,"\
      "[StartDate] [date] NOT NULL,"\
      "[EndDate] [date] NOT NULL,"\
      "[LineKey] [int] NOT NULL,"\
      "[CompanyKey] [int] NOT NULL,"\
      "[tractionType] [int] NOT NULL,"\
      "[Direction] [varchar](6) NOT NULL,"\
      "[next] [int] NOT NULL,"\
      "[prev] [int] NOT NULL,"\
      "[normalEnter] [int] NOT NULL,"\
      "[normalLeave] [int] NOT NULL,"\
      "[reverseEnter] [int] NOT NULL,"\
      "[reverseLeave] [int] NOT NULL,"\
      "[name] [varchar](125) NOT NULL,"\
      "[lastUpdate] [datetime] NOT NULL,"\
   "CONSTRAINT [PK_Routes] PRIMARY KEY CLUSTERED"\
  "("\
      "[Route] ASC,"\
      "[StartDate] ASC,"\
      "[EndDate] ASC,"\
      "[LineKey] ASC,"\
      "[name] ASC"\
     ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];");
    CommandText.append("SET ANSI_PADDING OFF;");
    CommandText.append("ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_AltRoute_route] FOREIGN KEY([Route])"\
    "REFERENCES [dbo].[AltRoute] ([route]);"\
    "ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_AltRoute_route];"\
    "ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Companies] FOREIGN KEY([CompanyKey])"\
    "REFERENCES [dbo].[Companies] ([key]);"\
    "ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Companies];"\
    "ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Routes_Segments] FOREIGN KEY([LineKey])"\
    "REFERENCES [dbo].[Segments] ([SegmentId]);"\
    "ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Routes_Segments];"\
    "ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Routes_TractionType] FOREIGN KEY([tractionType])"\
    "REFERENCES [dbo].[TractionTypes] ([tractionType]);"\
    "ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Routes_TractionType];"\
    "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_CompanyKey]  DEFAULT ((-1)) FOR [CompanyKey];"\
    "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [[DF_Routes_tractionType]]]  DEFAULT ((1)) FOR [tractionType];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [[DF_Routes_Direction]]]  DEFAULT ('') FOR [Direction];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_next]  DEFAULT ((-1)) FOR [next];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_prev]  DEFAULT ((-1)) FOR [prev];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_normalEnter]  DEFAULT ((0)) FOR [normalEnter];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_normalLeave]  DEFAULT ((0)) FOR [normalLeave];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_reverseEnter]  DEFAULT ((0)) FOR [reverseEnter];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_reverseLeave]  DEFAULT ((0)) FOR [reverseLeave];"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_newName]  DEFAULT ('') FOR [name]"\
   "ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");

 }

 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}

bool ExportSql::createRouteCommentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("RouteComments", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE `RouteComments` (  `route` int(6) NOT NULL,  `date` date NOT NULL,  `commentKey` int(11) NOT NULL,  `companyKey` int(11) NOT NULL,  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  constraint pk PRIMARY KEY (`route`,`date`))";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `RouteComments` ("\
    "`route` int(6) NOT NULL,"\
    "`date` date NOT NULL,"\
    "`commentKey` int(11) NOT NULL,"\
    "`companyKey` int(11) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`route`,`date`)"\
  ") ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_german1_ci";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;";
  CommandText.append("SET QUOTED_IDENTIFIER ON;");
  CommandText.append("CREATE TABLE [dbo].[RouteComments]("\
        "[route] [int] NOT NULL,"\
        "[date] [date] NOT NULL,"\
        "[commentKey] [int] NOT NULL,"\
        "[companyKey] [int] NOT NULL,"\
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_RouteComments] PRIMARY KEY CLUSTERED"\
    "("\
        "[route] ASC,"\
        "[date] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];");
  CommandText.append("ALTER TABLE [dbo].[RouteComments] ADD  CONSTRAINT [DF_RouteComments_companyKey]  DEFAULT ((0)) FOR [companyKey];");
  CommandText.append("ALTER TABLE [dbo].[RouteComments] ADD  CONSTRAINT [DF_RouteComments_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}

bool ExportSql::createStationsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Stations", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE `Stations` ("\
    "`stationKey` integer NOT NULL primary key AUTOINCREMENT,"\
    "`route` int(11) NOT NULL DEFAULT 0,"\
    "`name` varchar(75) NOT NULL,"\
    "`suffix` varchar(4) NOT NULL DEFAULT '',"\
    "`latitude` decimal(15,13) NOT NULL DEFAULT 0.0,"\
    "`longitude` decimal(15,13) NOT NULL DEFAULT 0.0,"\
    "`startDate` date NOT NULL,"\
    "`endDate` date NOT NULL,"\
    "`SegmentId` int(11) NOT NULL DEFAULT -1,"\
    "`point` int(11) NOT NULL DEFAULT 0,"\
    "`infoKey` int(11) DEFAULT NULL,"\
    "`geodb_loc_id` int(11) DEFAULT NULL,"\
    "`routeType` int(11) NOT NULL DEFAULT -1,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"\
    "constraint main unique (`route`,`name`,`startDate`,`endDate`),"\
    "constraint `stationKey` UNIQUE (`stationKey`));";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Stations` ("\
    "`stationKey` int(11) NOT NULL AUTO_INCREMENT,"\
    "`route` int(11) NOT NULL,"\
    "`name` varchar(75) NOT NULL,"\
    "`suffix` varchar(4) NOT NULL DEFAULT '',"\
    "`latitude` decimal(15,13) NOT NULL,"\
    "`longitude` decimal(15,13) NOT NULL,"\
    "`startDate` date NOT NULL,"\
    "`endDate` date NOT NULL,"\
    "`SegmentId` int(11) Default -1,"\
    "`point` int(11) NOT NULL DEFAULT 0,"\
    "`infoKey` int(11) DEFAULT NULL,"\
    "`geodb_loc_id` int(11) DEFAULT NULL,"\
    "`routeType` int(11) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`route`,`name`,`startDate`,`endDate`),"\
    "UNIQUE KEY `stationKey` (`stationKey`)"\
    ") ENGINE=InnoDB AUTO_INCREMENT=41 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;";
  CommandText.append("SET QUOTED_IDENTIFIER ON;");
  CommandText.append("SET ANSI_PADDING ON;");
  CommandText.append("CREATE TABLE [dbo].[stations]("\
        "[stationKey] [int] IDENTITY(1,1) NOT NULL,"\
        "[route] [int] NOT NULL,"\
        "[name] [varchar](75) NOT NULL,"\
        "[suffix] varchar(4) NOT NULL DEFAULT '',"\
        "[latitude] [decimal](15, 13) NOT NULL,"\
        "[longitude] [decimal](15, 13) NOT NULL,"\
        "[startDate] [date] NOT NULL,"\
        "[endDate] [date] NOT NULL,"\
        "[SegmentId] [int] DEFAULT -1,"\
        "[point] [int] NOT NULL DEFAULT 0,"\
        "[infoKey] [int] NULL,"\
        "[geodb_loc_id] [int] NULL,"\
        "[routeType] [int] NOT NULL,"\
        "[lastUpdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_station] PRIMARY KEY CLUSTERED"\
    "("\
        "[route] ASC,"\
        "[name] ASC,"\
        "[startDate] ASC,"\
        "[endDate] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];");
  CommandText.append("SET ANSI_PADDING OFF;");
//  CommandText.append("ALTER TABLE [dbo].[stations]  WITH CHECK ADD  CONSTRAINT [FK__stations__segmen__4AB81AF0] FOREIGN KEY([lineSegmentId])"\
//    "REFERENCES [dbo].[LineSegment] ([Key]);");
  //CommandText.append("ALTER TABLE [dbo].[stations] CHECK CONSTRAINT [FK__stations__segmen__4AB81AF0];");
  CommandText.append("ALTER TABLE [dbo].[stations] ADD  CONSTRAINT [DF_stations_route]  DEFAULT ((0)) FOR [route];");
  CommandText.append("ALTER TABLE [dbo].[stations] ADD  CONSTRAINT [DF_stations_routeType]  DEFAULT ((0)) FOR [routeType];");
  CommandText.append("ALTER TABLE [dbo].[stations] ADD  CONSTRAINT [DF_stations_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}

bool ExportSql::createTerminalsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Terminals", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE `Terminals` (  `Route` int(11) NOT NULL,  `Name` varchar(125) NOT NULL,  `StartDate` date NOT NULL,  `EndDate` date NOT NULL,  `StartSegment` int(11) NOT NULL,  `StartWhichEnd` char(1) NOT NULL,  `EndSegment` int(11) NOT NULL,  `EndWhichEnd` char(1) NOT NULL,  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  constraint pk PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`))";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Terminals` ("\
    "`Route` int(11) NOT NULL,"\
    "`Name` varchar(125) CHARACTER SET latin1 COLLATE latin1_german1_ci NOT NULL,"\
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
  CommandText = "SET ANSI_NULLS ON;";
  CommandText.append("SET QUOTED_IDENTIFIER ON;");
  CommandText.append("SET ANSI_PADDING ON;");
  CommandText.append("CREATE TABLE [dbo].[Terminals]("\
        "[Route] [int] NOT NULL,"\
        "[StartDate] [date] NOT NULL,"\
        "[EndDate] [date] NOT NULL,"\
        "[StartSegment] [int] NOT NULL,"\
        "[StartWhichEnd] [char](1) NOT NULL,"\
        "[EndSegment] [int] NOT NULL,"\
        "[EndWhichEnd] [char](1) NOT NULL,"\
        "[name] [varchar](125) NULL,"\
        "[lastUpdate] [datetime] NOT NULL);");
  CommandText.append("ALTER TABLE [dbo].[Terminals] ADD  CONSTRAINT [DF_Terminals_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];");
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}

#if 0 // template
bool ExportSql::createRouteCommentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("RouteComments", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "";
 else if(dbType == "MySql")
  CommandText = "";
 else if(dbType == "MsSql")
 {
  CommandText = "";
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}
#endif

bool ExportSql::createAltRouteTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("altRoute", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE `altRoute` (" \
    "`route` integer NOT NULL primary key AUTOINCREMENT, "\
    "`routePrefix` varchar(10) default '', " \
    "`routeAlpha` varchar(10) NOT NULL, "\
    "`baseRoute` int(11) NOT NULL DEFAULT 0, " \
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,  CONSTRAINT `index` unique ( routePrefix, routeAlpha) )";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `altRoute` (" \
    "`route` int(11) NOT NULL AUTO_INCREMENT, " \
    "`routePrefix` varchar(10) NOT NULL DEFAULT '', "\
    "`routeAlpha` varchar(10) NOT NULL, "\
    "`baseRoute` int(11) NOT NULL, "\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP," \
    "PRIMARY KEY (`routePrefix`,`routeAlpha`)," \
    "UNIQUE KEY `route` (`route`) "\
    ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {

  /****** Object:  Table [dbo].[AltRoute]    Script Date: 03/21/2016 13:22:36 ******/
  CommandText =
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
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
bool ExportSql::createParametersTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Parameters", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE if not exists `Parameters` ("\
          "`key` integer NOT NULL primary key asc AUTOINCREMENT,"\
          "`lat` decimal(18,15) NOT NULL default (0),"\
          "`lon` decimal(18,15) NOT NULL default (0),"\
          "`title` varchar(50) NOT NULL,"\
          "`city` varchar(50) NOT NULL,"\
          "`minDate` date NOT NULL,"\
          "`maxDate` date NOT NULL,"\
          "`alphaRoutes` char(1) NOT NULL default ('Y'),"\
          "`lastUpdate` timestamp NOT NULL DEFAULT (CURRENT_TIMESTAMP));";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Parameters` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`lat` decimal(18,15) NOT NULL,"\
    "`lon` decimal(18,15) NOT NULL,"\
    "`title` varchar(50) NOT NULL,"\
    "`city` varchar(50) NOT NULL,"\
    "`minDate` date NOT NULL,"\
    "`maxDate` date NOT NULL,"\
    "`alphaRoutes` char(1) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "UNIQUE KEY `key` (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "SET ANSI_PADDING ON;"\
  "CREATE TABLE [dbo].[parameters]("\
        "[key] [int] IDENTITY(1,1) NOT NULL,"\
        "[lat] [decimal](18, 15) NOT NULL,"\
        "[lon] [decimal](18, 15) NOT NULL,"\
        "[title] [varchar](50) NOT NULL,"\
        "[city] [varchar](50) NOT NULL,"\
        "[minDate] [date] NOT NULL,"\
        "[maxDate] [date] NOT NULL,"\
        "[alphaRoutes] [char](1) NOT NULL,"\
        "[lastUpdate] [datetime] NOT NULL"\
    ") ON [PRIMARY];"\
   "SET ANSI_PADDING OFF;"\
   "ALTER TABLE [dbo].[parameters]  WITH CHECK ADD  CONSTRAINT [CK_parameters] CHECK  (([alphaRoutes]='Y' OR [alphaRoutes]='N'));"\
   "ALTER TABLE [dbo].[parameters] CHECK CONSTRAINT [CK_parameters];"\
   "ALTER TABLE [dbo].[parameters] ADD  CONSTRAINT [DF_parameters_minDate]  DEFAULT ('1899-01-01') FOR [minDate];"\
   "ALTER TABLE [dbo].[parameters] ADD  CONSTRAINT [DF_parameters_maxDate]  DEFAULT ('1966-5-21') FOR [maxDate];"\
   "ALTER TABLE [dbo].[parameters] ADD  CONSTRAINT [DF_parameters_alphaRoutes]  DEFAULT ('N') FOR [alphaRoutes];"\
   "ALTER TABLE [dbo].[parameters] ADD  CONSTRAINT [DF_parameters_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << CommandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}
bool ExportSql::createTractionTypesTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("TractionTypes", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE if not exists `TractionTypes` (\
          `tractionType` integer NOT NULL primary key AUTOINCREMENT,\
          `description` varchar(50) NOT NULL DEFAULT '',\
          `displayColor` char(7) NOT NULL DEFAULT '#000000',\
          `routeType` int(11) NOT NULL DEFAULT '0',\
          `icon` varchar(10) NOT NULL DEFAULT '',\
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP \
        );";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `TractionTypes` ("\
    "`tractionType` int(11) NOT NULL AUTO_INCREMENT,"\
    "`description` varchar(50) NOT NULL DEFAULT '',"\
    "`displayColor` char(7) NOT NULL DEFAULT '#000000',"\
    "`routeType` int(11) NOT NULL DEFAULT '0',"\
    "`icon` varchar(10) NOT NULL DEFAULT '',"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`tractionType`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;"\
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
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
bool ExportSql::createCompaniesTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Companies", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE if not exists `Companies` ( "\
          "`key` integer NOT NULL primary key AUTOINCREMENT,"\
          "`Description` varchar(50) NOT NULL, "\
          "`routePrefix` varchar(10) default '', "\
          "`startDate` date DEFAULT NULL," \
          "`endDate` date DEFAULT NULL," \
          "`firstRoute` int(11) DEFAULT NULL," \
          "`lastRoute` int(11) DEFAULT NULL," \
          "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP" \
    ");";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Companies` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`Description` varchar(50) NOT NULL,"\
    "`routePrefix` varchar(10) DEFAULT '',"\
    "`startDate` date DEFAULT NULL,"\
    "`endDate` date DEFAULT NULL,"\
    "`firstRoute` int(11) DEFAULT NULL,"\
    "`lastRoute` int(11) DEFAULT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "UNIQUE KEY `key` (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;"\
  "SET QUOTED_IDENTIFIER ON;"\
  "CREATE TABLE [dbo].[Companies]("\
        "[key] [int] IDENTITY(1,1) NOT NULL,"\
        "[Description] [varchar](50) NOT NULL,"\
    "[routePrefix] [varchar](10) NOT NULL,"\
        "[startDate] [date] NULL,"\
        "[endDate] [date] NULL,"\
        "[firstRoute] [int] NULL,"\
        "[lastRoute] [int] NULL,"\
        "[lastUpdate] [datetime] NOT NULL, "\
    "CONSTRAINT [PK_Companies_1] PRIMARY KEY CLUSTERED"\
    "(" \
        "[key] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];"\
    "SET ANSI_PADDING OFF;"\
    "ALTER TABLE [dbo].[Companies] ADD  CONSTRAINT [DF_Companies_routePrefix]  DEFAULT ('') FOR [routePrefix];"\
    "ALTER TABLE [dbo].[Companies] ADD  CONSTRAINT [DF_Companies_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];";
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
bool ExportSql::createIntersectionsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Intersections", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "CREATE TABLE if not exists `Comments` ("\
          "`commentKey` integer NOT NULL primary key AUTOINCREMENT,"\
          "`tags` varchar(1000) NOT NULL,"\
          "`comments` mediumtext NOT NULL,"\
          "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP"\
        ");";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Intersections` ("\
    "`key` int(11) NOT NULL AUTO_INCREMENT,"\
    "`Street1` varchar(50) NOT NULL,"\
    "`Street2` varchar(50) NOT NULL,"\
    "`Latitude` decimal(15,13) NOT NULL,"\
    "`Longitude` decimal(15,13) NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`key`)"\
  ") ENGINE=InnoDB AUTO_INCREMENT=507 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;"\
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
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
bool ExportSql::createCommentsTable(QSqlDatabase db, QString dbType)
{
 QSqlQuery query = QSqlQuery(db);
 QString CommandText;
 bool bQuery;
 dropTable("Comments", db, dbType);

 if(dbType == "Sqlite")
  CommandText = "        CREATE TABLE if not exists `Intersections` ("\
          "`key` integer NOT NULL primary key AUTOINCREMENT,"\
          "`Street1` varchar(50) NOT NULL,"\
          "`Street2` varchar(50) NOT NULL,"\
          "`Latitude` decimal(15,13) NOT NULL,"\
          "`Longitude` decimal(15,13) NOT NULL,"\
          "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP"\
        ");";
 else if(dbType == "MySql")
  CommandText = "CREATE TABLE `Comments` ("\
    "`commentKey` int(11) NOT NULL AUTO_INCREMENT,"\
    "`tags` varchar(1000) NOT NULL,"\
    "`comments` mediumtext CHARACTER SET utf8 NOT NULL,"\
    "`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"\
    "PRIMARY KEY (`commentKey`)"\
    ") ENGINE=InnoDB AUTO_INCREMENT=79 DEFAULT CHARSET=latin1";
 else if(dbType == "MsSql")
 {
  CommandText = "SET ANSI_NULLS ON;"\
   "SET QUOTED_IDENTIFIER ON;"\
   "SET ANSI_PADDING ON;"\
   "CREATE TABLE [dbo].[comments]("\
        "[commentKey] [int] IDENTITY(1,1) NOT NULL,"\
        "[tags] [varchar](1000) NOT NULL,"\
        "[comments] [varchar](max) NOT NULL,"\
        "[lastupdate] [datetime] NOT NULL,"\
     "CONSTRAINT [PK_comments] PRIMARY KEY CLUSTERED"\
    "("\
        "[commentKey] ASC"\
    ")WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]"\
    ") ON [PRIMARY];" \
    "SET ANSI_PADDING OFF;"\
    "ALTER TABLE [dbo].[comments] ADD  CONSTRAINT [DF_comments_new_comments]  DEFAULT ('') FOR [comments];"\
    "ALTER TABLE [dbo].[comments] ADD  CONSTRAINT [DF_comments_lastupdate]  DEFAULT (getdate()) FOR [lastupdate];";
 }
 query.prepare(CommandText);
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
bool ExportSql::createMySqlFunctions(QSqlDatabase db)
{
 return false;
}
