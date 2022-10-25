#include "sql.h"

#include "data.h"
#include "mainwindow.h"
//#include <sqlite3.h>
#include "/home/allen/Qt/5.15.2/Src/qtbase/src/3rdparty/sqlite/sqlite3.h"
#include <algorithm>
#include <QDebug>
#include <QTextEdit>

SQL* SQL::_instance = NULL;
SQL::SQL()
{
 currentTransaction = "";
 config = Configuration::instance();
 _instance = this;
}

SQL* SQL::instance()
{
 if(_instance == NULL)
  _instance = new SQL();
 return _instance;
}

void SQL::setConfig(Configuration *cfg)
{
 config = cfg;
}

bool SQL::dbOpen()
{
    QSqlDatabase db = QSqlDatabase::database();
    if(db.isOpen())
        return true;

    QSqlError err = db.lastError();
    QString msg;
    msg = err.text() + "\n"
             + db.driverName() + "\n"
             + "User:" + db.userName() + "\n"
             + "Host:" + db.hostName()+ "\n"
             + "Connection name: " + db.connectionName() + "\n"
             + "DSN:" + db.databaseName() + "\n"
      + "driver: " + config->currConnection->driver() + "\n";
    if(config->currConnection->driver()=="QSQLITE")
    {
     QFileInfo info(db.hostName());
     msg.append(info.absoluteFilePath());
    }
    QMessageBox::warning(NULL, "Warning", msg);
#if 0
    db = QSqlDatabase::addDatabase("QODBC");
    //db.setHostName("10.0.1.100");
    //db.setPort(1433);
    db.setDatabaseName("Newhp2_Berlin");
    //db.setDatabaseName("SQLSERVER_SAMPLE");
    db.setUserName("newhp2\\allen");
    db.setPassword("iic723@knobacres");
#else

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Resources/databases/StLouis.sqlite3");
#endif
    bool ok = db.open();
    if(!ok)
    {
        QSqlError err = db.lastError();
        qDebug() << err.text();
        qDebug() << db.driverName();
        qDebug() << "User:" + db.userName();
        qDebug() << "Host:" + db.hostName();
        qDebug() << "Connection name: " + db.connectionName();
        qDebug() << "DSN:" + db.databaseName();

    }
    else
    {
//        cout << "Logged on\n";
     if(config->currConnection->driver()=="QODBC" || config->currConnection->driver() == "QODBC3")
     {
      if(config->currConnection->useDatabase() != "default" && config->currConnection->useDatabase() != "")
      {
       QSqlQuery query = QSqlQuery(db);
       if(!query.exec(tr("use [%1]").arg(config->currConnection->useDatabase())))
       {
        SQLERROR(query);
        db.close();
        exit(EXIT_FAILURE);
       }
      }
     }
    }

    return ok;
}

/// <summary>
/// Begin a transaction
/// </summary>
/// <param name="name"></param>
void SQL::BeginTransaction (QString name)
{
 if(!currentTransaction.isEmpty())
 {
//  qDebug() << "Warning! Begin transaction ignored for '" + name + "'; transaction '" + currentTransaction + "' is already active";
  return;
 }
 QString commandText = "Begin Transaction " +name;
 QSqlDatabase db = QSqlDatabase::database();
 //QSqlQuery query = QSqlQuery(db);
 //bool bQuery = query.exec(commandText);
 bool bQuery = db.transaction();
 if(!bQuery)
 {
     QSqlError err = db.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     //db.close();
     //exit(EXIT_FAILURE);
 }
 currentTransaction = name;
}

/// <summary>
/// Commits a series of sql commands
/// </summary>
/// <param name="name"></param>
void SQL::CommitTransaction (QString name)
{
  if(name != currentTransaction)
  {
//   qDebug() << QString("Commit transaction for '%1' ignored because another transaction: '%2' is active").arg(name).arg(currentTransaction);
      return;
  }
  QString commandText = "Commit Transaction " +name;
  QSqlDatabase db = QSqlDatabase::database();
  //QSqlQuery query = QSqlQuery(db);
  //bool bQuery = query.exec(commandText);
  bool bQuery = db.commit();
  if(!bQuery)
  {
      QSqlError err = db.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      //db.close();
      //exit(EXIT_FAILURE);
  }
  qDebug() << commandText << " comitted";
  currentTransaction = "";
}

/// <summary>
/// Aborts and rolls back a series of SQL commands
/// </summary>
/// <param name="name"></param>
void SQL::RollbackTransaction (QString name)
{
 QString commandText = "Rollback Transaction " +name;
 QSqlDatabase db = QSqlDatabase::database();
 //QSqlQuery query = QSqlQuery(db);
 //bool bQuery = query.exec(commandText);
 bool bQuery = db.rollback();
 if(!bQuery)
 {
  QSqlError err = QSqlError();
  qWarning() <<tr("Rollback transaction %1 failed %2").arg(currentTransaction).arg(err.driverText());
  db.close();
  exit(EXIT_FAILURE);
 }
 currentTransaction = "";
}

void SQL::myExceptionHandler(Exception e)
{
    Q_UNUSED(e)
    qDebug() << "SQL exception " << e.msg;
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
    exit(EXIT_FAILURE);
}

int routeSortType;
bool sortbyalpha_route_date( const RouteData & s1 , const RouteData & s2 )
{
//cout<<"\n" << __FUNCTION__;
    QString s1Route, s2Route, s1Sfx=".0", s2Sfx = ".0";

    s1Route = s1.alphaRoute;
    if(s1.alphaRoute.contains('.'))
    {
        s1Route = s1.alphaRoute.mid(0,s1.alphaRoute.indexOf('.'));
        s1Sfx = s1.alphaRoute.mid(s1.alphaRoute.indexOf('.'));
    }
    s2Route = s2.alphaRoute;
    if(s2.alphaRoute.contains('.'))
    {
        s2Route = s2.alphaRoute.mid(0,s2.alphaRoute.indexOf('.'));
        s2Sfx = s2.alphaRoute.mid(s2.alphaRoute.indexOf('.'));
    }
    if(s1Route.length() > 0 && s1Route.at(0).isNumber())
    {
        int n=0;
        for(int i=0; i < s1Route.length(); i++)
        {
            if(s1Route.at(i).isNumber())
            {
                n++;
                continue;
            }
            break;
        }
        // add leading zeroes
        if(n == 1)
            s1Route = "00" + s1Route;
        if(n == 2 )
            s1Route  = "0" + s1Route;
    }
    else
    {
        // Alpha Route
        if(s1Route.length()> 1 && s1Route.at(1).isDigit())
        {
            QString str = s1Route;
            if(str.length()==2)
                str.insert(1,QString("0"));
            s1Route = str;
        }
    }
    if(s2Route != "" && s2Route.at(0).isNumber())
    {

        int n=0;
        for(int i=0; i < s2Route.length(); i++)
        {
            if(s2Route.at(i).isNumber())
            {
                n++;
                continue;
            }
            break;
        }
        // add leading zeroes
        if(n == 1)
            s2Route = "00" + s2Route;
        if(n == 2)
            s2Route = "0" + s2Route;
    }
    else
    {
        // Alpha route
        if(s2Route.length() > 1 && s2Route.at(1).isDigit())
        {
            QString str = s2Route;
            if(str.length()==2)
                str.insert(1,QString("0"));
            s2Route = str;
        }

    }

    switch(routeSortType)
       {
    case 0:
    default:
        return ( s1Route + s1.endDate.toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.endDate.toString("yyyy/MM/dd")+ s2Sfx);
    case 1:
        return ( s1Route + s1.startDate.toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.startDate.toString("yyyy/MM/dd")+ s2Sfx);
    case 2:
        return ( s1Route + s1.name + s1.endDate.toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.name + s2.endDate.toString("yyyy/MM/dd")+ s2Sfx);
    case 3:
        return ( s1Route + s1.name + s1.startDate.toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.name + s2.startDate.toString("yyyy/MM/dd")+ s2Sfx);
    case 4:
        return (s1.name + s1Route  + s1.startDate.toString("yyyy/MM/dd")+ s1Sfx <   s2.name + s2Route + s2.startDate.toString("yyyy/MM/dd")+ s2Sfx);


    }

    //return ( s1Route + s1.endDate.date().toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.endDate.date().toString("yyyy/MM/dd")+ s2Sfx);
}
QList<RouteData> SQL::getRoutesByEndDate()
{
    return getRoutesByEndDate(0);
}

QList<RouteData> SQL::getRoutesByEndDate(qint32 companyKey)
{
 QList<RouteData> list;
 //QList<routeData> list2;
 RouteData rd;
 QSqlQuery query;
 QString commandText;
 qint32 currRoute = -1, route = 0;
 QString currName = "", name = "";
 QDate currDate, date;
 QString currDateStr, dateStr;
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 if(companyKey < 1)
  //commandText = "Select distinct a.route, name, a.endDate, a.companyKey, tractionType/*, routeAlpha, c.baseRoute*/ from Routes a join altRoute c on a.route =  c.route group by a.route, name, a.endDate, a.companykey,tractionType, c.routeAlpha/*, c.baseRoute*/ order by c.routeAlpha, name, a.endDate";
  commandText = "Select distinct a.route, name, a.endDate, a.companyKey, tractionType, routeAlpha"
                " from Routes a join altRoute c on a.route =  c.route"
                " group by a.route, name, a.endDate/*, a.companykey,tractionType, c.routeAlpha*/"
                " order by c.routeAlpha, name, a.endDate";
 else
  //commandText = "Select distinct a.route, name, a.endDate, a.companyKey, tractionType, routeAlpha/*, c.baseRoute*/ from Routes a join altRoute c on a.route = c.route  where a.companyKey = " + QString("%1").arg(companyKey)+ " group by a.route, name, a.endDate, a.companykey, tractionType, c.routeAlpha/*, c.baseRoute*/ order by c.routeAlpha, name, a.endDate";
  commandText = "Select distinct a.route, name, a.endDate, a.companyKey, tractionType, routeAlpha"
                " from Routes a join altRoute c on a.route = c.route"
                "  where a.companyKey = " + QString("%1").arg(companyKey)+ ""
                " group by a.route, name, a.endDate/*, a.companykey, tractionType, c.routeAlpha*/"
                " order by c.routeAlpha, name, a.endDate";
 query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  return list;
 }
 while (query.next())
 {
   route = query.value(0).toInt();
   name = query.value(1).toString().trimmed();
   date = query.value(2).toDate();
   currDateStr = currDate.toString("yyyy/MM/dd");
   dateStr = date.toString("yyyy/MM/dd");
   if(route == currRoute && currName == name && /*currDateStr == dateStr*/date == currDate)
       continue;
   else
   {
//    if (currRoute != -1)
//    {
//     list.append(rd);
//     rd = RouteData();
//    }
    currRoute = route;
    currName = name;
    currDate = date;
    rd.route = route;
    rd.name = name;
    rd.endDate = date;
    rd.companyKey = query.value(3).toInt();
    rd.tractionType = query.value(4).toInt();
    rd.alphaRoute = query.value(5).toString();
//    rd.baseRoute = query.value(6).toInt();
//    if(rd.baseRoute == 0)
//     rd.baseRoute = route;
    list.append(rd);
   }

//  if(currRoute!= -1)
//   list.append(rd);
  }
  //foreach(routeData rd1 in list)
  for(int i=0; i < list.size(); i++)
  {
   RouteData *rd1 = (RouteData*)&list.at(i);
   bool bStartDateFound = false;
   QString strRoute = QString("%1").arg(rd1->route);
   //commandText = "Select Max(endDate) from Routes  where route = "+strRoute+" and name = '"+rd1->name+"'  and endDate < '"+rd1->endDate.toString("yyyy/MM/dd")+"' group  by endDate order by endDate asc";
   commandText = "Select Max(startDate) from Routes  where route = "+strRoute+" and name = '"+rd1->name+"'  and endDate = '"+rd1->endDate.toString("yyyy/MM/dd")+"' and startDate < endDate group  by endDate order by endDate asc";
//             if(config->currConnection->servertype() == "MsSql")
//                 commandText = "Select Max(startDate) from Routes  where route = "+strRoute+" and RTRIM(LTRIM(name)) = '"+rd1->name.trimmed() +"'  and endDate >= '"+rd1->endDate.toString("yyyy/MM/dd")+"' and startDate <= '"+ rd1->endDate.toString("yyyy/MM/dd")+"'group  by endDate order by endDate asc";
//            else
//                 commandText = "Select Max(startDate) from Routes  where route = "+strRoute+" and TRIM(name) = '"+rd1->name.trimmed() +"'  and endDate >= '"+rd1->endDate.toString("yyyy/MM/dd")+"' and startDate <= '"+ rd1->endDate.toString("yyyy/MM/dd")+"'group  by endDate order by endDate asc";

   bQuery = query.exec(commandText);
   if(!bQuery)
   {
    SQLERROR(query);
    db.close();
    exit(EXIT_FAILURE);
   }

   while (query.next())
   {
    rd1->startDate = query.value(0).toDate();
    //rd1->startDate = query.value(0).toDateTime().addDays(1);
    //rd1->startDate.addDays(1);
    bStartDateFound = true;
   }

   if (!bStartDateFound)
   {
       commandText = "Select Min(startDate) from Routes "  \
           "where route = "+strRoute+"  and startDate <= '"+rd1->endDate.toString("yyyy/MM/dd")+"' " \
          "group by startDate";
       bQuery = query.exec(commandText);
       if(!bQuery)
       {
        SQLERROR(query);
           db.close();
           exit(EXIT_FAILURE);
       }

       //if (myReader.HasRows != true)
       if(!query.isActive())
       {
           //
           //list2.append(rd1);
           continue;
       }

       while (query.next())
       {
           rd1->startDate = query.value(0).toDate();
           //list.append(*rd1);

       }
       continue;
   }
   QString rd1StartDate =rd1->startDate.toString("yyyy/MM/dd");
   //list2.append(rd1);
  }
 //db.close();
 routeSortType = config->currCity->routeSortType;
 qSort(list.begin(), list.end(), sortbyalpha_route_date);
 return list;
}

#if 0
routeInfo SQL::GetRoutePoints(qint32 route, QString name, QString date)
{
 routeInfo ri = routeInfo();
 segmentGroup sg = segmentGroup();
 segmentData sd =  segmentData();
 QSqlQuery query;
 QString commandText;
 qint32 currSegmentId = -1, sequence = -1, prevSeq = -1;
 double prevLat = 0, prevLon = 0, distance = 0;
 QString sDistance, sOneWay;

 ri.route = route;
 //ri.segments = QList<segmentData>();
 qint32 SegmentId;
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString strRoute = QString("%1").arg( route);
  commandText = "select name, tractionType from Routes where route = " + strRoute + " and '" + date + "' between startDate and endDate";
  query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      db.close();
      exit(EXIT_FAILURE);
  }
  if(!query.isActive())
  {
      return routeInfo();
  }
  while (query.next())
  {
      ri.routeName = query.value(0).toString();
      ri.tractionType = query.value(1).toInt();
      break;
  }
  if(config->currConnection->servertype() == "MsSql")
   commandText = "SELECT StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId, Sequence, Description, OneWay, route, TractionType, length FROM view_all_route_points where Route = " + strRoute + " and '" + date + "' between startDate and endDate and RTRIM(LTRIM(name)) = '" + name + "' order by route, segmentId, sequence";
  else
   commandText = "SELECT StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId, Sequence, Description, OneWay, route, TractionType, length FROM view_all_route_points where Route = " + strRoute + " and '" + date + "' between startDate and endDate and TRIM(name) = '" + name + "' order by route, segmentId, sequence";

  //qDebug() << commandText;
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   db.close();
   exit(EXIT_FAILURE);
  }
  if (!query.isActive())
  {
   return routeInfo();
  }
  while (query.next())
  {
   SegmentId = query.value(5).toInt();
   sequence = query.value(6).toInt());
   distance = query.value(11).toDouble();
   sDistance = query.value(11).toString();
   sOneWay = query.value(8).toString();

   if (SegmentId != currSegmentId)
   {
    if (currSegmentId != -1)
    {
     prevSeq = -1;
     ri.segments.append(sg);
     sg = segmentGroup();
    }
    prevSeq = -1;
    prevLat = query.value(0).toDouble();
    prevLon = query.value(1).toDouble();

    sg.SegmentId = SegmentId;
    sg.Description = query.value(7).toString();
    sg.OneWay = query.value(8).toString();
    sg.tractionType = query.value(10).toInt();
    //if (sg.tractionType < 1 || sg.tractionType > 6)
    //    sg.tractionType = 0;
    //sg.data = QList<segmentGroup>();
    //ri.segments.append(sg);
    currSegmentId = SegmentId;
   }
   sd = segmentData();
   sd.startLat = query.value(0).toDouble();
   sd.startLon = query.value(1).toDouble();
   sd.endLat = query.value(2).toDouble();
   sd.endLon = query.value(3).toDouble();
   sd.streetName = query.value(4).toString();
   sd.SegmentId = SegmentId;
   sd.sequence = sequence;
   sd.description = query.value(7).toString();
   sd.route = query.value(9).toInt();
   sd.oneWay = (sOneWay.toLower()=="y");
   if(sd.oneWay)
    sd.distance = distance;
   else
    sd.distance = distance*2;

   Bearing b(sd.startLat, sd.startLon, sd.endLat, sd.endLon);
   double calcDistance = b.Distance();
   if(calcDistance < distance)
   {
    if(sd.oneWay)
     sd.distance = calcDistance;
    else
     sd.distance = calcDistance*2;
   }
   sg.distance += sd.distance;
   ri.length += sd.distance;

   if (sequence == prevSeq)
   {
    qDebug() << QString(tr("Duplicate route segment: %1 ignored")).arg(currSegmentId)+ " Route:"+ QString("%1").arg(route)+ " " + name + " "+ date;
    continue;
   }
   else
    sg.data.append(sd);
   if( prevLat != sd.startLat || prevLon != sd.startLon)
    qDebug() << tr("Start location mismatch: segmentId = ") + QString("%1").arg(SegmentId) + " sequence = " +QString("%1").arg(sequence);
   if((prevSeq + 1) != sequence)
    qDebug() << tr("Sequence error: segmentId = ") + QString("%1").arg(SegmentId) + " sequence = " +QString("%1").arg(sequence);
   prevSeq++;
   prevLat = sd.endLat;
   prevLon = sd.endLon;
  }
  ri.segments.append(sg);
 }
 catch (Exception& e)
 {
  myExceptionHandler(e);
 }
 return ri;
}
#else
RouteInfo SQL::getRoutePoints(qint32 route, QString name, QString date)
{
 RouteInfo ri = RouteInfo();
 SegmentInfo si = SegmentInfo();
 //segmentData sd =  segmentData();
 QSqlQuery query;
 QString commandText;
 qint32 currSegmentId = -1;
 double distance = 0;
 QString sDistance, sOneWay;

 ri.route = route;
 //ri.segments = QList<segmentData>();
 qint32 SegmentId;
 if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString strRoute = QString("%1").arg( route);
 commandText = "select name, tractionType from Routes where route = " + strRoute + " and '" + date + "' between startDate and endDate";
 query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
//      db.close();
//      exit(EXIT_FAILURE);
   return RouteInfo();
 }
 if(!query.isActive())
 {
     return RouteInfo();
 }
 while (query.next())
 {
     ri.routeName = query.value(0).toString();
     ri.tractionType = query.value(1).toInt();
     break;
 }
 if(config->currConnection->servertype() == "MsSql")
  commandText = "SELECT s.pointArray, s.street, s.SegmentId, s.Description, r.OneWay, r.route,"
                " r.TractionType, s.length, s.Tracks, s.Type, r.trackUsage"
                " FROM Routes r  join Segments s on r.lineKey = s.segmentId"
                " where r.Route = " + strRoute + " and '" + date + "' between r.startDate and r.endDate"
                " and RTRIM(LTRIM(name)) = '" + name + "' order by r.route, s.segmentId";
 else
  commandText = "SELECT s.pointArray, s.street, s.SegmentId, s.Description, r.OneWay, r.route,"
                " r.TractionType, s.length, s.Tracks, s.Type, r.trackUsage"
                " FROM Routes r join Segments s on r.lineKey = s.segmentId"
                " where r.Route = " + strRoute + " and '" + date + "' between r.startDate and r.endDate"
                " and TRIM(name) = '" + name + "' order by r.route, s.segmentId";

 //qDebug() << commandText;
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return RouteInfo();
 }
 while (query.next())
 {
  SegmentId = query.value(2).toInt();
  distance = query.value(7).toDouble();
  sDistance = query.value(7).toString();
  sOneWay = query.value(4).toString();

  if (SegmentId != currSegmentId)
  {
   if (currSegmentId != -1)
   {
    ri.segments.append(si);
    si = SegmentInfo();
   }

   si.segmentId = SegmentId;
   si.description = query.value(3).toString();
   si.oneWay = query.value(4).toString();
   si.tractionType = query.value(6).toInt();
   si.tracks = query.value(8).toInt();
   si.checkTracks();
   si.routeType = (RouteType)query.value(9).toInt();
   si.trackUsage = query.value(10).toString();

   currSegmentId = SegmentId;
  }
  si.length = distance;
  if(sOneWay.toLower()=="Y")
  {
   ri.length += distance;
  }
  else
  {
   ri.length += distance*2;
  }
  if(distance > 15.0)
   si.bNeedsUpdate = true;
  si.setPoints(query.value(0).toString());
 }
 ri.length = ri.length/2;
 ri.segments.append(si);
 return ri;
}
#endif

TerminalInfo SQL::getTerminalInfo(qint32 route, QString name, QString endDate)
{
 TerminalInfo ti;
 ti.startSegment = -1;
 ti.endSegment = -1;
 ti.route = -1;
 QString commandText;

 try
 {
  if(!dbOpen())
      throw Exception("dataase not open");
  QSqlDatabase db = QSqlDatabase::database();
  QString strRoute = QString("%1").arg(route);

  // Works in MSSSQL but not my SQL.
  // see: http://www.xaprb.com/blog/2006/05/26/how-to-write-full-outer-join-in-mysql/
  //        commandText = "select a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
  //            "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
  //            "from Terminals a "
  //            "full outer join Segments b on b.segmentId = startSegment "
  //            "full outer join Segments c on c.segmentId = endSegment "
  //            "where route = " + strRoute + " and a.endDate = '" + endDate + "' and name = '" + name + "'";
  if(config->currConnection->servertype() != "MsSql")
      commandText = "select a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
      "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
      "from Terminals a " \
      "join Segments b on b.segmentId = startSegment " \
      "join Segments c on c.segmentId = endSegment " \
      "where route = " + strRoute + " and a.endDate = '" + endDate + "' and name = '" + name + "'";
  else
      commandText = "select a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
          "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
          "from Terminals a " \
          "full outer join Segments b on b.segmentId = startSegment " \
          "full outer join Segments c on c.segmentId = endSegment " \
          "where route = " + strRoute + " and a.endDate = '" + endDate + "' and name = '" + name + "'";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
   SQLERROR(query);
//      db.close();
//      exit(EXIT_FAILURE);
      return TerminalInfo();
  }
  if (!query.isActive())
  {
      //throw (new ApplicationException("error selecting Terminal info"));
      ti.route = route;
      ti.name = name;
      //ti.endDate = endDate;
      ti.endLatLng =  LatLng();
      ti.startLatLng = LatLng();
      return ti;
  }
  while (query.next())
  {
      ti.startDate = query.value(0).toDateTime();
      ti.endDate = query.value(1).toDateTime();
      if (query.value(2).isNull())
          ti.startSegment = -1;
      else
      {
          ti.startSegment = query.value(2).toInt();
          ti.startWhichEnd = query.value(3).toString();
      }
      if (query.value(4).isNull())
          ti.endSegment = -1;
      else
      {
          ti.endSegment = query.value(4).toInt();
          ti.endWhichEnd = query.value(5).toString();
      }
      if (ti.startSegment > 0)
      {
          if(ti.startWhichEnd == "S" )
              ti.startLatLng =  LatLng(query.value(6).toDouble(), query.value(7).toDouble());
          else
              ti.startLatLng =  LatLng(query.value(8).toDouble(), query.value(9).toDouble());
      }
      if (ti.endSegment > 0 )
      {
          if (ti.endWhichEnd == "S")
              ti.endLatLng =  LatLng(query.value(10).toDouble(),query.value(11).toDouble());
          else
              ti.endLatLng =  LatLng(query.value(12).toDouble(), query.value(13).toDouble());
      }
  }
  ti.route = route;
  ti.name = name;

 }
 catch (Exception e)
 {
     myExceptionHandler(e);
 }

 return ti;
}

// return a list of TerminalInfo's using a segment
QList<TerminalInfo> SQL::getTerminalInfoUsingSegment(int segmentId)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;

 if(config->currConnection->servertype() != "MsSql")
     commandText = "select route, name, a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
     "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
     "from Terminals a " \
     "join Segments b on b.segmentId = startSegment " \
     "join Segments c on c.segmentId = endSegment " \
     "where " + QString("%1").arg(segmentId) + " in (startSegment, endSegment)";
 else
     commandText = "select route, name, a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
         "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
         "from Terminals a " \
         "full outer join Segments b on b.segmentId = startSegment " \
         "full outer join Segments c on c.segmentId = endSegment " \
     "where " + QString("%1").arg(segmentId) + " in (startSegment, endSegment)";

 QList<TerminalInfo> list;
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  TerminalInfo ti;
  ti.route = query.value(0).toInt();
  ti.name = query.value(1).toString();
  ti.startDate = query.value(2).toDateTime();
  ti.endDate = query.value(3).toDateTime();
  ti.startSegment = query.value(4).toInt();
  ti.startWhichEnd = query.value(5).toString();
  ti.endSegment = query.value(6).toInt();
  ti.endWhichEnd = query.value(7).toString();

  if (ti.startSegment > 0)
  {
      if(ti.startWhichEnd == "S" )
          ti.startLatLng =  LatLng(query.value(8).toDouble(), query.value(9).toDouble());
      else
          ti.startLatLng =  LatLng(query.value(10).toDouble(), query.value(11).toDouble());
  }
  if (ti.endSegment > 0 )
  {
      if (ti.endWhichEnd == "S")
          ti.endLatLng =  LatLng(query.value(12).toDouble(),query.value(13).toDouble());
      else
          ti.endLatLng =  LatLng(query.value(14).toDouble(), query.value(15).toDouble());
  }
  list.append(ti);
 }
 return list;
}

QString SQL::getAlphaRoute(qint32 route, qint32 company)
{
 QString routeAlpha = "";
 QSqlDatabase db = QSqlDatabase::database();
 QString strRoute = QString("%1").arg(route);
 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
  commandText = "select routeAlpha from altRoute a " \
   "where route = " + strRoute; // + " and c.`key` =" +QString::number(company) + "";
 else
   commandText = "select routeAlpha from altRoute a " \
   "where route = " + strRoute; // + " and c.[key] =" +QString::number(company) + "";

 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
     return "";
 }

 while (query.next())
 {
     routeAlpha = query.value(0).toString();
 }

 return routeAlpha;
}
QStringList SQL::getAlphaRoutes(QString text)
{
    QStringList list;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select routeAlpha from altRoute " \
                "where routeAlpha like '" + text+ "%'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return list;
        }

        while (query.next())
        {
            list.append(query.value(0).toString());
        }

    }
    catch (Exception& e)
    {
        myExceptionHandler(e);
    }

    return list;
}

QList<tractionTypeInfo> SQL::getTractionTypes()
{
    QList<tractionTypeInfo> myArray;
    tractionTypeInfo tti;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select tractionType, description, displayColor,routeType, icon from TractionTypes";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            tti = tractionTypeInfo();
            tti.tractionType = query.value(0).toInt();
            tti.description = query.value(1).toString();
            tti.displayColor = query.value(2).toString();
            tti.routeType = (RouteType)query.value(3).toInt();
            tti.icon = query.value(4).toString();
            myArray.append(tti);
        }
        
    }
    catch (Exception& e)
    {
        myExceptionHandler(e);

    }

    
    return myArray;
}

QList<SegmentInfo> SQL::getSegmentInfo()
{
 QList<SegmentInfo> myArray;
 SegmentInfo sI;
            
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "Select SegmentId, description, OneWay, startDate, endDate, length, points, "
"startLat, startLon, endLat, EndLon, type, street, pointArray, tracks from Segments ";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     qDebug() << "Is default database correct?";
//            db.close();
//            exit(EXIT_FAILURE);
     return QList<SegmentInfo>();
 }
 while (query.next())
 {
  sI = SegmentInfo();
  sI.segmentId = query.value(0).toInt();
  sI.description = query.value(1).toString();
  sI.oneWay = query.value(2).toString();
  sI.startDate = query.value(3).toDateTime().toString("yyyy/MM/dd") ;
  sI.endDate = query.value(4).toDateTime().toString("yyyy/MM/dd") ;
  sI.length = query.value(5).toDouble();
  sI.lineSegments = query.value(6).toInt();
  sI.startLat = query.value(7).toDouble();
  sI.startLon = query.value(8).toDouble();
  sI.endLat = query.value(9).toDouble();
  sI.endLon = query.value(10).toDouble();
  sI.bearing = Bearing(sI.startLat, sI.startLon, sI.endLat, sI.endLon);
  sI.direction = sI.bearing.strDirection();
  if (sI.oneWay == "N")
      sI.direction += "-" + sI.bearing.strReverseDirection();
  sI.routeType = (RouteType)query.value(11).toInt();
  sI.streetName = query.value(12).toString();
  sI.setPoints(query.value(13).toString());
  sI.tracks = query.value(14).toInt();
  sI.checkTracks();
  myArray.append(sI);
 }

 return myArray;
}

QMap<int, SegmentData> SQL::getSegmentDataList()
{
 QMap<int, SegmentData> myArray;

 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "Select SegmentId, description, OneWay, startDate, endDate, length, points, "
"startLat, startLon, endLat, EndLon, type, street, pointArray, tracks from Segments order by description";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     qDebug() << "Is default database correct?";
//            db.close();
//            exit(EXIT_FAILURE);
     return QMap<int, SegmentData>();
 }
 while (query.next())
 {
  SegmentData sd;
  sd._segmentId = query.value(0).toInt();
  sd._description = query.value(1).toString();
  sd._oneWay = query.value(2).toString();
  sd._startDate = query.value(3).toDate();
  sd._endDate = query.value(4).toDate();
  sd._length = query.value(5).toDouble();
  sd.points = query.value(6).toInt();
  sd._startLat = query.value(7).toDouble();
  sd._startLon = query.value(8).toDouble();
  sd._endLat = query.value(9).toDouble();
  sd._endLon = query.value(10).toDouble();
  sd._bearing = Bearing(sd._startLat, sd._startLon, sd._endLat, sd._endLon);
  sd._direction = sd._bearing.strDirection();
  sd._routeType = (RouteType)query.value(11).toInt();
  sd._streetName = query.value(12).toString();
  sd.setPoints(query.value(13).toString());  // array of points
//  if(sd.pointList().isEmpty())
//   populatePointList(sd);
  sd._tracks = query.value(14).toInt();
  if(sd.pointList().count() > 1)
  {
   sd._bearingStart = Bearing(sd._startLat, sd._startLon, sd.pointList().at(1).lat(), sd.pointList().at(1).lon());
   sd._bearingEnd = Bearing(sd.pointList().at(sd.points-2).lat(), sd.pointList().at(sd.points-2).lon(), sd._endLat, sd._endLon);
  }
  sd.bounds = Bounds(query.value(13).toString());

  myArray.insert(sd.segmentId(),sd);
 }

 return myArray;
}

SegmentInfo SQL::getSegmentInfo(int segmentId)
{
 SegmentInfo sI = SegmentInfo();

 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "Select SegmentId, description, OneWay, startDate, endDate, length, points,"
                     " startLat, startLon, endLat, endLon, type, street, pointArray, tracks"
                     " from Segments where segmentId = " + QString("%1").arg(segmentId);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return sI;
 }
 while (query.next())
 {
     sI.segmentId = query.value(0).toInt();
     sI.description = query.value(1).toString();
     sI.oneWay = query.value(2).toString();
     sI.startDate = query.value(3).toDate().toString("yyyy/MM/dd");
     sI.endDate = query.value(4).toDate().toString("yyyy/MM/dd");
     sI.length = query.value(5).toDouble();
     sI.lineSegments = query.value(6).toInt();
     sI.startLat = query.value(7).toDouble();
     sI.startLon = query.value(8).toDouble();
     sI.endLat = query.value(9).toDouble();
     sI.endLon = query.value(10).toDouble();
     sI.routeType = (RouteType)query.value(11).toInt();
     sI.bearing = Bearing(sI.startLat, sI.startLon, sI.endLat, sI.endLon);
     sI.direction = sI.bearing.strDirection();
     if (sI.oneWay == "N")
         sI.direction += "-" + sI.bearing.strReverseDirection();
     sI.streetName = query.value(12).toString();
     sI.setPoints(query.value(13).toString());
     sI.tracks = query.value(14).toInt();
     sI.checkTracks();
     if(sI.pointList.count() > 1)
     {
      sI.bearingStart = Bearing(sI.startLat, sI.startLon, sI.pointList.at(1).lat(), sI.pointList.at(1).lon());
      sI.bearingEnd = Bearing(sI.pointList.at(sI.points-2).lat(), sI.pointList.at(sI.points-2).lon(), sI.endLat, sI.endLon);
     }

 }
 return sI;
}

