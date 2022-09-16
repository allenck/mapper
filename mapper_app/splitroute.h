#ifndef SPLITROUTE_H
#define SPLITROUTE_H

#include <QDialog>
#include "sql.h"
#include "data.h"
#include "configuration.h"

namespace Ui {
    class SplitRoute;
}

class SplitRoute : public QDialog
{
    Q_OBJECT

public:
    explicit SplitRoute(Configuration *cfg, QWidget *parent = 0);
    ~SplitRoute();
    void setConfiguration(Configuration *cfg);
    void setRouteData(RouteData rd);
    RouteData getRoute();
    RouteData getNewRoute();

private:
    Ui::SplitRoute *ui;
    Configuration *config;
    RouteData _rd, _newRoute;
    QList<RouteData> routeDataList;
    QList<CompanyData*> _companyList;
    qint32 _routeNbr1, _routeNbr2;
    QString _alphaRoute1, _alphaRoute2;
    bool bRoute1Changed, bRoute2Changed;
    void refreshRoutes();
    void fillCompanies();
    SQL* sql;
private slots:
    void txtNewRouteNbr1_TextChanged(QString text);
    void txtNewRouteNbr2_TextChanged(QString text);
    void txtNewRouteNbr1_Leave();
    void txtNewRouteNbr2_Leave();
    void txtNewRouteName1_Leave();
    void txtNewRouteName2_Leave();
    void dateFrom1_Leave();
    void dateTo1_ValueChanged();
    void dateTo1_Leave();
    void dateFrom2_ValueChanged();
    void dateFrom2_Leave();
    void dateTo2_Leave();
    void btnOK_Click();
    void Cancel_Click();

};

#endif // SPLITROUTE_H
