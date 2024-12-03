#ifndef MODIFYROUTEDATEDLG_H
#define MODIFYROUTEDATEDLG_H

#include <QDialog>
#include "sql.h"
#include "configuration.h"
#include "data.h"


namespace Ui {
    class ModifyRouteDateDlg;
}

class ModifyRouteDateDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyRouteDateDlg(QWidget *parent = 0);
    ~ModifyRouteDateDlg();
    //void setConfiguration(Configuration *cfg);
    //void setRouteData(routeData rd);
    void setRouteData(RouteData* rd);
    RouteData* getRouteData();

private:
    Ui::ModifyRouteDateDlg *ui;
    RouteData rd;
    RouteData* _rd;
    RouteData oRd;
    RouteData* _otherRd;
    Configuration *config;
    SQL* sql;
    int _currentIx;
    QList<RouteData> routeList;
    QDate maxEndDate;
    CompanyData* cd = nullptr;

private slots:
    void dateTimePicker1_ValueChanged(QDate date);
    void btnOK_Click();
    void btnCancel_Click();
    void rbStartToggled(bool state);
};

#endif // MODIFYROUTEDATEDLG_H
