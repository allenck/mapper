#include "segmentviewtablemodel.h"
#include "sql.h"

SegmentViewTableModel::SegmentViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

SegmentViewTableModel::SegmentViewTableModel(QList<SegmentData> segmentDataList, double Lat, double Lon, qint32 route, QString date, QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfSegments = segmentDataList;
     lat= Lat;
     lon = Lon;
     m_routeNbr = route;
     m_date = date;
 }

 int SegmentViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfSegments.size();
 }

 int SegmentViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 7;
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
             SegmentData sd = listOfSegments.at(index.row());
             //segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
            if (sql->isRouteUsedOnDate(m_routeNbr, sd.segmentId(), m_date))
                return Qt::Checked;
            else
                return Qt::Unchecked;
         }
     }

     if (role == Qt::DisplayRole) {
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
             a2 = sd.bearingStart().getBearing();
         }
         else
         {
             sd.setWhichEnd("E");
             a2 = sd.bearingEnd().getBearing();
         }
         //diff = angleDiff(a1, a2);
         switch(index.column())
         {
         case SEGMENTID:
             //TODO setup checkbox if segment used in route.
             return sd.segmentId();
         case DESCRIPTION:
             return sd.description();
         case TRACKS:
             return sd.tracks();
         case STREETNAME:
             return sd.streetName();
         case DIRECTION:
//             if (sd.oneWay() == "Y")
//                 return (sd.bearing().strDirection());
//             else
//                 return (sd.bearing().strDirection() + "-" + sd.bearing().strReverseDirection());
          return sd.direction();
         case LAT:
             if(sd.whichEnd() == "S")
                 return QString("%1").arg(sd.startLat(),0,'f',8);
             else
                 return QString("%1").arg(sd.endLat(),0,'f',8);
         case LON:
             if(sd.whichEnd()=="S")
                 return QString("%1").arg(sd.startLon(),0,'f',8);
             else
                 return QString("%1").arg(sd.endLon(),0,'f',8);
             //TODO  Determine which segments can be safely selected
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
     if (role != Qt::DisplayRole)
         return QVariant();
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case 0:
                 return tr("SegId");
             case 1:
                 return tr("Name");
             case 2:
                 return tr("Tracks");
             case 3:
                 return tr("Street");
             case 4:
                 return tr("Dir");
             case 5:
                return tr("Latitude");
             case 6:
                return tr("Longitude");
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
  for (row = 0; row < listOfSegments.count(); row++)
  {
   if(listOfSegments.at(row).segmentId() == segmentId)
   {
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

 bool SegmentViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
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

         SegmentData si = listOfSegments.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;

         listOfSegments.replace(row, si);
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
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< SegmentData > SegmentViewTableModel::getList()
 {
     return listOfSegments;
 }

 SegmentData SegmentViewTableModel::selectedSegment(int row)
 {
  return listOfSegments.at(row);
 }