// update segment begin/end dates based on route usage
bool SQL::updateSegmentDates(SegmentInfo* si)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 commandText = "select min(startDate), max(EndDate) from routes where linekey = " +QString("%1").arg(si->segmentId);
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
    db.close();
    exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return  false;
 }
 while (query.next())
 {
  si->startDate = query.value(0).toString();
  si->endDate = query.value(1).toString();
 }
 commandText = "update Segments set startDate = '" + si->startDate
   + "', enddate = '" + si->endDate
   + "' where segmentId = " + QString("%1").arg(si->segmentId);
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
    db.close();
    exit(EXIT_FAILURE);
 }
 return true;
}
#if 0
// used to populate segmentInfo conversion
void SQL::populatePointList(SegmentData sd)
{
 int seq = 0;
 sd.points = 0;
 sd.length = 0;
 bool more=true;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 int rows;
 while(more)
 {
  commandText = "Select startLat, startLon, endLat, endLon, StreetName from LineSegment where segmentid = " +QString("%1").arg(sd.segmentId );
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
   SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
  }
  if (!query.isActive())
  {
     ////sI = null;
     //
     qDebug() << "SegmentID " + QString("%1").arg(sd.segmentId()) + " missing sequence nbr " + QString("%1").arg(sd.points - 1);
     qDebug() << " or number of points ( " + QString("%1").arg(sd.points) + ") is incorrect";

     //return sI;
  }
  while (query.next())
  {
   LatLng start(query.value(0).toDouble(), query.value(1).toDouble());
   LatLng end(query.value(2).toDouble(), query.value(3).toDouble());
   sd._streetName = query.value(4).toString();
   Bearing b(start,end);
   sd.length += b.Distance();
   if(seq == 0 && start.isValid())
   {
    sd.pointList().append(start);

    sd.bearingStart = b;
    sd._startLat = start.lat();
    sd._startLon = start.lon();
   }
   else
   {
    if(end.isValid() && !(sd.pointList().last() == start))
     qDebug() << tr("segment %1 point mismatch, seq %2").arg(sd.segmentId()).arg(seq);
    sd.bearingEnd = b;
    sd._endLat = end.lat();
    sd._endLon = end.lon();
   }
   if(end.isValid())
    sd.pointList().append(end);

   sd.points++;
   seq++;

  }
  more = false;
 }

 commandText = "update Segments set startDate = '"
   + sd.startDate.toString("yyyy/MM/dd")
   + "', endDate ='" + sd.endDate.toString("yyyy/MM/dd") + "', "
   + "street= '" + sd._streetName + "', pointArray= '"
   + sd.pointsString() + "', "
   + "length=" + QString::number(sd.length) +
     ", lastUpdate=:lastUpdate where SegmentId = " + QString("%1").arg(sd.segmentId());
 bQuery = query.prepare(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
 }
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
     //
 }
}
#endif
/// <summary>
/// Get all line segments for a segment in reverse order
/// </summary>
/// <param name="SegmentId"></param>
/// <returns></returns>
//QList<SegmentData> SQL::getSegmentData(qint32 SegmentId)
//{
//    QList<SegmentData> myArray;
//    SegmentData sd;
//    //double endLat = 0, endLon = 0;
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();

//        QString commandText = "SELECT StartLat,StartLon,EndLat,EndLon,StreetName,Sequence,Length FROM LineSegment where SegmentId = " + QString("%1").arg(SegmentId) + " order by sequence desc";
//        QSqlQuery query = QSqlQuery(db);
//        bool bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//         SQLERROR(query);
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        if (!query.isActive())
//        {
//            return myArray;
//        }
//        //                myArray = new LatLng[myReader.RecordsAffected];
//        while (query.next())
//        {
//            sd = SegmentData();
//            sd.startLat = query.value(0).toDouble();
//            sd.startLon = query.value(1).toDouble();
//            sd.endLat = query.value(2).toDouble();
//            sd.endLon = query.value(3).toDouble();
//            sd.streetName = query.value(4).toString();
//            sd.length = query.value(6).toDouble();
//            myArray.append(sd);
//        }
//        //myArray.Add(new LatLng(endLat, endLon));
//    }
//    catch (Exception e)
//    {
//        myExceptionHandler(e);
//    }
//    return myArray;
//}

SegmentData SQL::getSegmentData(qint32 SegmentId)
{
 SegmentData sd;

 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString commandText;

  if(config->currConnection->servertype() != "MsSql")
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, pointArray from Segments"
                     " where SegmentId = " + QString("%1").arg(SegmentId);
  else
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, pointArray from Segments"
                     " where SegmentId = " + QString("%1").arg(SegmentId);
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      db.close();
      exit(EXIT_FAILURE);
  }
  if (!query.isActive())
  {

      return sd;
  }
  while (query.next())
  {
   sd._segmentId = query.value(0).toInt();
   sd._description = query.value(1).toString();
   sd._tracks = query.value(2). toInt();
   sd._routeType = (RouteType)query.value(3).toInt();
   sd._startLat = query.value(4).toDouble();
   sd._startLon = query.value(5).toDouble();
   sd._endLat = query.value(6).toDouble();
   sd._endLon = query.value(7).toDouble();
   sd._length = query.value(8).toDouble();
   sd._startDate = query.value(9).toDate();
   sd._endDate = query.value(10).toDate();
   sd._direction = query.value(11).toString();
   sd._streetName = query.value(12).toString();
   sd.setPoints(query.value(13).toString());  // array of points
  }
  if(sd._startLat ==0 ||sd._startLat ==0 || sd._endLat == 0 || sd._endLon ==0 && sd._pointList.count() > 1 )
  {
   sd._startLat = sd._pointList.at(0).lat();
   sd._startLon = sd._pointList.at(0).lon();
   sd._endLat = sd._pointList.at(sd.points-1).lat();
   sd._endLon = sd._pointList.at(sd.points-1).lon();
  }
  sd._bearing = Bearing(sd._startLat, sd._startLon, sd._endLat, sd._endLon);
  if(sd.pointList().count() > 1)
  {
   sd._bearingStart = Bearing(sd._startLat, sd._startLon, sd.pointList().at(1).lat(), sd.pointList().at(1).lon());
   sd._bearingEnd = Bearing(sd.pointList().at(sd.points-2).lat(), sd.pointList().at(sd.points-2).lon(), sd._endLat, sd._endLon);
  }
  sd.bounds = Bounds(query.value(13).toString());
  if(sd._length == 0)
  {
   //sd._length = distance(LatLng(sd._startLat, sd._startLon), LatLng(sd._endLat, sd._endLon));
   for(int i=0; i <  sd.points-2; i++)
    sd._length += distance(sd._pointList.at(i), sd._pointList.at(i+1));
  }


  return  sd;
 }
 catch (Exception& e)
 {
    myExceptionHandler(e);
 }

 return sd;
} 

bool SQL::checkConnectingSegments(QList<SegmentInfo> segmentInfoList)
{
    int segmentsNotConnected = 0;
    int segmentsMultiplyConnected=0;
    for(int i=0; i < segmentInfoList.count(); i++)
    {
        SegmentInfo *si = (SegmentInfo*)&segmentInfoList.at(i);
        // check connections to start of segment
        int startConnects = 0;
        int startSegment=-1;
        int endConnects =0;
        int endSegment=-1;

        foreach(SegmentInfo si2, segmentInfoList)
        {
            if(si->segmentId == si2.segmentId)  // ignore self
                continue;
            if(si->oneWay == "N")
            {
                // Only check connections to start if a twoway segment
                if(si->oneWay == "N" && si2.oneWay =="N") // only twoway can connect start to start!
                {
                    double dist = Distance(si->startLat, si->startLon, si2.startLat, si2.startLon);

                    if(dist < .020)
                    {
                        if(startConnects == 0)
                            startSegment = si2.segmentId;
                        startConnects++;
                    }
                }
                if((si->oneWay == "N" && si2.oneWay =="N") || si2.oneWay == "Y")
                {
                    double dist = Distance(si->startLat, si->startLon, si2.endLat, si2.endLon);

                    if(dist < .020 )
                    {
                        if(startConnects == 0)
                            startSegment = si2.segmentId;
                        startConnects++;
                    }
                }
            }
            // Now check connections to end
            if((si->oneWay == "N" && si2.oneWay =="N") || si2.oneWay == "Y")
            {
                double dist = Distance(si->endLat, si->endLon, si2.startLat, si2.startLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                        endSegment = si2.segmentId;
                    endConnects++;
                }
            }
            if(si->oneWay == "N" && si2.oneWay =="N") // only twoway can connect end to end
            {
                double dist = Distance(si->endLat, si->endLon, si2.endLat, si2.endLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                        endSegment = si2.segmentId;
                    endConnects++;
                }
            }
        }
        if(startConnects == 1)
            si->prev = startSegment;
        if(endConnects == 1)
            si->next = endSegment;
        if(startConnects == 0 && endConnects == 0)
        {
            segmentsNotConnected++;
            qDebug()<< "segment "+ QString("%1").arg(si->segmentId)+ " not connected to another!";
        }
        if(startConnects > 1 || endConnects > 1)
        {
            segmentsMultiplyConnected ++;
            qDebug()<< "segment "+ QString("%1").arg(si->segmentId)+ " connects to multiple segments!";
        }
    }
    if(segmentsNotConnected > 1 || segmentsMultiplyConnected > 0)
        return false;
    return true;
}

/// <summary>
/// Get all route segment info with dup segments
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="date"></param>
/// <returns>array of segmentinfo</returns>
QList<SegmentInfo> SQL::getRouteSegmentsInOrder2(qint32 route, QString name, QString date)
{
 QList<SegmentInfo> myArray;
 SegmentInfo si = SegmentInfo();
 qint32 currSegment = -1, segmentId = -1;
 double startLat = 0, startLon = 0, endLat = 0, endLon = 0;
 QString oneWay = "";
// try
// {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  bool firstTry = true;
  while (true)
  {
   QString  commandText;
   if(firstTry)
    commandText = "Select c.startLat, c.startLon, c.endLat, c.endLon, c.segmentId, c.description, c.oneWay, "
                  "b.direction, b.next, b.prev, b.normalEnter, b.normalLeave, b.reverseEnter, b.reverseLeave,"
                  " b.startDate, b.endDate, c.length, c.tracks, c.pointArray, b.OneWay, b.TrackUsage, c.type"
                  " from Routes b join Segments c on c.segmentId = LineKey"
                  " where b.Route = " + QString("%1").arg(route) + " and Name = '" + name + "'"
                  " and '" + date + "' between b.StartDate and b.endDate"
                  /*" and b.endDate <= '"+ date + "'*/" order by b.startDate, b.endDate, c.segmentid";
   else
    commandText = "Select c.startLat, c.startLon, c.endLat, c.endLon, c.segmentId, c.description, c.oneWay,"
                  " b.direction, b.next, b.prev, b.normalEnter, b.normalLeave, b.reverseEnter, b.reverseLeave"
                  " b.startDate, b.endDate, c.length, c.tracks, c.pointArray, b.OneWay, b.TrackUsage, c.type"
                  " from Routes b join Segments c on c.segmentId = LineKey"
                  " where b.Route = " + QString("%1").arg(route) + " and Name = '" + name + "'"
                  "  order by b.startDate, b.endDate, c.segmentid";
   // Note: 1st Query fails if route has a single segment
   QSqlQuery query = QSqlQuery(db);
   bool bQuery = query.exec(commandText);
   bool bSequenceInfoPresent = true;
   if(!bQuery)
   {
    SQLERROR(query);
    if(!firstTry)
       return myArray;
    firstTry = false;
    continue;
   }
   qDebug() << "getRouteSegmentsInOrder2: " << commandText << " rows:" << query.size();

   if (!query.isActive())
   {
    if(!firstTry)
       return myArray;
    firstTry = false;
    continue;

   }
   //                myArray = new LatLng[myReader.RecordsAffected];

   while (query.next())
   {
    startLat = query.value(0).toDouble();
    startLon = query.value(1).toDouble();
    endLat = query.value(2).toDouble();
    endLon = query.value(3).toDouble();
    segmentId = query.value(4).toInt();
    oneWay = query.value(6).toString(); // Note OneWay in Segment!
 //   if (currSegment != segmentId)
 //   {
 //    if (currSegment != -1)
 //    {
 //     si.bearing = Bearing(si.startLat, si.startLon, si.endLat, si.endLon);
 //     if (oneWay == "Y")
 //      si.direction = si.bearing.strDirection();
 //     else
 //      si.direction = si.bearing.strDirection() + "-" + si.bearing.strReverseDirection();
 //     myArray.append(si);
      si = SegmentInfo();
 //    }
     si.segmentId = currSegment = segmentId;
     si.startLat = startLat;
     si.startLon = startLon;
     si.bearingStart = Bearing(startLat, startLon, endLat, endLon);
     si.description = query.value(5).toString();
 //    si.oneWay = query.value(6).toString();
     si.next = query.value(8).toInt();
     si.prev = query.value(9).toInt();
     si.normalEnter = query.value(10).toInt();
     si.normalLeave = query.value(11).toInt();
     si.reverseEnter = query.value(12).toInt();
     si.reverseLeave = query.value(13).toInt();
     si.startDate = query.value(14).toDateTime().toString("yyyy/MM/dd");
     si.endDate = query.value(15).toDateTime().toString("yyyy/MM/dd");
 //   }
    si.endLat = endLat;
    si.endLon = endLon;
    si.bearingEnd = Bearing(startLat, startLon, endLat, endLon);
    si.length = query.value(16).toDouble();

    si.tracks = query.value(17).toInt();
    si.checkTracks();
    si.setPoints(query.value(18).toString());
    si.checkTracks();
    si.oneWay = query.value(19).toString();
    si.trackUsage = query.value(20).toString();
    si.routeType = (RouteType)query.value(21).toInt();
 //  }
 //  if (currSegment != -1)
 //  {
       si.bearing = Bearing(si.startLat, si.startLon, si.endLat, si.endLon);
 //      si.direction = si.bearing.strDirection();
       if (si.oneWay == "Y")
           si.direction = (si.bearing.strDirection());
       else
           si.direction = (si.bearing.strDirection() + "-" + si.bearing.strReverseDirection());
       if(si.next == -1 && si.prev == -1)
           bSequenceInfoPresent=false;
       myArray.append(si);
   }
   break;
  }
// }
// catch (Exception& e)
// {
//     myExceptionHandler(e);
// }
 //checkConnectingSegments(myArray);
 return myArray;
}

// List routes using a segment on a given date
QList<RouteData> SQL::getRouteSegmentsForDate(qint32 route, QString name, QString date)
{
 QList<RouteData> myArray;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT a.Route,Name,StartDate,EndDate,LineKey,companyKey,tractionType,direction, normalEnter, normalLeave, reverseEnter, reverseLeave, routeAlpha from Routes a join altRoute c on a.route = c.route where a.Route = " + QString("%1").arg(route) + " and '" + date + "' between startDate and endDate and TRIM(name) = '" + name + "'";

 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return myArray;
 }
 //                myArray = new LatLng[myReader.RecordsAffected];

 while (query.next())
 {
  RouteData rd = RouteData();
  rd.route = query.value(0).toInt();
  rd.name = query.value(1).toString();
  rd.startDate = query.value(2).toDate();
  rd.endDate = query.value(3).toDate();
  rd.lineKey = query.value(4).toInt();
  rd.companyKey = query.value(5).toInt();
  rd.tractionType = query.value(6).toInt();
  rd.direction = query.value(7).toString();
  rd.normalEnter = query.value(8).toInt();
  rd.normalLeave = query.value(9).toInt();
  rd.reverseEnter =query.value(10).toInt();
  rd.reverseLeave = query.value(11).toInt();
  rd.alphaRoute =query.value(12).toString();
  myArray.append(rd);
 }

 return myArray;
}

QList<RouteData> SQL::getRouteSegmentsForDate(int segmentId, QString date)
{
 QList<RouteData> myArray;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT a.Route,Name,StartDate,EndDate,LineKey,companyKey,tractionType,direction, normalEnter, normalLeave, reverseEnter, reverseLeave, routeAlpha from Routes a join altRoute c on a.route = c.route where '" + date + "' between startDate and endDate and lineKey = "+QString::number(segmentId);

 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return myArray;
 }
 //                myArray = new LatLng[myReader.RecordsAffected];

 while (query.next())
{
 RouteData rd = RouteData();
 rd.route = query.value(0).toInt();
 rd.name = query.value(1).toString();
 rd.startDate = query.value(2).toDate();
 rd.endDate = query.value(3).toDate();
 rd.lineKey = query.value(4).toInt();
 rd.companyKey = query.value(5).toInt();
 rd.tractionType = query.value(6).toInt();
 rd.direction = query.value(7).toString();
 rd.normalEnter = query.value(8).toInt();
 rd.normalLeave = query.value(9).toInt();
 rd.reverseEnter =query.value(10).toInt();
 rd.reverseLeave = query.value(11).toInt();
 rd.alphaRoute =query.value(12).toString();
 myArray.append(rd);
 }

 return myArray;
}

/// <summary>
/// return a list of routes using a segment as of a specific date
/// </summary>
/// <param name="segmentid"></param>
/// <param name="date"></param>
/// <returns></returns>
QList<RouteData> SQL::getRoutes(qint32 segmentid, QString date )
{
    RouteData rd;
    QList<RouteData> myArray = QList<RouteData>();

    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select a.route, name, startdate, endDate, companyKey, tractionType, routeAlpha from Routes a "
            "join altRoute b on a.route = b.route where lineKey = " + QString("%1").arg(segmentid) +
            " and '" + date + "' between startDate and endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return myArray;
        }
        while (query.next())
        {
            rd = RouteData();
            rd.route = query.value(0).toInt();
            rd.name = query.value(1).toString();
            rd.startDate = query.value(2).toDate();
            rd.endDate = query.value(3).toDate();
            rd.companyKey = query.value(4).toInt();
            rd.tractionType = query.value(5).toInt();
            rd.alphaRoute = query.value(6).toString();
            myArray.append(rd);
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);

    }

    return myArray;
}
/// <summary>
/// Get the array of points for a segment
/// </summary>
/// <param name="SegmentId">SegmentId to be retrieved</param>
/// <returns></returns>
//QList<LatLng>  SQL::GetSegmentPoints(qint32 SegmentId)
//{
//    QList<LatLng> myArray =  QList<LatLng>();
//    double endLat = 0, endLon = 0;
//    double startLat =0, startLon =0;
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();

////        QString commandText = "Select StartLat, StartLon, EndLat, EndLon from LineSegment where SegmentId = " + QString("%1").arg(SegmentId) + " order by sequence";
//        QString commandText = "Select StartLat, StartLon, EndLat, EndLon, pointArray from Segments where SegmentId = "+ QString("%1").arg(SegmentId);
//        QSqlQuery query = QSqlQuery(db);
//        bool bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            SQLERROR(query);
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        if (!query.isActive())
//        {
//            return myArray;
//        }
////                myArray = new LatLng[myReader.RecordsAffected];
//        while (query.next())
//        {
////            myArray.append( LatLng(query.value(0).toDouble(), query.value(1).toDouble()));
//         startLat = query.value(0).toDouble();
//         startLon = query.value(1).toDouble();
//         endLat = query.value(2).toDouble();
//         endLon = query.value(3).toDouble();
//         QString points = query.value(4).toString();
//         QStringList sl = points.split(",");

//         if(!(sl.count() &0x01))  // must be even # of points!
//         for(int i = 0; i < sl.count(); i+=2)
//         {
//          myArray.append(LatLng(sl.at(i).toDouble(),sl.at(i+1).toDouble()));
//         }

//        }
////        myArray.append(  LatLng(endLat, endLon));
//    }
//    catch (Exception e)
//    {
//        //myExceptionHandler(e);
//    }
//    return myArray;
//}
#if 0
QList<segmentData> SQL::getIntersectingSegments(double lat, double lon, double radius, RouteType type)
{
    QList<segmentData> myArray;
    segmentData sd =  segmentData();
    double startLat=0, startLon=0, endLat=0, endLon = 0;
    qint32 segmentId=0, sequence = 0, key=0;
    double distance = 0, length =0;
    qint32 currSegment = -1;
    double curSegmentDistance = radius+1;
    QString streetName = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName from LineSegment a join Segments b on a.segmentId = b.segmentId where b.type = " + QString("%1").arg(type )+ " and (distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId, sequence";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        while(query.next())
        {
            segmentId = query.value(0).toInt();
            startLat = query.value(1).toDouble();
            startLon = query.value(2).toDouble();
            endLat = query.value(3).toDouble();
            endLon = query.value(4).toDouble();
            key = query.value(5).toInt();
            sequence = query.value(6).toInt();
            length = query.value(7).toDouble();
            streetName = query.value(8).toString();

            if (segmentId != currSegment)
            {
                if (currSegment > 0 && curSegmentDistance < radius)
                {
                    myArray.append(sd);
                }

                sd =  segmentData();
                currSegment = segmentId;
                curSegmentDistance = radius + 1.0;
            }

            distance = Distance(lat, lon, startLat, startLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = type;
                sd.whichEnd = "S";
            }
            // check the ending point
            distance = Distance(lat, lon, endLat, endLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence + 1;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = (RouteType)type;
                sd.whichEnd = "E";
            }
        }


        if(curSegmentDistance < radius)
            myArray.append(sd);

    }
    catch (Exception e)
    {
        //myExceptionHandler(e);

    }

    return myArray;
}
#else
QList<SegmentData> SQL::getIntersectingSegments(double lat, double lon, double radius, RouteType type)
{
 QList<SegmentData> myArray;
 SegmentData sd =  SegmentData();
 double startLat=0, startLon=0, endLat=0, endLon = 0;
 qint32 segmentId=0, sequence = 0;
 double distance = 0, length =0;
 qint32 currSegment = -1;
 double curSegmentDistance = radius+1;
 QString streetName = "", description="", oneWay="", pointArray="" ;
 QString typeWhere = "type="+QString::number(type);
 int tracks = 0;
 if(type == Surface || type == SurfacePRW)
  typeWhere = "type in(0,1)";
 if(type == RapidTransit || type == Subway)
  typeWhere = "type in(2,3)";
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();

  QString commandText;
  if(config->currConnection->servertype() != "MsSql")
   commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.length, a.street, " \
        "a.description, a.OneWay, a.pointArray, a.tracks, a.type " \
              "from Segments a " \
              "where "+typeWhere + " and (distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") " \
              "order by a.segmentId";
        else
            commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon,"
              " a.length, a.street, a.description, a.OneWay, a.pointArray, a.tracks, a.type from Segments a "
              "where "+ typeWhere + " and (dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by a.segmentId";
  //qDebug() << commandText + "\n";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
   SQLERROR(query);
   db.close();
   //exit(EXIT_FAILURE);
  }

  while(query.next())
  {
   segmentId = query.value(0).toInt();
   startLat = query.value(1).toDouble();
   startLon = query.value(2).toDouble();
   endLat = query.value(3).toDouble();
   endLon = query.value(4).toDouble();
   length = query.value(5).toDouble();
   streetName = query.value(6).toString();
   description = query.value(7).toString();
   oneWay = query.value(8).toString();
   pointArray = query.value(9).toString();
   tracks = query.value(10).toInt();
   type = (RouteType)query.value(11).toInt();
   if (segmentId != currSegment)
   {
    if (currSegment > 0 && curSegmentDistance < radius)
    {
        myArray.append(sd);
    }

    sd =  SegmentData();
    currSegment = segmentId;
    curSegmentDistance = radius + 1.0;
   }

   distance = Distance(lat, lon, startLat, startLon);
   if (distance < curSegmentDistance)
   {
    sd._segmentId = segmentId;
    sd._startLat = startLat;
    sd._startLon = startLon;
    sd._endLat = endLat;
    sd._endLon = endLon;
    sd._length = distance;
    curSegmentDistance = distance;
    sd._streetName = streetName;
    sd._routeType = type;
    sd._whichEnd = "S";
    sd._description = description;
    sd._bearing = Bearing(lat, lon, startLat, startLon);
    sd._oneWay = oneWay;
    sd._tracks = tracks;
    sd._routeType = type;
   }

   // check the ending point
   distance = Distance(lat, lon, endLat, endLon);
   if (distance < curSegmentDistance)
   {
    sd._segmentId = segmentId;
    sd._startLat = startLat;
    sd._startLon = startLon;
    sd._endLat = endLat;
    sd._endLon = endLon;
    sd._length = distance;
    curSegmentDistance = distance;
    sd._streetName = streetName;
    sd._routeType = (RouteType)type;
    sd._whichEnd = "E";
    sd._description = description;
    sd._bearing = Bearing(lat, lon, endLat, endLon);
    sd._oneWay = oneWay;
    sd._tracks = tracks;
    sd._routeType = type;
   }
   sd.setPoints(pointArray);
  }

  if(curSegmentDistance < radius)
      myArray.append(sd);
 }
 catch (Exception e)
 {
     //myExceptionHandler(e);
 }

 return myArray;
}

#endif
#if 0
QList<segmentData> SQL::getIntersectingSegmentsWithRoute(double lat, double lon, double radius, RouteType type)
{
    QList<segmentData> myArray;
    segmentData sd =  segmentData();
    double startLat=0, startLon=0, endLat=0, endLon = 0;
    qint32 segmentId=0, sequence = 0, key=0;
    qint32 route = -1;
    double distance = 0, length =0;
    qint32 currSegment = -1;
    double curSegmentDistance = radius+1;
    QString streetName = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select b.segmentId, b.startLat, b.startLon, b.endLat, a.EndLon, a.`key`, a.sequence, a.length, a.streetName, r.route, ar.routeAlpha from LineSegment a join Segments b on a.segmentId = b.segmentId AND ((a.startLat = b.startLat and a.startLon= b.startLon) OR (a.startLat =  b.endLat and a.startLon = b.endLon) OR (a.endLat = b.endLat and a.endLon = b.endLon) OR (a.endLat = b.startLat and a.endLon = b.startLon)) join Routes r on r.linekey = b.segmentId JOIN altRoute ar ON ar.route = r.route where (" + QString("%1").arg((int)type )+ "= -1 OR b.type = " + QString("%1").arg((int)type )+ ") and (distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", b.startLat, b.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", b.endLat, b.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by r.route, segmentId, sequence";
        else
            commandText = "select b.segmentId, b.startLat, b.startLon, b.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName, r.route, ar.routeAlpha from LineSegment a join Segments b on a.segmentId = b.segmentId AND ((a.startLat = b.startLat and a.startLon= b.startLon) OR (a.startLat =  b.endLat and a.startLon = b.endLon) OR (a.endLat = b.endLat and a.endLon = b.endLon) OR (a.endLat = b.startLat and a.endLon = b.startLon)) join Routes r on r.linekey = b.segmentId JOIN altRoute ar ON ar.route = r.route where (" + QString("%1").arg((int)type )+ "= -1 OR b.type = " + QString("%1").arg((int)type )+ ") and (dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", b.startLat, b.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", b.endLat, b.endLon) < " + QString("%1").arg(radius,0,'f',8) + ")  order by r.route, segmentId, sequence";
        qDebug() << commandText + "\n";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        while(query.next())
        {
            segmentId = query.value(0).toInt();
            startLat = query.value(1).toDouble();
            startLon = query.value(2).toDouble();
            endLat = query.value(3).toDouble();
            endLon = query.value(4).toDouble();
            key = query.value(5).toInt();
            sequence = query.value(6).toInt();
            length = query.value(7).toDouble();
            streetName = query.value(8).toString();
            route = query.value(9).toInt();

            if (segmentId != currSegment)
            {
                if (currSegment > 0 && curSegmentDistance < radius)
                {
                    myArray.append(sd);
                }

                sd =  segmentData();
                currSegment = segmentId;
                curSegmentDistance = radius + 1.0;
            }

            distance = Distance(lat, lon, startLat, startLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = type;
                sd.whichEnd = "S";
                sd.route = route;
                sd.endSegment = key;
                sd.alphaRoute = query.value(10).toString();
            }
            // check the ending point
            distance = Distance(lat, lon, endLat, endLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence + 1;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = (RouteType)type;
                sd.whichEnd = "E";
                sd.route = route;
                sd.endSegment = key;
                sd.alphaRoute = query.value(10).toString();
            }
        }


        if(curSegmentDistance < radius)
            myArray.append(sd);

    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return myArray;
}
#endif

#if 0
QList<segmentData> SQL::getIntersectingSegments(double lat, double lon, double radius)
{
    QList<segmentData> myArray;
    segmentData sd =  segmentData();
    double startLat = 0, startLon = 0, endLat = 0, endLon = 0;
    qint32 segmentId = 0, sequence = 0, key = 0;
    double distance = 0, length=0;
    qint32 currSegment = -1;
    double curSegmentDistance = radius + 1;
    QString streetName = "";
    RouteType type = Other;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName, b.type from LineSegment a join Segments b on a.segmentId = b.segmentId where (distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId, sequence";
        else
        commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName, b.type from LineSegment a join Segments b on a.segmentId = b.segmentId where (dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId, sequence";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            segmentId = query.value(0).toInt();
            startLat = query.value(1).toDouble();
            startLon = query.value(2).toDouble();
            endLat = query.value(3).toDouble();
            endLon = query.value(4).toDouble();
            key = query.value(5).toInt();
            sequence = query.value(6).toInt();
            length = query.value(7).toDouble();
            streetName = query.value(8).toString();
            type = (RouteType)query.value(9).toInt();

            if (segmentId != currSegment)
            {
                if (currSegment > 0 && curSegmentDistance < radius)
                {
                    myArray.append(sd);
                }

                sd =  segmentData();
                currSegment = segmentId;
                curSegmentDistance = radius + 1.0;
            }

            distance = Distance(lat, lon, startLat, startLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = type;
                sd.whichEnd = "S";
            }
            // check the ending point
            distance = Distance(lat, lon, endLat, endLon);
            if (distance < curSegmentDistance)
            {
                sd.key = key;
                sd.SegmentId = segmentId;
                sd.startLat = startLat;
                sd.startLon = startLon;
                sd.endLat = endLat;
                sd.endLon = endLon;
                sd.sequence = sequence + 1;
                sd.distance = distance;
                curSegmentDistance = distance;
                sd.streetName = streetName;
                sd.routeType = (RouteType)type;
                sd.whichEnd = "E";
            }
        }


        if (curSegmentDistance < radius)
            myArray.append(sd);

    }
    catch (Exception e)
    {
        //myExceptionHandler(e);

    }

    return myArray;
}
#endif
// Return a list of all segments that have their starting or ending location within the stated radius
QList<SegmentData> SQL::getIntersectingSegments(double lat, double lon, double radius)
{
 QList<SegmentData> myArray;
 SegmentData sd =  SegmentData();
 double startLat = 0, startLon = 0, endLat = 0, endLon = 0;
 qint32 segmentId = 0;
 double distance = 0, length =0;
 qint32 currSegment = -1;
 double curSegmentDistance = radius + 1;
 QString streetName = "", description="", oneWay="", pointArray = "";
 RouteType type = Other;
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();

  QString commandText;
  //if(config->currConnection->servertype() != "MsSql")
      commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.length, a.streetName, a.type, a.description,  a.oneWay, a.pointArray from Segments a where (distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId";
//  else
//  commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName, b.type from LineSegment a join Segments b on a.segmentId = b.segmentId where (dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId, sequence";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
   SQLERROR(query);
   db.close();
   exit(EXIT_FAILURE);
  }
  while (query.next())
  {
   segmentId = query.value(0).toInt();
   startLat = query.value(1).toDouble();
   startLon = query.value(2).toDouble();
   endLat = query.value(3).toDouble();
   endLon = query.value(4).toDouble();
   length = query.value(5).toDouble();
   streetName = query.value(6).toString();
   type = (RouteType)query.value(7).toInt();
   description = query.value(8).toString();
   oneWay = query.value(9).toString();
   pointArray = query.value(10).toString();

   if (segmentId != currSegment)
   {
    if (currSegment > 0 && curSegmentDistance < radius)
    {
        myArray.append(sd);
    }

    sd =  SegmentData();
    currSegment = segmentId;
    curSegmentDistance = radius + 1.0;
   }

   distance = Distance(lat, lon, startLat, startLon);
   sd._segmentId = segmentId;
   sd._startLat = startLat;
   sd._startLon = startLon;
   sd._endLat = endLat;
   sd._endLon = endLon;
   sd._streetName = streetName;
   sd._routeType = type;
   if (distance < curSegmentDistance)
   {
    curSegmentDistance = distance;
    sd._length = distance;
    sd._whichEnd = "S";
   }
   // check the ending point
   distance = Distance(lat, lon, endLat, endLon);
   if (distance < curSegmentDistance)
   {
    curSegmentDistance = distance;
    sd._length = distance;
    sd._whichEnd = "E";
   }
   sd._description = description;
   sd._oneWay = oneWay;
   sd.setPoints(pointArray);
  }

  if (curSegmentDistance < radius)
   myArray.append(sd);
 }
 catch (Exception e)
 {
  //myExceptionHandler(e);
 }

 return myArray;
}

