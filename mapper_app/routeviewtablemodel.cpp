#include "routeviewtablemodel.h"
#include <QDebug>
#include "webviewbridge.h"
#include "mainwindow.h"

RouteViewTableModel::RouteViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
 selectedRow = -1;
 bSelectedRowChanged = false;
 bIsSequenced = false;
 tractionTypes = SQL::instance()->getTractionTypes();
 connect(SQL::instance(), SIGNAL(routeChange(SQL::ROUTECHANGETYPE, SegmentData)),
         SLOT(routeChange(SQL::ROUTECHANGETYPE, SegmentData)));
}

RouteViewTableModel::RouteViewTableModel(qint32 route, QString name, QDate dtStart,
                                         QDate dtEnd, QList<SegmentData *> segmentDataList,
                                         QObject *parent)
     : QAbstractTableModel(parent)
{
 this->route = route;
 this->name = name;
 selectedRow = -1;
 bSelectedRowChanged = false;
 bIsSequenced = false;
 listOfSegments = segmentDataList;
 //saveSegmentDataList = QList<SegmentData>(segmentDataList);
 saveSegmentDataList = QList<SegmentData>();
 for(SegmentData* sd : segmentDataList)
  saveSegmentDataList.append(SegmentData(*sd));
 this->dtEnd = dtEnd;
 this->dtStart = dtStart;
 startDate = dtStart;
 endDate = dtEnd;
 startRow = -1;
 endRow = -1;
 connect(SQL::instance(), SIGNAL(routeChange(SQL::ROUTECHANGETYPE, SegmentData)),
         SLOT(routeChange(SQL::ROUTECHANGETYPE, SegmentData)));

 TerminalInfo ti = SQL::instance()->getTerminalInfo(route,name, endDate.toString("yyyy/MM/dd"));
 for(int i =0; i < listOfSegments.count(); i++)
 {
  SegmentData* sd = listOfSegments.at(i);
  if(ti.startSegment == sd->segmentId())
   startRow = i;
  if(ti.endSegment == sd->segmentId())
   endRow = i;
 }

 tractionTypes = SQL::instance()->getTractionTypes();

}

