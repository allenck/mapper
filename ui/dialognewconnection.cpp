#include "dialognewconnection.h"
#include "qpushbutton.h"
#include "qstandarditemmodel.h"
#include "sql.h"
#include "ui_dialognewconnection.h"
#include "connection.h"
#include "configuration.h"
#include "odbcutil.h"
#include <QSqlDatabase>
#include <QSqlError>
#include "city.h"

DialogNewConnection::DialogNewConnection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNewConnection)
{
    ui->setupUi(this);
    config = Configuration::instance();
    sql= SQL::instance();
    ui->lblHelp->setStyleSheet("color: green");
    drivers = QSqlDatabase::drivers();
    odbcUtil = ODBCUtil::instance();
    odbcUtil->initialize();
    ui->lblDsn->setHidden(true);
    ui->cbDsn->setHidden(true);
    ui->textEdit->setHidden(true);
    ui->textEdit->setReadOnly(true);
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
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    QPushButton* btnTest = new QPushButton(tr("Test"));
    ui->buttonBox->addButton(btnTest, QDialogButtonBox::ActionRole);
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),this, SLOT(buttonClicked(QAbstractButton*)));
    ui->gb1->hide();
    ui->tbView->setIcon(QIcon(":/show-password.png"));
    ui->txtPWD->setEchoMode(QLineEdit::Password);
    connect(ui->tbView, &QToolButton::toggled, [=](bool checked){
        if(checked)
        {
            ui->tbView->setIcon(QIcon(":/show-password.png"));
            ui->txtPWD->setEchoMode(QLineEdit::Password);
        }
        else
        {
            ui->tbView->setIcon(QIcon(":/hidden.png"));
            ui->txtPWD->setEchoMode(QLineEdit::Normal);
        }
    });
}

DialogNewConnection::~DialogNewConnection()
{
    delete ui;

    QSqlDatabase db = QSqlDatabase::database("testConnection");
    if(db.isOpen()) db.close();
    QSqlDatabase::removeDatabase("testConnection");
}

void DialogNewConnection::onCbDbType_currentTextChanged(QString dbType)
{
    _dbType = dbType;
    ui->cbDsn->clear();
    ui->lblHelp->clear();
    ui->textEdit->clear();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

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
        _driver = "QODBC";
        ui->cbConnect->setCurrentText("ODBC");
    }
    else
        if(_dbType == "MsSql")
        {
            QStandardItem* item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(false); // MsSql only has ODBC driver
            item = _cbConnectModel->item(ui->cbConnect->findText("ODBC"));
            item->setEnabled(odbcUtil->dsnByType.values("MsSql").count()>0);
            item = _cbConnectModel->item(ui->cbConnect->findText("Direct"));
            item->setEnabled(false);
            item = _cbConnectModel->item(ui->cbConnect->findText("Sqlite"));
            item->setEnabled(true);
            _driver = "QODBC";
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
            _driver = "QPSQL";
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
            _driver = "QMIMER";
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
            _driver = "QSQLITE";
            ui->cbConnect->setCurrentText("SqLite");
        }
        if(_driver == "QODBC")
        {
            if(_dbType == "MimerSql" || _dbType == "PosgreSQL")
                odbcUtil->fillDSNCombo(ui->cbDsn,_dbType,_cityLabel);
            else
                odbcUtil->fillDSNCombo(ui->cbDsn,_dbType,"");
        }
        else
            ui->cbDsn->clear();
}

