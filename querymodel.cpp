#include "querymodel.h"
#include "qsqltablemodel.h"
#include "ui_querydialog.h"
#include <QDate>
#include <QDateTime>
#include <QSqlRecord>
#include <QDebug>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include <QAction>
#include <QMenu>
#include <querydialog.h>

class QSqlRecord;

QueryModel::QueryModel(QObject *parent, QSqlDatabase db, QString dbtype) :
    QSqlQueryModel(parent)
{
 driver=db.driverName();
 dbType = dbtype;
 qDebug() << "connection name:" << db.connectionName();
 qDebug() << "driver:" << db.driverName() << " database:" << db.databaseName();
}

void QueryModel::sort( int column, Qt::SortOrder order)
{
 //table_blobs->on_init_list(-1);
 return QSqlQueryModel::sort(column, order);
}

QVariant QueryModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const
{
 if (!index.isValid())
  return QVariant();
 QSqlRecord this_record; // Do not define vars in a case statement
 QString s_Result;             // Do not define vars in a case statement
 QVariant value = QSqlQueryModel::data(index, role);
 QVariant::Type Type= value.type();

 switch(role)
 {
  case Qt::DisplayRole: // Qt::DisplayRole 0
   this_record=record(index.row());
   // qDebug()<<"QGeomCollTableModel::edit1  row["<<index.row()<<"] column["<<index.column()<<"] ";
//   s_Result=table_blobs->on_check_blob_list(QSqlQueryModel::rowCount(),&this_record,index.row(),index.column());
//   if (s_Result != "")
//   {
//    // qDebug()<<"QGeomCollTableModel::edit2  row["<<index.row()<<"] column["<<index.column()<<"] result["<<s_Result<<"]";
//    if (!(s_Result.contains("Hex Blob") && Type == QMetaType::QString))
//     return s_Result;
//   }
   // get the actual value from base class
   {
    switch(Type)
    {
    case QMetaType::QDate:
     {
      QDate date = value.toDate();
      value = date.toString("yyyy-MM-dd");
      return value;
     }
    case QMetaType::QDateTime:
     {
      QDateTime datetime = value.toDateTime();
      value = datetime.toString("yyyy-MM-dd hh:mm:ss");
      return value;
     }
    case QMetaType::Double:
     {
      double val = value.toDouble();
      return QString("%1").arg(val,2, 'g',10);
     }
    default:
     break;
    }
   }

   // qDebug()<<"QGeomCollTableModel::edit3  row["<<index.row()<<"] column["<<index.column()<<"] result["<<s_Result<<"]";
   break;
  case Qt::DecorationRole: // Qt::DecorationRole  1
   break;
  case Qt::EditRole: // :Qt::EditRole  2
   break;
  case Qt::ToolTipRole: // Qt::ToolTipRole 3
   break;
  case Qt::StatusTipRole: // Qt::StatusTipRole  4
   break;
  case Qt::WhatsThisRole: // Qt::WhatsThisRole  5
   break;
  case Qt::FontRole: // Qt::FontRole 6
   break;
  case Qt::TextAlignmentRole: // Qt::TextAlignmentRole 7
   break;
  case Qt::BackgroundRole: // Qt::BackgroundColorRole  8
   break;
  case Qt::ForegroundRole: // Qt::ForegroundRole 9
   break;
  case Qt::CheckStateRole: // Qt::CheckStateRole  10
   break;
  case Qt::AccessibleTextRole: // Qt::AccessibleTextRole 11
   break;
  case Qt::AccessibleDescriptionRole: // Qt::AccessibleDescriptionRole 12
   break;
  case Qt::SizeHintRole: // Qt::SizeHintRole 13
   break;
  case Qt::UserRole: // Qt::UserRole 32 The first role that can be used for application-specific purposes.
   // - up to Qt::MaxUserRole ['MaxuserRole'] is not a member of 'QT'
   break;
 }
 if ((role > 31) && (role < 10000))
  qDebug()<<"QGeomCollTableModel::data  role["<<role<<"]";
 return QSqlQueryModel::data(index, role);
}
Qt::ItemFlags QueryModel::flags(const QModelIndex &index) const
{
// QString s_Result=table_blobs->on_data_blob_list(index.row(),index.column(),NULL);
// if (s_Result != "")
// {
//  // If this is NOT a Blob, use normal settings
//  return  QAbstractTableModel::flags(index);
// }
 // A Blob should not be editble after a doubleClick
 Qt::ItemFlags flags = QAbstractTableModel::flags(index);
 flags |= Qt::ItemIsEditable;
 return flags;
}
QVariant QueryModel::edit(const QModelIndex &index) const
{
 if (!index.isValid())
  return QVariant();
// QString s_Result=table_blobs->on_data_blob_list(index.row(),index.column(),NULL);
// if (s_Result != "")
// {
//  // Edit Blob
//  QSqlField this_field=record(index.row()).field(index.column());
//  if (!table_blobs->on_edit_blob(&this_field,/*this,*/ QPersistentModelIndex(index)))
//  {
//   //qDebug()<<"QGeomCollTableModel::edit  row["<<index.row()<<"] column["<<index.column()<<"] result["<<s_Result<<"]";
//  }
//  //qDebug()<<"QGeomCollTableModel::edit  row["<<index.row()<<"] column["<<index.column()<<"] result["<<s_Result<<"]";
// }
 return QVariant();
}
void QueryModel::setTabName(QString name)
{
 tabName = name;
}