/// <summary>
/// Get all the segments at a point that intersect for a route
/// </summary>
/// <param name="lat"></param>
/// <param name="lon"></param>
/// <param name="radius"></param>
/// <param name="route"></param>
/// <param name="routeName"></param>
/// <param name="date"></param>
/// <returns></returns>
QList<SegmentInfo> SQL::getIntersectingRouteSegmentsAtPoint(double lat, double lon, double radius, qint32 route,
                                                            QString routeName, QString date)
{
 QList<SegmentInfo> myArray = QList<SegmentInfo>();
// double startLat = 0, startLon = 0, endLat = 0, endLon = 0;
// double lastStartLat = 0, lastStartLon = 0;
// qint32 segmentId = 0;
// qint32 currSegment = -1;
// QString direction = "";
// QString description = "";
// QString oneWay = "";
 double distanceToStart = 0, distanceToEnd = 0;

 QSqlDatabase db = QSqlDatabase::database();
//    QString commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, c.direction, b.description, b.oneWay,  c.next, c.prev, c.normalEnter, c.normalLeave, c.reverseEnter, c.reverseLeave from LineSegment a join Segments b on b.segmentId = a.SegmentId join Routes c on c.lineKey = a.segmentId where c.route = " + QString("%1").arg(route) + " and c.name = '" + routeName + "' and '" + date + "' between c.startDate and c.endDate order by a.segmentId, a.sequence";
 QString commandText = "select b.segmentId, b.startLat, b.startLon, b.endLat, b.EndLon, c.direction, b.description,"
                       " b.oneWay,  c.next, c.prev, c.normalEnter, c.normalLeave, c.reverseEnter, c.reverseLeave,"
                       " b.pointArray, b.tracks"
                       " from Segments b join Routes c on c.lineKey = b.segmentId"
                       " where c.route = " + QString("%1").arg(route)
                       + " and c.name = '" + routeName
                       + "' and '" + date + "' between c.startDate and c.endDate"
                       " order by b.segmentId";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }

 while(query.next())
 {
  SegmentInfo si = SegmentInfo();

  si.segmentId = query.value(0).toInt();
  si.startLat = query.value(1).toDouble();
  si.startLon = query.value(2).toDouble();
  si.endLat = query.value(3).toDouble();
  si.endLon = query.value(4).toDouble();
  si.direction = query.value(5).toString();
  si.description = query.value(6).toString();
  si.oneWay = query.value(7).toString();
  si.tracks = query.value(15).toInt();
  si.checkTracks();
  si.setPoints(query.value(14).toString());
  si.next = query.value(8).toInt();
  si.prev = query.value(9).toInt();
  si.normalEnter = query.value(10).toInt();
  si.normalLeave = query.value(11).toInt();
  si.reverseEnter = query.value(12).toInt();
  si.reverseLeave = query.value(13).toInt();
  if(si.pointList.count() >=2)
  {
   si.bearingStart = Bearing(si.startLat, si.startLon, si.pointList.at(1).lat(), si.pointList.at(1).lon());
   si.bearingEnd = Bearing(si.pointList.at(si.points-2).lat(), si.pointList.at(si.points-2).lon(), si.endLat, si.endLon);
   si.bearing = Bearing(si.startLat, si.startLon, si.endLat, si.endLon);
   si.direction = si.bearing.strDirection();
  }
  distanceToEnd = Distance(lat, lon, si.endLat, si.endLon);
  distanceToStart = Distance(lat, lon, si.startLat, si.startLon);
  if (distanceToEnd < distanceToStart)
      si.whichEnd = "E";
  else
      si.whichEnd = "S";
  if (distanceToEnd < radius || distanceToStart < radius)
  myArray.append(si);
 }
 return myArray;
}

int SQL::sequenceRouteSegments(qint32 segmentId, QList<SegmentInfo> segmentList, qint32 route, QString name, QString date)
{
 QList<SegmentInfo> intersects;
 qint32 endSegment = -1;
 double dToBegin = 0, dToEnd=0;
 double diff;
 qint32 matchedSegment=-1;
 qint32 currSegment = segmentId;
 SegmentInfo six = SegmentInfo();
 six.segmentId = -1;
 SegmentInfo* si = &six;
 qint32 sequence = 0, reverseSeq = 0;
 bool bForward = true;
 double nextLat = 0, nextLon = 0;
 bool bOutbound = true;
 double a1 = 0, a2 = 0;
 bool bfirstSegment = true;

 if(!dbOpen())
  throw Exception(tr("database not open: %1").arg(__LINE__));

 //foreach (segmentInfo si0 in segmentList)
 for(int i = 0; i < segmentList.count(); i ++)
 {
  SegmentInfo* si0 = (SegmentInfo*)&segmentList.at(i);
  si0->sequence = -1;
  si0->returnSeq = -1;
 }

 while (currSegment >= 0 )//&& sequence < segmentList.Count && reverseSeq < segmentList.Count)
 {
  for(int i = 0; i < segmentList.count(); i ++)
  {
   SegmentInfo* sii = (SegmentInfo*)&segmentList.at(i);
   if(sii->segmentId == currSegment)
   {
    si = sii;
    if(si->segmentId == 454)
     qDebug() << "break";

    if (bOutbound)
    {
     if (si->sequence != -1)
     {
      //currSegment = -1;
      //Console.WriteLine("Possible endless loop at segment: " + si->SegmentId + " and segment: " + sii->SegmentId);
      //break;
      bOutbound = !bOutbound;
      si->returnSeq = reverseSeq++;
     }
     else
      si->sequence = sequence++;
    }
    else
    {
     if (si->returnSeq != -1)
     {
      currSegment = -1;
      qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(si->segmentId) + " and segment: " + QString("%1").arg(sii->segmentId);
      break;
     }
     si->returnSeq = reverseSeq++;
    }

    break;
   }
  }
  if (currSegment == -1)
   break;

  if (bfirstSegment && si->oneWay=="N")
  {
   intersects = getIntersectingRouteSegmentsAtPoint(si->startLat, si->startLon, .020, route, name, date);
   if ( intersects.count() <= 1)
   {
    bForward = true;
    nextLat = si->endLat;
    nextLon = si->endLon;
    si->whichEnd = "E";
   }
   else
   {
    bForward = false;
    nextLat = si->startLat;
    nextLon = si->startLon;
    si->whichEnd = "S";
   }
  }
  else
  {
   if (bForward)
   {
    nextLat = si->endLat;
    nextLon = si->endLon;
    si->whichEnd = "E";
   }
   else
   {
    nextLat = si->startLat;
    nextLon = si->startLon;
    si->whichEnd = "S";
   }
  }
  bfirstSegment = false;

//  if (si->segmentId == 8)
//  {
//  }
  intersects = getIntersectingRouteSegmentsAtPoint(nextLat, nextLon, .020, route, name, date);
  for(int i = 0; i < intersects.count(); i ++)
  {
   SegmentInfo* sij = (SegmentInfo*)&intersects.at(i);
   if (sij->segmentId == si->segmentId)
    continue; // Ignore myself
   if (sij->oneWay == "Y" && sij->whichEnd == "E")
    continue;

   dToBegin = Distance(nextLat, nextLon, sij->startLat, sij->startLon);
   dToEnd = Distance(nextLat, nextLon, sij->endLat, sij->endLon);
   if (dToBegin > .020 && dToEnd > .020)
    continue;   // only match to a begin points
   matchedSegment = -1;
   if (sij->whichEnd != si->whichEnd)
   {
    // meeting start to end
    if (si->whichEnd == "S")
     a1 = si->bearingStart.getBearing();
    else
     a1 = si->bearingEnd.getBearing();
    if (sij->whichEnd == "S")
     a2 = sij->bearingStart.getBearing();
    else
     a2 = sij->bearingEnd.getBearing();

    diff = angleDiff(a1, a2);
    //diff = angleDiff(si->bearingEnd.getBearing, sij->bearingStart.getBearing);
    //if (bOutbound)
    {
     switch (bForward ? si->normalLeave : si->reverseLeave)
     {
     case 1:     // leave to left
      if (diff >= -135 && diff <= -45)
       //sii.next = sij->SegmentId;
       matchedSegment = sij->segmentId;
      break;
     case 0:     // leave ahead
      if (diff >= -45 && diff <= +45)
       //sii.next = sij.SegmentId;
       matchedSegment = sij->segmentId;
      // if (diff >= -225 && diff <= -135)
      //    matchedSegment = sij.SegmentId;
      break;
     case 2: // leave to right
      if (diff >= 45 && diff <= 135)
       //sii.next = sij.SegmentId;
       matchedSegment = sij->segmentId;
      break;
     }
    }
   }
   else
   {
    if (si->whichEnd == "S")
     a1 = si->bearingStart.getBearing();
    else
     a1 = si->bearingEnd.getBearing();
    if (sij->whichEnd == "S")
     a2 = sij->bearingStart.getBearing();
    else
     a2 = sij->bearingEnd.getBearing();
    diff = angleDiff(a1, a2);

    //if (si->whichEnd == "E")
    //    diff = -diff;
    //diff = angleDiff(si->bearingEnd.getBearing, sij.bearingStart.getBearing);
    //if (bOutbound)
    {
     switch (bForward ? si->normalLeave : si->reverseLeave)
     {
     case 2:     // leave to left
      if (diff >= -135 && diff <= -45)
       //sii.next = sij.SegmentId;
       matchedSegment = sij->segmentId;
      break;
     case 0:     // leave ahead
      if (diff >= -45 && diff <= +45)
       //sii.next = sij.SegmentId;
       matchedSegment = sij->segmentId;
      if(diff >= -225 && diff <= -135)
       matchedSegment = sij->segmentId;
      if(diff >= 125 && diff <= 225)
       matchedSegment = sij->segmentId;
      break;
     case 1: // leave to right
      if (diff >= 45 && diff <= 135)
       //sii.next = sij.SegmentId;
       matchedSegment = sij->segmentId;
      break;
     }
    }

    if (matchedSegment >= 0)
     bForward = !bForward;
    //else
    //    if(intersects.Count == 2)
    //        matchedSegment = matchedSegment;
   }

   if (matchedSegment >= 0)
   {
    if (bOutbound)
    {
     //if (Distance(si->startLat, si->startLon, si->endLat, si->endLon) < .020)
     //    si->next = -1;
     //else
     si->next = matchedSegment;
    }
    else
     si->prev = matchedSegment;
    currSegment = matchedSegment;
    if (currSegment == segmentId)
    {
     // Back at the beginning.
     for(int i = 0; i < segmentList.count(); i ++)
     {
      SegmentInfo* sii = (SegmentInfo*)&segmentList.at(i);
      if (sii->segmentId == currSegment)
      {
       si = sii;

       if (bOutbound)
       {
        if (si->sequence != -1)
        {
         currSegment = -1;
         qDebug() << "Possible endless loop at segment: " + QString("%1").arg(si->segmentId) + "  " + si->description;
         break;
        }
        si->sequence = sequence++;
       }
       else
       {
        if (si->returnSeq != -1)
        {
         currSegment = -1;
         qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(si->segmentId) + "  " + si->description;
         break;
        }
        si->returnSeq = reverseSeq++;
       }

       break;
      }
     }
     currSegment = -1;   // back at the beginning
    }

    break;
   }
   continue;
  }
  QString text = "";

  if (matchedSegment == -1)
  {
   text ="No segment matching for segment " + QString("%1").arg(si->segmentId) + " " + si->description;
   for(int i = 0; i < segmentList.count(); i ++)
   {
    SegmentInfo* sik = (SegmentInfo*)&segmentList.at(i);
    if (sik->segmentId == si->segmentId)
     continue; // Ignore myself

    dToBegin = Distance(nextLat, nextLon, sik->startLat, sik->startLon);
    dToEnd = Distance(nextLat, nextLon, sik->endLat, sik->endLon);
    if (dToBegin > .020 && dToEnd > .020)
     continue;   // only match to a begin points
    if (sik->whichEnd != si->whichEnd)
    {
     if (si->whichEnd == "S")
      a1 = si->bearingStart.getBearing();
     else
      a1 = si->bearingEnd.getBearing();
     if (sik->whichEnd == "S")
      a2 = sik->bearingStart.getBearing();
     else
      a2 = sik->bearingEnd.getBearing();

     diff = angleDiff(a1, a2);
    }
    else
    {
     if (si->whichEnd == "S")
      a1 = si->bearingStart.getBearing();
     else
      a1 = si->bearingEnd.getBearing();
     if (sik->whichEnd == "S")
      a2 = sik->bearingStart.getBearing();
     else
      a2 = sik->bearingEnd.getBearing();
     diff = angleDiff(a1, a2);
    }
    text = text + "\n possible choices were segment " + QString("%1").arg(sik->segmentId) + " angle: " + QString("%1").arg(diff) + "° " + sik->description;
   }
   if(text != "")
   {
    qDebug() << text;
    QMessageBox* box = new QMessageBox(QMessageBox::Information, tr("Segment connection error"), tr("The following segment has an error connecting to the next segment"));
    box->setInformativeText(text);
    box->exec();
   }
   if(segmentList.count()==1)
   {
    matchedSegment = segmentList.at(0).segmentId;
    if(bOutbound)
    {
     si->next = matchedSegment;
     if (si->sequence != -1)
     {
      currSegment = -1;
      qDebug() << "Possible endless loop at segment: " + QString("%1").arg(si->segmentId) + "  " + si->description;
      break;
     }
    }
    else
     si->prev = matchedSegment;
    if (si->returnSeq != -1)
    {
     currSegment = -1;
     qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(si->segmentId) + "  " + si->description;
     break;
    }
    si->returnSeq = reverseSeq++;
   }
  }
  if (bOutbound)
  {
   if (si->next == -1  || (intersects.count() == 1 && intersects.at(0).segmentId == si->segmentId))
   {
    bOutbound = !bOutbound;
    bForward = !bForward;
    bfirstSegment = true;
   }
  }
  else
   if (si->prev == -1)
   {
    currSegment = -1;
   }
 }
 return endSegment;
}

/// <summary>
/// return the number of points in a segment.
/// </summary>
/// <param name="segmentId"></param>
/// <returns></returns>
//qint32 SQL::getNbrPoints(qint32 segmentId)
//{
//    int points=0;
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();

//        QString commandText = "select points from Segments where segmentid = " + QString("%1").arg(segmentId);
//        QSqlQuery query = QSqlQuery(db);
//        bool bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        if (!query.isActive())
//        {
//            return 0;
//        }
//        //                myArray = new LatLng[myReader.RecordsAffected];

//        while (query.next())
//        {
//            points= query.value(0).toInt();
//        }
//    }
//    catch (Exception e)
//    {
//        //myExceptionHandler(e);
//    }
//    return points;
//}
#if 0
#region routes
public ArrayList updateLikeRoutes(int segmentid, int route, string name, string date)
{
    ArrayList intersectList = new ArrayList();
    routeIntersects ri = new routeIntersects();
    segmentInfo si = getSegmentInfo(segmentid);  // get some info about the segment.
    ArrayList  myArray = new ArrayList());
    compareSegmentDataClass compareSegments = new compareSegmentDataClass();

    // get a list of all the routes using this segment on this date
    ArrayList segmentInfoList = getRouteSegmentsBySegment(segmentid);

    // get other segments intersecting with the start of this segment
    myArray = getIntersectingSegments(si.startLat, si.startLon, .020, si.routeType);
    foreach (segmentData sd1 in myArray)
    {
        if (sd1.SegmentId == segmentid)
            continue;   // ignore myself
        // select only those segments actually used.
        if (isRouteUsedOnDate(route, sd1.SegmentId, date))
            ri.startIntersectingSegments.Add(sd1);
    }
    ri.startIntersectingSegments.Sort(compareSegments);

    // get other segments intersecting with the end of this segment
    myArray = getIntersectingSegments(si.endLat, si.endLon, .020, si.routeType);
    foreach (segmentData sd2 in myArray)
    {
        if (sd2.SegmentId == segmentid)
            continue;   // ignore myself
        // select only those segments actually used
        if (isRouteUsedOnDate(route, sd2.SegmentId, date))
            ri.endIntersectingSegments.Add(sd2);
    }
    ri.endIntersectingSegments.Sort(compareSegments);

    // Populate the intersect list
    foreach (routeData rdi in segmentInfoList)
    {
        // Get my info
        if (rdi.route == route && rdi.name == name && formatDate(rdi.endDate) == date)
            ri.rd = rdi;
        else
        {
            routeIntersects ri2 = new routeIntersects();
            ri2.rd = rdi;
            myArray = getIntersectingSegments(si.startLat, si.startLon, .020, si.routeType);
            foreach (segmentData sd1 in myArray)
            {
                foreach (segmentData mysd1 in ri.startIntersectingSegments)
                {
                    if (sd1.SegmentId == mysd1.SegmentId )
                    {
                        // select only those segments actually used.
                        if (isRouteUsedOnDate(rdi.route, sd1.SegmentId, formatDate(rdi.endDate)))
                            ri2.startIntersectingSegments.Add(sd1);
                    }
                }
            }
            myArray = getIntersectingSegments(si.endLat, si.endLon, .020, si.routeType);
            foreach(segmentData sd2 in myArray)
            {
                foreach (segmentData mysd2 in ri.endIntersectingSegments)
                {
                    if (sd2.SegmentId == mysd2.SegmentId )
                    {
                        // select only those segments actually used
                        if (isRouteUsedOnDate(rdi.route, sd2.SegmentId, formatDate(rdi.endDate)))
                            ri2.endIntersectingSegments.Add(sd2);
                    }
                }
            }
            ri2.startIntersectingSegments.Sort(compareSegments);
            ri2.endIntersectingSegments.Sort(compareSegments);
            if(ri2.endIntersectingSegments.Count == ri.endIntersectingSegments.Count &&
                ri2.startIntersectingSegments.Count == ri.startIntersectingSegments.Count)
            intersectList.Add(ri2);
        }
    }
    return intersectList;
}
#endif
double SQL::angleDiff(double A1, double A2)
{
    double difference = A2 - A1;
    while (difference < -180) difference += 360;
    //while (difference < -90) difference += 180;
    while (difference > 180) difference -= 360;
    //while (difference > 90) difference -= 180;
    return difference;

}
double /*static*/ SQL::distance(LatLng latlng1, LatLng latlng2)
{
 if (latlng1 == latlng2 )
     return 0;
 double R = 6371; // RADIUS OF THE EARTH IN KM
 double dToRad = 0.0174532925;
 double lat1 = latlng1.lat() * dToRad;
 //double lon1 = Lon1 * dToRad;
 double lat2 = latlng2.lat() * dToRad;
 //double lon2 = Lon2 * dToRad;
 double dLat = dToRad * (latlng2.lat() - latlng1.lat());
 double dLon = dToRad * (latlng2.lon() - latlng1.lon());
 double a = qSin(dLat / 2) * qSin(dLat / 2)
     + qCos(lat1) * qCos(lat2)
     * qSin(dLon / 2) * qSin(dLon / 2);
 double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
 double d = R * c;
 return d; // distance in kilometers
}
/// <summary>
/// Calculate distance in km between two points
/// </summary>
/// <param name="Lat1">starting latitude</param>
/// <param name="Lon1">starting longitude</param>
/// <param name="Lat2">ending latitude</param>
/// <param name="Lon2">ending longitude</param>
/// <returns>distance in km</returns>
double SQL::Distance(double Lat1, double Lon1, double Lat2, double Lon2)
{// Calculate distance and bearing http://www.movable-type.co.uk/scripts/latlong.html
    if (Lat1 == Lat2 && Lon1 == Lon2)
        return 0;
    double R = 6371; // RADIUS OF THE EARTH IN KM
    double dToRad = 0.0174532925;
    double lat1 = Lat1 * dToRad;
    //double lon1 = Lon1 * dToRad;
    double lat2 = Lat2 * dToRad;
    //double lon2 = Lon2 * dToRad;
    double dLat = dToRad * (Lat2 - Lat1);
    double dLon = dToRad * (Lon2 - Lon1);
    double a = qSin(dLat / 2) * qSin(dLat / 2)
        + qCos(lat1) * qCos(lat2)
        * qSin(dLon / 2) * qSin(dLon / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    double d = R * c;
    return d; // distance in kilometers
}

bool SQL::updateSegmentDescription(qint32 SegmentId, QString description, QString oneWay, int tracks, double length)
{
 qint32 rows = 0;

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString street;
 int ix = description.indexOf(",");
 if(ix >= 0)
 {
  street = description.mid(0, ix);
 }

 QString commandText = "update Segments set description = '" + description +
   "', oneWay = '" + oneWay + "', tracks="+QString::number(tracks) +
   ", street = '" +street +
   "', length=" +QString::number(length,'g',8)+
   ", lastUpdate=:lastUpdate where SegmentId = " + QString("%1").arg(SegmentId);
 QSqlQuery query = QSqlQuery(db);
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bool bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  QSqlError err = query.lastError();
  QMessageBox::critical(NULL, "Error", "A fatal SQL error has occured:\n" + err.text() + "\n"+query.lastQuery() + " line:" + QString("%1").arg(__LINE__));
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
    return false;
 }

 return true;
}

// <summary>
/// Add a new point to the LineSegment table
/// </summary>
/// <param name="pt"></param>
/// <param name="SegmentId"></param>
/// <param name="BeginLat"></param>
/// <param name="BeginLon"></param>
/// <param name="EndLat"></param>
/// <param name="EndLon"></param>
/// <param name="length"></param>
/// <param name="StreetName"></param>
/// <returns></returns>
//bool SQL::addPoint( qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon,  double EndLat, double EndLon,  QString StreetName) // deprecated
//{
//    bool ret = false;
//    int rows = 0;

//    double distance = Distance(BeginLat, BeginLon,  EndLat, EndLon);
//    // commandText = "INSERT INTO LineSegment( StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId, Sequence, Length)  VALUES( 38.6326955063775, -90.2344036113843, 38.6319663570569, -90.2345430862531, 'South Grand', 1, 2, 0)"
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();

//        BeginTransaction("addPoint");

        
//        QString commandText = "INSERT INTO LineSegment( StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId,Sequence, Length)  VALUES( " + QString("%1").arg(BeginLat,0,'f',8) + ", " + QString("%1").arg(BeginLon) + ", " + QString("%1").arg(EndLat,0,'f',8) + ", " + QString("%1").arg(EndLon,0,'f',8) + ", '" + StreetName + "', " + QString("%1").arg(SegmentId) + ", " + QString("%1").arg(pt) + ", " + QString("%1").arg(distance) + ")";
//        QSqlQuery query = QSqlQuery(db);
//        bool bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        rows = query.numRowsAffected();
//        if (rows == 0)
//        {
//            ret = false;
//        }

//        ret = true;
//        CommitTransaction("addPoint");

//    }

//    catch (Exception e)
//    {
//        //myExceptionHandler(e);
//    }

//    return ret;
//}
/// <summary>
/// Move a point. Update the end location of the previous segment if not the 1st and update
/// the begin location of the specified segment. 
/// </summary>
/// <param name="pt"></param>
/// <param name="SegmentId"></param>
/// <param name="BeginLat"></param>
/// <param name="BeginLon"></param>
/// <returns></returns>
//bool SQL::movePoint(qint32 pt, qint32 SegmentId, double BeginLat, double BeginLon)
//{
// bool ret = false;
// int rows = 0;
// double startLat = 0, startLon = 0;
// double distance = 0, oldDistance =0;
// double EndLat = 0, EndLon = 0;
// QString strStartLat ="", strStartLon = "";
// int count;

// try
// {
//  if(!dbOpen())
//      throw Exception(tr("database not open: %1").arg(__LINE__));
//  QSqlDatabase db = QSqlDatabase::database();
//  QString commandText;

//  // Get the number of points
//  commandText = "select count(*) from LineSegment where segmentId = " + QString::number(SegmentId);
//  QSqlQuery query = QSqlQuery(db);
//  bool bQuery = query.exec(commandText);
//  if(!bQuery)
//  {
//      QSqlError err = query.lastError();
//      qDebug() << err.text() + "\n";
//      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//      db.close();
//      exit(EXIT_FAILURE);
//  }
//  if (!query.isActive())
//  {
//      return false;
//  }
//  while (query.next())
//  {
//      count = query.value(0).toInt();
//  }
//  if (count > 0)
//      ret = true;
//  BeginTransaction("movePoint");

//  if (pt > 0  && (pt != count))
//  {
//   // Retrieve the current start location for pt-1 to calculate the length
//   commandText = "Select StartLat, StartLon, length from LineSegment where SegmentId = " + QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg((pt-1));
//   QSqlQuery query = QSqlQuery(db);
//   bool bQuery = query.exec(commandText);
//   if(!bQuery)
//   {
//       QSqlError err = query.lastError();
//       qDebug() << err.text() + "\n";
//       qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//       db.close();
//       exit(EXIT_FAILURE);
//   }
//   if (!query.isActive())
//   {
//       RollbackTransaction("movePoint");
//       return ret;
//   }
//   while (query.next())
//   {
//       startLat = query.value(0).toDouble();
//       strStartLat = query.value(0).toString();
//       startLon = query.value(1).toDouble();
//       strStartLon =query.value(1).toString();
//       oldDistance = query.value(2).toDouble();
//   }

//   // update the ending location and distance  for the previous segment
//   distance = Distance(startLat, startLon, BeginLat, BeginLon);
//   commandText = "UPDATE LineSegment SET  EndLat= " + QString("%1").arg(BeginLat,0,'f',8) + ", EndLon= " + QString("%1").arg(BeginLon,0,'f',8) + ", length= " +QString("%1").arg( distance) + ",lastUpdate = :lastUpdate WHERE sequence = " + QString("%1").arg((pt - 1)) + " and SegmentId = " + QString("%1").arg(SegmentId);
//   query.prepare(commandText);
//   query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//   bQuery = query.exec();
//   if(!bQuery)
//   {
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//    db.close();
//    exit(EXIT_FAILURE);
//   }
//   rows = query.numRowsAffected();
//   if (rows == 0)
//   {
//    if(db.driver()->hasFeature(QSqlDriver::QuerySize))
//        qDebug()<<"driver supports query size";
//    else
//        qDebug()<<"driver does not support query size";
//    //RollbackTransaction("movePoint");
//    //
//    //return ret;
//    //throw(new ApplicationException("movePoint: previous segment not found.\n" + commandText));
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << "movePoint: previous segment not found.\n" + commandText;
//    //db.close();
//    //exit(EXIT_FAILURE);
//   }
//  }

//  if(pt < count)
//  {
//   // Retrieve the current end location for pt.
//   commandText = "Select EndLat, EndLon, length from LineSegment where SegmentId = " +
//       QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg(pt) ;
//   query = QSqlQuery(db);
//   bQuery = query.exec(commandText);
//   if(!bQuery)
//   {
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//    db.close();
//    exit(EXIT_FAILURE);
//   }
//   if (!query.isActive())
//   {
//    CommitTransaction("movePoint");
//    return true;
//   }
// //  if(!query.next())
// //  {
// //   CommitTransaction("movePoint");
// //   return true;
// //  }
//   while (query.next())
//   {
//    EndLat = query.value(0).toDouble();
//    EndLon = query.value(1).toDouble();
//    oldDistance = query.value(2).toDouble();
//   }

//   // update the starting location for the current segment
//   distance = Distance(EndLat, EndLon, BeginLat, BeginLon);
//   commandText = "UPDATE LineSegment SET  StartLat= " +QString("%1").arg( BeginLat,0,'f',8) + ", StartLon= " + QString("%1").arg(BeginLon,0,'f',8) + ", length= " + QString("%1").arg(distance) + ",lastUpdate =:lastUpdate WHERE sequence = " + QString("%1").arg(pt)  + " and SegmentId = " + QString("%1").arg(SegmentId); // ACK 2/7/16
//   query.prepare(commandText);
//   query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//   bQuery = query.exec();
//   if(!bQuery)
//   {
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//    db.close();
//    exit(EXIT_FAILURE);
//   }
//   rows = query.numRowsAffected();
//   if (rows == 0)
//   {
//    qDebug()<< "(movePoint) no rows updated: "+ commandText + "\n";
//    if(config->currConnection->servertype() != "MySql")
//    {
//        RollbackTransaction("movePoint");
//        return ret;
//    }
//    // My Sql returns 0 if the values were the same so we must assume that the query was OK!
//    qDebug()<< "(movePoint) assume update was OK!";
//   }
//  }
//  else
//  {
//   // update the ending point
//   // Retrieve the current end location for pt.
//   commandText = "Select StartLat, StartLon, length from LineSegment where SegmentId = " +
//       QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg(pt-1) ;
//   query = QSqlQuery(db);
//   bQuery = query.exec(commandText);
//   if(!bQuery)
//   {
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//    db.close();
//    exit(EXIT_FAILURE);
//   }
//   if (!query.isActive())
//   {
//    CommitTransaction("movePoint");
//    return true;
//   }
// //  if(!query.next())
// //  {
// //   CommitTransaction("movePoint");
// //   return true;
// //  }
//   while (query.next())
//   {
//    startLat = query.value(0).toDouble();
//    startLon = query.value(1).toDouble();
//    oldDistance = query.value(2).toDouble();
//   }

//   // update the starting location for the current segment
//   distance = Distance(startLat, startLon, BeginLat, BeginLon);
//   commandText = "UPDATE LineSegment SET  endLat= " +QString("%1").arg( BeginLat,0,'f',8) + ", endLon= " + QString("%1").arg(BeginLon,0,'f',8) + ", length= " + QString("%1").arg(distance) + ",lastUpdate =:lastUpdate WHERE sequence = " + QString("%1").arg(pt-1)  + " and SegmentId = " + QString("%1").arg(SegmentId); // ACK 2/7/16
//   query.prepare(commandText);
//   query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//   bQuery = query.exec();
//   if(!bQuery)
//   {
//    QSqlError err = query.lastError();
//    qDebug() << err.text() + "\n";
//    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//    db.close();
//    exit(EXIT_FAILURE);
//   }
//   rows = query.numRowsAffected();
//   if (rows == 0)
//   {
//    qDebug()<< "(movePoint) no rows updated: "+ commandText + "\n";
//    if(config->currConnection->servertype() != "MySql")
//    {
//        RollbackTransaction("movePoint");
//        return ret;
//    }
//    // My Sql returns 0 if the values were the same so we must assume that the query was OK!
//    qDebug()<< "(movePoint) assume update was OK!";
//   }

//  }
//  CommitTransaction("movePoint");
//  ret = true;
// }
// catch(Exception e)
// {
//     //myExceptionHandler(e);
// }
// return ret;
//}

bool SQL::updateSegment(SegmentData* sd)
{
 bool ret = false;
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 sd->_length = 0;
 int rows = 0;

 for(int i=0; i < sd->pointList().count(); i++)
 {
  if(i == 0)
  {
   sd->_startLat = sd->pointList().at(0).lat();
   sd->_startLon = sd->pointList().at(0).lon();
  }
  else
  {
   sd->_endLat = sd->pointList().at(i).lat();
   sd->_endLon = sd->pointList().at(i).lon();
   Bearing b(sd->pointList().at(i-1).lat(), sd->pointList().at(i-1).lon(), sd->pointList().at(i).lat(), sd->pointList().at(i).lon());
   sd->_length += b.Distance();
  }
 }
 if(sd->pointList().count() <1)
  return false;
 Q_ASSERT(sd->_startLat != 0);
 Q_ASSERT(sd->_startLon != 0);
 if(sd->pointList().count() > 1)
 {
  Q_ASSERT(sd->_endLat != 0);
  Q_ASSERT(sd->_endLon != 0);
 }
 commandText = "Update Segments set startLat= " + QString("%1").arg(sd->_startLat,0,'f',8)
   + ", startLon= " + QString("%1").arg(sd->_startLon,0,'f',8)
   + ", endLat= " + QString("%1").arg(sd->_endLat,0,'f',8)
   + ", endLon= " + QString("%1").arg(sd->_endLon,0,'f',8)
   + ", length= " + QString("%1").arg(sd->_length)
   + ", points= " + QString("%1").arg(sd->pointList().count())
   + ", direction = '" + sd->_direction + "', "
   + "pointArray='" + sd->pointsString() + "', "
   + "description='" + sd->_description + "',"
   + "street='" + sd->_streetName + "',"
//   + "oneWay='" + sd->oneWay + "',"
   + "tracks="+ QString::number(sd->_tracks) + ","
   + "type="+QString::number((int)sd->_routeType) + ","
   + "startDate='" + sd->_startDate.toString("yyyy/MM/dd") + "', "
   + "endDate='" + sd->_endDate.toString("yyyy/MM/dd") + "', "
   + "lastUpdate=:lastUpdate "
   + "where SegmentId = " + QString("%1").arg(sd->segmentId());
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bQuery = query.exec();
 if(!bQuery)
 {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      db.close();
      exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
  return ret;
 }
 ret = true;
 emit segmentsChanged(sd->segmentId());

 return ret;
}

