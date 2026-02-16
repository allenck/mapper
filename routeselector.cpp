#include "routeselector.h"
#include "sql.h"
#include <QItemSelectionModel>

RouteSelector::RouteSelector(QWidget *parent) : QTableView(parent)
{
 list = SQL::instance()->routeNameList();
 RouteSelectorTableModel* model = new RouteSelectorTableModel(list);
 setModel(model);
 setSelectionBehavior(QAbstractItemView::SelectRows);
 setMultiSelection(true);
}

QList<int>* RouteSelector::selectedRoutes()
{
 QModelIndexList ixList = selectedIndexes();
 QList<int>* out = new QList<int>();
 for(QModelIndex index : ixList)
 {
  out->append(list->values().at(index.row())->route());
 }
 return out;
}

void RouteSelector::setSelections(QList<int> *sellist)
{
 for(int route : *sellist)
 {
  int row = 0;

  for(RouteName* item2 : list->values())
  {
   if(route==item2->route())
   {
       selectRow(row);
       qDebug() << "select row:" << row << " route" << route;
       emit routeSelected(route, row);
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
 emit selections_changed(selected.indexes(), deselected.indexes());
}

QList<RouteName*> RouteSelector::getList()
{
 return list->values();
}


//**************************************************************************

RouteSelectorTableModel::RouteSelectorTableModel(QMap<int,RouteName*>* list, QObject* parent) : QAbstractTableModel(parent)
{
 this->list = list;
}

int RouteSelectorTableModel::rowCount(const QModelIndex &parent) const
{
 return list->count();
}

int RouteSelectorTableModel::columnCount(const QModelIndex &parent) const
{
 return 6;
}

QVariant RouteSelectorTableModel::data(const QModelIndex &index, int role) const
{
 if(role == Qt::DisplayRole)
 {
  RouteName* routeName = list->values().at(index.row());
  switch(index.column())
  {
   case ROUTE:
   return routeName->route();
  case NAME:
   return routeName->routeName();
  case ROUTEPREFIX:
   return routeName->routePrefix();
  case ROUTEALPHA:
   return routeName->routeAlpha();
  case COMPANY:
    return routeName->companyKey();
  case COMPANYNAME:
    return routeName->companyName();
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
  RouteName* routeName = list->values().at(index.row());
  // switch(index.column())
  // {
  //  case ROUTEPREFIX:
  //  routeName->setRoutePrefix(value.toString());
  //  SQL::instance()->updateAltRoute(routeName->route(), value.toString());
  // }
 }
 return false;
}

void RouteSelectorTableModel::createList(QList<RouteData>* rdList, QDate dt)
{
    beginResetModel();
    list->clear();
    foreach(RouteData rd, *rdList)
    {
        if(dt >= rd.startDate() && dt <= rd.endDate())
        {
            RouteName* rn = new RouteName();
            rn->setRoute(rd.route());
            rn->setRouteName(rd.routeName());
            rn->setRoutePrefix(rd.routePrefix());
            rn->setRouteAlpha(rd.alphaRoute());
            rn->setBaseRoute(rd.baseRoute());
            rn->setCompanyKey(rd.companyKey());
            rn->setCompanyName(rd.companyName());
            list->insert(rd.route(), rn);
        }
    }
    endResetModel();
}

QString RouteSelectorTableModel::getRouteName(int route)
{
    foreach (RouteName* rn , list->values()) {
        if(rn->route() == route)
        {
            return rn->routeName();
        }
    }
    return QString();
}


//**************************************************************************************************
RouteName::RouteName(QObject * parent) : QObject(parent)
{

}
