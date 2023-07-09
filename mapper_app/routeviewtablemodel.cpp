#include "routeviewtablemodel.h"
#include <QDebug>
#include "sql.h"
#include "webviewbridge.h"


RouteViewTableModel::RouteViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
 selectedRow = -1;
 bSelectedRowChanged = false;
 bIsSequenced = false;
 tractionTypes = SQL::instance()->getTractionTypes();
}

RouteViewTableModel::RouteViewTableModel(qint32 route, QString name, QDate dtStart, QDate dtEnd, QList<SegmentData> segmentDataList, QObject *parent)
     : QAbstractTableModel(parent)
{
 this->route = route;
 this->name = name;
 selectedRow = -1;
 bSelectedRowChanged = false;
 bIsSequenced = false;
 listOfSegments = segmentDataList;
 saveSegmentDataList = QList<SegmentData>(segmentDataList);
 this->dtEnd = dtEnd;
 this->dtStart = dtStart;
 startDate = dtStart;
 endDate = dtEnd;
 startRow = -1;
 endRow = -1;
 TerminalInfo ti = SQL::instance()->getTerminalInfo(route,name, endDate.toString("yyyy/MM/dd"));
 for(int i =0; i < listOfSegments.count(); i++)
 {
  SegmentData sd = listOfSegments.at(i);
  if(ti.startSegment == sd.segmentId())
   startRow = i;
  if(ti.endSegment == sd.segmentId())
   endRow = i;
 }

 tractionTypes = SQL::instance()->getTractionTypes();

}

int RouteViewTableModel::rowCount(const QModelIndex &parent) const
{
 Q_UNUSED(parent);
 return listOfSegments.size();
}

int RouteViewTableModel::columnCount(const QModelIndex &parent) const
{
 Q_UNUSED(parent);
 return ENDDATE+1;
}

QVariant RouteViewTableModel::data(const QModelIndex &index, int role) const
{
 if (!index.isValid())
     return QVariant();

 if (index.row() >= listOfSegments.size() || index.row() < 0)
     return QVariant();
 SegmentData sd = listOfSegments.at(index.row());
 if(sd.tractionType() < 0)
  qDebug() << tr("invalid tractionType") << sd.tractionType();

 if(role == Qt::BackgroundRole)
 {
  QVariant background = QVariant();
  if ( index.row() == startRow)
  {
   background = QVariant( QColor(Qt::green));
  }

  if ( index.row() == endRow)
  {
   background =  QVariant( QColor(Qt::red) );
  }

  switch(index.column())
  {
  case NEXT:
   if(sd.next() == -1)
    background = QColor(Qt::yellow);
   break;
  case PREV:
   if(sd.prev() == -1)
    background = QColor(Qt::yellow);
   break;
  case SEQ:
   if(sd.sequence() == -1)
    background = QColor(Qt::yellow);
   break;
  case RSEQ:
   if(sd.returnSeq() == -1 && sd.oneWay() != "Y")
    background = QColor(Qt::yellow);
   break;
  }
  return background;
 }

 if (role == Qt::DisplayRole /*|| role == Qt::EditRole*/)
 {

   switch(index.column())
   {
   case SEGMENTID:
    for(int i=0; i < changedRows.count(); i++)
    {
     //if((changedRows.at(i)).index == index)
     if(index.row() == changedRows.at(i)->row)
     {
      if(changedRows.at(i)->bDeleted)
         return QString("%1").arg(sd.segmentId())+" !";

      if(changedRows.at(i)->bChanged)
         return QString("%1").arg(sd.segmentId())+" *";
     }
    }
    return sd.segmentId();
   case NAME:
       return sd.description();
   case ONEWAY:
       return sd.oneWay();
   case USAGE:
       return sd.trackUsage();
   case TRACTIONTYPE:
       //return sd.tractionType();
       return tractionTypes.value(sd.tractionType()).description;
   case TRACKS:
       return sd.tracks();
   case TYPE:
       return SegmentData::ROUTETYPES.at(sd.routeType());
   case NEXT:
       return sd.next();
   case PREV:
       return sd.prev();
   case DIR:
       return sd.direction();
   case SEQ:
       return sd.sequence();
   case RSEQ:
    if(sd.oneWay() == "Y")
     return QVariant();
    return sd.returnSeq();
   case STARTDATE:
       return sd.startDate().toString("yyyy/MM/dd");
   case ENDDATE:
       return sd.endDate().toString("yyyy/MM/dd");
   case DISTANCE:
       return sd.length();
  }
 }
 return QVariant();
}