bool SQL::updateSegment(SegmentInfo* si)
{
 bool ret = false;
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 si->length = 0;
 int rows = 0;

 for(int i=0; i < si->pointList.count(); i++)
 {
  if(i == 0)
  {
   si->startLat = si->pointList.at(0).lat();
   si->startLon = si->pointList.at(0).lon();
  }
  else
  {
   si->endLat = si->pointList.at(i).lat();
   si->endLon = si->pointList.at(i).lon();
   Bearing b(si->pointList.at(i-1).lat(), si->pointList.at(i-1).lon(), si->pointList.at(i).lat(), si->pointList.at(i).lon());
   si->length += b.Distance();
  }
 }
 if(si->pointList.count() <1)
  return false;
 Q_ASSERT(si->startLat != 0);
 Q_ASSERT(si->startLon != 0);
 if(si->pointList.count() > 1)
 {
  Q_ASSERT(si->endLat != 0);
  Q_ASSERT(si->endLon != 0);
 }
 commandText = "Update Segments set startLat= " + QString("%1").arg(si->startLat,0,'f',8)
   + ", startLon= " + QString("%1").arg(si->startLon,0,'f',8)
   + ", endLat= " + QString("%1").arg(si->endLat,0,'f',8)
   + ", endLon= " + QString("%1").arg(si->endLon,0,'f',8)
   + ", length= " + QString("%1").arg(si->length)
   + ", points= " + QString("%1").arg(si->pointList.count())
   + ", direction = '" + si->direction + "', "
   + "pointArray='" + si->pointsString() + "', "
   + "description='" + si->description + "',"
   + "street='" + si->streetName + "',"
//   + "oneWay='" + si->oneWay + "',"
   + "tracks="+ QString::number(si->tracks) + ","
   + "type="+QString::number((int)si->routeType) + ","
   + "startDate='" + si->startDate + "', "
   + "endDate='" + si->endDate + "', "
//   + "trackUsage = '" + si->trackUsage + "', "
   + "lastUpdate=:lastUpdate "
   + "where SegmentId = " + QString("%1").arg(si->segmentId);
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bQuery = query.exec();
 if(!bQuery)
 {
      QSqlError err = query.lastError();
      qDebug() << err.text() + "\n";
      qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      db.close();
      exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
  return ret;
 }
 ret = true;
 emit segmentsChanged(si->segmentId);

 return ret;
}

/// <summary>
/// Update the startLat, startLon, endLat & endLon for a segment by scanning all the line segments.
/// </summary>
/// <param name="SegmentId"></param>
/// <returns></returns>
bool SQL::updateSegment(qint32 SegmentId)
{
 if(SegmentId <= 0)
  //throw IllegalArgumentException(tr("invalid segmentId %1").arg(SegmentId));
  return false;
 bool ret = false;
 double startLat=0, startLon=0, endLat=0, endLon=0, length=0;
 int rows=0, points=0;
 QString direction = "";
 QString oneWay = "";
 SegmentInfo si;
 si.segmentId = SegmentId;

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 BeginTransaction("UpdateSegment");

 QString commandText = "Select startLat, startLon, endLat, endLon, length, pointArray, oneWay, tracks, street from Segments where SegmentId = " + QString("%1").arg(SegmentId);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  RollbackTransaction("UpdateSegment");
  return true;
 }

 while (query.next())
 {
  startLat = query.value(0).toDouble();
  startLon = query.value(1).toDouble();

  endLat = query.value(2).toDouble();
  endLon = query.value(3).toDouble();
  length += query.value(4).toDouble();

  QString pointArray = query.value(5).toString();
  si.setPoints(pointArray);
  si.oneWay = query.value(5).toString();
  si.tracks = query.value(6).toInt();
  si.checkTracks();
  si.streetName = query.value(7).toString();
 }

//        commandText = "select oneWay from Segments where segmentId = " + QString("%1").arg(SegmentId);
//        bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        while (query.next())
//        {
//            oneWay = query.value(0).toString();
//        }

 // get the last ending point and sum the lengths
 Bearing b = Bearing(si.startLat, si.startLon, si.endLat, si.endLon);
 if (oneWay == "N")
     si.direction = b.strDirection() + "-" + b.strReverseDirection();
 else
     si.direction = b.strDirection();
 if(points < 2)
 {
  RollbackTransaction("UpdateSegment");
  return false;
 }

 Q_ASSERT(si.startLat != 0);
 Q_ASSERT(si.startLon != 0);
 if(si.pointList.count() >1)
 {
  Q_ASSERT(si.endLat != 0);
  Q_ASSERT(si.endLon != 0);
 }
 commandText = "Update Segments set startLat= " + QString("%1").arg(si.startLat,0,'f',8) + ", startLon= " +
     QString("%1").arg(si.startLon,0,'f',8) + ", endLat= " + QString("%1").arg(si.endLat,0,'f',8) + ", endLon= " + QString("%1").arg(si.endLon,0,'f',8) + ", length= " + QString("%1").arg(si.length) +
   ", points= " + QString("%1").arg(si.points) + ", direction = '" + si.direction + "', tracks="+ QString::number(si.tracks)+ ", pointArray='" + si.pointsString() + "', lastUpdate=:lastUpdate " +
     "where SegmentId = " + QString("%1").arg(SegmentId);
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bQuery = query.exec();
 if(!bQuery)
 {
   SQLERROR(query);
   db.close();
   exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
   CommitTransaction("UpdateSegment");
   return ret;
 }
 ret = true;
 CommitTransaction("UpdateSegment");
 emit segmentsChanged(SegmentId);

 return ret;
}

StationInfo SQL::getStationInfo(qint32 stationKey)
{
 StationInfo sti = StationInfo();

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT stationKey, a.name, latitude, longitude, a.SegmentId, infoKey, startDate, endDate, geodb_loc_id, routeType, a.route, markerType from Stations a left join altRoute r on r.route = a.route where stationKey = " + QString("%1").arg(stationKey);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     SQLERROR(query);
    return StationInfo();
 }
 if (!query.isActive())
 {
     return sti;
 }
 while (query.next())
 {
  sti.stationKey = query.value(0).toInt();
  sti.stationName = query.value(1).toString();
  sti.latitude = query.value(2).toDouble();
  sti.longitude = query.value(3).toDouble();
  sti.segmentId = query.value(4).toInt();
  if (!query.value(5).isNull())
      sti.infoKey = query.value(5).toInt();
  else
      sti.infoKey = -1;
  if (query.value(6).isNull())
      sti.startDate = QDateTime();
  else
      sti.startDate = query.value(6).toDateTime();
  if (query.value(7).isNull())
      sti.endDate = QDateTime::fromString("3000-01-01", "yyyy/MM/dd");
  else
      sti.endDate = query.value(7).toDateTime();
  if (!query.value(8).isNull())
      sti.geodb_loc_id = query.value(8).toInt();
  sti.routeType = (RouteType)query.value(9).toInt();
  sti.route = query.value(10).toInt();
  sti.markerType = query.value(11).toString();
 }
 return sti;
}
//TODO this query may return multiple rows!
StationInfo SQL::getStationInfo(QString name)
{
 StationInfo sti =  StationInfo();

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT stationKey, a.name, latitude, longitude, a.SegmentId, infoKey, startDate, endDate, markerType  "
                       "from Stations a where name = '" + name + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     SQLERROR(query);
     return sti;
 }
 if (!query.isActive())
 {
     return sti;
 }
 while (query.next())
 {
  sti.stationKey = query.value(0).toInt();
  sti.stationName = query.value(1).toString();
  sti.latitude = query.value(2).toDouble();
  sti.longitude = query.value(3).toDouble();
  sti.lineSegmentId = query.value(4).toInt();
  if (!query.value(5).isNull())
      sti.infoKey = query.value(5).toInt();
  else
      sti.infoKey = -1;
  if (query.value(6).isNull())
      sti.startDate = QDateTime();
  else
      sti.startDate = query.value(6).toDateTime();
  if (query.value(7).isNull())
      sti.endDate = QDateTime::fromString("3000-01-01", "yyyy/MM/dd");
  else
      sti.endDate = query.value(7).toDateTime();
  sti.markerType = query.value(8).toString();

 }
 return sti;
}

