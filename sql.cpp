#include "sql.h"
#include "data.h"
#include "sqlite3.h"
#include <QApplication>
#include <algorithm>
#include <QDebug>
#include <QTextEdit>
#include <routeselector.h>
#include "mainwindow.h"
#include "exportsql.h"
#include <QRadioButton>
#include <QtSql>
#include <QUrl>
#include "streetstablemodel.h"

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
    if(config->currConnection->driver()=="QSQLITE"
       || config->currConnection->driver()=="QSQLITE3")
    {
     QFileInfo info(db.hostName());
     msg.append(info.absoluteFilePath());
    }
    QMessageBox::warning(NULL, "Warning", msg);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Resources/databases/StLouis.sqlite3");

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
      if(config->currConnection->database() != "default" && config->currConnection->database() != "")
      {
       QSqlQuery query = QSqlQuery(db);
       if(!query.exec(tr("use [%1]").arg(config->currConnection->database())))
       {
           QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
           qDebug() << errCommand;
           QSqlError error = query.lastError();
           SQLERROR(std::move(query));
           throw SQLException(error.text() + " " + errCommand);
       }
      }
     }
    }
    if(config->currConnection->servertype() == "Sqlite")
     SQL::instance()->setForeignKeyCheck(config->foreignKeyCheck());
    return ok;
}

bool SQL::isTransactionActive()
{
 QSqlDatabase db = QSqlDatabase::database();
 QVariant v = db.driver()->handle();
 sqlite3 *handle = NULL;
 if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0)
 {
  // v.data() returns a pointer to the handle
  handle = *static_cast<sqlite3 **>(v.data());

  const char * schema = "";
  int txn = sqlite3_txn_state(handle, nullptr);
  if(txn == SQLITE_TXN_WRITE)
   return true;
 }
 return false;
}



int SQL::sqlErrorMessage(QSqlQuery query, QMessageBox::StandardButtons buttons)
{
 MyMessageBox* mb = new MyMessageBox(nullptr, tr("Sqlerror"), tr("An sql error has occurred: %1 "
                        "<br> <B>Query:</B %2").arg(query.lastError().text()).arg(query.lastQuery()),
                                     QMessageBox::Critical, buttons);
 int ret = mb->exec();
 return ret;
}


/// <summary>
/// Begin a transaction
/// </summary>
/// <param name="name"></param>
void SQL::beginTransaction (QString name)
{
 if(!currentTransaction.isEmpty())
 {
  qDebug() << "Warning! Begin transaction ignored for '" + name + "'; transaction '" + currentTransaction + "' is already active";
  return;
 }
 QString commandText = "Begin Transaction " +name;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 //bool bQuery = db.transaction();
 if(!bQuery)
 {
     QString errCommand = commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = db.lastError();
     SQLERROR(std::move(query));
     //throw SQLException(error.text() + " " + errCommand);
     return;
 }
 currentTransaction = name;
}

/// <summary>
/// Commits a series of sql commands
/// </summary>
/// <param name="name"></param>
void SQL::commitTransaction (QString name)
{
  if(name != currentTransaction)
  {
   qDebug() << "Warning: " << QString("Commit transaction for '%1' ignored because another transaction: '%2' is active").arg(name).arg(currentTransaction);
      return;
  }
  QString commandText = "Commit Transaction " +name;
  QSqlDatabase db = QSqlDatabase::database();
  //QSqlQuery query = QSqlQuery(db);
  //bool bQuery = query.exec(commandText);
  bool bQuery = db.commit();
  if(!bQuery)
  {
      QString errCommand = commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = db.lastError();
      //SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  qDebug() << commandText << " comitted";
  currentTransaction = "";
}

/// <summary>
/// Aborts and rolls back a series of SQL commands
/// </summary>
/// <param name="name"></param>
void SQL::rollbackTransaction (QString name)
{
 QString commandText = "Rollback Transaction " +name;
 QSqlDatabase db = QSqlDatabase::database();
 //QSqlQuery query = QSqlQuery(db);
 //bool bQuery = query.exec(commandText);
 bool bQuery = db.rollback();
 if(!bQuery)
 {
  QSqlError err = db.lastError();
  qWarning() <<tr("Rollback transaction %1 failed %2").arg(currentTransaction).arg(err.driverText());
//  db.close();
//  exit(EXIT_FAILURE);
  return;
 }
 qWarning() <<tr("Rollback transaction %1 ").arg(currentTransaction);

 currentTransaction = "";
}

void SQL::myExceptionHandler(Exception e)

{
    Q_UNUSED(e)
    qDebug() << "SQL exception " << e.msg;
    QSqlDatabase db = QSqlDatabase::database();
    //db.close();
    exit(EXIT_FAILURE);
}

int routeSortType;
bool sortbyalpha_route_date( const RouteData & s1 , const RouteData & s2 )
{
//cout<<"\n" << __FUNCTION__;
    QString s1Route, s2Route, s1Sfx=".0", s2Sfx = ".0";

    s1Route = s1.alphaRoute();
    if(s1.alphaRoute().contains('.'))
    {
        s1Route = s1.alphaRoute().mid(0,s1.alphaRoute().indexOf('.'));
        s1Sfx = s1.alphaRoute().mid(s1.alphaRoute().indexOf('.'));
    }
    s2Route = s2.alphaRoute();
    if(s2.alphaRoute().contains('.'))
    {
        s2Route = s2.alphaRoute().mid(0,s2.alphaRoute().indexOf('.'));
        s2Sfx = s2.alphaRoute().mid(s2.alphaRoute().indexOf('.'));
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
        return ( s1Route + s1.endDate().toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.endDate().toString("yyyy/MM/dd")+ s2Sfx);
    case 1:
        return ( s1Route + s1.startDate().toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.startDate().toString("yyyy/MM/dd")+ s2Sfx);
    case 2:
        return ( s1Route + s1.routeName() + s1.endDate().toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.routeName() + s2.endDate().toString("yyyy/MM/dd")+ s2Sfx);
    case 3:
        return ( s1Route + s1.routeName() + s1.startDate().toString("yyyy/MM/dd")+ s1Sfx < s2Route + s2.routeName() + s2.startDate().toString("yyyy/MM/dd")+ s2Sfx);
    case 4:
        return (s1.routeName() + s1Route  + s1.startDate().toString("yyyy/MM/dd")+ s1Sfx <   s2.routeName() + s2Route + s2.startDate().toString("yyyy/MM/dd")+ s2Sfx);


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
 QList<RouteInfo*> riList;
 RouteData rd;
 QSqlQuery query;
 QString commandText;
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString where;
 if(companyKey >0)
  where= " where r.companyKey = " + QString("%1").arg(companyKey);
 if(config->currConnection->servertype() == "MySql")
    commandText = "Select distinct a.baseRoute, r.route, n.name, r.startDate, "
               "r.endDate, r.companyKey, tractionType, a.routeAlpha, c.mnemonic,r.routeId  "
               "from Routes r "
               "join AltRoute a on r.route =  a.route "
               "join Companies c on r.companyKey = c.`key` "
               "join RouteName n on r.routeId = n.routeId "
               + where +
               " group by a.baseRoute, r.route, n.name, r.startDate, r.endDate, "
               " r.companykey,tractionType, a.routeAlpha "
               " order by a.routeAlpha, n.name, r.endDate ";
 else if(config->currConnection->servertype() == "MsSql")
     commandText = "Select distinct a.baseRoute, r.route, n.name, r.startDate, "
                   "r.endDate, r.companyKey, tractionType, a.routeAlpha, c.mnemonic, r.routeId  "
                   "from Routes r "
                   "join AltRoute a on r.route =  a.route "
                   "join Companies c on r.companyKey = c.[key] "
                   "join RouteName n on r.routeId = n.routeId "
                   + where +
                   " group by a.baseRoute, r.route, n.name, r.startDate, r.endDate, "
                   " r.companykey,tractionType, a.routeAlpha "
                   " order by a.routeAlpha, n.name, r.endDate ";
 else
    commandText = "Select distinct a.baseRoute, r.route, n.name, r.startDate, "
            "r.endDate, r.companyKey, tractionType, a.routeAlpha, c.mnemonic, r.routeId  "
            "from Routes r "
            "join AltRoute a on r.route =  a.route "
            "join Companies c on r.companyKey = c.key "
            "join RouteName n on r.routeId = n.routeId "
            + where +
            " group by a.baseRoute, r.route, n.name, r.startDate, r.endDate, "
            " r.companykey,c.mnemonic,tractionType, a.routeAlpha, r.routeid "
            " order by a.routeAlpha, n.name, r.endDate ";
 query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 int ix =-1;
 while (query.next())
 {
  ix++;
  RouteInfo* ri = new RouteInfo();
  ri->baseRoute = query.value(0).toInt();
  ri->route = query.value(1).toInt();
  ri->routeName = query.value(2).toString().trimmed();
  ri->startDate = query.value(3).toDate();
  if(ix > 0)
  {
   RouteInfo* riPrev = riList.at(ix - 1);
   if(ri->route == riPrev->route && ri->routeName == riPrev->routeName && ri->startDate < riPrev->endDate)
    riList.at(ix - 1)->endDate = ri->startDate.addDays(-1);
  }
  ri->endDate = query.value(4).toDate();
  ri->companyKey = query.value(5).toInt();
  ri->tractionType = query.value(6).toInt();
  ri->alphaRoute = query.value(7).toString();
  ri->companyMnemonic = query.value(8).toString();
  ri->_routeId = query.value(9).toInt();
  riList.append(ri);
 }
 for(RouteInfo* ri : riList)
 {
  if(ri->endDate < ri->startDate)
   continue;
  RouteData rd = RouteData();
  rd.setRoute(ri->route);
  rd.setRouteName(ri->routeName);
  rd.setStartDate(ri->startDate);
  rd.setEndDate(ri->endDate);
  rd.setAlphaRoute(ri->alphaRoute);
  rd.setCompanyKey(ri->companyKey);
  rd.setTractionType(ri->tractionType);
  rd.setRoutePrefix(ri->routePrefix);
  rd.setCompanyMnemonic(ri->companyMnemonic);
  rd.setRouteId(ri->_routeId);
  list.append(rd);
 }
 return list;;
}
QList<RouteData> SQL::getRoutesByEndDate(QList<int> compayList)
{
 QList<RouteData> list;
 QList<RouteInfo*> riList;
 RouteData rd;
 QSqlQuery query;
 QString commandText;
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString where;
 //if(companyKey >0)
  where= " where r.companyKey in( " + list2String(compayList) +")";
 commandText = "Select distinct a.baseRoute, r.route, n.name, r.startDate, r.endDate, r.companyKey, "
               "tractionType, a.routeAlpha, c.routePrefix, r.routeId  "
               "from Routes r "
               "join AltRoute a on r.route =  a.route "
               "join Companies c on r.companyKey = c.[key] "
               "join RouteName n on r.routeId = n.routeId "
               + where +
               " group by a.baseRoute, r.route, n.name, r.startDate, r.endDate, r.companykey,"
               " tractionType, a.routeAlpha, c.routePrefix, r.routeId "
               " order by a.routeAlpha, n.name, r.endDate ";
 query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 int ix =-1;
 while (query.next())
 {
  ix++;
  RouteInfo* ri = new RouteInfo();
  ri->baseRoute = query.value(0).toInt();
  ri->route = query.value(1).toInt();
  ri->routeName = query.value(2).toString().trimmed();
  ri->startDate = query.value(3).toDate();
  if(ix > 0)
  {
   RouteInfo* riPrev = riList.at(ix - 1);
   if(ri->route == riPrev->route && ri->routeName == riPrev->routeName && ri->startDate < riPrev->endDate)
    riList.at(ix - 1)->endDate = ri->startDate.addDays(-1);
  }
  ri->endDate = query.value(4).toDate();
  ri->companyKey = query.value(5).toInt();
  ri->tractionType = query.value(6).toInt();
  ri->alphaRoute = query.value(7).toString();
  ri->routePrefix = query.value(8).toString();
  ri->_routeId = query.value(9).toInt();
  riList.append(ri);
 }
 for(RouteInfo* ri : riList)
 {
  if(ri->endDate < ri->startDate)
   continue;
  RouteData rd = RouteData();
  rd.setRoute(ri->route);
  rd.setRouteName(ri->routeName);
  rd.setStartDate(ri->startDate);
  rd.setEndDate(ri->endDate);
  rd.setAlphaRoute(ri->alphaRoute);
  rd.setCompanyKey(ri->companyKey);
  rd.setTractionType(ri->tractionType);
  rd.setBaseRoute(ri->baseRoute);
  rd._routePrefix = ri->routePrefix;
  rd.setRouteId(ri->_routeId);
  list.append(rd);
 }
 return list;;
}

RouteSeq SQL::getRouteSeq(RouteData rd)
{
 RouteSeq rs;
 QList<QPair<int, QString>> list;
 if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query;
 QString commandText = QString("select segmentList, firstSegment, whichEnd"
                       " from RouteSeq"
                       " where route = %1 and name = '%2' and "
                       " startDate = '%3' and endDate = '%4'")
                       .arg(rd.route()).arg(rd.routeName(),
                              rd.startDate().toString("yyyy/MM/dd"),
                              rd.endDate().toString("yyyy/MM/dd"));
 query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  rs._seqList = rd.setSeqList(query.value(0).toString());
  rs._firstSegment = query.value(1).toInt();
  rs._whichEnd = query.value(2).toString();
  rs._rd = rd;
 }
 return rs;
}

bool SQL::deleteRouteSeq(RouteSeq rs)
{
 if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = QString("delete RouteSeq "
                       " where route = %1 and name = '%2' and "
                       " startDate = '%3' and endDate = '%4'")
         .arg(rs._rd.route()).arg(rs._rd.routeName(), rs._rd.startDate().toString("yyyy/MM/dd"),
                              rs._rd.endDate().toString("yyyy/MM/dd"));
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
}

TerminalInfo SQL::getTerminalInfo(qint32 route, QString name, QDate endDate)
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
      "where route = " + strRoute + " and a.endDate = '" + endDate.toString("yyyy/MM/dd") + "' and name = '" + name + "'";
  else
      commandText = "select a.startDate, a.endDate, startSegment, startWhichEnd, endSegment, endWhichEnd, "
          "b.startLat, b.startLon, b.endLat, b.endLon, c.startLat, c.startLon, c.endLat, c.endLon "
          "from Terminals a " \
          "full outer join Segments b on b.segmentId = startSegment " \
          "full outer join Segments c on c.segmentId = endSegment " \
          "where route = " + strRoute + " and a.endDate = '" + endDate.toString("yyyy/MM/dd") + "' and name = '" + name + "'";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  qDebug() << commandText;
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
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
      ti.startDate = query.value(0).toDate();
      ti.endDate = query.value(1).toDate();
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
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 if (!query.isActive())
 {
  TerminalInfo ti;
  ti.route = query.value(0).toInt();
  ti.name = query.value(1).toString();
  ti.startDate = query.value(2).toDate();
  ti.endDate = query.value(3).toDate();
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

QString SQL::getAlphaRoute(qint32 route, QString routePrefix)
{
 QString routeAlpha = "";
 QSqlDatabase db = QSqlDatabase::database();
 QString strRoute = QString("%1").arg(route);
 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
  commandText = "select routeAlpha from AltRoute a " \
   "where route = " + strRoute + " and routePrefix ='" +routePrefix + "'";
 else
   commandText = "select routeAlpha from AltRoute a " \
                 "where route = " + strRoute + " and routePrefix ='" +routePrefix + "'";

 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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

        QString commandText = "select routeAlpha from AltRoute " \
                "where routeAlpha like '" + text+ "%'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

bool SQL::deleteAlphaRoute(QString routeAlpha)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "delete from AltRoute where routeAlpha = '" + routeAlpha + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return  true;
}

QMap<int,TractionTypeInfo> SQL::getTractionTypes()
{
    QMap<int,TractionTypeInfo> myArray;
    TractionTypeInfo tti;
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            tti = TractionTypeInfo();
            tti.tractionType = query.value(0).toInt();
            tti.description = query.value(1).toString();
            tti.displayColor = query.value(2).toString();
            tti.routeType = (RouteType)query.value(3).toInt();
            tti.icon = query.value(4).toString();
            myArray.insert(tti.tractionType, tti);
        }
    }
    catch (Exception& e)
    {
        myExceptionHandler(e);

    }
    return myArray;
}
#if 0
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
#endif

QList<SegmentInfo> SQL::getSegmentsForStreet(QString street, QString location)
{
 QList< SegmentInfo> myArray;

 QSqlDatabase db = QSqlDatabase::database();


 QString commandText;
 if(location != " " && !location.isEmpty())
 {
  commandText= "Select SegmentId, description, OneWay, s.startDate, s.endDate,"
                       " s.length, points, s.startLat, s.startLon, s.endLat, s.EndLon, type, s.street,"
                       " s.location, pointArray, tracks, direction, DoubleDate, newerName, s.streetid "
                       " from Segments s"
                       //" join `StreetDef`  on streetdef.StreetId = s.streetId  "
                       " where s.Street = '" + street + "' and s.location = '" + location + "'"
                       //" and  s.streetId > 0 " +
                       "order by description";
 }
 else {
   commandText= "Select SegmentId, description, OneWay, s.startDate, s.endDate,"
                        " s.length, points, s.startLat, s.startLon, s.endLat, s.EndLon, type, s.street,"
                        " s.location, pointArray, tracks, direction, DoubleDate, newerName, s.streetid"
                " from Segments s"
                //" join `StreetDef`  on streetdef.StreetId = s.streetId  "
                " where s.Street = '" + street + "' "
                //" and  s.streetId > 0 " +
                "order by description";

  }
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
  SegmentInfo si;
  si._segmentId = query.value(0).toInt();
  si._description = query.value(1).toString();
  //si._oneWay = query.value(2).toString();
  si._dateBegin = query.value(3).toDate();
  si._dateEnd = query.value(4).toDate();
  si._length = query.value(5).toDouble();
  si._points = query.value(6).toInt();
  si._startLat = query.value(7).toDouble();
  si._startLon = query.value(8).toDouble();
  si._endLat = query.value(9).toDouble();
  si._endLon = query.value(10).toDouble();
  si._bearing = Bearing(si._startLat, si._startLon, si._endLat, si._endLon);
  si._direction = si._bearing.strDirection();
  si._routeType = (RouteType)query.value(11).toInt();
  si._streetName = query.value(12).toString();
  si._location = query.value(13).toString();
  QString pointArray = query.value(14).toString();
  si._tracks = query.value(15).toInt();
  si.direction() = query.value(16).toString();
  si._dateDoubled = query.value(17).toDate();
  si._newerStreetName = query.value(18).toString();
  si._streetId = query.value(19).toInt();

  si.setPoints(pointArray);  // initialize array of points (i.e pointList)
  if(si.pointList().count() > 1)
  {
   si._bearingStart = Bearing(si._startLat, si._startLon, si.pointList().at(1).lat(), si.pointList().at(1).lon());
   si._bearingEnd = Bearing(si.pointList().at(si._points-2).lat(), si.pointList().at(si._points-2).lon(), si._endLat, si._endLon);
  }

  si._bounds = Bounds(pointArray);
  if(si.description().isEmpty())
  {
   si._description = si._bearing.strDirection();
   si._bNeedsUpdate = true;
  }
  if(si._streetName.isEmpty() || si._streetName.indexOf("(")> 0)
  {
   QStringList tokens = si._description.split(",");
   if(tokens.count() > 1)
   {
    QString street = tokens.at(0).trimmed();
    street= street.mid(0, street.indexOf("("));
    si._streetName = street;
    si._bNeedsUpdate = true;
   }
   else
   {
    tokens = si._description.split(" ");
    if(tokens.count() > 1)
    {
     QString street = tokens.at(0).trimmed();
     street= street.mid(0, street.indexOf("("));
     si._streetName = street;
     si._bNeedsUpdate = true;
    }
   }
  }
  myArray.append(si);
 }

 return myArray;
}

QMap<int, SegmentInfo> SQL::getSegmentInfoList(QString location)
{
 QMap<int, SegmentInfo> myArray;

 QSqlDatabase db = QSqlDatabase::database();


 QString commandText;
 if(location != " " && !location.isEmpty())
 {
  commandText= "Select SegmentId, description, OneWay, startDate, endDate,"
                       " length, points, startLat, startLon, endLat, EndLon, type, street,"
                       " location, pointArray, tracks, direction, DoubleDate, newerName,"
                       " streetid, rowid "
                       " from Segments where location = '" + location + "' "
                       + "order by description";
 }
 else {
   commandText= "Select SegmentId, description, OneWay, startDate, endDate,"
                        " length, points, startLat, startLon, endLat, EndLon, type, street,"
                        " location, pointArray, tracks, direction, DoubleDate, newerName,"
                        " streetid, rowid"
                        " from Segments order by description";
  }
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
  SegmentInfo si;
  si._segmentId = query.value(0).toInt();
  si._description = query.value(1).toString();
  //si._oneWay = query.value(2).toString();
  si._dateBegin = query.value(3).toDate();
  si._dateEnd = query.value(4).toDate();
  si._length = query.value(5).toDouble();
  si._points = query.value(6).toInt();
  si._startLat = query.value(7).toDouble();
  si._startLon = query.value(8).toDouble();
  si._endLat = query.value(9).toDouble();
  si._endLon = query.value(10).toDouble();
  si._bearing = Bearing(si._startLat, si._startLon, si._endLat, si._endLon);
  si._direction = si._bearing.strDirection();
  si._routeType = (RouteType)query.value(11).toInt();
  si._streetName = query.value(12).toString();
  si._location = query.value(13).toString();
  QString pointArray = query.value(14).toString();
  si._tracks = query.value(15).toInt();
  si.direction() = query.value(16).toString();
  si._dateDoubled = query.value(17).toDate();
  si._newerStreetName = query.value(18).toString();
  si._streetId = query.value(19).toInt();
  si._rowid = query.value(20).toInt();

  si.setPoints(pointArray);  // initialize array of points (i.e pointList)
  if(si.pointList().count() > 1)
  {
   si._bearingStart = Bearing(si._startLat, si._startLon, si.pointList().at(1).lat(), si.pointList().at(1).lon());
   si._bearingEnd = Bearing(si.pointList().at(si._points-2).lat(), si.pointList().at(si._points-2).lon(), si._endLat, si._endLon);
  }

  si._bounds = Bounds(pointArray);
  if(si.description().isEmpty())
  {
   si._description = si._bearing.strDirection();
   si._bNeedsUpdate = true;
  }
  if(si._streetName.isEmpty() || si._streetName.indexOf("(")> 0)
  {
   QStringList tokens = si._description.split(",");
   if(tokens.count() > 1)
   {
    QString street = tokens.at(0).trimmed();
    street= street.mid(0, street.indexOf("("));
    si._streetName = street;
    si._bNeedsUpdate = true;
   }
   else
   {
    tokens = si._description.split(" ");
    if(tokens.count() > 1)
    {
     QString street = tokens.at(0).trimmed();
     street= street.mid(0, street.indexOf("("));
     si._streetName = street;
     si._bNeedsUpdate = true;
    }
   }
  }
  myArray.insert(si.segmentId(),si);
 }

 return myArray;
}

QStringList SQL::getLocations()
{
 QSqlDatabase db = QSqlDatabase::database();
 QStringList list;

 QString commandText = "select location from Segments group by location";
 QSqlQuery query(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
  list.append(query.value(0).toString());
 }
 return  list;
}
#if 0
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
#endif
// update segment begin/end dates based on route usage
bool SQL::updateSegmentDates(SegmentInfo* si)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 commandText = "select min(startDate), max(EndDate)  from Routes "
               "where linekey = " + QString("%1").arg(si->segmentId());
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 if (!query.isActive())
 {
  return  false;
 }
 while (query.next())
 {
  si->_dateBegin = query.value(0).toDate();
  si->_dateEnd = query.value(1).toDate();
 }
 if(!si->_dateBegin.isValid() || !si->_dateEnd.isValid())
     return false;
 commandText = "update Segments set startDate = '"
   + si->startDate().toString("yyyy/MM/dd")
   + "', enddate = '" + si->endDate().toString("yyyy/MM/dd")
   + "', doubleDate = '" + si->doubleDate().toString("yyyy/MM/dd")
   + "' where segmentId = " + QString("%1").arg(si->segmentId());
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
}
SegmentInfo SQL::getSegmentInfo(qint32 segmentId)
{
 SegmentInfo si;
 if(segmentId <= 0)
     return si;
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString commandText = "Select SegmentId, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, location, pointArray, DoubleDate, FormatOK, NewerName,StreetId, rowid"
                     " from Segments"
                     " where SegmentId = " + QString("%1").arg(segmentId);
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  if (!query.isActive())
  {

      return si;
  }
  while (query.next())
  {
   si._segmentId = query.value(0).toInt();
   si._description = query.value(1).toString();
   si._tracks = query.value(2). toInt();
   si._routeType = (RouteType)query.value(3).toInt();
   si._startLat = query.value(4).toDouble();
   si._startLon = query.value(5).toDouble();
   si._endLat = query.value(6).toDouble();
   si._endLon = query.value(7).toDouble();
   si._length = query.value(8).toDouble();
   si._dateBegin = query.value(9).toDate();
   si._dateEnd = query.value(10).toDate();
   si._direction = query.value(11).toString();
   si._streetName = query.value(12).toString();
   si._location = query.value(13).toString();
   si.setPoints(query.value(14).toString());  // array of points
   si._dateDoubled = query.value(15).toDate();
   si._formatOK = query.value(16).toBool();
   si._newerStreetName = query.value(17).toString();
   si._streetId = query.value(18).toInt();
   si._rowid = query.value(19).toInt();
  }

  si._points = si._pointList.count();
  if((si._startLat ==0 ||si._startLat ==0 || si._endLat == 0 || si._endLon ==0) && si._pointList.count() > 1 )
  {
   si._startLat = si._pointList.at(0).lat();
   si._startLon = si._pointList.at(0).lon();
   si._endLat = si._pointList.at(si._points-1).lat();
   si._endLon = si._pointList.at(si._points-1).lon();
  }
  si._bearing = Bearing(si._startLat, si._startLon, si._endLat, si._endLon);
  if(si._pointList.count() > 1)
  {
   si._bearingStart = Bearing(si._startLat, si._startLon, si._pointList.at(1).lat(), si._pointList.at(1).lon());
   si._bearingEnd = Bearing(si._pointList.at(si._points-2).lat(), si._pointList.at(si._points-2).lon(), si._endLat, si._endLon);
  }
  if(si._pointList.count()> 1)
   si._bounds = Bounds::fromPointList(si._pointList);
  if(si._length == 0 && si._pointList.count() > 1)
  {
   //sd._length = distance(LatLng(sd._startLat, sd._startLon), LatLng(sd._endLat, sd._endLon));
   for(int i=0; i <  si._points-2; i++)
    si._length += distance(si._pointList.at(i), si._pointList.at(i+1));
  }

  return  si;
 }
 catch (Exception& e)
 {
    myExceptionHandler(e);
 }

 return si;
}

SegmentInfo SQL::getSegmentIdForDescription(QString description)
{
 SegmentInfo si;
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString commandText;

  if(config->currConnection->servertype() != "MsSql")
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, location, pointArray from Segments"
                     " where description = " + QString("'%1'").arg(description);
  else
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, location, pointArray from Segments"
                     " where description = " + QString("'%1'").arg(description);
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  if (!query.isActive())
  {

      return si;
  }
  while (query.next())
  {
   si._segmentId = query.value(0).toInt();
   si._description = query.value(1).toString();
   si._tracks = query.value(2). toInt();
   si._routeType = (RouteType)query.value(3).toInt();
   si._startLat = query.value(4).toDouble();
   si._startLon = query.value(5).toDouble();
   si._endLat = query.value(6).toDouble();
   si._endLon = query.value(7).toDouble();
   si._length = query.value(8).toDouble();
   si._dateBegin = query.value(9).toDate();
   si._dateEnd = query.value(10).toDate();
   si._direction = query.value(11).toString();
   si._streetName = query.value(12).toString();
   si._location = query.value(13).toString();
   si.setPoints(query.value(14).toString());  // array of points
  }

  si._points = si._pointList.count();
  if((si._startLat ==0 ||si._startLat ==0 || si._endLat == 0 || si._endLon ==0) && si._pointList.count() > 1 )
  {
   si._startLat = si._pointList.at(0).lat();
   si._startLon = si._pointList.at(0).lon();
   si._endLat = si._pointList.at(si._points-1).lat();
   si._endLon = si._pointList.at(si._points-1).lon();
  }
  si._bearing = Bearing(si._startLat, si._startLon, si._endLat, si._endLon);
  if(si._pointList.count() > 1)
  {
   si._bearingStart = Bearing(si._startLat, si._startLon, si._pointList.at(1).lat(), si._pointList.at(1).lon());
   si._bearingEnd = Bearing(si._pointList.at(si._points-2).lat(), si._pointList.at(si._points-2).lon(), si._endLat, si._endLon);
  }
  if(si._pointList.count()> 1)
   si._bounds = Bounds::fromPointList(si._pointList);
  if(si._length == 0 && si._pointList.count() > 1)
  {
   //sd._length = distance(LatLng(sd._startLat, sd._startLon), LatLng(sd._endLat, sd._endLon));
   for(int i=0; i <  si._points-2; i++)
    si._length += distance(si._pointList.at(i), si._pointList.at(i+1));
  }
  return  si;
 }
 catch (Exception& e)
 {
    myExceptionHandler(e);
 }

 return si;

}

bool SQL::checkConnectingSegments(QList<SegmentData> segmentDataList)
{
    int segmentsNotConnected = 0;
    int segmentsMultiplyConnected=0;
    for(int i=0; i < segmentDataList.count(); i++)
    {
        SegmentData *sd = (SegmentData*)&segmentDataList.at(i);
        // check connections to start of segment
        int startConnects = 0;
        int startSegment=-1;
        int endConnects =0;
        int endSegment=-1;

        foreach(SegmentData sd2, segmentDataList)
        {
            if(sd->segmentId() == sd2.segmentId())  // ignore self
                continue;
            if(sd->oneWay() == "N")
            {
                // Only check connections to start if a twoway segment
                if(sd->_oneWay == "N" && sd2._oneWay =="N") // only twoway can connect start to start!
                {
                    double dist = Distance(sd->_startLat, sd->_startLon, sd2._startLat, sd2._startLon);

                    if(dist < .020)
                    {
                        if(startConnects == 0)
                            startSegment = sd2._segmentId;
                        startConnects++;
                    }
                }
                if((sd->_oneWay == "N" && sd2._oneWay =="N") || sd2._oneWay == "Y")
                {
                    double dist = Distance(sd->_startLat, sd->_startLon, sd2._endLat, sd2._endLon);

                    if(dist < .020 )
                    {
                        if(startConnects == 0)
                            startSegment = sd2._segmentId;
                        startConnects++;
                    }
                }
            }
            // Now check connections to end
            if((sd->_oneWay == "N" && sd2._oneWay =="N") || sd2._oneWay == "Y")
            {
                double dist = Distance(sd->_endLat, sd->_endLon, sd2._startLat, sd2._startLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                        endSegment = sd2._segmentId;
                    endConnects++;
                }
            }
            if(sd->_oneWay == "N" && sd2._oneWay =="N") // only twoway can connect end to end
            {
                double dist = Distance(sd->_endLat, sd->_endLon, sd2._endLat, sd2._endLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                        endSegment = sd2._segmentId;
                    endConnects++;
                }
            }
        }
        if(startConnects == 1)
            sd->_prev = startSegment;
        if(endConnects == 1)
            sd->_next = endSegment;
        if(startConnects == 0 && endConnects == 0)
        {
            segmentsNotConnected++;
            qDebug()<< "segment "+ QString("%1").arg(sd->_segmentId)+ " not connected to another!";
        }
        if(startConnects > 1 || endConnects > 1)
        {
            segmentsMultiplyConnected ++;
            qDebug()<< "segment "+ QString("%1").arg(sd->_segmentId)+ " connects to multiple segments!";
        }
    }
    if(segmentsNotConnected > 1 || segmentsMultiplyConnected > 0)
        return false;
    return true;
}
QList<SegmentData*> SQL::getRouteSegmentsInOrder(qint32 route, QString name, int companyKey, QDate date)
{
 QString where = "where Route = " + QString("%1").arg(route)
                 + " and trim(RouteName) = '" + name + "'"
                 " and '" + date.toString("yyyy/MM/dd") + "' between StartDate and EndDate"
                 " and companyKey = " + QString("%1").arg(companyKey) +
                 " order by StartDate, EndDate, Segmentid";

 return segmentDataListFromView(where);
}


QList<SegmentData*> SQL::getRouteSegmentsForDate(QDate date, int companyKey)
{
 QString company;
 if(companyKey > 0)
     company = " and companyKey = " + QString("%1").arg(companyKey);

 QString where = "where '"+ date.toString("yyyy/MM/dd") + "' between StartDate and EndDate"
                  + company +
                 " order by RouteName, Segmentid";

 return segmentDataListFromView(where);

}
#if 0
// List routes using a segment on a given date
QList<RouteData> SQL::getRouteDatasForDate(qint32 route, QString name, int companyKey, QString date)
{
 QList<RouteData> myArray;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT r.Route,n.Name,r.StartDate,r.EndDate,LineKey,r.companyKey,"
                       " tractionType,s.direction, normalEnter, normalLeave,"
                       " reverseEnter, reverseLeave, routeAlpha, r.oneWay, r.trackUsage,"
                       " s.tracks, c.baseRoute, r.routeId"
                       " from Routes r"
                       " join AltRoute c on r.route = c.route"
                       " join Segments s on r.lineKey = s.segmentId"
                       " join RouteName n on r.routeId = n.routeId"
                       " where r.Route = " + QString("%1").arg(route) + ""
                       " and '" + date + "' between r.startDate and r.endDate"
                       " and TRIM(n.name) = '" + name + "'"
                       " and r.companyKey = " + QString::number(companyKey);

 QSqlQuery query = QSqlQuery(db);
 qDebug() << commandText;
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(std::move(query));
//  db.close();
//  exit(EXIT_FAILURE);
  return myArray;
 }
 if (!query.isActive())
 {
  return myArray;
 }
 //                myArray = new LatLng[myReader.RecordsAffected];

 while (query.next())
 {
  RouteData rd = RouteData();
  rd._route = query.value(0).toInt();
  rd._name = query.value(1).toString();
  rd._dateBegin = query.value(2).toDate();
  rd._dateEnd = query.value(3).toDate();
  rd._lineKey = query.value(4).toInt();
  rd._companyKey = query.value(5).toInt();
  rd._tractionType = query.value(6).toInt();
  rd._direction = query.value(7).toString();
  rd._normalEnter = query.value(8).toInt();
  rd._normalLeave = query.value(9).toInt();
  rd._reverseEnter =query.value(10).toInt();
  rd._reverseLeave = query.value(11).toInt();
  rd._alphaRoute =query.value(12).toString();
  rd._oneWay = query.value(13).toString();
  rd._trackUsage = query.value(14).toString();
  rd._tracks = query.value(15).toInt();
  rd._baseRoute = query.value(16).toInt();
  rd._routeId =query.value(17).toInt();
  myArray.append(rd);
 }

 return myArray;
}
#endif
// List segments in a route on a given date
QList<SegmentData*> SQL::getSegmentDatasForDate(qint32 route, QString name, int companyKey, QDate date)
{
 QList<SegmentData*> myArray;

 QString where = " where Route = " + QString("%1").arg(route) + ""
 " and '" + date.toString("yyyy/MM/dd") + "' between startDate and endDate"
 " and trim(RouteName) = '" + name.trimmed() + "'"
 " and companyKey = " + QString::number(companyKey);
 return segmentDataListFromView(where);
}