void DialogNewConnection::onCbconnect_currentTextChanged(QString connect)
{
    ui->lblHelp->clear();
    ui->textEdit->clear();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->gb1->setVisible(false);

    _dbType = ui->cbDbType->currentText();
    _connect = connect;
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
        _driver = "QODBC";
        // get a list of all DSNs for server type (MySql,MsSql, PostgreSql, MimerSql)
        QList dsns = odbcUtil->dsnByType.values(_dbType);
        if(dsns.count() < 1)
        {
            ui->lblHelp->setStyleSheet("color: red;");
            ui->lblHelp->setText("no DSNs found for " +_dbType);
            return;
        }
        // MimerSql DSNs an others may be for a specific city. Remove those for some other location.
        for(int i = dsns.count()-1; i >=0; i--)
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
        // ui->cbDsn->clear();
        // foreach (DSN* dsn, dsns) {
        //     ui->cbDsn->addItem(dsn->name+"/"+dsn->descr, VPtr<DSN>::asQVariant(dsn));
        // }
        if(_dbType == "MimerSql" || _dbType == "PosgreSQL")
            odbcUtil->fillDSNCombo(ui->cbDsn,_dbType,_cityLabel);
        else
            odbcUtil->fillDSNCombo(ui->cbDsn,_dbType,"");
        ui->lblDsn->setHidden(false);
        ui->cbDsn->setHidden(false);
        if(dsns.count()> 1)
        {
            ui->lblHelp->setStyleSheet("color: #FF8000;");
            ui->lblHelp->setText("Select a dsn");
            //return;
        }
        //onCbDsn_currentIndexChanged(ui->cbDsn->currentIndex());
    }
    else {
        ui->lblDsn->setHidden(true);
        ui->cbDsn->setHidden(true);
    }
}