bool SQL::updateStation(qint32 stationKey, qint32 infoKey)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateStation");

        QString commandText = "update Stations set infoKey = " + QString("%1").arg(infoKey) + ",lastUpdate=:lastUpdate " +
            "where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if(rows > 0)
        {
            CommitTransaction("updateStation");
            ret = true;
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateStationRoute(qint32 stationKey, qint32 route) // not used
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateStation");

        QString commandText = "update Stations set route = " + QString("%1").arg(route) + " " +
            "where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if(rows > 0)
        {
            CommitTransaction("updateStation");
            ret = true;
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateStation(qint32 stationKey, LatLng latLng)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateStation");

        QString commandText = "update Stations set latitude = " + QString("%1").arg(latLng.lat(),0,'f',8) + ", longitude = " + QString("%1").arg(latLng.lon(),0,'f',8) + ",lastUpdate=:lastUpdate where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
        {
            CommitTransaction("updateStation");
            ret = true;
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateStation(qint32 stationKey, LatLng latLng, qint32 segmentId)
{
    bool ret = false;
    int rows = 0;
    //int lineSegment=-1;
//    try
//    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateStation");

        QString commandText;
        QSqlQuery query = QSqlQuery(db);
#if 0
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select `key`, startLat, startLon, endLat, endLon from LineSegment " \
                "where segmentId = "+QString("%1").arg(segmentId) + \
                    " AND (distance(startLat, startLon,"+ QString("%1").arg(latLng.lat(),0, 'f',8)+","+ QString("%1").arg(latLng.lon(),0, 'f',8)+ ") < .020 OR distance(endLat, endLon,"+ QString("%1").arg(latLng.lat(),0, 'f',8)+","+ QString("%1").arg(latLng.lon(),0, 'f',8)+ ") < .020)";
        else
            commandText = "select [key], startLat, startLon, endLat, endLon from LineSegment " \
                "where segmentId = "+QString("%1").arg(segmentId) + \
                    " AND (distance(startLat, startLon,"+ QString("%1").arg(latLng.lat(),0, 'f',8)+","+ QString("%1").arg(latLng.lon(),0, 'f',8)+ ") < .020 OR distance(endLat, endLon,"+ QString("%1").arg(latLng.lat(),0, 'f',8)+","+ QString("%1").arg(latLng.lon(),0, 'f',8)+ ") < .020)";
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if(!query.isActive())
        {
            qDebug()<< "line segment not found!\n";
            exit(EXIT_FAILURE);

        }
        while(query.next())
        {
            lineSegment = query.value(0).toInt();
        }
#endif
        commandText = "update Stations set latitude = " + QString("%1").arg(latLng.lat(),0,'f',8) + ", longitude = " + QString("%1").arg(latLng.lon(),0,'f',8) + \
                ", SegmentId ="+ QString("%1").arg(segmentId) + \
                ", lastUpdate=:lastUpdate where stationKey = " + QString("%1").arg(stationKey);
        query.prepare(commandText);
        query.bindValue(":lastUpdate",QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            SQLERROR(query);
            return false;
        }
        rows = query.numRowsAffected();
        if (rows > 0)
        {
            CommitTransaction("updateStation");
            ret = true;
        }
//    }
//    catch (Exception e)
//    {
//        //myExceptionHandler(e);
//    }
    return ret;
}

bool SQL::updateStation(qint32 stationKey,  qint32 route, qint32 lineSegmentId, qint32 segmentId, QString startDate, QString endDate, qint32 *newStationId, int point)
{
    Q_UNUSED(segmentId)
    bool ret = false;
    int rows = 0;
    int dbroute = -1;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateStation");

        QString commandText = "select route from Stations where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        while(query.next())
        {
            dbroute = query.value(0).toInt();
        }
        if(route == dbroute || dbroute == 0)
        {
            //commandText = "update Stations set route = " + QString("%1").arg(route) + ",lastUpdate=:lastUpdate where stationKey = " + QString("%1").arg(stationKey);
            commandText = QString("update Stations set route = %1, segmentId = %2, point = %3, lastUpdate= :lastUpdate where stationKey = %4").arg(route).arg(segmentId).arg(point).arg(stationKey);
            query.prepare(commandText);
            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
            bQuery = query.exec();
            if(!bQuery)
            {
                SQLERROR(query);
                db.close();
                exit(EXIT_FAILURE);
            }
            rows = query.numRowsAffected();
            if (rows >= 0)
            {
                CommitTransaction("updateStation");
                ret = true;
            }
        }
        else
        {
            // we need to add a new station record for a different route
            StationInfo sti = getStationInfo(stationKey);
            if(sti.route < 1)
            {
                RollbackTransaction("updateStation");
                return false;
            }
            *(newStationId) = addStation(sti.stationName,LatLng(sti.latitude, sti.longitude),lineSegmentId, startDate, endDate, sti.geodb_loc_id, sti.infoKey, sti.routeType, sti.markerType, point);
            if(*(newStationId) < 0)
            {
                RollbackTransaction("updateStation");
                return false;
            }
            CommitTransaction("updateStation");
        }

    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
/// <summary>
/// Update all stations with the same geodb_loc_id
/// </summary>
/// <param name="geodb_loc_id"></param>
/// <param name="stationKey"></param>
/// <param name="latLng"></param>
/// <returns></returns>
bool SQL::updateStation(qint32 geodb_loc_id, qint32 stationKey, LatLng latLng)
{
    Q_UNUSED(stationKey)
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "update Stations set latitude = " + QString("%1").arg(latLng.lat(),0,'f',8) + ", longitude = " + QString("%1").arg(latLng.lon(),0,'f',8) + ",lastUpdate=:lastUpdate where geodb_loc_id = " + QString("%1").arg(geodb_loc_id);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
bool SQL::updateStationLineSegment(qint32 route, qint32 lineSegmentId, LatLng pt)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "update Stations set lineSegmentId = " + QString("%1").arg(lineSegmentId) + ", latitude = " + QString("%1").arg(pt.lat(),0,'f', 8) + ",longitude = " + QString("%1").arg(pt.lon(),0, 'f',8) + ",lastUpdate=:lastUpdate where route = " + QString("%1").arg(route);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
bool SQL::updateStationLineSegment(qint32 geodb_loc_id, qint32 route, qint32 lineSegmentId, LatLng pt)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "update Stations set lineSegmentId = " + QString("%1").arg(lineSegmentId) + ", latitude = "+QString("%1").arg(pt.lat(),0,'f',8) + ",longitude = " + QString("%1").arg(pt.lon(),0,'f',8)  +",lastUpdate=:lastUpdate where geodb_loc_id = " + QString("%1").arg(geodb_loc_id)+ " and route ="+QString("%1").arg(route);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateStation(qint32 stationKey, qint32 infoKey, QString name, qint32 segmentId, QString startDate, QString endDate, QString markerType)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "update Stations set infoKey = " + QString("%1").arg(infoKey) + ", name ='" + name + "', segmentId =" + QString("%1").arg(segmentId)+ ", startDate='" + startDate + "',endDate='" + endDate + "',markerType='" + markerType +"',lastUpdate=:lastUpdate where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            SQLERROR(query);
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
StationInfo SQL::getStationAtPoint(LatLng pt)
{
    StationInfo sti =  StationInfo();

    try
    {

        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "SELECT stationKey, a.name, latitude, longitude, a.segmentId, infoKey,  a.startDate, a.endDate  from Stations a where latitude = " + QString("%1").arg(pt.lat(),0,'f',8) + " and longitude = " + QString("%1").arg(pt.lon(),0,'f',8);
        else
            commandText = "SELECT stationKey, a.name, latitude, longitude, a.segmentId, infoKey,  a.startDate, a.endDate  from Stations a where latitude = " + QString("%1").arg(pt.lat(),0,'f',8) + " and longitude = " + QString("%1").arg(pt.lon(),0,'f',8);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            sti.stationKey = query.value(0).toInt();
            sti.stationName = query.value(1).toString();
            sti.latitude = query.value(2).toDouble();
            sti.longitude = query.value(3).toDouble();
            sti.segmentId = query.value(4).toInt();
            if (!query.value(5).isNull())
                sti.infoKey = query.value(5).toInt();
            else
                sti.infoKey = -1;
            if (query.value(6).isNull())
                sti.startDate = QDateTime();
            else
                sti.startDate = query.value(6).toDateTime();
            if (query.value(7).isNull())
                sti.endDate = QDateTime(QDate(3000,1,1));
            else
                sti.endDate = query.value(7).toDateTime();

        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return sti;
}

QList<StationInfo> SQL::getStationsLikeName(QString name)
{
 QList<StationInfo> list;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "SELECT stationKey, a.name, latitude, longitude, a.SegmentId, infoKey, startDate, endDate, markerType  from Stations a where lower(name) like '%" + name.toLower() + "%' COLLATE NOCASE";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     SQLERROR(query);
    return list;
 }
 if (!query.isActive())
 {
     return list;
 }
 while (query.next())
 {
  StationInfo sti;
     sti.stationKey = query.value(0).toInt();
     sti.stationName = query.value(1).toString();
     sti.latitude = query.value(2).toDouble();
     sti.longitude = query.value(3).toDouble();
     sti.lineSegmentId = query.value(4).toInt();
     if (!query.value(5).isNull())
         sti.infoKey = query.value(5).toInt();
     else
         sti.infoKey = -1;
     if (query.value(6).isNull())
         sti.startDate = QDateTime();
     else
         sti.startDate = query.value(6).toDateTime();
     if (query.value(7).isNull())
         sti.endDate = QDateTime::fromString("3000-01-01", "yyyy/MM/dd");
     else
         sti.endDate = query.value(7).toDateTime();
     if (!query.value(8).isNull())
         sti.geodb_loc_id = query.value(8).toInt();
     sti.routeType = (RouteType)query.value(9).toInt();
     sti.route = query.value(10).toInt();
     if(query.value(11).isNull())
         sti.alphaRoute = "";
     else
         sti.alphaRoute = query.value(11).toString();
     sti.markerType = query.value(11).toString();
     list.append(sti);
 }
 return list;
}

LatLng SQL::getPointOnSegment(qint32 pt, qint32 segmentId)
{
    LatLng latLng =  LatLng();
    int points = -1;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QString commandText = "select endLat, endLon, points from Segments where segmentId = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {

            double lat = query.value(0).toDouble();
            double lon = query.value(1).toDouble();
            latLng =  LatLng(lat,lon);
            points = query.value(2).toInt();
        }
        if (points > pt)
        {
            QString commandText = "select startLat, startLon from LineSegment where sequence = " + QString("%1").arg(pt) + " and segmentId = " + QString("%1").arg(segmentId);
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            while (query.next())
            {
                latLng =  LatLng( query.value(0).toDouble(),  query.value(1).toDouble());
            }
        }
    }
    catch (Exception e)
    {
       myExceptionHandler(e);
    }
    return latLng;
}

//bool SQL::insertPoint(qint32 pt, qint32 SegmentId, double newLat, double newLon)
//{
//    qint32 lineSegmentKey = -1;
//    return insertPoint(pt, SegmentId, newLat, newLon, &lineSegmentKey);
//}
/// <summary>
/// Add a new point after segment 'pt'. Update segment pt's end location with the new location. 
/// Increment the sequence value of all records after pt's sequence.
/// Create a new segment (pt +1) with the new location specified as the begin location and the end location the 
/// former ending location of pt. 
/// </summary>
/// <param name="pt"></param>
/// <param name="SegmentId"></param>
/// <param name="newLat"></param>
/// <param name="?"></param>
/// <returns></returns>
//bool SQL::insertPoint(qint32 pt, qint32 segmentId, double newLat, double newLon/*,  qint32* lineSegmentKey*/)
//{
//    bool ret = false;
//    double endLat = 0, endLon = 0, startLat=0, startLon = 0;
//    double distance = 0;
//    QString StreetName = "";
////    *lineSegmentKey = 0;
//    int rows = 0;
//    int firstSeq = -1, lastSeq=-1, lineSegments =0;
//    LatLng newLatLng = LatLng(newLat, newLon);
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();
//        BeginTransaction("insertPoint");

//        // Get the current number of points for this segment
//        QString commandText = "select min(sequence), max(sequence), count(*) from LineSegment where SegmentId = " +  QString("%1").arg(segmentId);
//        QSqlQuery query = QSqlQuery(db);
//        bool bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        while(query.next())
//        {
//            firstSeq = query.value(0).toInt();
//            lastSeq = query.value(1).toInt();
//            lineSegments = query.value(2).toInt();
//        }
//        if(lineSegments != (lastSeq + 1))
//        {
//            qDebug()<< "Segment " + QString("%1").arg(segmentId) + " invalid line segment count " +QString("%1").arg(lineSegments);
//            exit(EXIT_FAILURE);
//        }

//        // Retrieve the current ending location for pt.
//        commandText = "Select StartLat, StartLon, EndLat, EndLon, StreetName from LineSegment where SegmentId = " +  QString("%1").arg(segmentId) + " and sequence = " + QString("%1").arg(pt);
//        query = QSqlQuery(db);
//        bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        if(!query.isActive())
//        {
//            //
//            //RollbackTransaction("insertPoint");
//            //
//            //return ret;
//            qDebug() << "insertPoint: segment not found\n" + commandText;
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        while (query.next())
//        {
//            startLat = query.value(0).toDouble();
//            startLon = query.value(1).toDouble();
//            endLat = query.value(2).toDouble();
//            endLon = query.value(3).toDouble();
//            StreetName = query.value(4).toString();
//        }

//        // update the pt record
//        distance = Distance(startLat, startLon, newLat, newLon);
//        commandText = "UPDATE LineSegment SET  EndLat= " + QString("%1").arg(newLat,0,'f',8) + ", EndLon= " + QString("%1").arg(newLon,0,'f',8) + ", length= " + QString("%1").arg(distance) + ",lastUpdate=:lastUpdate WHERE sequence = " + QString("%1").arg(pt) + " and SegmentId = " + QString("%1").arg(segmentId);
//        query = QSqlQuery(db);
//        query.prepare(commandText);
//        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//        bQuery = query.exec();
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        rows = query.numRowsAffected();

//        if (rows == 0)
//        {
//            //RollbackTransaction("insertPoint");
//            //
//            //return ret;
//            qDebug() <<"insertPoint: segment not found\n" + commandText;
//            db.close();
//            exit(EXIT_FAILURE);
//        }

////        // Update the sequence number in the remaining segments
////        commandText = "UPDATE LineSegment SET  sequence= sequence +1 WHERE sequence > " + QString("%1").arg(pt) + " and SegmentId = " + QString("%1").arg(SegmentId);
////        bQuery = query.exec(commandText);
////        if(!bQuery)
////        {
////            QSqlError err = query.lastError();
////            qDebug() << err.text() + "\n";
////            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
////            db.close();
////            exit(EXIT_FAILURE);
////        }
////        rows = query.numRowsAffected();
//        rows = 0;
//        // update backwards to avoid getting a unique key duplicate error.
//        for(int curSeq = lastSeq; curSeq > pt; curSeq--)
//        {
//            commandText = "UPDATE LineSegment SET  sequence= sequence +1,lastUpdate=:lastUpdate WHERE sequence = " + QString("%1").arg(curSeq) + " and SegmentId = " + QString("%1").arg(segmentId);
//            query.prepare(commandText);
//            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//            bQuery = query.exec();
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            rows += query.numRowsAffected();
//        }

//        // Insert the new record with sequence pt +1
//        distance = Distance(newLat, newLon, endLat, endLon);
//        commandText = "INSERT INTO LineSegment( StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId, Sequence, Length)  VALUES( " + QString("%1").arg(newLat,0,'f',8) + ", " + QString("%1").arg(newLon,0,'f',8) + ", " + QString("%1").arg(endLat,0,'f',8) + ", " + QString("%1").arg(endLon,0,'f',8) + ", '" + StreetName + "', " + QString("%1").arg(segmentId) + ", " + QString("%1").arg((pt+1)) + ", " + QString("%1").arg(distance) + ")";
//        bQuery = query.exec(commandText);
//        if(!bQuery)
//        {
//            QSqlError err = query.lastError();
//            qDebug() << err.text() + "\n";
//            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//            db.close();
//            exit(EXIT_FAILURE);
//        }
//        rows = query.numRowsAffected();
//        if (rows == 0)
//        {
//            RollbackTransaction("insertPoint");
//            return ret;
//        }
//        //if (rows == 1)
//        {
//            if(config->currConnection->servertype() == "Sqlite")
//                commandText = "SELECT LAST_INSERT_ROWID()";
//            else
//            if(config->currConnection->servertype() != "MsSql")
//                commandText = "SELECT LAST_INSERT_ID()";
//            else
//                commandText = "SELECT IDENT_CURRENT('stations')";
//            bQuery = query.exec(commandText);
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            while (query.next())
//            {
//                *lineSegmentKey  =query.value(0).toInt();
//            }
//        }

//        CommitTransaction("insertPoint");
//        ret = true;

//    }
//    catch (Exception e)
//    {
//       //myExceptionHandler(e);
//    }
//    return ret;
//}

/// <summary>
/// Delete a segment.
/// </summary>
/// <param name="pt">desired segment</param>
/// <param name="SegmentId"></param>
/// <returns></returns>
//bool SQL::deletePoint(qint32 pt, qint32 SegmentId, qint32 nbrPts)
//{
//    bool ret = false;
//    double endLat = 0, endLon = 0, startLat = 0, startLon = 0;
//    double distance = 0, oldDistance=0;
//    QString StreetName = "";
//    int rows = 0;
//    try
//    {
//        if(!dbOpen())
//            throw Exception(tr("database not open: %1").arg(__LINE__));
//        QSqlDatabase db = QSqlDatabase::database();
//        BeginTransaction("deletePoint");

        
//        if(pt < (nbrPts-1))
//        {
//            // Retrieve the current ending location for pt.
//            QString commandText = "Select EndLat, EndLon, StreetName, length from LineSegment where SegmentId = " +                QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg(pt);
//            QSqlQuery query = QSqlQuery(db);
//            bool bQuery = query.exec(commandText);
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            if(!query.isActive())
//            {
//                //
//                //RollbackTransaction("deletePoint");
//                //
//                //return ret;
//                qDebug() <<"deletePoint: segment not found\n" + commandText;
//                db.close();
//                exit(EXIT_FAILURE);

//            }
//            while (query.next())
//            {
//                endLat = query.value(0).toDouble();
//                endLon = query.value(1).toDouble();
//                StreetName = query.value(2).toString();
//                oldDistance = query.value(3).toDouble();
//            }
//        }
//        if (pt == (nbrPts - 1))
//        {
//            // Delete the last record

//            QString commandText = "Delete from LineSegment where SegmentId = " +
//                QString("%1").arg(SegmentId) + " and sequence = " +QString("%1").arg( (pt-1));
//            QSqlQuery query = QSqlQuery(db);
//            bool bQuery = query.exec(commandText);
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            rows = query.numRowsAffected();
//            if (rows == 0)
//            {
//                //RollbackTransaction("deletePoint");
//                //
//                //return ret;
//                qDebug() << "deletePoint: segment not found\n" + commandText;
//                db.close();
//                exit(EXIT_FAILURE);

//            }
//            CommitTransaction("deletePoint");
//            ret = true;
//            return ret;
//        }
//        else
//        {
//            // Delete the desired record
//            QString commandText = "Delete from LineSegment where SegmentId = " +
//                QString("%1").arg(SegmentId) + " and sequence = " +QString("%1").arg( pt);
//            QSqlQuery query = QSqlQuery(db);
//            bool bQuery = query.exec(commandText);
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            rows = query.numRowsAffected();
//            if (rows == 0)
//            {
//                RollbackTransaction("deletePoint");
//                return ret;
//            }
//            // Update the sequence number in the remaining segments
//            commandText = "UPDATE LineSegment SET  sequence= sequence -1,lastUpdate=:lastUpdate WHERE sequence > " + QString("%1").arg(pt) + " and SegmentId = " + QString("%1").arg(SegmentId);
//            query.prepare(commandText);
//            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//            bQuery = query.exec();
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            rows = query.numRowsAffected();

//            if (pt > 0)
//            {
//                // Retrieve the current starting location for pt-1.
//                commandText = "Select startLat, startLon, StreetName from LineSegment where SegmentId = " +                    QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg((pt - 1));
//                bQuery = query.exec(commandText);
//                if(!bQuery)
//                {
//                    QSqlError err = query.lastError();
//                    qDebug() << err.text() + "\n";
//                    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                    db.close();
//                    exit(EXIT_FAILURE);
//                }
//                if(!query.isActive())
//                {
//                    //RollbackTransaction("deletePoint");
//                    return ret;
//                }
//                while (query.next())
//                {
//                    startLat = query.value(0).toDouble();
//                    startLon = query.value(1).toDouble();
//                    StreetName = query.value(2).toString();
//                }
//            }
//        }
//        if (pt > 0)
//        {
//            // update the starting location  and distance for the previous segment
//            distance = Distance(startLat, startLon, endLat, endLon);
//            QString commandText = "UPDATE LineSegment SET  endLat= " + QString("%1").arg(endLat,0,'f',8) + ", endLon= " + QString("%1").arg(endLon,0,'f',8) + ", length= " + QString("%1").arg(distance) + ",lastUpdate=:lastUpdate WHERE sequence = " + QString("%1").arg((pt - 1)) + " and SegmentId = " + QString("%1").arg(SegmentId);
//            QString("%1").arg(SegmentId) + " and sequence = " +QString("%1").arg( pt);
//            QSqlQuery query = QSqlQuery(db);
//            query.prepare(commandText);
//            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
//            bool bQuery = query.exec();
//            if(!bQuery)
//            {
//                QSqlError err = query.lastError();
//                qDebug() << err.text() + "\n";
//                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//                db.close();
//                exit(EXIT_FAILURE);
//            }
//            rows = query.numRowsAffected();
//            if (rows == 0)
//            {
//                //RollbackTransaction("deletePoint");
//                //
//                //return ret;
//                qDebug() <<"deletePoint: segment not found\n" + commandText;

//            }
//        }


//        CommitTransaction("deletePoint");


//        ret = true;

//    }
//    catch (Exception e)
//    {
//        //myExceptionHandler(e);
//    }

//    return ret;
//}
QString SQL::getSegmentOneWay(qint32 SegmentId)
{
    QString description = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select OneWay from Segments where SegmentId = " + QString("%1").arg(SegmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            description = query.value(0).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return description;
}

bool SQL::doesSegmentExist(QString descr, QString oneWay)
{
    bool ret = false;
    int count = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select count(*) from Segments where description = '" +
            descr + "' and oneWay= '" + oneWay + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return false;
        }
        while (query.next())
        {
            count = query.value(0).toInt();
        }
        if (count > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
QString SQL::getSegmentDescription(qint32 SegmentId)
{
    QString description = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select description from Segments where SegmentId = " + QString("%1").arg(SegmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            description = query.value(0).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return description;
}

/// <summary>
/// Update the street name and length for a line segment
/// </summary>
/// <param name="sd"></param>
/// <returns></returns>
bool SQL::updateRecord(SegmentData sd)
{
    bool ret=false;
    int rows=0;
    //sd.distance = Distance(sd.startLat, sd.startLon, sd.endLat, sd.endLon);
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        Q_ASSERT(sd._startLat != 0);
        Q_ASSERT(sd._startLon != 0);
        if(sd._endLat == 0)
        {
         sd._endLat = sd.pointList().at(sd.pointList().length()-1).lat();
        }
        if(sd._endLon == 0)
        {
         sd._endLon = sd.pointList().at(sd.pointList().length()-1).lon();
        }
        Q_ASSERT(sd._endLat != 0);
        Q_ASSERT(sd._endLon != 0);

        QString commandText = "update Segments set street = '" + sd._streetName + "', length= " + QString("%1").arg(sd._length) +
            ", tracks="+ QString::number(sd._tracks) + ", startLat=" + QString::number(sd._startLat, 'g', 8) +",startLon=" + QString::number(sd._startLon,'g',8)+ + ", endLat=" + QString::number(sd._endLat, 'g', 8) +",endLon=" + QString::number(sd._endLon,'g',8)+
            ",oneWay='" + sd._oneWay + "'" +
            ",lastUpdate=:lastUpdate where SegmentId = " + QString("%1").arg(sd.segmentId());
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
//                    RollbackTransaction("deletePoint");
            //
              return ret;
            //throw(new ApplicationException("insertPoint: record not found\n" + commandText));

        }

        ret = true;

    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

/// <summary>
/// Get a list of companies
/// </summary>
/// <returns></returns>
QList<CompanyData*> SQL::getCompanies()
{
 QList<CompanyData*> myArray;
 CompanyData* cd;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
     commandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute from Companies";
 else
     commandText = "select [key], description, startDate, endDate, firstRoute, lastRoute from companies";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
  return myArray;
 }
 while (query.next())
 {
     cd = new CompanyData();
     cd->companyKey = query.value(0).toInt();
     cd->name = query.value(1).toString();
     if(query.value(2).isNull())
         cd->startDate = QDate();
     else
         cd->startDate = query.value(2).toDate();
     if (query.value(3).isNull())
         cd->endDate = QDate();
     else
         cd->endDate = query.value(3).toDate();
     cd->firstRoute = query.value(4).toInt();
     cd->lastRoute = query.value(5).toInt();
     myArray.append(cd);
 }
 std::sort(myArray.begin(), myArray.end(), [](const CompanyData* a, const CompanyData* b) -> bool { return a->name < b->name; });

 return myArray;
}

/// <summary>
/// Get the data for a company
/// </summary>
/// <returns></returns>
CompanyData* SQL::getCompany(qint32 companyKey)
{
    CompanyData* cd = nullptr;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute, routePrefix from Companies where `key` = " +QString("%1").arg(companyKey);
        else
            commandText = "select [key], description, startDate, endDate, firstRoute, lastRoute, routePrefix from companies where [key] = " + QString("%1").arg(companyKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
//        if(query.numRowsAffected()== 0)
//         throw  Exception(tr("company key %1 invalid").arg(companyKey));
        while (query.next())
        {
            cd = new CompanyData();
            cd->companyKey = query.value(0).toInt();
            cd->name = query.value(1).toString();
            if(query.value(2).isNull())
                cd->startDate = QDate();
            else
                cd->startDate = query.value(2).toDate();
            if (query.value(3).isNull())
                cd->endDate = QDate();
            else
                cd->endDate = query.value(3).toDate();
            cd->firstRoute = query.value(4).toInt();
            cd->lastRoute = query.value(5).toInt();
            cd->routePrefix = query.value(6).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }
    return cd;
}
/// <summary>
/// Add a new route number.
/// </summary>
/// <param name="routeAlpha"></param>
/// <returns></returns>
qint32 SQL::addAltRoute(QString routeAlpha, QString routePrefix)
{
 int route=-1, rows=0, count=0;
 QSqlDatabase db = QSqlDatabase::database();
 Q_ASSERT(!routeAlpha.isEmpty() && !routeAlpha.startsWith(" "));

 QString commandText = "select max(route) from altRoute group by route";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 while(query.next())
 {
  count = query.value(0).toInt();
 }
 bool bOk = false;
 route = routeAlpha.toInt(&bOk,10);

// if (!bOk)
// {
//  if (count < 999)    // make sure that all Alpha routes have a numeric route >= 1000
//   count = 999;
  route = count + 1;
 //}
 //else
 //    route = -1;

 // First see if it already exists
 commandText = "select route from altRoute where routeAlpha = '" + QString("%1").arg(routeAlpha) + "' " \
   "and routePrefix = '" + routePrefix.trimmed() + "'";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
 }
 int newroute = -1;
 if (query.isActive())
 {
  while (query.next())
  {
   newroute = query.value(0).toInt();
  }
  if(newroute >0 )
   return newroute;
 }
 int baseRoute =route;
 if(routeAlpha.contains("."))
 {
     QString strBase = routeAlpha.mid(0, routeAlpha.indexOf("."));
     baseRoute = addAltRoute(strBase, routePrefix);   // add the base route if it doesn't exist
 }

 commandText = "insert into altRoute (route, routeAlpha, baseRoute, routePrefix) values ( " + QString("%1").arg(route) + ", '" + routeAlpha + "','" + QString("%1").arg(baseRoute) + "','" + routePrefix +"')";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
//  db.close();
//  exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();

 // check if inserted OK
 commandText = QString("select route from altRoute where routeAlpha = '%1' and routePrefix = '%2'").arg(routeAlpha).arg(routePrefix);
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 int newKey = 0;
 while(query.next())
 {
  newKey = query.value(0).toInt();
 }
 return route;
}

bool SQL::deleteRouteSegment(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate, QString routeStartDate, QString routeEndDate)
{
 if(startDate == routeStartDate && endDate == routeEndDate)
  return deleteRouteSegment(route,name,segmentId, startDate, endDate);

}

/// <summary>
/// Delete a segment from a route
/// </summary>
/// <param name="route"></param>
/// <param name="SegmentId"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <returns></returns>
bool SQL::deleteRouteSegment(qint32 route, QString name, qint32 SegmentId, QString startDate, QString endDate)
{
    bool ret = false;
    int rows = 0;
    name = name.trimmed();
    QString commandText;
    QString segStartDate = "", segEndDate = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("deleteRoute");
        if(config->currConnection->servertype() == "MsSql")
         commandText = "delete from Routes where route = " + QString("%1").arg(route) + " and LTRIM(RTRIM(name)) = '" + name + "' and LineKey = " + QString("%1").arg(SegmentId) + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        else
         commandText = "delete from Routes where route = " + QString("%1").arg(route) + " and TRIM(name) = '" + name + "' and LineKey = " + QString("%1").arg(SegmentId) + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //
            //ret = false;
            qDebug() <<"deleteRoute: not found. " + commandText;
            //exit(EXIT_FAILURE);
            return false;
        }

        // Scan the remaining routes to find a new start and end date
        commandText = "select min(startDate), max(endDate) from Routes where lineKey = " + QString("%1").arg(SegmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (query.isActive())
        {
            while (query.next())
            {
                if(query.value(0).isNull())
                    segStartDate = "1890-01-01";
                else
                segStartDate = query.value(0).toDateTime().toString("yyyy/MM/dd");
                if (query.value(1).isNull())
                    segEndDate = "1890-01-01";
                else
                segEndDate = query.value(1).toDateTime().toString("yyyy/MM/dd");
            }

        }
        else
        {
            segStartDate = "1890-01-01";
            segEndDate = "1890-01-01";
        }

        commandText = "update Segments set startDate = '" + segStartDate  + "', endDate = '" + segEndDate + "' " +
            ",lastUpdate=:lastUpdate where SegmentId =" + QString("%1").arg(SegmentId);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();

        CommitTransaction("deleteRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

bool SQL::addSegmentToRoute(RouteData rd)
{
  return addSegmentToRoute( rd.route, rd.name, rd.startDate,
                            rd.endDate, rd.lineKey, rd.companyKey,
                            rd.tractionType, rd.direction, rd.next, rd.prev, rd.normalEnter, rd.normalLeave,
                            rd.reverseEnter, rd.reverseLeave, rd.oneWay, rd.trackUsage);
}
#if 0
/// <summary>
/// Adds a new route segment
/// </summary>
/// <param name="routeNbr"></param>
/// <param name="routeName"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <param name="SegmentId"></param>
/// <returns></returns>
bool SQL::addSegmentToRoute(qint32 routeNbr, QString routeName, QString startDate, QString endDate,
                            qint32 SegmentId, qint32 companyKey, qint32 tractionType,
                            QString direction, qint32 normalEnter, qint32 normalLeave,
                            qint32 reverseEnter, qint32 reverseLeave, QString oneWay,
                            QString trackUsage) //14
{
    bool ret = false;
    int rows = 0;
    if (routeNbr < 1)
    {
        qDebug()<<"Invalid route number";
        throw Exception(tr("route number invalid!"));
    }
    if (routeName == "" || routeName.length() > 100)
    {
        qDebug()<<"invalid route name";
        throw Exception(tr("invalid route name '%1'").arg(routeName));
    }
    QDateTime dtStart = QDateTime::fromString(startDate, "yyyy/MM/dd");
    QDateTime dtEnd = QDateTime::fromString(endDate, "yyyy/MM/dd");

    if (dtEnd.date() < dtStart.date())
    //    throw (new ApplicationException("Invalid end date" + endDate));
    {
        qDebug()<<"end date ("+ endDate +") before start date("+ startDate+")!";
        throw Exception("end date ("+ endDate +") before start date("+ startDate+")!");
    }
    //if (SegmentId <= 0)
    //    throw (new ApplicationException("invalid segmentid:" + SegmentId));
    if (companyKey < 1)
    {
        qDebug()<<"invalid company key: " + QString("%1").arg(companyKey);
        throw Exception("invalid company key: " + QString("%1").arg(companyKey));
    }
    CompanyData* cd = getCompany(companyKey);
    if(routeNbr != 9998 && routeNbr != 9999)
        updateCompany(companyKey, routeNbr);

    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        BeginTransaction("addSegmentToRoute");

        QString commandText = "INSERT INTO Routes(Route, Name, StartDate, EndDate, LineKey,"
           " companyKey, tractionType, direction, normalEnter, normalleave,"
           " reverseEnter, reverseLeave, OneWay, trackUsage) VALUES("
           + QString("%1").arg(routeNbr) + ", '"
           + routeName.trimmed() + "', '"
           + startDate + "', '"
           + endDate + "', " + QString("%1").arg(SegmentId) + ", "
           + QString("%1").arg(companyKey) +","
           + QString("%1").arg(tractionType) +",'"
           + QString("%1").arg(direction) +"', "
           + QString("%1").arg(normalEnter) + ","
           + QString("%1").arg(normalLeave) + ","
           + QString("%1").arg(reverseEnter) + ", "
           + QString("%1").arg(reverseLeave) + ", '"
           + QString("%1").arg(oneWay) + "', '"
           + trackUsage
           + "')";
        qDebug() << commandText;
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            SQLERROR(query);
            //exit(EXIT_FAILURE);
            return false;
        }
        rows = query.numRowsAffected();
//        if (rows == 0)
//        {
//            //                    RollbackTransaction("deletePoint");
//            return ret;
//        }

        updateSegmentDates(SegmentId);

        CommitTransaction("addSegmentToRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
#endif
bool SQL::addSegmentToRoute(qint32 routeNbr, QString routeName, QDate startDate, QDate endDate,
                            qint32 SegmentId, qint32 companyKey, qint32 tractionType,
                            QString direction, qint32 next, qint32 prev, qint32 normalEnter,
                            qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave,
                            QString oneWay, QString trackUsage) //16
{
    bool ret = false;
    int rows = 0;
    if (routeNbr < 1)
    {
        qDebug()<<"Invalid route number";
        return ret;
    }
    if (routeName == "" || routeName.length() > 100)
    {
        qDebug()<<"invalid route name";
        return ret;
    }
//    QDateTime dtStart = QDateTime::fromString(startDate, "yyyy/MM/dd");
//    QDateTime dtEnd = QDateTime::fromString(endDate, "yyyy/MM/dd");

    if (endDate < startDate)
    //    throw (new ApplicationException("Invalid end date" + endDate));
    {
        qDebug()<<"end date ("+ endDate.toString("yyyy/MM/DD") +") before start date("+ startDate.toString("yyyy/MM/DD")+")!";
        return ret;
    }
    //if (SegmentId <= 0)
    //    throw (new ApplicationException("invalid segmentid:" + SegmentId));
    if (companyKey < 1)
    {
        qDebug()<<"invalid company key: " + QString("%1").arg(companyKey);
        return ret;
    }
    CompanyData* cd = getCompany(companyKey);
    if(routeNbr != 9998 && routeNbr != 9999)
        updateCompany(companyKey, routeNbr);

    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        //BeginTransaction("addSegmentToRoute");

        QString commandText = "INSERT INTO Routes(Route, Name, StartDate, EndDate, LineKey, "
                "companyKey, tractionType, direction, next, prev, normalEnter, normalleave,"
                " reverseEnter, reverseLeave, oneWay, trackUsage) "
                "VALUES(" + QString("%1").arg(routeNbr) + ", '"
                + routeName.trimmed() + "', '"
                + startDate.toString("yyyy/MM/dd") + "', '"
                + endDate.toString("yyyy/MM/dd") + "',"
                + QString("%1").arg(SegmentId) + ", "
                + QString("%1").arg(companyKey)+","
                + QString("%1").arg(tractionType)+",'"
                + direction +"', "
                + QString("%1").arg(next) +", "
                + QString("%1").arg(prev) +", "
                + QString("%1").arg(normalEnter) + ","
                + QString("%1").arg(normalLeave) + ","
                + QString("%1").arg(reverseEnter) + ", "
                + QString("%1").arg(reverseLeave) + ", '"
                + QString("%1").arg(oneWay)  + "', '"
                + trackUsage + "')";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
         //db.close();
         return ret;
        }
        rows = query.numRowsAffected();
//        if (rows == 0)
//        {
//            //                    RollbackTransaction("deletePoint");
//            return ret;
//        }

        updateSegmentDates(SegmentId);

        //CommitTransaction("addSegmentToRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateTerminals(TerminalInfo ti)
{
 return  updateTerminals(ti.route, ti.name, ti.startDate.toString("yyyy/MM/dd"), ti.endDate.toString("yyyy/MM/dd"), ti.startSegment, ti.startWhichEnd,
                         ti.endSegment, ti.endWhichEnd);
}

bool SQL::updateTerminals(qint32 route, QString name, QString startDate, QString endDate,
                          qint32 startSegment, QString startWhichEnd, qint32 endSegment, QString endWhichEnd)
{
    bool ret = false;
    int rows = 0;
    qint32 cStartSegment=-1, cEndSegment=-1;
    QString cStartWhichEnd, cEndWhichEnd;
    QString commandText;
    try
    {
        int count =0;
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        if(config->currConnection->servertype() == "MsSql")
         commandText = "select count(*),startSegment, startWhichEnd, endSegment, endWhichEnd from Terminals where route = " + QString("%1").arg(route) + " and endDate = '" + endDate + "' and LTRIM(RTRIM(name)) = '" + name.trimmed() + "' group by startSegment, startWhichEnd, endSegment, endWhichEnd";
        else
         commandText = "select count(*),startSegment, startWhichEnd, endSegment, endWhichEnd from Terminals where route = " + QString("%1").arg(route) + " and endDate = '" + endDate + "' and TRIM(name) = '" + name.trimmed() + "' group by startSegment, startWhichEnd, endSegment, endWhichEnd";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            qDebug() << "error selecting Terminal info";
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            count = query.value(0).toInt();
            cStartSegment = query.value(1).toInt();
            cStartWhichEnd = query.value(2).toString();
            cEndSegment = query.value(3).toInt();
            cEndWhichEnd = query.value(4).toString();
        }
        if (count == 0)
        {
            commandText = "insert into Terminals (route, name, startDate, endDate, startSegment, startWhichEnd, endSegment, endWhichEnd) values (" + QString("%1").arg(route) + ", '" + name.trimmed() + "', '" + startDate + "', '" + endDate + "', " + QString("%1").arg(startSegment) + ", '" + startWhichEnd + "', " + QString("%1").arg(endSegment) + ", '" + endWhichEnd + "')";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            rows = query.numRowsAffected();
            //if (rows == 1)
                ret = true;
        }
        else
        {
            if(startSegment > 0)
            {
                cStartSegment = startSegment;
                cStartWhichEnd = startWhichEnd;
            }
            if(endSegment > 0)
            {
                cEndSegment = endSegment;
                cEndWhichEnd = endWhichEnd;
            }
            commandText = "update Terminals set startdate = '" + startDate + "', endDate = '" + endDate + "', startSegment=" + QString("%1").arg(cStartSegment) + ", startWhichEnd='" + cStartWhichEnd + "', endSegment = " + QString("%1").arg(cEndSegment) + ", endWhichEnd = '" + cEndWhichEnd + "',lastUpdate=:lastUpdate where route = " + QString("%1").arg(route) + " and name = '" + name + "' and endDate = '" + endDate + "'";
            query.prepare(commandText);
            qDebug()<<commandText;
            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
            bQuery = query.exec();
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            rows = query.numRowsAffected();
            //if (rows == 1)
                ret = true;
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}
 /// <summary>
/// Get the numeric route given display value for the route.
/// </summary>
/// <param name="route">alpha route </param>
/// <returns>numeric route number or -1 if route not found</returns>
qint32 SQL::getNumericRoute(QString routeAlpha, QString * newAlphaRoute, bool * bAlphaRoute, int companyKey)
{
    int route = -1;
    *bAlphaRoute = false;
    if (routeAlpha == "NR")
    {
        *(bAlphaRoute) = true;
        route = 9999;
        *(newAlphaRoute) = "NR";
        return route;
    }
    if (routeAlpha == "OS")
    {
        *(bAlphaRoute) = true;
        route = 9998;
        *(newAlphaRoute) = "OS";
        return route;
    }
        //route = Convert.ToInt32(routeAlpha);
    bool bOk=false;
    route = routeAlpha.toInt(&bOk, 10);
    if(!bOk)
    {

        route = -1;
        if (config->currCity->bAlphaRoutes)
        {
            *(newAlphaRoute) = routeAlpha;
            *(bAlphaRoute) = true;
        }
        else
        {
            *(newAlphaRoute) = "";
        }
    }

    if (route != -1)
        *(newAlphaRoute) = (route < 10 ? "0" : "") + QString("%1").arg(route);
    else
        *(newAlphaRoute) = routeAlpha;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText ;
    if(config->currConnection->servertype() != "MsSql")
     commandText = "select route from altRoute a join Companies c on c.routePrefix = a.routePrefix where routeAlpha = '" + routeAlpha + "' and c.`Key` =" + QString::number(companyKey);
    else
     commandText = "select route from altRoute a join Companies c on c.routePrefix = a.routePrefix where routeAlpha = '" + routeAlpha + "' and c.[Key] =" + QString::number(companyKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
         db.close();
         exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return -1;
        }
        route = -1;
        while (query.next())
        {
         route = query.value(0).toInt();
        }

    return route;
}
 /// <summary>
/// Get a list of distinct routeData items
/// </summary>
/// <param name="route"></param>
/// <returns></returns>
QList<RouteData> SQL::getRouteInfo(qint32 route)
{

    QList<RouteData>  myArray;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select distinct a.route, name, startDate, endDate, routeAlpha from Routes a join altRoute b on a.route = b.route where a.route = " + QString("%1").arg(route) + " group by a.route, name, startDate, endDate, routeAlpha";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            RouteData rd =  RouteData ();
            rd.route = query.value(0).toInt();
            rd.name = query.value(1).toString();
            rd.startDate = query.value(2).toDate();
            rd.endDate = query.value(3).toDate();
            //rd.companyKey = (int)myReader.GetInt32(4);
            //rd.tractionType = (int)myReader.GetInt32(5);
            rd.alphaRoute = query.value(4).toString();
            myArray.append(rd);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return myArray;
}
/// <summary>
/// Update company record
/// </summary>
/// <param name="companyKey"></param>
/// <param name="name"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <param name="route"></param>
/// <returns></returns>
bool SQL::updateCompany(qint32 companyKey, qint32 route)
{
    if (route < 1)
        qDebug()<<"Invalid value " + QString("%1").arg(route) + " for route";
    bool ret = false;
    int rows = 0;
    CompanyData* cd ;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QString commandText;
        BeginTransaction("updateCompanies");
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute from Companies where `key` = " + QString("%1").arg(companyKey);
        else
            commandText = "select [key], description, startDate, endDate, firstRoute, lastRoute from companies where [key] = " + QString("%1").arg(companyKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            cd = new CompanyData();
            cd->companyKey = query.value(0).toInt();
            cd->name = query.value(1).toString();
            if (query.value(2).isNull())
                cd->startDate =  QDate();
            else
                cd->startDate = query.value(2).toDate();
            if (query.value(3).isNull())
                cd->endDate =  QDate();
            else
                cd->endDate = query.value(3).toDate();
            cd->firstRoute = query.value(4).toInt();
            cd->lastRoute =query.value(5).toInt();
        }

        if (cd->firstRoute < 1 || route < cd->firstRoute)
            cd->firstRoute = route;
        if (route > cd->lastRoute)
            cd->lastRoute = route;

        if(config->currConnection->servertype() != "MsSql")
            commandText = "Update Companies set  firstRoute= " +QString("%1").arg( cd->firstRoute) + ", lastRoute = " + QString("%1").arg(cd->lastRoute) + ",lastUpdate=:lastUpdate where `key` = " + QString("%1").arg(companyKey);
        else
            commandText = "Update companies set  firstRoute= " +QString("%1").arg( cd->firstRoute) + ", lastRoute = " + QString("%1").arg(cd->lastRoute) + ",lastUpdate=:lastUpdate where [key] = " + QString("%1").arg(companyKey);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
        {
            ret = true;
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }
    CommitTransaction("updateCompanies");
    return ret;
}

/// <summary>
/// Get well code to fix segment start and end dates
/// </summary>
/// <returns></returns>
bool SQL::updateSegmentDates()
{
    QList<SegmentData> myArray;
    SegmentData sd;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText ="Select segmentid, min(b.StartDate), MAX(b.EndDate) from Segments a join Routes b on LineKey = SegmentId group by b.LineKey, SegmentId";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (query.isActive()!= true)
        {
            return true;
        }
        while (query.next())
        {
            sd = SegmentData();
            sd._segmentId = query.value(0).toInt();
            sd._startDate = query.value(1).toDate();
            sd._endDate = query.value(2).toDate();
            myArray.append(sd);
        }
        //foreach(segmentData sd1 in myArray)
        for(int i=0; i<myArray.count(); i++)
        {
            SegmentData sd1 = (SegmentData)myArray.at(i);
            QString commandText = "update Segments set startDate = '"
                    + sd1._startDate.toString("yyyy/MM/dd")+ "', endDate = '"
                    + sd1._endDate.toString("yyyy/MM/dd")
                    + "',lastUpdate=:lastUpdate where segmentid = "
                    + QString("%1").arg(sd1._segmentId);
            query.prepare(commandText);
            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
            bQuery = query.exec();
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            rows = query.numRowsAffected();
            if (rows == 0)
            {
                return false;
            }
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return true;
}
/// <summary>
/// Update the start and end dates for a segment
/// </summary>
/// <param name="SegmentId"></param>
void SQL::updateSegmentDates(int segmentId)
{
    QString segStartDate = "";
    QString segEndDate = "";
    int rows = 0;

    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    QSqlDatabase db = QSqlDatabase::database();

    QString commandText = "select min(startDate), max(endDate) from Routes where linekey = " + QString("%1").arg(segmentId);
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        db.close();
        exit(EXIT_FAILURE);
    }
    if (!query.isActive())
    {
        //
        //
        //return false;
        //throw (new ApplicationException("addSegmentToRoute: segment not found\n" + commandText));
        segStartDate = "1890-1-1";
        segEndDate = "1890-1-1";

    }
    while (query.next())
    {
        if (query.value(0).isNull())
            segStartDate = "1890-01-01";
        else
            segStartDate = query.value(0).toDateTime().toString("yyyy/MM/dd");
        if (query.value(1).isNull())
            segEndDate = "1890-01-01";
        else
            segEndDate = query.value(1).toDateTime().toString("yyyy/MM/dd");
    }
    /*
                    // update the segment record
                    if( String.Compare(startDate,  segStartDate) < 0)
                        segStartDate = startDate;
                    if (String.Compare(endDate, segEndDate) > 0)
                        segEndDate = endDate;
    */
    commandText = "update Segments set startDate = '" + segStartDate + "', endDate ='" + segEndDate + "' " +
        ",lastUpdate=:lastUpdate where SegmentId = " + QString("%1").arg(segmentId);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
    bQuery = query.exec();
    if(!bQuery)
    {
        QSqlError err = query.lastError();
        qDebug() << err.text() + "\n";
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
        db.close();
        exit(EXIT_FAILURE);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
        //
    }
}

QPair<QDate,QDate> SQL::getStartAndEndDates(int segmentId)
{
 QPair<QDate,QDate> pair;

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "select min(startDate), max(endDate) from Routes where linekey = " + QString("%1").arg(segmentId);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QSqlError err = query.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     db.close();
     exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return pair;
 }
 while (query.next())
 {
     pair.first = query.value(0).toDate();
     pair.second = query.value(1).toDate();
 }

 return pair;
}

/// <summary>
/// Get a list of distinct route names for a route number.
/// </summary>
/// <param name="route"></param>
/// <returns></returns>
QList<QString> SQL::getRouteNames(qint32 route)
{
    QList<QString>  myArray;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select distinct name from Routes where route = " + QString("%1").arg(route) + " order by name";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return myArray;
        }
        while (query.next())
        {
            myArray.append(query.value(0).toString());
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    //IComparer myComparer = new compareRoutesClass();
    //myArray.Sort(myComparer);
    return myArray;
}


qint32 SQL::getRouteCompany(qint32 route)
{
    int companyKey=-1;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select companyKey from Routes where route = " + QString("%1").arg(route);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            companyKey = query.value(0).toInt();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return companyKey;
}
/// <summary>
/// Retrieve the parameters for the web site
/// </summary>
/// <returns></returns>
parameters SQL::getParameters()
{
    parameters parms = parameters();
    QString alphaRoutes;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select lat, lon, title, city, minDate, maxDate, alphaRoutes from Parameters";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return parms;
        }
        while (query.next())
        {
            parms.lat = query.value(0).toDouble();
            parms.lon = query.value(1).toDouble();
            parms.title = query.value(2).toString();
            parms.city = query.value(3).toString();
            parms.minDate = query.value(4).toDateTime();
            parms.maxDate = query.value(5).toDateTime();
            alphaRoutes = query.value(6).toString();
            if (alphaRoutes == "Y")
                parms.bAlphaRoutes = true;
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return parms;
}
/// <summary>
/// Get a list of segments for a route and segmentId
/// </summary>
/// <param name="route"></param>
/// <returns></returns>
QList<RouteData> SQL::getRouteSegmentsBySegment(qint32 segmentId)
{
    QList<RouteData> myArray;
//    try
//    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select a.route, name, startDate, endDate, lineKey, companyKey, tractionType, direction, "
              "normalEnter, normalLeave, reverseEnter, reverseLeave, routeAlpha, OneWay"
              " from Routes a join altRoute b on a.route = b.route where lineKey = " + QString("%1").arg(segmentId)
              + " order by routeAlpha, name, endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            SQLERROR(query);
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            RouteData rd =  RouteData();
            rd.route = query.value(0).toInt();
            rd.name = query.value(1).toString();
            rd.startDate =query.value(2).toDate();
            rd.endDate = query.value(3).toDate();
            rd.lineKey = query.value(4).toInt();
            rd.companyKey = query.value(5).toInt();
            rd.tractionType = query.value(6).toInt();
            rd.direction = query.value(7).toString();
            rd.normalEnter = query.value(8).toInt();
            rd.normalLeave = query.value(9).toInt();
            rd.reverseEnter = query.value(10).toInt();
            rd.reverseLeave = query.value(11).toInt();
            rd.alphaRoute = query.value(12).toString();
            QString rdStartDate= rd.startDate.toString("yyyy/MM/dd");
            rd.oneWay = query.value(13).toString();
            myArray.append(rd);
        }
//    }
//    catch (Exception e)
//    {
//        myExceptionHandler(e);

//    }
//    IComparer compareRoutes = new compareAlphaRoutesClass();
//    myArray.Sort(compareRoutes);
    return myArray;
}
/// <summary>
/// Get start and end dates, company key and traction type for a route
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <returns></returns>
QList<RouteData> SQL::getRouteDataForRouteName(qint32 route, QString name)
{
    QList<RouteData> myArray;
    RouteData rd = RouteData();
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
        {
            commandText = "select min(a.startdate), Max(a.enddate), Name, a.route, b.`Key`, tractionType, routeAlpha, a.OneWay from Routes a join Companies b on " + QString("%1").arg(route) + " between firstRoute and lastRoute join altRoute c on a.route = c.route where a.Route = " + QString("%1").arg(route) + " and Name = '" + name + "' group by a.StartDate, a.endDate, Name, a.route, b.`Key`, tractionType, routeAlpha";
        }
        else
        {
            commandText = "select min(a.startdate), Max(a.enddate), Name, a.route, b.[Key], tractionType, routeAlpha from Routes a join companies b on " + QString("%1").arg(route) + " between firstRoute and lastRoute join altRoute c on a.route = c.route where a.Route = " + QString("%1").arg(route) + " and Name = '" + name + "' group by a.StartDate, a.endDate, Name, a.route, b.[Key], tractionType, routeAlpha";
        }
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return myArray;
        }
        while (query.next())
        {
            rd = RouteData();
            rd.startDate =query.value(0).toDate();
            rd.endDate = query.value(1).toDate();
            rd.name = query.value(2).toString();
            rd.route = query.value(3).toInt();
            rd.companyKey = query.value(4).toInt();
            rd.tractionType = query.value(5).toInt();
            rd.alphaRoute = query.value(6).toString();
            rd.oneWay = query.value(6).toString();
            myArray.append(rd);
        }
        //rd.companyKey = -1;
        //rd.tractionType = 1;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return myArray;
}
/// <summary>
/// Returns the earliest end date before the supplied date for a route's segment
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="SegmentId"></param>
/// <param name="date"></param>
/// <returns></returns>
QDate SQL::getRoutesEarliestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date)
{
    Q_UNUSED(SegmentId)
    QDate dt = QDate().fromString(date, "yyyy/MM/dd");
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select Min(startDate) from Routes where route = " + QString("%1").arg(route) + " and name = '" + name +"'  and startDate < '" + date + "'  group  by startDate order by startDate desc";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return dt;
        }
        while (query.next())
        {
            dt = query.value(0).toDate();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return dt;
}
/// <summary>
/// Returns the latest end date before the supplied date for a route's segment
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="SegmentId"></param>
/// <param name="date"></param>
/// <returns></returns>
QDate SQL::getRoutesLatestDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date)
{
    Q_UNUSED(SegmentId)
    QDate dt = QDate().fromString(date, "yyyy/MM/dd");
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select Max(endDate) from Routes where route = " + QString("%1").arg(route) + " and name = '" + name + "'  and endDate < '" + date + "'  group  by endDate order by endDate asc";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return dt;
        }
        while (query.next())
        {
            dt = query.value(0).toDate();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return dt;
}
/// <summary>
/// Returns the Earliest start date after the supplied end date for a route's segment
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="SegmentId"></param>
/// <param name="date"></param>
/// <returns></returns>
QDate SQL::getRoutesNextDateForSegment(qint32 route, QString name, qint32 SegmentId, QString date)
{
    Q_UNUSED(SegmentId)
    QDate dt = QDate().fromString(date, "yyyy/MM/dd");
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select min(startDate) from Routes where route = " + QString("%1").arg(route) + " and name = '" + name + "'  and startDate >= '" + date + "'  group  by startDate order by startDate desc";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return dt;
        }
        while (query.next())
        {
            dt = query.value(0).toDate();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return dt;
}
bool SQL::doesRouteSegmentExist(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate)
{
    bool ret = false;
    int count = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select count(*) from Routes where route = " + QString("%1").arg(route)
                              + " and name = '" + name
                              + "' and lineKey= " + QString("%1").arg(segmentId)
                              + " and startDate = '"
                              + startDate + "' and endDate='"
                              + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return false;
        }
        while (query.next())
        {
            count = query.value(0).toInt();
        }
        if (count > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

SegmentData SQL::getSegmentInSameDirection(SegmentData sdIn)
{
    sdIn._bearing = Bearing(sdIn._startLat, sdIn._startLon, sdIn._endLat, sdIn._endLon);
    SegmentData sd = SegmentData();
    QList<SegmentData> startIntersects = getIntersectingSegments(sdIn._startLat, sdIn._startLon, .020, sdIn._routeType);
    Bearing b = Bearing();
    if(startIntersects.isEmpty())
        return sd;

    QList<SegmentData> endIntersects = getIntersectingSegments(sdIn._endLat, sdIn._endLon, .020, sdIn._routeType);
    if(endIntersects.isEmpty())
        return sd;

    //foreach (segmentData sdStart in startIntersects)
    for(int i= 0; i <startIntersects.count();i++)
    {
        SegmentData sdStart = startIntersects.at(i);
        if (sdStart._segmentId == sdIn._segmentId)
            continue;
        //foreach (segmentData sdEnd in endIntersects)
        for(int j=0; j <endIntersects.count(); j++)
        {
            SegmentData sdEnd =endIntersects.at(j);
            if (sdEnd._segmentId == sdIn._segmentId)
                continue;
            b = Bearing(sdEnd._startLat, sdEnd._startLon, sdEnd._endLat, sdEnd._endLon);
            if (sdStart._segmentId == sdEnd._segmentId && sdIn._bearing.getDirection() == b.getDirection())
            {
                SegmentData sdWrk = getSegmentData(sdEnd._segmentId);
                if (sdWrk._oneWay == sdIn._oneWay)
                {
                    sd = sdWrk;
                    break;
                }
            }
            if(sdIn._oneWay == "N" )
            {
                if (sdStart._segmentId == sdEnd._segmentId )
                {
                    SegmentData sdWrk = getSegmentData(sdEnd._segmentId);
                    if (sdWrk._oneWay == sdIn._oneWay)
                    {
                        sd = sdWrk;
                        break;
                    }
                }
            }

        }
    }
    return sd;
}

bool SQL::deleteSegment(qint32 SegmentId)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        BeginTransaction("deleteSegment");

        QString commandText;
        QSqlQuery query = QSqlQuery(db);

        // delete any Line Segments using this segment
        commandText = "delete from LineSegment where segmentid = " + QString("%1").arg(SegmentId);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        // delete any routes referencing the segment
        commandText = "delete from Routes where LineKey = " + QString("%1").arg(SegmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();

        commandText = "delete from Segments where segmentid = " + QString("%1").arg(SegmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //throw( new ApplicationException("deleteSegment, segmentID " + SegmentId + " not found"));
            qDebug()<< "deleteSegment, SegmentId " + QString("%1").arg(SegmentId) + " not found";
            //
            //ret = false;
            exit(EXIT_FAILURE);
        }
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    CommitTransaction("deleteSegment");
    return ret;
}
/// <summary>
/// Return default company key for a date.
/// </summary>
/// <param name="date"></param>
/// <returns></returns>
qint32 SQL::getDefaultCompany(qint32 route, QString date)
{
    int companyKey = -1;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select `key`, description from Companies where '" + date + "' between startDate and endDate and " + QString("%1").arg(route) + " between firstRoute and lastRoute";
        else
            commandText = "select [key], description from companies where '" + date + "' between startDate and endDate and " + route + " between firstRoute and lastRoute";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            companyKey = query.value(0).toInt();
        }

    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }
    return companyKey;
}
LatLng SQL::getPointInfo(qint32 pt, qint32 SegmentId)
{
    LatLng pi;
    try
    {

        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        // Retrieve the current start location for pt-1 to calculate the length
        QString commandText = "Select StartLat, StartLon from [dbo].[LineSegment] where SegmentId = " +         QString("%1").arg(SegmentId) + " and sequence = " + QString("%1").arg(pt);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (!query.isActive())
        {
            return pi;
        }
        while (query.next())
        {
            pi.setLat(query.value(0).toDouble());
            pi.setLon(query.value(1).toDouble());
        }
    }
     catch (Exception e)
     {
         myExceptionHandler(e);
     }
     return pi;
}
qint32 SQL::addCompany(QString name, qint32 route, QString startDate, QString endDate)
{
    int companyKey =-1;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "select `key`, description from Companies where description = '" + name + "'";
        else
            commandText = "select [key], description from companies where description = '" + name + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            companyKey = query.value(0).toInt();
        }

        if(companyKey == -1)
        {
            commandText = "insert into Companies (description, startDate, endDate, firstRoute, lastRoute) values ('" + name + "', '"+ startDate + "','"+endDate+"',"+QString("%1").arg(route)+","+QString("%1").arg(route)+")";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            rows = query.numRowsAffected();
            if(rows > 0)
            {
                if(config->currConnection->servertype() != "MsSql")
                    commandText = "select `key`, description from Companies where description = '" + name + "'";
                else
                    commandText = "select [key], description from Companies where description = '" + name + "'";
                bQuery = query.exec(commandText);
                if(!bQuery)
                {
                    QSqlError err = query.lastError();
                    qDebug() << err.text() + "\n";
                    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                    db.close();
                    exit(EXIT_FAILURE);
                }
                while (query.next())
                {
                    companyKey = query.value(0).toInt();
                }

            }
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }

    return companyKey;
}        /// <summary>
/// Add or retrieve a segment Id corresponding to the Description
/// </summary>
/// <param name="Description"></param>
/// <returns></returns>
qint32 SQL::addSegment(QString Description, QString OneWay, int tracks, RouteType routeType, const QList<LatLng> pointList, bool *bAlreadyExists, bool forceInsert)
{
 int rows = 0;
 int SegmentId = -1;
 *(bAlreadyExists) = false;
 QString street;
 if(Description.contains(","))
  street = Description.mid(0,Description.indexOf(","));

 try
 {
 if(!dbOpen())
    throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 BeginTransaction("addSegment");

 QString commandText = "Select SegmentId from Segments where Description = '" + Description
   + "' and tracks = " + QString("%1").arg(tracks)
   + " and type = "  + QString("%1").arg(routeType)
   + " and OneWay= '" + OneWay + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  db.close();
  exit(EXIT_FAILURE);
 }
 while (query.next())
 {
  SegmentId = query.value(0).toInt();
 }

 if (SegmentId > 0 && !forceInsert)
 {
  *(bAlreadyExists) = true;
  return SegmentId;
 }
 // Add a new SegmentId
 QString pointArray = "";
 for(int i = 0; i < pointList.count(); i ++)
 {
  if(i > 0)
   pointArray.append(",");
  LatLng pt = pointList.at(i);
  pointArray.append(pt.str());
 }
 if(pointArray.count() < 2)
 {
  qDebug() << "Warning segment '" << Description << "' has less than two points!!";
 }
 commandText = "Insert into Segments (street, Description, OneWay, type, pointArray, tracks) values ('" +street+"','" + Description + "', '" + OneWay + "'," +   QString("%1").arg((qint32)routeType) + ",'" + pointArray+ "', " + QString::number(tracks)+ ")";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
    db.close();
    exit(EXIT_FAILURE);
  }
  rows = query.numRowsAffected();

  // Now get the SegmentId (identity) value so it can be returned.
  if(config->currConnection->servertype() == "Sqlite")
    commandText = "SELECT LAST_INSERT_ROWID()";

  else
 if(config->currConnection->servertype() != "MsSql")
    commandText = "SELECT LAST_INSERT_ID()";
  else
    commandText = "SELECT IDENT_CURRENT('segments')";
  bQuery = query.exec(commandText);
 if(!bQuery)
  {
    QSqlError err = query.lastError();
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    db.close();
    exit(EXIT_FAILURE);
  }
  while (query.next())
  {
    SegmentId = query.value(0).toInt();
  }

 }
 catch (Exception e)
 {
    myExceptionHandler(e);
 }
 CommitTransaction("addSegment");

 emit segmentsChanged(SegmentId);
 return SegmentId;
}
/// <summary>
/// Split an existing line segment
/// </summary>
/// <param name="pt"></param>
/// <param name="SegmentId"></param>
/// <param name="oldDesc"></param>
/// <param name="oldOneWay"></param>
/// <param name="newDesc"></param>
/// <param name="newOneWay"></param>
/// <returns>new segment id</returns>
qint32 SQL::splitSegment(qint32 pt, qint32 SegmentId, QString oldDesc, QString oldOneWay, QString newDesc,
                         QString newOneWay, RouteType routeType, RouteType newRouteType,int oldTracks, int newTracks,
                         QString oldStreet, QString newStreet)
{
int rows = 0, newSegmentId=-1;
try
{
 if(!dbOpen())
    throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;
 BeginTransaction("splitSegment");

 SegmentData oldSd = getSegmentData(SegmentId); // Original segment before any changes.

 LatLng p = oldSd.pointList().at(pt); // breaking at this point!
 QString pointArray = "";
 double length = 0;
 for(int i =0; i <= pt; i++)
 {
  LatLng pt = oldSd.pointList().at(i);
  if(i > 0)
  {
   pointArray.append(",");
   length += Distance(pt.lat(), pt.lon(), oldSd.pointList().at(i-1).lat(), oldSd.pointList().at(i-1).lon());
  }
  pointArray.append(pt.str());
 }
 Bearing b= Bearing(oldSd._startLat, oldSd._startLon, p.lat(), p.lon());
 QString direction = oldSd._direction;
// if(oldSd.oneWay == "Y")
//  direction = b.strDirection();
// else
// direction = b.strDirection()+"-"+b.strReverseDirection();

 // Modify the description of the remaining (original) segment as well as the end point and length.
 Q_ASSERT(p.lat() != 0);
 Q_ASSERT(p.lon() != 0);

 commandText = "update Segments set description = '" + oldDesc + "', OneWay='" + oldOneWay + "',endLat="+QString("%1").arg(p.lat(),0,'f',8)+",endLon="+ QString("%1").arg(p.lon(),0,'f',8)+",length="+QString("%1").arg(b.Distance(),0,'f',8)+",points="+QString("%1").arg(pt)+",direction = '"+ direction + "', pointArray='"+pointArray + "', tracks=" +QString::number(oldTracks)+ ",street='"+newStreet + "',lastUpdate=:lastUpdate where SegmentId = " + QString("%1").arg(SegmentId);
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bQuery = query.exec();
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if(rows == 0)
 {
  qCritical() << "splitSegment: not found " + commandText;
  exit(EXIT_FAILURE);
 }

 // Add the new segment
 QString pointArray2 = "";
 LatLng endpt; // when loop finishes, this is the new end point.
 length = 0;
 for(int i =pt; i < oldSd.pointList().count(); i++)
 {
  endpt = oldSd.pointList().at(i);
  if(i != pt)
  {
   pointArray2.append(",");
   length += Distance(endpt.lat(), endpt.lon(), oldSd.pointList().at(i-1).lat(), oldSd.pointList().at(i-1).lon());
  }
  pointArray2.append(endpt.str());
 }
 b = Bearing(p.lat(), p.lon(), oldSd._endLat, oldSd._endLon);
 // if(oldSi.oneWay == "Y")
 //  direction = b.strDirection();
 // else
 //  direction = b.strDirection()+"-"+b.strReverseDirection();
 commandText = "Insert into Segments (Description, OneWay, type, startLat, startLon, endLat, "\
     "endLon, length, startDate, endDate, points, direction, pointArray, tracks, street) values ('"
     + newDesc + "', '"
     + newOneWay + "',"
     + QString("%1").arg((int)newRouteType)
     + ","+QString("%1").arg(p.lat(),0,'f',8)+","+QString("%1").arg(p.lon(),0,'f',8)
     + ","+QString("%1").arg(endpt.lat(),0,'f',8)+","+QString("%1").arg(endpt.lon(), 0, 'f', 8)+","
     + QString("%1").arg(length,0,'f',8)+",'"
     + oldSd._startDate.toString("yyyy/MM/dd")+"','"+oldSd._endDate.toString("yyyy/MM/dd")+"',"
     + QString("%1").arg(pointArray2.count())+",'" + direction+"','" + pointArray2 + "', "
     + QString::number(newTracks)+",'"+newStreet + "')";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  db.close();
  exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
  qDebug()<<"(splitSegment insert failed:" + commandText + "\n";
    return -1;
 }
 // Now get the SegmentId (identity) value of the new segment so it can be returned.
 if(config->currConnection->servertype() == "Sqlite")
  commandText = "SELECT LAST_INSERT_ROWID()";

 else
 if(config->currConnection->servertype() != "MsSql")
    commandText = "SELECT LAST_INSERT_ID()";
 else
    commandText = "SELECT IDENT_CURRENT('segments')";

 bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
    db.close();
    exit(EXIT_FAILURE);
 }
 if(!query.isActive())
 {
  qWarning() <<"(splitSegment insert failed:" + commandText + "\n";
  return -1;
 }
 while (query.next())
 {
  newSegmentId = query.value(0).toInt();
 }


  // Now change the SegmentId and sequence for the remaining line segments
  oldSd = getSegmentData(SegmentId);
  SegmentData newSd = getSegmentData(newSegmentId);
  double diff = angleDiff(oldSd._bearing.getBearing(), newSd._bearing.getBearing());
  int normalEnter = 0, normalLeave = 0, reverseEnter = 0, reverseLeave = 0;
  if (diff < -45)
  {
   normalEnter = 1;
   normalLeave = 1;
   reverseEnter = 2;
   reverseLeave = 2;
  }
  if (diff > 45)
  {
   normalEnter = 2;
   normalLeave = 2;
   reverseEnter = 1;
   reverseLeave = 1;
  }

  // Get a list of routes using the old segment and insert route segments for the new segment.
  QList<RouteData> routes;


  commandText = "SELECT a.route, name, a.startDate, a.endDate, companyKey, tractionType, routeAlpha, normalEnter, normalLeave, reverseEnter, reverseLeave, a.OneWay from Routes a join Segments b on LineKey = b.SegmentId join altRoute c on a.route = c.route where SegmentId = " + QString("%1").arg(SegmentId);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
   SQLERROR(query);
   db.close();
   exit(EXIT_FAILURE);
  }
  while (query.next())
  {
   RouteData rd = RouteData();
   rd.route = query.value(0).toInt();
   rd.name = query.value(1).toString();
   rd.startDate = query.value(2).toDate();
   rd.endDate = query.value(3).toDate();
   rd.companyKey = query.value(4).toInt();
   rd.tractionType = query.value(5).toInt();
   rd.alphaRoute = query.value(6).toString();
   rd.normalEnter = query.value(7).toInt();
   rd.normalLeave = query.value(8).toInt();
   rd.reverseEnter = query.value(9).toInt();
   rd.reverseLeave = query.value(10).toInt();
   rd.lineKey = SegmentId;
   rd.oneWay = query.value(11).toString();
   routes.append( rd);
  }


  //foreach (routeData rd1  in routes)
  for(int i=0; i < routes.count(); i++)
  {
   RouteData rd1 =routes.at(i);
   // Check to see if the route exists already
   int count =0;
   commandText = "Select count(*) from Routes where route = " + QString("%1").arg(rd1.route) + " and " +
           "startDate = '" + rd1.startDate.toString("yyyy/MM/dd") + "' and enddate = '" + rd1.endDate.toString("yyyy/MM/dd") + "' and " +
       "lineKey = " + QString("%1").arg(newSegmentId);
   bQuery = query.exec(commandText);
   if(!bQuery)
   {
    QSqlError err = query.lastError();
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    db.close();
    exit(EXIT_FAILURE);
   }
   while (query.next())
   {
    count = query.value(0).toInt();
   }

   if (count == 0)
   {

    commandText = "insert into Routes ( route, name, startDate, endDate, lineKey, companyKey, tractionType, "
                  "normalEnter, normalLeave, reverseEnter, reverseLeave, OneWay) "
                  " values ("
                  + QString("%1").arg(rd1.route) + ", '"
                  + rd1.name + "', '"
                  +  rd1.startDate.toString("yyyy/MM/dd") +   "', '"
                  +  rd1.endDate.toString("yyyy/MM/dd") + "', "
                  + QString("%1").arg(newSegmentId) + ","
                  + QString("%1").arg(rd1.companyKey) + ","
                  + QString("%1").arg(rd1.tractionType) + ","
                  + QString("%1").arg(normalEnter) + ","
                  + QString("%1").arg(rd1.normalLeave) + ","
                  + QString("%1").arg(rd1.reverseEnter) + ","
                  + QString("%1").arg(reverseLeave) + ", '"
                  + rd1.oneWay + "')";
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
     QSqlError err = query.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     db.close();
     exit(EXIT_FAILURE);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
     qDebug()<<"(splitSegment insert failed:" + commandText + "\n";
     return -1;
    }
    // Now update the turn info for the original segment
    commandText = "update Routes set normalLeave = " + QString("%1").arg(normalLeave) + ", reverseEnter = " + QString("%1").arg(reverseEnter) + ",lastUpdate=:lastUpdate where route = " + QString("%1").arg(rd1.route) + " and name = '" + rd1.name + "' and startDate = '" + rd1.startDate.toString("yyyy/MM/dd") + "' and endDate = '" + rd1.endDate.toString("yyyy/MM/dd") + "' and lineKey = " + QString("%1").arg(rd1.lineKey);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
    bQuery = query.exec();
    if(!bQuery)
    {
     SQLERROR(query);
     db.close();
     exit(EXIT_FAILURE);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
     if(config->currConnection->servertype() !=  "MySql")
     {
         return -1;
     }
     qDebug()<< "assume update was successful:" + commandText + "\n";
    }
   }
  }


  updateSegmentDates(SegmentId);
  updateSegmentDates(newSegmentId);
  CommitTransaction("splitSegment");

 }
 catch (Exception e)
 {
  myExceptionHandler(e);
 }

 return newSegmentId;
}
/// <summary>
/// Get the info for a route segment
/// </summary>
/// <param name="route"></param>
/// <param name="SegmentId"></param>
/// <returns></returns>
RouteData SQL::getRouteData(qint32 route, qint32 SegmentId, QString startDate, QString endDate)
{
 RouteData rd = RouteData();
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "Select a.route, name, a.startDate, a.endDate, a.companyKey, a.tractionType,"
                       "b.routeAlpha, a.OneWay, a.TrackUsage, "
                       "s.startLat, s.startLon, s.endLat, s.endLon, s.tracks, s.description "
                       " from Routes a "
                       " join Segments s on a.linekey = s.segmentId "
                       " join altRoute b on a.route = b.route"
                       " where a.route = " + QString("%1").arg(route)
                       + " and lineKey = " + QString("%1").arg(SegmentId)
                       + " and a.endDate = '" + endDate + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  db.close();
  exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
  return rd;
 }
 qDebug() << commandText << " line:" <<__LINE__;

 while (query.next())
 {
  rd.route = query.value(0).toInt();
  rd.name = query.value(1).toString();
  rd.startDate = query.value(2).toDate();
  rd.endDate = query.value(3).toDate();
  rd.lineKey = SegmentId;
  rd.companyKey = query.value(4).toInt();
  rd.tractionType = query.value(5).toInt();
  rd.alphaRoute = query.value(6).toString();
  rd.oneWay = query.value(7).toString();
  rd.trackUsage = query.value(8).toString();
  rd.bearing = new Bearing(query.value(8).toDouble(),query.value(9).toDouble(), query.value(10).toDouble(),query.value(11).toDouble());
  rd.sd->_tracks = query.value(12).toInt();
  rd.sd->_description = query.value(13).toString();
 }


 return rd;
}
/// <summary>
/// Update a route segment
/// </summary>
/// <param name="routeNbr"></param>
/// <param name="routeName"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <param name="SegmentId"></param>
/// <returns></returns>
bool SQL::updateSegmentToRoute(qint32 routeNbr, QString routeName, QString startDate, QString endDate,
                               qint32 SegmentId, qint32 companyKey, qint32 tractionType,
                               qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave, QString oneWay)
{
    bool ret = false;
    int rows = 0;
//            string  segStartDate = "";
//            string  segEndDate = "";

    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("updateSegmentToRoute");

        QString commandText = "Update Routes set companyKey= " +QString("%1").arg( companyKey)
          + ", tractionType=" + QString("%1").arg(tractionType)
          + ", normalEnter = " + QString("%1").arg(normalEnter)
          + ", normalLeave= " + QString("%1").arg(normalLeave)
          + ", reverseEnter= " + QString("%1").arg(reverseEnter)
          + ", reverseLeave = " + QString("%1").arg(reverseLeave)
          + ", OneWay = '" + oneWay
          + "', lastUpdate=:lastUpdate "
          "where route = " + QString("%1").arg(routeNbr)
          + " and Name= '" + routeName
          + "' and LineKey = " + QString("%1").arg(SegmentId)
          + " and StartDate= '" + startDate
          + "' and EndDate= '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        bool bQuery = query.exec();
        if(!bQuery)
        {
            SQLERROR(query);
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //                    RollbackTransaction("deletePoint");

            return ret;
        }

        updateSegmentDates(SegmentId);

        CommitTransaction("updateSegmentToRoute");

        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }

    return ret;
}
RouteData SQL::getSegmentInfoForRouteDates(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate)
{
    RouteData rd;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT a.Route,Name,StartDate,EndDate,LineKey,CompanyKey,tractionType,"
                              "direction, normalEnter, normalLeave, reverseEnter, reverseLeave, "
                              "routeAlpha, a.OneWay, a.TrackUsage from Routes a "
                              "join altRoute c on a.route = c.route"
                              " where a.Route = " + QString("%1").arg(route) + ""
                              " and ('" + startDate + "' between startDate and endDate"
                              " or  '" + endDate + "' between startDate and endDate)"
                              " and name = '" + name + "'"
                              " and a.LineKey = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (!query.isActive())
        {
            return rd;
        }
        //                myArray = new LatLng[myReader.RecordsAffected];

        while (query.next())
        {
            rd.route = query.value(0).toInt();
            rd.name = query.value(1).toString();
            rd.startDate = query.value(2).toDate();
            rd.endDate = query.value(3).toDate();
            rd.lineKey = query.value(4).toInt();
            rd.companyKey = query.value(5).toInt();
            rd.tractionType = query.value(6).toInt();
            rd.direction = query.value(7).toString();
            rd.normalEnter = query.value(8).toInt();
            rd.normalLeave = query.value(9).toInt();
            rd.reverseEnter = query.value(10).toInt();
            rd.reverseLeave = query.value(11).toInt();
            rd.alphaRoute = query.value(12).toString();
            rd.oneWay = query.value(13).toString();
            rd.trackUsage = query.value(14).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return rd;
}
/// <summary>
/// Delete an entire route
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <returns></returns>
bool SQL::deleteRoute(qint32 route, QString name, QString startDate, QString endDate)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("deleteRoute");

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "delete from Routes where route = " + QString("%1").arg(route) + " and name = '" + name + "' and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        else
            commandText = "delete from [dbo].[routes] where [route] = " + QString("%1").arg(route) + " and name = '" + name + "' and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //myConnection.Close();
            //ret = false;
            //throw (new ApplicationException("deleteRoute: not found. " + myCommand.commandText));
            qDebug()<<"deleteRoute: not found. " + commandText;
            exit(EXIT_FAILURE);
        }

        CommitTransaction("deleteRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

// delete a single segment
bool SQL::deleteRoute(RouteData rd)
{
 bool ret = true;
 int rows = 0;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
  commandText = "delete from Routes where route = " + QString("%1").arg(rd.route) + " and name = '" + rd.name + "' and startDate = '" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate = '" + rd.endDate.toString("yyyy/MM/dd") + "' and lineKey ="+ QString::number(rd.lineKey);
 else
  commandText = "delete from [dbo].[routes] where [route] = " + QString("%1").arg(rd.route) + " and name = '" + rd.name + "' and startDate = '" + rd.startDate.toString("yyyy/MM/dd") + "' and endDate = '" + rd.endDate.toString("yyyy/MM/dd") + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
  return false;
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
     //myConnection.Close();
     //ret = false;
     //throw (new ApplicationException("deleteRoute: not found. " + myCommand.commandText));
     qDebug()<<"deleteRoute: not found. " + commandText;
     return false;
 }

 return ret;
}

