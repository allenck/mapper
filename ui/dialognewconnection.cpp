#include "dialognewconnection.h"
#include "qstandarditemmodel.h"
#include "ui_dialognewconnection.h"
#include "connection.h"
#include "configuration.h"
#include "odbcutil.h"
#include <QSqlDatabase>
#include "vptr.h"
#include <QSqlError>

DialogNewConnection::DialogNewConnection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNewConnection)
{
    ui->setupUi(this);
    config = Configuration::instance();
    ui->lblHelp->setStyleSheet("color: green");
    drivers = QSqlDatabase::drivers();
    odbcUtil = ODBCUtil::instance();
    odbcUtil->initialize();
    ui->lblDsn->setHidden(true);
    ui->cbDsn->setHidden(true);
    connect(ui->cbConnect, SIGNAL(currentTextChanged(QString)), this, SLOT(onCbconnect_currentTextChanged(QString)));
    for (int i = 0; i < ui->cbConnect->count(); ++i) {
        dbConnectItems << ui->cbConnect->itemText(i);
    }
    _cbConnectModel = qobject_cast<QStandardItemModel*>(ui->cbConnect->model());
    connect(ui->cbDsn, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbDsn_currentIndexChanged(int)));
    _cbConnectModel->item(ui->cbConnect->findText("Sqlite"))->setEnabled(false);
    connect(ui->cbDbType, SIGNAL(currentTextChanged(QString)), this, SLOT(onCbDbType_currentTextChanged(QString)));
    QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
    item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
    item->setEnabled(false);
    item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
    item->setEnabled(false);
    ui->cbConnect->setCurrentText("SqLite");
}

DialogNewConnection::~DialogNewConnection()
{
    delete ui;
}

void DialogNewConnection::onCbDbType_currentTextChanged(QString dbType)
{
    _dbType = dbType;
    ui->cbDsn->clear();
    if(_dbType == "MySql")
    {
        QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
        item->setEnabled(drivers.contains("QMYSQL"));
        item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
        item->setEnabled(odbcUtil->dsnByType.values("MySql").count()>0);
        item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
        item->setEnabled(true);
        item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
        item->setEnabled(odbcUtil->dsnByType.values("QMYSQL").count()>0);
        ui->cbConnect->setCurrentText("ODBC");
    }
    else
        if(_dbType == "MsSql")
        {
            QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(drivers.contains("QMSSQL"));
            item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
            item->setEnabled(odbcUtil->dsnByType.values("MsSql").count()>0);
            item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(false);
            item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
            item->setEnabled(true);
            ui->cbConnect->setCurrentText("ODBC");
        }
    else
        if(_dbType == "PostgreSQL")
        {
            QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(drivers.contains("QPSQL"));
            item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
            item->setEnabled(odbcUtil->dsnByType.values("PostgreSQL").count()>0);
            item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(true);
            item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
            item->setEnabled(false);
            ui->cbConnect->setCurrentText("Direct");
        }
    else
        if(_dbType == "MimerSql")
        {
            QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(drivers.contains("QMIMER"));
            item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
            item->setEnabled(odbcUtil->dsnByType.values("MimerSql").count()>0);
            item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(true);
            item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
            item->setEnabled(false);
            ui->cbConnect->setCurrentText("Direct");
        }
    else
        if(_dbType == "Sqlite")
        {
            QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(drivers.contains("QSQLITE"));
            item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
            item->setEnabled(true);
            item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
            item->setEnabled(false);
            item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(false);
            ui->cbConnect->setCurrentText("SqLite");
        }

}

void DialogNewConnection::onCbconnect_currentTextChanged(QString connect)
{
    _dbType = ui->cbDbType->currentText();
    ui->cbDsn->clear();
    if(connect == "Direct")
    {
        ui->lblDsn->setHidden(true);
        ui->cbDsn->setHidden(true);

        if(_dbType == "MySql")
            _driver = "QMYSQL";
        else if(_dbType == "MimerSql")
            _driver = "QMIMER";
        else if(_dbType == "PostgreSQL")
            _driver = "QPSQL";
        else
            _driver = "unknown";

    } else if (connect == "ODBC")
    {
        // get a list of all DSNs for server type (MySql,MsSql, PostgreSql, MimerSql)
        QList dsns = odbcUtil->dsnByType.values(_dbType);
        if(dsns.count() < 1)
        {
            ui->lblHelp->setStyleSheet("color: red;");
            ui->lblHelp->setText("no DSNs found for " +_dbType);
            return;
        }
        // MimerSql DSNs an others may be for a specific city. Remove those for some other location.
        for(int i = dsns.count()-1; i >0; i--)
        {
            DSN* dsn = dsns.at(i);
            if(!dsn->location.isEmpty() && dsn->location != _cityLabel)
                dsns.removeAt(i);
        }
        if(dsns.count() < 1)
        {
            ui->lblHelp->setStyleSheet("color: red;");
            ui->lblHelp->setText("no DSNs found for " +_dbType);
            return;
        }
        ui->cbDsn->clear();
        foreach (DSN* dsn, dsns) {
            ui->cbDsn->addItem(dsn->name+"/"+dsn->descr, VPtr<DSN>::asQVariant(dsn));
        }
        ui->lblDsn->setHidden(false);
        ui->cbDsn->setHidden(false);
        if(dsns.count()> 1)
        {
            ui->lblHelp->setStyleSheet("color: #FF8000;");
            ui->lblHelp->setText("Select a dsn");
            //return;
        }
        onCbDsn_currentIndexChanged(ui->cbDsn->currentIndex());
    }
    else {
        ui->lblDsn->setHidden(true);
        ui->cbDsn->setHidden(true);
    }

}

