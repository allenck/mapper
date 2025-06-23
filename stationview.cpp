#include "stationview.h"
#include "sql.h"
#include "webviewbridge.h"
#include <QApplication>
#include "editstation.h"

StationView* StationView::_instance = NULL;

StationView::StationView(Configuration *cfg, QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
//    config = cfg;
//    sql->setConfig(config);
    config = Configuration::instance();
    sql = SQL::instance();
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    ui = myParent->ui->tblStationView;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int, int)));

    ui->setAlternatingRowColors(true);
    ui->setColumnWidth(0,25);
    //m_myParent = myParent;
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );

    sourceModel = new StationViewTableModel();
    proxymodel = new StationViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->setSortingEnabled(true);
    ui->setModel(proxymodel);
    //connect(this, SIGNAL(sendRows(int, int)), proxymodel, SLOT(getRows(int,int)));
    connect(ui, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelectionChanged(QModelIndex)));

    //create contextmenu
    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setStatusTip(tr("Copy Table Location"));
    copyAction->setShortcut(tr("Ctrl+C"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(aCopy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setStatusTip(tr("Paste"));
    pasteAction->setShortcut(tr("Ctrl+V"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(aPaste()));

    displayAction = new QAction(tr("&Display"), this);
    displayAction->setStatusTip(tr("Toggle &display of station marker"));
    displayAction->setShortcut(tr("Ctrl+D"));
    connect(displayAction, SIGNAL(triggered(bool)), this, SLOT(on_DisplayTriggered(bool)));

    editAction = new QAction(tr("&Edit"), this);
    editAction->setStatusTip(tr("&Edit station marker"));
    editAction->setShortcut(tr("Ctrl+E"));
    connect(editAction, SIGNAL(triggered(bool)), this, SLOT(on_editTriggered()));

    deleteAction = new QAction(tr("Delete station"),this);
    connect(deleteAction, &QAction::triggered, [=]{
        int stationKey = currentIndex.model()->index(currentIndex.row(), 0).data().toInt();
        qDebug() << "delete" << stationKey;
        sourceModel->removeRows(currentIndex.row(), currentIndex.row());
        sql->deleteStation(stationKey);
    });

    myParent->ui->tabWidget->setTabText(3, "Stations");
    _instance = this;

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));

}
/*static*/ StationView* StationView::instance()
{
 return _instance;
}

void StationView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)

    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
}

void StationView::showStations(QList<StationInfo> stationList)
{
    //SQL sql;
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    //sql->setConfig( myParent->getConfiguration());
    //QList<StationInfo> stationList =   sql->getStations();
    //QList<StationInfo> stationList =   sql->getStations(myParent->m_alphaRoute, QDate::fromString(myParent->m_currRouteEndDate, "yyyy/MM/dd"));
    if(stationList.isEmpty())
        return;
    ui->setSortingEnabled(true);

    sourceModel = new StationViewTableModel(stationList, this);
    proxymodel->setSourceModel(sourceModel);

    ui->setModel(proxymodel);
    ui->setSortingEnabled(true);

    ui->horizontalHeader()->setStretchLastSection(false);
    ui->horizontalHeader()->resizeSection(0,60);
    //ui->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
    ui->horizontalHeader()->resizeSection(2,30);
    ui->horizontalHeader()->resizeSection(3,70);
    ui->horizontalHeader()->resizeSection(4, 70);
    ui->horizontalHeader()->resizeSection(5, 70);
    ui->horizontalHeader()->resizeSection(6, 70);
    ui->horizontalHeader()->resizeSection(7, 40);
    ui->horizontalHeader()->resizeSection(8, 65);
    ui->horizontalHeader()->resizeSection(9, 40);

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
}

void StationView::changeStation(QString typeOfChg, StationInfo sti)
{
    if(typeOfChg == "add")
        sourceModel->addStation(sti);
    else
        sourceModel->changeStation(sti);
}

