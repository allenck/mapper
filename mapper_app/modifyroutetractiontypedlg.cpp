#include "modifyroutetractiontypedlg.h"
#include "ui_modifyroutetractiontypedlg.h"
#include "sql.h"

ModifyRouteTractionTypeDlg::ModifyRouteTractionTypeDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModifyRouteTractionTypeDlg)
{
 ui->setupUi(this);
 ui->comboBox->clear();
 QMap<int,TractionTypeInfo> tractionTypeList = sql->getTractionTypes();
 foreach(TractionTypeInfo ti, tractionTypeList.values())
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
 RouteData rd = routeList.at(currentIx);
 _sd = &sd;

 ui->lblError->setText("");
 ui->lblRoute->setText(_sd->toString());

 ui->dateEdit->setDate(_sd->startDate());
 ui->comboBox->findData(sd.tractionType());
}

SegmentData *ModifyRouteTractionTypeDlg::getRouteData()
{
 return _sd;
}

void ModifyRouteTractionTypeDlg::dateChanged(QDate date)
{
  if (date >= _sd->endDate() || date >= _sd->startDate())
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
 if(ui->dateEdit->date() < _sd->startDate())
 {
     ui->lblError->setText(tr("New end date < route start date!"));
     QApplication::beep();
     return;
 }
 QList<SegmentData> rdList = sql->getRouteSegmentsForDate(_sd->route(), _sd->routeName(), _sd->startDate().toString("yyyy/MM/dd"));
 if(rdList.count())
 {
  sql->beginTransaction("ModifyRouteTractionType");
  foreach(SegmentData sd, rdList)
  {
   if(ui->dateEdit->date() >= sd.startDate() && ui->dateEdit->date()< sd.endDate())
   {
    //if(!sql->deleteRouteSegment(sd._route, sd._name, sd._lineKey, sd._startDate.toString("yyyy/MM/dd"),sd._endDate.toString("yyyy/MM/dd")))
    if(!sql->deleteRouteSegment(sd))
    {
     sql->commitTransaction("ModifyRouteTractionType");
     this->reject();
    }
    SegmentData sd1(sd);
    sd1.setEndDate(ui->dateEdit->date().addDays(-1));
    if(!sql->addSegmentToRoute(sd1))
    {
     sql->rollbackTransaction("ModifyRouteTractionType");
     this->reject();
    }
    SegmentData sd2(sd);
    sd2.setStartDate(ui->dateEdit->date());
    sd2.setTractionType(ui->comboBox->currentData().toInt());
    if(!sql->addSegmentToRoute(sd2))
    {
     sql->rollbackTransaction("ModifyRouteTractionType");
     this->reject();
    }

   }
  }
  sql->commitTransaction("ModifyRouteTractionType");
 }
}