void DialogNewConnection::onCbDsn_currentIndexChanged(int ix)
{
    _dsn = VPtr<DSN>::asPtr(ui->cbDsn->itemData(ix));
    _bOpen = false;
    ui->txtServer->setText(_dsn->server);
    ui->txtDbName->clear();
    ui->txtDescription->setText(_dsn->descr);
    qApp->processEvents();

    _dbType = ui->cbDbType->currentText();
    if(_dbType == "MySql")
        _driver = "QMYSQL";
    if(_dbType == "MimerSql")
        _driver = "QMIMER";
    if(_dbType == "PostgreSQL")
        _driver = "QPSQL";

    if(_dbType == "MimerSql")
    {
        Connection::connectToCityMimer(_dsn->database, _dsn->server, "testConnection");
        db = QSqlDatabase::database("testConnection");
    } else {
        if(QSqlDatabase::contains("testConnection"));
        {
            QSqlDatabase oldDb = QSqlDatabase::database("testConnection");
            if(oldDb.isOpen()) oldDb.close();
            QSqlDatabase::removeDatabase("testConnection");
        }
        db = QSqlDatabase::addDatabase(_driver, "testConnection");
        db.setDatabaseName(_dsn->name);
    }
    QSqlError sqlError = db.lastError();
    if(sqlError.type() != QSqlError::NoError)
    {
        ui->lblHelp->setStyleSheet("color: red;");
        ui->lblHelp->setText("Error: " + db.lastError().text());
        return;
    }
    if(_dsn->server.isEmpty())
    {
        ui->lblHelp->setStyleSheet("color: #ff8000;");
        ui->lblHelp->setText("A server is needed");
        return;
    }
    db.setHostName(_dsn->server);
    if(_dsn->password.isEmpty())
    {
        if(!_dsn->userId.isEmpty())
        {
            ui->lblHelp->setStyleSheet("color: #ff8000;");
            ui->lblHelp->setText("A password is needed for user "+ _dsn->userId);
        }
        else
        {
            ui->lblHelp->setStyleSheet("color: #ff8000;");
            ui->lblHelp->setText("UserId and password is needed");
        }
        return;
    }
    ui->txtDbName->setText(_dsn->database);
    if(!db.isOpen())
        db.open(_dsn->userId,_dsn->password);
    displayDbInfo(db);
    if(db.isOpen())
    {
        _bOpen = true;
        QStringList tables = db.tables();
        ui->lblHelp->setStyleSheet("color: green");
        ui->lblHelp->setText(tr("The database is open on <B>%1</B> for user <B><I>%2</I></B>"
                                " using database <B><I>%3</I></B>"
                                "\nThere are %4 tables.")
                                 .arg(_dsn->server, _dsn->userId, _dsn->database).arg(tables.count()));
        return;
    }
    else
    {
        sqlError = db.lastError();
        //qDebug() << sqlError.text();
        ui->lblHelp->setStyleSheet("color: red");
        ui->lblHelp->setText("Error: " +sqlError.text());
        return;
    }
}

void DialogNewConnection::displayDbInfo(QSqlDatabase db)
{
    qDebug() << "QSqlDatabase parameters:";
    qDebug() <<(tr(" connection name: %1 ").arg(db.connectionName()));
    qDebug() <<(tr(" driverName: %1 ").arg(db.driverName()));
    qDebug() <<(tr(" DSN: %1 ").arg(db.databaseName()));
    qDebug() <<(tr(" hostName: %1; port: %2 ").arg(db.hostName()).arg(db.port()));
    qDebug() <<(tr(" userName: %1; pwd: %2 ").arg(db.userName(),db.password()));
    qDebug() <<(tr(" isOpen: %1; isValid: %2 ").arg(db.isOpen()?"true":"false", db.isValid()?"true":"false"));
    qDebug() <<(tr(" last error: %1 ").arg(db.lastError().text()));
}
