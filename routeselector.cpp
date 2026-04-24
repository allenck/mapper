#include "routeselector.h"
#include "sql.h"
#include <QItemSelectionModel>
#include "mainwindow.h"

RouteSelector::RouteSelector(QWidget *parent) : QTableView(parent)
{
    // list = SQL::instance()->routeNameList();
    //aList = SQL::instance()->routeNameAList();


    RouteSelectorTableModel* model = new RouteSelectorTableModel(this);

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    setModel(proxyModel);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setMultiSelection(true);
    setSortingEnabled(true);
    horizontalHeader()->setStretchLastSection(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    for(QVariant col : Configuration::instance()->rcd.hiddenColumns)
    {
        hideColumn(col.toInt());
    }
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

QList<int>* RouteSelector::selectedRoutes()
{
    QModelIndexList ixList = selectedIndexes();
    QList<int>* out = new QList<int>();
    for(QModelIndex index : ixList)
    {
        QModelIndex sIndex = proxyModel->mapToSource(index);
        out->append(list->values().at(sIndex.row())->route());
    }
    return out;
}

QList<QString>* RouteSelector::selectedARoutes()
{
    QModelIndexList ixList = selectedIndexes();
    QList<QString>* out = new QList<QString>();
    for(QModelIndex index : ixList)
    {
        QModelIndex sIndex = proxyModel->mapToSource(index);
        out->append(aList->values().at(sIndex.row())->alphaRoute());
    }
    return out;
}


void RouteSelector::setSelections(QList<int> *sellist)
{
 for(int route : *sellist)
 {
  int row = 0;

  for(RouteData* item2 : list->values())
  {
   if(route==item2->route())
   {
       selectRow(row);
       qDebug() << "select row:" << row << " route" << route << " alphaRoute:" << item2->alphaRoute();
       emit routeSelected(route, item2->alphaRoute(), item2->startDate(),row);
       break;
   }
   row++;
  }
 }
}

void RouteSelector::setSelections(QList<QString> *sellist)
{

    if(!aList)
        return;
    for(QString alphaRoute : *sellist)
    {
        int row = 0;

        for(RouteData* item2 : aList->values())
        {
            if(alphaRoute==item2->alphaRoute())
            {
                selectRow(row);
                qDebug() << "select row:" << row << " route" << item2->route() << " alphaRoute:" << item2->alphaRoute();
                emit routeSelected(item2->route(), item2->alphaRoute(), item2->startDate(),row);
                break;
            }
            row++;
        }
    }
}


void RouteSelector::setMultiSelection(bool multi)
{
 if(multi)
  setSelectionMode(QAbstractItemView::MultiSelection);
 else
  setSelectionMode(QAbstractItemView::SingleSelection);
}

void RouteSelector::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel* selectionModel = this->selectionModel();
    QModelIndexList modelIndexList = selectionModel->selectedRows();
    foreach (QModelIndex index, modelIndexList) {
        QModelIndex ix = proxyModel->mapToSource(index);
        QModelIndex aix = proxyModel->sourceModel()->index(ix.row(), RouteSelectorTableModel::ROUTEALPHA);
        QModelIndex rix = proxyModel->sourceModel()->index(ix.row(), RouteSelectorTableModel::ROUTE);
        QModelIndex dtix = proxyModel->sourceModel()->index(ix.row(), RouteSelectorTableModel::DATE);
        emit routeSelected(rix.data().toInt(), aix.data().toString(), dtix.data().toDate(), ix.row());
    }

    emit selections_changed(selected.indexes(), deselected.indexes());
}

QList<RouteData*> RouteSelector::getList()
{
 return list->values();
}

void RouteSelector::setAList(QList<RouteData> *routeList)
{
     ((RouteSelectorTableModel*)model())->createList(routeList, QDate());
}

//**************************************************************************

RouteSelectorTableModel::RouteSelectorTableModel(QObject* parent) : QAbstractTableModel(parent)
{
 //this->list = list;
    //this->aList = aList;
 aList = createList(&MainWindow::instance()->routeList, QDate());
 setList(&MainWindow::instance()->routeList);
}

int RouteSelectorTableModel::rowCount(const QModelIndex &parent) const
{
 return aList->count();
}

int RouteSelectorTableModel::columnCount(const QModelIndex &parent) const
{
 return 8;
}

QVariant RouteSelectorTableModel::data(const QModelIndex &index, int role) const
{
 if(role == Qt::DisplayRole)
 {
  //RouteData* routeName = aList->values().at(index.row());
     RouteData rd = routeList->at(index.row());
  switch(index.column())
  {
   case ROUTE:
   return rd.route();
  case NAME:
   return rd.routeName();
  case ROUTEPREFIX:
   return rd.routePrefix();
  case ROUTEALPHA:
   return rd.alphaRoute();
  case COMPANY:
    return rd.companyKey();
  case COMPANYNAME:
    return rd.companyName();
  case ROUTEID:
      return rd.routeId();
  case DATE:
      return rd.startDate().toString("yyyy/MM/dd");
  }
 }
 return QVariant();
}

QVariant RouteSelectorTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
 {
  switch(section)
  {
   case ROUTE:
    return tr("Route");
   case NAME:
    return tr("RouteName");
   case ROUTEPREFIX:
    return tr("Prefix");
   case ROUTEALPHA:
    return tr("Alpha");
   case COMPANY:
    return tr("Company id");
   case COMPANYNAME:
    return tr("Company Name");
   case ROUTEID:
       return "RouteId";
    case DATE:
       return "Date";
  }
 }
 return QVariant();
}