//create table input context menu
void StationView::tablev_customContextMenu( const QPoint& pt)
{
 curRow = ui->rowAt(pt.y());
 curCol = ui->columnAt(pt.x());
 // check is item in QTableView exist or not
 if(boolGetItemTableView(ui))
 {
  //menu = QMenu(m_parent*);

  menu.addAction(copyAction);
  menu.addAction(pasteAction);
  menu.addSeparator();
  menu.addAction(displayAction);
  menu.addAction(editAction);
  menu.addAction(deleteAction);

  QItemSelectionModel * model = ui->selectionModel();
  QModelIndexList indexes = model->selectedIndexes();
  //qint32 row = model->currentIndex().row();
  //qint32 col =model->currentIndex().column();
  QModelIndex Index = proxymodel->mapToSource(indexes.at(0));
  QString txtSegmentId = Index.data().toString();
  txtSegmentId.replace("!", "");
  txtSegmentId.replace("*", "");
  qint32 stationKey = txtSegmentId.toInt();
  menu.exec(QCursor::pos());
 }
}
//get QTableView selected item
bool StationView::boolGetItemTableView(QTableView *table)
{
 // get model from tableview
 QItemSelectionModel * model = table->selectionModel();
 if(model)
 {
  currentIndex = model->currentIndex();
  return (true);
 }
 else                //QTableView doesn't have selected data
  return (false);

}
void StationView::aCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    if(currentIndex.isValid())
        clipboard->setText(currentIndex.data().toString());

}

void StationView::aPaste()
{

}

void StationView::on_DisplayTriggered(bool)
{
 if(currentIndex.isValid())
 {
  int stationKey = currentIndex.model()->index(currentIndex.row(), 0).data().toInt();
  QVariantList objArray;
  objArray << stationKey;
  WebViewBridge::instance()->processScript("isStationMarkerDisplayed", objArray);
  while(!WebViewBridge::instance()->isResultReceived())
  {
   qApp->processEvents();
  }
  QString rslt = WebViewBridge::instance()->getRslt().toString();
  qDebug() << tr("stationmarker %1 is displayed %2").arg(stationKey).arg(rslt);

  bool bDisplay = true;
  if(rslt == "false")
  {
   StationInfo sti = sql->getStationInfo(stationKey);
   objArray.clear();
   objArray << sti.latitude << sti.longitude << bDisplay << sti.segmentId << sti.stationName << sti.stationKey << sti.infoKey << "" << sti.markerType;
    WebViewBridge::instance()->processScript("addStationMarker", objArray);
  }
  objArray.clear();
  objArray << stationKey << true;
  WebViewBridge::instance()->processScript("displayStationMarker", objArray);
 }
}

void StationView::on_editTriggered()
{
 if(currentIndex.isValid())
 {
  int stationKey = currentIndex.model()->index(currentIndex.row(), 0).data().toInt();
  StationInfo sti = SQL::instance()->getStationInfo(stationKey);
  EditStation dlg(sti);
  dlg.exec();
 }
}

StationViewSortProxyModel::StationViewSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}
bool StationViewSortProxyModel::lessThan(const QModelIndex &left,
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
QVariant StationViewSortProxyModel::data ( const QModelIndex & index, int role ) const
{
    QModelIndex sourceIndex;
    if (!index.isValid())
        return QVariant();

    // let the base class handle all other cases
    return QSortFilterProxyModel::data( index, role );
}

StationViewTableModel::StationViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}
StationViewTableModel::StationViewTableModel(QList<StationInfo> stationList,  QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfStations = stationList;
 }

void StationViewTableModel::setStationList(QList<StationInfo> stationList)
{
 listOfStations = stationList;
}

 int StationViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfStations.size();
 }

 int StationViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 8;
 }

 QVariant StationViewTableModel::data(const QModelIndex &index, int role) const
 {
     SQL* sql = SQL::instance();
     if (!index.isValid())
         return QVariant();

     if (index.row() >= listOfStations.size() || index.row() < 0)
         return QVariant();

     StationInfo sti = listOfStations.at(index.row());
     if (role == Qt::DisplayRole) {
         switch(index.column())
         {
         case STATIONKEY:
             //TODO setup checkbox if segment used in route.
             return sti.stationKey;
         case NAME:
             return sti.stationName;
         case ROUTES:
             return sti.routes.join(",");
         case STARTDATE:
             return sti.startDate.toString("yyyy/MM/dd");
         case ENDDATE:
             return sti.endDate.toString("yyyy/MM/dd");
         case LATITUDE:
             return QString("%1").arg(sti.latitude, 0, 'f', 8);
         case LONGITUDE:
             return QString("%1").arg(sti.longitude, 0, 'f', 8);
         case MARKER:
             return sti.markerType;
        }
     }
     return QVariant();
 }
 QVariant StationViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();