QList<SegmentData*> SQL::getRouteDatasForDate(int segmentId, QDate date)
{
 QString where = " where '" + date.toString("yyyy/MM/dd") + "' between startDate and endDate"
                       " and segmentid = "+QString::number(segmentId);
 return segmentDataListFromView(where);
}

QList<SegmentData> SQL::getRouteDatasForDate(int segmentId, QString date)
{
 QList<SegmentData> myArray;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT a.Route,n.Name,a.StartDate,a.EndDate,LineKey,a.companyKey,"
                       " a.tractionType, a.direction, normalEnter, normalLeave,"
                       " reverseEnter, reverseLeave, routeAlpha, s.tracks, a.trackUsage,"
                       " a.oneWay, s.street, s.description, c.baseRoute, a.routeId"
                       " from Routes a"
                       " join AltRoute c on a.route = c.route"
                       " join Segments s on a.lineKey = s.segmentId"
                       " join RouteName n on a.routeId = n.routeid"
                       " where '" + date + "' between a.startDate and a.endDate"
                       " and lineKey = "+QString::number(segmentId);

 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 if (!query.isActive())
 {
  return myArray;
 }
 //                myArray = new LatLng[myReader.RecordsAffected];

 while (query.next())
{
 SegmentData sd = SegmentData();
 sd._route = query.value(0).toInt();
 sd._routeName = query.value(1).toString();
 sd._dateBegin = query.value(2).toDate();
 sd._dateEnd = query.value(3).toDate();
 sd._segmentId = query.value(4).toInt();
 sd._companyKey = query.value(5).toInt();
 sd._tractionType = query.value(6).toInt();
 sd._direction = query.value(7).toString();
 sd._normalEnter = query.value(8).toInt();
 sd._normalLeave = query.value(9).toInt();
 sd._reverseEnter =query.value(10).toInt();
 sd._reverseLeave = query.value(11).toInt();
 sd._alphaRoute =query.value(12).toString();
 sd._tracks = query.value(13).toInt();
 sd._trackUsage = query.value(14).toString();
 sd._oneWay = query.value(15).toString();
 sd._streetName = query.value(16).toString();
 sd._description = query.value(17).toString();
 sd._baseRoute = query.value(18).toInt();
 sd._routeId = query.value(19).toInt();
 myArray.append(sd);
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

        QString commandText = "select a.route, n.name, a.startdate, a.endDate, "
            "a.companyKey, tractionType,"
            " b.routeAlpha, baseRoute, a.routeId"
            " from Routes a "
            " join AltRoute b on a.route = b.route"
            " join RouteName n on a.routeId = n.routeId"
            " where lineKey = " + QString("%1").arg(segmentid) +
            " and '" + date + "' between a.startDate and a.endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
            return myArray;
        }
        while (query.next())
        {
            rd = RouteData();
            rd._route = query.value(0).toInt();
            rd._name = query.value(1).toString();
            rd._dateBegin = query.value(2).toDate();
            rd._dateEnd = query.value(3).toDate();
            rd._companyKey = query.value(4).toInt();
            rd._tractionType = query.value(5).toInt();
            rd._alphaRoute = query.value(6).toString();
            rd._baseRoute = query.value(7).toInt();
            rd._routeId = query.value(8).toInt();
            myArray.append(rd);
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);

    }

    return myArray;
}

bool SQL::saveRouteSequence(RouteData rd, int firstSegment, QString whichEnd)
{
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString commandText = QString("select count(*) from RouteSeq "
                        " where route = %1 and name = '%2'"
                        "  and startDate = '%3' and endDate = '%4'")
                        .arg(rd._route).arg(rd._name)
    .arg(rd._dateBegin.toString("yyyy/MM/dd"),rd._dateEnd.toString("yyyy/MM/dd"));
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  int count =0;
  while(query.next())
  {
   count = query.value(0).toInt();
  }
  if(count)
  {
   commandText = QString("update RouteSeq set segmentList ='%5', firstSegment = %6, whichEnd = '%7' "
                        " where route = %1 and name = '%2'"
                        "  and startDate = '%3' and endDate = '%4'")
                        .arg(rd._route).arg(rd._name)
    .arg(rd._dateBegin.toString("yyyy/MM/dd"),rd._dateEnd.toString("yyyy/MM/dd"),
         rd.seqToString()).arg(firstSegment).arg(whichEnd);
  } else {
   commandText = QString("insert into RouteSeq (route, routeId,name, startDate, endDate, segmentList,"
                         "firstSegment, whichEnd)"
                         " values (%1, '%2', '%3', '%4', '%5', %6, '%7')")
     .arg(rd._route, rd.routeId()).arg(rd._name)
     .arg(rd._dateBegin.toString("yyyy/MM/dd"),rd._dateEnd.toString("yyyy/MM/dd"),
          rd.seqToString()).arg(firstSegment).arg(whichEnd);
  }
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  return true;
}

bool SQL::addRouteSeq(RouteSeq rs)
{
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = QString("insert into RouteSeq (route, name, startDate, endDate, segmentList,"
                       "firstSegment, whichEnd)"
                       " values (%1, '%2', '%3', '%4', '%5', %6, '%7')")
   .arg(rs.route()).arg(rs.routeName())
   .arg(rs.startDate().toString("yyyy/MM/dd"),rs.endDate().toString("yyyy/MM/dd"),
        rs._rd.seqToString()).arg(rs.firstSegment()).arg(rs.whichEnd());

 QSqlQuery query;
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
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
QList<SegmentInfo> SQL::getIntersectingSegments(double lat, double lon, double radius, RouteType type)
{
 QList<SegmentInfo> myArray;
 SegmentInfo si =  SegmentInfo();
// double startLat=0, startLon=0, endLat=0, endLon = 0;
// qint32 segmentId=0, sequence = 0;
 double distance = 0, length =0;
//qint32 currSegment = -1;
 double curSegmentDistance = radius+1;
// QString streetName = "", description="", oneWay="", pointArray="" ;
 QString typeWhere = "type="+QString::number(type);
// int tracks = 0;
 if(type == Surface || type == SurfacePRW)
  typeWhere = "type in(0,1)";
 if(type == RapidTransit || type == Subway)
  typeWhere = "type in(2,3)";

 QString distanceWhere;
#ifndef NO_UDF
 distanceWhere = " and (distance(" + QString("%1").arg(lat,0,'f',8) + ","
                          + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < "
                          + QString("%1").arg(radius,0,'f',8 )
                          + " OR distance(" + QString("%1").arg(lat,0,'f',8) + ","
                          + QString("%1").arg(lon,0,'f',8)
                          + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") ";
#endif
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();

  QString commandText;

  if(config->currConnection->servertype() != "MsSql")
   commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, "
        " a.length, a.street, "
        " a.description, a.OneWay, a.pointArray, a.tracks, a.type, a.startDate, a.doubledate, a.enddate,"
        " a.streetId, a.rowid"
        " from Segments a "
        " where "+typeWhere  + distanceWhere +
        " order by a.segmentId";
   else
    commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon,"
        " a.length, a.street, a.description, a.OneWay, a.pointArray, a.tracks,"
        " a.type, a.startDate, a.doubledate, a.enddate,"
        " a.streetId, a.rowid"
        " from Segments a "
        "where "+ typeWhere  + distanceWhere +
        "order by a.segmentId";
  //qDebug() << commandText + "\n";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }

  while(query.next())
  {
   si = SegmentInfo();
   si._segmentId = query.value(0).toInt();
   si._startLat = query.value(1).toDouble();
   si._startLon = query.value(2).toDouble();
   si._endLat = query.value(3).toDouble();
   si._endLon = query.value(4).toDouble();
   si._length = query.value(5).toDouble();
   si._streetName = query.value(6).toString();
   si._description = query.value(7).toString();
   //sd._oneWay = query.value(8).toString();
   si.setPoints(query.value(9).toString());
   si._tracks = query.value(10).toInt();
   si._routeType = (RouteType)query.value(11).toInt();
   si._dateBegin = query.value(12).toDate();
   si._dateDoubled = query.value(13).toDate();
   si._dateEnd = query.value(14).toDate();
   si._streetId = query.value(15).toInt();
   si._rowid = query.value(16).toInt();
#ifdef NO_UDF
   // eliminate segments not close
   if((Distance(lat, lon, si._startLat, si._startLon) > radius) &&
      (Distance(lat, lon, si._endLat, si._endLon) > radius))
    continue;
#endif
    curSegmentDistance = radius + 1.0;
//   }

   distance = Distance(lat, lon, si._startLat, si._startLon);
   if (distance < radius)
   {
    si._whichEnd = "S";
   }
   // check the ending point
   distance = Distance(lat, lon, si._endLat, si._endLon);
   if (distance < radius)
   {
    si._whichEnd = "E";
   }
//  }

//  if(curSegmentDistance < radius)
      myArray.append(si);
  }
 }
 catch (Exception e)
 {
     //myExceptionHandler(e);
 }

 return myArray;
}

#endif

// Return a list of all segments that have their starting or ending location within the stated radius
QList<SegmentInfo> SQL::getIntersectingSegments(double lat, double lon, double radius)
{
 QList<SegmentInfo> myArray;
 SegmentInfo si =  SegmentInfo();
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
  QString distanceWhere;
#ifndef NO_UDF
  distanceWhere = "(distance(" + QString("%1").arg(lat,0,'f',8) + ","
                            + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < "
                            + QString("%1").arg(radius,0,'f',8 )+ " OR distance("
                            + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8)
                            + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ")";
#endif
  QString commandText;
  //if(config->currConnection->servertype() != "MsSql")
      commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.length,"
                    " a.street, a.type, a.description,  a.oneWay, a.pointArray,"
                    " a.tracks, a.startDate, a.doubledate, a.endDate, a.location, a.newerName, "
                    " a.streetId"
                    " from Segments a where " + distanceWhere +
                    " order by segmentId";
//  else
//  commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, a.[key], a.sequence, a.length, a.streetName, b.type from LineSegment a join Segments b on a.segmentId = b.segmentId where (dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.startLat, a.startLon) < " + QString("%1").arg(radius,0,'f',8 )+ " OR dbo.distance(" + QString("%1").arg(lat,0,'f',8) + "," + QString("%1").arg(lon,0,'f',8) + ", a.endLat, a.endLon) < " + QString("%1").arg(radius,0,'f',8) + ") order by segmentId, sequence";
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
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
        myArray.append(si);
    }

    si =  SegmentInfo();
    currSegment = segmentId;
    curSegmentDistance = radius + 1.0;
   }

   distance = Distance(lat, lon, startLat, startLon);
   si._segmentId = segmentId;
   si._startLat = startLat;
   si._startLon = startLon;
   si._endLat = endLat;
   si._endLon = endLon;
   si._streetName = streetName;
   si._routeType = type;
   si._tracks = query.value(11).toInt();
   si._dateBegin = query.value(12).toDate();
   si._dateDoubled = query.value(13).toDate();
   si._dateEnd = query.value(14).toDate();
   si._location = query.value(15).toString();
   si._newerStreetName = query.value(16).toString();
   si._streetId = query.value(17).toInt();
#ifdef NO_UDF
   // eliminate segments not close
   if((Distance(lat, lon, si._startLat, si._startLon) > radius) &&
      (Distance(lat, lon, si._endLat, si._endLon) > radius))
    continue;
#endif

   if (distance < curSegmentDistance)
   {
    curSegmentDistance = distance;
    si._length = distance;
    si._whichEnd = "S";
   }
   // check the ending point
   distance = Distance(lat, lon, endLat, endLon);
   if (distance < curSegmentDistance)
   {
    curSegmentDistance = distance;
    si._length = distance;
    si._whichEnd = "E";
   }
   si._description = description;
   //si._oneWay = oneWay;
   si.setPoints(pointArray);
  }

  if (curSegmentDistance < radius)
   myArray.append(si);
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
QList<SegmentData*> SQL::getIntersectingRouteSegmentsAtPoint(int ignoreThis, double lat, double lon, double radius, qint32 route,
                                                            QString routeName, QString date)
{
#if 0
 QList<SegmentData*> myArray = QList<SegmentData*>();
 double distanceToStart = 0, distanceToEnd = 0;

 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "select b.segmentId, b.startLat, b.startLon, b.endLat, b.EndLon,"
                       " c.direction, b.description, c.oneWay,  c.next, c.prev,"
                       " c.normalEnter, c.normalLeave, c.reverseEnter, c.reverseLeave,"
                       " b.pointArray, b.tracks, c.startDate,"
                       " c.endDate, b.location, b.type, b.street"
                       " from Segments b join Routes c on c.lineKey = b.segmentId"
                       " where c.route = " + QString("%1").arg(route)
                       + " and c.name = '" + routeName
                       + "' and '" + date + "' between c.startDate and c.endDate"
                       + " and b.segmentId != " + QString("%1").arg(ignoreThis)
                       + " order by b.segmentId";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
  SQLERROR(std::move(query));
  db.close();
  exit(EXIT_FAILURE);
 }

 while(query.next())
 {
  SegmentData* sd = new SegmentData();

  sd->_segmentId = query.value(0).toInt();
  sd->_startLat = query.value(1).toDouble();
  sd->_startLon = query.value(2).toDouble();
  sd->_endLat = query.value(3).toDouble();
  sd->_endLon = query.value(4).toDouble();
  sd->_direction = query.value(5).toString();
  sd->_description = query.value(6).toString();
  sd->_oneWay = query.value(7).toString();
  sd->_next = query.value(8).toInt();
  sd->_prev = query.value(9).toInt();
  sd->_normalEnter = query.value(10).toInt();
  sd->_normalLeave = query.value(11).toInt();
  sd->_reverseEnter = query.value(12).toInt();
  sd->_reverseLeave = query.value(13).toInt();
  sd->setPoints(query.value(14).toString());
  sd->_tracks = query.value(15).toInt();
  sd->_dateBegin = query.value(16).toDate();
  sd->_dateEnd = query.value(17).toDate();
  sd->_location = query.value(18).toString();
  sd->_routeType = (RouteType)query.value(19).toInt();
  sd->_streetName = query.value(20).toString();
  if(sd->_pointList.count() >=2)
  {
   sd->_bearingStart = Bearing(sd->_startLat, sd->_startLon, sd->_pointList.at(1).lat(), sd->_pointList.at(1).lon());
   sd->_bearingEnd = Bearing(sd->_pointList.at(sd->_points-2).lat(), sd->_pointList.at(sd->_points-2).lon(), sd->_endLat, sd->_endLon);
   sd->_bearing = Bearing(sd->_startLat, sd->_startLon, sd->_endLat, sd->_endLon);
   sd->_direction = sd->_bearing.strDirection();
  }
  distanceToEnd = Distance(lat, lon, sd->_endLat, sd->_endLon);
  distanceToStart = Distance(lat, lon, sd->_startLat, sd->_startLon);
  if (distanceToEnd < distanceToStart)
      sd->_whichEnd = "E";
  else
      sd->_whichEnd = "S";
  sd->_route = route;
  sd->_routeName = routeName;

  if((sd->tracks() == 1 && sd->oneWay() == "Y" && sd->whichEnd() == "E")
     || (sd->tracks() == 2 && sd->oneWay()=="Y"
         &&((sd->whichEnd() == "E" && sd->trackUsage() == "R")
         || (sd->whichEnd() == "S" && sd->trackUsage() == "L"))))
   continue;


  if (distanceToEnd < radius || distanceToStart < radius)
   myArray.append(sd);
 }
#endif
 QString where = "where Route = " + QString("%1").arg(route)
         + " and trim(RouteName) = '" + routeName.trimmed()
         + "' and '" + date + "' between StartDate and EndDate"
         + " and SegmentId != " + QString("%1").arg(ignoreThis)
         + " order by SegmentId";
 return segmentDataListFromView(where);

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
QList<SegmentData*> SQL::getIntersectingRouteSegmentsAtPoint(SegmentData* sd1,
                                                             double radius,
                                                             QString date,
                                                             QMap<int,SegmentData*> segMap,
                                                             int firstSegment,
                                                             bool enableTurnCheck)
{
 QList<SegmentData*> myArray = QList<SegmentData*>();
 double distanceToStart = 0, distanceToEnd = 0;
 double angle;

 QSqlDatabase db = QSqlDatabase::database();
 double lat, lon;
 if(sd1->whichEnd()=="S")
 {
  lat = sd1->_startLat;
  lon = sd1->_startLon;
 }
 else
 {
  lat = sd1->_endLat;
  lon = sd1->_endLon;
 }
 QString commandText = "select s.segmentId, s.startLat, s.startLon, s.endLat, s.EndLon,"
                       " r.direction, s.description, r.oneWay, r.next, r.prev,"
                       " r.normalEnter, r.normalLeave, r.reverseEnter, r.reverseLeave,"
                       " s.pointArray, s.tracks, r.startDate,"
                       " r.endDate, s.location, s.type, s.street, r.route, r.name, "
                       " distance(" + QString::number(lat) +"," +  QString::number(lon) +", s.startLat, s.startLon) as toStart,"
                       " distance(" + QString::number(lat) +"," + QString::number(lon) +", s.endLat, s.endLon) as toEnd,"
                       " r.nextR, r.prevR "
                       " from Segments s "
                       " join Routes r on r.lineKey = s.segmentId"
                       " where r.route = " + QString("%1").arg(sd1->_route)
                       + " and r.name = '" + sd1->_routeName
                       + "' and '" + date + "' between r.startDate and r.endDate"
                       + " and s.segmentId != " + QString("%1").arg(sd1->_segmentId)
                       + " and MIN(toStart, toEnd) < " +QString::number(radius)
                       + " order by s.segmentId";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 qDebug() << commandText;
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }

 while(query.next())
 {
  SegmentData* sd = new SegmentData();

  sd->_segmentId = query.value(0).toInt();
  sd->_startLat = query.value(1).toDouble();
  sd->_startLon = query.value(2).toDouble();
  sd->_endLat = query.value(3).toDouble();
  sd->_endLon = query.value(4).toDouble();
  sd->_direction = query.value(5).toString();
  sd->_description = query.value(6).toString();
  sd->_oneWay = query.value(7).toString();
  sd->_next = query.value(8).toInt();
  sd->_prev = query.value(9).toInt();
  sd->_normalEnter = query.value(10).toInt();
  sd->_normalLeave = query.value(11).toInt();
  sd->_reverseEnter = query.value(12).toInt();
  sd->_reverseLeave = query.value(13).toInt();
  sd->setPoints(query.value(14).toString());
  sd->_tracks = query.value(15).toInt();
  sd->_dateBegin = query.value(16).toDate();
  sd->_dateEnd = query.value(17).toDate();
  sd->_location = query.value(18).toString();
  sd->_routeType = (RouteType)query.value(19).toInt();
  sd->_streetName = query.value(20).toString();
  sd->_route = query.value(21).toInt();
  sd->_routeName = query.value(22).toString();
  double toStart = query.value(23).toDouble();
  double toEnd = query.value(24).toDouble();
  sd->_nextR = query.value(25).toInt();
  sd->_prevR = query.value(26).toInt();

  if(sd->_pointList.count() >=2)
  {
   sd->_bearingStart = Bearing(sd->_startLat, sd->_startLon, sd->_pointList.at(1).lat(), sd->_pointList.at(1).lon());
   sd->_bearingEnd = Bearing(sd->_pointList.at(sd->_points-2).lat(), sd->_pointList.at(sd->_points-2).lon(), sd->_endLat, sd->_endLon);
   sd->_bearing = Bearing(sd->_startLat, sd->_startLon, sd->_endLat, sd->_endLon);
   sd->_direction = sd->_bearing.strDirection();
  }

  double lat, lon;
  if(sd1->whichEnd() == "S")
  {
   lat = sd1->_startLat;
   lon = sd1->_startLon;
  }
  else {
   lat = sd1->_endLat;
   lon = sd1->_endLon;

  }
  distanceToEnd = Distance(lat, lon, sd->_endLat, sd->_endLon);
  distanceToStart = Distance(lat, lon, sd->_startLat, sd->_startLon);
  if (distanceToEnd < distanceToStart)
      sd->_whichEnd = "E";
  else
      sd->_whichEnd = "S";

  if((sd->tracks() == 1 && sd->oneWay() == "Y" && sd->whichEnd() == "E")
     || (sd->tracks() == 2 && sd->oneWay()=="Y"
         &&((sd->whichEnd() == "E" && sd->trackUsage() == "R")
         || (sd->whichEnd() == "S" && sd->trackUsage() == "L"))))
   continue;


  if (distanceToEnd < radius || distanceToStart < radius)
   myArray.append(sd);
 }

 // Delete any impossible matches
 for(int i=myArray.count()-1; i >= 0; i--)
 {
  SegmentData* sdj = myArray.at(i);

  if((sdj->tracks() == 1 && sdj->whichEnd()=="E" && sdj->oneWay() == "Y")
     || (sdj->tracks() == 2 && sdj->oneWay()=="Y"
      && ((sdj->whichEnd() == "S" && sdj->trackUsage() == "L")
       || (sdj->whichEnd() == "E" && sdj->trackUsage() == "R") ) ))
  {
   myArray.removeAt(i); // can't connect!
   continue;
  }

  SegmentData* sd0 = segMap.value(sdj->segmentId());
  if(sd0->segmentId() == firstSegment)
   continue; // let initial segment through so it can be checked to see if done!
  if(sd0->oneWay() == "Y" && sd0->tracks()== 1 && sd0->sequence() != -1)
  {
   myArray.removeAt(i); // already used
   continue;
  }
  if(sd0->tracks()==2 && sd0->sequence() != -1 && sd0->returnSeq() != -1)
   myArray.removeAt(i); // already used
 }
 // now check turn hints
 if(enableTurnCheck)
 {
  bool bypassChecks = myArray.count() < 2;
  for(int i=myArray.count()-1; i >= 0; i--)
  {
   SegmentData* sdj = myArray.at(i);

   // in the interest of getting errors when only one intersect is present, only chec when there are
   // more than 1.
   if(bypassChecks)
    continue;
   angle = intersectingAngle(*sd1, *sdj);
 //  if(!bOutbound)
 //   angle = -angle;
 //  double angle1 = intersectingAngle(*sdj, *sd1);

 //  qDebug() << "angle " << sd1->segmentId() << " to intersecting " << sdj->segmentId()
 //           << " = " << angle;

   bool canConnect = false;
   if(sd1->whichEnd()=="E")
   {
    switch (sd1->normalLeave())
    {
    case 0: // ahead
     if(angle > -45 && angle < 45)
       canConnect = true;
     break;
    case 1: // left
     if(angle > -135 && angle < -45 )
      canConnect = true;
     break;
    case 2:
     if(angle > 45 && angle < 135)
      canConnect =true;
     break;
    default:
     break;
    }
   }
   if(!canConnect)
    myArray.removeAt(i);
  }
 }
 return myArray;
}
#if 1
// original function
int SQL::sequenceRouteSegments(qint32 segmentId, QList<SegmentData*> segmentList,
                               RouteData* rd, QString whichEnd)
{
 int route = rd->_route;
 QString routeName = rd->_name;
 QDate date = rd->_dateEnd;
 QList<SegmentData*> intersects;
 qint32 endSegment = -1;
 double dToBegin = 0, dToEnd=0;
 double diff, diff2;
 qint32 matchedSegment=-1;
 qint32 startingSegment = segmentId;
 qint32 currSegment = segmentId;
 SegmentData* sd = nullptr;
 qint32 sequence = 0, reverseSeq = 0;
 bool bFirst = true;
 //double nextLat = 0, nextLon = 0;
 bool bOutbound = true;
 //double a1 = 0, a2 = 0;
 QMap<int, SegmentData*> segMap;
 QVariantList objArray;

 if(!dbOpen())
  throw Exception(tr("database not open: %1").arg(__LINE__));

 //foreach (segmentInfo si0 in segmentList)
 for(int i = 0; i < segmentList.count(); i ++)
 {
  SegmentData* sd0 = (SegmentData*)segmentList.at(i);
  sd0->_sequence = -1;
  sd0->_returnSeq = -1;
  sd0->_next = -1;
  sd0->_prev = -1;
  sd0->_nextR = -1;
  sd0->_prevR = -1;

  segMap.insert(sd0->segmentId(), sd0);
 }

 sd = segMap.value(currSegment);
 //sd->_whichEnd = whichEnd;
 if(sd->oneWay()== "Y")
 {
  if(sd->tracks() ==1 || (sd->trackUsage()!="L"))
  {
//   nextLat = sd->endLat();
//   nextLon = sd->endLon();
   whichEnd = "E";
  }
  else
  {
//   nextLat = sd->startLat();
//   nextLon = sd->startLon();
   whichEnd = "S";
  }
 }
 else
 if(sd->whichEnd()=="S")
 {
//  nextLat = sd->startLat();
//  nextLon = sd->startLon();
 }
 else
 {
//  nextLat = sd->endLat();
//  nextLon = sd->endLon();
 }
 rd->addSequence(QPair<int, QString>(sd->segmentId(), whichEnd=="E"?"F":"R"));
 while (currSegment >= 0 )//&& sequence < segmentList.Count && reverseSeq < segmentList.Count)
 {
  sd = segMap.value(currSegment);
  sd->_whichEnd = whichEnd;
  if(currSegment == 1528 )
   qDebug() << "break";
//  if(sd->sequence() != -1)
//   bOutbound = ! bOutbound;
  if (bOutbound)
  {
   //outbound
#if 1
   if (sd->_sequence != -1)
   {
    //currSegment = -1;
    //Console.WriteLine("Possible endless loop at segment: " + si->SegmentId + " and segment: " + sii->SegmentId);
    //break;
    bOutbound = !bOutbound;
    sd->_returnSeq = reverseSeq++;
   }
   else
    sd->_sequence = sequence++;
#endif
  }
  else
  {
   //inbound
   if (sd->_returnSeq != -1)
   {
    currSegment = -1;
    qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(sd->_segmentId)
               + " and segment: " + QString("%1").arg(sd->_segmentId);
    break;
   }
   sd->_returnSeq = reverseSeq++;
  }

  if (currSegment == -1)
   break;
  intersects = getIntersectingRouteSegmentsAtPoint(sd, .020, date.toString("yyyy/MM/dd"), segMap,startingSegment);
  if(intersects.count() == 0 )
  {
   if(sd->oneWay() == "Y")
   {
    QMessageBox::critical(nullptr, tr("Error"), tr("No connecting segment can be found for\n"
                          "segment%1 %2 to connect to ").arg(sd->segmentId()).arg(sd->description()));
    return -1;
   }
   bOutbound = !bOutbound;
   sd->_whichEnd = (sd->_whichEnd=="S")?"E":"S";
   intersects = getIntersectingRouteSegmentsAtPoint(sd, .020, date.toString("yyyy/MM/dd"),
                                                    segMap, startingSegment, true);
  }

  for(int i = 0; i < intersects.count(); i ++)
  {
   SegmentData* sdj = intersects.at(i);
   SegmentData* nextSd = nullptr;
   matchedSegment = -1;
   if(intersects.count() == 1)
   {
    if(sdj->segmentId() == startingSegment)
     return sd->segmentId();
    matchedSegment = sdj->segmentId();
    nextSd = segMap.value(matchedSegment);
    QPair<int, QString> pair = QPair<int, QString>(matchedSegment, sdj->whichEnd()=="S"?"F":"R");
    nextSd->setWhichEnd(sdj->whichEnd());
    rd->addSequence(pair);
    if(sdj->sequence()!= -1 && sdj->returnSeq() == -1)
     bOutbound = !bOutbound;
    whichEnd = (sdj->whichEnd() == "S")?"E":"S"; // next segment will match to other end!

//    MainWindow::instance()->m_bridge->processScript("clearMarker");
//     objArray << 0 << nextLat<<nextLon<<1<<"pointO"<<sd->_segmentId;
//    MainWindow::instance()->m_bridge->processScript("addMarker",objArray);
    if (sd->_whichEnd == "E")
     sd->_next = matchedSegment;
    else
     sd->_nextR = matchedSegment;
    if(nextSd->whichEnd() == "S")
     nextSd->_prev = sd->segmentId();
    else
     nextSd->_prevR = sd->segmentId();
    nextSd->_bNeedsUpdate = true;
    sd->_bNeedsUpdate=true;
    updateRoute(*sd,*sd);

    if (!bFirst && currSegment == startingSegment)
    {
     // Back at the beginning.
      SegmentData* sdi = segMap.value(currSegment);
       sdi->_next = sd->_next;
       sdi->_prev = sd->_prev;
       sdi->_nextR = sd->_nextR;
       sdi->_prevR = sd->_prevR;
       sd->_bNeedsUpdate = true;
       updateRoute(*sdi,*sdi);
       sd = sdi;
       whichEnd = intersects.at(0)->whichEnd()=="S"?"E":"S";
       if (bOutbound)
       {
        if (sd->_sequence != -1)
        {
         currSegment = -1;
         qDebug() << "Possible endless loop at segment: " + QString("%1").arg(sd->_segmentId) + "  " + sd->_description;
         break;
        }
        sd->_sequence = sequence++;
       }
       else
       {
        if (sd->_returnSeq != -1)
        {
         currSegment = -1;
         qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(sd->_segmentId) + "  " + sd->_description;
         break;
        }
        sd->_returnSeq = reverseSeq++;
       }

     return currSegment;   // back at the beginning
    }
    currSegment = matchedSegment;
    break;
   }
   bFirst = false;
   continue;
  }
  QString text = "";

  if (matchedSegment == -1)
  {
   text ="No segment matching for segment " + QString("%1").arg(sd->_segmentId)
         + " " + sd->_description;
   QList<SegmentData*> options = getIntersectingRouteSegmentsAtPoint(sd, .020,
                                                                     date.toString("yyyy/MM/dd"),
                                                                     segMap, startingSegment, false);
   QButtonGroup* bg = nullptr;
   QWidget* extra = nullptr;
   int selected =-1;
   for(int i = 0; i < options.count(); i ++)
   {
    SegmentData* sdk = options.at(i);
    if (sdk->_segmentId == sd->_segmentId)
     continue; // Ignore myself

//    if((sdk->tracks() == 1 && sdk->whichEnd()=="E" && sdk->oneWay()=="Y") || (sdk->tracks() == 2
//        && ((sdk->whichEnd()== "S" && sdk->trackUsage() == "L")
//            || (sdk->whichEnd() == "E" && sdk->trackUsage() == "R") ) ))
//     text = text + "no possible match with segment " + QString("%1").arg(sdk->_segmentId)
//       + " angle: " + QString("%1").arg(diff) + "° " + sdk->_description;
    if(intersects.count() > 1)
    {
     if(!bg)
     {
      bg =  new QButtonGroup();
      extra= new QWidget();
      extra->setLayout(new QVBoxLayout());
     }
     QRadioButton* rb = new QRadioButton(tr("Segment %1 - %2").arg(sdk->segmentId()).arg(sdk->_description));
     bg->addButton(rb, sdk->segmentId());

     extra->layout()->addWidget(rb);
     double angle = intersectingAngle(*sd, *sdk);
     text = text + "\n possible choices are segment " + QString("%1").arg(sdk->_segmentId)
      + " angle: " + QString("%1").arg(angle) + "° " + sdk->_description;

    }
   }
   if(text != "")
   {

    MainWindow::instance()->m_bridge->processScript("clearMarker");
    //QVariantList objArray;
    if(sd->whichEnd()=="S")
     objArray << 0 << sd->startLat()<<sd->startLon()<<7<<"pointO"<<sd->_segmentId;
    else
     objArray << 0 << sd->endLat()<<sd->endLon()<<7<<"pointO"<<sd->_segmentId;
    MainWindow::instance()->m_bridge->processScript("addMarker",objArray);
    qDebug() << text;
    QMessageBox* box = new QMessageBox(QMessageBox::Information,
                                       tr("Segment connection error"),
                                       tr("The following segment has an error connecting to the next segment"));
    box->setInformativeText(text);
    if(extra)
     box->layout()->addWidget(extra);
    box->exec();
    if(bg && bg->checkedId() >0)
     selected =bg->checkedId();
   }
   if(segmentList.count()==1 || selected >0)
   {
    if(selected > 0)
     matchedSegment = selected;
    else
     matchedSegment = segmentList.at(0)->segmentId();

    if(sd->whichEnd() == "E")
    {
     sd->_next = matchedSegment;
     if (sd->_sequence != -1)
     {
      currSegment = -1;
      qDebug() << "Possible endless loop at segment: " + QString("%1").arg(sd->_segmentId) + "  " + sd->_description;
      break;
     }
    }
    else
     sd->_nextR = matchedSegment;
    SegmentData* nextSegmentData = segMap.value(matchedSegment);
    nextSegmentData->setWhichEnd(segmentList.at(0)->_whichEnd);
    if(nextSegmentData->whichEnd()== "S")
     nextSegmentData->_prev = sd->segmentId();
    else
     nextSegmentData->_prevR = sd->segmentId();
    if (sd->_returnSeq != -1)
    {
     currSegment = -1;
     qDebug() <<"Possible endless loop at segment: " + QString("%1").arg(sd->_segmentId) + "  " + sd->_description;
     break;
    }
    sd->_returnSeq = reverseSeq++;
   }
  }
  if(matchedSegment > 0)
   continue;
  if (bOutbound)
  {
   if (sd->_next == -1  || (intersects.count() == 1 && intersects.at(0)->segmentId() == sd->_segmentId))
   {
    bOutbound = !bOutbound;
   }
  }
  else
   if (sd->_prev == -1)
   {
    currSegment = -1;
   }
 }
 return endSegment;
}
#else
bool SQL::canConnect(SegmentData sd1, QString matchedTo, SegmentData sd2)
{
 double diff = connectingAngle(sd1, sd2);
 qDebug() << "angle difference = " << diff << " to " << sd2.segmentId() << "-" << sd2.description();

 if((sd2.tracks() == 1 && sd2.oneWay() == "Y" && sd2.whichEnd() == "E")
    || (sd2.tracks() == 2 && sd2.oneWay()=="Y"
        &&((sd2.whichEnd() == "E" && sd2.trackUsage() == "R")
        || (sd2.whichEnd() == "S" && sd2.trackUsage() == "L"))))
  return false;

 if((sd1.tracks()==2 && sd1.oneWay()!= "Y" && sd2.tracks() == 2 && sd2.oneWay()!="Y" )
    || (sd1.tracks() == 2 && sd1.oneWay()!= "Y" && (sd2.tracks()==1 && sd2.oneWay()!= "Y"))
    || (sd1.tracks() == 1 && sd1.oneWay()!= "Y" && (sd2.tracks() == 1 && sd2.oneWay()!= "Y"))
    || (sd1.tracks() == 1 && (sd2.tracks()==2 && sd2.oneWay()== "Y"))
    || (sd1.tracks() == 1 /*&& sd->oneWay() == "Y"*/ && (sd2.tracks()==1 && sd2.whichEnd()=="S"))
    || (sd1.tracks() == 2 && sd2.tracks() ==1 && (sd2.oneWay() != "Y" ||sd2.whichEnd()=="S"))
//    || (sd1.tracks() == 2 &&  sd2.tracks() ==2 && (diff < 45))
    || (sd2.whichEnd() == "S" ||  sd2.oneWay() != "Y")
    || (sd2.tracks()==1 && sd2.whichEnd() == "S" && sd2.oneWay() != "Y"))
  return true;
 return false;
}

