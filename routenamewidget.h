#ifndef ROUTENAMEWIDGET_H
#define ROUTENAMEWIDGET_H

#include <QWidget>
#include "sql.h"

namespace Ui {
 class RouteNameWidget;
}

class RouteNameWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit RouteNameWidget(QWidget *parent = nullptr);
  ~RouteNameWidget();
  void setCompanyKey(int companyKey);
  void setSegmentData(SegmentData* sd);
  void setRouteData(RouteData* rd);
  int newRoute();
  QString newRouteName();
  bool routeNbrMustBeAdded() {return bNewRouteNbr;} // true if entry to altRoute must be added
  void setHelpLabel(QLabel *lblHelpText ) {this->lblHelpText = lblHelpText;}
  void configure(SegmentData* sd,QLabel *lblHelpText);
  void configure(RouteData *rd, QLabel *lblHelpText);
  QString alphaRoute() {return _alphaRoute;}
  void setRouteName(QString name);
  void setRouteName(RouteData rd);
  void setAlphaRoute(QString);
  void clear();
  int getRouteId();

 private slots:
  void txtRouteNbr_Leave();
  void txtRouteName_Leave();
  void companyChange(int index);
  void routeChange(int index);

 signals:
  void routeNameChanged(QString name);
  void routeNumberChange(int route);
  void rdSelected(RouteData rd);


 private:
  Ui::RouteNameWidget *ui;
  bool bNewRouteNbr = false;
  bool bAlphaRoute = false;
  bool bNbrEdited = false;
  SQL* sql;
  QString _alphaRoute;
  int companyKey=-1;
  bool bRouteChanging;
  int _routeNbr;
  SegmentData* sd = nullptr;
  RouteData* rd = nullptr;
  Configuration* config;
  QLabel* lblHelpText = nullptr;
  QString strNoRoute = tr("New Route Name");
  RouteData _rd;
};

#endif // ROUTENAMEWIDGET_H
