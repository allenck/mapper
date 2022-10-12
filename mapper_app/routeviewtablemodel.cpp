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
}

RouteViewTableModel::RouteViewTableModel(qint32 route, QString name, QDate dtStart, QDate dtEnd, QList<SegmentInfo> segmentInfoList, QObject *parent)
     : QAbstractTableModel(parent)
{
 this->route = route;
 this->name = name;
 selectedRow = -1;
 bSelectedRowChanged = false;
 bIsSequenced = false;
 listOfSegments = segmentInfoList;
 saveSegmentInfoList = QList<SegmentInfo>(segmentInfoList);
 this->dtEnd = dtEnd;
 this->dtStart = dtStart;
 startDate = dtStart.toString("yyyy/MM/dd");
 endDate = dtEnd.toString("yyyy/MM/dd");
 startRow = -1;
 endRow = -1;
 TerminalInfo ti = SQL::instance()->getTerminalInfo(route,name, endDate);
 for(int i =0; i < listOfSegments.count(); i++)
 {
  SegmentInfo si = listOfSegments.at(i);
  if(ti.startSegment == si.segmentId)
   startRow = i;
  if(ti.endSegment == si.segmentId)
   endRow = i;
 }

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
 SegmentInfo si = listOfSegments.at(index.row());

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
   if(si.next == -1)
    background = QColor(Qt::yellow);
   break;
  case PREV:
   if(si.prev == -1)
    background = QColor(Qt::yellow);
   break;
  case SEQ:
   if(si.sequence == -1)
    background = QColor(Qt::yellow);
   break;
  case RSEQ:
   if(si.returnSeq == -1 && si.oneWay != "Y")
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
         return QString("%1").arg(si.segmentId)+" !";

      if(changedRows.at(i)->bChanged)
         return QString("%1").arg(si.segmentId)+" *";
     }
    }
    return si.segmentId;
   case NAME:
       return si.description;
   case ONEWAY:
       return si.oneWay;
   case USAGE:
       return si.trackUsage;
   case TRACKS:
       return si.tracks;
   case TYPE:
       return SegmentInfo::ROUTETYPES.at(si.routeType);
   case NEXT:
       return si.next;
   case PREV:
       return si.prev;
   case DIR:
       return si.direction;
   case SEQ:
       return si.sequence;
   case RSEQ:
    if(si.oneWay == "Y")
     return QVariant();
    return si.returnSeq;
   case STARTDATE:
       return si.startDate;
   case ENDDATE:
       return si.endDate;
   case DISTANCE:
       return si.length;
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
     SegmentInfo si;
     listOfSegments.insert(position, si);
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

  SegmentInfo si = listOfSegments.value(row);

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
     si.oneWay = s;
    if(s == "N")
    {
     si.trackUsage = " ";
    }
    break;
   }
  case USAGE:
  {
   QString s = value.toString().toUpper();
   if(s == "B" || s=="L" || s == "R" || s == " ")
    si.trackUsage = s;
   break;
  }
//  case TRACKS:
//    tracks = value.toInt();
//    if(tracks == 1 || tracks == 2)
//     si.tracks = tracks;
//    break;
   case STARTDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
       if(dt <= QDate::fromString(si.endDate, "yyyy/MM/dd"))
        si.startDate = value.toDate().toString("yyyy/MM/dd");
      }
      break;
   case ENDDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
       if(dt >= QDate::fromString(si.startDate, "yyyy/MM/dd"))
       // si.endDate = (QString&)value;
        si.endDate = value.toDate().toString("yyyy/MM/dd");
      }
      break;

  }
  listOfSegments.replace(row, si);

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
   changeEntry->segmentId = si.segmentId;
   changedRows.append(changeEntry);
  }
  changeEntry->si = si;
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

QList< SegmentInfo > RouteViewTableModel::getList()
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
  SegmentInfo si = changedRows.at(i)->si;
  if(si.segmentId != changedRows.at(i)->segmentId)
  {
      qDebug() << "Error, wrong segmentId found! " << si.segmentId << " vs << changedRows.at(i)->segmentId";
      return;
  }
  //segmentInfo siOld = segmentInfoList.at(row);
  SegmentInfo siOld = saveSegmentInfoList.at(row);

  RouteData rd = SQL::instance()->getRouteData(route, si.segmentId, si.startDate, si.endDate);
  if(rd.route <= 0)
  {
      qDebug()<< "route data not found:";
      qDebug() << " route="+ QString("%1").arg(route)+ " segmentId="+ QString("%1").arg(siOld.segmentId)+ " "+ siOld.startDate + " "+siOld.endDate;
      return;
  }

  SQL::instance()->BeginTransaction("updateRoute");
  if(siOld.oneWay != si.oneWay  || siOld.length != si.length  || siOld.bNeedsUpdate)
  {
   SQL::instance()->updateSegmentDescription(si.segmentId, si.description, si.oneWay, si.tracks, si.length);
  }

  if(si.oneWay == "N")
   si.trackUsage = " ";
  if(siOld.trackUsage != si.trackUsage)
  {
   if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), rd.lineKey, rd.next, rd.prev, si.trackUsage ))
    return;
  }

  if(!SQL::instance()->deleteRouteSegment(route, name, si.segmentId, siOld.startDate, siOld.endDate))
      return;
  if(siOld.startDate < startDate)
  {
   // add back segment used before route start date
   QString newEndDate = QDate::fromString(startDate, "yyyy/MM/dd").addDays(-1).toString("yyyy/MM/dd");
   if(!SQL::instance()->addSegmentToRoute(route, name, si.startDate, newEndDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave, rd.oneWay, si.trackUsage))
       return;
  }

  if(siOld.endDate > endDate)
  {
   QString newStartDate = QDate::fromString(endDate, "yyyy/MM/dd").addDays(+1).toString("yyyy/MM/dd");
   if(!SQL::instance()->addSegmentToRoute(route, name, newStartDate, si.endDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave, rd.oneWay, si.trackUsage))
       return;
  }
  if(changedRows.at(i)->bDeleted)
  {
   //myParent->ProcessScript("clearPolyline", QString("%1").arg(si.segmentId));
   webViewBridge::instance()->processScript("clearPolyline", QString("%1").arg(si.segmentId));
  }
  if(changedRows.at(i)->bChanged && !changedRows.at(i)->bDeleted)
  {
      if(!SQL::instance()->addSegmentToRoute(route, name, si.startDate, si.endDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave, si.oneWay, si.trackUsage))
          return;
  }
  if(changedRows.at(i)->bDeleted)
   removeRow(row);

  changedRows.at(i)->bChanged = false;
  changedRows.at(i)->bDeleted = false;
  si.bNeedsUpdate = false;
  SQL::instance()->CommitTransaction("updateRoute");
  saveSegmentInfoList.replace(i, si);
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
  SegmentInfo si = listOfSegments.at(i);
  if(si.segmentId == segmentId )
  {
   rc->row= i;
   rc->si = SegmentInfo(si);
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
  if(listOfSegments.at(row).segmentId == segmentId)
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