int SQL::sequenceRouteSegments(qint32 startSegment, QList<SegmentData> segmentList,
                               qint32 route, QString name, QString date, QString whichEnd)
{
 QList<SegmentData> intersects;
 qint32 endSegment = -1;
 double dToBegin = 0, dToEnd=0;
 double diff;
 SegmentData* matchedSegment = nullptr;
 qint32 _startSegment = startSegment;
 qint32 currSegment = startSegment;
 SegmentData sdx = SegmentData();
 sdx._segmentId = -1;
 SegmentData* sd;
 qint32 sequence = 0, reverseSeq = 0;
 bool bForward = true;
 double nextLat = 0, nextLon = 0;
 bool bOutbound = true;
 double a1 = 0, a2 = 0;
 bool bfirstSegment = true;
 QMap<int, SegmentData*> segmentMap;
 QString matchedto = whichEnd;

 if(!dbOpen())
  throw Exception(tr("database not open: %1").arg(__LINE__));

 //foreach (segmentInfo si0 in segmentList)
 for(int i = 0; i < segmentList.count(); i ++)
 {
  SegmentData* sd0 = (SegmentData*)&segmentList.at(i);
  sd0->_sequence = -1;
  sd0->_returnSeq = -1;
  sd0->_next = -1;
  sd0->_prev = -1;
  sd0->_sequence = -1;
  sd0->_returnSeq = -1;
  segmentMap.insert(sd0->segmentId(), sd0);
 }
 sd = segmentMap.value(startSegment);
 currSegment = sd->segmentId();
 while(currSegment != -1 || (!bForward && currSegment != startSegment))
 {
  if( (bForward && sd->next() == -1)
      || (!bForward && (sd->returnSeq() == -1 || sd->sequence()==-1)) )
  {
   // get segments intersecting to this segment's end.
   if(matchedto == "S")
   {
    intersects = getIntersectingRouteSegmentsAtPoint(sd->segmentId(),
                                                     sd->endLat(), sd->endLon(),
                                                     .020, route, name, date);
    sd->setWhichEnd("E");
   }
   else
   {
    intersects = getIntersectingRouteSegmentsAtPoint(sd->segmentId(),
                                                     sd->startLat(), sd->startLon(),
                                                     .020, route, name, date);
    sd->setWhichEnd("S");
   }

   MainWindow::instance()->m_bridge->processScript("clearMarker");
   QVariantList objArray;
   if(sd->whichEnd()=="E")
    objArray << 0 << sd->startLat()<<sd->startLon()<<2<<"pointO"<<sd->_segmentId;
   else
    objArray << 0 << sd->endLat()<<sd->endLon()<<1<<"pointO"<<sd->_segmentId;
   MainWindow::instance()->m_bridge->processScript("addMarker",objArray);
   qApp->processEvents();
   QThread::msleep(10);

   if(intersects.count()< 1)
   {
    if(bForward && sd->oneWay() != "Y" && intersects.isEmpty())
    {
     if(sd->prev() != -1)
     {
      int nextSegment = sd->prev();
      sd->setSequence(sequence++);
      sd->setReturnSeq(reverseSeq++);
      sd->_bNeedsUpdate = true;

      sd = segmentMap.value(nextSegment);
      if(matchedto == "S")
       matchedto = "E";
      else
       matchedto = "S";
//      sd->setSequence(sequence++);
//      sd->setReturnSeq(reverseSeq++);
      bForward = false;
      continue;
     }
//     else
//     {
//      sd->setSequence(sequence++);
//      sd->setReturnSeq(reverseSeq++);
//     }
    }
    break; // no segments intersect
   }
   for(int i=0; i < intersects.count(); i++)
   {
    SegmentData* currI = (SegmentData*)&intersects.at(i);

//    if(currI->segmentId() == sd->segmentId())
//     continue; // ignore self
    if(intersects.count() < 1)
    {
     currSegment = -1;
     break;
    }

    if(intersects.count() == 1)
    {
#if 0
     if((sd->tracks()==2 && sd->oneWay()!= "Y" && currI->tracks() == 2 && currI->oneWay()!="Y" )
        || (sd->tracks() == 2 && sd->oneWay()!= "Y" && (currI->tracks()==1 && currI->oneWay()!= "Y"))
        || (sd->tracks() == 1 && sd->oneWay()!= "Y" && (currI->tracks() == 1 && currI->oneWay()!= "Y"))
        || (sd->tracks() == 1 && (currI->tracks()==2 && currI->oneWay()== "Y"))
        || (sd->tracks() == 1 /*&& sd->oneWay() == "Y"*/ && (currI->tracks()==1 && currI->whichEnd()=="S"))
        )
#else
     if(canConnect(*sd, whichEnd, *currI))
#endif
     {
      if(currI->segmentId() == startSegment)
      {
       sd->setReturnSeq(reverseSeq++);
       segmentMap.value(currI->segmentId())->setReturnSeq(reverseSeq++);
       MainWindow::instance()->m_bridge->processScript("clearMarker");
       QVariantList objArray;
       if(sd->whichEnd()=="E")
        objArray << 0 << currI->startLat()<<currI->startLon()<<2<<"pointO"<<sd->_segmentId;
       else
        objArray << 0 << currI->endLat()<<currI->endLon()<<1<<"pointO"<<sd->_segmentId;
       MainWindow::instance()->m_bridge->processScript("addMarker",objArray);
       return currI->segmentId();
      }
      sd->_next = currI->segmentId();
      matchedSegment = currI;
      segmentMap.value(sd->_next)->_prev = sd->segmentId();
      if(bForward)
       sd->_sequence = sequence++;
      else
       sd->_returnSeq = reverseSeq++;
      sd->_bNeedsUpdate = true;
      matchedto = currI->whichEnd();
     }
     else
     {
      qDebug() << "no match";
      QString connections = "Possible choices are:\n";
      foreach(SegmentData sdi, intersects)
      {
       connections = connections + tr("Segment %1 - %2 at %3\n").arg(sdi.segmentId()).arg(sdi.description()).arg(sdi.whichEnd() != "S"?"end":"start");
      }
      QMessageBox::warning(nullptr, tr("Connect error"), tr("No suitable segment found to connect to segment %1 - %2 at %3\n"
                           "meeting at %4 degrees")
                           .arg(sd->segmentId()).arg(sd->description()).arg(matchedto == "S"?"end":"start").arg(diff) + connections);
      currSegment = -1;
      break;
     }
     //matchedto = currentIntersect->whichEnd();
     //sd = segmentMap.value(sd->next());
     qDebug() << sd->segmentId() << " " << sd->description() << " connects to "
              << currI->whichEnd() << " " << currI->segmentId() << " " << currI->description();
     qApp->processEvents();
     //sd = currI;
     sd = segmentMap.value(sd->next());
     currSegment = sd->segmentId();
     if(currSegment == 1574)
      qDebug() << "debug halt";
    }
    else
    {
     qDebug() << "more than one intersects";
     matchedSegment = nullptr;
     for(int i=0; i < intersects.count(); i++)
     {
      SegmentData* currI = (SegmentData*)&intersects.at(i);
      //double diff = angleDiff(sd->bearingEnd().getDirection(), -currI->bearingStart().getDirection());
      double diff = connectingAngle(*sd, *currI);
      qDebug() << "angle difference = " << diff << " to " << currI->segmentId() << "-" << currI->description();
//      if((sd->tracks() == 2 && currI->tracks() ==1 && (currI->oneWay() != "Y" ||currI->whichEnd()=="S"))
//         || (sd->tracks() == 2 &&  currI->tracks() ==2 && (diff < 45))
//         || (currI->whichEnd() == "S" ||  currI->oneWay() != "Y")
//         || (currI->tracks()==1 && currI->whichEnd() == "S" && currI->oneWay() != "Y")
//         )
      if(canConnect(*sd, matchedto, *currI))
      {
       int sw;
       if(sd->oneWay() == "Y")
        sw = sd->normalLeave();
       else
        sw = bForward?sd->normalLeave():sd->reverseLeave();
       switch (sw) {
       case 0:
        if(abs(diff)< 45)
         matchedSegment = currI;
        break;
       case 1:
        if(diff < -45)
         matchedSegment = currI;
        break;
       case 2:
        if(diff > 45)
         matchedSegment = currI;
        break;
       }
      }
     }
     if(matchedSegment)
     {
      sd->_next = matchedSegment->segmentId();
      segmentMap.value(sd->_next)->_prev = sd->segmentId();
      if(bForward)
       sd->_sequence = sequence++;
      else
       sd->_returnSeq = reverseSeq++;
      sd->_bNeedsUpdate = true;
      matchedto = matchedSegment->whichEnd();

      qDebug() << sd->segmentId() << " " << sd->description() << " connects to "
               << matchedSegment->whichEnd() << " " << matchedSegment->segmentId() << " " << matchedSegment->description();
      qApp->processEvents();
      //sd = currI;
      sd = segmentMap.value(sd->next());
      currSegment = sd->segmentId();
      break;
     }
     currSegment = -1;
     break;
    }
   }
   if(currSegment == -1)
    break;
  }
  else
  {
   if(sd->returnSeq() == -1)
   {
    bForward = false;
    break;
   }
   if(sd->next() != -1)
    return -1;
  }
 }
 SegmentData* currI = segmentMap.value(currSegment);
 MainWindow::instance()->m_bridge->processScript("clearMarker");
 if(currSegment >0)
 {
  QVariantList objArray;
  if(whichEnd=="E")
   objArray << 0 << currI->startLat()<<currI->startLon()<<2<<"pointO"<<sd->_segmentId;
  else
   objArray << 0 << currI->endLat()<<currI->endLon()<<1<<"pointO"<<sd->_segmentId;
  MainWindow::instance()->m_bridge->processScript("addMarker",objArray);
 }
 return currSegment;
}
#endif
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

double SQL::intersectingAngle(SegmentData sd1, SegmentData sd2)
{
 double angle = -180.0, angleH, angleOff;
 double m1=0, m2=0;
 double aOut=0, aIn =0;
 LatLng l1;
 LatLng l2;
 if(sd1.whichEnd() == "S") // which end is coming out?
 {
  m1 = (sd1.pointList().at(1).lat() - sd1.pointList().at(0).lat())/
    (sd1.pointList().at(1).lon() - sd1.pointList().at(0).lon());
  aOut = Bearing(sd1.pointList().at(1), sd1.pointList().at(0)).angle();
  l1 = sd1.pointList().at(0);
 }
 else
 {
  m1 = (sd1.pointList().at(sd1._points-2).lat() - sd1.pointList().at(sd1._points-1).lat())/
    (sd1.pointList().at(sd1._points-2).lon() - sd1.pointList().at(sd1._points-1).lon());
  aOut = Bearing(sd1.pointList().at(sd1._points-2), sd1.pointList().at(sd1._points-1)).angle();
  l1 = sd1.pointList().at(sd1._points-1);
 }
 if(sd2.whichEnd() == "S") // going in at which end?
 {
  m2 = (sd2.pointList().at(0).lat() - sd2.pointList().at(1).lat())/
    (sd2.pointList().at(0).lon() - sd2.pointList().at(1).lon());
  aIn = Bearing(sd2.pointList().at(0), sd2.pointList().at(1)).angle();
  l2 = sd2.pointList().at(1);
 }
 else
 {
  m2 = (sd2.pointList().at(sd2._points-1).lat() - sd2.pointList().at(sd2._points-2).lat())/
    (sd2.pointList().at(sd2._points-1).lon() - sd2.pointList().at(sd2._points-2).lon());
  aIn = Bearing(sd2.pointList().at(sd2._points-1), sd2.pointList().at(sd2._points-2)).angle();
  l2 = sd2.pointList().at(sd2._points-2);
 }
 double rad  = qAtan(abs((m1-m2)/(1+(m1*m2))));
 angle = qRadiansToDegrees(rad);
 angleH = Bearing(l1, l2).angle();
 angleOff = angleDiff(aOut, angleH);
 if(angleOff < 0)
  angle = -angle;

 qDebug() << sd1._description<< " "<< aOut << " " <<sd1.segmentId()<<sd1.whichEnd()<< " --> " << sd2._description << " "<< aIn <<  " "
          << sd2.segmentId()<<sd2.whichEnd() << " angle: " << angle;

 return angle;
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

double SQL::Distance(LatLng latLng1, LatLng latLng2)
{
    return Distance(latLng1.lat(),latLng1.lon(), latLng2.lat(),latLng2.lon());
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



bool SQL::updateSegment(SegmentInfo* si, bool bNotify)
{
 bool ret = false;
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 si->_length = 0;
 int rows = 0;

 if(si->pointList().isEmpty())
     return false;

 si->_startLat = si->pointList().at(0).lat();
 si->_startLon = si->pointList().at(0).lon();
 for(int i=0; i < si->pointList().count(); i++)
 {
  if(i > 0)
  {
   si->_endLat = si->pointList().at(i).lat();
   si->_endLon = si->pointList().at(i).lon();
   Bearing b(si->pointList().at(i-1).lat(), si->pointList().at(i-1).lon(), si->pointList().at(i).lat(), si->pointList().at(i).lon());
   si->_length += b.Distance();
  }
 }
 if(si->pointList().count() <1)
  return false;
 if(!si->startDate().isValid() || !si->endDate().isValid())
     qDebug() <<"invalid dates";
 if(si->doubleDate() > si->startDate())
     qDebug() << tr("segment %1 has doubledate > start date").arg(si->segmentId());
 Q_ASSERT(si->_startLat != 0);
 Q_ASSERT(si->_startLon != 0);
 if(si->pointList().count() > 1)
 {
  Q_ASSERT(si->_endLat != 0);
  Q_ASSERT(si->_endLon != 0);
 }
 commandText = "Update Segments set startLat= " + QString("%1").arg(si->_startLat,0,'f',8)
   + ", startLon= " + QString("%1").arg(si->_startLon,0,'f',8)
   + ", endLat= " + QString("%1").arg(si->_endLat,0,'f',8)
   + ", endLon= " + QString("%1").arg(si->_endLon,0,'f',8)
   + ", length= " + QString("%1").arg(si->_length)
   + ", points= " + QString("%1").arg(si->pointList().count())
   + ", type= " + QString("%1").arg((int)si->_routeType)
   + ", StreetId = " + QString("%1").arg(si->_streetId)
   + ", direction = '" + si->_direction + "', "
   + "pointArray='" + si->pointsString() + "', "
   + "description='" + si->_description + "',"
   + "street='" + si->_streetName + "',"
   + "NewerName='" + si->_newerStreetName + "',"
   + "location='" + si->_location + "',"
   + "tracks="+ QString::number(si->_tracks) + ","
   + "startDate='" + si->_dateBegin.toString("yyyy/MM/dd") + "', "
   + "DoubleDate='" + si->_dateDoubled.toString("yyyy/MM/dd") + "', "
   + "endDate='" + si->_dateEnd.toString("yyyy/MM/dd") + "', "
   + "formatOk=" + QString::number(si->_formatOK)  +", "
   + "lastUpdate=CURRENT_TIMESTAMP "
   + "where SegmentId = " + QString("%1").arg(si->segmentId());
 qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
  return ret;
 }
 ret = true;
 if(bNotify)
    emit segmentChanged(*si, CHANGETYPE::MODIFYSEG);

 return ret;
}

bool SQL::updateSegment(SegmentData* sd)
{
 bool ret = false;
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 bool bQuery;
 sd->_length = 0;
 int rows = 0;
 // SegmentInfo si = getSegmentInfo(sd->segmentId());
 // if(sd->startDate()< si.startDate())
 // {
 //     if(si.doubleDate() == si.startDate())
 //         si.setDoubleDate(sd->startDate());
 //     si.setStartDate(sd->startDate());
 // }
 // if(sd->segmentStartDate() < si.startDate())
 //     sd->setSegmentStartDate(si.startDate());

 // if(sd->endDate() > si.endDate())
 //     si.setEndDate(sd->endDate());
 // if(sd->endDate() > si.endDate())
 //  sd->setSegmentEndDate(sd->endDate());
 // if(sd->doubleDate() < si.doubleDate())
 //     si.setDoubleDate(sd->doubleDate());
 if(sd->tracks() == 2 && sd->doubleDate() > sd->segmentStartDate())
     qWarning() << tr("segment %1 doubleDate not = startDate").arg(sd->segmentId());
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
   + ", type= " + QString("%1").arg(sd->_routeType)
   + ", direction = '" + sd->_direction + "', "
   + "pointArray='" + sd->pointsString() + "', "
   + "description='" + sd->_description + "',"
   + "street='" + sd->_streetName + "',"
   + "streetId= " + QString::number(sd->_streetId) + ","
   + "NewerName='" + sd->_newerName + "',"
   + "location='" + sd->_location + "',"
   + "tracks="+ QString::number(sd->_tracks) + ","
   + "startDate='" + sd->segmentStartDate().toString("yyyy/MM/dd") + "', "
   + "endDate='" + sd->segmentEndDate().toString("yyyy/MM/dd") + "', "
   + "DoubleDate='" + sd->_dateDoubled.toString("yyyy/MM/dd") + "', "
   + "lastUpdate=CURRENT_TIMESTAMP "
   + "where SegmentId = " + QString("%1").arg(sd->segmentId());
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
  return ret;
 }
 ret = true;
 emit segmentChanged(*sd, CHANGETYPE::MODIFYSEG);

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
 SegmentData sd;
 sd.setSegmentId(SegmentId);

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 beginTransaction("UpdateSegment");

 QString commandText = "Select startLat, startLon, endLat, endLon, length, pointArray, oneWay,"
                       " tracks, street from Segments"
                       " where SegmentId = " + QString("%1").arg(SegmentId);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 if (!query.isActive())
 {
  rollbackTransaction("UpdateSegment");
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
  sd.setPoints(pointArray);
  sd._oneWay = query.value(5).toString();
  sd._tracks = query.value(6).toInt();
  sd.checkTracks();
  sd._streetName = query.value(7).toString();
 }

 // get the last ending point and sum the lengths
 Bearing b = Bearing(sd.startLat(), sd.startLon(), sd.endLat(), sd.endLon());
 if (oneWay == "N")
     sd._direction = b.strDirection() + "-" + b.strReverseDirection();
 else
     sd._direction = b.strDirection();
 if(points < 2)
 {
  rollbackTransaction("UpdateSegment");
  return false;
 }

 Q_ASSERT(sd.startLat() != 0);
 Q_ASSERT(sd.startLon() != 0);
 if(sd.pointList().count() >1)
 {
  Q_ASSERT(sd.endLat() != 0);
  Q_ASSERT(sd.endLon() != 0);
 }
 commandText = "Update Segments set startLat= " + QString("%1").arg(sd.startLat(),0,'f',8) + ", startLon= " +
     QString("%1").arg(sd.startLon(),0,'f',8) + ", endLat= " + QString("%1").arg(sd.endLat(),0,'f',8)
     + ", endLon= " + QString("%1").arg(sd.endLon(),0,'f',8) + ", length= " + QString("%1").arg(sd.length())
     + ", points= " + QString("%1").arg(sd._points) + ", direction = '" + sd.direction()
     + "', tracks="+ QString::number(sd.tracks())+ ", pointArray='" + sd.pointsString() + "', "
     "lastUpdate=CURRENT_TIMESTAMP " +
     "where SegmentId = " + QString("%1").arg(SegmentId);
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 rows = query.numRowsAffected();
 if (rows == 0)
 {
   commitTransaction("UpdateSegment");
   return ret;
 }
 ret = true;
 commitTransaction("UpdateSegment");
 emit segmentChanged(SegmentInfo(sd), CHANGETYPE::MODIFYSEG);

 return ret;
}

StationInfo SQL::getStationInfo(qint32 stationKey)
{
 StationInfo sti = StationInfo();

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT stationKey, a.name, latitude, longitude, "
                       " startDate, endDate, routes, markerType, segmentId,"
                       " infoKey, segments, routeType"
                       " from Stations a "
                       " where stationKey = " + QString("%1").arg(stationKey);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
  sti.startDate = query.value(4).toDate();
  sti.endDate = query.value(5).toDate();
  sti.routes = query.value(6).toString().split(",");
  sti.markerType = query.value(7).toString();
  sti.segmentId = query.value(8).toInt();
  sti.infoKey = query.value(9).toInt();
  sti.segments = query.value(10).toString().split(",");
  sti.routeType = (RouteType)query.value(11).toInt();
 }
 return sti;
}

QList<StationInfo> SQL::getStationsOnSegment(qint32 segmentId)
{
 StationInfo sti = StationInfo();
 QList<StationInfo> sList;
 QString sTxt = QString::number(segmentId);
 SegmentInfo si = getSegmentInfo(segmentId);
 if(si.segmentId()== -1)
  return sList;
 for(LatLng latLng : si.pointList())
 {
  QList<StationInfo> sList1 = getStationAtPoint(latLng);
  for(StationInfo sti0 : sList1)
  {
   if(!sti0.segments.contains(sTxt))
   {
    sti0.segments.append(sTxt);
    updateStation(sti0);
   }
   sList.append(sti0);
  }
 }
 return sList;
}

