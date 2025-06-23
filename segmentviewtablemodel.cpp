#include "segmentviewtablemodel.h"
#include "sql.h"
#include "mainwindow.h"
#include "segmentdescription.h"

SegmentViewTableModel::SegmentViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    common();
}

SegmentViewTableModel::SegmentViewTableModel(QList<SegmentInfo> segmentDataList, double Lat, double Lon, qint32 route, QString date, QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfSegments = segmentDataList;
     checkList(listOfSegments);
     lat= Lat;
     lon = Lon;
     m_routeNbr = route;
     m_date = date;
     common();
 }

 void SegmentViewTableModel::common()
 {
     connect(SQL::instance(), &SQL::segmentChanged, this, [=](const SegmentInfo si){
         int row = getRow(si.segmentId());
         if(row > 0)
         {
             listOfSegments.replace(row, si);
             emit dataChanged(index(row, SEGMENTID), index(row, WHICHEND), QList<int>());
         }
     });
 }

 int SegmentViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfSegments.size();
 }

 int SegmentViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 8;
 }

 QVariant SegmentViewTableModel::data(const QModelIndex &index, int role) const
 {
     SQL* sql = SQL::instance();
     double a2 = 0;
     if (!index.isValid())
         return QVariant();

     if (index.row() >= listOfSegments.size() || index.row() < 0)
         return QVariant();
     if(role == Qt::CheckStateRole)
     {
         if(index.column() != 0)
             return QVariant();
         else
         {
             SegmentInfo si = listOfSegments.at(index.row());
             SegmentData sd = SegmentData(si);
             sd.updateRouteInfo(MainWindow::instance()->_rd);
             //segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
            //if (sql->isRouteUsedOnDate(m_routeNbr, si.segmentId(), m_date))
             if(sql->doesRouteSegmentExist(sd))
                return Qt::Checked;
            else
                return Qt::Unchecked;
         }
     }

     if (role == Qt::DisplayRole || role == Qt::EditRole) {
         SegmentData sd = listOfSegments.at(index.row());

         //segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
         if (sd.segmentId() < 1)
         {
             qDebug() <<"segmentID " + QString("%1").arg(sd.segmentId()) + " not found";
             return QVariant();
         }
         if( sql->Distance(lat, lon, sd.startLat(),sd.startLon()) < .020)
         {
             sd.setWhichEnd("S");
             a2 = sd.bearingStart().angle();
         }
         else
         {
             sd.setWhichEnd("E");
             a2 = sd.bearingEnd().angle();
         }
         //diff = angleDiff(a1, a2);
         switch(index.column())
         {
         case SEGMENTID:
             return sd.segmentId();
         case DESCRIPTION:
             return sd.description();
         case TRACKS:
             return sd.tracks();
         case STREETNAME:
             return sd.streetName();
         case LOCATION:
             return sd.location();
         case STREETID:
             return sd.streetId();
         case DIRECTION:
          return sd.direction();
         case LATLNG:
                if(sd.whichEnd() == "S")
                    return sd.startLatLng().str();
                else
                    return sd.endLatLng().str();
         case WHICHEND:
             return sd.whichEnd();
        }
     }
     return QVariant();
 }

 double SegmentViewTableModel::angleDiff(double A1, double A2)
 {
     double difference = A2 - A1;
     while (difference < -180) difference += 360;
     while (difference > 180) difference -= 360;
     return difference;

 }

 QVariant SegmentViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole || role == Qt::EditRole)
         return QVariant();
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case SEGMENTID:
                 return tr("SegId");
             case DESCRIPTION:
                 return tr("Name");
             case TRACKS:
                 return tr("Tracks");
             case STREETNAME:
                 return tr("Street");
             case LOCATION:
                 return tr("Location");
             case STREETID:
                 return tr("Street Id");
             case DIRECTION:
                 return tr("Dir");
             case LATLNG:
                 return tr("LatLng");
             case WHICHEND:
                 return tr("Which end");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

 int SegmentViewTableModel::getRow(int segmentId)
 {
  int row = -1;
  if(listOfSegments.isEmpty())
   return row;
  for (int i = 0; i < listOfSegments.count(); i++)
  {
   if(listOfSegments.at(i).segmentId() == segmentId)
   {
        row = i;
        break;
   }
  }
  return row;
 }

 bool SegmentViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginInsertRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; row++) {
         //QPair<QString, QString> pair(" ", " ");
         SegmentInfo sd;
         listOfSegments.insert(position, sd);
     }

     endInsertRows();
     return true;
 }

 bool SegmentViewTableModel::removeRows(int position, int nbrRows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+nbrRows-1);

     for (int row=0; row < nbrRows; ++row) {
         listOfSegments.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool SegmentViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         SegmentData sd = listOfSegments.value(row);
         switch(index.column())
         {
         case LOCATION:
             sd.setLocation(value.toString());
             SQL::instance()->updateSegment(&sd);
             break;
         }

         listOfSegments.replace(row, sd);
         emit(dataChanged(index, index));

         return true;
     }

     return false;
 }
 void SegmentViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags SegmentViewTableModel::flags(const QModelIndex &index) const
 {
     switch (index.column()) {
     case LOCATION:
         return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
     default:
         break;
     }
     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< SegmentInfo >* SegmentViewTableModel::getList()
 {
     return &listOfSegments;
 }

 void SegmentViewTableModel::setList(QList<SegmentInfo> list)
 {
     checkList(list);
     beginResetModel();
     listOfSegments = list;
     endResetModel();
 }

 SegmentInfo SegmentViewTableModel::selectedSegment(int row)
 {
  return listOfSegments.at(row);
 }

 void SegmentViewTableModel::checkList(QList<SegmentInfo> segmentDataList)
 {
     for(int i =0; i < segmentDataList.count(); i++)
     {
         SegmentInfo si = segmentDataList.at(i);
         sd = new SegmentDescription(si.description());
         if(si.streetName() != sd->street() && sd->isValid())
         {
             si.setDescription(sd->newDescription());
             segmentDataList.replace(i, si);
             SQL::instance()->updateSegment(&si,false);
         }
     }
 }

 void SegmentViewTableModel::updateRow(int row, SegmentInfo si)
 {
     listOfSegments.replace(row, si);
     emit dataChanged(index(row, SEGMENTID), index(row,WHICHEND ),QList<int>());
 }

