#ifndef CREATESQLITEDATABASEDIALOG_H
#define CREATESQLITEDATABASEDIALOG_H

#include <QDialog>
#include "configuration.h"
#include "data.h"

namespace Ui {
class createSqliteDatabaseDialog;
}

class CreateSqliteDatabaseDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CreateSqliteDatabaseDialog(LatLng * latLng, QWidget *parent = 0);
    ~CreateSqliteDatabaseDialog();
    
private:
    Ui::createSqliteDatabaseDialog *ui;
    Configuration *config;
    QString strNew;
    LatLng* latLng;

private slots:
    void on_btnBrowse_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void on_txtDbName_editingFinished();
    void on_cbCities_currentIndexChanged(int);
    void on_edTitle_editingFinished();
    void txtLatLngChanged();

};

#endif // CREATESQLITEDATABASEDIALOG_H
