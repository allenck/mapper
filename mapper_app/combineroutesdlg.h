#ifndef COMBINEROUTESDLG_H
#define COMBINEROUTESDLG_H

#include <QDialog>
#include "configuration.h"
#include "sql.h"
#include <QDialogButtonBox>
#include <QAbstractButton>

namespace Ui {
class CombineRoutesDlg;
}

class CombineRoutesDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit CombineRoutesDlg(int companyKey,QWidget *parent = 0);
    ~CombineRoutesDlg();
    
private:
    Ui::CombineRoutesDlg *ui;
    Configuration *config;
    QWidget* myParent;
    void refreshRoutes();
    SQL* sql;
    QList<RouteData> routeDataList;
    qint32 _routeNbr;
    QString _alphaRoute;
    RouteData _rd1;
    RouteData _rd2;
    CompanyData* companyData;

private slots:
    void on_txtNewRouteNbr_editingFinished();
    void on_txtNewRouteName_editingFinished();
    void on_cbRoute1_currentIndexChanged(int sel);
    void on_cbRoute2_currentIndexChanged(int sel);
    void on_buttonBox_clicked(QAbstractButton * button);
    void on_dateEdit_editingFinished();
    void on_dateEdit_dateChanged(QDate date);
    void on_endDate_dateChanged(QDate date);
};

#endif // COMBINEROUTESDLG_H