//TODO this query may return multiple rows!
StationInfo SQL::getStationInfo(QString name)
{
 StationInfo sti =  StationInfo();

 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "SELECT stationKey, a.name, latitude, longitude,"
                       " startDate, endDate, segmentId, MarkerType, routes,"
                       " infoKey, segments, routeType "
                       "from Stations a where a.name = '" + name + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
  sti.startDate = query.value(4).toDate();
  sti.endDate = query.value(5).toDate();
  sti.segmentId = query.value(6).toInt();
  sti.markerType = query.value(7).toString();
  sti.routes = query.value(8).toString().split(",");
  sti.infoKey = query.value(9).toInt();
  sti.segments = query.value(10).toString().split(",");
  sti.routeType = (RouteType)query.value(11).toInt();
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
        beginTransaction("updateStation");

        QString commandText = "update Stations set infoKey = " + QString("%1").arg(infoKey)
                + ",lastUpdate=CURRENT_TIMESTAMP " +
            "where stationKey = " + QString("%1").arg(stationKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        if(rows > 0)
        {
            commitTransaction("updateStation");
            ret = true;
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return ret;
}


bool SQL::updateStation(StationInfo sti)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        beginTransaction("updateStation");

        QString commandText = "update Stations set latitude = " + QString("%1").arg(sti.latitude,0,'f',8)
                              + ", longitude = " + QString("%1").arg(sti.longitude,0,'f',8)
                              + ", startDate ='" + sti.startDate.toString("yyyy/MM/dd") + "'"
                              + ", endDate ='" + sti.endDate.toString("yyyy/MM/dd") + "'"
                              + ", routes = '" + sti.routes.join(",") + "'"
                              + ", name = '" + sti.stationName + "'"
                              + ", markerType ='" + sti.markerType + "'"
                              + ", segmentId = " + QString::number(sti.segmentId)
                              + ", infoKey = " + QString::number(sti.infoKey)
                              + ", segments = '" + sti.segments.join(",")+ "'"
                              + ", routeType = " + QString::number(sti.routeType)
                              + ",lastUpdate=CURRENT_TIMESTAMP "
                              " where stationKey = " + QString("%1").arg(sti.stationKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        if (rows > 0)
        {
            commitTransaction("updateStation");
            ret = true;
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return ret;
}



QList<StationInfo> SQL::getStationAtPoint(LatLng pt)
{
    StationInfo sti =  StationInfo();
    QList<StationInfo> list;
    try
    {

        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = QString("SELECT stationKey, routes, a.name, latitude, longitude, "
                          "  a.startDate, a.endDate, segmentId, infoKey, markerType,"
                          " segments, routeType"
                          " from Stations a"
                          " where (latitude = %1 and longitude = %2)"
                          " or distance(latitude, longitude,%1,%2) < .020")
                          .arg(QString("%1").arg(pt.lat(),0,'f',8)).arg(QString("%1").arg(pt.lon(),0,'f',8));
        else
            commandText = QString("SELECT stationKey, routes, a.name, latitude, longitude, "
                          "a.segmentId, infoKey,  a.startDate, a.endDate,  "
                          "segmentid, infoKey,markerType, segments, routeType"
                          "from Stations a "
                          " where (latitude = %1 and longitude = %2)"
                          " or distance(latitude, longitude,%1,%2) < .020")
                          .arg(QString("%1").arg(pt.lat(),0,'f',8)).arg(QString("%1").arg(pt.lon(),0,'f',8));
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
         return list;
        }
        while (query.next())
        {
         sti =  StationInfo();
            sti.stationKey = query.value(0).toInt();
            sti.routes = query.value(1).toString().split(",");
            sti.stationName = query.value(2).toString();
            sti.latitude = pt.lat(); //query.value(3).toDouble();
            sti.longitude = pt.lon(); //query.value(4).toDouble();
            sti.startDate = query.value(5).toDate();
            sti.endDate = query.value(6).toDate();
            sti.segmentId = query.value(7).toInt();
            sti.infoKey = query.value(8).toInt();
            sti.markerType = query.value(9).toString();
            sti.segments = query.value(10).toString().split(",");
            sti.routeType = (RouteType)query.value(11).toInt();
            list.append(sti);
        }
    }
    catch (Exception e)
    {
        //myExceptionHandler(e);
    }
    return list;
}

QList<StationInfo> SQL::getStationsLikeName(QString name)
{
 QList<StationInfo> list;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "SELECT stationKey, a.name, latitude, longitude,"
                       " startDate, endDate, segmentId, Infokey, markerType,"
                       " routes, segments, routeType"
                       " from Stations a where lower(name) like '%" + name.toLower() + "%' COLLATE NOCASE";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
     sti.startDate = query.value(4).toDate();
     sti.endDate = query.value(5).toDate();
     sti.segmentId = query.value(6).toInt();
     sti.infoKey = query.value(7).toInt();
     sti.markerType = query.value(8).toString();
     sti.routes = query.value(9).toString().split(",");
     sti.segments = query.value(10).toString().split(",");
     sti.routeType = (RouteType)query.value(11).toInt();
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
        QString commandText = "select endLat, endLon, points from Segments"
                              " where segmentId = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
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

QString SQL::getSegmentOneWay(qint32 SegmentId)
{
    QString description = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select OneWay from Segments"
                              " where SegmentId = " + QString("%1").arg(SegmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

bool SQL::doesSegmentExist(QString descr, QString oneWay, QString location)
{
    bool ret = false;
    int count = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select count(*) from Segments where description = '" +
            descr + "' and oneWay= '" + oneWay + "' and Location= '" + location + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
bool SQL::updateRecord(SegmentInfo sd)
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

        QString commandText = "update Segments set street = '" + sd._streetName + "', "
                              "length= " + QString("%1").arg(sd._length) +
            ", tracks="+ QString::number(sd._tracks) +
            ", startLat=" + QString::number(sd._startLat, 'g', 8) +
            ", startLon=" + QString::number(sd._startLon,'g',8)+
            ", endLat=" + QString::number(sd._endLat, 'g', 8) +
            ", endLon=" + QString::number(sd._endLon,'g',8)+
            ", type=" + QString::number((int)sd._routeType) + " " +
            ", lastUpdate=CURRENT_TIMESTAMP "
            "where SegmentId = " + QString("%1").arg(sd.segmentId());
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

void SQL::setDefaultCompanyMnemonic(CompanyData* cd)
{
    if(cd->mnemonic.isEmpty())
    {
      if(!cd->routePrefix.isEmpty())
          cd->mnemonic = cd->routePrefix;
      else if(cd->name.contains("(") and cd->name.contains(")"))
      {
       int first = cd->name.indexOf("(");
       int last = cd->name.indexOf(")");
       int size = last - first - 1;
       cd->mnemonic = cd->name.mid(first+1,size);
      }
      else
      {
         QStringList sl = cd->name.split(" ");
         foreach (QString token, sl) {
            if(token.isEmpty())
                continue;
             if( token.at(0) == '(')
                 break;
             cd->mnemonic.append(token.at(0));
         }
      }
    }
    return;
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
 if(config->currConnection->servertype() == "MySql")
     commandText = "select `key`, description, routePrefix, startDate, endDate,"
                   " firstRoute, lastRoute, mnemonic, info, url, selected, lastUpdate from Companies";
 else if(config->currConnection->servertype() == "MsSql")

     commandText = "select [key], description, routePrefix, startDate, endDate,"
                   " firstRoute, lastRoute, mnemonic, info, url, selected,lastUpdate from Companies";
 else //Sqlite and PostgreSQL
 commandText = "select key, description, routePrefix, startDate, endDate,"
               " firstRoute, lastRoute, mnemonic, info, url, selected, lastUpdate from Companies";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
     cd = new CompanyData();
     cd->companyKey = query.value(0).toInt();
     cd->name = query.value(1).toString();
     cd->routePrefix = query.value(2).toString();
     if(query.value(3).isNull())
         cd->startDate = QDate();
     else
         cd->startDate = query.value(3).toDate();
     if (query.value(4).isNull())
         cd->endDate = QDate();
     else
         cd->endDate = query.value(4).toDate();
     cd->firstRoute = query.value(5).toInt();
     cd->lastRoute = query.value(6).toInt();
     cd->mnemonic = query.value(7).toString();
     cd->info = query.value(8).toString();
     cd->url = QUrl(query.value(9).toString());
     cd->bSelected = query.value(10).toBool();
     cd->lastUpdated = query.value(11).toDateTime();
     setDefaultCompanyMnemonic(cd);
     myArray.append(cd);
 }
 std::sort(myArray.begin(), myArray.end(), [](const CompanyData* a, const CompanyData* b) -> bool { return a->name < b->name; });

 return myArray;
}

bool SQL::updateCompany(CompanyData* cd)
{
    bool ret = false;
    QString commandText;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query = QSqlQuery(db);
    bool bQuery;
    int rows = 0;
    if(config->currConnection->servertype() == "MySql")
        commandText = "update companies set "
            "description= '" + cd->name + "',"
            "mnemonic= '" + cd->mnemonic + "',"
            "routePrefix= '" + cd->routePrefix + "',"
            "info= '" + cd->info + "',"
            "url= '" + cd->url.toDisplayString() + "',"
            "startDate = '" + cd->startDate.toString("yyyy/MM/dd")+ "',"
            "endDate = '" + cd->endDate.toString("yyyy/MM/dd")+ "',"
            "firstroute = " +QString::number(cd->firstRoute) + ","
            "lastroute = " + QString::number(cd->lastRoute) + ", "
            "selected = " + QString::number(cd->bSelected) + " "
            "lastUpdate=CURRENT_TIMESTAMP "
            "where `key` = " +  QString::number(cd->companyKey);
    else if(config->currConnection->servertype() == "MsSql")
        commandText = "update companies set "
            "description= '" + cd->name + "',"
            "mnemonic= '" + cd->mnemonic + "',"
            "routePrefix= '" + cd->routePrefix + "',"
            "info= '" + cd->info + "',"
            "url= '" + cd->url.toDisplayString() + "',"
            "startDate = '" + cd->startDate.toString("yyyy/MM/dd")+ "',"
            "endDate = '" + cd->endDate.toString("yyyy/MM/dd")+ "',"
            "firstroute = " +QString::number(cd->firstRoute) + ","
            "lastroute = " + QString::number(cd->lastRoute) + ", "
            "selected = " + QString::number(cd->bSelected) + " "
            "lastUpdate=CURRENT_TIMESTAMP "
            "where [key] = " +  QString::number(cd->companyKey);
    else // Sqlite and PostgreSQL
        commandText = "update companies set "
            "description= '" + cd->name + "',"
            "mnemonic= '" + cd->mnemonic + "',"
            "routePrefix= '" + cd->routePrefix + "',"
            "info= '" + cd->info + "',"
            "url= '" + cd->url.toDisplayString() + "',"
            "startDate = '" + cd->startDate.toString("yyyy/MM/dd")+ "',"
            "endDate = '" + cd->endDate.toString("yyyy/MM/dd")+ "',"
            "firstroute = " +QString::number(cd->firstRoute) + ","
            "lastroute = " + QString::number(cd->lastRoute) + ", "
            "selected = " + QString::number(cd->bSelected) + ", "
            "lastUpdate=CURRENT_TIMESTAMP  "
            "where key = " +  QString::number(cd->companyKey);
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
     return ret;
    }
    ret = true;

    return ret;
}

/// <summary>
/// Get a list of companies in date range
/// </summary>
/// <returns></returns>
QList<CompanyData*> SQL::getCompaniesInDateRange(QDate startDate, QDate endDate)
{
 QList<CompanyData*> myArray;
 CompanyData* cd;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText;
 if(config->currConnection->servertype() == "MySql")
     commandText = QString("select `key`, description, routePrefix, startDate, endDate,"
                   " firstRoute, lastRoute, mnemonic, url, selected from Companies"
                   " where startDate between '%1' and '%2' or endDate between '%1' and '%2'")
       .arg(startDate.toString("yyyy/MM/dd")).arg(endDate.toString("yyyy/MM/dd"));
 else if(config->currConnection->servertype() == "MsSql")

     commandText = QString("select [key], description, routePrefix, startDate, endDate,"
                   " firstRoute, lastRoute, mnemonic, url, selected from Companies"
                   " where startDate between '%1' and '%2' or endDate between '%1' and '%2'")
                   .arg(startDate.toString("yyyy/MM/dd"))
                   .arg(endDate.toString("yyyy/MM/dd"));
 else
    commandText = QString("select key, description, routePrefix, startDate, endDate,"
               " firstRoute, lastRoute, mnemonic, url, selected from Companies"
               " where startDate between '%1' and '%2' or endDate between '%1' and '%2'")
   .arg(startDate.toString("yyyy/MM/dd")).arg(endDate.toString("yyyy/MM/dd"));
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
     cd = new CompanyData();
     cd->companyKey = query.value(0).toInt();
     cd->name = query.value(1).toString();
     cd->routePrefix = query.value(2).toString();
     if(query.value(3).isNull())
         cd->startDate = QDate();
     else
         cd->startDate = query.value(3).toDate();
     if (query.value(4).isNull())
         cd->endDate = QDate();
     else
         cd->endDate = query.value(4).toDate();
     cd->firstRoute = query.value(5).toInt();
     cd->lastRoute = query.value(6).toInt();
     cd->mnemonic = query.value(7).toString();
     cd->url = QUrl(query.value(8).toString());
     cd->bSelected = query.value(9).toBool();
     setDefaultCompanyMnemonic(cd);
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
        if(config->currConnection->servertype() == "MySql")
            commandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute,"
                          " routePrefix, mnemonic, info, url, selected from Companies"
                          " where `key` = " +QString("%1").arg(companyKey);
        else if(config->currConnection->servertype() == "MsSql")
            commandText = "select [key], description, startDate, endDate, firstRoute, lastRoute,"
                          " routePrefix,memonic, info, url, selected from companies"
                          " where [key] = " + QString("%1").arg(companyKey);
        else // Sqlite and PostgreSQL
        commandText = "select key, description, startDate, endDate, firstRoute, lastRoute,"
                      " routePrefix, mnemonic, info, url, selected from Companies"
                      " where key = " +QString("%1").arg(companyKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            cd->mnemonic = query.value(7).toString();
            cd->info = query.value(8).toString();
            cd->url = QUrl(query.value(9).toString());
            cd->bSelected = query.value(10).toBool();
            setDefaultCompanyMnemonic(cd);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }
    return cd;
}

bool SQL::doesAltRouteExist(int route, QString alphaRoute)
{
 bool ret = false;
 int count = 0;
 try
 {
     if(!dbOpen())
         throw Exception(tr("database not open: %1").arg(__LINE__));
     QSqlDatabase db = QSqlDatabase::database();

     QString commandText = "select count(*) from AltRoute where route = " + QString::number(route)
                           + " and routeAlpha = '" + alphaRoute + "'";
     QSqlQuery query = QSqlQuery(db);
     bool bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
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

/// <summary>
/// Add a new route number.
/// </summary>
/// <param name="routeAlpha"></param>
/// <returns></returns>
qint32 SQL::addAltRoute(QString routeAlpha, QString routePrefix)
{
 int route=-1, rows=0, count=0;
 QString commandText;
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery  query = QSqlQuery(db);
 bool bQuery;
 Q_ASSERT(!routeAlpha.isEmpty() && !routeAlpha.startsWith(" "));

 QString newAlpha;
 bool bAlphaRoute;
 int nbr = getNumericRoute(routeAlpha, &newAlpha, &bAlphaRoute,routePrefix);
 if(nbr > 0)
     return nbr;
 bool isNumeric = true;
 nbr = routeAlpha.toInt(&isNumeric);
 if(isNumeric && routePrefix.isEmpty() && nbr < 1000)
 {
    commandText = "insert into altRoute (route, routeAlpha, routePrefix, baseRoute) values("
                  + QString::number(nbr) +", '"
                  + routeAlpha + "',"
                  + "'', "
                  + QString::number(nbr) + ")";
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    return nbr;
 }

 commandText = "select max(route) from AltRoute group by route";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 commandText = "select route from AltRoute"
               " where routeAlpha = '" + QString("%1").arg(routeAlpha) + "' " \
   "and routePrefix = '" + routePrefix.trimmed() + "'";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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

 commandText = "insert into AltRoute (route, routeAlpha, baseRoute, routePrefix) values ( "
         + QString("%1").arg(route) + ", '" + routeAlpha + "','" + QString("%1").arg(baseRoute) + "','" + routePrefix.trimmed() +"')";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 rows = query.numRowsAffected();

 // check if inserted OK
 commandText = QString("select route from AltRoute where routeAlpha = '%1' and routePrefix = '%2'").arg(routeAlpha).arg(routePrefix);
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 int newKey = 0;
 while(query.next())
 {
  newKey = query.value(0).toInt();
 }
 return route;
}

bool SQL::addAltRoute(int routeNum, QString routeAlpha, QString routePrefix){
 QSqlDatabase db = QSqlDatabase::database();
 bool isNumeric;
 int baseRoute =routeAlpha.toInt(&isNumeric);
 if(!isNumeric)
 {
  for(int i=0; i < routeAlpha.length(); i++)
  {
   if(routeAlpha.at(i).isDigit())
   {
    baseRoute = baseRoute*10 + QString(routeAlpha.at(i)).toInt();
    break;
   }
  }
 }
 QString commandText = "insert into AltRoute (route, routeAlpha, routePrefix, baseRoute)"
                       " values (" +QString::number(routeNum)
                       + ", '" + routeAlpha + "','" + routePrefix.trimmed() + "','" + QString::number(baseRoute) +"')";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
}


bool SQL::updateAltRoute(int route, QString routeAlpha)
{
 QSqlDatabase db = QSqlDatabase::database();
 Q_ASSERT(!routeAlpha.isEmpty() && !routeAlpha.startsWith(" "));

 QString commandText = "update AltRoute "
                       "set AltRoute=" + routeAlpha.trimmed()+
                       ",lastUpdate=CURRENT_TIMESTAMP"
                       " where route = " +QString::number(route);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }

 return true;
}

bool SQL::deleteRouteSegment(SegmentData sd, bool bNotify)
{
    bool ret = false;
    int rows = 0;
    //name = name.trimmed();
    QString commandText;
    QString segStartDate = "", segEndDate = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QString activeTransaction = currentTransaction;
        if(!isTransactionActive())
        {
            beginTransaction("deleteRoute");
            activeTransaction = currentTransaction;
        }
        if(config->currConnection->servertype() == "MsSql")
         commandText = "delete from Routes where route = " + QString("%1").arg(sd._route)
           + " and routeId = " + QString::number(sd._routeId)
           + " and LineKey = " + QString("%1").arg(sd._segmentId)
           + " and startDate = '" + sd.startDate().toString("yyyy/MM/dd") + "'"
           " and endDate = '" + sd.endDate().toString("yyyy/MM/dd") + "'";
        else
         commandText = "delete from Routes where route = " + QString("%1").arg(sd._route)
                 + " and routeId = " + QString::number(sd._routeId)
                 + " and LineKey = " + QString("%1").arg(sd._segmentId)
                 + " and startDate = '" + sd.startDate().toString("yyyy/MM/dd")  + "'"
                 + " and endDate = '" + sd.endDate().toString("yyyy/MM/dd") + "'";
        QSqlQuery query = QSqlQuery(db);
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";

        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        if(bNotify)
            emit routeChange(NotifyRouteChange(DELETESEG, &sd));

        // Scan the remaining routes to find a new start and end date
        commandText = "select min(startDate), max(endDate) from Routes where lineKey = " + QString("%1").arg(sd._segmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

        commandText = "update Segments set startDate = '" + segStartDate  + "', endDate = '"
                      + segEndDate + "' "
                      + ",lastUpdate=CURRENT_TIMESTAMP "
                        "where SegmentId =" + QString("%1").arg(sd._segmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();

        if(activeTransaction == "deleteRoute")
            commitTransaction("deleteRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

bool SQL::deleteRouteSegment(qint32 route, int routeId, qint32 segmentId, QString startDate, QString endDate, QString routeStartDate, QString routeEndDate)
{
 if(startDate == routeStartDate && endDate == routeEndDate)
  return deleteRouteSegment(route,routeId,segmentId, startDate, endDate);

}

/// <summary>
/// Delete a segment from a route
/// </summary>
/// <param name="route"></param>
/// <param name="SegmentId"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <returns></returns>
bool SQL::deleteRouteSegment(qint32 route, int routeId, qint32 SegmentId,
                             QString startDate, QString endDate)
{
    bool ret = false;
    int rows = 0;
    //name = name.trimmed();
    QString commandText;
    QString segStartDate = "", segEndDate = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QString activeTransaction = currentTransaction;
        if(!isTransactionActive())
        {
            beginTransaction("deleteRoute");
            activeTransaction = currentTransaction;
        }
        if(config->currConnection->servertype() == "MsSql")
         commandText = "delete from Routes where route = " + QString("%1").arg(route)
           + " and routeId = " + QString::number(routeId)
           + " and LineKey = " + QString("%1").arg(SegmentId)
           + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        else
         commandText = "delete from Routes where route = " + QString("%1").arg(route)
                 + " and routeId = " + QString::number(routeId)
                 + " and LineKey = " + QString("%1").arg(SegmentId) + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";

        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        RouteInfo ri = getRouteName(routeId);
        SegmentData sd(route, ri.routeName, SegmentId, QDate::fromString(startDate,"yyyy/MM/dd"),
                                               QDate::fromString(endDate,"yyyy/MM/dd"));
        emit routeChange(NotifyRouteChange(DELETESEG, &sd));

        // Scan the remaining routes to find a new start and end date
        commandText = "select min(startDate), max(endDate) from Routes where lineKey = " + QString("%1").arg(SegmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

        commandText = "update Segments set startDate = '" + segStartDate  + "', endDate = '"
                      + segEndDate + "' "
                      + ",lastUpdate=CURRENT_TIMESTAMP where SegmentId =" + QString("%1").arg(SegmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();

        if(activeTransaction == "deleteRoute")
            commitTransaction("deleteRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

// NOTE: except for pointer ref to SegmentData this function is the same as insertRouteSegment()
// except that SegmentInfo fields are updated in SegmentData
bool SQL::addSegmentToRoute(SegmentData* sd, bool notify)
{
    if(sd->startDate().isNull() || sd->endDate().isNull() || !sd->startDate().isValid()
            || !sd->endDate().isValid() || sd->endDate() < sd->startDate())
     throw IllegalArgumentException(tr("invalid dates"));
    bool ret = false;
    int rows = 0;
    if (sd->route() < 1)
    {
        qDebug()<<"Invalid route number";
        return ret;
    }
    if (sd->routeName() == "" || sd->routeName().length() > 140)
    {
        qDebug()<<"invalid route name";
        return ret;
    }

    if (sd->endDate() < sd->startDate())
    {
        qDebug()<<"end date ("+ sd->endDate().toString("yyyy/MM/dd") +") before start date("+ sd->startDate().toString("yyyy/MM/dd")+")!";
        throw (new ApplicationException("Invalid end date" + sd->endDate().toString("yyyy/MM/dd")));
    }
    if (sd->segmentId() <= 0)
        throw (new ApplicationException("invalid segmentid:" + QString::number(sd->segmentId())));
    if (sd->companyKey() < 1)
    {
        qDebug()<<"invalid company key: " + QString("%1").arg(sd->companyKey());
        return ret;
    }
    CompanyData* cd = getCompany(sd->companyKey());
    if(sd->route() != 9998 && sd->route() != 9999)
        updateCompany(sd->companyKey(), sd->route());


    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        if(!isTransactionActive())
            beginTransaction("addSegmentToRoute");
        bool bAllreadyPresent = false;
        RouteInfo ri = RouteInfo(*sd);
        int routeId = addRouteName(ri, &bAllreadyPresent);
        if(routeId < 0)
        {
            if(currentTransaction == "addSegmentToRoute")
                rollbackTransaction("addSegmentToRoute");
            return false;
        }
        sd->setRouteId(routeId);

        QString commandText = "INSERT INTO Routes(Route, routeId, StartDate, EndDate, LineKey, "
                "companyKey, tractionType, direction, next, prev, normalEnter, normalleave,"
                " reverseEnter, reverseLeave, sequence, ReverseSeq, oneWay, trackUsage) "
                "VALUES(" + QString("%1").arg(sd->route()) + ", "
                + QString::number(sd->routeId()) + ",'"
                + sd->startDate().toString("yyyy/MM/dd") + "', '"
                + sd->endDate().toString("yyyy/MM/dd") + "',"
                + QString("%1").arg(sd->segmentId()) + ", "
                + QString("%1").arg(sd->companyKey())+","
                + QString("%1").arg(sd->tractionType())+",'"
                + sd->direction() +"', "
                + QString("%1").arg(sd->next()) +", "
                + QString("%1").arg(sd->prev()) +", "
                + QString("%1").arg(sd->normalEnter()) + ","
                + QString("%1").arg(sd->normalLeave()) + ","
                + QString("%1").arg(sd->reverseEnter()) + ", "
                + QString("%1").arg(sd->reverseLeave()) + ", "
                + QString("%1").arg(sd->sequence()) + ", "
                + QString("%1").arg(sd->returnSeq()) + ", '"
                + QString("%1").arg(sd->oneWay())  + "', '"
                + sd->trackUsage() + "')";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            qDebug() << query.lastError().text() << commandText;
            if(currentTransaction == "addSegmentToRoute")
                rollbackTransaction("addSegmentToRoute");
            QString text;
            SQLERROR1(std::move(query), QMessageBox::Ignore | QMessageBox::Ok, text);
            switch(errReturn)
            {
            case QMessageBox::Abort:
                EXIT_FAILURE;
            case QMessageBox::Ignore:
                return true;
            case QMessageBox::Ok:
                return false;
            default:
                break;
            }
         //db.close();
         return ret;
        }
        rows = query.numRowsAffected();

        //updateSegmentDates(sd->segmentId());
        updateSegment(sd);

        if(currentTransaction == "addSegmentToRoute")
            commitTransaction("addSegmentToRoute");
        if(notify)
            emit routeChange(NotifyRouteChange(SQL::ADDSEG, sd));
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

// if a segment is split, makesure that any route using the original
// segment get the new segment added.
bool SQL::addSegmentToRoutes(int _newSegmentId, int _segmentId)
{
 if(!dbOpen())
     throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 SegmentInfo si = getSegmentInfo(_segmentId);
 SegmentInfo siNew = getSegmentInfo(_segmentId);
 QList<SegmentData> routes = getRouteDatasForDate(_segmentId, si.startDate().toString("yyyy/MM/dd"));
 for(SegmentData sd : routes)
 {
  SegmentData sdNew = SegmentData(siNew);
  sdNew.setAlphaRoute(sd.alphaRoute());
  sdNew.setRoute(sd.route());
  sdNew.setRouteName(sd.routeName());
  sdNew.setTractionType(sd.tractionType());
  sdNew.setRouteType(sd.routeType());
  sdNew.setCompanyKey(sd._companyKey);
  if(!doesRouteSegmentExist(sd))
  {
   if(!addSegmentToRoute(&sd))
   {
    return false;
   }
  }
 }
 return true;
}

bool SQL::updateTerminals(TerminalInfo ti)
{
 return  updateTerminals(ti.route, ti.name, ti.startDate, ti.endDate,
                         ti.startSegment, ti.startWhichEnd,
                         ti.endSegment, ti.endWhichEnd);
}

bool SQL::updateTerminals(qint32 route, QString name, QDate startDate, QDate endDate,
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
         commandText = "select count(*),startSegment, startWhichEnd, "
                       "endSegment, endWhichEnd from Terminals"
                       " where route = " + QString("%1").arg(route) + ""
                       " and endDate = '" + endDate.toString("yyyy/MM/dd")
                       + "' and LTRIM(RTRIM(name)) = '" + name.trimmed() + "' group by startSegment, startWhichEnd, endSegment, endWhichEnd";
        else
         commandText = "select count(*),startSegment, startWhichEnd, endSegment,"
                       " endWhichEnd from Terminals where route = " + QString("%1").arg(route)
                       + " and endDate = '" + endDate.toString("yyyy/MM/dd") + "' and TRIM(name) = '" + name.trimmed() + "' group by startSegment, startWhichEnd, endSegment, endWhichEnd";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            commandText = "insert into Terminals (route, name, startDate,"
                          " endDate, startSegment, startWhichEnd, endSegment, endWhichEnd)"
                          " values (" + QString("%1").arg(route) + ", '" + name.trimmed()
                          + "', '" + startDate.toString("yyyy/MM/dd") + "', '"
                          + endDate.toString("yyyy/MM/dd") + "', " + QString("%1").arg(startSegment) + ", '" + startWhichEnd + "', " + QString("%1").arg(endSegment) + ", '" + endWhichEnd + "')";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
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
            commandText = "update Terminals set startdate = '"
                          + startDate.toString("yyyy/MM/dd") + "', endDate = '"
                          + endDate.toString("yyyy/MM/dd") + "', startSegment="
                          + QString("%1").arg(cStartSegment) + ", startWhichEnd='"
                          + cStartWhichEnd + "', endSegment = "
                          + QString("%1").arg(cEndSegment) + ", endWhichEnd = '"
                          + cEndWhichEnd + "',lastUpdate=CURRENT_TIMESTAMP"
                          " where route = " + QString("%1").arg(route) + ""
                          " and name = '" + name + "' and endDate = '"
                          + endDate.toString("yyyy/MM/dd") + "'";
            qDebug()<<commandText;
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
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
qint32 SQL::getNumericRoute(QString routeAlpha, QString * newAlphaRoute, bool * bAlphaRoute, QString routePrefix)
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
    else
    {
     QString alphaRoute = getAlphaRoute(route, routePrefix);
     if(!alphaRoute.isEmpty())
     {
      *newAlphaRoute = alphaRoute;
      *bAlphaRoute = true;
      return route;
     }
    }

    if (route != -1 && routePrefix.trimmed().isEmpty())
        *(newAlphaRoute) = (route < 10 ? "0" : "") + QString("%1").arg(route);
    else
        *(newAlphaRoute) = routeAlpha;
    QSqlDatabase db = QSqlDatabase::database();
    QString commandText ;
    if(config->currConnection->servertype() != "MsSql")
     commandText = "select route from AltRoute a"
                   " where routeAlpha = '" + routeAlpha + "'"
                   " and routePrefix = '" + routePrefix + "'";
    else
     commandText = "select route from AltRoute a"
                   " where routeAlpha = '" + routeAlpha + "'"
                   " and routePrefix = '" + routePrefix + "'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
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


int SQL:: findNextRouteInRange(QString routeAlpha)
{
 QStringList tokens = routeAlpha.split(",");
 if(tokens.count() != 2)
  return -1;
 bool bOk;
 int lowRange = tokens.at(0).toUInt(&bOk);
 if(!bOk)
  return -1;
 int highRange = tokens.at(1).toUInt(&bOk);
 if(!bOk)
  return -1;
 if(highRange < lowRange)
 {
  QMessageBox::critical(nullptr, tr("Error"), tr("Range end must be greater than start in range %1 to %2")
                        .arg(lowRange).arg(highRange));
  return -1;
 }
 int next = nextRouteNumberInRange(lowRange, highRange) + 1;
 if(next >= highRange )
 {
  QMessageBox::critical(nullptr, tr("Error"), tr("No numbers available in range %1 to %2").arg(lowRange).arg(highRange));
  return -1;
 }
 return next;
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

        QString commandText = "Select distinct a.route, n.name, startDate, endDate, "
                              "routeAlpha, b.baseRoute, a.routeId"
                              " from Routes a "
                              "join AltRoute b on a.route = b.route "
                              "join RouteName n on n.routeId = a.routeId "
                              "where a.route = " + QString("%1").arg(route)
                              + " group by a.route, n.name, startDate, endDate, routeAlpha";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            RouteData rd =  RouteData ();
            rd._route = query.value(0).toInt();
            rd._name = query.value(1).toString();
            rd._dateBegin = query.value(2).toDate();
            rd._dateEnd = query.value(3).toDate();
            //rd.companyKey = (int)myReader.GetInt32(4);
            //rd.tractionType = (int)myReader.GetInt32(5);
            rd._alphaRoute = query.value(4).toString();
            rd._baseRoute = query.value(5).toInt();
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
        //beginTransaction("updateCompanies");
        if(config->currConnection->servertype() == "MySql")
            commandText = "select `key`, description, startDate, endDate, firstRoute, lastRoute from Companies where `key` = " + QString("%1").arg(companyKey);
        else if(config->currConnection->servertype() == "MsSql")
            commandText = "select [key], description, startDate, endDate, firstRoute, lastRoute from companies where [key] = " + QString("%1").arg(companyKey);
        else // sqlite and PostgreSQL
            commandText = "select key, description, startDate, endDate, "
                          "firstRoute, lastRoute "
                          "from Companies "
                          "where key = " + QString("%1").arg(companyKey);

        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

        if(config->currConnection->servertype() == "MySql")
            commandText = "Update Companies set  "
                          "firstRoute= " +QString("%1").arg( cd->firstRoute) +
                          ", lastRoute = " + QString("%1").arg(cd->lastRoute) +
                    ",lastUpdate=CURRENT_TIMESTAMP where `key` = " + QString("%1").arg(companyKey);
        else if(config->currConnection->servertype() == "MsSql")
            commandText = "Update companies set  firstRoute= " +QString("%1").arg( cd->firstRoute) + ", lastRoute = " + QString("%1").arg(cd->lastRoute) + ",lastUpdate=:lastUpdate where [key] = " + QString("%1").arg(companyKey);
        else // Sqlite and PostgreSQL
            commandText = "Update Companies set  "
                          "firstRoute= " +QString("%1").arg( cd->firstRoute) +
                          ", lastRoute = " + QString("%1").arg(cd->lastRoute) +
                    ",lastUpdate=CURRENT_TIMESTAMP where key = " + QString("%1").arg(companyKey);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
    //commitTransaction("updateCompanies");
    return ret;
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

    QString commandText = "select min(startDate), max(endDate) from Routes"
                          " where linekey = " + QString("%1").arg(segmentId);
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    if (!query.isActive())
    {
        //
        //
        //return false;
        //throw (new ApplicationException("addSegmentToRoute: segment not found\n" + commandText));
        segStartDate = "1890-1-1";
        segEndDate = "2050-12-31";

    }
    while (query.next())
    {
        if (query.value(0).isNull())
            segStartDate = "1890-01-01";
        else
            segStartDate = query.value(0).toDateTime().toString("yyyy/MM/dd");
        if (query.value(1).isNull())
            segEndDate = "2050-12-31";
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
    commandText = "update Segments set startDate = '" + segStartDate + "', "
                  "endDate ='" + segEndDate + "' " +
                  ",lastUpdate=CURRENT_TIMESTAMP "
                  "where SegmentId = " + QString("%1").arg(segmentId);
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
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
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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

        QString commandText = "Select distinct n.name "
                              "from Routes r"
                " join RouteName n on n.routeId = r.routeId"
                " where r.route = " + QString("%1").arg(route)
                + " order by n.name";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
Parameters SQL::getParameters(QSqlDatabase db)
{
     Parameters parms = Parameters();
    QString alphaRoutes;
    try
    {
    //if(!dbOpen())
     //throw Exception(tr("database not open: %1").arg(__LINE__));
     // db = QSqlDatabase::database();

        QString commandText = "select lat, lon, title, city, minDate, maxDate, "
                              "alphaRoutes, abbreviationsList "
                              "from Parameters";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            //throw SQLException(error.text() + " " + errCommand);
            return parms;
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
            parms.minDate = query.value(4).toDate();
            parms.maxDate = query.value(5).toDate();
            alphaRoutes = query.value(6).toString();
            if (alphaRoutes == "Y")
                parms.bAlphaRoutes = true;
            QString abbreviationsStr = query.value(7).toString();

            QStringList list =  abbreviationsStr.split(",");
            parms.abbreviationsList.clear();
            foreach(QString aPair, list)
            {
                if(aPair.isEmpty())
                    continue;
                QStringList values = aPair.split(":");
                if(values.count()>1)
                    parms.abbreviationsList.append(QPair<QString, QString>(values.at(0),values.at(1)));
            }        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return parms;
}

bool SQL::insertParameters(Parameters parms, QSqlDatabase db)
{
    try
    {
        if(!db.isOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        //QSqlDatabase db = QSqlDatabase::database();

        QString commandText = QString("insert into Parameters (lat, lon, title, city, minDate, maxDate, alphaRoutes,lastupdate)"
                              " values (%1, %2, '%3', '%4', '%5', '%6', '%7',CURRENT_TIMESTAMP)")
                .arg(parms.lat).arg(parms.lon).arg(parms.title,parms.city,parms.minDate.toString("yyyy/MM/dd"),
                    parms.maxDate.toString("yyyy/MM/dd"), parms.bAlphaRoutes?"Y":"N");
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return true;
}

bool SQL::updateParameters(Parameters parms, QSqlDatabase db)
{
    try
    {
        if(!db.isOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        //QSqlDatabase db = QSqlDatabase::database();

        QString commandText = QString("update Parameters set lat = %1, lon=%2, title='%3', city='%4', "
                              "minDate = '%5', maxDate= '%6', alphaRoutes = '%7',"
                              "abbreviationsList = '%8',lastUpdate=CURRENT_TIMESTAMP")
                .arg(parms.lat).arg(parms.lon).arg(parms.title,parms.city,parms.minDate.toString("yyyy/MM/dd"),
                    parms.maxDate.toString("yyyy/MM/dd"), parms.bAlphaRoutes?"Y":"N");
        QSqlQuery query = QSqlQuery(db);
        QString abbreviationsStr;
        for (QPair<QString,QString> pair: parms.abbreviationsList) {
            abbreviationsStr.append(pair.first+":"+pair.second +",");
        }
        abbreviationsStr.chop(1);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return true;
}

QList<SegmentData*> SQL::getRouteSegmentsBySegment(qint32 segmentId)
{
  QList<SegmentData*> myArray;
  QString where = "where SegmentId = " + QString("%1").arg(segmentId)
          + " order by route";
  myArray = segmentDataListFromView(where);
  return myArray;
}

// TODO: replace with segmentDataFromView
QList<SegmentData> SQL::getRouteSegmentsBySegment(int route, qint32 segmentId)
{
    QList<SegmentData> myArray;
//    try
//    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select r.route, n.name, r.startDate, r.endDate, r.lineKey, r.companyKey,"
              " tractionType, r.direction, normalEnter, normalLeave, reverseEnter, reverseLeave,"
              " a.routeAlpha, r.OneWay, s.description, next, prev, s.tracks, s.street, r.trackUsage,"
              " pointArray, s.newerName, r.routeId "
              " from Routes r"
              " join AltRoute a on a.route = r.route"
              " join Segments s on s.segmentId = r.lineKey"
              " join RouteName n on n.routeId =r.routeId"
              " where r.lineKey = " + QString("%1").arg(segmentId)
              + " and a.route = " + QString("%1").arg(route)
              + " order by a.routeAlpha, n.name, r.startDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            SegmentData sd =  SegmentData();
            sd._route = query.value(0).toInt();
            sd._routeName = query.value(1).toString();
            sd._dateBegin =query.value(2).toDate();
            sd._dateEnd = query.value(3).toDate();
            sd._segmentId = query.value(4).toInt();
            sd._companyKey = query.value(5).toInt();
            sd._tractionType = query.value(6).toInt();
            sd._direction = query.value(7).toString();
            sd._normalEnter = query.value(8).toInt();
            sd._normalLeave = query.value(9).toInt();
            sd._reverseEnter = query.value(10).toInt();
            sd._reverseLeave = query.value(11).toInt();
            sd._alphaRoute = query.value(12).toString();
            sd._oneWay = query.value(13).toString();
            sd._description = query.value(14).toString();
            sd._next = query.value(15).toInt();
            sd._prev = query.value(16).toInt();
            sd._tracks = query.value(17).toInt();
            sd._streetName = query.value(18).toString();
            sd._trackUsage = query.value(19).toString();
            sd.setPoints(query.value(20).toString());
            sd._newerName = query.value(21).toString();
            sd._routeId = query.value(22).toInt();

            if(sd._normalEnter > 2 || sd._normalEnter < 0)
             sd._normalEnter = 0;
            if(sd._normalLeave > 2 || sd._normalLeave < 0)
             sd._normalLeave = 0;
            if(sd._reverseEnter > 2 || sd._reverseEnter < 0)
             sd._reverseEnter = 0;
            if(sd._reverseLeave > 2 || sd._reverseLeave < 0)
             sd._reverseLeave = 0;

            myArray.append(sd);
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

QList<SegmentData> SQL::getRouteSegmentsForRouteNbr(QString route)
{
    QList<SegmentData> myArray;
//    try
//    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select a.route, n.name, a.startDate, a.endDate, lineKey,"
              " companyKey, tractionType, direction, next, prev, trackUsage,"
              " normalEnter, normalLeave, reverseEnter, reverseLeave, routeAlpha,"
                              " a.OneWay, a.routeId"
              " from Routes a"
              " join AltRoute b on a.route = b.route"
              " join RouteName n on n.routeId = a.RouteId"
              " where b.routeAlpha = '" + route + "'"
              + " order by companyKey, name, a.endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            SegmentData sd =  SegmentData();
            sd._route = query.value(0).toInt();
            sd._routeName = query.value(1).toString();
            sd._dateBegin =query.value(2).toDate();
            sd._dateEnd = query.value(3).toDate();
            sd._segmentId = query.value(4).toInt();
            sd._companyKey = query.value(5).toInt();
            sd._tractionType = query.value(6).toInt();
            sd._direction = query.value(7).toString();
            sd._next = query.value(8).toInt();
            sd._prev = query.value(9).toInt();
            sd._trackUsage = query.value(10).toString();
            sd._normalEnter = query.value(11).toInt();
            sd._normalLeave = query.value(12).toInt();
            sd._reverseEnter = query.value(13).toInt();
            sd._reverseLeave = query.value(14).toInt();
            sd._alphaRoute = query.value(15).toString();
            QString rdStartDate= sd._dateBegin.toString("yyyy/MM/dd");
            sd._oneWay = query.value(16).toString();
            sd._routeId = query.value(17).toInt();
            myArray.append(sd);
        }
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
        if(config->currConnection->servertype() == "MySql")
        {
            commandText = "select min(a.startdate), Max(a.enddate), n.Name, a.route, b.`Key`,"
                          " tractionType, routeAlpha, a.routeId"
                          " from Routes a"
                          " join Companies b on " + QString("%1").arg(route)
                          + " between firstRoute and lastRoute"
                          " join RouteName n on n.routeid = a.routeId"
                          " join AltRoute c on a.route = c.route "
                          "where a.Route = " + QString("%1").arg(route)
                          + " and n.Name = '" + name + "' "
                          "group by a.StartDate, a.endDate, n.Name, a.route, b.`Key`, tractionType, routeAlpha";
        }
        else if(config->currConnection->servertype() == "MsSql")
        {
            commandText = "select min(a.startdate), Max(a.enddate), n.Name, a.route, b.[Key],"
                          " tractionType, routeAlpha, a.routeId"
                          " from Routes a"
                          " join Companies b on " + QString("%1").arg(route) + " between firstRoute and lastRoute"
                          " join RouteName n on n.routeid = a.routeId"
                          " join AltRoute c on a.route = c.route "
                          "where a.Route = " + QString("%1").arg(route)
                          + " and n.Name = '" + name + "' "
                          "group by a.StartDate, a.endDate, n.Name, a.route, b.[Key], tractionType, routeAlpha";
        }
        else // Sqlite && PostgreSQL
            commandText = "select min(a.startdate), Max(a.enddate), n.Name, a.route, b.Key,"
                          " tractionType, routeAlpha, a.routeId"
                          " from Routes a"
                          " join Companies b on " + QString("%1").arg(route)
                          + " between firstRoute and lastRoute"
                          " join AltRoute c on a.route = c.route "
                          " join RouteName n on n.routeId = a.routeId "
                          "where a.Route = " + QString("%1").arg(route) +
                          " and n.Name = '" + name + "' "
                          "group by a.StartDate, a.endDate, n.Name, a.route, b.Key, tractionType, routeAlpha";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
            return myArray;
        }
        while (query.next())
        {
            rd = RouteData();
            rd._dateBegin =query.value(0).toDate();
            rd._dateEnd = query.value(1).toDate();
            rd._name = query.value(2).toString();
            rd._route = query.value(3).toInt();
            rd._companyKey = query.value(4).toInt();
            rd._tractionType = query.value(5).toInt();
            rd._alphaRoute = query.value(6).toString();
            rd._routeId = query.value(7).toInt();
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

        QString commandText = "Select Min(r.startDate)"
                              " from Routes r"
                              " join RouteName n on r.routeid = n.routeid"
                              " where r.route = " + QString("%1").arg(route)
                              + " and n.name = '" + name +"'  and r.startDate < '" + date
                              + "'  group  by r.startDate"
                              " order by r.startDate desc";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

bool SQL::recalculateSegmentDates(SegmentInfo* si)
{

    if(si->segmentId() <= 0)
        return false;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select min(startDate), max(endDate) from Routes "
                              "where lineKey = " + QString::number(si->segmentId());
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
            return false;
        }
        while (query.next())
        {
            si->setStartDate(query.value(0).toDate());
            si->setEndDate(query.value(1).toDate());
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }
    return true;
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

        QString commandText = "Select min(startDate) from Routes r"
                              " join RouteName on n.routeId = r.routeId"
                              " where route = " + QString("%1").arg(route)
                              + " and n.name = '" + name + "'"
                              " and r.startDate >= '" + date + "'"
                              " group  by r.startDate"
                              " order by r.startDate desc";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

bool SQL::doesRouteSegmentExist(SegmentData sd)
{
 return doesRouteSegmentExist(sd.route(), sd.routeName(), sd.segmentId(), sd.startDate(), sd.endDate());
}

bool SQL::doesRouteSegmentExist(qint32 route, QString name, qint32 segmentId, QDate startDate, QDate endDate)
{
    bool ret = false;
    int count = 0;
    if( startDate.isValid() && endDate.isValid()
        && (startDate > endDate))
    {
     throw IllegalArgumentException(tr("doesRouteSegmentExist: invalid dates: %1 %2 route: %3 %4")
                                    .arg(startDate.toString("yyyy/MM/dd"), endDate.toString("yyyy/MM/dd")
                                    .arg(route).arg(name)));
    }
    if(route <= 0 /*|| name.isEmpty()*/ || segmentId <= 0 )
    {
     return false;
    }
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "select count(*) from Routes r"
                              " join RouteName n on r.routeid = n.routeId"
                              " where route = " + QString("%1").arg(route)
                              + " and n.name = '" + name
                              + "' and lineKey= " + QString("%1").arg(segmentId)
                              + " and r.startDate = '"
                              + startDate.toString("yyyy/MM/dd")
                              + "' and r.endDate='"+ endDate.toString("yyyy/MM/dd") + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
        else
        {
         qDebug() << "no results: " << commandText;
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}

QList<SegmentInfo> SQL::getSegmentsInSameDirection(SegmentInfo siIn, bool reverse)
{
 QList<SegmentInfo> myArray;
  QString startLat = QString("%1").arg(siIn.startLat(),0,'f',8);
  QString startLon = QString("%1").arg(siIn.startLon(),0,'f',8);
  QString endLat = QString("%1").arg(siIn.endLat(),0,'f',8);
  QString endLon = QString("%1").arg(siIn.endLon(),0,'f',8);
  double radius = .020;
  QString distanceWhere;
  if(!reverse)
  { //compare start to start, end to end,
     distanceWhere = " type = " + QString("%1").arg(siIn.routeType())
                        + " and distance(" + startLat + "," + startLon + ", a.startLat, a.startLon) < "
                        + QString("%1").arg(radius,0,'f',8 ) +
                        " AND distance(" + endLat + "," + endLon + ", a.endLat, a.endLon) "
                        "< " + QString("%1").arg(radius,0,'f',8);
  }
  else { // compare start to end, end to start
     distanceWhere = " type =  " + QString("%1").arg(siIn.routeType())
                        + " and distance(" + startLat + "," + startLon + ", a.endLat, a.endLon) < "
                        + QString("%1").arg(radius,0,'f',8 ) +
                        " AND distance(" + endLat + "," + endLon + ", a.startLat, a.startLon) "
                        "< " + QString("%1").arg(radius,0,'f',8);
  }

  try
  {
    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    QSqlDatabase db = QSqlDatabase::database();

    QString commandText;

    if(config->currConnection->servertype() != "MsSql")
     commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon, "
          " a.length, a.street, a.description, a.OneWay, a.pointArray, a.tracks, a.type, DoubleDate"
          " from Segments a "
          " where " + distanceWhere +" and a.segmentId != " + QString("%1").arg(siIn.segmentId()) +
          " order by a.segmentId";
     else
      commandText = "select a.segmentId, a.startLat, a.startLon, a.endLat, a.EndLon,"
          " a.length, a.street, a.description, a.OneWay, a.pointArray, a.tracks,"
          " a.type, DoubleDate"
          " from Segments a "
          "where " + distanceWhere +" and a.segmentId != " + QString("%1").arg(siIn.segmentId()) +
          " order by a.segmentId";
    //qDebug() << commandText + "\n";
    QSqlQuery query = QSqlQuery(db);
    qDebug() << commandText;
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    SegmentInfo si;

    while(query.next())
    {
     si = SegmentInfo();
     si._segmentId = query.value(0).toInt();
     si._startLat = query.value(1).toDouble();
     si._startLon = query.value(2).toDouble();
     si._endLat = query.value(3).toDouble();
     si._endLon = query.value(4).toDouble();
     si._length = query.value(5).toDouble();
     si._streetName = query.value(6).toString();
     si._description = query.value(7).toString();
     //sd._oneWay = query.value(8).toString();
     si.setPoints(query.value(9).toString());
     si._tracks = query.value(10).toInt();
     si._routeType = (RouteType)query.value(11).toInt();
     si._dateDoubled = query.value(12).toDate();
     myArray.append(si);
    }

  }
  catch (Exception e)
  {
      //myExceptionHandler(e);
  }
  return myArray;
 }

QList<SegmentInfo> SQL::getDupSegments(SegmentInfo si)
{
   QList<SegmentInfo> list = getSegmentsInOppositeDirection(si);
   QList<SegmentInfo> list2 = getSegmentsInSameDirection(si);
   list.append(list2);
   return list;
}

QList<SegmentInfo> SQL::getSegmentsInOppositeDirection(SegmentInfo sdIn)
{
 return getSegmentsInSameDirection(sdIn,true);
}

bool SQL::deleteSegment(qint32 segmentId)
{
    bool ret = false;
    int rows = 0;
    SegmentInfo si = getSegmentInfo(segmentId);
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        beginTransaction("deleteSegment");

        QString commandText;
        QSqlQuery query = QSqlQuery(db);

        // delete any Line Segments using this segment
        commandText = "delete from LineSegment where segmentid = " + QString("%1").arg(segmentId);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        // delete any routes referencing the segment
        commandText = "delete from Routes where LineKey = " + QString("%1").arg(segmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();

        commandText = "delete from Segments where segmentid = " + QString("%1").arg(segmentId);
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //throw( new ApplicationException("deleteSegment, segmentID " + SegmentId + " not found"));
            qDebug()<< "deleteSegment, SegmentId " + QString("%1").arg(segmentId) + " not found";
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
    commitTransaction("deleteSegment");
    emit segmentChanged(si, CHANGETYPE::DELETESEG);
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
        if(config->currConnection->servertype() == "MySql")
            commandText = "select `key`, description from Companies where '" + date + "'"
                          " between startDate and endDate"
                          " and " + QString("%1").arg(route) + " between firstRoute and lastRoute";
        else if(config->currConnection->servertype() == "MsSql")

            commandText = "select [key], description from companies where '" + date + "'"
                          " between startDate and endDate and " + QString::number(route) +
                          " between firstRoute and lastRoute";
        else {
            commandText = "select key, description from Companies where '" + date + "'"
                          " between startDate and endDate and " + QString("%1").arg(route) +
                          " between firstRoute and lastRoute";

        }QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
        if(config->currConnection->servertype() == "MySql")
            commandText = "select `key`, description from Companies"
                          " where description = '" + name + "'";
        else if(config->currConnection->servertype() == "MsSql")
            commandText = "select [key], description from companies"
                          " where description = '" + name + "'";
        else {
            commandText = "select key, description from Companies"
                          " where description = '" + name + "'";

        }QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            companyKey = query.value(0).toInt();
        }

        if(companyKey == -1)
        {
            commandText = "insert into Companies (description, startDate, endDate, firstRoute, lastRoute) "
                          "values ('" + name + "', '"+ startDate + "','"+endDate+"',"+QString("%1").arg(route)+","+QString("%1").arg(route)+")";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
            }
            rows = query.numRowsAffected();
            if(rows > 0)
            {
                if(config->currConnection->servertype() == "MySql")
                    commandText = "select `key`, description from Companies where description = '" + name + "'";
                else if(config->currConnection->servertype() == "MsSql")
                    commandText = "select [key], description from Companies where description = '" + name + "'";
                else // Sqlite and PostgreSQL
                    commandText = "select key, description from Companies where description = '" + name + "'";
                bQuery = query.exec(commandText);
                if(!bQuery)
                {
                    QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                    qDebug() << errCommand;
                    QSqlError error = query.lastError();
                    SQLERROR(std::move(query));
                    throw SQLException(error.text() + " " + errCommand);
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
}
/// <summary>
/// Add or retrieve a segment Id corresponding to the Description
/// </summary>
/// <param name="Description"></param>
/// <returns></returns>
qint32 SQL::addSegment(QString Description, QString OneWay, int tracks, RouteType routeType, const QList<LatLng> pointList, QString location, bool *bAlreadyExists, bool forceInsert)
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
 beginTransaction("addSegment");

 QString commandText = "Select SegmentId from Segments where Description = '" + Description
   + "' and tracks = " + QString("%1").arg(tracks)
   + " and type = "  + QString("%1").arg(routeType)
   + " and OneWay= '" + OneWay + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 commandText = "Insert into Segments (street, Location, Description, OneWay, type, pointArray, tracks, "
               "startLat, startlon, endLat, endLon, length, Direction, startDate, endDate) "
               "values ('" +street+"','" + location + "','" + Description + "', '" + OneWay + "',"
               +   QString("%1").arg((qint32)routeType) + ",'"
               + pointArray+ "', "
               + QString::number(tracks)
               + ",0,0,0,0,0"
               + ", ' ', '1800/01/01'"
               + ", '2050/12/31'"
               + ")";
 bQuery = query.exec(commandText);
 qDebug() << "SQL::addSegment: " << commandText;
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
  }
  rows = query.numRowsAffected();

  // Now get the SegmentId (identity) value so it can be returned.
  if(config->currConnection->servertype() == "Sqlite")
    commandText = "SELECT LAST_INSERT_ROWID()";
  else if(config->currConnection->servertype() == "MySql")
    commandText = "SELECT LAST_INSERT_ID()";
  else if(config->currConnection->servertype() == "MsSql")
    commandText = "SELECT IDENT_CURRENT('Segments')";
  else // PostgreSQL
    commandText = "SELECT max(segmentId) from segments";
  bQuery = query.exec(commandText);
 if(!bQuery)
  {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 commitTransaction("addSegment");

 SegmentInfo si = getSegmentInfo(SegmentId);
 emit segmentChanged(si,CHANGETYPE::ADDSEG);
 return SegmentId;
}

/// <summary>
/// Add or retrieve a segment Id corresponding to the Description
/// </summary>
/// <param name="Description"></param>
/// <returns></returns>
qint32 SQL::addSegment(SegmentInfo si, bool *bAlreadyExists, bool forceInsert)
{
 int rows = 0;
 int segmentId = -1;
 *(bAlreadyExists) = false;
 QString street;
 if(si._description.contains(","))
  street = si._description.mid(0,si._description.indexOf(","));
 if(!si.doubleDate().isValid())
     qDebug() << tr("warning! segment %1 invalid doubledate").arg(si.segmentId());
 try
 {
 if(!dbOpen())
    throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 beginTransaction("addSegment");

 QString commandText = "Select SegmentId from Segments where Description = '" + si._description
   + "' and tracks = " + QString("%1").arg(si._tracks)
   + " and type = "  + QString("%1").arg(si._routeType);
//   + " and OneWay= '" + sd._oneWay + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
  segmentId = query.value(0).toInt();
 }

 if (segmentId > 0 && !forceInsert)
 {
  *(bAlreadyExists) = true;
  return segmentId;
 }

 // Add a new SegmentId
 QString pointArray = "";
 for(int i = 0; i < si._pointList.count(); i ++)
 {
  if(i > 0)
   pointArray.append(",");
  LatLng pt = si._pointList.at(i);
  pointArray.append(pt.str());
 }
 if(pointArray.count() < 2)
 {
  qDebug() << "Warning segment '" << si._description << "' has less than two points!!";
 }
 commandText = "Insert into Segments (street, Location, Description, NewerName, type, pointArray, points, tracks, "
               "startLat, startlon, endLat, endLon, length, Direction, startDate, endDate, DoubleDate) "
               "values ('" +street+"','" + si.location()+"','"+ si._description + "', '"
               + si._newerStreetName + "',"
               + QString("%1").arg((qint32)si._routeType) + ",'"
               + pointArray+ "', "
               + QString("%1").arg(si._points) + ", "
               + QString::number(si._tracks) + ", "
               //+ ",0,0,0,0,0"
               + QString("%1").arg(si._startLat,0,'f',8) + ", "
               + QString("%1").arg(si._startLon,0,'f',8) + ", "
               + QString("%1").arg(si._endLat,0,'f',8) + ", "
               + QString("%1").arg(si._endLon,0,'f',8) + ", "
               + QString("%1").arg(si._length,0,'f',8) + ", "
               //+ ", ' ', '1800/01/01'"
               + "'" + si._direction + "', "
               + "'" + si._dateBegin.toString("yyyy/MM/dd")+"', "
               //+ ", '2050/12/31'"
               + "'" + si._dateEnd.toString("yyyy/MM/dd")+"'"
               + ",'" + si._dateDoubled.toString("yyyy/MM/dd")+"'"
               + ")";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
  }
  rows = query.numRowsAffected();

  // Now get the SegmentId (identity) value so it can be returned.
  if(config->currConnection->servertype() == "Sqlite")
    commandText = "SELECT LAST_INSERT_ROWID()";
  else if(config->currConnection->servertype() == "MySql")
    commandText = "SELECT LAST_INSERT_ID()";
  else if(config->currConnection->servertype() == "MsSql")
    commandText = "SELECT IDENT_CURRENT('Segments')";
  else // PostgreSQL
    commandText = "SELECT max(segmentId) from segments";
  bQuery = query.exec(commandText);
 if(!bQuery)
  {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
  }
  while (query.next())
  {
    segmentId = query.value(0).toInt();
  }

 }
 catch (Exception e)
 {
    myExceptionHandler(e);
 }
 commitTransaction("addSegment");

 si._segmentId = segmentId;
 emit segmentChanged(si, CHANGETYPE::ADDSEG);
 return segmentId;
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
qint32 SQL::splitSegment(qint32 pt, qint32 segmentId, QString oldDesc, QString oldOneWay,
                         QString newDesc,
                         QString newOneWay, RouteType routeType, RouteType newRouteType,
                         int oldTracks, int newTracks,
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
 beginTransaction("splitSegment");

 SegmentInfo oldSi = getSegmentInfo(segmentId); // Original segment before any changes.
 // Sanity check
 if(pt < 1 || pt > oldSi.pointList().count()-2)
  throw IllegalArgumentException("bad split point");

 LatLng p = oldSi._pointList.at(pt); // breaking at this point!
 QString pointArray = "";
 double length = 0;
 for(int i =0; i <= pt; i++)
 {
  LatLng pt = oldSi._pointList.at(i);
  if(i > 0)
  {
   pointArray.append(",");
   length += Distance(pt.lat(), pt.lon(), oldSi._pointList.at(i-1).lat(), oldSi._pointList.at(i-1).lon());
  }
  pointArray.append(pt.str());
 }
 Bearing b= Bearing(oldSi._startLat, oldSi._startLon, p.lat(), p.lon());
 QString direction = oldSi._direction;
// if(oldSd.oneWay == "Y")
//  direction = b.strDirection();
// else
// direction = b.strDirection()+"-"+b.strReverseDirection();

 // Modify the description of the remaining (original) segment as well as the end point and length.
 Q_ASSERT(p.lat() != 0);
 Q_ASSERT(p.lon() != 0);

 commandText = "update Segments set description = '" + oldDesc + "', "
               //"OneWay='" + oldOneWay + "',"
               "endLat="+QString("%1").arg(p.lat(),0,'f',8)+","
               "endLon="+ QString("%1").arg(p.lon(),0,'f',8)+","
               "length="+QString("%1").arg(b.Distance(),0,'f',8)+","
               "points="+QString("%1").arg(pt+1)+","
               "direction = '"+ direction + "',"
               "pointArray='"+ pointArray + "', "
               "tracks=" +QString::number(oldTracks)+ ","
               "street='"+ oldStreet + "',"
               "lastUpdate=CURRENT_TIMESTAMP"
               " where SegmentId = " + QString("%1").arg(segmentId);
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 for(int i =pt; i < oldSi._pointList.count(); i++)
 {
  endpt = oldSi._pointList.at(i);
  if(i != pt)
  {
   pointArray2.append(",");
   length += Distance(endpt.lat(), endpt.lon(),
                      oldSi._pointList.at(i-1).lat(),
                      oldSi._pointList.at(i-1).lon());
  }
  pointArray2.append(endpt.str());
 }
 b = Bearing(p.lat(), p.lon(), oldSi._endLat, oldSi._endLon);
 // if(oldSi.oneWay == "Y")
 //  direction = b.strDirection();
 // else
 //  direction = b.strDirection()+"-"+b.strReverseDirection();
 commandText = "Insert into Segments (Description, type, startLat, startLon, endLat, "\
     "endLon, length, startDate, endDate, points, direction, pointArray, tracks, street) "
     "values ("
     "'" + newDesc + "', "
     + QString("%1").arg((int)newRouteType) + ","
     + QString("%1").arg(p.lat(),0,'f',8)+","
     + QString("%1").arg(p.lon(),0,'f',8) + ","
     + QString("%1").arg(endpt.lat(),0,'f',8) +","
     + QString("%1").arg(endpt.lon(),0,'f',8)+","
     + QString("%1").arg(length,0,'f',8)+",'"
     + oldSi._dateBegin.toString("yyyy/MM/dd")+"',"
     "'" + oldSi._dateEnd.toString("yyyy/MM/dd")+"',"
     + QString("%1").arg(pointArray2.size())+","
     "'" + direction+"',"
     "'" + pointArray2 + "', "
     + QString::number(newTracks)+","
     "'" + newStreet + "')";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 else if(config->currConnection->servertype() == "MySql")
   commandText = "SELECT LAST_INSERT_ID()";
 else if(config->currConnection->servertype() == "MsSql")
   commandText = "SELECT IDENT_CURRENT('Segments')";
 else // PostgreSQL
   commandText = "SELECT max(segmentId) from segments";

 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
  oldSi = getSegmentInfo(segmentId);
  SegmentInfo newSd = getSegmentInfo(newSegmentId);
  double diff = angleDiff(oldSi._bearing.angle(), newSd._bearing.angle());
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
  QList<SegmentData> routes;


  commandText = "SELECT a.route, n.name, a.startDate, a.endDate, a.companyKey, tractionType,"
                "c.routeAlpha, "
                "normalEnter, normalLeave, reverseEnter, reverseLeave, a.OneWay, a.Direction,"
                " trackUsage, b.street, b.description, a.routeId "
                "from Routes a "
                "join Segments b on LineKey = b.SegmentId "
                "join AltRoute c on a.route = c.route "
                "join RouteName n on n.routeId = a.routeId "
                "where SegmentId = " + QString("%1").arg(segmentId);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  while (query.next())
  {
   SegmentData sd = SegmentData();
   sd._route = query.value(0).toInt();
   sd._routeName = query.value(1).toString();
   sd._dateBegin = query.value(2).toDate();
   sd._dateEnd = query.value(3).toDate();
   sd._companyKey = query.value(4).toInt();
   sd._tractionType = query.value(5).toInt();
   sd._alphaRoute = query.value(6).toString();
   sd._normalEnter = query.value(7).toInt();
   sd._normalLeave = query.value(8).toInt();
   sd._reverseEnter = query.value(9).toInt();
   sd._reverseLeave = query.value(10).toInt();
   sd._segmentId = segmentId;
   sd._oneWay = query.value(11).toString();
   sd._direction = query.value(12).toString();
   sd._trackUsage = query.value(13).toString();
   sd._streetName = query.value(14).toString();
   sd._description = query.value(15).toString();
   sd._routeId = query.value(16).toInt();
   routes.append( sd);
  }


  //foreach (routeData rd1  in routes)
  for(int i=0; i < routes.count(); i++)
  {
   SegmentData sd1 =routes.at(i);
   // Check to see if the route exists already
   int count =0;
   commandText = "Select count(*) from Routes where route = " + QString("%1").arg(sd1._route) + " and " +
           "startDate = '" + sd1._dateBegin.toString("yyyy/MM/dd") + "'"
           " and enddate = '" + sd1._dateEnd.toString("yyyy/MM/dd") + "'"
           " and lineKey = " + QString("%1").arg(newSegmentId);
   bQuery = query.exec(commandText);
   if(!bQuery)
   {
       QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
       qDebug() << errCommand;
       QSqlError error = query.lastError();
       SQLERROR(std::move(query));
       throw SQLException(error.text() + " " + errCommand);
   }
   while (query.next())
   {
    count = query.value(0).toInt();
   }

   if (count == 0)
   {

    commandText = "insert into Routes ( route, routeId, startDate, endDate, "
                  "lineKey, companyKey, tractionType, "
                  "next, prev, normalEnter, normalLeave, reverseEnter, reverseLeave,"
                  " OneWay, Direction, trackUsage) "
                  " values ("
                  + QString("%1").arg(sd1._route) + ", "
                  + QString("%1").arg(sd1._routeId) + ", '"
                  +  sd1._dateBegin.toString("yyyy/MM/dd") +   "', '"
                  +  sd1._dateEnd.toString("yyyy/MM/dd") + "', "
                  + QString("%1").arg(newSegmentId) + ","
                  + QString("%1").arg(sd1._companyKey) + ","
                  + QString("%1").arg(sd1._tractionType) + ","
                  + QString("%1").arg(sd1._next) + ","
                  + QString("%1").arg(sd1._prev) + ","
                  + QString("%1").arg(normalEnter) + ","
                  + QString("%1").arg(sd1._normalLeave) + ","
                  + QString("%1").arg(sd1._reverseEnter) + ","
                  + QString("%1").arg(reverseLeave) + ", '"
                  + sd1._oneWay + "', '"
                  + newSd.bearing().strDirection() + "', '"
                  + sd1.trackUsage() + "')";

    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
     qDebug()<<"(splitSegment insert failed:" + commandText + "\n";
     return -1;
    }
    // Now update the turn info for the original segment
    commandText = "update Routes set normalLeave = " + QString("%1").arg(normalLeave) + ","
                  " reverseEnter = " + QString("%1").arg(reverseEnter)
                  + ",lastUpdate=CURRENT_TIMESTAMP"
                  " where route = " + QString("%1").arg(sd1._route) +
 //               " and name = '" + sd1._routeName + "'"
                  " and routeId = " + QString::number(sd1._routeId) +
                  " and startDate = '" + sd1._dateBegin.toString("yyyy/MM/dd")+ "'"
                  " and endDate = '" + sd1._dateEnd.toString("yyyy/MM/dd") + "'"
                  " and lineKey = " + QString("%1").arg(sd1._segmentId);
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    rows = query.numRowsAffected();
    if (rows == 0)
    {
     if(config->currConnection->servertype() !=  "MySql")
     {
         return -1;
     }
     emit routeChange(NotifyRouteChange(MODIFYSEG, &sd1));

     qDebug()<< "assume update was successful:" + commandText + "\n";
    }
   }
  }

  updateSegmentDates(segmentId);
  updateSegmentDates(newSegmentId);
  commitTransaction("splitSegment");

 }
 catch (Exception e)
 {
  myExceptionHandler(e);
 }

 return newSegmentId;
}

SegmentData* SQL::getSegmentData(qint32 route, qint32 segmentId, QString startDate, QString endDate)
{
 QString where = "where route = " + QString("%1").arg(route)
   + " and segmentId = " + QString("%1").arg(segmentId)
   + " and endDate = '" + endDate + "'";
 QList<SegmentData*> list = segmentDataListFromView(where);
 if(!list.isEmpty())
  return list.at(0);
 return nullptr;
}

QList<SegmentData*> SQL::getSegmentDataList(RouteData rd)
{
 QString where = " where route = " + QString("%1").arg(rd._route)
   + " and trim(routeName) = '" + rd._name.trimmed() + "'"
//                       + " and a.companyKey = " + QString::number(rd.companyKey)
   + " and startDate = '" + rd._dateBegin.toString("yyyy/MM/dd") + "'"
   + " and endDate = '" + rd._dateEnd.toString("yyyy/MM/dd") + "'";
 return segmentDataListFromView(where);
}
#if 0
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
                               qint32 normalEnter, qint32 normalLeave, qint32 reverseEnter, qint32 reverseLeave,
                               QString oneWay, QString newerName)
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
        beginTransaction("updateSegmentToRoute");

        QString commandText = "Update Routes set companyKey= " +QString("%1").arg( companyKey)
          + ", tractionType=" + QString("%1").arg(tractionType)
          + ", normalEnter = " + QString("%1").arg(normalEnter)
          + ", normalLeave= " + QString("%1").arg(normalLeave)
          + ", reverseEnter= " + QString("%1").arg(reverseEnter)
          + ", reverseLeave = " + QString("%1").arg(reverseLeave)
          + ", OneWay = '" + oneWay
          + "', lastUpdate=CURRENT_TIMESTAMP "
          "where route = " + QString("%1").arg(routeNbr)
          // + " and Name= '" + routeName
          + "and routeId = " + QString::number(routeid)
          + " and LineKey = " + QString("%1").arg(SegmentId)
          + " and StartDate= '" + startDate
          + "' and EndDate= '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            SQLERROR(std::move(query));
            db.close();
            exit(EXIT_FAILURE);
        }
        rows = query.numRowsAffected();
        if (rows == 0)
        {
            //                    RollbackTransaction("deletePoint");

            return ret;
        }

        //updateSegmentDates(SegmentId);

        commitTransaction("updateSegmentToRoute");

        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }

    return ret;
}
#endif
SegmentData* SQL::getConflictingSegmentDataForRoute(qint32 route, QString name, qint32 segmentId,
                                              QString startDate, QString endDate)
{
 QString where = " where Route = " + QString("%1").arg(route) + ""
                 " and ('" + startDate + "' between startDate and endDate"
                 " or  '" + endDate + "' between startDate and endDate)"
                 " and trim(RouteName) = '" + name.trimmed() + "'"
                 " and SegmentId = " + QString("%1").arg(segmentId);
 QList<SegmentData*> list = segmentDataListFromView(where);
 if(list.count() > 0)
  return list.at(0);
 if(list.count()>1)
  qDebug() << "warning query should not have returned more than 1 row!";
 return nullptr;
}

RouteData SQL::getRouteDataForRouteDates(qint32 route, QString name, qint32 segmentId, QString startDate, QString endDate)
{
    RouteData rd;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT a.Route,n.Name,a.StartDate,a.EndDate,LineKey,CompanyKey,tractionType,"
                              " a.direction, normalEnter, normalLeave, reverseEnter, reverseLeave, "
                              " routeAlpha, a.OneWay, s.tracks, a.TrackUsage"
                              " from Routes a "
                              " join AltRoute c on a.route = c.route"
                              " join Segments s on a.lineKey = s.segmentId"
                              " join RouteName on a.routeId = n.routeId"
                              " where a.Route = " + QString("%1").arg(route) + ""
                              " and ('" + startDate + "' between a.startDate and a.endDate"
                              " or  '" + endDate + "' between a.startDate and a.endDate)"
                              " and n.name = '" + name + "'"
                              " and a.LineKey = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
            return rd;
        }
        //                myArray = new LatLng[myReader.RecordsAffected];

        while (query.next())
        {
            rd._route = query.value(0).toInt();
            rd._name = query.value(1).toString();
            rd._dateBegin = query.value(2).toDate();
            rd._dateEnd = query.value(3).toDate();
            rd._lineKey = query.value(4).toInt();
            rd._companyKey = query.value(5).toInt();
            rd._tractionType = query.value(6).toInt();
            rd._direction = query.value(7).toString();
            rd._normalEnter = query.value(8).toInt();
            rd._normalLeave = query.value(9).toInt();
            rd._reverseEnter = query.value(10).toInt();
            rd._reverseLeave = query.value(11).toInt();
            rd._alphaRoute = query.value(12).toString();
            rd._oneWay = query.value(13).toString();
            rd._tracks = query.value(14).toInt();
            rd._trackUsage = query.value(15).toString();
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
bool SQL::deleteRoute(qint32 route, int routeId, QString startDate, QString endDate)
{
    bool ret = false;
    int rows = 0;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        beginTransaction("deleteRoute");

        QString commandText;
        if(config->currConnection->servertype() != "MsSql")
            commandText = "delete from Routes where route = " + QString("%1").arg(route)
              + " and routeId = " + QString::number(routeId)
              + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        else
            commandText = "delete from [dbo].[routes] where [route] = " + QString("%1").arg(route)
              + " and routeId = '" + QString::number(routeId)
              + " and startDate = '" + startDate + "' and endDate = '" + endDate + "'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

        commitTransaction("deleteRoute");
        ret = true;
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }
    return ret;
}


// delete a single segment
bool SQL::deleteRoute(SegmentData sd)
{
 bool ret = true;
 int rows = 0;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText;
 if(config->currConnection->servertype() != "MsSql")
  commandText = "delete from Routes"
                " where route = " + QString("%1").arg(sd.route())
                + " and routeId = " + QString::number(sd.routeId())
                + " and startDate = '" + sd.startDate().toString("yyyy/MM/dd")
                + "' and endDate = '" + sd.endDate().toString("yyyy/MM/dd")
                + "' and lineKey ="+ QString::number(sd.segmentId());
 else
  commandText = "delete from [dbo].[routes] where [route] = " + QString("%1").arg(sd.route()) + " and name = '"
    + sd.routeName() + "' and startDate = '" + sd.startDate().toString("yyyy/MM/dd") + "' and endDate = '"
    + sd.endDate().toString("yyyy/MM/dd") + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 emit routeChange(NotifyRouteChange(DELETESEG, &sd));
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
  commandText = "delete from Routes where route = " + QString("%1").arg(rd.route())
    + " and routeId = " + QString::number(rd._routeId)
    + " and startDate = '" + rd._dateBegin.toString("yyyy/MM/dd")
    + "' and endDate = '" + rd._dateEnd.toString("yyyy/MM/dd")
    + "' and lineKey ="+ QString::number(rd.segmentId());
 else
  commandText = "delete from [dbo].[routes] where [route] = " + QString("%1").arg(rd._route)
                + " and routeId = " + QString::number(rd._routeId)
                + " and startDate = '" + rd._dateBegin.toString("yyyy/MM/dd")
                + "' and endDate = '" + rd.endDate().toString("yyyy/MM/dd") + "'";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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

bool SQL::modifyRouteDate(RouteData* rd, bool bStartDate, QDate dt/*, QString name1, QString name2*/)
{
 bool ret = true;
 int rows = 0;
  beginTransaction("modifyRouteDate");
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

  if(!modifyCurrentRoute(rd, bStartDate, dt/*, name1, name2*/))
    ret = false;

  if(ret)
   commitTransaction("modifyRouteDate");
  else
   rollbackTransaction("modifyRouteDate");
 return ret;
}

bool SQL::modifyCurrentRoute(RouteData* rd, bool bStartDate, QDate dt)
{
 bool ret = false;
 QSqlDatabase db = QSqlDatabase::database();
 QList<SegmentData*> segmentlist = getSegmentDatasForDate(rd->route(), rd->routeName(),rd->companyKey(), rd->endDate());
 QString commandText;
 int rows;
 if(!executeCommand(QString("delete from routes where route=%1 and routeid = %2")
                    .arg(rd->_route).arg(rd->_routeId)))
 {
       qDebug() << "delete route failed";
         return false;
 }
 RouteInfo ri = RouteInfo(*rd);
 if (bStartDate)
     ri.startDate = dt;
 else
     ri.endDate = dt;
 if(!updateRouteName(ri))
 {
    qDebug() << "updateRouteName failed";
    return false;
 }
 foreach(SegmentData* sd, segmentlist)
 {
     if (bStartDate)
         sd->_dateBegin = dt;
     else
         sd->_dateEnd = dt;
    if(!addSegmentToRoute(sd, true))
    {
        qDebug() << "insert route failed";
        return false;
     }
 }
  ret = true;
 return ret;
}

// helper routines for Routes
QString SQL::getPrevRouteName(QDate dt)
{
 QString commandText;
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 commandText = QString("select n.name from Routes r "
                       "join RouteName n on n.routeId = r.routeid "
                       "where r.endDate = '%1'").arg(dt.addDays(-1).toString("yyyy/MM/dd"));
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 commandText = QString("select n.name from Routes r "
                       "join RouteName n on n.routeId = r.routeid "
                       "where r.startDate = '%1'").arg(dt.addDays(1).toString("yyyy/MM/dd"));
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  return query.value(0).toString();
 }
 return "";
}

bool SQL::insertRouteSegment(SegmentData sd, bool bNotify)
{
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 if(!sd._dateBegin.isValid() || !sd._dateEnd.isValid() || sd._dateEnd < sd._dateBegin)
  throw IllegalArgumentException("Invalid dates ");
 if(!isCompanyValid(sd))
 {
  qWarning() << "invalid companyKey " << sd.companyKey();
  //return false;
 }

 commandText = "INSERT INTO Routes (Route, RouteId, StartDate, EndDate, LineKey, companyKey, tractionType, direction, "
               "next, prev, normalEnter, normalleave, reverseEnter, reverseLeave,sequence, reverseSeq, "
               "oneWay, trackusage) "
               "VALUES(" + QString("%1").arg(sd._route) + ", "
               + QString("%1").arg(sd._routeId) + ",'"
               + sd._dateBegin.toString("yyyy/MM/dd") + "', '"
               + sd._dateEnd.toString("yyyy/MM/dd") + "',"
               + QString("%1").arg(sd._segmentId) + ", "
               + QString("%1").arg(sd._companyKey)+","
               + QString("%1").arg(sd._tractionType)+",'"
               + sd._direction +"', "
               + QString("%1").arg(sd._next) + ","
               + QString("%1").arg(sd._prev) + ","
               + QString("%1").arg(sd._normalEnter) + ","
               + QString("%1").arg(sd._normalLeave) + ","
               + QString("%1").arg(sd._reverseEnter) + ", "
               + QString("%1").arg(sd._reverseLeave) + ", "
               + QString("%1").arg(sd._sequence) + ", "
               + QString("%1").arg(sd._returnSeq) + ", '"
               + sd._oneWay + "', '"
               + sd._trackUsage
               + "')";
 if(!query.exec(commandText))
 {
  QString text;
  SQLERROR1(std::move(query), QMessageBox::Ignore | QMessageBox::Ok, text);
  switch(errReturn)
  {
  case QMessageBox::Abort:
      EXIT_FAILURE;
  case QMessageBox::Ignore:
      return true;
  case QMessageBox::Ok:
      return false;
  default:
      break;
  }
 }
 if(bNotify)
    emit routeChange(NotifyRouteChange(ADDSEG, &sd));
 return true;
}


/// <summary>
/// Returns a list of route segments with dates that overlap the specified route segment.
/// </summary>
/// <param name="route"></param>
/// <param name="name"></param>
/// <param name="startDate"></param>
/// <param name="endDate"></param>
/// <param name="segmentId"></param>
/// <returns></returns>
QList<SegmentData*> SQL::getConflictingRouteSegments(qint32 route, QString name,
                                                    QDate startDate, QDate endDate, int companyKey,
                                                    qint32 segmentId)
{
#if 0
    QList<SegmentData*> myArray;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "Select a.route, n.name, a.startDate, a.endDate, "
                              "lineKey, tractionType, a.companyKey, a.direction,"
                              " normalEnter, normalLeave, reverseEnter, reverseLeave,"
                              " b.routeAlpha, a.OneWay, s.description, s.length, s.startDate, s.endDate, a.routeId"
                              " from Routes a"
                              " join AltRoute b on a.route = b.route"
                              " join Segments s on a.linekey = s.segmentid"
                              " join RouteName n on n.routeId = a.routeid "
                              " where ((a.startDate between '" + startDate.toString("yyyy/MM/dd") + "'"
                              " and '" + endDate.toString("yyyy/MM/dd)" + "')"
                              " or (a.endDate between '" + startDate.toString("yyyy/MM/dd") + "'"
                              " and '" + endDate.toString("yyyy/MM/dd") + "'))"
                              " and a.route = " + QString("%1").arg(route) + ""
                              //" and name = '" + name + "' and a.endDate <> '" + endDate + "'"
                              " and a.endDate <> '" + endDate + "'"
                              " and a.companyKey = " + QString::number(companyKey) +
                              " and lineKey = " + QString("%1").arg(segmentId);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QSqlError err = query.lastError();
            SQLERROR(std::move(query));
            qDebug() << err.text() + "\n";
            qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
            db.close();
            exit(EXIT_FAILURE);
        }
        while (query.next())
        {
            SegmentData* sd = new SegmentData();
            sd->_route = query.value(0).toInt();
            sd->_routeName = query.value(1).toString();
            sd->_dateBegin = query.value(2).toDate();
            sd->_dateEnd = query.value(3).toDate();
            sd->_segmentId = query.value(4).toInt();
            sd->_tractionType =query.value(5).toInt();
            sd->_companyKey = query.value(6).toInt();
            sd->_direction = query.value(7).toString();
            sd->_normalEnter = query.value(8).toInt();
            sd->_normalLeave = query.value(9).toInt();
            sd->_reverseEnter = query.value(10).toInt();
            sd->_reverseLeave = query.value(11).toInt();
            sd->_oneWay = query.value(12).toString();
            sd->_description = query.value(13).toString();
            sd->_length = query.value(14).toInt();
            sd->_segmentDateStart = query.value(15).toDate();
            sd->_segmentDateEnd = query.value(16).toDate();
            sd->_routeId = query.value(17).toInt();
            myArray.append(sd);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }

    return myArray;
#endif
    QString where = " where ((StartDate between '" + startDate.toString("yyyy/MM/dd") + "'"
                    " and '" + endDate.toString("yyyy/MM/dd") + "')"
                    " or (EndDate between '" + startDate.toString("yyyy/MM/dd") + "'"
                    " and '" + endDate.toString("yyyy/MM/dd") + "'))"
                    " and Route = " + QString("%1").arg(route) + ""
                    //" and name = '" + name + "' and a.endDate <> '" + endDate + "'"
                    " and EndDate <> '" + endDate.toString("yyyy/MM/dd") + "'"
                    " and CompanyKey = " + QString::number(companyKey) +
                    " and SegmentId != " + QString("%1").arg(segmentId);
    return segmentDataListFromView(where);
}

bool compareSegmentData(const SegmentData & sd1, const SegmentData & sd2)
{
 return sd1.segmentId() < sd2.segmentId();
}
bool compareSegmentInfo(const SegmentInfo & sd1, const SegmentInfo & sd2)
{
 return sd1.segmentId() < sd2.segmentId();
}

QList<SegmentData*> SQL::getRoutes(qint32 segmentid)
{
    //SegmentInfo sd = getSegmentInfo(segmentid);  // get some info about the segment.
    //QList<SegmentData>  myArray;
    //compareSegmentDataClass compareSegments = new compareSegmentDataClass();

    // get a list of all the routes using this segment on this date
    QList<SegmentData*> segmentInfoList = getRouteSegmentsBySegment(segmentid);

    return segmentInfoList;
}

QList<RouteIntersects> SQL::updateLikeRoutes(qint32 segmentid, qint32 route, QString name, QString date, bool bAllRoutes)
{
    QList<RouteIntersects> intersectList;
    RouteIntersects ri;
    SegmentInfo sd = getSegmentInfo(segmentid);  // get some info about the segment.
    QList<SegmentInfo>  myArray;
    //compareSegmentDataClass compareSegments = new compareSegmentDataClass();

    // get a list of all the routes using this segment on this date
    QList<SegmentData*> segmentInfoList = getRouteSegmentsBySegment(segmentid);

    // get other segments intersecting with the start of this segment
    myArray = getIntersectingSegments(sd.startLat(), sd.startLon(), .020, sd.routeType());
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
    std::sort(ri.startIntersectingSegments.begin(), ri.startIntersectingSegments.end(), compareSegmentData);

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
    std::sort(ri.endIntersectingSegments.begin(), ri.endIntersectingSegments.end(), compareSegmentData);

    // Populate the intersect list
    //foreach (routeData rdi in segmentInfoList)
    for(int i =0; i < segmentInfoList.count(); i++)
    {
        SegmentData *sd1 = new SegmentData(*segmentInfoList.at(i));
        // Get my info
        if (sd1->route() == route && sd1->routeName() == name && sd1->endDate().toString("yyyy/MM/dd") == date)
            ri.sd = *sd1;
        else
        {
            RouteIntersects ri2 =  RouteIntersects();
            ri2.sd = *sd1;
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
                        if (isRouteUsedOnDate(sd1.route(), sd1.segmentId(), sd1.endDate().toString("yyyy/MM/dd")))
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
                        if (isRouteUsedOnDate(sd1->route(), sd2.segmentId(), sd1->endDate().toString("yyyy/MM/dd")))
                            ri2.endIntersectingSegments.append(sd2);
                    }
                }
            }
            //ri2.startIntersectingSegments.Sort(compareSegments);
            std::sort(ri2.startIntersectingSegments.begin(), ri2.startIntersectingSegments.end(), compareSegmentData);
            //ri2.endIntersectingSegments.Sort(compareSegments);
            std::sort(ri2.endIntersectingSegments.begin(), ri2.endIntersectingSegments.end(), compareSegmentData);
            if(ri2.endIntersectingSegments.count() == ri.endIntersectingSegments.count() &&
                    ri2.startIntersectingSegments.count() == ri.startIntersectingSegments.count())
            intersectList.append(ri2);
        }
    }
    return intersectList;
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

        QString commandText = "select count(*) from Routes "
                              "where route = " + QString("%1").arg(route)
                              + " and lineKey= " + QString("%1").arg(segmentId)
                              + " and '"+ date + "' between startDate and endDate";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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

QList<StationInfo> SQL::getStations(QString alphaRoute, QDate date)
{
 bool bZeroRoutes = false;
 QList<StationInfo> myArray;
 if(!dbOpen())
    throw Exception(tr("database not open: %1").arg(__LINE__));
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 StationInfo sti;
 //int pointsCount = 0;
 bool bQuery;
    commandText = "select stationKey, routes, name, latitude, longitude, startDate, "
                  "endDate, MarkerType from Stations where  routes like '%" + alphaRoute+ "%' "
                  " and '" + date.toString("yyyy/MM/dd")
                  + "' between startDate and endDate";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }

 if (!query.isActive())
 {
    return myArray;
 }
 while (query.next())
 {
    StationInfo sti =  StationInfo();
    sti.stationKey = query.value(0).toInt();
    sti.routes = query.value(1).toString().split(",");
    sti.stationName = query.value(2).toString();
    sti.latitude = query.value(3).toDouble();
    sti.longitude = query.value(4).toDouble();
    sti.startDate = query.value(5).toDate();
    sti.endDate = query.value(6).toDate();
    sti.markerType = query.value(7).toString();
    if(!sti.routes.contains(alphaRoute))
     continue;
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
     commandText = "SELECT stationKey, a.name, latitude, longitude,  "
                   "a.endDate "
                   "from Stations a  GROUP BY stationKey, a.name, latitude, longitude, "
                   "a.startDate, a.endDate";
 else
     commandText = "SELECT stationKey, a.name, latitude, longitude,  a.startDate, "
                   "a.endDate, from Stations a  GROUP BY stationKey, a.name, latitude, longitude,  "
                   "a.startDate, a.endDate";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
     sti.startDate = query.value(7).toDate();
     sti.endDate = query.value(8).toDate();
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
        beginTransaction("addComment");

        //if(config->currConnection->servertype() != "MySql")
        {
            comments = comments.replace(up, "&up");
            comments = comments.replace(down, "&down");
            //comments = comments.toLatin1();
        }

        QString commandText = QString("insert into Comments (comments, tags ) "
                                      "values('%1','%2')").arg(comments.replace("'","\''"),tags);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            //throw SQLException(error.text() + " " + errCommand);
            return -1;
        }

        //int rows = query.numRowsAffected();
        //if (rows == 1)
        {
            if(config->currConnection->servertype() == "Sqlite")
              commandText = "SELECT LAST_INSERT_ROWID()";
            else if(config->currConnection->servertype() == "MySql")
              commandText = "SELECT LAST_INSERT_ID()";
            else if(config->currConnection->servertype() == "MsSql")
              commandText = "SELECT IDENT_CURRENT('comments')";
            else // PostgreSQL
              commandText = "SELECT max(commentKey) from comments";
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
            }
            while (query.next())
            {
                infoKey = query.value(0).toInt();
            }
            commitTransaction("addComment");
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

        QString commandText = "update  Comments set comments='"+ comments.replace("'","\''")+"', "
                              "tags ='" + tags + "',"
                              "lastUpdate=CURRENT_TIMESTAMP "
                              "where commentKey = " + QString("%1").arg(infoKey);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
    if(!date.isValid())
     date = QDate::fromString("1800/01/01", "yyyy/MM/dd");

    RouteComments rc;
    rc.route = -1;
    rc.commentKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    rc.companyKey = companyKey;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = QString("SELECT rc.commentKey, c.commentKey, rc.companyKey, comments, tags, rc.latitude, "
            "rc.longitude, n.name, a.routeAlpha "
            "from Comments c "
            "join RouteComments rc on rc.commentKey = c.commentKey "
            "join Routes r on r.route = rc.route and '%2' between r.startDate and r.endDate "
            "JOIN AltRoute a ON a.route = rc.route "
            "join RouteName n on r.routeid =n.routeid "
            "where rc.route = %1 and rc.date = '%2'").arg(route).arg(date.toString("yyyy/MM/dd"));

        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.commentKey = query.value(0).toInt();
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

        if(rc.commentKey == -1)
        {
            //db.transaction();
            beginTransaction("updateouteComment");
            //qDebug()<< rc.ci.comments;

            rc.ci.commentKey = addComment(rc.ci.comments, rc.ci.tags);

            commandText = QString("insert into RouteComments (route, date, commentKey, companyKey,"
                                  " latitude, longitude) "
            " values(%1, '%2', %3, %4, %5, %6)").arg(rc.route).arg(rc.date.toString("yyyy/MM/dd"))
                    .arg(rc.ci.commentKey ).arg(rc.companyKey).arg(rc.pos.lat()).arg(rc.pos.lon());
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                //throw SQLException(error.text() + " " + errCommand);
                return false;
            }
            db.commit();
            ret = true;
        }
        else
        {
            commandText = QString("update RouteComments set companyKey = %1, "
                          "latitude = %2, longitude=%3, "
                          "lastUpdate=CURRENT_TIMESTAMP  where route = %4 and date = '%5'")
                          .arg(rc.companyKey).arg(rc.pos.lat()).arg(rc.pos.lon()).arg(rc.route)
                          .arg(rc.date.toString("yyyy/MM/dd"));
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
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
     commandText = "select count(*) from Stations where infoKey = " + QString::number(commentKey);
     bQuery = query.prepare(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
     }
     bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
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
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
     }
     bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
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

        beginTransaction("deleteRouteComment");

        commandText = QString("delete from RouteComments where route = %1 and date = '%2'")
                .arg(rc.route).arg(rc.date.toString("yyyy/MM/dd"));
        bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }

        int count = countCommentUsers(rc.ci.commentKey);
        if(count ==0)
        {
         if(deleteComment(rc.ci.commentKey))
         {
          commitTransaction("deleteRouteComment");
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
    rollbackTransaction("deleteRouteComment");
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
    if(!date.isValid())
     date = QDate::fromString("1800/01/01", "yyyy/MM/dd");

    RouteComments rc;
    rc.route = -1;
    rc.commentKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    rc.companyKey = companyKey;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT rc.commentKey, comments, tags, date, rc.companyKey, n.name,"
                " a.routeAlpha, rc.latitude, rc.longitude "
                "from RouteComments rc "
                "join Comments c on rc.commentKey = c.commentKey "
                "JOIN AltRoute a ON a.route = rc.route "
                "join Routes r on r.route = rc.route and '"+date.toString("yyyy/MM/dd")+"' between r.startDate and r.endDate "
                "join RouteName n on r.routeId = n.Routeid "
                "and rc.date  > '" + date.toString("yyyy/MM/dd")+"'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        //qDebug()<< query.lastQuery();
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.commentKey = query.value(0).toInt();
            rc.ci.commentKey = rc.commentKey;
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
    rc.commentKey=-1;
    rc.ci.commentKey = -1;
    rc.ci.comments = "";
    rc.ci.tags = "";
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();

        QString commandText = "SELECT rc.commentKey, comments, tags, date, rc.companyKey, n.name,"
                " a.routeAlpha, rc.latitude, rc.longitude "
                "from RouteComments rc "
                "join Comments c on rc.commentKey = c.commentKey "
                "JOIN AltRoute a ON a.route = rc.route "
                "join Routes r on r.route = rc.route"
                " and '"+date.toString("yyyy/MM/dd")+"' between r.startDate and r.endDate "
                "join RouteName n on r.routeId = n.Routeid "
                "where rc.route = "+ QString("%1").arg(route) +" "
                "and rc.date  < '" + date.toString("yyyy/MM/dd")+"'";
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }

        if (!query.isActive())
        {
            return rc;
        }
        while (query.next())
        {
            rc.commentKey = query.value(0).toInt();
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
qint32 SQL::addStation(StationInfo sti)
{
   int stationKey = -1;
   int rows = -1;
   try
   {
       if(!dbOpen())
           throw Exception(tr("database not open: %1").arg(__LINE__));
       QSqlDatabase db = QSqlDatabase::database();

       QString commandText = "select stationKey from Stations where name ='"
         + sti.stationName + "' and startDate ='" + sti.startDate.toString("yyyy/MM/dd")
         + "' and endDate = '" + sti.endDate.toString("yyyy/MM/dd") + "'";
       QSqlQuery query = QSqlQuery(db);
       bool bQuery = query.exec(commandText);
       if(!bQuery)
       {
           QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
           qDebug() << errCommand;
           QSqlError error = query.lastError();
           SQLERROR(std::move(query));
           throw SQLException(error.text() + " " + errCommand);
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
       commandText = " insert into Stations (routes, name, latitude, longitude, "
                     "startDate, endDate, segmentId, infoKey,MarkerType," \
                     "segments, routeType) ";
         "values ('" + sti.routes.join(",") + "','" +sti.stationName + "', "
         + QString::number(sti.latitude,'g',8) + ","
         + QString::number(sti.longitude,'g',8) + ",'"
         + sti.startDate.toString("yyyy/MM/dd") + "','"
         + sti.endDate.toString("yyyy/MM/dd")
         + "', " + QString::number(sti.segmentId)
         + ", " + QString::number(sti.infoKey)
         + ",'" +sti.markerType + "'"
         + ", '" + sti.segments.join(",") + "'"
         + "," + QString::number(sti.routeType) + ")";
       bQuery = query.exec(commandText);
       if(!bQuery)
       {
           QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
           qDebug() << errCommand;
           QSqlError error = query.lastError();
           SQLERROR(std::move(query));
           throw SQLException(error.text() + " " + errCommand);
       }
       rows = query.numRowsAffected();
       //if (rows == 1)
       {
           //myCommand.commandText = "SELECT stationKey from Stations where name = '" + name + "'";
            if(config->currConnection->servertype() == "Sqlite")
               commandText = "SELECT LAST_INSERT_ROWID()";
             else if(config->currConnection->servertype() == "MySql")
               commandText = "SELECT LAST_INSERT_ID()";
             else if(config->currConnection->servertype() == "MsSql")
               commandText = "SELECT IDENT_CURRENT('dbo.stations')";
             else // PostgreSQL
               commandText = "SELECT max(stationKey) from stations";

           bQuery = query.exec(commandText);
           if(!bQuery)
           {
               QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
               qDebug() << errCommand;
               QSqlError error = query.lastError();
               SQLERROR(std::move(query));
               throw SQLException(error.text() + " " + errCommand);
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
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
#if 0
bool SQL::updateRoute(qint32 route, QString name, QString endDate, qint32 segmentId,
                      qint32 next, qint32 prev, QString trackUsage)
{
 if(!QDate::fromString(endDate, "yyyy/MM/dd").isValid())
  throw IllegalArgumentException(tr("invalid date '%1'").arg(endDate));
 bool ret = false;
 int rows = 0;
 QSqlDatabase db = QSqlDatabase::database();

 QString commandText = "update Routes set next = " + QString("%1").arg(next)
             + ", trackUsage  = '" + trackUsage + "'"
             + ", prev=" + QString("%1").arg(prev)+ ",lastUpdate=CURRENT_TIMESTAMP"
             " where route ="+QString("%1").arg(route)
             + " and name ='"+name+"' and endDate='"+endDate
             +"' and lineKey="+QString("%1").arg(segmentId);
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
 return ret;
}
#endif
// if notify is false, routeview will not be notified of changes
bool SQL::updateRoute(SegmentData osd, SegmentData sd, bool notify, bool ignoreErr)
{
 bool ret = false;
 int rows = 0;
 QSqlDatabase db = QSqlDatabase::database();
 // changes to lineKey not possible!
 if(sd.route() < 1 || osd.segmentId() != sd.segmentId())
 {
  qCritical() << " illegal change to route orsegmentId";
  return false;
 }
 if(!ignoreErr)
 {
     if( sd.startDate() > sd.endDate())
     {
      qCritical() << " date error ";
      return false;
     }
     if( sd.startDate() == sd.endDate())
     {
      qWarning() << " startdate = enddate ";
      return true;
     }

     if(!isCompanyValid(sd))
     {
      //qDebug() << "invalid companyKey " << sd.companyKey();
         CompanyData* cd = getCompany(sd.companyKey());
         if(cd)
         qWarning() << tr("Company key %1 %2 %3-%4 is invalid for route %5 %6 %7-%8")
                       .arg(cd->companyKey).arg(cd->name, cd->startDate.toString("yyyy/MM/dd"),cd->endDate.toString("yyyy/MM/dd"))
                       .arg(sd.route()).arg(sd.routeName(),sd.startDate().toString("yyyy/MM/dd"),sd.endDate().toString("yyyy/MM/dd"));
         else
             qWarning() << tr("Company key %1 %2 %3-%4 is invalid for route %5 %6 %7-%8")
                           .arg(sd.companyKey()).arg("not found", "","")
                           .arg(sd.route()).arg(sd.routeName(),sd.startDate().toString("yyyy/MM/dd"),sd.endDate().toString("yyyy/MM/dd"));
        return false;
     }

    if(sd._normalEnter > 2 || sd._normalEnter < 0
    || sd._normalLeave > 2 || sd._normalLeave < 0
    || sd._reverseEnter > 2 || sd._reverseEnter < 0
    || sd._reverseLeave > 2 || sd._reverseLeave < 0)
    {
    qDebug() << "invalid normal/reverse vales";
    return false;
    }
 }
 if(sd.trackUsage().isEmpty() )
  sd.setTrackUsage(" ");

 // if any primary key fields are changing, delete and insert!
 if(osd.startDate() != sd.startDate() || osd.endDate() != sd.endDate() || osd.companyKey() != sd.companyKey() ||
         osd.route() != sd.route() || osd.routeName() != sd.routeName() || osd.segmentId() != sd.segmentId())
 {
     beginTransaction("routeKeyChange");
     if(!deleteRouteSegment(osd))
     {
         rollbackTransaction("routeKeyChange");
         return false;
     }
     if(!insertRouteSegment(sd))
     {
         rollbackTransaction("routeKeyChange");
         return false;
     }
     commitTransaction("routeKeyChange");
     return true;
 }

 QString commandText = "update Routes "
             " set next = " + QString("%1").arg(sd.next())
             + ", prev =" + QString("%1").arg(sd.prev())
             + ", nextR = " + QString("%1").arg(sd.nextR())
             + ", prevR =" + QString("%1").arg(sd.prevR())
             + ", trackUsage  = '" + sd.trackUsage() + "'"
             + ", oneWay  = '" + sd.oneWay() + "'"
             + ", direction  = '" + sd.direction() + "'"
             + ", companyKey =" + QString("%1").arg(sd.companyKey())
             + ", tractionType =" + QString("%1").arg(sd.tractionType())
             + ", normalEnter = " + QString::number(sd.normalEnter())
             + ", normalLeave = " + QString::number(sd.normalLeave())
             + ", reverseEnter = " + QString::number(sd.reverseEnter())
             + ", reverseLeave = " + QString::number(sd.reverseLeave())
             + ", Sequence = " + QString::number(sd._sequence)
             + ", ReverseSeq = " + QString::number(sd._returnSeq)
             + ", Route = " + QString::number(sd._route)
             + ", RouteId = " + QString::number(sd._routeId)
             // + ", name = '" + sd.routeName() + "'"
             + ", routeId = " + QString::number(sd.routeId())
             + ", startDate = '" + sd.startDate().toString("yyyy/MM/dd")+ "'"
             + ", endDate = '" + sd.endDate().toString("yyyy/MM/dd")+ "'"
             + ", lastUpdate=CURRENT_TIMESTAMP"
             " where route =" + QString("%1").arg(osd.route())
             // + " and name ='" + osd.routeName() + "'"
             + " and routeId = " + QString::number(osd._routeId)
             + " and startDate ='" + osd.startDate().toString("yyyy/MM/dd")+ "'"
             + " and endDate='" + osd.endDate().toString("yyyy/MM/dd")+ "'"
             + " and lineKey=" + QString("%1").arg(osd.segmentId());
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
  //qDebug() << commandText + " line:" + QString("%1").arg(__LINE__) +"\n";
 rows = query.numRowsAffected();
 if (rows > 0)
 {
  SegmentData* sdNew = new SegmentData(sd);
  if(notify)
   emit routeChange(NotifyRouteChange(MODIFYSEG, sdNew));
  ret = true;
 }
 else
  qDebug() << "update failed: " << commandText;
 return ret;
}
#if 0
int SQL::updateRouteDate(int segmentId, QString startDate, QString endDate)
{
 QDate dateStart = QDate::fromString(startDate, "yyyy/MM/dd");
 if(dateStart.isNull() || !dateStart.isValid())
  throw IllegalArgumentException(tr("invalid start date '%1'").arg(startDate));
 QDate dateEnd = QDate::fromString(endDate, "yyyy/MM/dd");
 if(dateEnd.isNull() || !dateEnd.isValid() || dateEnd < dateStart)
    throw IllegalArgumentException(tr("invalid end date '%1'").arg(endDate));

 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "Update Routes set startDate = '" + startDate
             + "', endDate='" + endDate+ "',"
             "lastUpdate=CURRENT_TIMESTAMP"
             " where lineKey =" +QString::number(segmentId);
 QSqlQuery query = QSqlQuery(db);
 if(!query.exec(commandText))\
 {
  SQLERROR(std::move(query));
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

 if(!QDate::fromString(startDate, "yyyy/MM/dd").isValid() || !QDate::fromString(endDate, "yyyy/MM/dd").isValid())
     throw IllegalArgumentException("invalid dates");

 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "Update Routes set startDate = '" + startDate
             + "', endDate='" + endDate+ "'"
             + ", linekey = " +QString::number(newSegmentId)
             + ",lastUpdate=CURRENT_TIMESTAMP"
             + " whe're lineKey =" +QString::number(segmentId);
 QSqlQuery query = QSqlQuery(db);
 if(!query.exec(commandText))\
 {
  SQLERROR(std::move(query));
  return -1;
 }
 return query.numRowsAffected();
}
#endif
QStringList SQL::showDatabases(QString connection, QString servertype)
{
 QStringList ret;
 QSqlDatabase db = QSqlDatabase::database(connection);

 if(servertype != "MsSql")
 {
  QString commandText = "show databases";
  if(servertype == "PostgreSQL")
      commandText = "SELECT datname FROM pg_database WHERE datistemplate = false";
  QSqlQuery query = QSqlQuery(db);
  if(!db.open())
  {
      qInfo() << "showDatabases:" << displayDbInfo(db);
      return QStringList();
  }
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      //throw SQLException(error.text() + " " + errCommand);
      return ret;
  }
  while(query.next())
  {
   ret.append(query.value(0).toString());
  }
 }
 return ret;
}
QStringList SQL::showPostgreSQLDatabases(QSqlDatabase db)
{
 QStringList ret;

  QString commandText = "SELECT datname FROM pg_database WHERE datistemplate = false";
  QSqlQuery query = QSqlQuery(db);
  if(!db.open())
  {
      qInfo() << "showDatabases:" << displayDbInfo(db);
      return QStringList();
  }
  bool bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      //throw SQLException(error.text() + " " + errCommand);
      return ret;
  }
  while(query.next())
  {
   QString value = query.value(0).toString();
   if(value != "postgres")
    ret.append(value);
  }

 return ret;
}


#if 0
bool SQL::loadSqlite3Functions(QSqlDatabase db)
{
 bool ret=false;
  //QSqlDatabase db = QSqlDatabase::database();
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
     if(err.text().contains("not authorized"))
     {
      // load_extension was found!; the "error" is expected because no valid parameters were supplied
      bLoadExtensionEnabled = false;
     }
     else
     {
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
  if(db.driverName() == "QSQLITE" || db.driverName() == "QSQLITE3")
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
    if(err.text().contains("not authorized"))
     return true; // error was expected and is ok!
    SQLERROR(query);
    QMessageBox::information(NULL, "open sqlite", QString("error loading functions = %1").arg(err.text()));

    return ret;
   }
   ret=true;

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
#endif
 return ret;
}
#else
#ifndef NO_UDF

void distanceFunc(sqlite3_context* ctx, int argc, sqlite3_value **argv)
{
 double Lat1, Lon1, Lat2, Lon2;
Lat1 = sqlite3_value_double(argv[0]);
Lon1 = sqlite3_value_double(argv[1]);
Lat2 = sqlite3_value_double(argv[2]);
Lon2 = sqlite3_value_double(argv[3]);

//  sqlite3_result_value(distance(LatLng(a1,a2),LatLng(a3,a4)));
  if (Lat1 == Lat2 && Lon1 == Lon2)
      sqlite3_result_double(ctx,0);
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
  sqlite3_result_double(ctx,d); // distance in kilometers
}

bool SQL::loadSqlite3Functions(QSqlDatabase db)
{
 QVariant v = db.driver()->handle();
 sqlite3 *db_handle = NULL;
 if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0)
 {
  // v.data() returns a pointer to the handle
  db_handle = *static_cast<sqlite3 **>(v.data());
  if (!db_handle) {
   qCritical() <<"Cannot get a sqlite3 handler.";
   return false;
  }
  sqlite3_initialize();
  if(sqlite3_create_function(db_handle, "distance", 4, SQLITE_ANY, 0, &distanceFunc, 0, 0))
  {
   qCritical() << "Cannot create SQLite functions: sqlite3_create_function failed.";
   return false;
  }
  return true;
 }
 qCritical() << "Cannot get a sqlite3 handle to the driver.";
 return false;
}
#endif
#endif

bool SQL::checkSegments()
{
 QSqlDatabase db = QSqlDatabase::database();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;
 QMap<int, SegmentInfo> myArray;
 SegmentInfo si;
 if(SQL::instance()->isTransactionActive())
 {
     QMessageBox::critical(nullptr, tr("Transaction active"), tr("A transaction is active: %1")
                           .arg(currentTransaction));
     return false;
 }
 myArray = getSegmentInfoList();
 foreach(SegmentInfo si, myArray.values())
 {
     qApp->processEvents();
     if(si.streetId() < 0)
     {
         QList<StreetInfo> list = StreetsTableModel::instance()->getStreetInfoList(si.streetName());
         if(list.count() == 1)
         {
             si._bNeedsUpdate = true;
             StreetInfo sti = list.at(0);
             si._streetId = sti.streetId;
             if(!sti.newerName.isEmpty())
                si._newerStreetName = sti.newerName;
             if(!si.location().isEmpty() && !sti.location.isEmpty())
             {
                 sti.location = si.location();
                 StreetsTableModel::instance()->updateStreetName(sti);
             }
             if(!sti.location.isEmpty())
                 si._location = sti.location;
             if(!sti.segments.contains(si.segmentId()))
             {
                 sti.segments.append(si.segmentId());
                 sti.updateSegmentInfo(si);
                 StreetsTableModel::instance()->updateStreetName(sti);
             }
         }
     }
  if(si._bNeedsUpdate)
  {
      if(si.needsUpdate())
      {
          if(updateSegmentDates(&si))
          {
              si.setNeedsUpdate(false);
              continue;
          }
      }
   updateSegment(&si);
  }
 }

 return scanRoutes(MainWindow::instance()->routeList);
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
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  while(query.next())
  {
   QString col = query.value("name").toString();
   if(col.compare(column, Qt::CaseInsensitive)==0)
    return true;
  }
  return false;
 }
 else if(config->currConnection->servertype() == "MySql")
 {
  int count;
  commandText = "Select count(*) from information_schema.COLUMNS"
                " where table_schema ='" + config->currConnection->database()
                + "' and table_name = '" + table +"' and column_name = '" + column + "'";
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
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
  //commandText = "Select count(*) from information_schema.COLUMNS where table_schema ='dbo' and table_name = '" + table +"' and column_name = '" + column + "'";
  commandText = "select col_length('" + table + "','" +column +"')";
 }
 else // PostgreSQL
 {
      commandText = QString("Select count(*) from INFORMATION_SCHEMA.COLUMNS"
                            " where TABLE_NAME='%1' and COLUMN_NAME='%2'")
              .arg(table.toLower(),column.toLower());
 }
 int count;
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
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

bool SQL::doesConstraintExist(QString tbName, QString name)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;

 if(config->currConnection->servertype() == "Sqlite")
 {
  commandText = QString("select sql from sqlite_master where name = '%1'").arg(tbName);
  bQuery = query.exec(commandText);
  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
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

bool SQL::addColumn(QString tbName, QString name, QString type, QString after)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText;
 bool bQuery;

 if(config->currConnection->servertype() == "Sqlite" )
  commandText = "alter table '" + tbName + "' add column  '" + name + "' " + type +" ";
 else if(config->currConnection->servertype() == "MySql" )
 {
  commandText = "alter table " + tbName + " add column " + name + " " + type +"";
  if(!after.isEmpty())
    commandText.append(" after `" + after + "`");
 }
 else  if(config->currConnection->servertype() == "PostgreSQL" )
     commandText = "alter table " + tbName + " add column  " + name + " " + type +" ";
 else
  commandText = "alter table dbo." + tbName + " add " + name + " " + type +" ";
 bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
        commandText = "select count(*) from TractionTypes where tractionType = "
                      + QString("%1").arg(tractionType);
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
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
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
            }
            rows = query.numRowsAffected();
            //if (rows == 1)
                ret = true;
        }
        else
        {
            commandText = "update TractionTypes set description = '" + description + "', "
                          "displayColor = '" + displayColor + "', routeType=" + QString("%1").arg(routeType)  + ", "
                          "lastUpdate=CURRENT_TIMESTAMP "
                          "where tractionType = " + QString("%1").arg(tractionType) ;
            // query.prepare(commandText);
            qDebug()<<commandText;
            bQuery = query.exec(commandText);
            if(!bQuery)
            {
                QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
                qDebug() << errCommand;
                QSqlError error = query.lastError();
                SQLERROR(std::move(query));
                throw SQLException(error.text() + " " + errCommand);
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

// check tables to see if alterations need to be made
void SQL::checkTables(QSqlDatabase db)
{
 QDir rsrc(":/sql");
 QList<QFileInfo> sqlfiles = rsrc.entryInfoList();
 // check for presence of Parameters table.
 QStringList tableList;
 if(db.isOpen())
 {
  qDebug() << db.driverName() + "\n";
  qDebug() << "User:" + db.userName() + "\n";
  qDebug() << "Host:" + db.hostName() + "\n";
  qDebug() << "Connection name: " + db.connectionName() + "\n";
  qDebug() << "DSN:" + db.databaseName() + "\n";

  setForeignKeyCheck(0);
  QList<FKInfo> fkList;
  //if(config->currConnection->servertype() == "Sqlite")
   fkList = getForeignKeyInfo(db,config->currConnection);

  //tableList = getTableList(db, config->currConnection->servertype());
  tableList = db.tables();

  if(!doesColumnExist("Companies", "RoutePrefix"))
  {
   addColumn("Companies", "RoutePrefix", "varchar(10) NOT NULL default ''");
   if(config->currConnection->servertype() == "Sqlite")
    executeScript(":/sql/Sqlite3/sqlite3_recreateSegmentsTable.sql",db);
  }

  if(!doesColumnExist("Companies", "info"))
  {
   addColumn("Companies", "info", "varchar(50) NOT NULL default ''", "Description");
  }
  if(!doesColumnExist("Companies", "selected"))
  {
   addColumn("Companies", "selected", "int", "Description");
  }


  if(!doesColumnExist("Companies", "Mnemonic"))
  {
      if(config->currConnection->servertype() == "MySql")
          addColumn("Companies", "Mnemonic", "varchar(10) NOT NULL default '' ", "`key`");
      else
          addColumn("Companies", "Mnemonic", "varchar(10) NOT NULL default ''");
      // if(config->currConnection->servertype() == "Sqlite")
      //  executeScript(":/sql/sqlite3_recreateCompanies.sql",db);
  }
  if(!doesColumnExist("Companies", "Url"))
  {
      if(config->currConnection->servertype() == "MySql")
          addColumn("Companies", "Url", "varchar(100) NOT NULL default '' ", "`key`");
      else
          addColumn("Companies", "Url", "varchar(100) NOT NULL default ''");
      // if(config->currConnection->servertype() == "Sqlite")
      //  executeScript(":/sql/sqlite3_recreateCompanies.sql",db);
  }
  if(!doesColumnExist("Segments", "DoubleDate"))
  {
     addColumn("Segments", "DoubleDate", "date NOT NULL DEFAULT '2000-01-01'");
     // if(config->currConnection->servertype() == "Sqlite")
     //  executeScript(":/sql/sqlite3_recreateSegmentsTable.sql",db);
  }

  if(!doesColumnExist("Segments", "FormatOK"))
  {
     addColumn("Segments", "FormatOK", "int(1) NOT NULL DEFAULT FALSE");
     // if(config->currConnection->servertype() == "Sqlite")
     //  executeScript(":/sql/sqlite3_recreateSegmentsTable.sql",db);
  }
  if(!doesColumnExist("Segments", "NewerName"))
  {
      addColumn("Segments", "NewerName", "text NOT NULL Default ''");
      // if(config->currConnection->servertype() == "Sqlite")
      //  executeScript(":/sql/sqlite3_recreateSegmentsTable.sql",db);
  }

  if(!doesColumnExist("Segments", "StreetId"))
  {
      addColumn("Segments", "StreetId", "integer NOT NULL Default -1");
      // if(config->currConnection->servertype() == "Sqlite")
      //  executeScript(":/sql/sqlite3_recreateSegmentsTable.sql",db);
  }
  if(!tableList.contains("StreetDef",Qt::CaseInsensitive))
  {
      if(config->currConnection->servertype() == "Sqlite")
       executeScript(":/sql/Sqlite3/sqlite3_create_streetdef.sql",db);
  }
  if(!doesColumnExist("Routes", "routeId"))
  {
    addColumn("Routes", "routeId", "int(11) NOT NULL DEFAULT -1", "Name");
    //executeScript(":/sql/Sqlite3/sqlite3_recreate_routes.sql",db);
  }
  if(config->currConnection->servertype() == "PostgreSQL")
  {
   // PostgreSQL does not support 'rowid' as in Sqlite. So
   // export must not export rowids so we have to create a pseudo rowid in some tables
   // that use 'rowid' in queries. Export creates the table without a column named 'rowid'
   // so we add one here after the tables are complete.
   if(!doesColumnExist("routes", "rowid"))
   {
       addColumn("routes", "rowid", "integer GENERATED ALWAYS AS IDENTITY");
   }
   if(!doesColumnExist("segments", "rowid"))
   {
       addColumn("segments", "rowid", "integer GENERATED ALWAYS AS IDENTITY");
   }
   if(!doesColumnExist("streetdef", "rowid"))
   {
       addColumn("streetdef", "rowid", "integer GENERATED ALWAYS AS IDENTITY");
   }
   if(!executeScript(":/sql/create_routeView.sql", db))
       exit(EXIT_FAILURE);
   updateIdentitySequence("streetdef", "streetid");
   updateIdentitySequence("segments", "segmentid");
   updateIdentitySequence("companies", "key");
   updateIdentitySequence("comments", "commentkey");
   updateIdentitySequence("stations", "stationkey");
   updateIdentitySequence("routename", "routeid");

  }
  // if(doesColumnExist("routename", "startDate"))
  // {
  //     if(!executeScript(":/sql/create_routeView_x.sql", db))
  //         exit(EXIT_FAILURE);

  //     executeCommand("begin");
  //     if(!executeScript(":/sql/create_routeName.sql", db))
  //        exit(EXIT_FAILURE);
  //     if(!populateRouteId())
  //         exit(EXIT_FAILURE);
  //     if(!executeScript(":/sql/create_routeView.sql", db))
  //         exit(EXIT_FAILURE);
  //   executeCommand("commit");
  // }

  if(!tableList.contains("RouteSeq", Qt::CaseInsensitive))
  {
   ExportSql* esql = new ExportSql(config,false);
   esql->createRouteSeqTable(db, config->currConnection->servertype());
  }
  if(!doesColumnExist("Segments", "pointArray"))
  {
   addColumn("Segments", "pointArray", "text");
  }
  if(!doesColumnExist("Segments", "street"))
  {
   addColumn("Segments", "street", "text");
  }

  if(!doesColumnExist("Segments", "Location"))
  {
   addColumn("Segments", "Location", "varchar(30) not null default ''", "Tracks");
   if(config->currConnection->servertype() == "Sqlite")
    executeScript(":/sql/Sqlite3/sqlite3_recreateSegmentsTable.sql",db);
   else
   {
    if(config->currConnection->servertype() != "MySql")
      QMessageBox::information(nullptr, tr("Column added"),
                               tr("one or more columns have been added to the Segments table at the end.\n"
                                  " You may use a graphical interface to reorder the column."));\
   }
  }

  if(!doesColumnExist("Stations", "routes"))
  {
   addColumn("Stations", "routes", "varchar(50)");
   addColumn("Stations", "segments", "varchar(50)");
   if(config->currConnection->servertype() == "MsSql")
    executeScript(":/sql/MsSql/mssql_recreateStationTable.sql",db);
   else
    executeScript(":/sql/recreateStationTable.sql",db); // sqlite & mysql
  }


//   if(!doesColumnExist("Routes", "OneWay"))
//   {
//    addColumn("Routes", "OneWay", "char(1) default 'Y'");
//    executeScript(":/sql/updateOneWay.sql",db);
//   }

//   if(!doesColumnExist("Routes", "TrackUsage"))
//   {
//    if(config->currConnection->servertype() == "Sqlite" )
//    {
//     addColumn("Routes", "TrackUsage", " text check(`TrackUsage` in ('B', 'L', 'R', ' ')) default ' ' NOT NULL");
//     executeScript(":/sql/sqlite3_recreate_routes.sql",db);
//    }
//    else if(config->currConnection->servertype() == "MySql")
//    {
//     addColumn("Routes", "TrackUsage", "ENUM('N', 'B', 'R')");
//     executeScript(":/sql/mysql_recreate_routes.sql");
//    }
//    // TODO: add Sql Server syntax
//    //executeScript(":/sql/mssql_recreate_routes.sql");
//   }

//   if(!doesColumnExist("Routes", "Sequence"))
//   {
//    if(addColumn("Routes", "Sequence", "int(11) NOT NULL DEFAULT -1", "ReverseLeave"))
//     if(addColumn("Routes", "ReverseSeq", "int(11) NOT NULL DEFAULT -1", "Sequence"))
//     {
// //     if(config->currConnection->servertype() == "Sqlite" )
// //      executeScript(":/sql/sqlite3_recreate_routes.sql");
//     }
//   }

//   if(!doesColumnExist("Routes", "NextR"))
//   {
//    if(config->currConnection->servertype() == "Sqlite" )
//    {
//     addColumn("Routes", "NextR", "int(11) NOT NULL DEFAULT -1", "ReverseLeave");
//     addColumn("Routes", "PrevR", "int(11) NOT NULL DEFAULT -1", "NextR");
// //    executeScript(":/sql/sqlite3_recreate_routes.sql",db);
// //    executeScript(":/sql/create_routeView", db);
//    }
//    else if(config->currConnection->servertype() == "MySql")
//    {
// //    addColumn("Routes", "NextR", "int(11) NOT NULL DEFAULT -1", "ReverseLeave");
// //    addColumn("Routes", "PrevR", "int(11) NOT NULL DEFAULT -1", "NextR");

//    }
//    // TODO: add Sql Server syntax
//    //executeScript(":/sql/recreate_routes.sql");
//   }

  // if(doesColumnExist("routes", "name"))
  // {
  //     if(executeScript(":/sql/sqlite3_recreate_routes.sql"))
  //     {
  //         executeCommand("commit");
  //     }
  // }

  if(!doesColumnExist("Parameters", "abbreviationsList"))
  {
      addColumn("Parameters", "abbreviationsList", "varchar[200] not null default ''");
      if(config->currConnection->servertype() == "Sqlite")
         executeScript(":/sql/Sqlite3/sqlite3_recreate_parameters.sql");
  }
  QStringList routesPk = listPkColumns("Routes", config->currConnection->servertype(), db);
  if(!routesPk.contains("CompanyKey", Qt::CaseInsensitive)!=0)
  {
//   if(config->currConnection->servertype() == "Sqlite")
//    executeScript(":/sql/sqlite3_recreate_routes.sql");
//   else if(config->currConnection->servertype() == "MySql")
//    executeScript(":/sql/mysql_recreate_routes.sql");
//   else
//   {
//    // TODO: MsSql query
//   }
  }

  if(!doesColumnExist("RouteComments", "latitude"))
   executeScript(":/sql/recreateRouteComments.sql", db);

//  QStringList views = listViews();
//  if(!views.contains("RouteView", Qt::CaseInsensitive))
  if(config->currConnection->servertype() == "MsSql")
   executeScript(":/sql/MsSql/mssql_create_routeView.sql");
  else
   executeScript(":/sql/create_routeView.sql");

  bool found = false;
  foreach(FKInfo info, fkList)
  {
   if(info.name == "RouteComments" && info.table == "Comments")
    found = true;
  }
  if(!found)
  {
   if(config->currConnection->servertype() == "Sqlite")
    executeScript(":/sql/recreateRouteComments.sql");
   // export should take care of MySql & MsSql.
  }
 }
 setForeignKeyCheck(config->foreignKeyCheck());
}

bool SQL::executeCommand(QString commandString, QSqlDatabase db,  QList<QVariant> *pList)
{
 QSqlQuery query = QSqlQuery(db);
 if(!query.exec(commandString))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     //throw SQLException(error.text() + " " + errCommand);
     return false;
 }
 if(pList)
 {
     pList->clear();
     int ix =0;
     while(query.next())
     {
         pList->append(query.value(ix++));
     }
 }
 return true;
}
bool SQL::executeScript(QString path, QSqlDatabase db)
{
 QFileInfo info(path);
 qDebug() << "execute script " << path;
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  qCritical()<<file.errorString() + " '" + file.fileName()+"'";
  QMessageBox::critical(nullptr, tr("error"), tr("Error opening file %1 %2").arg(file.fileName(), file.errorString()));
  //ui->lblHelp->setText(file.errorString() + " '" + file.fileName()+"'");
  return false;
 }
 scriptName = info.fileName();
 QTextStream* in = new QTextStream(&file);
 //bool ret =processFile(in, db, false);
 bool ret= processStream(in,db);
 scriptName = "";
 return ret;
}

bool SQL::processFile(QTextStream* in, QSqlDatabase db, bool bIsInclude)
{
 QString sqltext;
 while(!in->atEnd())
 {
  QString line = in->readLine();
  if(line.startsWith("#"))
  {
   if(line.startsWith("#include", Qt::CaseInsensitive))
   {
    line.remove("\n");
    QString fn = line.mid(8).trimmed();
    QFile this_file(config->q.s_query_path+"/"+fn);
    if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
     QMessageBox::critical(nullptr,tr("Error"), "Could not load sql query text file");
     return false;
     QTextStream* in = new QTextStream(&this_file);
     // if(!processFile(in, db, true))
     if(!processStream(in, db))
             return false;
    }
   }
   else
    continue;
  }
  else {
   sqltext.append(line);
  }
  if(sqltext.contains(";"))
  {
   QSqlQuery query = QSqlQuery(db);
   if(!query.exec(sqltext))
   {
       QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
       qDebug() << errCommand;
       QSqlError error = query.lastError();
       SQLERROR(std::move(query));
       //throw SQLException(error.text() + " " + errCommand);
       return false;
   }
   sqltext="";
  }
 }
 return true;
}


bool SQL::testAltRoute()
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "select sql from sqlite_master where tbl_name = 'AltRoute'";
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 QString commandText = "select count(*) from Routes "
                       "where linekey =" + QString("%1").arg(segmentId) ;
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
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
 QString commandText;

 beginTransaction("replaceSegment");
 QList< SegmentData*> list = getRouteSegmentsBySegment(segmentId1);
 if(list.isEmpty())
 {
     rollbackTransaction("replaceSegment");
     return false;
 }
 foreach (SegmentData* sd, list) {
     if( !deleteRouteSegment(*sd))
     {
         qCritical() << tr("delete segmentdata segmentid %1 failed").arg(sd->segmentId());
         rollbackTransaction("replaceSegment");
         return false;
     }

     sd->setSegmentId(segmentId2);
     if(!doesRouteSegmentExist(*sd))
     {
         if(!addSegmentToRoute(sd))
         {
             qCritical() << tr("add segmentdata segmentid %1 failed").arg(sd->segmentId());
             rollbackTransaction("replaceSegment");
             return false;
         }
     }
     rows ++;
 }

 if(rows)
 {
  if(!deleteSegment(segmentId1))
  {
   rollbackTransaction("replaceSegment");
   return false;
  }

  int cnt = getCountOfStationsUsingSegment(segmentId1);
  if(cnt)
  {
   commandText = QString("update Stations set segmentId = %1, "
                         "lastUpdate=CURRENT_TIMESTAMP "
                         "where segmentId = %2").arg(segmentId2).arg(segmentId1);
   if(!query.exec(commandText))
   {
       QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
       qDebug() << errCommand;
       QSqlError error = query.lastError();
       SQLERROR(std::move(query));
       throw SQLException(error.text() + " " + errCommand);
   }
   if(rows != cnt)
   {
    qDebug() << "number of stations updated not equal expected" << rows << " vs " << cnt;
    rollbackTransaction("replaceSement");
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
     rollbackTransaction("replaceSegment");
     return false;
    }
   }
  }
  commitTransaction("replaceSegment");
  return true;
 }
 else
 {
  if(!deleteSegment(segmentId1))
  {
   qCritical() << tr("delete segment %1 failed").arg(segmentId1);
   rollbackTransaction("replaceSegment");
   return false;
  }
 }
 commitTransaction("replaceSegment");
 return true;
}

QList<SegmentInfo> SQL::getUnusedSegments()
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);
 QList<SegmentInfo> list;
 QString commandText = "select  distinct s.segmentid, s.description, s.tracks, s.type"
                       " from Segments s where s.segmentid not in (select linekey from Routes r) ";
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  SegmentInfo si;
  si._segmentId = query.value(0).toInt();
  si._description = query.value(1).toString();
  si._tracks = query.value(2).toInt();
  si._routeType = (RouteType)query.value(3).toInt();

  list.append(si);
 }
 return list;
}

// given a list of segments replace them with a new segment from alist.
bool SQL::replaceSegmentsInRoutes(QStringList oldSegments, QStringList newSegments, QDate ignoreDate)
{
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);

 QString oldIn = "(";
 beginTransaction("replaceSegments");
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
                "r.route, n.name, r.direction, r.next, r.prev, r.normalEnter,"
                " r.normalLeave, r.reverseEnter, r.reverseLeave, "
                "r.startDate, r.endDate, s.length, s.tracks, r.OneWay, r.TrackUsage, s.type, "
                "r.companyKey, r.tractionType, r.routeId "
                "from Routes r "
                "join Segments s on s.segmentId = r.lineKey "
                "join RouteName n on r.routeid = n.routeid "
                "where r.linekey in " +oldIn;
  if(!query.exec(commandText))
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  QList<SegmentData> list;
  while(query.next())
  {
   SegmentData sd;
   //sd.setStartLatLng(LatLng(query.value(0).toDouble(), query.value(1).toDouble()));
   //sd.setEndLatLng(LatLng(query.value(2).toDouble(), query.value(3).toDouble()));
   sd._startLat = query.value(0).toDouble();
   sd._startLon = query.value(1).toDouble();
   sd._endLat = query.value(2).toDouble();
   sd._endLon = query.value(3).toDouble();
   sd._segmentId = query.value(4).toInt();
   sd._description = query.value(5).toString();
   sd._route = query.value(6).toInt();
   sd._routeName = query.value(7).toString();
   sd._direction = query.value(8).toString();
   sd._next = query.value(9).toInt();
   sd._prev = query.value(10).toInt();
   sd._normalEnter = query.value(11).toInt();
   sd._normalLeave = query.value(12).toInt();
   sd._reverseEnter = query.value(13).toInt();
   sd._reverseLeave = query.value(14).toInt();
   sd._dateBegin = query.value(15).toDate();
   sd._dateEnd = query.value(16).toDate();
   sd._length = query.value(17).toInt();
   sd._tracks = query.value(18).toInt();
   sd._oneWay = query.value(19).toString();
   sd._trackUsage = query.value(20).toString();
   sd._routeType = (RouteType)query.value(21).toInt();
   sd._companyKey = query.value(22).toInt();
   sd._tractionType =query.value(23).toInt();
   sd._bearing = Bearing(sd.startLatLng(), sd.endLatLng());
   sd._routeId = query.value(24).toInt();
   list.append(sd);
  }

  foreach(SegmentData sd, list)
  {
   if(sd._dateBegin > ignoreDate)
   {
    QString err =tr("ignoring route %1, name %2, segment %3, %4, startDate = %5").arg(sd._route).arg(sd._description)
      .arg(sd._segmentId).arg(sd._routeName).arg(sd._dateBegin.toString("yyyy/MM/dd"));
    emit details(err);
    continue;
   }
   if(!SQL::deleteRouteSegment(sd._route,sd._routeId, sd._segmentId,sd._dateBegin.toString("yyyy/MM/dd"),sd._dateEnd.toString("yyyy/MM/dd")))
   {
    rollbackTransaction("replaceSegments");
    throw RecordNotFoundException(tr("Unable to delete %1 %2 %3 %4 segment: %5").arg(sd._route).arg(sd._routeName).arg(sd._dateBegin.toString("yyyy/MM/dd")).arg(sd._dateEnd.toString("yyyy/MM/dd")).arg(sd._segmentId));
   }
   foreach(QString segment, newSegments)
   {
    bool ok;
    SegmentData newRd = SegmentData(sd);
    //newRd.sd = SegmentInfo(SQL::getSegmentInfo(segment.toInt(&ok)));
    newRd._segmentId = segment.toInt(&ok);
    double diffBearing = sd._bearing.angle() - newRd._bearing.angle();
//    if(newRd._tracks == 2)
//    {
//     newRd.oneWay = "Y";
//     if(diffBearing > 45.0)
//      newRd.trackUsage = "L";
//     else
//      newRd.trackUsage = "R";
//    }
    if(SQL::doesRouteSegmentExist(newRd._route, newRd._routeName, newRd._segmentId,
                                  newRd._dateBegin, newRd._dateEnd))
     continue;


    if(!insertRouteSegment(newRd))
    {
     rollbackTransaction("replaceSegments");
     throw Exception("failed:" + commandText);
    }
    QString detail;
    detail = detail.append("route ").append(QString("%1").arg(sd._route)).append(" ").append(sd._routeName).append(", Segment ")
      .append(QString("%1").arg(sd._segmentId)).append(" ").append(sd._routeName).append(" replaced with ")
      .append(QString("%1").arg(newRd._segmentId)).append(" ").append(newRd._description);
    qDebug() << detail;
    emit details(detail);

   }
 }
 commitTransaction("replaceSegments");
 return true;
}

QList<FKInfo> SQL::getForeignKeyInfo(QSqlDatabase db, Connection* c, QString table)
{
 QList<FKInfo> list;
 //QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);

 QString commandText;
 if(c->servertype() == "Sqlite")
 {
  if(table.isEmpty())
   commandText=  "SELECT m.name, p.* FROM sqlite_master m JOIN pragma_foreign_key_list(m.name) p ON m.name != p.'table' "
                         "WHERE m.type = 'table' ORDER BY m.name";
  else
   commandText=  "SELECT m.name, p.* FROM sqlite_master m JOIN pragma_foreign_key_list(m.name) p ON m.name != p.'table' "
                         "WHERE m.type = 'table' and m.name = '" + table + "' ORDER BY m.name";
 }
 else if(c->servertype() == "MySql")
 {
  commandText = "SELECT "
                "TABLE_NAME,COLUMN_NAME,CONSTRAINT_NAME, REFERENCED_TABLE_NAME,REFERENCED_COLUMN_NAME "
                "FROM "
                "INFORMATION_SCHEMA.KEY_COLUMN_USAGE ";

  if(!table.isEmpty())
  {
   commandText.append(" Where TABLE_NAME = '" + table + "' "
                      "and REFERENCED_TABLE_SCHEMA = '" + c->defaultSqlDatabase() + "'");
  }
  else
  {
   commandText.append(" Where REFERENCED_TABLE_SCHEMA = '" + c->defaultSqlDatabase() + "'");
  }
 }
 else if(c->servertype() == "MsSql")
 {
  // MsSql
     commandText = "SELECT  obj.name AS FK_NAME, "
        "     sch.name AS [schema_name], "
        "     tab1.name AS [table], "
        "     col1.name AS [column], "
        "     tab2.name AS [referenced_table], "
        "     col2.name AS [referenced_column]"
        " FROM sys.foreign_key_columns fkc "
        " INNER JOIN sys.objects obj "
        "     ON obj.object_id = fkc.constraint_object_id"
        " INNER JOIN sys.tables tab1"
        "     ON tab1.object_id = fkc.parent_object_id"
        " INNER JOIN sys.schemas sch"
        "     ON tab1.schema_id = sch.schema_id"
        " INNER JOIN sys.columns col1"
        "     ON col1.column_id = parent_column_id AND col1.object_id = tab1.object_id"
        " INNER JOIN sys.tables tab2"
        "     ON tab2.object_id = fkc.referenced_object_id"
        " INNER JOIN sys.columns col2"
        "     ON col2.column_id = referenced_column_id AND col2.object_id = tab2.object_id";
 }
 else
 {
    commandText = "SELECT\
         tc.table_schema,\
         tc.constraint_name,\
         tc.table_name,\
         kcu.column_name,\
         ccu.table_name AS foreign_table_name,\
         ccu.column_name AS foreign_column_name\
     FROM information_schema.table_constraints AS tc\
     JOIN information_schema.key_column_usage AS kcu\
         ON tc.constraint_name = kcu.constraint_name\
         AND tc.table_schema = kcu.table_schema\
     JOIN information_schema.constraint_column_usage AS ccu\
         ON ccu.constraint_name = tc.constraint_name\
     WHERE tc.constraint_type = 'FOREIGN KEY'\
         AND tc.table_schema='public'\
         AND tc.table_name='" + table +"'";
 }
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 int seq = 0;
 while(query.next())
 {
  FKInfo info;
  if(c->servertype() == "Sqlite")
  {
   info.name = query.value(0).toString();
   info.id = query.value(1).toInt();
   info.seq = query.value(2).toInt();
   info.table = query.value(3).toString();
   info.from = query.value(4).toString();
   info.to = query.value(5).toString();
   info.on_update = query.value(6).toString();
   info.on_delete = query.value(7).toString();
   info.match = query.value(8).toString();
  }
  else if(c->servertype() == "MySql")
  {
   info.name = query.value("TABLE_NAME").toString();
   info.table = query.value("REFERENCED_TABLE_NAME").toString();
   info.from = query.value("COLUMN_NAME").toString();
   info.to = query.value("REFERENCED_COLUMN_NAME").toString();
   QString cn = query.value("CONSTRAINT_NAME").toString();
   info.id = cn.mid(cn.indexOf("ibfk_")+5).toInt();
  }
  else if(c->servertype() == "MsSql")
  {
   info.name = query.value("table").toString();
   info.table = query.value("referenced_table").toString();
   info.from = query.value("column").toString();
   info.to = query.value("referenced_column").toString();
   info.seq = seq++;
   if(!table.isEmpty())
   {
    if(info.name != table)
     continue;
   }
  }
  list.append(info);
 }
 return list;
}

QMap<int,RouteName*>* SQL::routeNameList()
{
 QMap<int, RouteName*>* list = new QMap<int, RouteName*>();
 QSqlDatabase db = QSqlDatabase();
 QSqlQuery query = QSqlQuery(db);

 QString commandText =  "SELECT route, routePrefix, routeAlpha, baseRoute FROM AltRoute";
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  RouteName* rn  = new RouteName;
  rn->setRoute(query.value(0).toInt());
  rn->setBaseRoute(query.value(3).toInt());
  rn->setRoutePrefix(query.value(1).toString());
  rn->setRouteAlpha(query.value(2).toString());
  list->insert(rn->route(), rn);
 }
 return list;
}

QString SQL::getDatabase(QString serverType, QSqlDatabase db)
{
 //QSqlDatabase db = QSqlDatabase();

 QSqlQuery query = QSqlQuery(db);
 QString dbName ="";
 QString commandText;
 if(serverType == "MsSql")
  commandText = "SELECT DB_NAME() ";
 else if(serverType == "MySql")
  commandText = "SELECT DATABASE()";
 else
  return dbName;

 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 else
 {
  while(query.next())
  {
   if(!query.value(0).isNull())
    dbName = query.value(0).toString();
  }
 }
 return dbName;
}

bool SQL::useDatabase(QString dbName, QString serverType, QSqlDatabase db)
{
    if(serverType == "PostgreSQL")
        return true;
    QString currentDb = getDatabase(serverType, db);
    if(currentDb == dbName)
        return true;
    QSqlQuery query = QSqlQuery(db);
    QString commandText = "use " +dbName;
    if(!query.exec(commandText))
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    return true;
}

bool SQL::createSqlDatabase(QString dbName, QSqlDatabase db, QString dbType)
{
 if(!db.isOpen())
  return false;
 QSqlQuery query = QSqlQuery(db);

 QString commandText;
 if(dbType == "MySql")
  commandText = "CREATE DATABASE IF NOT EXISTS " + dbName;
 else
  commandText = "CREATE DATABASE " + dbName;

 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
}



QStringList SQL::showMySqlDatabases(QSqlDatabase db)
{
 QStringList list;
 if(!db.isOpen())
  return QStringList();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "show databases";
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 QStringList excludes = {"mysql", "information_schema", "performance_schema", "phpmyadmin", "sys"};
 while(query.next())
 {
  if(!excludes.contains(query.value(0).toString()))
   list.append(query.value(0).toString());
 }
 return list;
}

QStringList SQL::showMsSqlDatabases(QSqlDatabase db)
{
 QStringList list;
 if(!db.isOpen())
  return QStringList();
 QSqlQuery query = QSqlQuery(db);
 QString commandText = "select name from sys.Databases WHERE name NOT IN ('master', 'tempdb', 'model', 'msdb')";
 if(!query.exec(commandText))
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
   list.append(query.value(0).toString());
 }
 return list;
}

// create a duplicate segment with a different number of tracks
SegmentInfo SQL::convertSegment(int segmentId, int tracks)
{
 SegmentInfo si = getSegmentInfo(segmentId);
 if(si.segmentId()< 1)
  return si;

 // see if another segment exists with the same starting and ending points.
 SegmentInfo si1;
 try
 {
  if(!dbOpen())
      throw Exception(tr("database not open: %1").arg(__LINE__));
  QSqlDatabase db = QSqlDatabase::database();
  QString commandText;
  QString distanceWhere;
#ifndef NO_UDF
  distanceWhere = " and distance(startLat, startLon, "
    + QString("%1").arg(si.startLat()) + ", " + QString("%1").arg(si.startLon())+ ") < .020 and "
    + " distance(endLat, endLon, "
    + QString("%1").arg(si.endLat()) + ", " + QString("%1").arg(si.endLon()) + ")";
#endif
  if(config->currConnection->servertype() != "MsSql")
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, pointArray from Segments"
                     " where tracks = " + QString("%1").arg(tracks)
                     +  distanceWhere;
  else
       commandText = "Select `SegmentId`, Description, tracks, type,"
                     " StartLat, StartLon, EndLat, EndLon, length, StartDate, EndDate, Direction,"
                     " Street, pointArray from Segments"
                     " where tracks = " + QString("%1").arg(tracks)
                     +  distanceWhere;
  QSqlQuery query = QSqlQuery(db);
  bool bQuery = query.exec(commandText);
  qDebug() << query.lastQuery() << " line:" <<__LINE__;

  if(!bQuery)
  {
      QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
      qDebug() << errCommand;
      QSqlError error = query.lastError();
      SQLERROR(std::move(query));
      throw SQLException(error.text() + " " + errCommand);
  }
  if (!query.isActive())
  {
    // no dup segment exists
   si._segmentId = -1;
   si._tracks = tracks;
   bool alreadyExists = false;
   int newSegmentId = addSegment(si, &alreadyExists,false);
   si._segmentId = newSegmentId;
      return si;
  }
  while (query.next())
  {
   si1._segmentId = query.value(0).toInt();
   si1._description = query.value(1).toString();
   si1._tracks = query.value(2). toInt();
   si1._routeType = (RouteType)query.value(3).toInt();
   si1._startLat = query.value(4).toDouble();
   si1._startLon = query.value(5).toDouble();
   si1._endLat = query.value(6).toDouble();
   si1._endLon = query.value(7).toDouble();
   si1._length = query.value(8).toDouble();
   si1._dateBegin = query.value(9).toDate();
   si1._dateEnd = query.value(10).toDate();
   si1._direction = query.value(11).toString();
   si1._streetName = query.value(12).toString();
   si1.setPoints(query.value(13).toString());  // array of points

   if((si1._startLat ==0 ||si1._startLat ==0 || si1._endLat == 0 || si1._endLon ==0) && si1._pointList.count() > 1 )
   {
    si1._startLat = si1._pointList.at(0).lat();
    si1._startLon = si1._pointList.at(0).lon();
    si1._endLat = si1._pointList.at(si1._points-1).lat();
    si1._endLon = si1._pointList.at(si1._points-1).lon();
   }
   si1._bearing = Bearing(si1._startLat, si1._startLon, si1._endLat, si1._endLon);
   if(si1._pointList.count() > 1)
   {
    si1._bearingStart = Bearing(si1._startLat, si1._startLon, si1._pointList.at(1).lat(), si1._pointList.at(1).lon());
    si1._bearingEnd = Bearing(si1._pointList.at(si1._points-2).lat(), si1._pointList.at(si1._points-2).lon(), si1._endLat, si1._endLon);
   }
   si1._bounds = Bounds(query.value(13).toString());
   if(si1._length == 0 && si1._pointList.count() > 1)
   {
    //sd._length = distance(LatLng(sd._startLat, sd._startLon), LatLng(sd._endLat, sd._endLon));
    for(int i=0; i <  si1._points-2; i++)
    si1._length += distance(si1._pointList.at(i), si1._pointList.at(i+1));
   }
  }
 }
 catch (Exception& e)
 {
    myExceptionHandler(e);
 }
 if(si1.segmentId() < 0)
 {
     si._segmentId = -1;
     bool alreadyExists = false;
     si._tracks = tracks;
     int newSegmentId = addSegment(si, &alreadyExists,true);
     si._segmentId = newSegmentId;
        return si;
 }
 return si1; // already exists

}

int SQL::nextRouteNumberInRange(int lowRange, int highRange){
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "select max(route) from AltRoute where route >= "
   + QString::number(lowRange) + " and route < " +QString::number(highRange);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 qDebug() << query.lastQuery() << " line:" <<__LINE__;

 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  int rslt = query.value(0).toUInt();
  if(rslt == 0)
   return lowRange;
  return rslt;
 }
 return lowRange;
}

bool SQL::renumberRoute(QString oldAlphaRoute, int newRoute, QString routePrefix)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 QSqlQuery query = QSqlQuery(db);
 //int oldRouteNumber = -1;

 beginTransaction("renumber");

 try
 {

  QList<SegmentData> routes = getRouteSegmentsForRouteNbr(oldAlphaRoute);
  if(routes.isEmpty())
  {
   rollbackTransaction("renumber");
   return false;
  }
  foreach(SegmentData sd, routes)
  {
   if(!deleteRoute(sd))
   {
    rollbackTransaction("renumber");
    return false;
   }
  }

  bool rslt = deleteAlphaRoute(oldAlphaRoute);
  if(!rslt)
  {
   rollbackTransaction("renumber");
   return false;
  }
  rslt = SQL::addAltRoute(newRoute, QString::number(newRoute), routePrefix);
  if(!rslt)
  {
   rollbackTransaction("renumber");
   return false;
  }

  foreach(SegmentData sd, routes)
  {
   int oldRouteNumber = sd._route;
   sd._route = newRoute;
   if(!insertRouteSegment(sd))
   {
    rollbackTransaction("renumber");
    return false;
   }

   QList<RouteComments> comments = commentsForRoute(oldRouteNumber);
   if(!comments.isEmpty())
   {
    foreach(RouteComments rc, comments)
    {
     if(!deleteRouteComment(rc))
     {
      rollbackTransaction("renumber");
      return false;
     }
     rc.route = newRoute;
     if(!updateRouteComment(rc))
     {
      rollbackTransaction("renumber");
      return false;
     }
    }
   }
   QList<TerminalInfo> terminals = terminalsForRoute(oldRouteNumber);
   if(!terminals.isEmpty())
   {
    foreach(TerminalInfo te, terminals)
    {
     if(!deleteTerminalInfo(oldRouteNumber))
     {
      rollbackTransaction("renumber");
      return false;
     }
     te.route = newRoute;
     if(!updateTerminals(te))
     {
      rollbackTransaction("renumber");
      return false;
     }
    }

    if(!updateRouteForStations(oldRouteNumber, newRoute))
    {
     rollbackTransaction("renumber");
     return false;
    }
   }
  }
 }
 catch(Exception ex)
 {
  rollbackTransaction("renumber");
  return false;
 }


 commitTransaction("renumber");
 return true;
}

QList<RouteComments> SQL::commentsForRoute(int route)
{

 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "select route, date, commentKey, companyKey, latitude, longitude "
                       "from RouteComments where route = " +QString::number(route);
 QSqlQuery query = QSqlQuery(db);
 QList<RouteComments>myArray;
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while (query.next())
 {
  RouteComments rc;
  rc.route = query.value(0).toInt();
  rc.date = query.value(1).toDate();
  rc.commentKey = query.value(2).toInt();
  rc.companyKey = query.value(3).toInt();
  rc.pos = LatLng(query.value(4).toDouble(), query.value(5).toDouble());
  myArray.append(rc);
 }

 return myArray;
}

QList<TerminalInfo> SQL::terminalsForRoute(int route)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "select route, name, startDate, endDate,startSegment, startWhichEnd,"
                       " endSegment, endWhichEnd"
                       " from Terminals where route = " + QString::number(route);
 QSqlQuery query = QSqlQuery(db);
 QList<TerminalInfo> myArray;
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  TerminalInfo te;
  te.route = query.value(0).toInt();
  te.startDate = query.value(1).toDate();
  te.endDate = query.value(2).toDate();
  te.startSegment = query.value(3).toInt();
  te.startWhichEnd = query.value(4).toChar();
  te.endSegment = query.value(5).toInt();
  te.endWhichEnd = query.value(6).toChar();
  myArray.append(te);
 }

 return myArray;
}

bool SQL::updateRouteForStations(int oldRoute, int newRoute)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "update Stations set route = " + QString::number(newRoute)
                       +",lastUpdate=CURRENT_TIMESTAMP  "
                        "where route = "
                       + QString::number(oldRoute);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;
}

bool SQL::deleteTerminalInfo(int route)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = "delete from terminals where route = " + QString::number(route);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 return true;

}

bool SQL::doesFunctionExist(QString name, QString serverType, QSqlDatabase db)
{
    //QSqlDatabase db = QSqlDatabase::database();
    QString commandText;
    if(serverType == "MsSql")
    commandText = QString("SELECT name,type "
                                 "FROM   sys.objects "
                                 "WHERE  object_id = OBJECT_ID(N'[master].[dbo].[%1]') "
                                        "AND type IN ( N'FN', N'IF', N'TF', N'FS', N'FT')")
            .arg(name);
    else if(serverType == "MySql")
     commandText = "show function status + name";
    else if(serverType == "SqLite")
     commandText = "select exists(select 1 from pragma_function_list where name='" + name +"')";
    else
     return false;
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    QString fName;
    QString type;
    if(serverType == "MsSql")
    {
     while(query.next())
     {
         fName = query.value(0).toString();
         type = query.value(1).toString();
         if(fName == name)
             return true;
     }
    }
    else if(serverType == "MySql")
    {
     while(query.next())
     {
      if(query.value("Name").toString() == name)
       return true;
     }
    }
    else if(serverType == "Sqlite")
    {
     while(query.next())
     {
      return query.value(0).toBool();
     }
    }

    return false;
}

QStringList SQL::listViews()
{
 QStringList list;
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 if(config->currConnection->servertype() == "MySql")
  commandText = QString("SHOW FULL TABLES IN %1 WHERE TABLE_TYPE LIKE 'VIEW'")
    .arg(config->currConnection->defaultSqlDatabase());
 else if(config->currConnection->servertype() == "Sqlite")
  commandText = "SELECT name FROM sqlite_schema WHERE type = 'view'";
 else
  commandText = "SELECT TABLE_NAME, TABLE_SCHEMA FROM INFORMATION_SCHEMA.VIEWS";
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  if(config->currConnection->servertype() == "MsSql")
   list.append(QString("[%1].[%2]").arg(query.value(1).toString()).arg(query.value(0).toString()));
  else
   list.append( query.value(0).toString());
 }
 return list;
}

QStringList SQL::listColumns(QString table, QString serverType, QSqlDatabase db, QStringList* types)
{
 QStringList columns;
 if(types)
   types->clear();
 //Connection* c = config->currConnection;
 //QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 if(serverType == "Sqlite")
  commandText = QString("pragma table_info('%1')").arg(table);
 else if(serverType == "MySql")
  commandText = "describe " + table;
 else if(serverType == "PostgreSQL")
  commandText = QString("select column_name, data_type, character_maximum_length, column_default, is_nullable\
                        from INFORMATION_SCHEMA.COLUMNS where table_name = '%1';").arg(table);
 else // SQL Server
  commandText = "EXEC sp_help " + table;
 //processALine(txt, v);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  if(serverType== "Sqlite")
  {
   columns.append(query.value("name").toString());
   if(types)
       types->append(query.value("type").toString());
  }
  else if(serverType == "MySql")
  {
   columns.append(query.value("Field").toString());
   if(types)
       types->append(query.value("Type").toString());
  }
  else if(serverType == "PostgreSQL")
  {
   //PostgreSQL extra colums start with 'specific_catalog'
   if(query.value("column_name").toString()== "rowid")
       break;
   if(query.value("column_name").toString()== "specific_catalog")
       break;

   columns.append(query.value("column_name").toString());
   if(types)
       types->append(query.value("data_type").toString());
  }
  else // SQL Server
  {
   columns.append(query.value("Name").toString());
  if(types)
      types->append(query.value("DATA_TYPE").toString());
  }
 }
 return columns;
}

