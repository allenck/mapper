#ifndef EDITCONNECTIONSDLG_H
#define EDITCONNECTIONSDLG_H

#include <QDialog>
#include "configuration.h"
#include "sql.h"
#include <QTcpSocket>


namespace Ui {
    class editConnectionsDlg;
}

class EditConnectionsDlg : public QDialog
{
    Q_OBJECT

public:
 EditConnectionsDlg(QWidget *parent = 0);
 ~EditConnectionsDlg();

private:
 Ui::editConnectionsDlg *ui;
 Configuration* config;
 QStringList drivers;
 QStringList dbType;
 bool bCbConnectionsTextChanged;
 bool bCbCitiesTextChanged;
 bool testConnection();
 bool openTestDb();
 QSqlDatabase db;
 QTimer *timer;
 QTcpSocket socket;
 QStringList databases;
 bool createSqliteTables(QSqlDatabase db);
 void populateDatabases();
 QString getDatabase();
#ifndef Q_WS_WIN
void findODBCDsn(QString iniFile, QStringList* dsnList);
#endif
bool verifyDatabase(QString name);

private slots:
 void cbCitiesSelectionChanged(int sel);
 void cbConnectionsSelectionChanged(int sel);
 void cbDriverTypeSelectionChanged(int sel);
 void btnTestClicked();
 void btnOKClicked();
 void btnCancelClicked();
 void btnDeleteClicked();
 void cbCitiesTextChanged(QString text);
 void cbCitiesLeave();
 void cbConnectionsTextChanged(QString text);
 void cbConnectionsLeave();
 void quickProcess();
 void txtHostLeave();
 void txtPortLeave();
 void txtPwdLeave();
 void txtDsnTextChanged(QString text);
 void on_tbBrowse_clicked();
 void ontxtDbOrDsn_editingFinished();
};

#endif // EDITCONNECTIONSDLG_H
