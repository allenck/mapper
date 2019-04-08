#include "segmentviewtablemodel.h"
#include "sql.h"

segmentViewTableModel::segmentViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}
segmentViewTableModel::segmentViewTableModel(QList<SegmentInfo> segmentDataList, double Lat, double Lon, qint32 route, QString date, QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfSegments = segmentDataList;
     lat= Lat;
     lon = Lon;
     m_routeNbr = route;
     m_date = date;
 }

 int segmentViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfSegments.size();
 }

 int segmentViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 7;
 }

 QVariant segmentViewTableModel::data(const QModelIndex &index, int role) const
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
             //segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
            if (sql->isRouteUsedOnDate(m_routeNbr, si.segmentId, m_date))
                return Qt::Checked;
            else
                return Qt::Unchecked;
         }
     }

     if (role == Qt::DisplayRole) {
         SegmentInfo si = listOfSegments.at(index.row());

         //segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
         if (si.segmentId < 1)
         {
             qDebug() <<"segmentID " + QString("%1").arg(si.segmentId) + " not found";
             return QVariant();
         }
         if( sql->Distance(lat, lon, si.startLat,si.startLon) < .020)
         {
             si.whichEnd = "S";
             a2 = si.bearingStart.getBearing();
         }
         else
         {
             si.whichEnd = "E";
             a2 = si.bearingEnd.getBearing();
         }
         //diff = angleDiff(a1, a2);
         switch(index.column())
         {
         case 0:
             //TODO setup checkbox if segment used in route.
             return si.segmentId;
         case 1:
             return si.description;
         case 2:
             return si.oneWay;
         case 3:
             return si.streetName;
         case 4:
             if (si.oneWay == "Y")
                 return (si.bearing.strDirection());
             else
                 return (si.bearing.strDirection() + "-" + si.bearing.strReverseDirection());
         case 5:
             if(si.whichEnd == "S")
                 return QString("%1").arg(si.startLat,0,'f',8);
             else
                 return QString("%1").arg(si.endLat,0,'f',8);
         case 6:
             if(si.whichEnd=="S")
                 return QString("%1").arg(si.startLon,0,'f',8);
             else
                 return QString("%1").arg(si.endLon,0,'f',8);
             //TODO  Determine which segments can be safely selected
        }
     }
     return QVariant();
 }

 double segmentViewTableModel::angleDiff(double A1, double A2)
 {
     double difference = A2 - A1;
     while (difference < -180) difference += 360;
     while (difference > 180) difference -= 360;
     return difference;

 }
 QVariant segmentViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();
//  "Item" << "Name" << "1 way" << "Next" << "Prev" << "Dir" << "Seq" << "RSeq" << "StartDate" << "EndDate"
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case 0:
                 return tr("SegId");
             case 1:
                 return tr("Name");
             case 2:
                 return tr("1Way");
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

 bool segmentViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
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

 bool segmentViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listOfSegments.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool segmentViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         SegmentInfo si = listOfSegments.value(row);

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
 void segmentViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags segmentViewTableModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< SegmentInfo > segmentViewTableModel::getList()
 {
     return listOfSegments;
 }