void DialogNewConnection::onCbDsn_currentIndexChanged(int ix)
{
    ui->lblHelp->clear();
    ui->textEdit->clear();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->gb1->setVisible(false);
    if(ix < 0)
        return;

    //_dsn = VPtr<DSN>::asPtr(ui->cbDsn->itemData(ix));
    _dsn = odbcUtil->getDsn(ui->cbDsn->itemData(ix).toString());
    _bOpen = false;
    ui->txtServer->setText(_dsn->server);
    ui->txtDbName->clear();
    ui->txtDescription->setText(_dsn->descr);
    ui->txtDbName->setText(_dsn->database);
    qApp->processEvents();

    _dbType = ui->cbDbType->currentText();
    // if(_dbType == "MySql")
    //     _driver = "QMYSQL";
    // if(_dbType == "MimerSql")
    //     _driver = "QMIMER";
    // if(_dbType == "PostgreSQL")
    //     _driver = "QPSQL";

    if(_dbType == "MimerSql")
    {
        Connection::connectToCityMimer(_dsn->database, _dsn->server, "testConnection");
        db = QSqlDatabase::database("testConnection");
        QSqlError sqlErr = db.lastError();
        if(db.isOpen())
        {
            _connectString = db.databaseName();
            db.close(); // Close immediately after capturing string parameter info
        }
        //QSqlDatabase::removeDatabase("testConnection");
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
    //db.setHostName(_dsn->server);
    if(_dsn->password.isEmpty())
    {
        if(!_dsn->userId.isEmpty())
        {
            ui->lblHelp->setStyleSheet("color: #ff8000;");
            ui->lblHelp->setText("A password is needed for user "+ _dsn->userId);
            ui->txtUid->setText(_dsn->userId);
            ui->gb1->setVisible(true);
        }
        else
        {
            ui->lblHelp->setStyleSheet("color: #ff8000;");
            ui->lblHelp->setText("UserId and password is needed");
            ui->gb1->setVisible(true);
        }
        return;
    }
    ui->txtDbName->setText(_dsn->database);
    setCursor(Qt::WaitCursor);
    if(!db.isOpen())
        db.open(_dsn->userId,_dsn->password);
    displayDbInfo(db);
    setCursor(Qt::ArrowCursor);
    if(db.isOpen())
    {
        _bOpen = true;
        QStringList tables = db.tables();
        if(tables.contains("Parameters",Qt::CaseInsensitive))
            if(!verifyDb())
            {
                ui->lblHelp->setStyleSheet("color: #ff8000");
                ui->lblHelp->setText(tr("connection OK but city name is not %1").arg(_city->name()));
                return;
            }
        ui->lblHelp->setStyleSheet("color: green");
        ui->lblHelp->setText(tr("The database is open on <B>%1</B> for user <B><I>%2</I></B>"
                                " using database <B><I>%3</I></B>"
                                "\nThere are %4 tables.")
                                 .arg(_dsn->server, _dsn->userId, _dsn->database).arg(tables.count()));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        return;
    }
    else
    {
        sqlError = db.lastError();
        //qDebug() << sqlError.text();
        ui->lblHelp->setStyleSheet("color: red");
        if(sqlError.text().length() < 100)
            ui->lblHelp->setText(sqlError.text());
        else
        {
            ui->textEdit->setHidden(false);
            ui->lblHelp->setText("Error: an error has occurred.");
            ui->textEdit->setText(sqlError.text());
        }
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

void DialogNewConnection::buttonClicked(QAbstractButton* b)
{
    if(b->text() == "Test" )
        onTestClicked();
}

void DialogNewConnection::onTestClicked()
{
    qDebug() << "test clicked";
    ui->lblHelp->clear();
    ui->textEdit->clear();
    if(_dsn->server.isEmpty() && ui->txtServer->text().isEmpty())
    {
        ui->lblHelp->setStyleSheet("color: #ff8000;");
        ui->lblHelp->setText("A server is needed");
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("testConnection");
    if(db.isOpen()) db.close();
    if(_dbType == "MimerSql")
    {
        Connection::connectToCityMimer(_dsn->database, ui->txtServer->text(), "testConnection");
        db = QSqlDatabase::database("testConnection");
        QSqlError sqlErr = db.lastError();
        if(db.isOpen())
        {
            _connectString = db.databaseName();
            db.close(); // Close immediately after capturing string parameter info
        }
    } else if(_dbType == "PostgresSQL") {
        Connection::connectToPostgreSQL(_dsn, _dsn->server, "testConnection", ui->txtDbName->text(),
                                        ui->txtUid->text(), ui->txtPWD->text());
    } else {
        if(QSqlDatabase::contains("testConnection"))
        {
            QSqlDatabase oldDb = QSqlDatabase::database("testConnection");
            if(oldDb.isOpen()) oldDb.close();
            QSqlDatabase::removeDatabase("testConnection");
        }
        db = QSqlDatabase::addDatabase(_driver, "testConnection");
        db.setDatabaseName(_dsn->name);
    }
    if(ui->txtUid->text().isEmpty())
    {
        if(_dsn->userId.isEmpty())
        db.open();
        else
        {
            db.open(_dsn->userId,_dsn->password);
        }
    }
    else
    {
        setCursor(Qt::WaitCursor);
        db.open(ui->txtUid->text(), ui->txtPWD->text());
        setCursor(Qt::ArrowCursor);
    }
    if(db.isOpen())
    {
        _bOpen = true;
        QStringList tables = db.tables();
        if(tables.contains("Parameters",Qt::CaseInsensitive))
            if(!verifyDb())
            {
                ui->lblHelp->setStyleSheet("color: #ff8000");
                ui->lblHelp->setText(tr("connection OK but city name is not %1").arg(_city->name()));
                return;
            }
        ui->lblHelp->setStyleSheet("color: green");
        ui->lblHelp->setText(tr("The database is open on <B>%1</B> for user <B><I>%2</I></B>"
                                " using database <B><I>%3</I></B>"
                                "\nThere are %4 tables.")
                                 .arg(_dsn->server, _dsn->userId, ui->txtDbName->text()).arg(tables.count()));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        return;
    }
    else
    {
        QSqlError sqlError = db.lastError();
        //qDebug() << sqlError.text();
        ui->lblHelp->setStyleSheet("color: red");
        ui->lblHelp->setText("Error: an error has occurred.");
        ui->textEdit->setPlainText(sqlError.text());
        return;
    }
}

void DialogNewConnection::setCity(City* city){
    _city = city;
    _cityLabel = _city->getLabel();
}

// verify that we have connected to the correct database
bool DialogNewConnection::verifyDb()
{
    QSqlDatabase db = QSqlDatabase::database("testConnection");
    if(_dbType == "MsSql" || _dbType == "Mysql")
        if(!sql->useDatabase(ui->txtDbName->text(), _dbType, db))
        {
            ui->lblHelp->setStyleSheet("color: red");
            ui->lblHelp->setText(tr("connection to server OK but unable to use database %1.")
                                     .arg(ui->txtDbName->text()));
            return false;
        }
    if(db.tables().contains("Parameters",Qt::CaseInsensitive))
    {
        Parameters* parms =new Parameters();
        sql->getParameters(parms,db);
        if(parms->city == _city->name())
            return true;
    }
    return false;
}