bool SQL::modifyRouteDate(RouteData* rd, bool bStartDate, QDate dt, QString name1, QString name2)
{
 bool ret = true;
 int rows = 0;
  BeginTransaction("modifyRouteDate");
//  if(otherRd != NULL)
//  {
//   if( bStartDate)
//   {
//    QDate endDate = dt.addDays(-1);
//    routeData* priorRd = otherRd;
//    if(!modifyCurrentRoute(priorRd, false, endDate))
//      ret = false;
//   }
//   else
//   {
//    QDate startDate = dt.addDays(+1);
//    routeData* nextRd = otherRd;
//    if(!modifyCurrentRoute(nextRd, true, startDate))
//     ret = false;
//   }
//  }

  if(!modifyCurrentRoute(rd, bStartDate, dt, name1, name2))
   ret = false;

  if(ret)
   CommitTransaction("modifyRouteDate");
  else
   RollbackTransaction("modifyRouteDate");
 return ret;
}

#if 0
bool SQL::modifyCurrentRoute(routeData* rd, bool bStartDate, QDate dt)
{
 bool ret = false;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 int rows;
 if (bStartDate)
  commandText = "update Routes set startDate = '" + dt.toString("yyyy/MM/dd") + "',lastUpdate=:lastUpdate where startDate = '" + rd->startDate.toString("yyyy/MM/dd") + "' and route = " + QString("%1").arg(rd->route);
  else
  commandText = "update Routes set endDate = '" + dt.toString("yyyy/MM/dd") + "',lastUpdate=:lastUpdate where endDate = '" + rd->endDate.toString("yyyy/MM/dd") + "' and route = " + QString("%1").arg(rd->route);
  QSqlQuery query = QSqlQuery(db);
  query.prepare(commandText);
  query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
  bool bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   db.close();
   exit(EXIT_FAILURE);
  }
  rows = query.numRowsAffected();
  if (rows > 0)
      ret = true;
  // modify any routes that begin the day after or end the day before.
  if (bStartDate)
   commandText = "update Routes set endDate = '" + dt.addDays(1).toString("yyyy/MM/dd") + "',lastUpdate=:lastUpdate where endDate = '" + rd->startDate.addDays(1).toString("yyyy/MM/dd") + "' and route = " + QString("%1").arg(rd->route);
  else
   commandText = "update Routes set startDate = '" + dt.addDays(1).toString("yyyy/MM/dd") + "',lastUpdate=:lastUpdate where startDate = '" + rd->endDate.addDays(1).toString("yyyy/MM/dd") + "' and route = " + QString("%1").arg(rd->route);
  query.prepare(commandText);
  query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
  bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   db.close();
   exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 ret = true;
 return ret;
}
#else
bool SQL::modifyCurrentRoute(RouteData* crd, bool bStartDate, QDate dt, QString name1, QString name2)
{
 bool ret = true;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 RouteData priorRd = RouteData();
 RouteData rd = RouteData();
 QDate pDt = dt.addDays(-1);
 int rows = 0;
 if(bStartDate)
 {
  commandText = QString("select Route, Name, StartDate, EndDate, LineKey, CompanyKey,  tractionType, next, prev,  normalEnter, normalLeave, reverseEnter, reverseLeave from Routes where route = %1 and '%2' between startDate and endDate order by lineKey, endDate").arg(crd->route).arg(crd->startDate.toString("yyyy/MM/dd"));
  if(!query.exec(commandText))
  {
   SQLERROR(query);
   return false;
  }
  while(query.next())
  {
   rd = RouteData();
   rd.route = query.value(0).toInt();
   rd.name = query.value(1).toString();
   rd.startDate = query.value(2).toDate();
   rd.endDate = query.value(3).toDate();
   rd.lineKey = query.value(4).toInt();
   rd.companyKey = query.value(5).toInt();
   rd.tractionType = query.value(6).toInt();
   rd.next = query.value(7).toInt();
   rd.prev = query.value(8).toInt();
   rd.normalEnter = query.value(9).toInt();
   rd.normalLeave = query.value(10).toInt();
   rd.reverseEnter = query.value(11).toInt();
   rd.reverseLeave = query.value(12).toInt();

   if(crd->startDate == rd.startDate)
   {
    commandText = QString("UPDATE Routes set startDate = '%1', lastUpdate=:lastUpdate where startDate = '%2' and route= %3 and name = '%4' and lineKey=%5").arg(dt.toString("yyyy/MM/dd")).arg(rd.startDate.toString("yyyy/MM/dd")).arg(rd.route).arg(rd.name).arg(rd.lineKey);
    QSqlQuery query = QSqlQuery(db);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
    if(!query.exec())
    {
     SQLERROR(query);
     return false;
    }
    rows += query.numRowsAffected();
    continue;
   }
   if(crd->startDate.addDays(-1) == rd.endDate)
   {
    commandText = QString("UPDATE Routes set endDate = '%1', lastUpdate=:lastUpdate where startDate = '%2' and route= %3 and name = '%4' and lineKey=%5").arg(dt.addDays(-1).toString("yyyy/MM/dd")).arg(rd.startDate.toString("yyyy/MM/dd")).arg(rd.route).arg(rd.name).arg(rd.lineKey);
    QSqlQuery query = QSqlQuery(db);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
    if(!query.exec())
    {
     SQLERROR(query);
     return false;
    }
    rows += query.numRowsAffected();
    continue;
   }
   // Split a record spanning the start date
   if(rd.startDate < dt)
   {
    if(deleteRoute(rd))
    {
     QDate saveDt = rd.endDate;
     rd.endDate = dt.addDays(-1);
     if(insertRouteSegment(rd))
     {
      rd.endDate = saveDt;
      rd.startDate = dt;
      if(!insertRouteSegment(rd))
      return false;
     }
     else return false;
    }
    else return false;
   }
  }
 }
 else
 {
  // modify endDate
  commandText = QString("select Route, Name, StartDate, EndDate, LineKey, CompanyKey,  tractionType, next, prev,  normalEnter, normalLeave, reverseEnter, reverseLeave, oneWay from Routes where route = %1 and '%2' between startDate and endDate order by lineKey, startDate DESC").arg(crd->route).arg(crd->endDate.toString("yyyy/MM/dd"));
  if(!query.exec(commandText))
  {
   SQLERROR(query);
   return false;
  }
  while(query.next())
  {
   RouteData rd = RouteData();
   rd.route = query.value(0).toInt();
   rd.name = query.value(1).toString();
   rd.startDate = query.value(2).toDate();
   rd.endDate = query.value(3).toDate();
   rd.lineKey = query.value(4).toInt();
   rd.companyKey = query.value(5).toInt();
   rd.tractionType = query.value(6).toInt();
   rd.next = query.value(7).toInt();
   rd.prev = query.value(8).toInt();
   rd.normalEnter = query.value(9).toInt();
   rd.normalLeave = query.value(10).toInt();
   rd.reverseEnter = query.value(11).toInt();
   rd.reverseLeave = query.value(12).toInt();
   rd.oneWay = query.value(13).toString();

   if(crd->endDate == rd.endDate)
   {
    commandText = QString("UPDATE Routes set endDate = '%1', lastUpdate=:lastUpdate where endDate = '%2' and route= %3 and name = '%4' and lineKey = %5").arg(dt.toString("yyyy/MM/dd")).arg(rd.endDate.toString("yyyy/MM/dd")).arg(rd.route).arg(rd.name).arg(rd.lineKey);
    QSqlQuery query = QSqlQuery(db);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
    if(!query.exec())
    {
     SQLERROR(query);
     return false;
    }
   }
   if( crd->endDate.addDays(1) == rd.startDate)
   {
    commandText = "update Routes set startDate = '" + dt.toString("yyyy/MM/dd") + "',lastUpdate=:lastUpdate where startDate = '" + crd->startDate.toString("yyyy/MM/dd") + "' and route = " + QString("%1").arg(crd->route);
    QSqlQuery query = QSqlQuery(db);
    query.prepare(commandText);
    query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());continue;
    if(!query.exec())
    {
     SQLERROR(query);
     return false;
    }
    rows += query.numRowsAffected();
    continue;
   }
  }
 }
 return ret;
}

// helper routines for Routes
QString SQL::getPrevRouteName(QDate dt)
{
 QString commandText;
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 commandText = QString("select name from Routes where endDate = '%1'").arg(dt.addDays(-1).toString("yyyy/MM/dd"));
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return "error!";
 }
 while(query.next())
 {
  return query.value(0).toString();
 }
 return "";
}

QString SQL::getNextRouteName(QDate dt)
{
 QString commandText;
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 commandText = QString("select name from Routes where startDate = '%1'").arg(dt.addDays(1).toString("yyyy/MM/dd"));
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return "error!";
 }
 while(query.next())
 {
  return query.value(0).toString();
 }
 return "";
}

bool SQL::insertRouteSegment(RouteData rd)
{
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 if(!rd.startDate.isValid() || !rd.endDate.isValid() || rd.endDate < rd.startDate)
  throw IllegalArgumentException("Invalid dates ");

 commandText = "INSERT INTO Routes(Route, Name, StartDate, EndDate, LineKey, companyKey, tractionType, direction, "
               "normalEnter, normalleave, reverseEnter, reverseLeave, oneWay, trackusage) "
               "VALUES(" + QString("%1").arg(rd.route) + ", '"
               + rd.name.trimmed() + "', '"
               + rd.startDate.toString("yyyy/MM/dd") + "', '"
               + rd.endDate.toString("yyyy/MM/dd") + "',"
               + QString("%1").arg(rd.lineKey) + ", "
               + QString("%1").arg(rd.companyKey)+","
               + QString("%1").arg(rd.tractionType)+",'"
               + rd.direction +"', "
               + QString("%1").arg(rd.normalEnter) + ","
               + QString("%1").arg(rd.normalLeave) + ","
               + QString("%1").arg(rd.reverseEnter) + ", "
               + QString("%1").arg(rd.reverseLeave) + ", '"
               + rd.oneWay + "', '"
               + rd.trackUsage
               + "')";
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return false;
 }
 return true;
}
#endif

/// <summary>
/// Returns a list of route segments with dates that overlap the specified route segment.
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <param name="segmentId"></param>
/// <returns></returns>
QList<RouteData> SQL::getConflictingRouteSegments(qint32 route, QString name, QString startDate, QString endDate, qint32 segmentId)
{
    QList<RouteData> myArray;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select a.route, name, startDate, endDate, lineKey, tractionType, companyKey, direction, normalEnter, normalLeave, reverseEnter, reverseLeave, routeAlpha, OneWay"
            " from Routes a join altRoute b on a.route = b.route where (startDate between '" + startDate + "' and '" + endDate + "' or endDate between '" + startDate + "' and '" + endDate + "') and a.route = " + QString("%1").arg(route) + " and name = '" + name + "' and endDate <> '" + endDate + "' and lineKey = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            RouteData rd = RouteData();
            rd.route = query.value(0).toInt();
            rd.name = query.value(1).toString();
            rd.startDate = query.value(2).toDate();
            rd.endDate = query.value(3).toDate();
            rd.lineKey = query.value(4).toInt();
            rd.tractionType =query.value(5).toInt();
            rd.companyKey = query.value(6).toInt();
            rd.direction = query.value(7).toString();
            rd.normalEnter = query.value(8).toInt();
            rd.normalLeave = query.value(9).toInt();
            rd.reverseEnter = query.value(10).toInt();
            rd.reverseLeave = query.value(11).toInt();
            rd.oneWay = query.value(12).toString();
            myArray.append(rd);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }

    return myArray;
}
bool compareSegmentData(const SegmentData & sd1, const SegmentData & sd2)
{
 return sd1.segmentId() < sd2.segmentId();
}

