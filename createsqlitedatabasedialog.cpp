#include "createsqlitedatabasedialog.h"
#include "ui_createsqlitedatabasedialog.h"
#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sql.h"

CreateSqliteDatabaseDialog::CreateSqliteDatabaseDialog(LatLng* latLng, QWidget *parent) :
 QDialog(parent),
 ui(new Ui::createSqliteDatabaseDialog)
{
 ui->setupUi(this);
 config=Configuration::instance();
 this->setWindowTitle(tr("Create new Sqlite database"));
 strNew = tr("New city");
 this->latLng = latLng;
 ui->lblHelp->setText("");
 ui->btnOk->setEnabled(false);
 ui->btnBrowse->setEnabled(false);
 ui->cbCities->clear();
 ui->cbCities->addItem(strNew);
 foreach(City* c, config->cityList)
 {
  ui->cbCities->addItem(c->name());
 }
 ui->dtCompanyStart->setDate(QDate(1856,1,1));
 ui->dtCompanyEnd->setDate(QDate(2050,12,31));
 ui->chkCreateConnection->setChecked(true);
}

CreateSqliteDatabaseDialog::~CreateSqliteDatabaseDialog()
{
 delete ui;
}
void CreateSqliteDatabaseDialog::on_txtDbName_editingFinished()
{
 if(!ui->txtDbName->text().isEmpty())
  ui->btnBrowse->setEnabled(true);
}
void CreateSqliteDatabaseDialog::on_btnBrowse_clicked()
{
 ui->btnOk->setEnabled(false);
 QFileDialog dlg(this);
 if(ui->txtPath->text().isEmpty())
    dlg.setDirectory("Resources/databases");
 else
  dlg.setDirectory(ui->txtPath->text());
 dlg.setFileMode(QFileDialog::Directory);
 if(dlg.exec())
 {
  QStringList list = dlg.selectedFiles();
  if(list.length()>0 && ui->txtDbName->text().length()>0)
  {
   ui->btnOk->setEnabled(true);
   ui->txtPath->setText(list.at(0));
  }
 }
}

void CreateSqliteDatabaseDialog::on_btnCancel_clicked()
{
 this->reject();
 this->close();
}

void CreateSqliteDatabaseDialog::on_btnOk_clicked()
{
 if(ui->cbCities->currentIndex() == 0  && ui->cbCities->currentText() == strNew)
 {
  ui->lblHelp->setText(tr("Enter new city name or select an existing one"));
  return;
 }
 QString dbName = ui->txtDbName->text();
 if(!dbName.toLower().endsWith(".sqlite3"))
  dbName.append(".sqlite3");
 QString newFile;
 if(ui->txtPath->text() == "")
  newFile = "Resources/databases/" + dbName;
 else
  newFile = ui->txtPath->text() + "/" + dbName;
 SQL* sql = SQL::instance();

 QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "newDb");
 db.setDatabaseName(newFile);
 if(!db.open())
 {
  qDebug()<<db.lastError();
  ui->lblHelp->setText(db.lastError().text());
  return;
 }
 QFile file(":/sqlite3_create_tables.sql");
 if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  qDebug()<<file.errorString() + " '" + file.fileName()+"'";
  ui->lblHelp->setText(file.errorString() + " '" + file.fileName()+"'");
  //return;
 }

 QTextStream in(&file);
 QString sqltext;
 while(!in.atEnd())
 {
  sqltext = sqltext +  in.readLine();
  if(sqltext.contains(";"))
  {
   QSqlQuery query = QSqlQuery(db);
   if(!query.exec(sqltext))
   {
    QSqlError err = query.lastError();
    ui->lblHelp->setText(err.text());
    return;
   }
   sqltext="";
  }
 }


 bool bSuccess = true;
 QString commandString;
 sql->beginTransaction("setup");
 // Populate the Parameters table
  commandString = "insert into Parameters  (lat, lon, title, city, minDate, maxDate, alphaRoutes) values(:lat, :lon, :title, :city, :minDate,:maxDate, :alphaRoute)";
 QSqlQuery query = QSqlQuery(db);
 query.prepare(commandString);
 query.bindValue(":lat", latLng->lat());
 query.bindValue(":lon", latLng->lon());
 query.bindValue(":title", ui->edTitle->text().trimmed());
 query.bindValue(":city", ui->cbCities->currentText());
 query.bindValue(":minDate", "1856/01/01");
 query.bindValue(":maxDate", "2000/01/01");
 query.bindValue("alphaRoutes", "Y");
 if(!query.exec())
 {
  SQLERROR(query);
  bSuccess = false;
 }

 if(!sql->updateTractionType(1, "Electric", "#FF0000", 0, db))
  bSuccess = false;
 if(!sql->updateTractionType(2, "Horse/Mule", "#61380b",0, db))
  bSuccess = false;

 commandString = "insert into Companies ( Description, startDate, endDate) values(:Description, :startDate, :endDate)";
 query.prepare(commandString);
 query.bindValue(":Description", ui->edCompany->text().trimmed());
 query.bindValue("dtCompayStart", ui->dtCompanyStart->date().toString("yyyy/MM/dd"));
 query.bindValue("dtCompayEnd", ui->dtCompanyEnd->date().toString("yyyy/MM/dd"));
 if(!query.exec())
 {
  SQLERROR(query);
  bSuccess = false;
 }
 if(bSuccess)
 {
  sql->commitTransaction("setup");
  if(ui->chkCreateConnection->isChecked())
  {
   City* c = NULL;
   int cId=0;
   if(ui->cbCities->currentIndex()==0)
   {
    City* nc = new City();
    nc->setName(ui->cbCities->currentText());
    nc->id = cId = config->cityList.count();
    nc->center = LatLng(latLng->lat(),latLng->lon());
    nc->mapType = "roadmap";
    nc->zoom = 10;
    nc->curConnectionId = 0;
    config->cityList.append(nc);
   }
   else
   {
    cId = ui->cbCities->currentIndex()-1;
   }
   c = config->cityList.at(cId);

   Connection* cn = new Connection();
   cn->setServerType("Sqlite");
   cn->setDriver("QSQLITE");
   cn->setDatabase(newFile);
   QFileInfo info(newFile);
   cn->setDescription ("Sqlite " + ui->cbCities->currentText());
   cn->setId(c->connections.count());
   if(!c->connections.contains(cn))
   {
//    c->connections.append(cn);
//    c->connectionNames.append(cn->description());
       c->addConnection(cn);
   }
   config->saveSettings();
  }
  this->accept();}
 else
  sql->rollbackTransaction("setup");

 this->close();
}

void CreateSqliteDatabaseDialog::on_cbCities_currentIndexChanged(int index)
{
 if(index > 0 )
 {
  ui->edCompany->setEnabled(false);
  ui->dtCompanyEnd->setEnabled(false);
  ui->dtCompanyStart->setEnabled(false);
  ui->btnOk->setEnabled(true);
 }
 else
 {
  ui->edCompany->setEnabled(true);
  ui->dtCompanyEnd->setEnabled(true);
  ui->dtCompanyStart->setEnabled(true);
  if(ui->cbCities->currentText() != strNew)
   ui->btnOk->setEnabled(true);
  else
   ui->btnOk->setEnabled(false);
 }
}

void CreateSqliteDatabaseDialog::on_edTitle_editingFinished()
{
 if(ui->edTitle->text().isEmpty())
  ui->btnOk->setEnabled(false);
}