//  "Item" << "Name" << "1 way" << "Next" << "Prev" << "Dir" << "Seq" << "RSeq" << "StartDate" << "EndDate"
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case STATIONKEY:
                 return tr("StnKey");
             case NAME:
                 return tr("Name");
             case ROUTES:
                 return tr("Routes");
             case STARTDATE:
                 return tr("Start date");
             case ENDDATE:
                 return tr("End Date");
             case LATITUDE:
                 return tr("latitude");
             case LONGITUDE:
                return tr("Longitude");
             case MARKER:
                return tr("Marker");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

 bool StationViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginInsertRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; row++) {
         //QPair<QString, QString> pair(" ", " ");
         StationInfo sti;
         listOfStations.insert(position, sti);
     }

     endInsertRows();
     return true;
 }

 bool StationViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listOfStations.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool StationViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         StationInfo sti = listOfStations.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;

         listOfStations.replace(row, sti);
         emit(dataChanged(index, index));

         return true;
     }

     return false;
 }

 void StationViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags StationViewTableModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< StationInfo > StationViewTableModel::getList()
 {
     return listOfStations;
 }
 void StationView::itemSelectionChanged(QModelIndex index )
 {
     //qint32 row = index.row();
     if(index.column() != 0)
         return;
     //qint32 stationKey = index.data().toInt();
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     MainWindow * parent = qobject_cast<MainWindow*>(this->m_parent);
     if(indexes.at(5).data().toDouble() == 0 && indexes.at(6).data().toDouble() == 0)
     {
         parent->ProcessScript("getCenter", "");
         parent->ProcessScript("addStationMarker", QString("%1").arg(parent->m_latitude) + "," + QString("%1").arg(parent->m_longitude)+",true,-1,'" +indexes.at(1).data().toString() + "',"+ indexes.at(0).data().toString()+",null,null,'arrow'");
     }
     else
        parent->ProcessScript("addStationMarker", indexes.at(5).data().toString() + "," + indexes.at(6).data().toString()+",true,-1,'" +indexes.at(1).data().toString() + "',"+ indexes.at(0).data().toString()+",null,null,'arrow'");
     if(!parent->ui->chkNoPan->isChecked())
     {
        parent->ProcessScript("setCenter", indexes.at(5).data().toString() + "," + indexes.at(6).data().toString());
        parent->ProcessScript("setZoom", "15");
     }
 }
 void StationViewTableModel::addStation(StationInfo sti)
 {
    qint32 row = listOfStations.count();
     insertRow(row , QModelIndex());

     QModelIndex index = this->index(row,0,QModelIndex());
     setData(index, sti.stationKey, Qt::EditRole);
     index = this->index(row, 1, QModelIndex());
     setData(index, sti.stationName, Qt::EditRole);
     index = this->index(row, 2, QModelIndex());
     setData(index, sti.routes, Qt::EditRole);
     index = this->index(row, 3, QModelIndex());
     setData(index, sti.latitude, Qt::EditRole);
     index = this->index(row, 4, QModelIndex());
     setData(index, sti.longitude, Qt::EditRole);
     index = this->index(row, 5, QModelIndex());
//     setData(index, sti.lineSegmentId, Qt::EditRole);
//     index = this->index(row, 6, QModelIndex());
//     setData(index, sti.geodb_loc_id, Qt::EditRole);
//     index = this->index(row, 7, QModelIndex());
//     setData(index, sti.segmentId, Qt::EditRole);

    listOfStations.replace(row,sti);

 }

 void StationViewTableModel::changeStation(StationInfo sti)
 {
  for(int i=0; i < listOfStations.count(); i++)
  {
   if(listOfStations.at(i).stationKey == sti.stationKey)
   {
    listOfStations.replace(i, sti);
    return;
   }
   listOfStations.append(sti);
  }
 }