void RouteViewTableModel::routeChange(SQL::ROUTECHANGETYPE type, SegmentData sd)
{
 int row = -1;
 for(int i=0; i < listOfSegments.count(); i++)
 {
  SegmentData* sd1 = listOfSegments.at(i);
  if(sd.route() == sd1->route() && sd.routeName()==sd1->routeName() && sd.segmentId()==sd1->segmentId()
     && sd.startDate()==sd1->startDate() && sd.endDate()==sd1->endDate())
   row = i;
  break;
 }
 if(row == -1 && type != SQL::ADD)
  return;
 if(type == SQL::DELETE)
 {
  beginRemoveRows(QModelIndex(), row, row);
  listOfSegments.removeAt(row);
  endRemoveRows();
 }
 else if(type == SQL::ADD)
 {
  beginInsertRows(QModelIndex(), listOfSegments.count(), listOfSegments.count());
  listOfSegments.append(&sd);
  endInsertRows();
  MainWindow::instance()->segmentChanged(0, sd.segmentId());
 }
 else
 {
  listOfSegments.replace(row,&sd);
  MainWindow::instance()->segmentChanged(sd.segmentId(), sd.segmentId());

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
 SegmentData* sd = listOfSegments.at(index.row());
 if(sd->tractionType() < 0)
  qDebug() << tr("invalid tractionType") << sd->tractionType();

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
   if(sd->next() == -1)
    background = QColor(Qt::yellow);
   break;
  case PREV:
   if(sd->prev() == -1)
    background = QColor(Qt::yellow);
   break;
  case NEXTR:
   if(sd->nextR() == -1)
    background = QColor(Qt::yellow);
   break;
  case PREVR:
   if(sd->prevR() == -1)
    background = QColor(Qt::yellow);
   break;
  case SEQ:
   if(sd->sequence() == -1)
    background = QColor(Qt::yellow);
   break;
  case RSEQ:
   if(sd->returnSeq() == -1 && sd->oneWay() != "Y")
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
//    for(int i=0; i < changedMap.values().count(); i++)
//    {
//     //if((changedMap.values().at(i)).index == index)
//     if(index.row() == changedMap.values().at(i)->row)
//     {
//      if(changedMap.values().at(i)->bDeleted)
//         return QString("%1").arg(sd->segmentId())+" !";

//      if(changedMap.values().at(i)->bChanged)
//         return QString("%1").arg(sd->segmentId())+" *";
//     }
    if(sd->markedForDelete())
     return QString("%1").arg(sd->segmentId())+" !";
    if(sd->needsUpdate())
     return QString("%1").arg(sd->segmentId())+" *";
//    }
    return sd->segmentId();
   case NAME:
       return sd->description();
   case ONEWAY:
       return sd->oneWay();
   case USAGE:
       return sd->trackUsage();
   case TRACTIONTYPE:
       //return sd->tractionType();
       return tractionTypes.value(sd->tractionType()).description;
   case TRACKS:
       return sd->tracks();
   case TYPE:
       return SegmentData::ROUTETYPES.at(sd->routeType());
   case NEXT:
       return sd->next();
   case PREV:
       return sd->prev();
   case NEXTR:
       return sd->nextR();
   case PREVR:
       return sd->prevR();
   case DIR:
       return sd->direction();
   case SEQ:
       return sd->sequence();
   case RSEQ:
    return sd->returnSeq();
   case NE:
    return turnMap.value(sd->normalEnter());
   case NL:
    return turnMap2.value(sd->normalLeave());
   case RE:
    return turnMap.value(sd->reverseEnter());
   case RL:
    return turnMap2.value(sd->reverseLeave());
   case STARTDATE:
       return sd->startDate().toString("yyyy/MM/dd");
   case ENDDATE:
       return sd->endDate().toString("yyyy/MM/dd");
   case DISTANCE:
       return sd->length();
   case ANGLES:
    return sd->bearingStart().angle();
   case ANGLEE:
    return sd->bearingEnd().angle();
  }
 }
 return QVariant();
}

QVariant RouteViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 if (role != Qt::DisplayRole)
    return QVariant();
 if(orientation == Qt::Vertical && role == Qt::DisplayRole)
 {
  SegmentData* sd = listOfSegments.at(section);
  return sd->description();
 }
 if(orientation == Qt::Vertical && role == Qt::BackgroundRole)
 {
  QVariant background = QVariant();
  if ( section == startRow)
  {
   background = QVariant( QColor(Qt::green));
  }

  if ( section == endRow)
  {
   background =  QVariant( QColor(Qt::red) );
  }
  return background;
 }
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
            return tr("Trks");
        case USAGE:
            return tr("Use");
        case TRACTIONTYPE:
            return tr("Tr Type");
        case TYPE:
            return tr("Rt Type");
        case NEXT:
            return tr("Next->");
        case PREV:
            return tr("Prev->");
        case NEXTR:
            return tr("Next<-");
        case PREVR:
            return tr("Prev<-");
        case DIR:
            return tr("Dir");
        case SEQ:
            return tr("Seq");
        case RSEQ:
            return tr("RSeq");
        case NE:
         return tr(" From");
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
        case ANGLES:
         return tr("Start Angle");
        case ANGLEE:
         return tr("End Angle");
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
     SegmentData* sd = nullptr;
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

  SegmentData* sd = listOfSegments.value(row);
  SegmentData oldSd = SegmentData(*sd);

  QDate dt;
