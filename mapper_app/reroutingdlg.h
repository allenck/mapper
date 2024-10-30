#ifndef REROUTINGDLG_H
#define REROUTINGDLG_H

#include <QDialog>
#include "data.h"
#include "configuration.h"
#include "sql.h"
#include <QAbstractButton>

namespace Ui {
class ReroutingDlg;
}

class ReroutingDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit ReroutingDlg(RouteData rd, Configuration * cfg, QWidget *parent = 0);
    ~ReroutingDlg();
    
private:
    Ui::ReroutingDlg *ui;
    Configuration *config;
    RouteData _rd;
    QWidget * myParent;
    SQL* sql;

private slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void on_fromDate_dateChanged(QDate date);
    void on_toDate_dateChanged(QDate date);
    void on_fromDate_editingFinished();
    void on_toDate_editingFinished();
};

#endif // REROUTINGDLG_H
