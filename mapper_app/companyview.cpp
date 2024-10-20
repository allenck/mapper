#include "companyview.h"
#include <QMenu>
#include <data.h>

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
    _instance = this;

    QSqlDatabase db = QSqlDatabase::database();
    //qDebug()<<db.databaseName();
    tableView->setAlternatingRowColors(true);
    connect(tableView->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    _model = new MyCompanyTableModel(this, db);
    _model->setTable("Companies");
    _model->setSort(1, Qt::AscendingOrder);
    connect(_model, SIGNAL(primeInsert(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));
    //model->setQuery("select * from Companies");
    connect(_model, SIGNAL(beforeUpdate(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));

    _model->setEditStrategy(QSqlTableModel::OnFieldChange);
    //model->query().setForwardOnly(false);
    _model->select();
    QString name = _model->record(0).value("description").toString();
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(_model);
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);
    QSqlRecord r = _model->record(0);
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

    toggleSelectionAct = new QAction(tr("toggle selection"),this);
    connect(toggleSelectionAct, &QAction::triggered, [=]{
        QModelIndex index = toggleSelectionAct->data().value<QModelIndex>();
        qint32 companyKey = index.data().toInt();
        if(config->currCity->selectedCompaniesList.contains(companyKey))
            config->currCity->selectedCompaniesList.removeOne(companyKey);
        else
            config->currCity->selectedCompaniesList.append(companyKey);
        model()->selectRow(index.row());
        emit dataChanged();
    });

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
    _model->select();
    tableView->show();

    // also refresh Mainwindow Company ComboBox
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    myParent->refreshCompanies();
}

void CompanyView::clear()
{
    _model->clear();
}

CompanyView* CompanyView::_instance = nullptr;
CompanyView* CompanyView::instance(){return _instance;}

void CompanyView::newRecord()
{
// QSqlRecord record = model->record();
// record.setValue(1, QVariant(tr("new description")));
// record.setValue(2, QVariant("")); // info
// record.setValue(3, QVariant("")); // prefix
// record.setValue(4, QVariant("1870/01/01"));
// record.setValue(5, QVariant("1950/12/31"));
 bool rslt = SQL::instance()->addCompany("new company", -1, "1870/01/01", "1950/12/31");
 if(!rslt)
 {
  QSqlDatabase db = QSqlDatabase::database();
  QSqlError err = db.lastError();
  QMessageBox::warning(nullptr, tr("Warning"), tr("no record was inserted.\n ")+err.text());
 }
 currentIndex = _model->index(_model->rowCount()-1,0);
 refresh();
}

void CompanyView::delRecord()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    QModelIndex ix = tableView->currentIndex();
 // int companyKey;
 // QItemSelectionModel * selectionModel = tableView->selectionModel();
 // QModelIndexList indexes = selectionModel->selectedIndexes();
 // if(indexes.empty())
 //     return;
 // QModelIndex Index = indexes.at(0);
 int companyKey = ix.model()->index(ix.row(), 0).data().toInt();
 CompanyData* cd = SQL::instance()->getCompany(companyKey);
 if(QMessageBox::question(nullptr, tr("Confirm Delete"), tr("Are you sure you want to delete company #%1 '%2'?").arg(companyKey).arg(cd->name),
                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
 {
    //_model->removeRow(ix.row());
     if(SQL::instance()->executeCommand(tr("delete from Companies where [key] = %1").arg(companyKey)))
        refresh();
 }
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
  toggleSelectionAct->setData(tableView->indexAt(pt));
  menu->addAction(toggleSelectionAct);
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

MyCompanyTableModel::MyCompanyTableModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent)
{
    config = Configuration::instance();
}

Qt::ItemFlags MyCompanyTableModel::flags(const QModelIndex &index) const
{
    if(index.column() == 0)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    return QSqlTableModel::flags(index);
}


QVariant MyCompanyTableModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();
 if(role == Qt::CheckStateRole and index.column()==0)
 {
     if(config->currCity->selectedCompaniesList.isEmpty() )
         return Qt::Checked;
     qint32 companyKey = data(index,Qt::DisplayRole).toInt();
     if(config->currCity->selectedCompaniesList.contains(companyKey))
         return Qt::Checked;
     else
         return Qt::Unchecked;
 }

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
 emit companyChange();
 return QSqlTableModel::setData( index, value, role );
}

void CompanyView::On_primeInsert(int, QSqlRecord&)
{
 bNeedsRefresh = true;
 emit dataChanged();
}