//  int tracks;
  switch(index.column())
  {
   case ONEWAY:
   {
    QString s = value.toString().toUpper();
    if(s == "N" || s== "Y" || s == " ")
     sd->setOneWay(s);
    if(s == "N")
    {
     sd->setTrackUsage(" ");
    }
   }
   break;
  case USAGE:
  {
   QString s = value.toString().toUpper();
   if(s == "B" || s=="L" || s == "R" || s == " ")
    sd->setTrackUsage(s);
   break;
  }
  case TRACTIONTYPE:
   sd->setTractionType(value.toInt());
   break;
  case TYPE:
   sd->setRouteType((RouteType)value.toInt());
   break;
  case NEXT:
   sd->setNext(value.toInt());
   break;
  case PREV:
   sd->setPrev(value.toInt());
   break;
  case NEXTR:
   sd->setNextR(value.toInt());
   break;
  case PREVR:
   sd->setPrevR(value.toInt());
   break;
  case SEQ:
   sd->setSequence(value.toInt());
   break;
  case RSEQ:
   sd->setReturnSeq(value.toInt());
   break;
  case NE:
   sd->setNormalEnter(value.toInt());
   break;
  case NL:
   sd->setNormalLeave(value.toInt());
   break;
  case RE:
   sd->setReverseEnter(value.toInt());
   break;
  case RL:
   sd->setReverseLeave(value.toInt());
   break;
   case STARTDATE:
      dt = value.toDate();
      if(dt.isValid())
      {

      sd->setStartDate(value.toDate());
      }
      break;
   case ENDDATE:
      dt = value.toDate();
      if(dt.isValid())
      {
//       if(dt >= sd->startDate())
//        sd->setEndDate(value.toDate());
       sd->setEndDate(value.toDate());
      }
      break;
  }
  sd->setNeedsUpdate(true);
  bChangesMade = true;

  listOfSegments.replace(row, sd);
  SQL::instance()->updateRoute(oldSd,*sd);

  selectedRow = row;
  bSelectedRowChanged =true;
//  RowChanged* changeEntry = nullptr;
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
//  for(int i=0; i < changedMap.values().count(); i++)
//  {
//   if(changedMap.values().at(i)->row == selectedRow)
//   {
//    changeEntry = changedMap.values().at(i);
//    break;
//   }
//  }
//  if(changeEntry == nullptr)
//  {
//   changeEntry = new RowChanged();
//   changeEntry->row = row;
//   changeEntry->index = index;
//   changeEntry->bChanged=true;
//   changeEntry->bDeleted = false;
//   changeEntry->segmentId = sd->segmentId();
//   changedMap.values().append(changeEntry);
//  }
//  changeEntry->sd = sd;
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
    case ANGLES:
    case ANGLEE:
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
    case NEXTR: // next reverse direction
    case PREVR: // prev
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

QList< SegmentData* > RouteViewTableModel::getList()
{
    return listOfSegments;
}

void RouteViewTableModel::setList(QList< SegmentData* > segmentDataList)
{
 beginResetModel();
 this->listOfSegments = segmentDataList;
 saveSegmentDataList = QList<SegmentData>();
 for(SegmentData* sd : segmentDataList)
  saveSegmentDataList.append(SegmentData(*sd));
 bChangesMade = false;
 reset();
 endResetModel();
}

void RouteViewTableModel::setSequenced(bool b)
{
 bIsSequenced = b;
}

bool sortbyrow( const RowChanged & s1 , const RowChanged & s2 )
{
    return s1.row > s2.row;
}
#if 0
bool RouteViewTableModel::commitChanges()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);

 // sort rows in descending sequence.
 //qSort(*changedMap.values().begin(),*changedMap.values().end(), sortbyrow );
#if 0
 for(int i=0; i < changedMap.values().count(); i++)
 {
  RowChanged* rc = changedMap.values().at(i);
  int row = rc->row;

  SQL::instance()->beginTransaction("updateRoute");

  if( SQL::instance()->doesRouteSegmentExist(rc->osd->route(), rc->osd->routeName(),
                                             rc->osd->segmentId(), rc->osd->startDate(),
                                             rc->osd->endDate()))
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
     if(!SQL::instance()->deleteRouteSegment(rc->osd->route(), rc->osd->routeName(),
                                            rc->osd->segmentId(), rc->osd->startDate().toString("yyyy/MM/dd"),
                                            rc->osd->endDate().toString("yyyy/MM/dd")))
     {
      SQL::instance()->rollbackTransaction("updateRoute");
      return false;
     }
    }
  }
 }
 SQL::instance()->commitTransaction("updateRoute");

 changedMap.clear();
