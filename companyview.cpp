#include "companyview.h"
#include <QMenu>

CompanyView::CompanyView(Configuration *cfg, QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = Configuration::instance();
    //sql->setConfig(config);
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    tableView = myParent->ui->tblCompanyView;
#if QT_VERSION < 0x050000
    tableView->horizontalHeader()->setMovable(true);

    tableView->horizontalHeader()->setSectionsMovable(true);
#endif

    QSqlDatabase db = QSqlDatabase::database();
    qDebug()<<db.databaseName();
    tableView->setAlternatingRowColors(true);
    connect(tableView->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    model = new MyCompanyTableModel(this, db);
    model->setTable("Companies");
    model->setSort(1, Qt::AscendingOrder);
    connect(model, SIGNAL(primeInsert(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));
    //model->setQuery("select * from Companies");
    connect(model, SIGNAL(beforeUpdate(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));

    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->query().setForwardOnly(false);
    model->select();
    QString name = model->record(0).value("description").toString();
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(model);
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);
    QSqlRecord r = model->record(0);
    int ixRoutePrefix = r.indexOf("routePrefix");
    tableView->horizontalHeader()->moveSection(ixRoutePrefix,2);
    tableView->resizeColumnsToContents();

    bNeedsRefresh = false;
    //connect(tableView, SIGNAL())

    menu = new QMenu();
    addAct = new QAction(tr("Add Company"),this);
    addAct->setToolTip(tr("Add a new company"));
    connect(addAct, SIGNAL(triggered()), this, SLOT(newRecord()));
    delAct = new QAction(tr("Delete Company"),this);
    delAct->setToolTip(tr("Delete an existing company"));
    connect(delAct, SIGNAL(triggered()), this, SLOT(delRecord()));
    refreshAct = new QAction(tr("Refresh "), this);
    refreshAct->setToolTip(tr("Refresh company list"));
    connect(refreshAct, SIGNAL(triggered(bool)), this, SLOT(refresh()));

    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));

    tableView->show();
}

void CompanyView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)
    tableView->resizeColumnsToContents ();
    tableView->resizeRowsToContents ();
}
void CompanyView::refresh()
{
    model->select();
    tableView->show();

    // also refresh Mainwindow Company ComboBox
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    myParent->refreshCompanies();
}

void CompanyView::clear()
{
    model->clear();
}

void CompanyView::newRecord()
{
 QSqlRecord record = model->record();
 record.setValue(1,QVariant(tr("new description")));
 record.setValue(2,QVariant("1890/01/01"));
 record.setValue(3,QVariant(tr("1950/12/31")));
 record.setValue(6, QVariant(""));
 model->insertRecord(-1,record);
 currentIndex = model->index(model->rowCount()-1,0);
 refresh();
}

void CompanyView::delRecord()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
 QItemSelectionModel * selectionModel = tableView->selectionModel();
 QModelIndexList indexes = selectionModel->selectedIndexes();
 QModelIndex Index = indexes.at(0);

 model->removeRow(Index.row());
}

//get QTableView selected item
bool CompanyView::boolGetItemTableView(QTableView *table)
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
void CompanyView::tablev_customContextMenu( const QPoint& pt)
{
 menu->clear();
 curRow = tableView->rowAt(pt.y());
 curCol = tableView->columnAt(pt.x());
 // check is item in QTableView exist or not
 if(boolGetItemTableView(tableView))
 {
  menu->addAction(addAct);
  menu->addAction(delAct);
  menu->addAction(refreshAct);
 }
 menu->exec(QCursor::pos());

}

QVariant MyCompanyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 QVariant value = QSqlTableModel::headerData(section, orientation,role);
 if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
  return hdrMap.value(value.toString());
 return value;
}


QVariant MyCompanyTableModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();

 // let the base class handle all other cases
 return QSqlTableModel::data( index, role );
}

bool MyCompanyTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 QVariant hdr;
 if(role == Qt::EditRole)
 {
  hdr = headerData(index.column(), Qt::Horizontal, Qt::DisplayRole);
 }
 return QSqlTableModel::setData( index, value, role );
}

void CompanyView::On_primeInsert(int, QSqlRecord&)
{
 bNeedsRefresh = true;
 emit dataChanged();
}
