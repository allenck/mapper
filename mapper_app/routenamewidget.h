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
  void setRouteData(RouteData *rd);
  int newRoute() {return _routeNbr;}
  QString newRouteName();
  bool newRouteNbr() {return bNewRouteNbr;}
  void setHelpLabel(QLabel *lblHelpText ) {this->lblHelpText = lblHelpText;}
  void configure(SegmentData* sd,QLabel *lblHelpText);
  void configure(RouteData *rd, QLabel *lblHelpText);
  QString alphaRoute() {return _alphaRoute;}
  void setRouteName(QString name);

 private slots:
  void txtRouteNbr_Leave();
  void txtRouteName_Leave();

 signals:
  void routeNameChange();

 private:
  Ui::RouteNameWidget *ui;
  bool bNewRouteNbr = false;
  SQL* sql;
  QString _alphaRoute;
  int companyKey;
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
