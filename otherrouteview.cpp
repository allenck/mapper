#include "otherrouteview.h"
#include "sql.h"

OtherRouteView* OtherRouteView::_instance = NULL;

OtherRouteView::OtherRouteView( QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = Configuration::instance();
    //sql->setConfig(config);
    sql = SQL::instance();
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    ui = myParent->ui->tblOtherRouteView;

    ui->setAlternatingRowColors(true);
    ui->resizeColumnsToContents();
    ui->setColumnWidth(0,25);
    //m_myParent = myParent;
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );

    sourceModel = new OtherRouteViewTableModel();
    proxymodel = new otherRouteViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->setModel(proxymodel);
    //connect(this, SIGNAL(sendRows(int, int)), proxymodel, SLOT(getRows(int,int)));
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    myParent->ui->tabWidget->setTabText(2, tr("Like Routes"));
    _instance = this;

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
}

OtherRouteView* OtherRouteView::instance( QObject* parent)
{
 if(_instance == NULL)
 {
  _instance = new OtherRouteView(parent);
 }
 return _instance;
}

void OtherRouteView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)

    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
}

void OtherRouteView::showRoutesUsingSegment(qint32 segmentId)
{
    //SQL sql;
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    //sql->setConfig(myParent->getConfiguration());
    QList<SegmentData> likeRoutes =   sql->getRoutes(segmentId);
    if(likeRoutes.isEmpty())
        return;
    SegmentInfo sd = sql->getSegmentInfo(segmentId);
    if(sd.segmentId()<1)
        return;
    ui->setSortingEnabled(false);
    myParent->ui->likeRoutesLabel->setText(tr("Routes using segment #%1 %2").arg(sd.segmentId()).arg(sd.description()));


    sourceModel = new OtherRouteViewTableModel(likeRoutes, sd, this);
    proxymodel->setSourceModel(sourceModel);

    ui->setModel(proxymodel);
    ui->setSortingEnabled(true);

    ui->resizeColumnsToContents();
    ui->horizontalHeader()->setStretchLastSection(true);
    ui->horizontalHeader()->resizeSection(0,60);
    //ui->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
//    ui->horizontalHeader()->resizeSection(2,QHeaderView::ResizeToContents);
//    ui->horizontalHeader()->resizeSection(3,QHeaderView::ResizeToContents);
//    ui->horizontalHeader()->resizeSection(4,QHeaderView::ResizeToContents);
//    ui->horizontalHeader()->resizeSection(5,QHeaderView::ResizeToContents);
//    ui->horizontalHeader()->resizeSection(6,QHeaderView::ResizeToContents);

    myParent->ui->tabWidget->setTabText(2, tr("Like Routes"));
    displayRouteAct = new QAction("Display route", this);
    connect(displayRouteAct, SIGNAL(triggered(bool)), this, SLOT(On_displayRouteAct_triggered(bool)));
}

//create table input context menu
void OtherRouteView::tablev_customContextMenu( const QPoint& pt)
{
    curRow = ui->rowAt(pt.y());
    curCol = ui->columnAt(pt.x());
    menu.clear();
    // check is item in QTableView exist or not
    if(boolGetItemTableView(ui))
    {
        QItemSelectionModel * model = ui->selectionModel();
        QModelIndexList indexes = model->selectedIndexes();
        //qint32 col =model->currentIndex().column();
        QModelIndex modelIndex = indexes.at(0);
        QModelIndex sourceModelIndex = ((QSortFilterProxyModel*)ui->model())->mapToSource(modelIndex);
        qint32 row = sourceModelIndex.row();
        QList<SegmentData> list = sourceModel->getList();
        rd = list.at(row);

        menu.addAction(displayRouteAct);
        menu.exec(QCursor::pos());
    }
}

//get QTableView selected item
bool OtherRouteView::boolGetItemTableView(QTableView *table)
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

void OtherRouteView::On_displayRouteAct_triggered(bool)
{
 emit displayRoute(rd);
}

