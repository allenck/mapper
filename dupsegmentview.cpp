#include "dupsegmentview.h"
#include "sql.h"
#include <QMessageBox>
#include "exceptions.h"

DupSegmentView::DupSegmentView(Configuration *cfg, QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = cfg;
    //sql->setConfig(config);
    sql = SQL::instance();
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    ui = myParent->ui->tblDupSegments;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    ui->setAlternatingRowColors(true);
    ui->setColumnWidth(0,25);
    //m_myParent = myParent;
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );

    sourceModel = new dupSegmentViewTableModel();
    proxymodel = new dupSegmentViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->setModel(proxymodel);

    myParent->ui->tabWidget->setTabText(6, tr("Duplicate Segments"));

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
}

void DupSegmentView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)

    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
}

void DupSegmentView::showDupSegments(QList<SegmentData> dupSegmentList)
{
    //SQL sql;
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    sql->setConfig(myParent->getConfiguration());
    if(dupSegmentList.isEmpty())
        return;
    ui->setSortingEnabled(false);
    ui->show();

    sourceModel = new dupSegmentViewTableModel(dupSegmentList, this);
    proxymodel->setSourceModel(sourceModel);

    ui->setModel(proxymodel);
    ui->setSortingEnabled(true);

    ui->horizontalHeader()->setStretchLastSection(true);
    ui->horizontalHeader()->resizeSection(0,60);
    //ui->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
    ui->horizontalHeader()->resizeSection(2,QHeaderView::ResizeToContents);
    ui->horizontalHeader()->resizeSection(3,QHeaderView::ResizeToContents);
    ui->horizontalHeader()->resizeSection(4,QHeaderView::ResizeToContents);
    ui->horizontalHeader()->resizeSection(5,QHeaderView::ResizeToContents);

    myParent->ui->tabWidget->setTabText(6, tr("Duplicate Segments"));
    selectSegmentAct = new QAction(tr("SelectSegment"),this);
    connect(selectSegmentAct, SIGNAL(triggered(bool)), this, SLOT(On_selectSegmentAct(bool)));
    deleteDuplicateAct = new QAction(tr("Delete dup segment"),this);
    connect(deleteDuplicateAct, &QAction::triggered, [=]{
     QString msg = tr("Do you want to delete segment %1 which is duplicate to segment %2"
        " and use %2 instead in Routes and Stations? Or delete %2 and use %1 instead?").arg(dupSegmentId).arg(segmentId);
     msg = msg + QString(tr("\nSegment %1 is used by %2 routes").arg(dupSegmentId).arg(sql->getCountOfRoutesUsingSegment(dupSegmentId)));
     msg = msg + QString(tr("\nSegment %1 is used by %2 routes").arg(segmentId).arg(sql->getCountOfRoutesUsingSegment(segmentId)));

     QMessageBox msgBox(QMessageBox::Question, "Delete segment?", msg);
     QPushButton* delete1Button = msgBox.addButton(tr("Delete segment %1").arg(dupSegmentId), QMessageBox::ActionRole);
     QPushButton* delete2Button = msgBox.addButton(tr("Delete segment %1").arg(segmentId), QMessageBox::ActionRole);
     QPushButton* abortButton =  msgBox.addButton(QMessageBox::Abort);
     msgBox.setDefaultButton(abortButton);

     QString detailedText;
     QList<SegmentData> list = sql->getRouteSegmentsBySegment(dupSegmentId);
     if(list.count())
     {
      detailedText = detailedText + tr("segmentId %1 is used by routes:").arg(dupSegmentId);
      foreach(SegmentData sd, list)
      {
       detailedText = detailedText + tr("\n%1 %2 %3->%4").arg(sd.route()).arg(sd.routeName())
         .arg(sd.startDate().toString("yyyy/MM/dd")).arg(sd.endDate().toString("yyyy/MM/dd"));
      }
     }
     else
     {
      detailedText = detailedText + tr("segmentId %1 is not used!").arg(dupSegmentId);
     }

     list = sql->getRouteSegmentsBySegment(segmentId);
     if(list.count())
     {
      detailedText = detailedText + tr("\n\nsegmentId %1 is used by routes:").arg(segmentId);
      for(int i=0; i < list.count(); i++)
      {
       detailedText = detailedText + tr("\n%1 %2 %3->%4").arg(list.at(i).route()).arg(list.at(i).routeName()).
         arg(list.at(i).startDate().toString("yyyy/MM/dd")).arg(list.at(i).endDate().toString("yyyy/MM/dd"));
      }
     }
     else
     {
      detailedText = detailedText + tr("\nsegmentId %1 is not used!").arg(segmentId);
     }
     msgBox.setDetailedText(detailedText);

     if((sd1.tracks() == 1 && sd2.tracks() == sd1.tracks()) && (sd1.direction() != sd2.direction()) )
     {
      msgBox.setIcon(QMessageBox::Warning);
      msg = msg + tr("\n\nWarning: the two segments are single tracks in opposite directions.");
      msg = msg + tr("\nand one should not deleted unless not used!");
     }
     msgBox.exec();
     if(msgBox.clickedButton() == delete1Button)
     {
      if(sql->deleteAndReplaceSegmentWith(dupSegmentId, segmentId))
      {
       sourceModel->deleteRow(modelRow);
      }
     }
     else if(msgBox.clickedButton() == delete2Button)
     {
      if(sql->deleteAndReplaceSegmentWith(segmentId, dupSegmentId))
      {
       sourceModel->deleteRow(modelRow);
      }
     }

     return;
    });
}

