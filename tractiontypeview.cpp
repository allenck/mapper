#include "tractiontypeview.h"
#include <QMenu>
#include <QString>

TractionTypeView::TractionTypeView(Configuration *cfg, QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = Configuration::instance();
    //sql->setConfig(config);
    sql = SQL::instance();
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    tableView = myParent->ui->tblTractionTypes;
    tableView->verticalHeader()->resize(2,20);

    QSqlDatabase db = QSqlDatabase::database();
    qDebug()<<db.databaseName();
    connect(tableView->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));
    tableView->setAlternatingRowColors(true);

    model = new MyTractionTypesTableModel(this, db);
    model->setTable("TractionTypes");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->query().setForwardOnly(false);
    model->select();
    QString name = model->record(0).value("description").toString();
    tableView->setModel(model);

    menu = new QMenu();
    addAct = new QAction(tr("Add Traction Type"),this);
    addAct->setToolTip(tr("Add a new traction type"));
    connect(addAct, SIGNAL(triggered()), this, SLOT(newRecord()));
    delAct = new QAction(tr("Delete Traction Type"),this);
    delAct->setToolTip(tr("Delete an existing traction type"));
    connect(delAct, SIGNAL(triggered()), this, SLOT(delRecord()));

    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));

    tableView->show();
}

void TractionTypeView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)
    tableView->resizeColumnsToContents ();
    tableView->resizeRowsToContents ();
}
MyTractionTypesTableModel::MyTractionTypesTableModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent, db)
{

}

QVariant MyTractionTypesTableModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();

 // We only wish to override the foreground role
 QSqlRecord r = record();
 if (role == Qt::ForegroundRole  && index.column() == r.indexOf("displayColor"))
 {
  //sourceIndex = mapToSource(index);
  qint32 row = index.row();
  QString value = record(row).value("displayColor").toString();
  QColor color(value);
  return QVariant( color);
 }
 // let the base class handle all other cases
 return QSqlTableModel::data( index, role );
}

bool MyTractionTypesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 QString text;
 QSqlRecord r = QSqlRecord();
 if(role == Qt::EditRole)
 {
  if(r.indexOf("displayColor") == index.column())
  {
   text = value.toString();
   if(text.at(0) != '#' || text.length() != 7) return false;
   for(int i= 1; i <= 6; i++)
   {
    if(!text.at(i).isDigit()) return false;
   }
  }
  if(r.indexOf("routeType")== index.column())
  {
   if(!to_enum(value.toInt()))
    return false;
  }
 }
 return QSqlTableModel::setData( index, value, role );
}

void TractionTypeView::clear()
{
 model->clear();
}

void TractionTypeView::newRecord()
{
 QSqlRecord record = model->record();
 record.setValue(1,QVariant(tr("new traction type name")));
 record.setValue(2,QVariant("#FF00FF"));
 record.setValue(3,QVariant("0"));
 record.setValue(4,QVariant(""));
 record.setValue(5,QVariant(QDateTime::currentDateTime()));
 model->insertRecord(-1,record);
 currentIndex = model->index(model->rowCount()-1,0);
}

void TractionTypeView::delRecord()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
 QItemSelectionModel * selectionModel = tableView->selectionModel();
 QModelIndexList indexes = selectionModel->selectedIndexes();
 QModelIndex Index = indexes.at(0);
 qint32 segmentId = Index.data().toInt();

 model->removeRow(Index.row());
}

//get QTableView selected item
bool TractionTypeView::boolGetItemTableView(QTableView *table)
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
//create table input context menu
void TractionTypeView::tablev_customContextMenu( const QPoint& pt)
{
 menu->clear();
 curRow = tableView->rowAt(pt.y());
 curCol = tableView->columnAt(pt.x());
 // check is item in QTableView exist or not
 if(boolGetItemTableView(tableView))
 {
  menu->addAction(addAct);
  menu->addAction(delAct);
 }
 menu->exec(QCursor::pos());

}
