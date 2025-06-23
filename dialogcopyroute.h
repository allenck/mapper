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
    DialogCopyRoute(RouteData rd, QWidget *parent = 0);
    ~DialogCopyRoute();
    void setRouteData(RouteData value);
    RouteData getRouteData();
    void setConfiguration(Configuration *value);
    void setCompany(int companyKey);

private:
    Ui::DialogCopyRoute *ui;

    QList<RouteData> routeDataList;
    QList<CompanyData*> _companyList;
    QMap<int, TractionTypeInfo> tractionList;
    SQL* sql;
    int _routeNbr=-1;
    QString _alphaRoute;
    bool bRouteChanged;
    Configuration *config;
    RouteData _rd;
    RouteData* _rd2;
    QDate maxEndDate;

    CompanyData* cd = nullptr;
    void refreshRoutes();
    void fillCompanies();
    void fillTractionTypes();
    bool bNewRouteNbr=false;
    bool bAddMode=false;

private slots:
    void btnCancel_Click();
//    void txtRouteNbr_TextChanged(QString text);
//    void txtRouteNbr_Leave();
    void btnOK_Click();
    void cbCompany_SelectedIndexChanged(int row);
    void dateEnd_ValueChanged();
    void dateStart_ValueChanged();   //SLOT
    void on_rdSelected(RouteData);
};

#endif // DIALOGCOPYROUTE_H
