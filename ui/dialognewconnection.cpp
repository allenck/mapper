#include "dialognewconnection.h"
#include "qcompleter.h"
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
#include <QHostInfo>

DialogNewConnection::DialogNewConnection(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNewConnection)
{
    ui->setupUi(this);
    config = Configuration::instance();
    sql= SQL::instance();
    ui->lblHelp->setStyleSheet("color: green");
    ui->tbView->setChecked(false);
    drivers = QSqlDatabase::drivers();
    odbcUtil = ODBCUtil::instance();
    odbcUtil->initialize();
    ui->lblDsn->setHidden(true);
    ui->cbDsn->setHidden(true);
    ui->textEdit->setHidden(true);
    ui->textEdit->setReadOnly(true);

    ui->label_7->setHidden(true);
    ui->lblDriver->setHidden(true);
    ui->label_8->setHidden(true);
    ui->lblDSNType->setHidden(true);

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
    connect(ui->txtPWD, &QLineEdit::editingFinished,this,[=](){
        onPasswordEntered();
    });
    connect(ui->txtDbName, &QLineEdit::selectionChanged,this,[=](){
        ui->lblHelp->setStyleSheet("color:magenta");
        ui->lblHelp->setText(tr("please verify that your description is represetative."));
    });
    connect(ui->txtDescription, &QLineEdit::editingFinished,this,[=](){
        _description = ui->txtDescription->text();
    });
    connect(ui->txtServer, &QLineEdit::editingFinished, this, [=](){
        checkHost(ui->txtServer->text());
    });
}

