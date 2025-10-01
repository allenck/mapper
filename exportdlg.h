#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#include <QDialog>
#include "sql.h"
#include "configuration.h"

namespace Ui {
    class ExportDlg;
}

class ExportSql;
class ExportDlg : public QDialog
{
    Q_OBJECT

public:
    ExportDlg(QWidget *parent = 0);
    ~ExportDlg();

private:
    Ui::ExportDlg *ui;
    Configuration * config;
    //QTimer *timer;
    bool stopEnabled = false;
    Connection* currConnection = nullptr;
    ExportSql* exprt= nullptr;


private slots:
    void chkAll_changed(bool isChecked);
    void btnCancel_clicked();
    void btnGo_clicked();
    //void quickProcess();
    void chkOverrideToggled(bool checked);
    void newProgressMsg(QString msg);
    void uncheckControl(QString control);
    void on_chkDrop_toggled(bool);
    void on_chkAltRoute_toggled(bool);
    void on_chkSegments_toggled(bool);
    void on_chkTractionTypes_toggled(bool);
    void on_chkCompanies_toggled(bool);
    void on_chkAll_toggled(bool);
    void on_chkRoutes_toggled(bool bChecked);
};

#endif // EXPORTDLG_H