QVariant RouteViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 if (role != Qt::DisplayRole)
    return QVariant();
 if (orientation == Qt::Horizontal)
 {
    switch (section)
    {
        case SEGMENTID:
            return tr("SegId");
        case NAME:
            return tr("Name");
        case ONEWAY:
            return tr("1Way");
        case TRACKS:
            return tr("Trk");
        case USAGE:
            return tr("Use");
        case TRACTIONTYPE:
            return tr("TractionType");
        case TYPE:
            return tr("RouteType");
        case NEXT:
            return tr("Next");
        case PREV:
            return tr("Prev");
        case DIR:
            return tr("Dir");
        case SEQ:
            return tr("Seq");
        case RSEQ:
            return tr("RSeq");
        case STARTDATE:
            return tr("StartDate");
        case ENDDATE:
            return tr("EndDate");
        case DISTANCE:
         return tr("Distance");

        default:
            return QVariant();
    }
 }
 return QVariant();
}

bool RouteViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
 Q_UNUSED(index);
 beginInsertRows(QModelIndex(), position, position+rows-1);

 for (int row=0; row < rows; row++) {
     //QPair<QString, QString> pair(" ", " ");
     SegmentData sd;
     listOfSegments.insert(position, sd);
 }

 endInsertRows();
 return true;
}

bool RouteViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
 Q_UNUSED(index);
 beginRemoveRows(QModelIndex(), position, position+rows-1);

 for (int row=0; row < rows; ++row) {
     listOfSegments.removeAt(position);
 }

 endRemoveRows();
 return true;
}

bool RouteViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 if (index.isValid() && role == Qt::EditRole)
 {
  int row = index.row();

  SegmentData sd = listOfSegments.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;
  QDate dt;
//  int tracks;
  switch(index.column())
  {
   case ONEWAY:
   {
    QString s = value.toString().toUpper();
//    if(tracks == 2)
//    {
//     si.oneWay = "N";
//     break;
//    }
    if(s == "N" || s== "Y")
     sd.setOneWay(s);
    if(s == "N")
    {
     sd.setTrackUsage(" ");
    }
    break;
   }
  case USAGE:
  {
   QString s = value.toString().toUpper();
   if(s == "B" || s=="L" || s == "R" || s == " ")
    sd.setTrackUsage(s);
   break;
  }
  case TRACTIONTYPE:
   sd.setTractionType(value.toInt());
   break;
//  case TRACKS:
//    tracks = value.toInt();
//    if(tracks == 1 || tracks == 2)
//     si.tracks = tracks;
//    break;
   case STARTDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
       if(dt <= sd.endDate())
        sd.setStartDate(value.toDate()/*.toString("yyyy/MM/dd")*/);
      }
      break;
   case ENDDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
       if(dt >= sd.startDate())
       // si.endDate = (QString&)value;
        sd.setEndDate(value.toDate());
      }
      break;

  }
  listOfSegments.replace(row, sd);

  selectedRow = row;
  bSelectedRowChanged =true;
  RowChanged* changeEntry = nullptr;
//  changeEntry->row = row;
//  changeEntry->index = index;
//  changeEntry->bChanged=true;
//  changeEntry->bDeleted = false;
//  changeEntry->segmentId = si.segmentId;
//  changeEntry->si = si;
//  int i;
//  for(i=0; i < changedRows.count(); i++)
//  {
//   if(changedRows.at(i)->index == index)
//      break;
//  }
//  if(i >= changedRows.count())
//  {
//      changedRows.append(changeEntry);
//      emit rowChange(row, si.segmentId, false, true);
//  }
//  listOfSegments.replace(row, si);
  for(int i=0; i < changedRows.count(); i++)
  {
   if(changedRows.at(i)->row == selectedRow)
   {
    changeEntry = changedRows.at(i);
    break;
   }
  }
  if(changeEntry == nullptr)
  {
   changeEntry = new RowChanged();
   changeEntry->row = row;
   changeEntry->index = index;
   changeEntry->bChanged=true;
   changeEntry->bDeleted = false;
   changeEntry->segmentId = sd.segmentId();
   changedRows.append(changeEntry);
  }
  changeEntry->sd = sd;
  emit dataChanged(index, index);

  return true;
 }

 return false;
}

void RouteViewTableModel::reset()
{
    //QAbstractTableModel::reset();
 beginResetModel();
 endResetModel();
}

