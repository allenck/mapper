#include "companyview.h"
#include <QMenu>
#include <data.h>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include "dateeditdelegate.h"

CompanyView::CompanyView(QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = Configuration::instance();
    sql = SQL::instance();
    //sql->setConfig(config);
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    tableView = myParent->ui->tblCompanyView;
#if QT_VERSION < 0x050000
    tableView->horizontalHeader()->setMovable(true);

    tableView->horizontalHeader()->setSectionsMovable(true);
#endif
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows );
    tableView->setSelectionMode( QAbstractItemView::SingleSelection );
    _instance = this;

    QSqlDatabase db = QSqlDatabase::database();
    //qDebug()<<db.databaseName();
    tableView->setAlternatingRowColors(true);
    connect(tableView->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    _model = new MyCompanyTableModel(this);
    // _model->setTable("Companies");
    // _model->setSort(1, Qt::AscendingOrder);
    //connect(_model, SIGNAL(primeInsert(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));
    // connect(_model, &QAbstractTableModel::dataChanged, [=](QModelIndex left, QModelIndex right, QList<int> roles){
    //     emit dataChanged();
    // });
    //model->setQuery("select * from Companies");
    //connect(_model, SIGNAL(beforeUpdate(int,QSqlRecord&)), this, SLOT(On_primeInsert(int,QSqlRecord&)));

    //_model->setEditStrategy(QSqlTableModel::OnFieldChange);
    //model->query().setForwardOnly(false);
    //_model->select();
    //QString name = _model->record(0).value("description").toString();
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(_model);
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);
    //QSqlRecord r = _model->record(0);
    // int ixRoutePrefix = r.indexOf("routePrefix");
    // tableView->horizontalHeader()->moveSection(ixRoutePrefix,2);
    tableView->setItemDelegateForColumn(MyCompanyTableModel::STARTDATE, new DateEditDelegate(this));
    tableView->setItemDelegateForColumn(MyCompanyTableModel::ENDDATE, new DateEditDelegate(this));
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
    urlAct = new QAction(tr("Open url in browser"),this);
    connect(urlAct, &QAction::triggered, menu, [=](bool){
        // QModelIndex ix = _model->index(urlAct->data().toInt(), MyCompanyTableModel::URL);
        // QModelIndex source = proxyModel->mapToSource(ix);
        // QModelIndex ix = tableView->currentIndex();
        // QModelIndex source = proxyModel->mapToSource(ix);
        // QUrl url = QUrl(source.data().toString());
        QUrl url = urlAct->data().toUrl();
        QDesktopServices::openUrl(url);
    });

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
    //_model->select();
    tableView->show();
    _model->reset();

    // also refresh Mainwindow Company ComboBox
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    myParent->refreshCompanies();
}

void CompanyView::clear()
{
    //_model->clear();
}

CompanyView* CompanyView::_instance = nullptr;
CompanyView* CompanyView::instance(){return _instance;}

void CompanyView::newRecord()
{
 int rslt = SQL::instance()->addCompany("new company", -1, "1870/01/01", "1950/12/31");
 if(!rslt)
 {
  QSqlDatabase db = QSqlDatabase::database();
  QSqlError err = db.lastError();
  QMessageBox::warning(nullptr, tr("Warning"), tr("no record was inserted.\n ")+err.text());
 }
 CompanyData* cd = sql->getCompany(rslt);
 _model->insertRecord(cd);
 //currentIndex = _model->index(_model->rowCount()-1,0);
 refresh();
}

void CompanyView::delRecord()
{
 //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
 QModelIndex ix = tableView->currentIndex();
 QModelIndex source = proxyModel->mapToSource(ix);
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
    bool result = _model->removeRow(source.row());
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
  CompanyData* cd = _model->getCompanyAtRow(curRow);
  if(cd)
  {
    menu->addAction(urlAct);
      urlAct->setData(cd->url);
  }
  urlAct->setEnabled(cd->url.isValid());
  menu->exec(QCursor::pos());
 }
}

// void CompanyView::On_primeInsert(int, QSqlRecord&)
// {
//     bNeedsRefresh = true;
//     emit dataChanged();
// }

QVariant MyCompanyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
 {
  return hdrMap.value(section);
 }
 return QVariant();
}

MyCompanyTableModel::MyCompanyTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    config = Configuration::instance();
    sql= SQL::instance();
    companyList = sql->getCompanies();
}

Qt::ItemFlags MyCompanyTableModel::flags(const QModelIndex &index) const
{
    if(index.column() == KEY  || index.column() == LASTUPDATED)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable ;
    if(index.column() == SELECT)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;

}


