#ifndef DIALOGNEWCONNECTION_H
#define DIALOGNEWCONNECTION_H

#include "qabstractbutton.h"
#include "qsqldatabase.h"
#include <QDialog>
#include <QtConcurrent>
#include <QFutureWatcher>

namespace Ui { class DialogNewConnection; }

class City;
class QHostInfo;
class Configuration;
class SQL;
class ODBCUtil;
class QStandardItemModel;
class DSN;
class DialogNewConnection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewConnection(QWidget *parent = nullptr);
    ~DialogNewConnection();
    void setCity(City* city);
    void displayDbInfo(QSqlDatabase db);
    DSN* getDsn(){return _dsn;}
    QString dbType() {return _dbType;}
    QString driver(){return _driver;}
    QString connectionType() {return _connect;}
    QString connectString() {return _connectString;}
    ODBCUtil* odbc(){return odbcUtil;}
    bool verifyDb(QString database);
    QStringList availableDatabases(){return _available;}
    QString useDataBase();
    QString description() {return _description;}
    QString uid();
    QString pwd();
    QString server();

private slots:
    void onCbDbType_currentTextChanged(QString);
    void onCbconnect_currentTextChanged(QString);
    void onCbDsn_currentIndexChanged(int);
    void buttonClicked(QAbstractButton*);
    void onTestClicked();
    void onPasswordEntered();
    void onOKClicked();
    void printResults(const QHostInfo& info);

private:
    Ui::DialogNewConnection *ui;
    Configuration* config = 0;
    SQL* sql=nullptr;
    ODBCUtil* odbcUtil=nullptr;
    QStringList drivers;
    QString _dbType;
    QString _cityLabel;
    QString _driver;
    City* _city =nullptr;
    QStandardItemModel* _cbConnectModel=nullptr;
    QStringList dbTypeItems;
    QStringList dbConnectItems;
    bool _bOpen = false;
    QSqlDatabase db;
    DSN* _dsn =nullptr;
    QString _connect;
    QString _connectString;
    void logonInBackground(QString driver, QString dsnName, QString dbType,
                           QString uid, QString pwd, bool allowOk = false);
    QStringList _available;
    QString _description;
    QString _testCN = "testConnection";
    void checkHost(QString txtHost);
    bool bCheckingHost=false;
    QString _lookupError;
};

#endif // DIALOGNEWCONNECTION_H
