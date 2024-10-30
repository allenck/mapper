#include "routeselector.h"
#include "sql.h"
#include <QItemSelectionModel>

RouteSelector::RouteSelector(QWidget *parent) : QTableView(parent)
{
 list = SQL::instance()->routeNameList();
 RouteSelectorTableModel* model = new RouteSelectorTableModel(list);
 setModel(model);
 setSelectionBehavior(QAbstractItemView::SelectRows);
 setSelectionMode(QAbstractItemView::SingleSelection);
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
  int row =0;

  for(RouteName* item2 : list->values())
  {
   if(route==item2->route())
    selectRow(row);
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
 return 4;
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
  case BASEROUTE:
   return routeName->baseRoute();
  case ROUTEPREFIX:
   return routeName->routePrefix();
  case ROUTEALPHA:
   return routeName->routeAlpha();
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
   case BASEROUTE:
    return tr("BaseRoute");
   case ROUTEPREFIX:
    return tr("Prefix");
   case ROUTEALPHA:
    return tr("Alpha");
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
  case BASEROUTE:
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

 case ROUTE:
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }
}

bool RouteSelectorTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 if(role == Qt::EditRole)
 {
  RouteName* routeName = list->values().at(index.row());
  switch(index.column())
  {
   case ROUTEPREFIX:
   routeName->setRoutePrefix(value.toString());
   SQL::instance()->updateAltRoute(routeName->route(), value.toString());
  }
 }
 return false;
}


//**************************************************************************************************
RouteName::RouteName(QObject * parent) : QObject(parent)
{

}