QStringList SQL::listPkColumns(QString table, QString serverType, QSqlDatabase db, QStringList* types)
{
 QStringList columns;
// Connection* c = config->currConnection;
// QSqlDatabase db = QSqlDatabase::database();
 QString commandText;
 if(serverType == "Sqlite")
  commandText = QString("pragma table_info('%1')").arg(table);
 else if(serverType == "MySql")
  commandText = "describe " + table;
 else // SQL Server
 {
  //commandText = "EXEC sp_help " + table;
     commandText = "select *"
             " from INFORMATION_SCHEMA.COLUMNS"
             " where TABLE_NAME='" + table + "'";
 }

 //processALine(txt, v);
 QSqlQuery query = QSqlQuery(db);
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  if(serverType== "Sqlite")
  {
   if(query.value("pk").toUInt() > 0)
    columns.append(query.value("name").toString());
   if(types)
       types->append(query.value("type").toString());
  }
  else if(serverType == "MySql")
  {
   if(query.value("Key").toString()== "PRI")
    columns.append(query.value("Field").toString());
   if(types)
       types->append(query.value("Type").toString());
  }
  else // SQL Server
  {
   columns.append(query.value("COLUMN_NAME").toString());
   if(types)
       types->append(query.value("DATA_TYPE").toString());
  }
 }
 return columns;
}

