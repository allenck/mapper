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
    for(int i=0; i < changedMap.values().count(); i++)
    {
     //if((changedMap.values().at(i)).index == index)
     if(index.row() == changedMap.values().at(i)->row)
     {
      if(changedMap.values().at(i)->bDeleted)
         return QString("%1").arg(sd.segmentId())+" !";

      if(changedMap.values().at(i)->bChanged)
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
//    if(sd.oneWay() == "Y")
//     return tr("na");
    return sd.returnSeq();
   case NE:
    return turnMap.value(sd.normalEnter());
   case NL:
    return turnMap2.value(sd.normalLeave());
   case RE:
    return turnMap.value(sd.reverseEnter());
   case RL:
    return turnMap2.value(sd.reverseLeave());
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
            return tr("SegmentId");
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
        case NE:
         return tr(" from");
        case NL:
         return tr("Leave ");
        case RE:
         return tr("R from");
        case RL:
         return tr("RLeave ");
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
    if(s == "N" || s== "Y")
     sd.setOneWay(s);
    if(s == "N")
    {
     sd.setTrackUsage(" ");
    }
    if(!changedMap.contains(index.row()))
     changedMap.insert(row, new RowChanged(row, true, sd, sd));
    else
    {
     changedMap.value(row)->sd.setOneWay(sd.oneWay());
     changedMap.value(row)->sd.setTrackUsage(sd.trackUsage());
    }
   }
  case USAGE:
  {
   QString s = value.toString().toUpper();
   if(s == "B" || s=="L" || s == "R" || s == " ")
    sd.setTrackUsage(s);
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setTrackUsage(sd.trackUsage());
   break;
  }
  case TRACTIONTYPE:
   sd.setTractionType(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setTractionType(sd.tractionType());
   break;
  case TYPE:
   sd.setRouteType((RouteType)value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setRouteType((RouteType)value.toInt());
   break;
  case NEXT:
   sd.setNext(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd,sd));
   else
    changedMap.value(row)->sd.setNext(sd.next());
   break;
  case PREV:
   sd.setPrev(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd,sd));
   else
    changedMap.value(row)->sd.setPrev(sd.prev());
   break;
  case SEQ:
   sd.setSequence(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setSequence(sd.sequence());
   break;
  case RSEQ:
   sd.setReturnSeq(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setReturnSeq(sd.returnSeq());
   break;
  case NE:
   sd.setNormalEnter(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setReturnSeq(sd.normalEnter());
   break;
  case NL:
   sd.setNormalLeave(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setReturnSeq(sd.normalLeave());
   break;
  case RE:
   sd.setReverseEnter(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setReturnSeq(sd.reverseEnter());
   break;
  case RL:
   sd.setReverseLeave(value.toInt());
   if(!changedMap.contains(index.row()))
    changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
   else
    changedMap.value(row)->sd.setReturnSeq(sd.reverseLeave());
   break;
   case STARTDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
       if(!changedMap.contains(index.row()))
        changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
       else
        changedMap.value(index.row())->sd.setStartDate(dt);

      sd.setStartDate(value.toDate());
      }
      break;
   case ENDDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
//       if(dt >= sd.startDate())
//        sd.setEndDate(value.toDate());
        if(!changedMap.contains(index.row()))
         changedMap.insert(index.row(), new RowChanged(index.row(), true, sd, sd));
        else
         changedMap.value(index.row())->sd.setEndDate(dt);
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
//  for(i=0; i < changedMap.values().count(); i++)
//  {
//   if(changedMap.values().at(i)->index == index)
//      break;
//  }
//  if(i >= changedMap.values().count())
//  {
//      changedMap.values().append(changeEntry);
//      emit rowChange(row, si.segmentId, false, true);
//  }
//  listOfSegments.replace(row, si);
  for(int i=0; i < changedMap.values().count(); i++)
  {
   if(changedMap.values().at(i)->row == selectedRow)
   {
    changeEntry = changedMap.values().at(i);
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
   changedMap.values().append(changeEntry);
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
    case TYPE:
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
    case NE:
    case NL:
    case RE:
    case RL:
     return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled;

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

bool sortbyrow( const RowChanged & s1 , const RowChanged & s2 )
{
    return s1.row > s2.row;
}

bool RouteViewTableModel::commitChanges()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);

 // sort rows in descending sequence.
 //qSort(*changedMap.values().begin(),*changedMap.values().end(), sortbyrow );

 for(int i=0; i < changedMap.values().count(); i++)
 {
  RowChanged* rc = changedMap.values().at(i);
  int row = rc->row;

  SQL::instance()->beginTransaction("updateRoute");

  if( SQL::instance()->doesRouteSegmentExist(rc->osd.route(), rc->osd.routeName(),
                                             rc->osd.segmentId(), rc->osd.startDate(),
                                             rc->osd.endDate()))
  {
    if(rc->bChanged )
    {
     if(!SQL::instance()->updateRoute(rc->osd, rc->sd))
     {
      SQL::instance()->rollbackTransaction("updateRoute");
      return false;
     }
     if(!SQL::instance()->updateSegment(&rc->sd))
     {
      SQL::instance()->rollbackTransaction("updateRoute");
      return false;
     }
    }

    if(rc->bDeleted)
    {
     if(!SQL::instance()->deleteRouteSegment(rc->osd.route(), rc->osd.routeName(),
                                            rc->osd.segmentId(), rc->osd.startDate().toString("yyyy/MM/dd"),
                                            rc->osd.endDate().toString("yyyy/MM/dd")))
     {
      SQL::instance()->rollbackTransaction("updateRoute");
      return false;
     }
    }
  }
 }
 SQL::instance()->commitTransaction("updateRoute");

 changedMap.clear();
 //myParent->refreshRoutes();
 //emit refreshRoutes();
 //updateRouteView();
 return true;
}

void RouteViewTableModel::discardChanges()
{
 listOfSegments = saveSegmentDataList;
 changedMap.clear();
 reset();
}

// Called by RouteView to add to list of segments to be deleted.
void RouteViewTableModel::deleteRow(qint32 segmentId, const QModelIndex &index)
{
 if(segmentId == 0)
  return;

 for(int i = 0; i < changedMap.values().count(); i++)
 {
  RowChanged* rc = changedMap.values().at(i);
  if(rc->segmentId == segmentId)
   return; // already posted
 }

 RowChanged* rc = new RowChanged();
 for(int i = 0; i < listOfSegments.count(); i++)
 {
  SegmentData osd = listOfSegments.at(i);
  if(osd.segmentId() == segmentId )
  {
   rc->row= i;
   rc->osd = SegmentData(osd);
   rc->bDeleted = true;
   break;
  }
 }
 //rc.row = index.row();
 rc->index = index;
 rc->bChanged =false;
 rc->bDeleted = true;
 rc->segmentId = segmentId;
 changedMap.insert(index.row(), rc);
 emit rowChange(rc->row, segmentId,  true, false);
}

void RouteViewTableModel::unDeleteRow(qint32 segmentId, const QModelIndex &index)
{
 Q_UNUSED(segmentId)
 for(int i = 0; i < changedMap.values().count(); i++)
 {
  RowChanged *rc = changedMap.values().at(i);
  if(rc->index == index && rc->bDeleted)
  {
   rc->bDeleted = false;
   if(!rc->bChanged && changedMap.values().count() == 1)
   {
    changedMap.values().clear();
   }
   return;
  }
 }
}
bool RouteViewTableModel::isSegmentMarkedForDelete(qint32 segmentId)
{
 for(int i = 0; i < changedMap.values().count(); i++)
 {
  RowChanged *rc = changedMap.values().at(i);
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

SegmentData RouteViewTableModel::segmentData(int row)
{
 return listOfSegments.at(row);
}