Qt::ItemFlags RouteViewTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    switch(index.column())
    {
    case SEGMENTID:
    case NAME:
     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    case ONEWAY: // 1Way
    case USAGE:
    case TRACTIONTYPE:
     return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    case TRACKS:
     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    case NEXT: // next
    case PREV: // prev
    case SEQ: // seq
    case RSEQ: // rSeq
        if(! bIsSequenced)
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        break;
    case STARTDATE: //start date
    case ENDDATE: // end date
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }
    // all other columns non-editable
    return QAbstractTableModel::flags(index);
}

QList< SegmentData > RouteViewTableModel::getList()
{
    return listOfSegments;
}

void RouteViewTableModel::setSequenced(bool b)
{
 bIsSequenced = b;
}

//void RouteViewTableModel::commitChanges()
//{
// for(int i=0; i < changedRows.count(); i ++)
// {
//  RowChanged* rc = changedRows.at(i);
//  SegmentInfo si = listOfSegments.at(rc->index.row());
//  removeRow(rc->index.row());
//  QString str =  si.description;
// }
//}

bool sortbyrow( const RowChanged & s1 , const RowChanged & s2 )
{
    return s1.row > s2.row;
}

void RouteViewTableModel::commitChanges()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);

 // sort rows in descending sequence.
 //qSort(*changedRows.begin(),*changedRows.end(), sortbyrow );

 for(int i=0; i < changedRows.count(); i++)
 {
  int row = changedRows.at(i)->row;
  SegmentData sd = changedRows.at(i)->sd;
  if(sd.segmentId() != changedRows.at(i)->segmentId)
  {
      qDebug() << "Error, wrong segmentId found! " << sd.segmentId() << " vs << changedRows.at(i)->segmentId";
      return;
  }
  //segmentInfo siOld = segmentInfoList.at(row);
  SegmentData sdOld = saveSegmentDataList.at(row);

  RouteData rd = SQL::instance()->getRouteData(route, sd.segmentId(), sd.startDate().toString("yyyy/MM/dd"), sd.endDate().toString("yyyy/MM/dd"));
  if(rd.route <= 0)
  {
      qDebug()<< "route data not found:";
      qDebug() << " route="+ QString("%1").arg(route)+ " segmentId="+ QString("%1").arg(sdOld.segmentId())+ " "+ sdOld.startDate().toString("yyyy/MM/dd")+ " "+sdOld.endDate().toString("yyyy/MM/dd");
      throw RecordNotFoundException(tr("route not found: %1, segment: %2 %3 %4").arg(route).arg(sdOld.segmentId()).arg(sdOld.startDate().toString("yyyy/MM/dd")).arg(sdOld.endDate().toString("yyyy/MM/dd")));
  }

  SQL::instance()->BeginTransaction("updateRoute");
  if(sdOld.oneWay() != sd.oneWay()  || sdOld.length() != sd.length()  || sdOld.needsUpdate())
  {
   SQL::instance()->updateSegmentDescription(sd.segmentId(), sd.description(), /*sd.oneWay(),*/ sd.tracks(), sd.length());
  }

  if(sd.oneWay() == "N")
   sd.setTrackUsage(" ");
  if(sdOld.trackUsage() != sd.trackUsage() || sdOld.tractionType() != sd.tractionType())
  {
   //if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), rd.lineKey, rd.next, rd.prev, si.trackUsage ))
   rd.tractionType = sd.tractionType();
   if(!SQL::instance()->updateRoute(rd))
    return;
  }

  if(!SQL::instance()->deleteRouteSegment(route, name, sd.segmentId(), sdOld.startDate().toString("yyyy/MM/dd"), sdOld.endDate().toString("yyyy/MM/dd")))
      return;
  if(sdOld.startDate() < startDate)
  {
   // add back segment used before route start date
   QDate newEndDate = startDate.addDays(-1);
   if(!SQL::instance()->addSegmentToRoute(route, name, sd.startDate(), newEndDate, sd.segmentId(), rd.companyKey,
                                          rd.tractionType, sd.bearing().strDirection(),sd.next(), sd.prev(),
                                          sd.normalEnter(), sd.normalLeave(), sd.reverseEnter(), sd.reverseLeave(), rd.oneWay, sd.trackUsage()))
       return;
  }

  if(sdOld.endDate() > endDate)
  {
   QDate newStartDate = endDate.addDays(+1);
   if(!SQL::instance()->addSegmentToRoute(route, name, newStartDate, sd.endDate(), sd.segmentId(), rd.companyKey,
                                          rd.tractionType, sd.bearing().strDirection(),sd.next(), sd.prev(),
                                          sd.normalEnter(), sd.normalLeave(), sd.reverseEnter(), sd.reverseLeave(), rd.oneWay, sd.trackUsage()))
       return;
  }
  if(changedRows.at(i)->bDeleted)
  {
   //myParent->ProcessScript("clearPolyline", QString("%1").arg(si.segmentId));
   WebViewBridge::instance()->processScript("clearPolyline", QString("%1").arg(sd.segmentId()));
  }
  if(changedRows.at(i)->bChanged && !changedRows.at(i)->bDeleted)
  {
      if(!SQL::instance()->addSegmentToRoute(route, name, sd.startDate(), sd.endDate(),
                                             sd.segmentId(), rd.companyKey, rd.tractionType,
                                             sd.bearing().strDirection(),sd.next(), sd.prev(), sd.normalEnter(), sd.normalLeave(),
                                             sd.reverseEnter(), sd.reverseLeave(), sd.oneWay(), sd.trackUsage()))
          return;
  }
  if(changedRows.at(i)->bDeleted)
   removeRow(row);

  changedRows.at(i)->bChanged = false;
  changedRows.at(i)->bDeleted = false;
  sd.setNeedsUpdate(false);
  SQL::instance()->CommitTransaction("updateRoute");
  saveSegmentDataList.replace(i, sd);
 }
 changedRows.clear();
 //myParent->refreshRoutes();
 emit refreshRoutes();
