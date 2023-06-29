#include "modifyroutetractiontypedlg.h"
#include "ui_modifyroutetractiontypedlg.h"
#include "sql.h"

ModifyRouteTractionTypeDlg::ModifyRouteTractionTypeDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModifyRouteTractionTypeDlg)
{
 ui->setupUi(this);
 ui->comboBox->clear();
 QList<TractionTypeInfo> tractionTypeList = sql->getTractionTypes();
 foreach(TractionTypeInfo ti, tractionTypeList)
 {
  ui->comboBox->addItem(ti.ToString(), ti.tractionType);
 }
}

ModifyRouteTractionTypeDlg::~ModifyRouteTractionTypeDlg()
{
 delete ui;
}

void ModifyRouteTractionTypeDlg::setConfiguration(Configuration* config)
{
 this->config = config;
}

void ModifyRouteTractionTypeDlg::setRouteData(QList<RouteData> routeList, int currentIx)
{
 this->routeList = routeList;
 _currentIx = currentIx;
 rd = routeList.at(currentIx);
 _rd = &rd;

 ui->lblError->setText("");
 ui->lblRoute->setText(_rd->toString());

 ui->dateEdit->setDate(_rd->startDate);
 ui->comboBox->findData(rd.tractionType);
}

RouteData* ModifyRouteTractionTypeDlg::getRouteData()
{
 return _rd;
}

void ModifyRouteTractionTypeDlg::dateChanged(QDate date)
{
  if (date >= _rd->endDate || date >= _rd->startDate)
  {
   ui->lblError->setText(tr("Invalid date"));
   //System.Media.SystemSounds.Exclamation.Play();
   ui->dateEdit->setFocus();
   QApplication::beep();

   return;
  }
}

void ModifyRouteTractionTypeDlg::btnOK_Click()      //SLOT
{
 bool bResult = true;
 ui->lblError->setText("");
 if(ui->dateEdit->date() < _rd->startDate)
 {
     ui->lblError->setText(tr("New end date < route start date!"));
     QApplication::beep();
     return;
 }
 QList<RouteData> rdList = sql->getRouteSegmentsForDate(_rd->route, _rd->name, _rd->startDate.toString("yyyy/MM/dd"));
 if(rdList.count())
 {
  sql->BeginTransaction("ModifyRouteTractionType");
  foreach(RouteData rd, rdList)
  {
   if(ui->dateEdit->date() >= rd.startDate && ui->dateEdit->date()< rd.endDate)
   {
    if(!sql->deleteRouteSegment(rd.route, rd.name, rd.lineKey, rd.startDate.toString("yyyy/MM/dd"),rd.endDate.toString("yyyy/MM/dd")))
    {
     sql->CommitTransaction("ModifyRouteTractionType");
     this->reject();
    }
    RouteData rd1(rd);
    rd1.endDate = ui->dateEdit->date().addDays(-1);
    if(!sql->addSegmentToRoute(rd1))
    {
     sql->RollbackTransaction("ModifyRouteTractionType");
     this->reject();
    }
    RouteData rd2(rd);
    rd2.startDate = ui->dateEdit->date();
    rd2.tractionType = ui->comboBox->currentData().toInt();
    if(!sql->addSegmentToRoute(rd2))
    {
     sql->RollbackTransaction("ModifyRouteTractionType");
     this->reject();
    }

   }
  }
  sql->CommitTransaction("ModifyRouteTractionType");
 }
}
