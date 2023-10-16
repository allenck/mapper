#ifndef MODIFYROUTEDIALOG_H
#define MODIFYROUTEDIALOG_H

#include <QDialog>
#include "data.h"
#include "configuration.h"
#include "sql.h"

namespace Ui {
    class DialogRenameRoute;
}

class ModifyRouteDialog : public QDialog
{
    Q_OBJECT

public:
    ModifyRouteDialog(Configuration * cfg, QWidget *parent = 0);
    ~ModifyRouteDialog();
    qint32 newRoute();
    void routeData(RouteData value);
    RouteData getRouteData();
    QString newName();
    void setConfig(Configuration *cfg);

private:
    Ui::DialogRenameRoute *ui;
    SQL* sql;
    RouteData _rd;
    Configuration *config;
    bool bRouteChanged;
    int _routeNbr;
    QString _alphaRoute;
    void refreshRoutes();
    QList<RouteData> routeDataList;

private slots:
    void cbRoutes_SelectedIndexChanged();
    void txtNewRouteNbr_TextChanged();
    void txtNewRouteName_Leave();
    void btnOK_Click();

};

#endif // MODIFYROUTEDIALOG_H