// create SegmentData from RouteView query
QList<SegmentData*>  SQL::segmentDataListFromView(QString where)
{
 QSqlDatabase db = QSqlDatabase::database();
 QString commandText = QString("select * from RouteView %1").arg(where);
 QSqlQuery query = QSqlQuery(db);
 QList<SegmentData*> list;
 bool bQuery = query.exec(commandText);
 if(!bQuery)
 {
     QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
     qDebug() << errCommand;
     QSqlError error = query.lastError();
     SQLERROR(std::move(query));
     throw SQLException(error.text() + " " + errCommand);
 }
 while(query.next())
 {
  SegmentData* sd = new SegmentData();
  sd->_route = query.value(0).toInt();
  sd->_alphaRoute = query.value(1).toString();
  sd->_routeName = query.value(2).toString();
  sd->_segmentId = query.value(3).toInt();
  sd->_description = query.value(4).toString();
  sd->_dateBegin = query.value(5).toDate();
  sd->_dateEnd = query.value(6).toDate();
  sd->_companyKey = query.value(7).toInt();
  // company Name not used
  // tractiontype name not used
  sd->_tracks = query.value(10).toInt();
  sd->_oneWay = query.value(11).toString();
  sd->_trackUsage = query.value(12).toString();
  sd->_length = query.value(13).toDouble();
  sd->_direction = query.value(14).toString();
  sd->_routeType = (RouteType)query.value(15).toInt();
  sd->_streetName = query.value(16).toString();
  sd->_location = query.value(17).toString();
  sd->_startLat = query.value(18).toDouble();
  sd->_startLon = query.value(19).toDouble();
  sd->_endLat = query.value(20).toDouble();
  sd->_endLon = query.value(21).toDouble();
  sd->_tractionType = query.value(22).toInt();
  sd->_next = query.value(23).toInt();
  sd->_prev = query.value(24).toInt();
  sd->_nextR = query.value(25).toInt();
  sd->_prevR = query.value(26).toInt();
  sd->_normalEnter = query.value(27).toInt();
  sd->_normalLeave = query.value(28).toInt();
  sd->_reverseEnter = query.value(29).toInt();
  sd->_reverseLeave = query.value(30).toInt();
  sd->_sequence = query.value(31).toInt();
  sd->_returnSeq = query.value(32).toInt();
  sd->_points = query.value(33).toInt();
  sd->setPoints(query.value(34).toString());
  sd->_baseRoute = query.value(35).toInt();
  sd->_dateDoubled = query.value(36).toDate();
  sd->_segmentDateStart = query.value(37).toDate();
  sd->_segmentDateEnd = query.value(38).toDate();
  sd->_newerName = query.value(39).toString();
  sd->_routePrefix = query.value(40).toString();
  sd->_streetId = query.value(41).toInt();
  sd->_routeId = query.value(42).toInt();
  if(!sd->segmentStartDate().isValid() || !sd->segmentEndDate().isValid())
  {
      SegmentInfo si = SegmentInfo(*sd);
      if(!updateSegmentDates(&si))
      {
          qDebug() << "failed to update segment dates";
      }
      sd->_segmentDateStart = si.startDate();
      sd->_segmentDateEnd = si.endDate();
  }
  if(sd->_dateBegin < sd->segmentStartDate())
  {
      if(sd->_dateDoubled == sd->_segmentDateStart)
          sd->doubleDate() = sd->startDate();
      sd->_segmentDateStart = sd->startDate();
      sd->setNeedsUpdate(true);
  }
  if(sd->endDate() > sd->_segmentDateEnd)
  {
      sd->_segmentDateEnd = sd->segmentStartDate();
      sd->setNeedsUpdate(true);
  }
  list.append(sd);
 }
 return list;
}

