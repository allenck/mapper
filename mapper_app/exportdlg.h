#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#include <QDialog>
#include "sql.h"
#include "configuration.h"
#include "exportsql.h"

namespace Ui {
    class ExportDlg;
}

class ExportDlg : public QDialog
{
    Q_OBJECT

public:
    ExportDlg(Configuration *cfg, QWidget *parent = 0);
    ~ExportDlg();

private:
    Ui::ExportDlg *ui;
    Configuration * config;
    QTimer *timer;


private slots:
    void chkAll_changed(bool isChecked);
    void btnCancel_clicked();
    void btnOK_clicked();
    void quickProcess();
    void chkOverrideToggled(bool checked);
    void newProgressMsg(QString msg);
    void uncheckControl(QString control);
    void on_chkDrop_toggled(bool);
    void on_chkAltRoute_toggled(bool);
    void on_chkSegments_toggled(bool);
    void on_chkTractionTypes_toggled(bool);
    void on_chkCompanies_toggled(bool);
    void on_chkAll_toggled(bool);
};

#endif // EXPORTDLG_H
