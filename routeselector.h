#ifndef ROUTESELECTOR_H
#define ROUTESELECTOR_H

#include <QTableView>
#include <QAbstractTableModel>

class RouteName;
class RouteData;
class RouteSelector : public QTableView
{
  Q_OBJECT
 public:
  RouteSelector(QWidget *parent=0);
  ~RouteSelector() {}
  RouteSelector(const RouteSelector &): QTableView() {}
  void setMultiSelection(bool);
  QList<int> *selectedRoutes();
  void setSelections(QList<int>* list);
  QList<RouteName *> getList();

 signals:
  void selections_changed(QModelIndexList added, QModelIndexList deleted);

 private:
  QMap<int, RouteName*>* list;
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

};

class RouteSelectorTableModel : public QAbstractTableModel
{
 Q_OBJECT
 public:
  RouteSelectorTableModel(QMap<int, RouteName *> *list, QObject *parent=0);
  enum COLUMNS{ROUTEALPHA, ROUTEPREFIX, ROUTE, NAME};
  void createList(QList<RouteData>*rdList, QDate dt, int companyKey);
  QString getRouteName(int route);


  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

 private:
  QMap<int, RouteName*>* list;


};

class RouteName : public QObject
{
  Q_OBJECT
 public:
  RouteName(QObject* parent =0);
  ~RouteName() {}
  RouteName(const RouteName&) : QObject() {}
  int route() {return _route;}
  int baseRoute(){return _baseRoute;}
  QString routeName() {return _name;}
  QString  routePrefix() {return _routePrefix;}
  QString routeAlpha(){return _routeAlpha;}
  void setRoute(int r){_route = r;}
  void setBaseRoute(int baseRoute){_baseRoute = baseRoute;}
  void setRoutePrefix(QString routePrefix){_routePrefix = routePrefix;}
  void setRouteAlpha(QString routeAlpha){_routeAlpha = routeAlpha;}
  void setRouteName(QString name) {_name = name;}
  bool equals(const RouteName& other)
  {
   if(_route == other._route || _routeAlpha == _routeAlpha)
    return true;
   else return false;
  }
  QString getRouteName(int route);

 private:
  int _route, _baseRoute;
  QString _routePrefix, _routeAlpha, _name;
};

#endif // ROUTESELECTOR_H