bool SQL::getForeignKeyCheck()
{
 bool ret = false;
 int count = 0;
 try
 {
  if(config->currConnection->servertype() != "Sqlite")
   return false;
     if(!dbOpen())
         throw Exception(tr("database not open: %1").arg(__LINE__));
     QSqlDatabase db = QSqlDatabase::database();

     QString commandText = "PRAGMA foreign_keys";
     QSqlQuery query = QSqlQuery(db);
     bool bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
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

void SQL::setForeignKeyCheck(bool b)
{
 try
 {
     if(!dbOpen())
         throw Exception(tr("database not open: %1").arg(__LINE__));
     if(config->currConnection->servertype() != "Sqlite")
         return;

     QSqlDatabase db = QSqlDatabase::database();

     QString commandText = "PRAGMA foreign_keys="+ QString(b?"1":"0");
     QSqlQuery query = QSqlQuery(db);
     bool bQuery = query.exec(commandText);
     if(!bQuery)
     {
         QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
         qDebug() << errCommand;
         QSqlError error = query.lastError();
         SQLERROR(std::move(query));
         throw SQLException(error.text() + " " + errCommand);
     }
 }
 catch (Exception e)
 {
     myExceptionHandler(e);
 }
 return;
}

int SQL::displaySqlError(QSqlQuery query, QMessageBox::StandardButtons buttons, QString text, QString func, QString file, int line)
{
 QSqlError err = query.lastError();
 QString msg = "An SQL error has occurred:\n";
 if(!scriptName.isEmpty())
  msg.append(tr("The error occured processing Sql script: '%1'\n").arg(scriptName));
 msg = msg.append(tr("reported in %1 at %2 line %3")).arg(func, file).arg(line);
 msg = msg.append(tr("\n<B>Abort</B> closes the app. "));
 if(buttons  &  QMessageBox::Ignore)
 msg = msg.append(tr("<B>Ignore</b> will return true ."));
 msg = msg.append(text);
 QString details = QString("%1\n%2").arg(err.text(), query.lastQuery());
 QMessageBox box(QMessageBox::Critical, tr("Sql Error"), msg, buttons );
 box.setInformativeText(details);
 if(buttons == QMessageBox::NoButton)
 {
     box.addButton(QMessageBox::Abort);
     box.addButton(QMessageBox::Ok);
 }
 int b = box.exec();
 if(b == QMessageBox::Abort)
     exit(EXIT_FAILURE);
 errReturn = (QMessageBox::StandardButton)b;
 return b;
}

bool SQL::isCompanyValid(SegmentData sd)
{
 CompanyData* cd = getCompany(sd.companyKey());
 if(!cd)
  return false;
 if(sd.startDate() >= cd->startDate && sd.endDate()<= cd->endDate)
  return true;
 return false;
}

// Assuming QList of integers
QString SQL::list2String(const QList<int> &list)
{
    QString s = "";

    for (auto &value : list)
    {
        s += QString("%1,").arg(value);
    }

    // Chop off the last comma
    s.chop(1);

    s += "";
    return s;
}

QList<QPair<SegmentInfo, SegmentInfo> > SQL::getDupSegmentsInList(QList<SegmentInfo> list)
{
    QList<QPair<SegmentInfo, SegmentInfo>> result;
    QList<QPair<int,int>> matchedList;
    if(list.count()<2)
        return result;

    LatLng latlngStart;
    LatLng latlngEnd;
    foreach (SegmentInfo si , list) {
            latlngStart = si.pointList().at(0); // will match to start
            latlngEnd = si.pointList().at(si.pointList().count()-1);

        //  now look at the other end of each other
        LatLng latlng2Start;
        LatLng latlng2End;
        foreach (SegmentInfo si2 , list) {
            if(si.segmentId() == si2.segmentId())
                continue; // ignore ourself
            latlng2Start = si2.pointList().at(0);
            latlng2End = si2.pointList().at(si2.pointList().count()-1);
            QPair<SegmentInfo,SegmentInfo> pair(si,si2);
            int matched = 0;
            QPair<int,int> segPairs;
            if(si.segmentId() < si2.segmentId())
                segPairs = QPair<int,int>(si.segmentId(), si2.segmentId() );
            else
                segPairs = QPair<int,int>(si2.segmentId(), si.segmentId() );

            if(distance(latlngStart, latlng2Start) < .020){
                matched++;
            }
            if(distance(latlngStart, latlng2End) < .020){
                matched++;
            }

            if(distance(latlngEnd, latlng2Start) < .020){
                matched++;
            }
            if(distance(latlngEnd, latlng2End) < .020){
                matched++;
            }
            if(matched >1){
                if(!matchedList.contains(segPairs))
                {
                    matchedList.append(segPairs);
                    result.append(pair);
                }
            }
        }
    }
    return result;
}

//Get list of possible conflicting routes with same name
QList<RouteData> SQL:: checkRouteName(QString name, QDate startDate, QDate endDate)
{
   QList<RouteData> list;
   RouteData rd = RouteData();
   QString commandText;
   try
   {
       if(!dbOpen())
           throw Exception(tr("database not open: %1").arg(__LINE__));
       QSqlDatabase db = QSqlDatabase::database();
           commandText = "select distinct r.startDate, r.endDate, n.name, r.route, "
                         "r.companyKey, tractionType, "
                         "a.routeAlpha, r.routeid "
                         "from routes r "
                         "join altRoute a on r.route = a.route "
                         "join RouteName n on n.routeid = r.routeid "
                         "where n.name = '" +name + "' "
                         "and '" + startDate.toString("yyyy/MM/dd") +
                         "' between r.startDate and r.endDate "
                         "and '" + endDate.toString("yyyy/MM/dd") +
                         "' between r.startDate and r.endDate "
                         "group by r.startDate, r.enddate, n.name,r.route,"
                         "r.companykey,r.tractiontype,a.routeAlpha, r.routeId";
       QSqlQuery query = QSqlQuery(db);
       bool bQuery = query.exec(commandText);
       if(!bQuery)
       {
           QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
           qDebug() << errCommand;
           QSqlError error = query.lastError();
           SQLERROR(std::move(query));
           throw SQLException(error.text() + " " + errCommand);
       }
       if (!query.isActive())
       {
           return list;
       }
       while (query.next())
       {
           rd = RouteData();
           rd._dateBegin =query.value(0).toDate();
           rd._dateEnd = query.value(1).toDate();
           rd._name = query.value(2).toString();
           rd._route = query.value(3).toInt();
           rd._companyKey = query.value(4).toInt();
           rd._tractionType = query.value(5).toInt();
           rd._alphaRoute = query.value(6).toString();
           rd._routeId = query.value(7).toInt();
           list.append(rd);
       }
   }
   catch (Exception e)
   {
       myExceptionHandler(e);

   }
   return list;
}

// return next start or end date for route after given date
QDate SQL::getNextStartOrEndDate(int route, QDate dt, int segmentId, bool bStart)
{
    QString commandText;
    QDate date;
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        if(segmentId > 0)
        {
            if(bStart)
                commandText = "select min(startDate) from Routes"
                        " where route = " + QString::number(route)
                        + " and lineKey = " + QString::number(segmentId)
                        + " and startDate > '" + dt.toString("yyyy/MM/dd") + "'";
            else
                commandText = "select min(endDate) from Routes"
                        " where route = " + QString::number(route)
                        + " and lineKey = " + QString::number(segmentId)
                        + " and endDate > '" + dt.toString("yyyy/MM/dd") + "'"
                        + " and startDate > '" + dt.toString("yyyy/MM/dd") + "'";
        }
        else
        {
            if(bStart)
                commandText = "select min(startDate) from Routes"
                        " where route = " + QString::number(route)
                        + " and startDate > '" + dt.toString("yyyy/MM/dd") + "'";
            else
                commandText = "select min(endDate) from Routes"
                        " where route = " + QString::number(route)
                        + " and endDate > '" + dt.toString("yyyy/MM/dd") + "'"
                        + " and startDate > '" + dt.toString("yyyy/MM/dd") + "'";
        }

        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        if (!query.isActive())
        {
            return date;
        }
        while (query.next())
        {
           return query.value(0).toDate();
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);

    }   return date;
}

//return list of any segments for other routes that conflict
QList<SegmentData*> SQL::getConflictingRouteSegments(RouteData rd)
{
    QList<SegmentData*> myArray;
    QString startDate = rd.startDate().toString("yyyy/MM/dd");
    QString endDate = rd.endDate().toString("yyyy/MM/dd");
    try
    {
        if(!dbOpen())
            throw Exception(tr("database not open: %1").arg(__LINE__));
        QSqlDatabase db = QSqlDatabase::database();
        QString commandText = "Select a.route, n.name, a.startDate, a.endDate, "
                              "lineKey, tractionType, companyKey, a.direction,"
                              " normalEnter, normalLeave, reverseEnter, reverseLeave,"
                              " routeAlpha, a.OneWay, s.description, s.length, s.startDate, s.endDate"
                              " from Routes a"
                              " join AltRoute b on a.route = b.route"
                              " join Segments s on a.linekey = s.segmentid"
                              " join RouteName n on n.routeId = a.routeId"
                              " where ((a.startDate between '" + startDate + "'"
                              " and '" + endDate + "') or (a.endDate between '" + startDate + "'"
                              " and '" + endDate + "'))"
                              " and a.route = " + QString("%1").arg(rd.route()) + ""
                              //" and name = '" + name + "' and a.endDate <> '" + endDate + "'"
                              " and a.endDate <> '" + endDate + "'"
                              " and companyKey = " + QString::number(rd.companyKey());
        QSqlQuery query = QSqlQuery(db);
        bool bQuery = query.exec(commandText);
        if(!bQuery)
        {
            QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
            qDebug() << errCommand;
            QSqlError error = query.lastError();
            SQLERROR(std::move(query));
            throw SQLException(error.text() + " " + errCommand);
        }
        while (query.next())
        {
            SegmentData* sd = new SegmentData();
            sd->_route = query.value(0).toInt();
            sd->_routeName = query.value(1).toString();
            sd->_dateBegin = query.value(2).toDate();
            sd->_dateEnd = query.value(3).toDate();
            sd->_segmentId = query.value(4).toInt();
            sd->_tractionType =query.value(5).toInt();
            sd->_companyKey = query.value(6).toInt();
            sd->_direction = query.value(7).toString();
            sd->_normalEnter = query.value(8).toInt();
            sd->_normalLeave = query.value(9).toInt();
            sd->_reverseEnter = query.value(10).toInt();
            sd->_reverseLeave = query.value(11).toInt();
            sd->_oneWay = query.value(12).toString();
            sd->_description = query.value(13).toString();
            sd->_length = query.value(14).toInt();
            sd->_segmentDateStart = query.value(15).toDate();
            sd->_segmentDateEnd = query.value(16).toDate();
            myArray.append(sd);
        }
    }
    catch (Exception e)
    {
        myExceptionHandler(e);
    }

    return myArray;
}

#if 0
bool SQL::splitSegmentDataForCompany(SegmentData sd)
{
    QMap<int, int> earlier = {{2,4}, {3,4}, {1,2}, {14,15}, {20,15}};
    QMap<int, int> later = {{2,1},{3,1}, {14,36}, {20,36},{15,14}};
    if(!earlier.contains(sd.companyKey()))
    {
        qDebug() << QString("need prior company for %1").arg(sd.companyKey());
        return true;
    }
    if(!later.contains(sd.companyKey()))
    {
        qDebug() << QString("need later company for %1").arg(sd.companyKey());
        return true;
    }


    CompanyData* priorCd = getCompany(earlier.value(sd.companyKey()));
    CompanyData* cd = getCompany(sd.companyKey());
    CompanyData* laterCd = getCompany(later.value(sd.companyKey()));

    beginTransaction("split");
    if(sd.startDate() < cd->startDate && sd.endDate()>= cd->startDate)
    {
        if(deleteRouteSegment(sd))
        {
            SegmentData sd1  = SegmentData(sd);
            sd1.setCompanyKey(priorCd->companyKey);
            sd1.setEndDate(priorCd->endDate);
            if(!doesRouteSegmentExist(sd1))
            {
                if(!insertRouteSegment(sd1))
                {
                    rollbackTransaction("split");
                    return false;
                }
            }

            SegmentData sd2 = SegmentData(sd);
            sd2.setStartDate(cd->startDate);

            SegmentData sd3 = SegmentData(sd2);
            sd3.setCompanyKey(laterCd->companyKey);
            if(sd2.endDate() >= laterCd->startDate)
            {
                sd2.setEndDate(laterCd->startDate.addDays(-1));
                sd3.setStartDate(laterCd->startDate);
                if(!doesRouteSegmentExist(sd3))
                {
                    if(!insertRouteSegment(sd3))
                    {
                        rollbackTransaction("split");
                        return false;
                    }
                }
            }
            else
            {
                if(!doesRouteSegmentExist(sd2))
                {
                    if(!insertRouteSegment(sd2))
                    {
                        rollbackTransaction("split");
                        return false;
                    }
                }
            }
        }
        else
        {
            rollbackTransaction("split");
            return false;
        }
        // commitTransaction("split");
        // return true;
    }
    else if(sd.endDate() >= laterCd->startDate)
    {
        if(deleteRouteSegment(sd))
        {
            SegmentData sd1  = SegmentData(sd);
            sd1.setEndDate(laterCd->startDate.addDays(-1));
            if(!doesRouteSegmentExist(sd1))
            {
                if(!insertRouteSegment(sd1))
                {
                    rollbackTransaction("split");
                    return false;
                }
            }

            SegmentData sd2  = SegmentData(sd);
            sd2.setCompanyKey(priorCd->companyKey);
            sd2.setStartDate(laterCd->startDate);
            if(!doesRouteSegmentExist(sd2))
            {
                if(!insertRouteSegment(sd2))
                {
                    rollbackTransaction("split");
                    return false;
                }
            }
        }
        else
        {
            rollbackTransaction("split");
            return false;
        }
        //return true;
    }
    else
    {
        QString msg = tr("route %1 %2 %3 - %4 company %5\n %6 - %7")
                .arg(sd.route()).arg(sd.routeName(), sd.startDate().toString("yyyy/MM/dd"),
                                     sd.endDate().toString("yyyy/MM/dd"))
                                    .arg(cd->companyKey).arg(cd->startDate.toString("yyyy/MM/dd"),
                                         cd->endDate.toString("yyyy/MM/dd"));
        QMessageBox::warning(nullptr, tr("Warning"), tr("How to handle this?\n")+msg);
        qDebug() << "something else: " << msg;
        throw Exception(msg);
    }
    commitTransaction("split");
    return true;
}
#endif
bool compareRoute(const RouteData & s1, const RouteData & s2)
{
    if(s1.startDate() == s2.startDate())
        return s1.startDate() < s2.startDate();
    return s1.alphaRoute() < s2.alphaRoute();
}


bool SQL::scanRoutes(QList<RouteData> routes)
{
    if(routes.length()<2)
        return false;
    std::stable_sort(routes.begin(), routes.end(), compareRoute);
    RouteData nxtRd;
    RouteData rd;
    CompanyData* cd = getCompany(MainWindow::instance()->ui->cbCompany->currentData().toInt());

    for(int i=routes.length()-2; i>=0; i--)
    {
        qApp->processEvents();
        nxtRd = routes.at(i+1);
        rd = routes.at(i);
        //get segments for route
        QList<SegmentData*> segmentDataList =
                SQL::instance()->getRouteSegmentsInOrder(rd.route(), rd.routeName(),
                                                         rd.companyKey(), rd.endDate());
        // if(rd._dateEnd > nxtRd._dateBegin)
        //     rd._dateEnd = nxtRd._dateBegin.addDays(-1);
        foreach (SegmentData* sd, segmentDataList) {
            qApp->processEvents();
            if(sd->_dateEnd > rd.endDate())
            {
                SegmentData sdNew = SegmentData(*sd);
                sdNew._companyKey = cd->companyKey;
                sdNew._dateEnd = rd.endDate();

                if(sdNew.startDate() < cd->startDate && cd->startDate>= rd.startDate())
                {
                    sdNew._dateBegin = cd->startDate;
                }
                SegmentData sdNew2 = SegmentData(*sd);
                sdNew2._dateBegin = sdNew._dateEnd.addDays(1);
                beginTransaction("scan");
                if(!deleteRoute(*sd))
                {
                    rollbackTransaction("scan");
                    return false;
                }
                if(!doesRouteSegmentExist(sdNew))
                {
                    if(!insertRouteSegment(sdNew), false)
                    {
                        rollbackTransaction("scan");
                        return false;
                    }
                }
                if(!doesRouteSegmentExist(sdNew))
                {
                    if(!insertRouteSegment(sdNew2), false)
                    {
                        rollbackTransaction("scan");
                        return false;
                    }
                }
                commitTransaction("scan");
            }
        }
    }
    return true;
}

bool SQL::populateRouteId()
{
    QList<SegmentData*> list = segmentDataListFromView("");// get all routes
    int count=0;
    foreach (SegmentData* sd, list) {
        SegmentData sd2 = SegmentData(*sd);
        RouteInfo ri = RouteInfo(*sd);
        bool bAlreadyExists = false;
        sd2._routeId = addRouteName(ri,&bAlreadyExists);
        if(sd2._routeId > 0)
        {
            if(!updateRoute(*sd, sd2, false, true))
            {
                //return false;
            }
        }
        qApp->processEvents();
        count++;
        if(count%100 == 0)
                qDebug() << tr("processed %1 of %2").arg(count).arg(list.count());
    }
    return true;
}

qint32 SQL::addRouteName(RouteInfo ri,bool *bAlreadyExists)
{
    int rows = 0;
    int routeId = -1;
    *(bAlreadyExists) = false;
    QSqlDatabase db = QSqlDatabase::database();
    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    QString commandText = "Select RouteId from RouteName where name = '" + ri.routeName +"'";
    QSqlQuery query = QSqlQuery(db);
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
    while (query.next())
    {
     routeId = query.value(0).toInt();
    }

    if (routeId > 0 /*&& !forceInsert*/)
    {
     *(bAlreadyExists) = true;
     return routeId;
    }

    commandText = "insert into RouteName (Name) values ("
            "'" + ri.routeName
            + "')";
    bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
     rows = query.numRowsAffected();

     // Now get the SegmentId (identity) value so it can be returned.
     if(config->currConnection->servertype() == "Sqlite")
       commandText = "SELECT LAST_INSERT_ROWID()";
     else if(config->currConnection->servertype() == "MySql")
       commandText = "SELECT LAST_INSERT_ID()";
     else if(config->currConnection->servertype() == "MsSql")
       commandText = "SELECT IDENT_CURRENT('RouteName')";
     else // PostgreSQL
       commandText = "SELECT max(routeid) from routename";
     bQuery = query.exec(commandText);
    if(!bQuery)
     {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
     }
     while (query.next())
     {
       routeId = query.value(0).toInt();
     }
   return routeId;
}

bool SQL::insertRouteName(RouteInfo ri)
{
    QSqlDatabase db = QSqlDatabase::database();
    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    QSqlQuery query = QSqlQuery(db);

    QString commandText = "insert into RouteName (Name) values ("
            "'" + ri.routeName.trimmed()
            + "')";
    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        throw SQLException(error.text() + " " + errCommand);
    }
     int rows = query.numRowsAffected();
     return rows;
}