MyHeaderView::MyHeaderView(Qt::Orientation orientation, QWidget *parent) : QHeaderView(orientation, parent)
{
 myParent = parent;
 view = qobject_cast<QTableView*>(parent);

 bAllowSortColumns = true;
 setContextMenuPolicy(Qt::CustomContextMenu);
// setMovable(true);
 connect(this ,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextMenuRequested(const QPoint)));

}
MyHeaderView::~MyHeaderView()
{

}
void MyHeaderView::setMoveAllowed(bool b)
{ bAllowSortColumns = b; }

void MyHeaderView::mousePressEvent(QMouseEvent *e)
{
 if(!bAllowSortColumns)
  return QHeaderView::mousePressEvent(e);
 if(e->button() == Qt::LeftButton)
 {
  QPoint pt = e->pos();
  int lIndex = logicalIndexAt(pt);
  int index = visualIndex(lIndex);
  //qDebug()<< QString("mouse press event section %1 Logical Index:  %2").arg(index).arg(lIndex);
  //qDebug()<<QString("at x: %1, y: %2").arg(pt.x()).arg(pt.y());
  fromCol = index;
  this->setCursor(Qt::DragMoveCursor);
 }
}

void MyHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
 if(!bAllowSortColumns)
  return QHeaderView::mouseReleaseEvent(e);

 if(e->button() == Qt::LeftButton)
 {
  QPoint pt = e->pos();
  int lIndex = logicalIndexAt(pt);
  int index = visualIndex(lIndex);
  //qDebug()<< QString("mouse release event section %1 Logical Index:  %2").arg(index).arg(lIndex);
  //qDebug()<<QString("at x: %1, y: %2").arg(pt.x()).arg(pt.y());
  if(fromCol != index)
   moveSection(fromCol, index);
  else
  {
   //emit sectionClicked(lIndex);
   //emit sectionPressed(lIndex);
   QTableView *view = qobject_cast<QTableView*>(myParent);
   QSortFilterProxyModel *proxyModel =  qobject_cast<QSortFilterProxyModel *>(view->model());
   int sortOrder= proxyModel->sortOrder();
   int sortColumn = proxyModel->sortColumn();
   if(sortColumn == index)
   {
    if(sortOrder == Qt::AscendingOrder)
     sortOrder = Qt::DescendingOrder;
    else
     sortOrder = Qt::AscendingOrder;
   }
   else
    sortOrder = Qt::AscendingOrder;
   proxyModel->sort(index, (Qt::SortOrder)sortOrder);
   setSortIndicator(lIndex, (Qt::SortOrder)sortOrder);
  }
  this->setCursor(Qt::ArrowCursor);
 }
}
void MyHeaderView::mouseMoveEvent(QMouseEvent *e)
{
 if(!bAllowSortColumns)
  return QHeaderView::mouseMoveEvent(e);

 if(e->buttons() & Qt::LeftButton)
 {
  this->setCursor(Qt::DragMoveCursor);
 }
}
void MyHeaderView::contextMenuRequested(const QPoint &pt)
{
 //QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());

 QAction *hideColumn = new QAction(tr("Hide Column"),this);
 QAction *showHiddenColumns = new QAction(tr("Show Hidden Columns"),this);
 //myHeaderView* hv = (myHeaderView*)view->horizontalHeader();
 QAction *moveOrResize = new QAction(bAllowSortColumns?tr("Allow resize columns"):tr("Allow sort/drag/move columns"),this);
 QAction *resizeToData = new QAction(tr("Resize to data"), this);
 connect(hideColumn, SIGNAL(triggered()),this,SLOT(on_queryView_hide_column()));
 connect(showHiddenColumns,SIGNAL(triggered()),this,SLOT(on_queryView_show_columns()));
 connect(moveOrResize, SIGNAL(triggered()),this,SLOT(onMoveOrRezize_columns()));
 connect(resizeToData, SIGNAL(triggered()),this,SLOT(onResizeToData()));
 QAction* sortAction = new QAction(tr("sort on column"),this);
 connect(sortAction, &QAction::triggered, this, [=]{
     on_sortAction();
 });
 QMenu menu;
 queryViewCurrColumn = view->columnAt(pt.x());
 menu.addAction(resizeToData);
 menu.addAction(hideColumn);
 menu.addAction(moveOrResize);
 menu.addAction(sortAction);
 for(int i=0; i < view->model()->columnCount(); i++)
 {
  if(view->isColumnHidden(i))
  {
   menu.addAction(showHiddenColumns);
   break;
  }
 }
 menu.exec(QCursor::pos());
}