DialogNewConnection::~DialogNewConnection()
{
    delete ui;

    QSqlDatabase db = QSqlDatabase::database(_testCN);
    if(db.isOpen()) db.close();
    QSqlDatabase::removeDatabase(_testCN);
    db = QSqlDatabase();
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
        ui->gb1->setVisible(true);
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
        ui->gb1->setVisible(true);
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
    if(connect == "Sqlite")
    {
        ui->label_7->setHidden(true);
        ui->lblDriver->setHidden(true);
        ui->label_8->setHidden(true);
        ui->lblDSNType->setHidden(true);

    } else
    if(connect == "Direct")
    {
        ui->lblDsn->setHidden(true);
        ui->cbDsn->setHidden(true);
        ui->label_7->setHidden(true);
        ui->lblDriver->setHidden(true);
        ui->label_8->setHidden(true);
        ui->lblDSNType->setHidden(true);

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
        ui->label_7->setHidden(false);
        ui->lblDriver->setHidden(false);
        ui->label_8->setHidden(true);
        ui->lblDSNType->setHidden(false);

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

    _dsn = odbcUtil->getDsn(ui->cbDsn->itemData(ix).toString());
    _bOpen = false;
    ui->txtServer->setText(_dsn->server);
    ui->txtDbName->clear();
    ui->txtDescription->setText(_dsn->descr);
    ui->txtDbName->setText(_dsn->database);
    ui->lblDriver->setText(_dsn->driverName);
    ui->lblDSNType->setText(_dsn->userDsn?"user DSN":"systemDSN");
    ui->txtUid->clear();
    ui->txtPWD->clear();
    qApp->processEvents();

    _dbType = ui->cbDbType->currentText();

    if(_dbType == "MimerSql")
    {
        Connection::connectToCityMimer(_dsn->database, _dsn->server, _testCN);
        db = QSqlDatabase::database(_testCN);
        QSqlError sqlErr = db.lastError();
        if(db.isOpen())
        {
            _connectString = db.databaseName();
            db.close(); // Close immediately after capturing string parameter info
        }
        //QSqlDatabase::removeDatabase(_testCN);
    } else {
        if(QSqlDatabase::contains(_testCN))
        {
            QSqlDatabase oldDb = QSqlDatabase::database(_testCN);
            if(oldDb.isOpen()) oldDb.close();
            QSqlDatabase::removeDatabase(_testCN);
        }
        db = QSqlDatabase::addDatabase(_driver, _testCN);
        db.setDatabaseName(_dsn->name);
        // Force the ODBC driver to drop the request quickly if the server doesn't respond
        db.setConnectOptions("SQL_ATTR_CONNECTION_TIMEOUT=3;SQL_ATTR_LOGIN_TIMEOUT=3");
        if(!_dsn->userId.isEmpty() && !_dsn->password.isEmpty())
        {
            QString driver = _driver;
            QString dsnName = _dsn ? _dsn->name : "";
            QString dbType = ui->cbDbType->currentText();
            QString uid = _dsn->userId;
            QString pwd = _dsn->password;

            if(dsnName.isEmpty() || uid.isEmpty()) {
                ui->lblHelp->setStyleSheet("color: #ff8000;");
                ui->lblHelp->setText("Missing DSN selection or username credentials.");
                return;
            }
            logonInBackground(driver, dsnName,dbType, uid,pwd);
        }
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
            if(!verifyDb(_dsn->database))
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
    qDebug() <<(tr(" connectOptions: %1").arg(db.connectOptions()));
    qDebug() <<(tr(" isOpen: %1; isValid: %2 ").arg(db.isOpen()?"true":"false", db.isValid()?"true":"false"));
    qDebug() <<(tr(" last error: %1 ").arg(db.lastError().text()));
}

void DialogNewConnection::buttonClicked(QAbstractButton* b)
{
    if(b->text() == "Test" )
        onTestClicked();
    if(b->text() == "OK")
        onOKClicked();
}

void DialogNewConnection::onOKClicked()
{
    QSqlDatabase db = QSqlDatabase::database(_testCN);
    _connectString = db.databaseName();
}

void DialogNewConnection::onTestClicked()
{
    qDebug() << "test clicked";
    ui->lblHelp->clear();
    ui->textEdit->clear();

    if(_dsn &&_dsn->server.isEmpty() && ui->txtServer->text().isEmpty())
    {
        ui->lblHelp->setStyleSheet("color: #ff8000;");
        ui->lblHelp->setText("A server is needed");
        return;
    }

    // Clean up older connection instance on the main thread safely
    QSqlDatabase db = QSqlDatabase::database(_testCN);
    if(db.isOpen()) db.close();

    if(_dbType == "MimerSql")
    {
        if(_connect == "ODBC")
        {
            Connection::connectToCityMimer(_dsn->database, ui->txtServer->text(), _testCN);
            db = QSqlDatabase::database(_testCN);
            QSqlError sqlErr = db.lastError();
            if(db.isOpen())
            {
                _connectString = db.databaseName();
                db.close(); // Close immediately after capturing string parameter info
            }
        }
        else
        {
            _dsn = new DSN();
            _dsn->database = ui->txtDbName->text();
            _dsn->server = ui->txtServer->text();
            _dsn->port = 1360;
            _dsn->userId = ui->txtUid->text();
            _dsn->password = ui->txtPWD->text();
            // Connection::connectToPostgreSQL(nullptr, ui->txtServer->text(), _testCN, ui->txtDbName->text(),
            //                                 ui->txtUid->text(), ui->txtPWD->text());
            if (QSqlDatabase::contains(_testCN)) {
                {
                    QSqlDatabase oldDb = QSqlDatabase::database(_testCN);
                    if (oldDb.isOpen()) oldDb.close();
                }
                // FIX WARNING: Clear the outer 'db' reference before removing from registry
                db = QSqlDatabase();
                QSqlDatabase::removeDatabase(_testCN);
            }

            // Initialize the primary QODBC driver base
            db = QSqlDatabase::addDatabase("QMIMER", _testCN);
            db.setDatabaseName(_dsn->database);
            db.setUserName(_dsn->userId);
            db.setPassword(_dsn->password);

            // FIX CONNECTION: Use Mimer-native parameter keywords (PROTOCOL=tcp and SERVICE=1360)
            QString connectOptions = QString("PROTOCOL=tcp;NODE=%1;SERVICE=%2;DATABASE=%3")
                                         .arg(_dsn->server)
                                         .arg(1360)
                                         .arg(_dsn->database);db.setConnectOptions(connectOptions);

            if (!db.open()) {
                qDebug() << "MimerSql Error targeting" << ui->txtDbName->text() << ":" << db.lastError().driverText();
                // return ;
            }
            else {
                displayDbInfo(db);
                qDebug() << "Successfully attached to MimerSql engine for:" << ui->txtDbName->text();
                //return ;
            }
        }
    } else if(_dbType == "PostgreSQL")
    {
        if(_connect == "ODBC")
        {
            Connection::connectToPostgreSQL(_dsn, _dsn->server, _testCN, ui->txtDbName->text(),
                                        ui->txtUid->text(), ui->txtPWD->text());
            if(db.isOpen())
            {
                _connectString = db.databaseName();
                db.close(); // Close immediately after capturing string parameter info
            }
        }
        else
        {
            _dsn = new DSN();
            _dsn->database = ui->txtDbName->text();
            _dsn->server = ui->txtServer->text();
            _dsn->userId = ui->txtUid->text();
            _dsn->password = ui->txtPWD->text();
            // Connection::connectToPostgreSQL(nullptr, ui->txtServer->text(), _testCN, ui->txtDbName->text(),
            //                                 ui->txtUid->text(), ui->txtPWD->text());
            if (QSqlDatabase::contains(_testCN)) {
                {
                    QSqlDatabase oldDb = QSqlDatabase::database(_testCN);
                    if (oldDb.isOpen()) oldDb.close();
                }
                QSqlDatabase::removeDatabase(_testCN);
            }

            // Initialize the primary QODBC driver base
            /*QSqlDatabase*/ db = QSqlDatabase::addDatabase("QPSQL", _testCN);
            db.setDatabaseName(_dsn->database);
            db.setHostName(_dsn->server);
            db.setUserName(_dsn->userId);
            db.setPassword(_dsn->password);
            if (!db.open()) {
                qDebug() << "PostgreSQL Error targeting" << ui->txtDbName->text() << ":" << db.lastError().driverText();
               // return ;
            }
            else {
            displayDbInfo(db);
            qDebug() << "Successfully attached to PostgreSQL engine for:" << ui->txtDbName->text();
            //return ;
            }
        }

    } else {
        if(QSqlDatabase::contains(_testCN))
        {
            QSqlDatabase oldDb = QSqlDatabase::database(_testCN);
            if(oldDb.isOpen()) oldDb.close();
            QSqlDatabase::removeDatabase(_testCN);
        }
        db = QSqlDatabase::addDatabase(_driver, _testCN);
        db.setDatabaseName((_dsn?_dsn->name:ui->txtDbName->text()));
        if(!_dsn || (ui->lblServer->text() != _dsn->server) )
        {
            db.setHostName(ui->lblServer->text());
        }
    }
    if(_connect != "Direct" && _dbType == "PostgreSQL")
    {
        if(ui->txtUid->text().isEmpty())
        {
            if(_dsn->userId.isEmpty())
            db.open();
            else
            {
                if(_dsn)
                    db.open(_dsn->userId,_dsn->password);
                else
                    db.open(ui->txtUid->text(), ui->txtPWD->text());
            }
        }
        else
        {
            setCursor(Qt::WaitCursor);
            db.open(ui->txtUid->text(), ui->txtPWD->text());
            setCursor(Qt::ArrowCursor);
        }
    }
    if(db.isOpen())
    {
        _bOpen = true;
        QStringList tables = db.tables();
        //if(tables.contains("Parameters",Qt::CaseInsensitive))
            try {
                if(!verifyDb(ui->txtDbName->text()))
                {
                    ui->lblHelp->setStyleSheet("color: #ff8000");
                    ui->lblHelp->setText(tr("connection OK but city name is not %1").arg(_city->name()));
                    return;
                }
            }
            catch (SQLException e){
                ui->lblHelp->setStyleSheet("color:red");
                ui->lblHelp->setText(e.msg);
            }

        tables = db.tables(); // get new tables list after parameters verification.
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
        if(sqlError.text().length()>80)
        {
            ui->lblHelp->setText("Error: an error has occurred.");
            ui->textEdit->setVisible(true);
            ui->textEdit->setPlainText(sqlError.text());
        }
        else {
            ui->lblHelp->setText("Error: "+sqlError.text());
        }
        displayDbInfo(db);
        return;
    }
}

void DialogNewConnection::setCity(City* city){
    _city = city;
    _cityLabel = _city->getLabel();
}

// verify that we have connected to the correct database
bool DialogNewConnection::verifyDb(QString database)
{
    QSqlDatabase db = QSqlDatabase::database(_testCN);
    if(_dbType == "MsSql" || _dbType == "MySql" || _dbType == "PostgreSQL" || _dbType =="MimerSql")
    {
        if(_dbType !="MimerSql")
        {
            if(!sql->useDatabase(database, _dbType, db))
                throw SQLException(tr("connection to server OK but unable to use database %1.").arg(ui->txtDbName->text()));
        }
        QString command;
        if(_dbType == "MySql")
            command = "Select database()";
        else if (_dbType == "PostgreSQL" || _dbType == "MimerSql")
            command = "select current_database()";
        else
            command = "Select DB_NAME()";
        QVariantList vl;
        if(sql->executeCommand(command, db, &vl))
        {
            qDebug() <<"current database: " << vl.at(0).toString();
            if(database.compare(vl.at(0).toString(),Qt::CaseInsensitive)!=0)
                throw SQLException(tr("current database(%1) is not %2").arg(database,vl.at(0).toString()) );
            return true;
        }
    }

    return false;
}
// Force the ODBC driver to drop the request quickly if the server doesn't respond
void DialogNewConnection::onPasswordEntered()
{
    // 1. Gather variables from the main thread UI elements safely
    QString driver = _driver;
    QString dsnName = _dsn ? _dsn->name : "";
    QString dbType = ui->cbDbType->currentText();
    QString uid = ui->txtUid->text();
    QString pwd = ui->txtPWD->text();

    if(dsnName.isEmpty() || uid.isEmpty()) {
        ui->lblHelp->setStyleSheet("color: #ff8000;");
        ui->lblHelp->setText("Missing DSN selection or username credentials.");
        return;
    }
    logonInBackground(driver, dsnName,dbType, uid,pwd);
}

void DialogNewConnection::logonInBackground(QString driver, QString dsnName,QString dbType,
                                            QString uid,QString pwd, bool allowOk)
{
    // 2. Adjust visual indicators on the interface to denote background progress
    setCursor(Qt::WaitCursor);
    ui->lblHelp->setStyleSheet("color: blue;");
    ui->lblHelp->setText("Querying server for available databases...");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    _lookupError.clear();

    // 3. Fire off the network query on a parallel worker thread context
    QFuture<QStringList> future = QtConcurrent::run([driver, dsnName, dbType, uid, pwd, this]() {
        QStringList databaseNames;

        // Setup thread-unique name tracking to prevent cross-contamination
        QString threadConn = QString("bg_fetch_db_%1").arg(quintptr(QThread::currentThreadId()));

        {
            QSqlDatabase db = QSqlDatabase::addDatabase(driver, threadConn);
            db.setDatabaseName(dsnName);

            // Limit timeout latency so the worker finishes rapidly on bad links
            db.setConnectOptions("SQL_ATTR_CONNECTION_TIMEOUT=3;SQL_ATTR_LOGIN_TIMEOUT=3");

            if (db.open(uid, pwd)) {
                QSqlQuery query(db);
                QString sqlString;

                // Pick query scheme matching active server environment
                if (dbType == "PostgreSQL") {
                    sqlString = "SELECT datname FROM pg_database WHERE datistemplate = false;";
                } else if (dbType == "MySql") {
                    sqlString = "SHOW DATABASES;";
                } else if (dbType == "MsSql") {
                    sqlString = "SELECT name FROM sys.databases WHERE database_id > 4;";
                }

                if (!sqlString.isEmpty() && query.exec(sqlString)) {
                    while (query.next()) {
                        QString name = query.value(0).toString();
                        if (name != "postgres" && name != "information_schema" && name != "mysql") {
                            databaseNames.append(name);
                        }
                    }
                }
                db.close();
            }
            else
            {
                QSqlError sqlError = db.lastError();
                _lookupError = sqlError.text();
            }
        }
        // Clean up database link definitions prior to worker closure
        QSqlDatabase::removeDatabase(threadConn);
        return databaseNames;
    });

    // 4. Watch for completion of background thread task
    QFutureWatcher<QStringList>* watcher = new QFutureWatcher<QStringList>(this);

    connect(watcher, &QFutureWatcher<QStringList>::finished, this, [this, watcher, allowOk]() {
        setCursor(Qt::ArrowCursor);
        QStringList results = watcher->result();
        _available = QStringList(results);

        if (!results.isEmpty()) {
            ui->lblHelp->setStyleSheet("color: green;");
            ui->lblHelp->setText(tr("Found %1 databases. Type to filter options.").arg(results.count()));

            // Clean old completers and safely update UI components on the main thread
            QCompleter* oldCompleter = ui->txtDbName->completer();
            QCompleter* newCompleter = new QCompleter(results, this);
            newCompleter->setCaseSensitivity(Qt::CaseInsensitive);

            ui->txtDbName->setCompleter(newCompleter);
            if (oldCompleter) oldCompleter->deleteLater();

            // Automatically select the first option or drop it down for accessibility
            ui->txtDbName->setText(results.first());
            if(allowOk)
            {
                QStringList tables = db.tables();
                if(tables.contains("Parameters",Qt::CaseInsensitive))
                {
                    try {
                        if(!verifyDb(ui->txtDbName->text()))
                        {
                            ui->lblHelp->setStyleSheet("color: #ff8000");
                            ui->lblHelp->setText(tr("connection OK but city name is not %1").arg(_city->name()));
                            return;
                        }
                    }
                    catch (SQLException e){
                        ui->lblHelp->setStyleSheet("color:red");
                        ui->lblHelp->setText(e.msg);
                    }
                }
                ui->lblHelp->setStyleSheet("color: green");
                ui->lblHelp->setText(tr("The database is open on <B>%1</B> for user <B><I>%2</I></B>"
                                        " using database <B><I>%3</I></B>"
                                        "\nThere are %4 tables.")
                                         .arg(_dsn->server, _dsn->userId, _dsn->database).arg(tables.count()));
                ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            }
        } else {
            ui->lblHelp->setStyleSheet("color: red;");
            ui->lblHelp->setText("Could not fetch database listings. Verify credentials or connection state.");
            ui->textEdit->setVisible(true);
            ui->textEdit->setText(_lookupError);
        }

        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

QString DialogNewConnection::useDataBase()
{
    return ui->txtDbName->text();
}

QString DialogNewConnection::uid()
{
    return ui->txtUid->text();
}

QString DialogNewConnection::pwd()
{
    return ui->txtPWD->text();
}

QString DialogNewConnection::server()
{
    return ui->txtServer->text();
}

void DialogNewConnection::checkHost(QString txtHost)
{
    if(txtHost.isEmpty())
        return;
    bCheckingHost = true;
    setCursor(Qt::WaitCursor);
    QString host = ui->txtServer->text();
    if(txtHost.contains('\\'))
        host = txtHost.mid(0,txtHost.indexOf('\\'));
    if(txtHost.contains('/'))
        host = txtHost.mid(0,txtHost.indexOf('/'));

    QHostInfo::lookupHost(host,this, &DialogNewConnection::printResults);
}

void DialogNewConnection::printResults(const QHostInfo& info)
{
    bCheckingHost = false;
    setCursor(Qt::ArrowCursor);
    ui->txtServer->setStyleSheet("color: black");

    QString hostname = info.hostName();
    QList<QHostAddress> addresses = info.addresses();
    if(info.error())
    {
        //QMessageBox::critical(this, tr("Error"), tr("Error looking up host: %1").arg(info.errorString()));
        ui->lblHelp->setText(tr("Error looking up host: %1").arg(info.errorString()));
        ui->lblHelp->setStyleSheet("color: red");
        // bDSNCanBeUsed = false;
        // ui->txtHost->setStyleSheet("color: red");
        // ui->cbConnections->setCurrentText("");
        return;
    }
    QString hostTxt = ui->txtServer->text();
    if(ODBCUtil::isIp(hostTxt ) && !(ui->txtServer->text() == "127.0.0.1" || ui->txtServer->text() == "::1"))
    {
        // setCursor(Qt::WaitCursor);
        // QHostInfo info = QHostInfo::fromName(hostTxt);
        // setCursor(Qt::ArrowCursor);
        if(hostTxt != info.hostName())
        {
            if(QMessageBox::warning(this, tr("Warning"), tr("Using IP address %1 instead of %2").arg(ui->txtServer->text(),info.hostName()), QMessageBox::Yes|QMessageBox::No)== QMessageBox::Yes)
            {
                ui->txtServer->setText(info.hostName());
                ui->txtServer->setStyleSheet("color: black;");
                // connection->setHost(info.hostName());
                // ui->cbConnections->setCurrentText("");
            }
            else
                ui->txtServer->setStyleSheet("color: #FF8000;");
        }
        else
        {
            ui->txtServer->setStyleSheet("color: red;");
            //QMessageBox::critical(this, tr("Error"), tr("Using invalid IP address %1. Enter a valid hostname or IP.").arg(ui->txtHost->text()));
            ui->lblHelp->setText(tr("Using invalid IP address %1. Enter a valid hostname or IP.").arg(ui->txtServer->text()));
            ui->lblHelp->setStyleSheet("color: red");
            // bDSNCanBeUsed = false;
            // ui->btnSave->setEnabled(false);
            // ui->cbConnections->setCurrentText("");

            return;
        }
    }
    return;
}
