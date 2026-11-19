#ifndef MODIFYROUTETRACTIONTYPEDLG_H
#define MODIFYROUTETRACTIONTYPEDLG_H

#include <QDialog>
#include "configuration.h"
#include "routeinfo.h"
#include "segmentdata.h"

namespace Ui {
 class ModifyRouteTractionTypeDlg;
}

class QAbstractButton;
class ModifyRouteTractionTypeDlg : public QDialog
{
  Q_OBJECT

 public:
  explicit ModifyRouteTractionTypeDlg(QWidget *parent = nullptr);
  ModifyRouteTractionTypeDlg(RouteInfo rd, QWidget *parent = nullptr);
  ~ModifyRouteTractionTypeDlg();
  //void setConfiguration(Configuration* config);
  void setRouteData(QList<RouteInfo> routeList, int currentIx);
  RouteInfo getRouteData();

 private:
  Ui::ModifyRouteTractionTypeDlg *ui;
  Configuration* config;
  SQL* sql;
  int _currentIx;
  QList<RouteInfo> routeList;
  SegmentData sd;
  SegmentData* _sd;
  RouteInfo _ri;
  void common();

 private slots:
  void dateChanged(QDate date);
  void btnOK_Click();     //SLOT

};

#endif // MODIFYROUTETRACTIONTYPEDLG_H
