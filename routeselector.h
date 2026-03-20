#ifndef ROUTESELECTOR_H
#define ROUTESELECTOR_H

#include "qdatetime.h"
#include <QTableView>
#include <QAbstractTableModel>

class QSortFilterProxyModel;
//class RouteName;
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
  QList<QString>* selectedARoutes();
  void setSelections(QList<int>* list);
  void setSelections(QList<QString> *sellist);
  QList<RouteData *> getList();
  void setAList(QList<RouteData> *routeList);

 signals:
    void selections_changed(QModelIndexList added, QModelIndexList deleted);
    void routeSelected(int route, QString alphaRoute, QDate date, int row);

 private:
    QMap<int, RouteData*>* list = nullptr;
    QMap<QString, RouteData*>* aList = nullptr;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    QSortFilterProxyModel* proxyModel = nullptr;
};

class RouteSelectorTableModel : public QAbstractTableModel
{
 Q_OBJECT
 public:
  RouteSelectorTableModel(QObject *parent=0);
  enum COLUMNS{ROUTEALPHA, ROUTEPREFIX, ROUTE, DATE, NAME, ROUTEID, COMPANY, COMPANYNAME};
  QMap<QString, RouteData*>* createList(QList<RouteData>*rdList, QDate dt);
  void setList(QList<RouteData>*rdList);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

 private:
    //QMap<int, RouteName*>* list = nullptr;
    QMap<QString, RouteData*>* aList = nullptr;
     QList<RouteData>* routeList;
     friend class RouteCommentsDlg;
};

#if 0
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
  int companyKey() {return _companyKey;}
  void setCompanyKey(int companyKey) {_companyKey = companyKey;}
  QString companyName() {return _coName;}
  void setCompanyName(QString name) {_coName = name;}
  int routeId() {return _routeId;}
  void setRouteId(int routeId) {_routeId = routeId;}
  void setDate(QDate date) {_date = date;}
  QDate date() {return _date;}
  bool equals(const RouteName& other)
  {
   if(_route == other._route || _routeAlpha == _routeAlpha)
    return true;
   else return false;
  }
  QString getRouteName(int route);

 private:
  int _route, _baseRoute, _companyKey, _routeId;
  QString _routePrefix, _routeAlpha, _name,_coName;
  QDate _date;
};
#endif
#endif // ROUTESELECTOR_H