QList<RouteData> SQL::getRoutes(qint32 segmentid)
{
    SegmentInfo si = getSegmentInfo(segmentid);  // get some info about the segment.
    QList<SegmentInfo>  myArray;
    //compareSegmentDataClass compareSegments = new compareSegmentDataClass();

    // get a list of all the routes using this segment on this date
    QList<RouteData> segmentInfoList = getRouteSegmentsBySegment(segmentid);

    return segmentInfoList;
}

QList<routeIntersects> SQL::updateLikeRoutes(qint32 segmentid, qint32 route, QString name, QString date, bool bAllRoutes)
{
    QList<routeIntersects> intersectList;
    routeIntersects ri;
    SegmentData sd = getSegmentData(segmentid);  // get some info about the segment.
    QList<SegmentData>  myArray;
    //compareSegmentDataClass compareSegments = new compareSegmentDataClass();

    // get a list of all the routes using this segment on this date
    QList<RouteData> segmentInfoList = getRouteSegmentsBySegment(segmentid);

    // get other segments intersecting with the start of this segment
    myArray = getIntersectingSegments(sd._startLat, sd._startLon, .020, sd._routeType);
    //foreach (segmentData sd1 in myArray)
    for(int i=0; i < myArray.count(); i++)
    {
        SegmentData sd1 = myArray.at(i);
        if (sd1._segmentId == segmentid)
            continue;   // ignore myself
        // select only those segments actually used.
        if (bAllRoutes || isRouteUsedOnDate(route, sd1._segmentId, date))
            ri.startIntersectingSegments.append(sd1);
    }
    //ri.startIntersectingSegments.Sort(compareSegments);
    qSort(ri.startIntersectingSegments.begin(), ri.startIntersectingSegments.end(), compareSegmentData);

    // get other segments intersecting with the end of this segment
    myArray = getIntersectingSegments(sd._endLat, sd._endLon, .020, sd._routeType);
    //foreach (segmentData sd2 in myArray)
    for(int i=0; i < myArray.count(); i++)
    {
        SegmentData sd2 = myArray.at(i);
        if (sd2._segmentId == segmentid)
            continue;   // ignore myself
        // select only those segments actually used
        if (isRouteUsedOnDate(route, sd2._segmentId, date))
            ri.endIntersectingSegments.append(sd2);
    }
    //ri.endIntersectingSegments.Sort(compareSegments);
    qSort(ri.endIntersectingSegments.begin(), ri.endIntersectingSegments.end(), compareSegmentData);

    // Populate the intersect list
    //foreach (routeData rdi in segmentInfoList)
    for(int i =0; i < segmentInfoList.count(); i++)
    {
        RouteData rdi = segmentInfoList.at(i);
        // Get my info
        if (rdi.route == route && rdi.name == name && rdi.endDate.toString("yyyy/MM/dd") == date)
            ri.rd = rdi;
        else
        {
            routeIntersects ri2 =  routeIntersects();
            ri2.rd = rdi;
            myArray = getIntersectingSegments(sd._startLat, sd._startLon, .020, sd._routeType);
            //foreach (segmentData sd1 in myArray)
            for(int i=0; i < myArray.count(); i++)
            {
                SegmentData sd1 = myArray.at(i);
                //foreach (segmentData mysd1 in ri.startIntersectingSegments)
                for(int j=0; j < ri.startIntersectingSegments.count(); j++)
                {
                    SegmentData mysd1 = ri.startIntersectingSegments.at(j);
                    if (sd1._segmentId == mysd1._segmentId )
                    {
                        // select only those segments actually used.
                        if (isRouteUsedOnDate(rdi.route, sd1._segmentId, rdi.endDate.toString("yyyy/MM/dd")))
                            ri2.startIntersectingSegments.append(sd1);
                    }
                }
            }
            myArray = getIntersectingSegments(sd._endLat, sd._endLon, .020, sd._routeType);
            //foreach(segmentData sd2 in myArray)
            for(int i=0; i < myArray.count(); i++)
            {
                SegmentData sd2 = myArray.at(i);
                //foreach (segmentData mysd2 in ri.endIntersectingSegments)
                for(int j=0; j < ri.endIntersectingSegments.count(); j++)
                {
                     SegmentData mysd2 = ri.endIntersectingSegments.at(j);
                    if (sd2._segmentId == mysd2._segmentId )
                    {
                        // select only those segments actually used
                        if (isRouteUsedOnDate(rdi.route, sd2._segmentId, rdi.endDate.toString("yyyy/MM/dd")))
                            ri2.endIntersectingSegments.append(sd2);
                    }
                }
            }
            //ri2.startIntersectingSegments.Sort(compareSegments);
            qSort(ri2.startIntersectingSegments.begin(), ri2.startIntersectingSegments.end(), compareSegmentData);
            //ri2.endIntersectingSegments.Sort(compareSegments);
            qSort(ri2.endIntersectingSegments.begin(), ri2.endIntersectingSegments.end(), compareSegmentData);
            if(ri2.endIntersectingSegments.count() == ri.endIntersectingSegments.count() &&
                    ri2.startIntersectingSegments.count() == ri.startIntersectingSegments.count())
            intersectList.append(ri2);
        }
    }
    return intersectList;
}
SegmentData SQL::getSegmentInOppositeDirection(SegmentData sdIn)
{
    SegmentData sd;
    QList<SegmentData> startIntersects = getIntersectingSegments(sdIn._startLat, sdIn._startLon, .020, sdIn._routeType);
    Bearing b = Bearing();

    QList<SegmentData> endIntersects = getIntersectingSegments(sdIn._endLat, sdIn._endLon, .020, sdIn._routeType);
    //foreach (segmentData sdStart in startIntersects)
    for(int i=0; i < startIntersects.count(); i++)
    {
        SegmentData sdStart = startIntersects.at(i);
        if (sdStart._segmentId == sdIn._segmentId)
            continue;
        //foreach (segmentData sdEnd in endIntersects)
        for(int j=0; j < endIntersects.count(); j++)
        {
            SegmentData sdEnd = endIntersects.at(j);
            if (sdEnd._segmentId == sdIn._segmentId)
                continue;
            b =  Bearing(sdEnd._startLat, sdEnd._startLon, sdEnd._endLat, sdEnd._endLon);
            if (sdStart._segmentId == sdEnd._segmentId && sdIn._bearing.getDirection() != b.getDirection() )
            {
                SegmentData sdWrk = getSegmentData(sdEnd._segmentId);
                if(sdWrk._oneWay == sdIn._oneWay)
                {
                    sd = sdWrk;
                    break;
                }
            }
        }
    }
    return sd;
}

bool SQL::isRouteUsedOnDate(qint32 route, qint32 segmentId,  QString date)
{
    bool ret = false;
    int count = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select count(*) from Routes where route = " + QString("%1").arg(route) + " and lineKey= " + QString("%1").arg(segmentId) + " and '"+ date + "' between startDate and endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        if (query.isActive() == false)
        {
            return false;
        }
        while (query.next())
        {
            count = query.value(0).toInt();
        }
        if (count > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

QList<StationInfo> SQL::getStations(qint32 route, QString name, QString date)
{
 bool bZeroRoutes = false;
 QList<StationInfo> myArray;
 if(!dbOpen())
    throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 //QSqlQuery query2 = QSqlQuery(db);
 //QSqlQuery query3 = QSqlQuery(db);
 StationInfo sti;
 //int pointsCount = 0;
 bool bQuery;

 // Begin conversion code
#if 0
 // create temp table
 commandText = "Create Temporary Table IF NOT EXISTS  `t_points` (latitude, longitude, segmentId, point, type, route)";
 if(!query2.exec(commandText))
 {
  SQLERROR(query2);
  return myArray;
 }
 commandText = "select r.lineKey, s.pointArray, r.route, s.type from Routes r join Segments s on r.lineKey = s.segmentId";
 if(!query2.exec(commandText))
 {
  SQLERROR(query2);
  return myArray;
 }
 while(query2.next())
 {
  segmentInfo si;
  LatLng ll;
  int route, type;
  sd.SegmentId = query2.value(0).toInt();
  si.setPoints(query2.value(1).toString());
  route = query2.value(2).toInt();
  si.routeType = (RouteType)query2.value(3).toInt();
  int point;
  for(int i = 0; i < si.pointList.count(); i++)
  {
   ll = si.pointList.at(i);
   point = i;

   commandText = QString("insert into`t_points` (latitude, longitude, segmentId, point, type, route) values(%1,%2,%3,%4,%5,%6)").arg(ll.lat()).arg(ll.lon()).arg(si.SegmentId).arg(point).arg(si.routeType).arg(route);
   if(!query3.exec(commandText))
   {
    SQLERROR(query3);
    return myArray;
   }
   pointsCount++;
  }
 }

 commandText = "select stationKey, latitude, longitude, name from Stations where segmentId < 0";
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return myArray;
 }
 while(query.next())
 {
  sti.stationKey = query.value(0).toInt();
  sti.latitude = query.value(1).toDouble();
  sti.longitude = query.value(2).toDouble();
  sti.stationName = query.value(3).toString();
  sti.segmentId = -1;

  commandText = QString("select latitude, longitude, segmentId, point, type, route from t_points where distance(latitude, longitude, %1, %2) < .020").arg(sti.latitude).arg(sti.longitude);
  if(!query2.exec(commandText))
  {
   SQLERROR(query2);
   return myArray;
  }

  while(query2.next())
  {
   double latitude = query2.value(0).toDouble();
   double longitude = query2.value(1).toDouble();
   int segmentId = query2.value(2).toInt();
   int point = query2.value(3).toInt();
   int type = query2.value(4).toInt();
   int route = query2.value(5).toInt();
   commandText = QString("update Stations set route = %1, segmentId =%2, point= %3, routeType =%4 where stationKey = %5").arg(route).arg(segmentId).arg(point).arg(type).arg(sti.stationKey);
   if(!query3.exec(commandText))
   {
    SQLERROR(query3);
    return myArray;
   }
  }
 }
#endif
 // end conversion code

// if(config->currConnection->servertype() != "MsSql")
//    commandText =
//    "SELECT stationKey, a.name, latitude, longitude, b.`key` , infoKey, b.segmentId, a.startDate, a.endDate, geodb_loc_id, c.route, r.routeAlpha, sr.type, b.sequence, a.segmentId, a.markerType " \
//            "FROM Stations a " \
//            "JOIN LineSegment b ON ( distance( latitude, longitude, b.startlat, b.startlon ) < .020 " \
//            "OR distance( latitude, longitude, b.endlat, b.endlon ) < .020 ) " \
//            "JOIN Routes c ON c.lineKey = b.segmentId " \
//            "JOIN altRoute r ON c.route = r.route " \
//            "JOIN Segments sr ON c.lineKey = sr.segmentid "\
//            "where r.route = " + QString("%1").arg(route) + " and c.name = '" + name + "'  and '" + date + "' between c.startDate and c.endDate  and '" + date + "' between a.startDate and a.endDate " \
//            "and a.endDate and a.routeType = sr.type " \
//            "GROUP BY stationKey, a.name, latitude, longitude, infoKey, a.startDate, a.endDate, geodb_loc_id, a.route, r.routeAlpha, sr.type";
//else
    commandText =
            "SELECT a.stationKey, a.name, latitude, longitude,  a.infoKey, a.segmentId, a.startDate, a.endDate, geodb_loc_id,"
            " a.route, r.routeAlpha, a.routeType, a.markerType " \
            "FROM Stations a " \
            "JOIN Routes c ON c.route = " + QString("%1").arg(route) + " " \
            "JOIN altRoute r ON r.route = c.route " \
            "JOIN Segments sr ON c.lineKey = sr.segmentid "\
            "where c.route = " + QString("%1").arg(route) + " and c.name = '" + name + "'  and '" + date + "' between c.startDate and c.endDate  and '" + date + "' between a.startDate and a.endDate " \
            "and a.routeType = sr.type and a.segmentid = c.lineKey " \
            "GROUP BY stationKey, a.name, a.latitude, a.longitude, a.infoKey, a.startDate, a.endDate, a.geodb_loc_id, a.route, r.routeAlpha, a.routeType, a.segmentId";
//qDebug()<< commandText + "\n";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
    SQLERROR(query);
    //db.close();
    //exit(EXIT_FAILURE);
    return myArray;
 }

 if (!query.isActive())
 {
    return myArray;
 }
 while (query.next())
 {
    StationInfo sti =  StationInfo();
    sti.stationKey = query.value(0).toInt();
    sti.stationName = query.value(1).toString();
    sti.latitude = query.value(2).toDouble();
    sti.longitude = query.value(3).toDouble();
    if(query.value(4).isNull())
        sti.infoKey = -1;
    else
        sti.infoKey = query.value(4).toInt();
    sti.segmentId = query.value(5).toInt();
    if(sti.segmentId < 0)
        bZeroRoutes = true;

    if (query.value(6).isNull())
        sti.startDate = QDateTime();
    else
        sti.startDate = query.value(6).toDateTime();
    if (query.value(7).isNull())
        sti.endDate = QDateTime::fromString("3000-01-01", "yyyy/MM/dd");
    else
        sti.endDate = query.value(7).toDateTime();
    if(!query.value(8).isNull())
        sti.geodb_loc_id = query.value(8).toInt();
    sti.route = query.value(9).toInt();
    if(sti.route < 1)
        bZeroRoutes = true;
    sti.alphaRoute = query.value(10).toString();
    sti.routeType = (RouteType)query.value(11).toInt();
    sti.markerType= query.value(12).toString();
    myArray.append(sti);
 }
 return myArray;
}

QList<StationInfo> SQL::getStations()
{
 QList<StationInfo> myArray;
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
     commandText = "SELECT stationKey, a.name, latitude, longitude, a.SegmentId, a.point, infoKey, a.startDate, "
                   "a.endDate, geodb_loc_id, routeType, suffix "
                   "from Stations a  GROUP BY stationKey, a.name, latitude, longitude, infoKey, "
                   "a.startDate, a.endDate, geodb_loc_id, routeType";
 else
     commandText = "SELECT stationKey, a.name, latitude, longitude, a.SegmentId, a.point, infoKey, a.startDate, "
                   "a.endDate, geodb_loc_id, routeType, suffix from Stations a  GROUP BY stationKey, a.name, latitude, longitude, infoKey, "
                   "a.startDate, a.endDate, geodb_loc_id, routeType";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(query);
     //db.close();
     //exit(EXIT_FAILURE);
     return myArray;
 }
 if (!query.isActive())
 {
     return myArray;
 }
 while (query.next())
 {
     StationInfo sti =  StationInfo();
     sti.stationKey = query.value(0).toInt();
     sti.stationName = query.value(1).toString();
     sti.latitude = query.value(2).toDouble();
     sti.longitude = query.value(3).toDouble();
     sti.segmentId = query.value(4).toInt();
     sti.point = query.value(5).toInt();
     if (query.value(6).isNull())
         sti.infoKey = -1;
     else
         sti.infoKey = query.value(6).toInt();
     if (query.value(7).isNull())
         sti.startDate = QDateTime();
     else
         sti.startDate = query.value(7).toDateTime();
     if (query.value(8).isNull())
         sti.endDate = QDateTime::fromString("3000-01-01");
     else
         sti.endDate = query.value(8).toDateTime();
     if (!query.value(9).isNull())
         sti.geodb_loc_id = query.value(9).toInt();
     //            sti.routeType = (RouteType)query.value(9).toInt();
     //            sti.segmentId = query.value(10).toInt();
     //            sti.route = query.value(11).toInt();
     //            if(query.value(12).isNull())
     //            {
     //                sti.alphaRoute = "";
     //            }
     //            else
     //            {
     //                sti.alphaRoute = query.value(12).toString();
     //            }
     sti.routeType = (RouteType)query.value(10).toInt();
     sti.stationSuffix = query.value(11).toString();
     myArray.append(sti);
 }
 return myArray;
}

CommentInfo SQL::getComments(qint32 infoKey)
{
//#ifdef WIN32
//    QString up = QString::fromUtf8("?");
//    QString down = QString::fromUtf8("?");
//#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
//#endif

    CommentInfo ci;
    ci.commentKey = -1;
    ci.comments = "";
    ci.tags = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT commentKey, comments, tags from Comments where commentKey = " + QString("%1").arg(infoKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return ci;
        }
        while (query.next())
        {
            ci.commentKey = query.value(0).toInt();
//            if(config->currConnection->servertype() != "MsSql")
            ci.comments = query.value(1).toString();
//            else
            {
                //ci.comments = query.value(1).toString().toUtf8();
                //qDebug()<<ci.comments;
                ci.comments.replace("&up", up);
                ci.comments.replace("&down", down);
                //qDebug()<<ci.comments;
            }
            ci.tags = query.value(2).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ci;
}
int SQL::addComment(QString comments, QString tags)
{
    int infoKey = -1;
//#ifdef WIN32
//    QString up = QString::fromUtf8("?");
//    QString down = QString::fromUtf8("?");
//#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
//#endif

    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        BeginTransaction("addComment");

        //if(config->currConnection->servertype() != "MySql")
        {
            comments = comments.replace(up, "&up");
            comments = comments.replace(down, "&down");
            //comments = comments.toLatin1();
        }

        QString commandText = "insert into Comments (comments, tags ) values(:comments,'" + tags + "')";
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":comments", comments);
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        //int rows = query.numRowsAffected();
        //if (rows == 1)
        {
            if(config->currConnection->servertype() == "Sqlite")
            {
                commandText = "SELECT LAST_INSERT_ROWID()";

            }
            else
            if(config->currConnection->servertype() != "MsSql")
            {
                commandText = "SELECT LAST_INSERT_ID()";
            }
            else
            {
                commandText = "SELECT IDENT_CURRENT('comments')";
            }
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
               QSqlError err = query.lastError();
               qDebug() << err.text() + "\n";
               qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
               db.close();
               exit(EXIT_FAILURE);
            }
            while (query.next())
            {
                infoKey = query.value(0).toInt();
            }
            CommitTransaction("addComment");
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return infoKey;
}

bool SQL::updateComment(qint32 infoKey, QString comments, QString tags)
{
//#ifdef WIN32
//    QString up = QString::fromUtf8("?");
//    QString down = QString::fromUtf8("?");
//#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
//#endif

    int rows = 0;
    bool rtn = false;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        //if(config->currConnection->servertype() != "MySql")
        {
            comments = comments.replace(up, "&up");
            comments = comments.replace(down, "&down");
            //comments = comments.toLatin1();
        }


        QString commandText = "update  Comments set comments=:comments, tags ='" + tags + "',lastUpdate=:lastUpdate where commentKey = " + QString("%1").arg(infoKey);
        QSqlQuery query = QSqlQuery(db);
        query.prepare(commandText);
        query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
        query.bindValue(":comments", comments);
        bool bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            rtn = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return rtn;
}

bool SQL::deleteComment(qint32 infoKey)
{
    int rows = 0;
    bool rtn = false;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "delete from Comments where commentKey = " + QString("%1").arg(infoKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            rtn = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return rtn;
}

// get the earliest comment later than date
QDate SQL::getFirstCommentDate(qint32 route, QDate date, qint32 companyKey)
{
 QDate result = date;
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "select min(date) from RouteComments where route in(0," + QString("%1").arg(route)
   + ") and companyKey = " + QString("%1").arg(companyKey)
   + " and date >= '" + QString("%1").arg(date.toString("yyyy/MM/dd"))
   + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QSqlError err = query.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     db.close();
     exit(EXIT_FAILURE);
 }
 if (!query.isActive())
 {
     return result;
 }
 while (query.next())
 {
  result = query.value(0).toDate();
 }
 return result;
}

RouteComments SQL::getRouteComment(qint32 route, QDate date, qint32 companyKey)
{
//#ifdef WIN32
//    QString up = QString::fromUtf8("?");
//    QString down = QString::fromUtf8("?");
//#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
//#endif

    RouteComments rc;
    rc.route = -1;
    rc.infoKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    rc.companyKey = companyKey;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT rc.commentKey, c.commentKey, rc.companyKey, comments, tags, rc.latitude, "
            "rc.longitude, r.name, a.routeAlpha "
            "from Comments c "
            "join RouteComments rc on rc.commentKey = c.commentKey "
            "join routes r on r.route = rc.route and :date between r.startDate and r.endDate "
            "JOIN altRoute a ON a.route = rc.route "
            "where rc.route = :route and rc.date = :date ";

        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.prepare(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        query.bindValue(":route", route);
        query.bindValue(":date", date.toString("yyyy/MM/dd"));
        bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.infoKey = query.value(0).toInt();
            rc.ci.commentKey = query.value(1).toInt();
            rc.companyKey = query.value(2).toInt();
            rc.ci.comments = query.value(3).toString();
            rc.ci.tags = query.value(4).toString();
            rc.pos = LatLng(query.value(5).toDouble(), query.value(6).toDouble());
            //if(config->currConnection->servertype() != "MySql")
            {
                {
                    //qDebug()<<rc.ci.comments;
                    rc.ci.comments.replace("&up", up);
                    rc.ci.comments.replace("&down", down);
                    rc.ci.comments.replace("&amp;up", up);
                    rc.ci.comments.replace("&amp;down", down);
                    //qDebug()<<rc.ci.comments;
                }
            }
            rc.name = query.value(7).toString();
            rc.routeAlpha = query.value(8).toString();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    rc.date = date;
    rc.route = route;
    QTextEdit* te = new QTextEdit();
    te->setHtml(rc.ci.comments);
    if(te->toPlainText().isEmpty())
     rc.ci.comments = "";
    return rc;
}

bool SQL::updateRouteComment(RouteComments rc)
{
    bool ret = false;
    bool bQuery;
    QString commandText;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query = QSqlQuery(db);
        if(rc.infoKey == -1)
        {
            db.transaction();
            //qDebug()<< rc.ci.comments;

            rc.ci.commentKey = addComment(rc.ci.comments, rc.ci.tags);

            commandText = "insert into RouteComments (route, date, commentKey, companyKey, latitude, longitude) "
            " values(:route, :date, :commentKey, :companyKey, :latitude, :longitude)";
            bQuery = query.prepare(commandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            query.bindValue(":route", rc.route);
            query.bindValue(":date", rc.date.toString("yyyy/MM/dd"));
            query.bindValue(":commentKey",rc.ci.commentKey );
            query.bindValue(":companyKey", rc.companyKey);
            query.bindValue(":latitude", rc.pos.lat());
            query.bindValue(":longitude", rc.pos.lon());

            bQuery = query.exec();
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }

            db.commit();
            ret = true;
        }
        else
        {
            commandText = "update RouteComments set companyKey = :companyKey, latitude = :latitude, longitude=:longitude, lastUpdate=:lastUpdate  where route = :route and date = :date";
            bQuery = query.prepare(commandText);
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }
            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
            query.bindValue(":route", rc.route);
            query.bindValue(":date", rc.date.toString("yyyy/MM/dd"));
            //query.bindValue(":commentKey",rc.ci.infoKey );
            query.bindValue(":companyKey", rc.companyKey);
            query.bindValue(":latitude", rc.pos.lat());
            query.bindValue(":longitude", rc.pos.lon());
            bQuery = query.exec();
            if(!bQuery)
            {
                QSqlError err = query.lastError();
                qDebug() << err.text() + "\n";
                qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
                db.close();
                exit(EXIT_FAILURE);
            }


            ret = updateComment(rc.ci.commentKey, rc.ci.comments, rc.ci.tags);
        }

    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

// get count of stations or route comments using this html comment
int SQL::countCommentUsers(int commentKey)
{
 int ret =0;
 bool bQuery;
 QString commandText;
 try
 {
     if(!dbOpen())
         throw Exception(tr("database not open: %1").arg(__LINE__));
     QSqlDatabase db = QSqlDatabase::database();
     QSqlQuery query = QSqlQuery(db);
     commandText = "select count(*) from stations where commentKey = " + QString::number(commentKey);
     bQuery = query.prepare(commandText);
     if(!bQuery)
     {
         QSqlError err = query.lastError();
         qDebug() << err.text() + "\n";
         qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
         db.close();
         exit(EXIT_FAILURE);
     }
     bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QSqlError err = query.lastError();
         qDebug() << err.text() + "\n";
         qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
         db.close();
         exit(EXIT_FAILURE);
     }
     while (query.next())
     {
      ret += query.value(0).toInt();
     }

     // now check RouteComments

     commandText = "select count(*) from routeComments where commentKey = " + QString::number(commentKey);
     bQuery = query.prepare(commandText);
     if(!bQuery)
     {
         QSqlError err = query.lastError();
         qDebug() << err.text() + "\n";
         qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
         db.close();
         exit(EXIT_FAILURE);
     }
     bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QSqlError err = query.lastError();
         qDebug() << err.text() + "\n";
         qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
         db.close();
         exit(EXIT_FAILURE);
     }
     while (query.next())
     {
      ret += query.value(0).toInt();
     }
 }
 catch (Exception e)
 {
     myExceptionHandler(e);
 }
 return ret;
}

bool SQL::deleteRouteComment(RouteComments rc)
{
    bool ret = false;
    bool bQuery;
    QString commandText;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query = QSqlQuery(db);

        BeginTransaction("deleteRouteComment");

        commandText = "delete from RouteComments where route = :route and date = :date";
        bQuery = query.prepare(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        query.bindValue(":route", rc.route);
        query.bindValue(":date", rc.date.toString("yyyy/MM/dd"));
        bQuery = query.exec();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        int count = countCommentUsers(rc.ci.commentKey);
        if(count ==0)
        {
         if(deleteComment(rc.ci.commentKey))
         {
          CommitTransaction("deleteRouteComment");
          ret = true;
         }
        }
        else
         qDebug() << count  << "stations or route comments still use this comment ";
   }
   catch (Exception e)
   {
       myExceptionHandler(e);
   }
    RollbackTransaction("deleteRouteComment");
   return ret;
}

RouteComments SQL::getNextRouteComment(qint32 route, QDate date, qint32 companyKey)
{
#ifdef WIN32
    QString up = QString::fromUtf8("?");
    QString down = QString::fromUtf8("?");
#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
#endif

    RouteComments rc;
    rc.route = -1;
    rc.infoKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    rc.companyKey = companyKey;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT rc.commentKey, comments, tags, date, rc.companyKey, r.name,"
                " a.routeAlpha, rc.latitude, rc.longitude "
                "from RouteComments rc "
                "join Comments c on rc.commentKey = c.commentKey "
                "JOIN altRoute a ON a.route = rc.route "
                "join routes r on r.route = rc.route and '"+date.toString("yyyy/MM/dd")+"' between r.startDate and r.endDate "
                "where rc.route = "+ QString("%1").arg(route) +" "
                "and rc.date  > '" + date.toString("yyyy/MM/dd")+"'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        //qDebug()<< query.lastQuery();
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.infoKey = query.value(0).toInt();
            rc.ci.comments = query.value(1).toString();
            rc.ci.tags = query.value(2).toString();
            rc.date = query.value(3).toDate();
            rc.companyKey = query.value(4).toInt();
            //if(config->currConnection->servertype() != "MySql")
            {
                {
                    //qDebug()<<rc.ci.comments;
                    rc.ci.comments = rc.ci.comments.replace("&up", up);
                    rc.ci.comments = rc.ci.comments.replace("&down", down);
                    rc.ci.comments.replace("&amp;up", up);
                    rc.ci.comments.replace("&amp;down", down);

                    //qDebug()<<rc.ci.comments;
                }
            }
            rc.name = query.value(5).toString();
            rc.routeAlpha = query.value(6).toString();
            rc.pos = LatLng(query.value(7).toDouble(), query.value(8).toDouble());

        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    rc.route = route;
    return rc;
}

RouteComments SQL::getPrevRouteComment(qint32 route, QDate date, qint32 companyKey)
{
    Q_UNUSED(companyKey)
#ifdef WIN32
    QString up = QString::fromUtf8("?");
    QString down = QString::fromUtf8("?");
#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
#endif

    RouteComments rc;
    rc.route =-1;
    rc.infoKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT rc.commentKey, comments, tags, date, rc.companyKey, r.name,"
                " a.routeAlpha, rc.latitude, rc.longitude "
                "from RouteComments rc "
                "join Comments c on rc.commentKey = c.commentKey "
                "JOIN altRoute a ON a.route = rc.route "
                "join routes r on r.route = rc.route and '"+date.toString("yyyy/MM/dd")+"' between r.startDate and r.endDate "
                "where rc.route = "+ QString("%1").arg(route) +" "
                "and rc.date  < '" + date.toString("yyyy/MM/dd")+"'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.infoKey = query.value(0).toInt();
            rc.ci.comments = query.value(1).toString();
            rc.ci.tags = query.value(2).toString();
            rc.date = query.value(3).toDate();
            rc.companyKey = query.value(4).toInt();
            //if(config->currConnection->servertype() != "MySql")
            {
                {
                    //qDebug()<<rc.ci.comments;
                    rc.ci.comments = rc.ci.comments.replace("&up", up);
                    rc.ci.comments = rc.ci.comments.replace("&down", down);
                    rc.ci.comments.replace("&amp;up", up);
                    rc.ci.comments.replace("&amp;down", down);

                    //qDebug()<<rc.ci.comments;
                }
            }
            rc.name = query.value(5).toString();
            rc.routeAlpha = query.value(6).toString();
            rc.pos = LatLng(query.value(7).toDouble(),query.value(8).toDouble());
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    rc.route = route;
    return rc;
}

CommentInfo SQL::getComment(qint32 commentKey, int pos)
{
#ifdef WIN32
    QString up = QString::fromUtf8("?");
    QString down = QString::fromUtf8("?");
#else
    QString up = QString::fromUtf8("▲");
    QString down = QString::fromUtf8("▼");
#endif

    CommentInfo ci;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(pos >0)
         commandText = "select commentKey, comments, tags from Comments where commentKey > " + QString::number(commentKey)+ " limit 1";
           else
         commandText = "select commentKey, comments, tags from Comments where commentKey < " + QString::number(commentKey)+ " limit 1";


        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return ci;
        }
        while (query.next())
        {
         ci.commentKey = query.value(0).toInt();
         ci.comments = query.value(1).toString();
         {
             //qDebug()<<rc.ci.comments;
             ci.comments = ci.comments.replace("&up", up);
             ci.comments = ci.comments.replace("&down", down);
             ci.comments.replace("&amp;up", up);
             ci.comments.replace("&amp;down", down);

             //qDebug()<<rc.ci.comments;
         }
         ci.tags = query.value(2).toString();
        }

        // get usage by Station
        commandText = "select stationKey from Stations where infoKey >= 0";
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return ci;
        }
        while (query.next())
        {
         ci.usedByStations.append(query.value(0).toString());
        }

        // get usage by Routes
        commandText = "select route from routeComments"
                      " where commentkey = " + QString::number(ci.commentKey);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }

        if (!query.isActive())
        {
            return ci;
        }
        while (query.next())
        {
         ci.usedByRoutes.append(query.value(0).toString());
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ci;
}