//    updateRouteView();
}


// Called by RouteView to add to list of segments to be deleted.
void RouteViewTableModel::deleteRow(qint32 segmentId, const QModelIndex &index)
{
 if(segmentId == 0)
  return;

 for(int i = 0; i < changedRows.count(); i++)
 {
  RowChanged* rc = changedRows.at(i);
  if(rc->segmentId == segmentId)
   return; // already posted
 }
//  SegmentInfo si = listOfSegments.at(rc.index.row());
//  if(rc.index == index)
//  {
//   if(QDate::fromString(si.startDate, "yyyy/MM/dd") < dtStart)
//   {
//    // This segment spans a route date change so just set the end date to the route start date - 1
//    setData(this->index(index.row(),ENDDATE ),QVariant(dtStart.addDays(-1)));
//    return;
//   }
//   if(QDate::fromString(si.endDate, "yyyy/MM/dd") > dtEnd)
//   {
//    // This segment spans a route date change so just set the start date to the route end date +1 1
//    setData(this->index(index.row(),STARTDATE ),QVariant(dtEnd.addDays(+1)));
//    return;
//   }
//   changedRows.at(i)->bDeleted = true;
//   return;
//  }
// }
 RowChanged* rc = new RowChanged();
 for(int i = 0; i < listOfSegments.count(); i++)
 {
  SegmentData sd = listOfSegments.at(i);
  if(sd.segmentId() == segmentId )
  {
   rc->row= i;
   rc->sd = SegmentData(sd);
   break;
  }
 }
 //rc.row = index.row();
 rc->index = index;
 rc->bChanged =false;
 rc->bDeleted = true;
 rc->segmentId = segmentId;
 changedRows.append(rc);
 emit rowChange(rc->row, segmentId,  true, false);
}

void RouteViewTableModel::unDeleteRow(qint32 segmentId, const QModelIndex &index)
{
 Q_UNUSED(segmentId)
 for(int i = 0; i < changedRows.count(); i++)
 {
  RowChanged *rc = changedRows.at(i);
  if(rc->index == index && rc->bDeleted)
  {
   rc->bDeleted = false;
   if(!rc->bChanged && changedRows.count() == 1)
   {
    changedRows.clear();
   }
   return;
  }
 }
}
bool RouteViewTableModel::isSegmentMarkedForDelete(qint32 segmentId)
{
 for(int i = 0; i < changedRows.count(); i++)
 {
  RowChanged *rc = changedRows.at(i);
  if(rc->segmentId == segmentId && rc->bDeleted)
   return true;
 }
 return false;
}

int RouteViewTableModel::getRow(int segmentId)
{
 int row = -1;
 if(listOfSegments.isEmpty())
  return row;
 for (row = 0; row < listOfSegments.count(); row++)
 {
  if(listOfSegments.at(row).segmentId() == segmentId)
  {
   break;
  }
 }
 return row;
}

void RouteViewTableModel::getRows(qint32 start, qint32 end)
{
 startRow = start;
 endRow = end;
 beginResetModel();
 endResetModel();
}
