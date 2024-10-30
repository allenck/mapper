#include "locatestreetdlg.h"
#include "ui_locatestreetdlg.h"
#include "mainwindow.h"

LocateStreetDlg::LocateStreetDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocateStreetDlg)
{
    //QTableView* uiTblView = ui->tblStreets;
    ui->setupUi(this);
    this->setWindowTitle(tr("Locate Geodb Object"));

    m_parent = parent;
    //ui = myParent->ui_stationView;
    connect(ui->tblStreets->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int, int)));

    ui->tblStreets->setAlternatingRowColors(true);
    ui->tblStreets->setColumnWidth(0,25);
    //m_myParent = myParent;
    ui->tblStreets->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->tblStreets->setSelectionMode( QAbstractItemView::SingleSelection );

    sourceModel = new GeodbViewTableModel();
    proxymodel = new GeodbViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->tblStreets->setModel(proxymodel);
    //connect(this, SIGNAL(sendRows(int, int)), proxymodel, SLOT(getRows(int,int)));
    connect(ui->tblStreets, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelectionChanged(QModelIndex)));
    connect(ui->txtStreet, SIGNAL(editingFinished()), this, SLOT(txtStreet_Leave()));

}

void LocateStreetDlg::txtStreet_Leave()
{
    GeodbSql sql;

    QString text = ui->txtStreet->text();
    if(text.length() == 0)
        return;
    QList<geodbObject> geodbObjectList = sql.getStreetList(text);

    sourceModel = new GeodbViewTableModel(geodbObjectList, this);
    proxymodel->setSourceModel(sourceModel);

    //QTableView* uiTblView = ui->tblStreets;
    ui->tblStreets->setSortingEnabled(false);
    ui->tblStreets->setModel(proxymodel);
    ui->tblStreets->setSortingEnabled(true);

    ui->tblStreets->horizontalHeader()->setStretchLastSection(false);
    ui->tblStreets->horizontalHeader()->resizeSection(0,60);
    //ui->tblStreets->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
    ui->tblStreets->horizontalHeader()->resizeSection(2,70);
    ui->tblStreets->horizontalHeader()->resizeSection(3,70);
    ui->tblStreets->horizontalHeader()->resizeSection(4, 70);
    ui->tblStreets->horizontalHeader()->resizeSection(5, 70);
    ui->tblStreets->horizontalHeader()->resizeSection(6, 70);
    ui->tblStreets->horizontalHeader()->resizeSection(7, 40);

}

LocateStreetDlg::~LocateStreetDlg()
{
    delete ui;
}
void LocateStreetDlg::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)

    ui->tblStreets->resizeColumnsToContents ();
    ui->tblStreets->resizeRowsToContents ();
}
void LocateStreetDlg::itemSelectionChanged(QModelIndex index )
{
   MainWindow * parent = qobject_cast<MainWindow*>(this->m_parent);
   QModelIndex sourceIndex;
   //qint32 row = index.row();
   double lat=0, lon =0;

   //
   sourceIndex = proxymodel->mapToSource(index);
   qint32 row = sourceIndex.row();
   geodbObject go =  sourceModel->getList().at(row);
   lat = go.latitude;
   lon = go.longitude;
   QVariantList objArray;
   objArray << 0 << lat<<lon<<1<<go.Strasse<<-1;
   parent->m_bridge->processScript("addMarker",objArray);
}
GeodbViewSortProxyModel::GeodbViewSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}
bool GeodbViewSortProxyModel::lessThan(const QModelIndex &left,
const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if(left.data().type() == QVariant::Int)
    {
        return (left.data().toInt() < right.data().toInt());
    }
    else if(leftData.type() == QVariant::String)
    {
        return (leftData.toString() < rightData.toString());
    }
    return false;
}
QVariant GeodbViewSortProxyModel::data ( const QModelIndex & index, int role ) const
{
    QModelIndex sourceIndex;
    if (!index.isValid())
        return QVariant();

    // We only wish to override the background role
//    if (role == Qt::BackgroundRole )
//    {
//        sourceIndex = mapToSource(index);
//        qint32 row = sourceIndex.row();
//        if ( row == startRow)
//        {
//            return QVariant( Qt::green );
//        }

//        if ( row == endRow)
//        {
//            return QVariant( Qt::red );
//        }
//    }
    // let the base class handle all other cases
    return QSortFilterProxyModel::data( index, role );
}