#else
 for(int i= listOfSegments.count()-1; i >= 0; i--)
 {
  SegmentData* sd = (SegmentData*)listOfSegments.at(i);
  if(sd->markedForDelete())
  {
   SQL::instance()->deleteRoute(*sd);
   listOfSegments.removeAt(i);
   continue;
  }
  if(sd->needsUpdate())
  {
   if(SQL::instance()->updateRoute(saveSegmentDataList.at(i), *sd))
    sd->setNeedsUpdate(false);
  }
 }
#endif
 bChangesMade = false;
 MainWindow::instance()->refreshRoutes();
 //emit refreshRoutes();
 reset();
 MainWindow::instance()->setCursor(Qt::ArrowCursor);
 return true;
}

void RouteViewTableModel::discardChanges()
{
 //listOfSegments = saveSegmentDataList;
 listOfSegments.clear();
 for(SegmentData sd : saveSegmentDataList)
  listOfSegments.append(&sd);
 //changedMap.clear();
 bChangesMade = false;
 reset();
}
#endif
// Called by RouteView to mark segment be deleted.
void RouteViewTableModel::deleteRow(qint32 segmentId, const QModelIndex &index)
{
 if(segmentId == listOfSegments.at(index.row())->segmentId())
 {
  QModelIndex parent = QModelIndex();
  beginRemoveRows(parent, index.row(), index.row());
  //listOfSegments.at(index.row()).markForDelete(true);
  SegmentData* sd = listOfSegments.at(index.row());
  listOfSegments.removeAt(index.row());
  SQL::instance()->deleteRouteSegment(*sd);
  endRemoveRows();
 }
}
#if 0
void RouteViewTableModel::unDeleteRow(qint32 segmentId, const QModelIndex &index)
{
// Q_UNUSED(segmentId)
// for(int i = 0; i < changedMap.values().count(); i++)
// {
//  RowChanged *rc = changedMap.values().at(i);
//  if(rc->index == index && rc->bDeleted)
//  {
//   rc->bDeleted = false;
//   if(!rc->bChanged && changedMap.values().count() == 1)
//   {
//    changedMap.values().clear();
//   }
//   return;
//  }
// }
 if(segmentId == listOfSegments.at(index.row())->segmentId())
 {
  //listOfSegments.at(index.row()).markForDelete(true);
  SegmentData* sd = listOfSegments.at(index.row());
//  sd->markForDelete(false);
//  listOfSegments.replace(index.row(), sd);
  beginRemoveRows(QModelIndex(), index.row(), index.row());
  listOfSegments.removeAt(index.row());
  SQL::instance()->deleteRoute(*sd);
  endRemoveRows();
 }
}
//bool RouteViewTableModel::isSegmentMarkedForDelete(qint32 segmentId)
//{
// for(int i = 0; i < changedMap.values().count(); i++)
// {
//  RowChanged *rc = changedMap.values().at(i);
//  if(rc->segmentId == segmentId && rc->bDeleted)
//   return true;
// }
// return false;
//}
#endif
int RouteViewTableModel::getRow(int segmentId)
{
 int row = -1;
 if(listOfSegments.isEmpty())
  return row;
 for (row = 0; row < listOfSegments.count(); row++)
 {
  if(listOfSegments.at(row)->segmentId() == segmentId)
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

SegmentData* RouteViewTableModel::segmentData(int row)
{
 return listOfSegments.at(row);
}

void RouteViewTableModel::clear()
{
    listOfSegments.clear();
    reset();
}