void MyHeaderView::on_queryView_hide_column()
{
 //QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
 view->hideColumn(queryViewCurrColumn);
}

void MyHeaderView::on_queryView_show_columns()
{
 //QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
 for(int i=0; i < view->model()->columnCount(); i++)
 {
  if(view->isColumnHidden(i))
  {
   view->showColumn(i);
  }
 }
}
void MyHeaderView::onMoveOrRezize_columns()
{
 //QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
 //myHeaderView* hv = (myHeaderView*)view->horizontalHeader();
 bAllowSortColumns = !bAllowSortColumns;
}

void MyHeaderView::onResizeToData()
{
// QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
// myHeaderView* hv =(myHeaderView*) view->horizontalHeader();
 resizeSections(QHeaderView::ResizeToContents);
 QVariantList colList;
 QVariantList mapList;
 for(int i=0; i < view->model()->columnCount(); i++)
 {
  if(view->isColumnHidden(i))
   colList << -(view->columnWidth(i));
  else
   colList << view->columnWidth(i);
  mapList << logicalIndex(i);
 }
}
void MyHeaderView::on_sortAction()
{
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(parent());
    if(!tabWidget)
        return;
    qint32 currTabIndex = tabWidget->currentIndex();
    if(currTabIndex<1)
        return;   // no results present
    QTableView *view = qobject_cast<QTableView*>(tabWidget->currentWidget());
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());

    ((QSqlTableModel*)proxyModel->sourceModel())->setSort(view->currentIndex().column(),Qt::SortOrder::AscendingOrder);
    ((QSqlTableModel*)proxyModel->sourceModel())->select();
}