GeodbViewTableModel::GeodbViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}
GeodbViewTableModel::GeodbViewTableModel(QList<geodbObject> objectList,  QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfObjects = objectList;
 }

 int GeodbViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfObjects.size();
 }

 int GeodbViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 8;
 }

 QVariant GeodbViewTableModel::data(const QModelIndex &index, int role) const
 {
     //SQL sql;
     if (!index.isValid())
         return QVariant();

     if (index.row() >= listOfObjects.size() || index.row() < 0)
         return QVariant();
//     if(role == Qt::CheckStateRole)
//     {
//         if(index.column() != 0)
//             return QVariant();
//         else
//         {
//             segmentData sd = listOfSegments.at(index.row());
//             segmentInfo si = sql.getSegmentInfo(sd.SegmentId);
//            if (sql.isRouteUsedOnDate(m_routeNbr, sd.SegmentId, m_date))
//                return Qt::Checked;
//            else
//                return Qt::Unchecked;
//         }
//     }

     geodbObject go = listOfObjects.at(index.row());
     if (role == Qt::DisplayRole) {
         switch(index.column())
         {
         case 0:
             //TODO setup checkbox if segment used in route.
             return go.Strasse;
         case 1:
             return go.PLZ;
         case 2:
             return go.Ortstiel;
         case 3:
             return go.Bezirk;
         case 4:
             return go.Stadt;
         case 5:
             switch(go.Strasse_type)
             {
             case 101100000:
                 return tr("Street");
             case 101104000:
                 return tr("Regional Bahn station");
             case 101105000:
                 return tr("S-Bahn station");
             case 101106000:
                 return tr("U-Bahn station");
             case  101300000:
                 return tr("Park");
             case 101400000:
                 return tr("Bridge");
             default:
                 return QString("%1").arg(go.Strasse_type);
             }

         case 6:
             return go.Strasse_von;
         case 7:
             return go.Strasse_bis;
        }
     }
     return QVariant();
 }
 QVariant GeodbViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();
//  "Item" << "Name" << "1 way" << "Next" << "Prev" << "Dir" << "Seq" << "RSeq" << "StartDate" << "EndDate"
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case 0:
                 return tr("Street");
             case 1:
                 return tr("PLZ");
             case 2:
                 return tr("Ortsteil");
             case 3:
                 return tr("Bezirk");
             case 4:
                 return tr("Stadt");
             case 5:
                 return tr("Type");
             case 6:
                return tr("Date From");
             case 7:
                return tr("Date to");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

 bool GeodbViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginInsertRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; row++) {
         //QPair<QString, QString> pair(" ", " ");
         geodbObject go;
         listOfObjects.insert(position, go);
     }

     endInsertRows();
     return true;
 }

 bool GeodbViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listOfObjects.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool GeodbViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         geodbObject go = listOfObjects.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;

         listOfObjects.replace(row, go);
         emit(dataChanged(index, index));

         return true;
     }

     return false;
 }
 void GeodbViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags GeodbViewTableModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< geodbObject > GeodbViewTableModel::getList()
 {
     return listOfObjects;
 }
// void StationView::itemSelectionChanged(QModelIndex index )
// {
//     //qint32 row = index.row();
//     if(index.column() != 0)
//         return;
//     qint32 stationKey = index.data().toInt();
//     QItemSelectionModel * model = ui->selectionModel();
//     QModelIndexList indexes = model->selectedIndexes();
//     MainWindow * parent = qobject_cast<MainWindow*>(this->m_parent);
//     if(indexes.at(5).data().toDouble() == 0 && indexes.at(6).data().toDouble() == 0)
//     {
//         parent->ProcessScript("getCenter", "");
//         parent->ProcessScript("addStationMarker", QString("%1").arg(parent->m_latitude) + "," + QString("%1").arg(parent->m_longitude)+",true,-1,'" +indexes.at(1).data().toString() + "',"+ indexes.at(0).data().toString()+",null,null,'arrow'");
//     }
//     else
//        parent->ProcessScript("addStationMarker", indexes.at(5).data().toString() + "," + indexes.at(6).data().toString()+",true,-1,'" +indexes.at(1).data().toString() + "',"+ indexes.at(0).data().toString()+",null,null,'arrow'");
//     if(!parent->ui_chkNoPan->isChecked())
//     {
//        parent->ProcessScript("setCenter", indexes.at(5).data().toString() + "," + indexes.at(6).data().toString());
//        parent->ProcessScript("setZoom", "15");
//     }
// }