//create table input context menu
void DupSegmentView::tablev_customContextMenu( const QPoint& pt)
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
        qint32 row = modelRow = sourceModelIndex.row();
        QList<SegmentData> list = sourceModel->getList();
        segmentId = list.at(row).segmentId();
        sd1 = list.at(row);
        dupSegmentId = list.at(row).next();
        for(int i = 0; i < list.size(); i++)
         if(list.at(i).segmentId() == dupSegmentId)
         {
          sd2 = list.at(i);
          break;
         }
        currSd = list.at(row);

        menu.addAction(selectSegmentAct);
        menu.addAction(deleteDuplicateAct);
        menu.exec(QCursor::pos());
    }
}

//get QTableView selected item
bool DupSegmentView::boolGetItemTableView(QTableView *table)
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

void DupSegmentView::On_selectSegmentAct(bool)
{
  emit selectSegment(segmentId);
}

dupSegmentViewSortProxyModel::dupSegmentViewSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}
bool dupSegmentViewSortProxyModel::lessThan(const QModelIndex &left,
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
QVariant dupSegmentViewSortProxyModel::data ( const QModelIndex & index, int role ) const
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

dupSegmentViewTableModel::dupSegmentViewTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}
dupSegmentViewTableModel::dupSegmentViewTableModel(QList<SegmentData> dupSegmentList, QObject *parent)
     : QAbstractTableModel(parent)
 {
     listOfSegments = dupSegmentList;
 }

 int dupSegmentViewTableModel::rowCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return listOfSegments.size();
 }

 int dupSegmentViewTableModel::columnCount(const QModelIndex &parent) const
 {
     Q_UNUSED(parent);
     return 6;
 }

 QVariant dupSegmentViewTableModel::data(const QModelIndex &index, int role) const
 {
     //SQL sql;
     if (!index.isValid())
         return QVariant();

     if (index.row() >= listOfSegments.size() || index.row() < 0)
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

     SegmentData sd = listOfSegments.at(index.row());
     if (role == Qt::DisplayRole) {
         switch(index.column())
         {
         case 0:
             //TODO setup checkbox if segment used in route.
             return QString("%1").arg(sd.segmentId());
         case 1:
             return sd.description();
         case 2:
             return sd.tracks();
         case 3:
             return sd.streetName();
         case 4:
//             if (si.oneWay == "Y")
//                 return(si.bearing.strDirection());
//            else
//                 return (si.bearing.strDirection() + "-" + si.bearing.strReverseDirection());
             return sd.direction();
         case 5:
             return sd.next();
        }
     }
     return QVariant();
 }

 QVariant dupSegmentViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();
     if (orientation == Qt::Horizontal)
     {
         switch (section)
         {
             case 0:
                 return tr("Segment");
             case 1:
                 return tr("Name");
             case 2:
                 return tr("Tracks");
             case 3:
                 return tr("Street");
             case 4:
                 return tr("Direction");
             case 5:
                return tr("Dup Segment");
             default:
                 return QVariant();
         }
     }
     return QVariant();
 }

 bool dupSegmentViewTableModel::insertRows(int position, int rows, const QModelIndex &index)
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

 bool dupSegmentViewTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
     Q_UNUSED(index);
     beginRemoveRows(QModelIndex(), position, position+rows-1);

     for (int row=0; row < rows; ++row) {
         listOfSegments.removeAt(position);
     }

     endRemoveRows();
     return true;
 }

 bool dupSegmentViewTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
 {
     Q_UNUSED(value)
     if (index.isValid() && role == Qt::EditRole) {
         int row = index.row();

         SegmentData sd = listOfSegments.value(row);

//         switch (index.column())
//             p.first = value.toString();
//         else if (index.column() == 1)
//             p.second = value.toString();
//         else
//             return false;

         listOfSegments.replace(row, sd);
         emit(dataChanged(index, index));

         return true;
     }

     return false;
 }
 void dupSegmentViewTableModel::reset()
 {
     //QAbstractTableModel::reset();
  beginResetModel();
  endResetModel();
 }

 Qt::ItemFlags dupSegmentViewTableModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
     return QAbstractTableModel::flags(index);
 }

 QList< SegmentData > dupSegmentViewTableModel::getList()
 {
     return listOfSegments;
 }

 void dupSegmentViewTableModel::deleteRow(int row)
 {
  if(row >= listOfSegments.size())
   throw Exception(tr("invalid row %1").arg(row));
  int otherSegment = listOfSegments.at(row).next();
  listOfSegments.removeAt(row);
  for(int i=0; i < listOfSegments.count(); i++)
  {
   if(listOfSegments.at(i).next() == otherSegment)
   {
    listOfSegments.removeAt(i);
    break;
   }
  }
  reset();
 }
