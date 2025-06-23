#ifndef MODIFYROUTETRACTIONTYPEDLG_H
#define MODIFYROUTETRACTIONTYPEDLG_H

#include <QDialog>
#include "configuration.h"

namespace Ui {
 class ModifyRouteTractionTypeDlg;
}

class QAbstractButton;
class ModifyRouteTractionTypeDlg : public QDialog
{
  Q_OBJECT

 public:
  explicit ModifyRouteTractionTypeDlg(QWidget *parent = nullptr);
  ModifyRouteTractionTypeDlg(RouteData rd, QWidget *parent = nullptr);
  ~ModifyRouteTractionTypeDlg();
  //void setConfiguration(Configuration* config);
  void setRouteData(QList<RouteData> routeList, int currentIx);
  RouteData getRouteData();

 private:
  Ui::ModifyRouteTractionTypeDlg *ui;
  Configuration* config;
  SQL* sql;
  int _currentIx;
  QList<RouteData> routeList;
  SegmentData sd;
  SegmentData* _sd;
  RouteData _rd;
  void common();

 private slots:
  void dateChanged(QDate date);
  void btnOK_Click();     //SLOT

};

#endif // MODIFYROUTETRACTIONTYPEDLG_H