otherRouteViewSortProxyModel::otherRouteViewSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool otherRouteViewSortProxyModel::lessThan(const QModelIndex &left,
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

QVariant otherRouteViewSortProxyModel::data ( const QModelIndex & index, int role ) const
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

OtherRouteViewTableModel::OtherRouteViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

OtherRouteViewTableModel::OtherRouteViewTableModel(QList<SegmentData> routeIntersectList, SegmentData sdIn, QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfRoutes = routeIntersectList;
     sd = sdIn;
 }

 int OtherRouteViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfRoutes.size();
 }

 int OtherRouteViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 8;
 }

 QVariant OtherRouteViewTableModel::data(const QModelIndex &index, int role) const
 {
     SQL* sql = SQL::instance();
     if (!index.isValid())
         return QVariant();

     if (index.row() >= listOfRoutes.size() || index.row() < 0)
         return QVariant();
//     if(role == Qt::CheckStateRole)
//     {
//         if(index.column() != 0)
//             return QVariant();
//         else
//         {
//             segmentData sd = listOfSegments.at(index.row());
//             segmentInfo si = sql->getSegmentInfo(sd.SegmentId);
//            if (sql->isRouteUsedOnDate(m_routeNbr, sd.SegmentId, m_date))
//                return Qt::Checked;
//            else
//                return Qt::Unchecked;
//         }
//     }

     SegmentData rd = listOfRoutes.at(index.row());
     if (role == Qt::DisplayRole) {
         switch(index.column())
         {
         case 0:
             //TODO setup checkbox if segment used in route.
             return rd.alphaRoute();
         case 1:
             return rd.routeName(); // route name
         case 2:
             return rd.startDate().toString("yyyy/MM/dd");
         case 3:
             return rd.endDate().toString("yyyy/MM/dd");
//         case 4:
//             return si.description; // segment name
         case 4:
             switch(rd.normalEnter())
             {
                case 1:
                 return "left";
                case 0:
                 return "back";
                case 2:
                 return "right";
                default:
                 return "???";
             }

         case 5:
             switch(rd.normalLeave())
             {
                case 1:
                 return "left";
                case 0:
                 return "Ahead";
                case 2:
                 return "right";
                default:
                 return "???";
             }
            case 6:
             if(sd.oneWay() == "N")
             {
                 switch(rd.reverseEnter())
                 {
                    case 1:
                     return "left";
                    case 0:
                     return "back";
                    case 2:
                     return "right";
                    default:
                     return "???";
                 }
             }
             return "na";
            case 7:
             if(sd.oneWay() == "N")
             {
                 switch(rd.reverseLeave())
                 {
                    case 1:
                     return "left";
                    case 0:
                     return "Ahead";
                    case 2:
                     return "right";
                    default:
                     return "???";
                 }
             }
             return "na";
        }
     }
     return QVariant();
 }
 QVariant OtherRouteViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();
//  "Item" << "Name" << "1 way" << "Next" << "Prev" << "Dir" << "Seq" << "RSeq" << "StartDate" << "EndDate"
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case 0:
                 return tr("Route");
             case 1:
                 return tr("Rt Name");
         case 2:
          return tr("Begin date");
             case 3:
                 return tr("End date");
//         case 4:
//          return tr("Segment name");
             case 4:
                 return tr("From");
             case 5:
                 return tr("To");
             case 6:
                return tr("Reverse From");
             case 7:
                return tr("Reverse To");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

 bool OtherRouteViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginInsertRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; row++) {
         //QPair<QString, QString> pair(" ", " ");
         SegmentData rd;
         listOfRoutes.insert(position, rd);
     }

     endInsertRows();
     return true;
 }

 bool OtherRouteViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listOfRoutes.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool OtherRouteViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         SegmentData rd = listOfRoutes.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;

         listOfRoutes.replace(row, rd);
         emit(dataChanged(index, index));

         return true;
     }

     return false;
 }
 void OtherRouteViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags OtherRouteViewTableModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< SegmentData > OtherRouteViewTableModel::getList()
 {
     return listOfRoutes;
 }