Qt::ItemFlags RouteSelectorTableModel::flags(const QModelIndex &index) const
{
 switch(index.column())
 {
  case ROUTEALPHA:
  case ROUTEPREFIX:
  case ROUTE:
  case NAME:
  case COMPANY:
  case COMPANYNAME:
  default:
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }
}

bool RouteSelectorTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 if(role == Qt::EditRole)
 {
  RouteData* routeName = aList->values().at(index.row());
  // switch(index.column())
  // {
  //  case ROUTEPREFIX:
  //  routeName->setRoutePrefix(value.toString());
  //  SQL::instance()->updateAltRoute(routeName->route(), value.toString());
  // }
 }
 return false;
}

QMultiMap<QString, RouteData*>* RouteSelectorTableModel::createList(QList<RouteData>* rdList, QDate dt)
{
    beginResetModel();
    //list->clear();
    // if(!aList)
    //     aList = SQL::instance()->routeNameAList();
    // else
    //     aList->clear();
    aList = new QMultiMap<QString, RouteData*>();
    foreach(RouteData rd, *rdList)
    {
        //if(dt >= rd.startDate().addDays(-700) && dt <= rd.endDate())
        {
            RouteData* rn = new RouteData(rd);
            rn->setRoute(rd.route());
            rn->setRouteName(rd.routeName());
            rn->setRoutePrefix(rd.routePrefix());
            rn->setAlphaRoute(rd.alphaRoute());
            rn->setBaseRoute(rd.baseRoute());
            rn->setCompanyKey(rd.companyKey());
            rn->setCompanyName(rd.companyName());
            rn->setRouteId(rd.routeId());
            rn->setStartDate(rd.startDate());
            aList->insert(rd.alphaRoute(), rn);
        }
    }
    endResetModel();
    return aList;
}

void RouteSelectorTableModel::setList(QList<RouteData>* routeList)
{
    beginResetModel();
    this->routeList = routeList;
    endResetModel();
}

// QString RouteSelectorTableModel::getRouteName(int route)
// {
//     foreach (RouteName* rn , aist->values()) {
//         if(rn->route() == route)
//         {
//             return rn->routeName();
//         }
//     }
//     return QString();
// }
// int RouteSelectorTableModel::getRouteId(int route)
// {
//     foreach (RouteName* rn , list->values()) {
//         if(rn->route() == route)
//         {
//             return rn->routeId();
//         }
//     }
//     return -1;
// }


// //**************************************************************************************************
// RouteName::RouteName(QObject * parent) : QObject(parent)
// {

// }