#if 0
qint32 SQL::addStation(QString name, LatLng location, qint32 lineSegmentId, RouteType type)
{
   int stationKey = -1;
   int rows = -1;
   try
   {
       if(!dbOpen())
           throw Exception(tr("database not open: %1").arg(__LINE__));
       QSqlDatabase db = QSqlDatabase::database();


       QString commandText = " insert into Stations (name, latitude, longitude, lineSegmentId, routeType) values ('" + name + "', " + QString("%1").arg(location.lat(),0,'f',8) + "," + QString("%1").arg(location.lon(),0,'f',8) + "," + QString("%1").arg(lineSegmentId) + "," + QString("%1").arg((int)type)+ ") ";
       QSqlQuery query = QSqlQuery(db);
       bool bQuery = query.exec(commandText);
       if(!bQuery)
       {
           QSqlError err = query.lastError();
           qDebug() << err.text() + "\n";
           qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
           db.close();
           exit(EXIT_FAILURE);
       }
       rows = query.numRowsAffected();
       //if (rows == 1)
       {
           //myCommand.commandText = "SELECT stationKey from Stations where name = '" + name + "'";
           if(config->currConnection->servertype() == "Sqlite")
               commandText = "SELECT LAST_INSERT_ROWID()";
           else
           if(config->currConnection->servertype() != "MsSql")
               commandText = "SELECT LAST_INSERT_ID()";
           else
               commandText = "SELECT IDENT_CURRENT('dbo.stations')";
           bQuery = query.exec(commandText);
           if(!bQuery)
           {
              QSqlError err = query.lastError();
              qDebug() << err.text() + "\n";
              qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
              db.close();
              exit(EXIT_FAILURE);
           }
           while (query.next())
           {
               stationKey = query.value(0).toInt();
           }
       }
   }
   catch (Exception e)
   {
       myExceptionHandler(e);
   }
   return stationKey;

}
#endif
#if 0
qint32 SQL::addStation(QString name, LatLng location, qint32 lineSegmentId, QString startDate, QString endDate, qint32 geodb_loc_id, RouteType routeType, QString markerType)
{
   int stationKey = -1;
   int rows = -1;
   try
   {
       if(!dbOpen())
           throw Exception(tr("database not open: %1").arg(__LINE__));
       QSqlDatabase db = QSqlDatabase::database();


       QString commandText = " insert into Stations (name, latitude, longitude, SegmentId, " \
         "startDate, endDate, geodb_loc_id, routeType) " \
         "values ('" + name + "', " + QString("%1").arg(location.lat(),0,'f',8) + "," +
         QString("%1").arg(location.lon(),0,'f',8)+ "," + QString("%1").arg(lineSegmentId) +
         ",'" + startDate + "','" + endDate + "', " + QString("%1").arg(geodb_loc_id) + "," +
         QString("%1").arg((int)routeType) + ",'"+ markerType+ "'') ";
       QSqlQuery query = QSqlQuery(db);
       bool bQuery = query.exec(commandText);
       if(!bQuery)
       {
           SQLERROR(query);
           return -1;
       }
       rows = query.numRowsAffected();
       //if (rows == 1)
       {
           //myCommand.commandText = "SELECT stationKey from Stations where name = '" + name + "'";
           if(config->currConnection->servertype() == "Sqlite")
               commandText = "SELECT LAST_INSERT_ROWID()";
           else
           if(config->currConnection->servertype() != "MsSql")
               commandText = "SELECT LAST_INSERT_ID()";
           else
               commandText = "SELECT IDENT_CURRENT('dbo.stations')";
           bQuery = query.exec(commandText);
           if(!bQuery)
           {
               QSqlError err = query.lastError();
               qDebug() << err.text() + "\n";
               qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
               db.close();
               exit(EXIT_FAILURE);
           }
           while (query.next())
           {
               stationKey = query.value(0).toInt();
           }
       }
   }
   catch (Exception e)
   {
       myExceptionHandler(e);
   }
   return stationKey;

}
#endif
qint32 SQL::addStation(QString name, LatLng location, qint32 segmentId, QString startDate, QString endDate, qint32 geodb_loc_id, qint32 infoKey, RouteType routeType, QString markerType, int point)
{
   int stationKey = -1;
   int rows = -1;
   try
   {
       if(!dbOpen())
           throw Exception(tr("database not open: %1").arg(__LINE__));
       QSqlDatabase db = QSqlDatabase::database();

       QString commandText = "select stationKey from Stations where name ='"+ name + "' and startDate ='" + startDate + "' and endDate = '" + endDate + "'";
       QSqlQuery query = QSqlQuery(db);
       bool bQuery = query.exec(commandText);
       if(!bQuery)
       {
           SQLERROR(query);
           db.close();
           exit(EXIT_FAILURE);
       }
       while(query.next())
       {
        stationKey = query.value(0).toInt();
       }

       if(stationKey>0)
       {
           // maybe need to do an update here!
           return stationKey;
       }
       commandText = " insert into Stations (name, latitude, longitude, segmentId, startDate, endDate, " \
         "geodb_loc_id, routeType, markerType, infoKey, point) " \
         "values ('" + name + "', " + QString("%1").arg(location.lat(),0,'f',8) + "," +
         QString("%1").arg(location.lon(),0,'f',8)+ "," + QString("%1").arg(segmentId) + ",'" +
         startDate + "','" + endDate + "', " + QString("%1").arg(geodb_loc_id) + "," +
         QString("%1").arg((int)routeType)+",'" +QString("%1").arg(markerType)+ "',"+QString("%1").arg(infoKey)+
         "," + QString::number(point) +" )";
       bQuery = query.exec(commandText);
       if(!bQuery)
       {
           SQLERROR(query);
           db.close();
           exit(EXIT_FAILURE);
       }
       rows = query.numRowsAffected();
       //if (rows == 1)
       {
           //myCommand.commandText = "SELECT stationKey from Stations where name = '" + name + "'";
           if(config->currConnection->servertype() == "Sqlite")
               commandText = "SELECT LAST_INSERT_ROWID()";
           else
           if(config->currConnection->servertype() != "MsSql")
               commandText = "SELECT LAST_INSERT_ID()";
           else
               commandText = "SELECT IDENT_CURRENT('dbo.stations')";
           bQuery = query.exec(commandText);
           if(!bQuery)
           {
               SQLERROR(query);
               db.close();
               exit(EXIT_FAILURE);
           }
           while (query.next())
           {
               stationKey = query.value(0).toInt();
           }
       }
   }
   catch (Exception e)
   {
       myExceptionHandler(e);
   }
   return stationKey;

}

bool SQL::deleteStation(qint32 stationKey)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "delete from Stations where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
            ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

bool SQL::updateRoute(qint32 route, QString name, QString endDate, qint32 segmentId, qint32 next, qint32 prev, QString trackUsage)
{
 bool ret = false;
 int rows = 0;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "update Routes set next = " + QString("%1").arg(next)
             + ", trackUsage  = '" + trackUsage + "'"
             + ", prev=" + QString("%1").arg(prev)+ ",lastUpdate=:lastUpdate"
             " where route ="+QString("%1").arg(route)
             + " and name ='"+name+"' and endDate='"+endDate
             +"' and lineKey="+QString("%1").arg(segmentId);
 QSqlQuery query = QSqlQuery(db);
 query.prepare(commandText);
 query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
 bool bQuery = query.exec();
 if(!bQuery)
 {
     QSqlError err = query.lastError();
     qDebug() << err.text() + "\n";
     qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     db.close();
     exit(EXIT_FAILURE);
 }
 rows = query.numRowsAffected();
 if (rows > 0)
     ret = true;
 return ret;
}

int SQL::updateRouteDate(int segmentId, QString startDate, QString endDate)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "Update Routes set startDate = '" + startDate
             + "', endDate='" + endDate+ "'"
             " where lineKey =" +QString::number(segmentId);
 QSqlQuery query = QSqlQuery(db);
 if(!query.exec(commandText))\
 {
  SQLERROR(query);
  return -1;
 }
 return query.numRowsAffected();
}

int SQL::updateRouteSegment(int segmentId, QString startDate, QString endDate, int newSegmentId)
{
 if(endDate < startDate)
 {
  throw Exception("updateRouteSegment: end date not > start date");
 }
 if(newSegmentId < 0 || newSegmentId == segmentId)
 {
  throw Exception("updateRouteSegment: invalid new segment");
 }

 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "Update Routes set startDate = '" + startDate
             + "', endDate='" + endDate+ "'"
             + ", linekey = " +QString::number(newSegmentId)
             + " where lineKey =" +QString::number(segmentId);
 QSqlQuery query = QSqlQuery(db);
 if(!query.exec(commandText))\
 {
  SQLERROR(query);
  return -1;
 }
 return query.numRowsAffected();
}

QStringList SQL::showDatabases(QString connection, QString servertype)
{
 QStringList ret;
 QSqlDatabase db = QSqlDatabase::database(connection);

 if(servertype != "MsSql")
 {
  QString commandText = "show databases";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
    QSqlError err = query.lastError();
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    db.close();
    //exit(EXIT_FAILURE);
    return ret;
  }
  while(query.next())
  {
   ret.append(query.value(0).toString());
  }
 }
 return ret;
}

bool SQL::loadSqlite3Functions()
{
 bool ret=false;
  QSqlDatabase db = QSqlDatabase::database();
  QVariant v = db.driver()->handle();
  sqlite3 *handle = NULL;
  if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0)
  {
   // v.data() returns a pointer to the handle
   handle = *static_cast<sqlite3 **>(v.data());

   if (handle != 0)
   {
    const char * rslt = sqlite3_libversion();
    qDebug() << "sqlite3 lib version (from libsqlite3)" << QString(rslt) ;
    qDebug() << "sqlite3 version (from sqlite3.h)" << SQLITE_VERSION;
    if(QString(rslt) != SQLITE_VERSION)
    {\
      QMessageBox::warning(NULL, tr("Sqlite3 library mismatch"), tr("The Sqlite3 library version, '%1' does not match the version '%2 that the app was compiled with").arg(QString(rslt)).arg(SQLITE_VERSION));
    }
   }
  }
//#ifndef WIN32
  bool bLoadExtensionEnabled = false;
  if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0)
  {
   // v.data() returns a pointer to the handle
   sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
   if (handle != 0)
   { // check that it is not NULL
    QSqlQuery query = QSqlQuery(db);
    // test for presence of load_extension() function.
    if(!query.exec("Select load_extension('', '')"))
    {
     QSqlError err = query.lastError();
     SQLERROR(query);
     if(err.text().startsWith( "The specified module could not be found.")
             /*|| err.text().contains("undefined symbol:  Unable to fetch row")*/)
     {
         bLoadExtensionEnabled = true;
     }
     else
     if(/*err.text().contains("not authorized") ||*/err.text().contains("no such function" ))
     {
      QMessageBox::critical(NULL, tr("SQlite Error"), tr("The version of SQLITE being used does not support loading extension functions so some queries will not work properly\nIf you are using the system SQLite library, it must be recompiled with the SQLITE_ENABLE_LOAD_EXTENSION define. Otherwise the QSQLITE plugin must be recompiled with that define"));
      return false;
     }
    }
    if(!bLoadExtensionEnabled)
    {
     int result;
//     result = sqlite3_enable_load_extension(handle, 1);
     QString ver = sqlite3_libversion();
     sqlite3_db_config(handle, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, &result);
     if(result == 1  && QString(sqlite3_libversion()) != "3.27.2")
     {
      result = sqlite3_enable_load_extension(handle, 1);
      if (result != SQLITE_OK && sqlite3_libversion())
      {
        qWarning() << "Could not enable load extension function: " << QString::number(result);

 #if 1
        if(sqlite3_db_config(handle, SQLITE_DBCONFIG_ENABLE_FKEY)!= SQLITE_OK)
        qWarning() << "sqlite3_config failed";
      }
      else
      {
       bLoadExtensionEnabled = true;
      }
     }

#endif
    }
    if(!query.exec("PRAGMA foreign_keys"))
    {
     SQLERROR(query);
     return false;
     int foreign_key=0;
     if(query.isActive())
      foreign_key = query.value(0).toInt();
     if(foreign_key != 1)
      qWarning() << "foreign_keys not enabled";
    }

   }
   else
   {
    qCritical() << "Could not get sqlite handle";
   }
  }
  else
  {
   qDebug() << "handle variant returned typename " <<  v.typeName();
  }
#if 1
  if(db.driverName() == "QSQLITE")
  {
// TODO: parameterize these path names
#ifdef WIN32
  //QString extName = "sqlite3ext_sqlfun.dll";
   QString extName = "sqlfun.dll";
   //QString commandText = "select load_extension('c:\\projects\\sqlfun\\sqlite3ext_sqlfun.dll')";

//   QFileInfo info("sqlfun.dll");
//   if(!info.exists())
//    qDebug() << info.absoluteFilePath()+ " not found";
//   QString commandText = QString("select load_extension('%1', 'distance_init')").arg(info.absoluteFilePath().replace("/", "\\"));
     QString commandText = QString("select load_extension('%1', 'sqlite3_extension_init')").arg(extName);
     //QString commandText = "select load_extension('functions.dll', 'distance_init')";

#else
//   QString commandText = "select load_extension('/usr/local/lib/distance.so', 'distance_init')";
      QFileInfo info("libsqlfun.so");
      QString path;
      if(!info.exists())
       qDebug() << "functions/libfunctions.so not found";
      else
       path = info.absoluteFilePath();
      //path.replace(".so", "");
      QString commandText = "select load_extension('" + path +"', 'sqlite3_extension_init')";
#endif
   QSqlQuery query = QSqlQuery(db);
   bool bQuery = query.exec(commandText);
   if(!bQuery)
   {
    QSqlError err = query.lastError();
    SQLERROR(query);
    QMessageBox::information(NULL, "open sqlite", QString("error loading functions = %1").arg(err.text()));

    return ret;
   }
   ret=true;
  }
#endif
 return ret;
}
QStringList SQL::getTableList(QSqlDatabase db, QString dbType)
{
 return db.tables();
}

bool SQL::checkSegments()
{
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;
 QList<SegmentInfo> myArray;
 SegmentInfo sI;

 try
 {
  if(!dbOpen())
   throw Exception(tr("database not open: %1").arg(__LINE__));

  commandText = "Select SegmentId, description, OneWay, startDate, endDate, length, points, startLat, startLon, endLat, EndLon, type, tracks from Segments ";
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
//   db.close();
//   exit(EXIT_FAILURE);
   return false;
  }
  while (query.next())
  {
   sI = SegmentInfo();
   sI.segmentId = query.value(0).toInt();
   sI.description = query.value(1).toString();
   sI.oneWay = query.value(2).toString();
   sI.startDate = query.value(3).toDateTime().toString("yyyy/MM/dd") ;
   sI.endDate = query.value(4).toDateTime().toString("yyyy/MM/dd") ;
   sI.length = query.value(5).toDouble();
   sI.lineSegments = query.value(6).toInt();
   sI.startLat = query.value(7).toDouble();
   sI.startLon = query.value(8).toDouble();
   sI.endLat = query.value(9).toDouble();
   sI.endLon = query.value(10).toDouble();
   sI.bearing = Bearing(sI.startLat, sI.startLon, sI.endLat, sI.endLon);
   sI.direction = sI.bearing.strDirection();
   if (sI.oneWay == "N")
    sI.direction += "-" + sI.bearing.strReverseDirection();
   sI.routeType = (RouteType)query.value(11).toInt();
   sI.tracks = query.value(12).toInt();
   sI.checkTracks();

   myArray.append(sI);
  }
 }
 catch (Exception& e)
 {
     myExceptionHandler(e);
 }


 foreach(SegmentInfo si, myArray)
 {
  commandText = "select count(*) from LineSegment where SegmentId = " + QString("%1").arg(si.segmentId);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   db.close();
   exit(EXIT_FAILURE);
  }
  int count=0;
  while (query.next())
  {
   count = query.value(0).toInt();
  }
  if(count == 0)
  {
   // need to add a LineSegment
   commandText = "INSERT INTO LineSegment( StartLat, StartLon, EndLat, EndLon, StreetName, SegmentId, Sequence, Length)  VALUES( " + QString("%1").arg(si.startLat,0,'f',8) + ", " + QString("%1").arg(si.startLon,0,'f',8) + ", " + QString("%1").arg(si.endLat,0,'f',8) + ", " + QString("%1").arg(si.endLon,0,'f',8) + ", '" + "??" + "', " + QString("%1").arg(si.segmentId) + ", " + QString("%1").arg(0) + ", " + QString("%1").arg(si.length) + ")";
   bQuery = query.exec(commandText);
   if(!bQuery)
   {
    QSqlError err = query.lastError();
    qDebug() << err.text() + "\n";
    qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
    db.close();
    exit(EXIT_FAILURE);
   }
   qDebug() << QString("Segment %1 corrected").arg(si.segmentId);
  }
 }

 return true;
}

// Check table for column name
bool SQL::doesColumnExist(QString table, QString column)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;

 if(config->currConnection->servertype() == "Sqlite")
 {
  commandText = "PRAGMA table_info(" + table+")";
  query.prepare(commandText);
  //query.bindValue(":tName",table);
  bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   return false;
  }
  while(query.next())
  {
   QString col = query.value(1).toString();
   if(col == column)
    return true;
  }
 }
 else if(config->currConnection->servertype() == "MySql")
 {
  int count;
  commandText = "Select count(*) from information_schema.COLUMNS where table_schema ='" + config->currConnection->database() + "' and table_name = '" + table +"' and column_name = '" + column + "'";
  query.prepare(commandText);
//  query.bindValue(":tbName",table);
//  query.bindValue(":schema", db.databaseName());
//  query .bindValue(":col", column);
  bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   return false;
  }
  while(query.next())
  {
   count = query.value(0).toInt();
   if(count > 0)
    return true;
  }
  QString txt = query.executedQuery();
  return false;
 }
 else if(config->currConnection->servertype() == "MsSql")
 {
  int count;
  commandText = "Select count(*) from information_schema.COLUMNS where table_schema ='dbo' and table_name = '" + table +"' and column_name = '" + column + "'";
  query.prepare(commandText);
//  query.bindValue(":tbName",table);
//  query.bindValue(":schema", db.databaseName());
//  query .bindValue(":col", column);
  bQuery = query.exec();
  if(!bQuery)
  {
   QSqlError err = query.lastError();
   qDebug() << err.text() + "\n";
   qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
   return false;
  }
  while(query.next())
  {
   count = query.value(0).toInt();
   if(count > 0)
    return true;
  }
  QString txt = query.executedQuery();
  return false;
 }
 else
 {
  return false;
 }
}

bool SQL::doesConstraintExist(QString tbName, QString name)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;

 if(config->currConnection->servertype() == "Sqlite")
 {
  commandText = QString("select sql from sqlite_master where name = '%1'").arg(tbName);
  query.prepare(commandText);
  //query.bindValue(":tName",table);
  bQuery = query.exec();
  if(!bQuery)
  {
   SQLERROR(query);
   return false;
  }
  while(query.next())
  {
   QString sql = query.value(0).toString();
   if(sql.contains(QString("constraint main unique (`%1`").arg(name)))
    return true;
  }
 }
 else if(config->currConnection->servertype() == "MySql")
 {
 }
 else if(config->currConnection->servertype() == "MsSql")
 {
 }
 return false;
}

bool SQL::addColumn(QString tbName, QString name, QString type)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;

 if(config->currConnection->servertype() == "Sqlite" )
  commandText = "alter table '" + tbName + "' add column  '" + name + "' " + type +" ";
 else if(config->currConnection->servertype() == "MySql")
  commandText = "alter table " + tbName + " add column " + name + " " + type +"";
 else
  commandText = "alter table dbo." + tbName + " add " + name + " " + type +" ";
 query.prepare(commandText);
 //query.bindValue(":tbName",tbName);
 //query.bindValue(":column", name);
 bQuery = query.exec();
 if(!bQuery)
 {
  QSqlError err = query.lastError();
  qDebug() << err.text() + "\n";
  qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
  return false;
 }
 return true;
}

bool SQL::updateTractionType(qint32 tractionType, QString description, QString displayColor, int routeType, QSqlDatabase  db)
{
    bool ret = false;
    int rows = 0;
    QString commandText;
    try
    {
        int count =0;
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        //QSqlDatabase db = db;
        commandText = "select count(*) from TractionTypes where tractionType = " + QString("%1").arg(tractionType);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
         SQLERROR(query);
         return false;

        }
        if (!query.isActive())
        {
            qDebug() << "error selecting traction type info";
            return false;
        }
        while (query.next())
        {
            count = query.value(0).toInt();
        }
        if (count == 0)
        {
         commandText = "insert into TractionTypes (tractionType, description, displayColor, routeType) values (" + QString("%1").arg(tractionType) + ", '" + description.trimmed() + "', '" + displayColor + "', " + QString::number(routeType) + ")";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
             SQLERROR(query);
             return false;

            }
            rows = query.numRowsAffected();
            //if (rows == 1)
                ret = true;
        }
        else
        {
            commandText = "update TractionTypes set description = '" + description + "', displayColor = '" + displayColor + "', routeType=" + QString("%1").arg(routeType)  + ", lastUpdate=:lastUpdate where tractionType = " + QString("%1").arg(tractionType) ;
            query.prepare(commandText);
            qDebug()<<commandText;
            query.bindValue(":lastUpdate", QDateTime::currentDateTimeUtc());
            bQuery = query.exec();
            if(!bQuery)
            {
             SQLERROR(query);
             return false;
            }
            rows = query.numRowsAffected();
            //if (rows == 1)
                ret = true;
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

// check tables to see if alteratios need to be made
void SQL::checkTables(QSqlDatabase db)
{
 // check for presence of Parameters table.
 QStringList tableList;
 if(db.isOpen())
 {
  qDebug() << db.driverName() + "\n";
  qDebug() << "User:" + db.userName() + "\n";
  qDebug() << "Host:" + db.hostName() + "\n";
  qDebug() << "Connection name: " + db.connectionName() + "\n";
  qDebug() << "DSN:" + db.databaseName() + "\n";

  QList<FKInfo> fkList = getForeignKeyInfo();

  tableList = getTableList(db, config->currConnection->servertype());
  if(!doesColumnExist("Segments", "pointArray"))
  {
   addColumn("Segments", "pointArray", "text");
  }
  if(!doesColumnExist("Segments", "street"))
  {
   addColumn("Segments", "street", "text");
  }

  if(!doesColumnExist("Companies", "routePrefix"))
  {
   addColumn("Companies", "routePrefix", "varchar(10)");
  }

  if(!doesColumnExist("Companies", "info"))
  {
   addColumn("Companies", "info", "varchar(50)");
  }

  if(!doesColumnExist("altRoute", "routePrefix") || !testAltRoute())
  {
   //addColumn("altRoute", "routePrefix", "varchar(10)");
   executeScript(":/recreateAltRoute.sql",db);
  }

  if(!doesColumnExist("Stations", "suffix"))
  {
   //addColumn("altRoute", "routePrefix", "varchar(10)");
   executeScript(":/recreateStationTable.sql",db);
  }
  if(!doesConstraintExist("Stations", "segmentId"))
  {
   // change main unique index to "constraint main unique (`segmentId`,`name`,`startDate`,`endDate`)"
   qDebug("Stations table must be modified!");
   executeScript(":/recreateStationTable.sql",db);
  }
  if(!doesColumnExist("Stations", "markerType"))
  {
   //addColumn("altRoute", "routePrefix", "varchar(10)");
   executeScript(":/recreateStationTable.sql",db);
  }
#if 1
  if(!doesColumnExist("Routes", "OneWay"))
  {
   addColumn("Routes", "OneWay", "char(1) default 'Y'");
   executeScript(":/updateOneWay.sql",db);
  }
  if(!doesColumnExist("Routes", "TrackUsage"))
  {
   if(config->currConnection->servertype() == "Sqlite" )
   {
    addColumn("Routes", "TrackUsage", " text check(`TrackUsage` in ('B', 'L', 'R', ' ')) default ' ' NOT NULL");
    executeScript(":/restructure_routes.sql",db);
   }
   else if(config->currConnection->servertype() == "MySql")
   {
    addColumn("Routes", "TrackUsage", "ENUM('N', 'B', 'R')");
   }
   // TODO: add Sql Server syntax
   executeScript(":/restructure_routes.sql");
  }

  if(!doesColumnExist("RouteComments", "latitude"))
   executeScript(":/recreateRouteComments.sql", db);

#endif
 }
}

bool SQL::executeScript(QString path, QSqlDatabase db)
{
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  qDebug()<<file.errorString() + " '" + file.fileName()+"'";
  //ui->lblHelp->setText(file.errorString() + " '" + file.fileName()+"'");
  return true;
 }

 QTextStream in(&file);
 QString sqltext;
 while(!in.atEnd())
 {
  sqltext = sqltext +  in.readLine();
  if(sqltext.contains(";"))
  {
   QSqlQuery query = QSqlQuery(db);
   if(!query.exec(sqltext))
   {
    SQLERROR(query);
    //ui->lblHelp->setText(err.text());
    RollbackTransaction("");
    return false;
   }
   sqltext="";
  }
 }
 return true;
}

// Query routes to find usage count and min start date, max end date
//bool SQL::getSegmentDates(SegmentInfo* si)
//{
// QSqlQuery query = QSqlQuery();
// QString CommandTxt;

// CommandTxt = QString("select count(*), min(startDate), max(endDate) from Routes where lineKey = %1").arg(si->segmentId);
// if(!query.exec(CommandTxt))
// {
//  SQLERROR(query);
//  return false;
// }
// while(query.next())
// {
//  si->routeCount = query.value(0).toInt();
//  si->startDate = query.value(1).toDate().toString("yyyy/MM/dd");
//  si->endDate = query.value(2).toDate().toString("yyyy/MM/dd");
//  si->bNeedsUpdate= true;
// }
// return true;
//}

bool SQL::testAltRoute()
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "select sql from sqlite_master where tbl_name = 'altRoute' ";
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return false;
 }
 QString sql;
 while(query.next())
 {
  sql = query.value(0).toString();
  if(sql.contains("`route` integer NOT NULL primary key AUTOINCREMENT"))
   return true;
 }
 return false;
}

// get count of routes using segment
int SQL::getCountOfRoutesUsingSegment(int segmentId)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "select count(*) from routes where linekey =" + QString("%1").arg(segmentId) ;
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return false;
 }
 QString sql;
 while(query.next())
 {
  return query.value(0).toInt();
 }
 return 0;
}

// get count of routes using segment
int SQL::getCountOfStationsUsingSegment(int segmentId)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "select count(*) from Stations where segmentId =" + QString("%1").arg(segmentId) ;
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return false;
 }
 QString sql;
 while(query.next())
 {
  return query.value(0).toInt();
 }
 return 0;
}


bool SQL::deleteAndReplaceSegmentWith(int segmentId1, int segmentId2)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 int rows =0;

 BeginTransaction("replaceSegment");
 QString commandText = "update routes set lineKey = " + QString("%1").arg(segmentId2) + "  where linekey =" + QString("%1").arg(segmentId1) ;
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return false;
 }
 rows = query.numRowsAffected();
 if(rows)
 {
  if(!deleteSegment(segmentId1))
  {
   RollbackTransaction("replaceSegment");
   return false;
  }

  int cnt = getCountOfStationsUsingSegment(segmentId1);
  if(cnt)
  {
   commandText = QString("update Stations set segmentId = %1 where segmentId = %2").arg(segmentId2).arg(segmentId1);
   if(!query.exec(commandText))
   {
    SQLERROR(query);
    RollbackTransaction("replaceSegment");
    return false;
   }
   if(rows != cnt)
   {
    qDebug() << "number of stations updated not equal expected" << rows << " vs " << cnt;
    RollbackTransaction("replaceSement");
    return false;
   }
  }
  QList<TerminalInfo> terminals = getTerminalInfoUsingSegment(segmentId1);
  if(terminals.count())
  {
   foreach(TerminalInfo ti, terminals)
   {
    if(ti.startSegment == segmentId1)
     ti.startSegment = segmentId2;
    if(ti.endSegment == segmentId1)
     ti.endSegment = segmentId2;
    if(!updateTerminals(ti))
    {
     RollbackTransaction("replaceSegment");
     return false;
    }
   }
  }
  CommitTransaction("replaceSegment");
  return true;
 }
 else
 {
  if(!deleteSegment(segmentId1))
  {
   RollbackTransaction("replaceSegment");
   return false;
  }
 }
 CommitTransaction("replaceSegment");
 return true;
}

QList<SegmentData> SQL::getUnusedSegments()
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QList<SegmentData> list;
 QString commandText = "select  distinct s.segmentid, s.description, s.tracks, s.type from segments s where s.segmentid not in (select linekey from routes r) ";
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return list;
 }
 while(query.next())
 {
  SegmentData sd;
  sd._segmentId = query.value(0).toInt();
  sd._description = query.value(1).toString();
  sd._tracks = query.value(2).toInt();
  sd._routeType = (RouteType)query.value(3).toInt();

  list.append(sd);
 }
 return list;
}

// given a list of segments replace them with a new segment from alist.
bool SQL::replaceSegmentsInRoutes(QStringList oldSegments, QStringList newSegments, QDate ignoreDate)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);

 QString oldIn = "(";
 BeginTransaction("replaceSegments");
 bool bFirst = true;
 foreach(QString segment, oldSegments)
 {
   bool ok;
   if(!bFirst)
    oldIn.append(", ");
   oldIn.append(QString("%1").arg(segment.toInt(&ok)));
   if(!ok)
    throw IllegalArgumentException(tr("invalid segment: '%1'").arg(segment));
   bFirst = false;
  }
  oldIn.append(")");

  QString commandText;
  commandText = "Select s.startLat, s.startLon, s.endLat, s.endLon, s.segmentId, s.description, "
                "r.route, r.name, r.direction, r.next, r.prev, r.normalEnter,"
                " r.normalLeave, r.reverseEnter, r.reverseLeave, "
                "r.startDate, r.endDate, s.length, s.tracks, r.OneWay, r.TrackUsage, s.type, r.companyKey, r.tractionType "
                "from Routes r join Segments s on s.segmentId = r.lineKey "
                "where r.linekey in " +oldIn;
  if(!query.exec(commandText))
  {
   SQLERROR(query);
   RollbackTransaction("replaceSegments");
   return false;
  }
  QList<RouteData> list;
  while(query.next())
  {
   RouteData rd;
   rd.startLatLng = LatLng(query.value(0).toDouble(), query.value(1).toDouble());
   rd.endLatLng = LatLng(query.value(2).toDouble(), query.value(3).toDouble());
   rd.lineKey = rd.sd->_segmentId = query.value(4).toInt();
   rd.sd->_description = query.value(5).toString();
   rd.route = query.value(6).toInt();
   rd.name = query.value(7).toString();
   rd.direction = query.value(8).toString();
   rd.next = query.value(9).toInt();
   rd.prev = query.value(10).toInt();
   rd.normalEnter = query.value(11).toInt();
   rd.normalLeave = query.value(12).toInt();
   rd.reverseEnter = query.value(13).toInt();
   rd.reverseLeave = query.value(14).toInt();
   rd.startDate = query.value(15).toDate();
   rd.endDate = query.value(16).toDate();
   rd.sd->_length = query.value(17).toInt();
   rd.sd->_tracks = query.value(18).toInt();
   rd.oneWay = query.value(19).toString();
   rd.trackUsage = query.value(20).toString();
   rd.sd->_routeType = (RouteType)query.value(21).toInt();
   rd.companyKey = query.value(22).toInt();
   rd.tractionType =query.value(23).toInt();
   rd.bearing = new Bearing(rd.startLatLng, rd.endLatLng);
   list.append(rd);
  }


  foreach(RouteData rd, list)
  {
   if(rd.startDate > ignoreDate)
   {
    QString err =tr("ignoring route %1, name %2, segment %3, %4, startDate = %5").arg(rd.route).arg(rd.name).arg(rd.lineKey).arg(rd.sd->_description).arg(rd.startDate.toString("yyyy/MM/dd"));
    emit details(err);
    continue;
   }
   if(!SQL::deleteRouteSegment(rd.route,rd.name, rd.lineKey,rd.startDate.toString("yyyy/MM/dd"),rd.endDate.toString("yyyy/MM/dd")))
   {
    RollbackTransaction("replaceSegments");
    throw RecordNotFoundException(tr("Unable to delete %1 %2 %3 %4 segment: %5").arg(rd.route).arg(rd.name).arg(rd.startDate.toString("yyyy/MM/dd")).arg(rd.endDate.toString("yyyy/MM/dd")).arg(rd.lineKey));
   }
   foreach(QString segment, newSegments)
   {
    bool ok;
    RouteData newRd = RouteData(rd);
    newRd.sd = new SegmentData(SQL::getSegmentData(segment.toInt(&ok)));
    newRd.lineKey = segment.toInt(&ok);
    double diffBearing = rd.bearing->getBearing() - newRd.sd->_bearing.getBearing();
    if(newRd.sd->_tracks == 2)
    {
     newRd.oneWay = "Y";
     if(diffBearing > 45.0)
      newRd.trackUsage = "L";
     else
      newRd.trackUsage = "R";
    }
    if(SQL::doesRouteSegmentExist(newRd.route, newRd.name, newRd.lineKey,
                                  newRd.startDate.toString("yyyy/MM/dd"),
                                  newRd.endDate.toString("yyyy/MM/dd")))
     continue;


    if(!insertRouteSegment(newRd))
    {
     RollbackTransaction("replaceSegments");
     throw Exception("failed:" + commandText);
    }
    QString detail;
    detail = detail.append("route ").append(QString("%1").arg(rd.route)).append(" ").append(rd.name).append(", Segment ")
      .append(QString("%1").arg(rd.lineKey)).append(" ").append(rd.sd->_description).append(" replaced with ")
      .append(QString("%1").arg(newRd.lineKey)).append(" ").append(newRd.sd->_description);
    qDebug() << detail;
    emit details(detail);

   }
 }
 CommitTransaction("replaceSegments");
 return true;
}

QList<FKInfo> SQL::getForeignKeyInfo()
{
 QList<FKInfo> list;
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);

 QString commandText =  "SELECT m.name, p.* FROM sqlite_master m JOIN pragma_foreign_key_list(m.name) p ON m.name != p.'table' "
                        "WHERE m.type = 'table' ORDER BY m.name";
 if(!query.exec(commandText))
 {
  SQLERROR(query);
  return list;
 }
 while(query.next())
 {
  FKInfo info;
  info.name = query.value(0).toString();
  info.id = query.value(1).toInt();
  info.seq = query.value(2).toInt();
  info.table = query.value(3).toString();
  info.from = query.value(4).toString();
  info.to = query.value(5).toString();
  info.on_update = query.value(6).toString();
  info.on_delete = query.value(7).toString();
  info.match = query.value(8).toString();
  list.append(info);
 }
 return list;
}