QVariant MyCompanyTableModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();
 CompanyData* cd = companyList.at(index.row());
 if(role == Qt::CheckStateRole and index.column()==SELECT)
 {
     // if(config->currCity->selectedCompaniesList.isEmpty() )
     //     return Qt::Checked;

     //if(config->currCity->selectedCompaniesList.contains(cd->companyKey))
     if(cd->bSelected)
         return Qt::Checked;
     else
         return Qt::Unchecked;
 }
 if(role == Qt::DisplayRole || role == Qt::EditRole)
 {
     switch (index.column()) {
     case KEY:
         return cd->companyKey;
     case MNEMONIC:
         return cd->mnemonic;
     case NAME:
         return cd->name;
     case ROUTEPREFIX:
         return cd->routePrefix;
     case STARTDATE:
         return cd->startDate.toString("yyyy/MM/dd");
     case ENDDATE:
         return cd->endDate.toString("yyyy/MM/dd");
     case FIRSTROUTE:
         return cd->firstRoute;
     case LASTROUTE:
         return cd->lastRoute;
     case LASTUPDATED:
         return cd->lastUpdated.toString();
     case INFO:
         return cd->info;
     case URL:
         return cd->url.toDisplayString();
     default:
         break;
     }
 }

 // let the base class handle all other cases
 return QVariant();
}

bool MyCompanyTableModel::setData(const QModelIndex &mindex, const QVariant &value, int role)
{
 CompanyData* cd = companyList.at(mindex.row());

 QVariant hdr;
 QList<int> roles;
 QModelIndex left = index(mindex.row(), 0);
 QModelIndex right = index(mindex.row(), LASTUPDATED-1);

 if( role == Qt::CheckStateRole || role == Qt::EditRole/* && index.column() == 0*/)
 {
     switch(mindex.column())
     {
        case SELECT:
        {
            //bool checked = value.toBool();
            // if(checked)
            // {
            //     if(!config->currCity->selectedCompaniesList.contains(cd->companyKey))
            //         config->currCity->selectedCompaniesList.append(cd->companyKey);
            // }
            // else
            // {
            //     if(config->currCity->selectedCompaniesList.contains(cd->companyKey))
            //         config->currCity->selectedCompaniesList.removeOne(cd->companyKey);
            // }
            bDirty = true;
            cd->bSelected = value.toBool();
            companyList.at(mindex.row())->bSelected = value.toBool();
            cd = companyList.at(mindex.row());
            dataChanged(mindex, index(mindex.row(), LASTROUTE));
            roles.append(role);
            sql->updateCompany(cd);
            emit companySelectionsChanged();
            return true;
        }
    }
 }


 if(role == Qt::EditRole)
 {
     switch (mindex.column()) {
     case NAME:
          cd->name = value.toString();
         break;
     case MNEMONIC:
         cd->mnemonic = value.toString();
         break;
     case ROUTEPREFIX:
         cd->routePrefix = value.toString();
         break;
     case STARTDATE:
         cd->startDate = QDate::fromString(value.toString(), "yyyy/MM/dd");
         break;
     case ENDDATE:
         cd->endDate = QDate::fromString(value.toString(), "yyyy/MM/dd");
         break;
     case FIRSTROUTE:
         cd->firstRoute = value.toInt();
         break;
     case LASTROUTE:
         cd->lastRoute = value.toInt();
         break;
     // case LASTUPDATED:
     //     cd->lastUpdated.toString();
     case INFO:
         cd->info = value.toString();
         break;
     case URL:
         cd->url = QUrl(value.toString());
         if(!cd->url.isValid())
             return false;
         break;
     }
     roles.append(role);

     if(sql->updateCompany(cd))
     {
         bDirty = false;
         companyList.replace(mindex.row(),cd);
         emit dataChanged(left, right,roles);
         emit companySelectionsChanged();
         return true;
     }
 }
 //emit dataChanged();
 return false;
}

void MyCompanyTableModel::insertRecord(CompanyData* cd)
{
    int newRow = rowCount(QModelIndex());
    beginInsertRows(QModelIndex(), newRow, newRow);
    companyList.append(cd);
    endInsertRows();
    emit dataChanged(index(newRow, 0), index(newRow, LASTUPDATED));
}

bool MyCompanyTableModel::removeRow(int row, const QModelIndex& parent )
{
    CompanyData* cd = companyList.at(row);
    if(SQL::instance()->executeCommand(tr("delete from Companies where [key] = %1").arg(cd->companyKey)))
    {
        beginRemoveRows(parent, row, row);
        companyList.removeAt(row);
        endRemoveRows();
        return true;
    }
    return false;
}

void MyCompanyTableModel::reset()
{
    beginResetModel();
    companyList = sql->getCompanies();
    endResetModel();
}

CompanyData* MyCompanyTableModel::getCompanyAtRow(int row)
{
    return companyList.at(row);
}
