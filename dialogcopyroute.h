#ifndef DIALOGCOPYROUTE_H
#define DIALOGCOPYROUTE_H

#include <QDialog>
#include "data.h"
#include "sql.h"
#include "configuration.h"

namespace Ui {
    class DialogCopyRoute;
}

class DialogCopyRoute : public QDialog
{
    Q_OBJECT

public:
    DialogCopyRoute(Configuration *cfg, QWidget *parent = 0);
    ~DialogCopyRoute();
    void setRouteData(RouteData value);
    RouteData getRouteData();
    void setConfiguration(Configuration *value);

private:
    Ui::DialogCopyRoute *ui;

    QList<RouteData> routeDataList;
    QList<CompanyData*> _companyList;
    QMap<int, TractionTypeInfo> tractionList;
    SQL* sql;
    int _routeNbr;
    QString _alphaRoute;
    bool bRouteChanged;
    Configuration *config;
    RouteData _rd;
    CompanyData* cd = nullptr;
    void refreshRoutes();
    void fillCompanies();
    void fillTractionTypes();

private slots:
    void btnCancel_Click();
    void txtRouteNbr_TextChanged(QString text);
    void txtRouteNbr_Leave();
    void btnOK_Click();
    void cbCompany_SelectedIndexChanged(int row);
    void dateEnd_ValueChanged();
    void dateStart_ValueChanged();   //SLOT

};

#endif // DIALOGCOPYROUTE_H