RouteInfo SQL::getRouteName(int routeId)
{
    RouteInfo ri;
    QSqlDatabase db = QSqlDatabase::database();
    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    QString commandText = "Select name "
                          "from RouteName where routeId = " + QString::number(routeId);
    QSqlQuery query = QSqlQuery(db);

    bool bQuery = query.exec(commandText);
   if(!bQuery)
    {
       QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
       qDebug() << errCommand;
       QSqlError error = query.lastError();
       SQLERROR(std::move(query));
       throw SQLException(error.text() + " " + errCommand);
    }

    while (query.next())
    {
        ri.routeName = query.value(0).toString();
    }
    return ri;
}

bool SQL::updateRouteName(RouteInfo ri)
{
    QSqlDatabase db = QSqlDatabase::database();
    if(!dbOpen())
        throw Exception(tr("database not open: %1").arg(__LINE__));
    if(ri._routeId < 0)
    {
        qDebug() << "routeId must be > 0";
        return false;
    }
    QString commandText = "Update RouteName set name ='" + ri.routeName + "',"
                          "lastUpdate =CURRENT_TIMESTAMP"
                          + " where routeId = " + QString::number(ri._routeId);
    QSqlQuery query = QSqlQuery(db);

    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        //throw SQLException(error.text() + " " + errCommand);
        return false;
    }
    return true;
}

// update the next value for an PosgreSQL identity column
// needed th first time a table is referenced after import
bool SQL::updateIdentitySequence(QString table, QString column)
{
    QString commandTxt = QString("SELECT setval(pg_get_serial_sequence('%1', '%2'),\
                                       (select max(%2) from %1))").arg(table,column);
     return executeCommand(commandTxt);
}

QString SQL::displayDbInfo(QSqlDatabase db)
{
    QString txt;
    txt.append(tr("connection name: %1 ").arg(db.connectionName()));
    txt.append(tr("driverName: %1 ").arg(db.driverName()));
    txt.append(tr("databaseName: %1 ").arg(db.databaseName()));
    txt.append(tr("hostName: %1 port: %2 ").arg(db.hostName()).arg(db.port()));
    txt.append(tr("userName: %1 pwd: %2 ").arg(db.userName(),db.password()));
    txt.append(tr("isOpen: %1 isValid: %2 ").arg(db.isOpen()?"true":"false", db.isValid()?"true":"false"));
    txt.append(tr("last error: %1 ").arg(db.lastError().text()));
    return txt;
}

bool SQL::processStream(QTextStream* in, QSqlDatabase db)
{
    _delimiter = ";";
    delimiters.clear();
    delimiters.push(_delimiter);
    linesRead =0;
    QString combined;
    QString text = in->readAll();
    QStringList lines = text.split("\n");
    QStringList statements;
    foreach(QString line, lines)
    {
        line = line.replace(QChar(8233)," ");
        if(line.contains("DELIMITER", Qt::CaseInsensitive))
        {
           int ix = line.indexOf("DELIMITER",Qt::CaseInsensitive)+ 9;
           QString delim = line.mid(ix).trimmed();
           _delimiter = delim;
           delimiters.push(_delimiter);
           continue;
        }
        if(line.isEmpty() || line.startsWith("#"))
            continue;
        if(line.contains(_delimiter))
        {
            combined.append(line);
            // if(delimiters.count(0) > 1)
            //     combined.replace(_delimiter,"delimiters.at(1)");
            if(delimiters.count()>1)
            {
                delimiters.pop();
                _delimiter = delimiters.top();
            }
            if(combined.endsWith(_delimiter))
            {
                statements.append(combined);
                combined.clear();//queryStr.clear();
            }
            continue;
        }
        if(line.contains("$"))
        {
            int ix = line.indexOf("$");
            QString delim = "$";
            int i =ix+1;
            while (i < line.length())
            {
                delim.append(line.at(i));
                if(line.at(i)== "$")
                {
                    _delimiter = delim;
                    delimiters.push(_delimiter);
                    break;
                }
                i++;
            }
        }
        combined.append(line);
    }
    foreach(QString txt, statements)
    {
        //qDebug()<<queryStr;
        query = new QSqlQuery(db);
        //QStringList sa_Message_Text;
        query->setForwardOnly(false);

        if(!query->exec(txt))
        {
            // QSqlError error = query->lastError();
            // qCritical() << "processStream failed with error: " << error.text();
            // qCritical() << queryStr;
            return false;  // caller can gall getQueryn and report any error.
        }
    }
    return true;
}

bool SQL::isFunctionInstalled(QString function, QString dbType,QString dbName, QSqlDatabase db)
{
    QString commandText;
    if(dbType == "Sqlite")
        return true;
    else if(dbType == "MySql")
    {
        commandText = QString("SELECT ROUTINE_NAME "
                              "FROM INFORMATION_SCHEMA.ROUTINES "
                              "WHERE "
                              "       ROUTINE_TYPE='FUNCTION'"
                              "   AND ROUTINE_SCHEMA='%1' ;").arg(dbName);

    }
    else if(dbType == "PostgreSQL")
    {
        commandText = "select p.oid::regproc "
                "from pg_proc p "
                "     join pg_namespace n "
                "     on p.pronamespace = n.oid "
               "where n.nspname not in ('pg_catalog', 'information_schema');";
    }
    else if(dbType == "MsSql")
    {
        commandText = QString("SELECT '%1' FROM INFORMATION_SCHEMA.ROUTINES WHERE ROUTINE_NAME = '%1' AND ROUTINE_TYPE = 'FUNCTION'").arg(dbName);
    }
    else {
        throw IllegalArgumentException(tr("bad dbtype: %1").arg(dbType));
    }
    QSqlQuery query = QSqlQuery(db);

    bool bQuery = query.exec(commandText);
    if(!bQuery)
    {
        QString errCommand = query.lastQuery() + " line:" + QString("%1").arg(__LINE__) +"\n";
        qDebug() << errCommand;
        QSqlError error = query.lastError();
        SQLERROR(std::move(query));
        //throw SQLException(error.text() + " " + errCommand);
        return false;
    }

    while(query.next())
    {
        QString rslt = query.value(0).toString();
        if(rslt == function)
            return true;
    }
    return false;
}

