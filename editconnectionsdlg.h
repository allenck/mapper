#ifndef EDITCONNECTIONSDLG_H
#define EDITCONNECTIONSDLG_H

#include <QDialog>
#include "configuration.h"
#include <QTcpSocket>
#include <QComboBox>

namespace Ui {
    class editConnectionsDlg;
}

class ODBCUtil;
class EditConnectionsDlg : public QDialog
{
    Q_OBJECT

public:
 EditConnectionsDlg(QWidget *parent = 0);
 void setCity(City* city);
 void refreshCities();
 void setParameter(Parameters p);
 QString displayDbInfo(QSqlDatabase db);

 ~EditConnectionsDlg();
 enum DBTYPE {MySql, MsSql,Sqlite,PostgreSQL};

private:
 Ui::editConnectionsDlg *ui;
 Configuration* config;
 QStringList drivers;
 QStringList dbTypes;
 bool bCbConnectionsTextChanged = false;
 bool bCbCitiesTextChanged=false;
 bool testConnection(bool bCreate = false);
 bool openTestDb();
 QSqlDatabase db;
 QTimer *timer;
 QTcpSocket socket;
 QStringList databases;
 QStringList availableDatabases;
 //QMap<QString, QString> odbcMap;
 bool createSqliteTables(QSqlDatabase db);
 bool populateDatabases();
 QString getDatabase();
// #ifndef Q_WS_WIN
//  void findODBCDsn(QString iniFile, QStringList* dsnList);
//  QString getODBCDSNValue(QString iniFile, QString dsn, QString key);
// #endif
  bool verifyDatabase(QString name);
  void setControls(QString txt);
  City* currCity = nullptr;
  Connection* connection;
  void newConnection();
  void setComboBoxItemEnabled(QComboBox * comboBox, int index, bool enabled);
  bool connectionChanging = false;
  QString basePath;
  void removeEmptyFiles();
  Parameters parms;
  //QMap<QString, QList<QPair<QString,QString>>> odbcPairMap;
  QMap<QString, QMap<QString,QString>> odbcPairMap;
  bool setDatabase(QString useDatabase);
  int findId(Connection* c);
  Connection* _testConnection = nullptr;
  ODBCUtil* odbcUtil = nullptr;

private slots:
 void cbCitiesSelectionChanged(int sel);
 void cbConnectionsSelectionChanged(int sel);
 void cbDriverTypeSelectionChanged(QString sel);
 void btnTestClicked();
 void btnSaveClicked();
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
 void ontxtSqliteFileName_editingFinished();
 void setupComboBoxes(QString);
 void onDbTypeChanged(QString);
 void cbConnections_contextMenuRequested(QPoint);
 //void handleOverrides(QString dbName);
};

#endif // EDITCONNECTIONSDLG_H
