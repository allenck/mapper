#include "modifyroutetractiontypedlg.h"
#include "ui_modifyroutetractiontypedlg.h"
#include "sql.h"

ModifyRouteTractionTypeDlg::ModifyRouteTractionTypeDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModifyRouteTractionTypeDlg)
{
  common();
}

ModifyRouteTractionTypeDlg::ModifyRouteTractionTypeDlg(RouteData rd,QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModifyRouteTractionTypeDlg)
{
 common();
  _rd = rd;
  ui->dateEdit->setDate(_rd.startDate());
  ui->lblRoute->setText(rd.toString());
}

void ModifyRouteTractionTypeDlg::common()
{
 ui->setupUi(this);
 ui->comboBox->clear();
 ui->lblError->setText("");
 QMap<int,TractionTypeInfo> tractionTypeList = sql->getTractionTypes();
 foreach(TractionTypeInfo ti, tractionTypeList.values())
 {
  ui->comboBox->addItem(ti.ToString(), ti.tractionType);
 }
 config = Configuration::instance();
 sql = SQL::instance();
 connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
 connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

ModifyRouteTractionTypeDlg::~ModifyRouteTractionTypeDlg()
{
 delete ui;
}

//void ModifyRouteTractionTypeDlg::setConfiguration(Configuration* config)
//{
// this->config = config;
//}

void ModifyRouteTractionTypeDlg::setRouteData(QList<RouteData> routeList, int currentIx)
{
 this->routeList = routeList;
 _currentIx = currentIx;
 _rd = routeList.at(currentIx);
 _sd = &sd;

 ui->lblError->setText("");
 ui->lblRoute->setText(_sd->toString());

 ui->dateEdit->setDate(_sd->startDate());
 ui->comboBox->findData(sd.tractionType());
}

RouteData ModifyRouteTractionTypeDlg::getRouteData()
{
 return _rd;
}

void ModifyRouteTractionTypeDlg::dateChanged(QDate date)
{
  if (date >= _sd->endDate() || date <= _sd->startDate())
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
  if(ui->dateEdit->date() < _rd.startDate())
  {
     ui->lblError->setText(tr("New start date < route start date!"));
     QApplication::beep();
     return;
  }
  if(ui->dateEdit->date() >= _rd.endDate())
  {
     ui->lblError->setText(tr("New end date > route end date!"));
     QApplication::beep();
     return;
  }

  if(ui->comboBox->currentData().toInt()== _rd.tractionType())
  {
     ui->lblError->setText(tr("traction type must be changed!"));
     QApplication::beep();
     return;
  }

  QList<SegmentData*> sdList = sql->getSegmentDataList(_rd);
  if(sdList.count())
  {
    sql->beginTransaction("ModifyRouteTractionType");
    foreach(SegmentData* sd, sdList)
    {
     if(ui->dateEdit->date() == sd->startDate())
     {
      SegmentData sd1(*sd);
      sd1.setTractionType(ui->comboBox->currentData().toInt());
      if(!sql->updateRoute(*sd, sd1))
      {
       sql->rollbackTransaction("ModifyRouteTractionType");
       this->reject();
      }
     }
     else  if(ui->dateEdit->date() > sd->startDate() && ui->dateEdit->date()< sd->endDate())
     {
        SegmentData sd1(*sd);
        sd1.setEndDate(ui->dateEdit->date().addDays(-1));
        if(!sql->updateRoute(*sd, sd1))
        {
         sql->rollbackTransaction("ModifyRouteTractionType");
         this->reject();
        }
        SegmentData sd2(*sd);
        sd2.setStartDate(ui->dateEdit->date());
        sd2.setTractionType(ui->comboBox->currentData().toInt());
        if(!sql->addSegmentToRoute(&sd2))
        {
         sql->rollbackTransaction("ModifyRouteTractionType");
         this->reject();
        }
       }
      }
      sql->commitTransaction("ModifyRouteTractionType");

      if(ui->checkBox->isChecked())
      {
         QString text = "Traction type changed to " + ui->comboBox->currentText();
          int commentKey =sql->addComment(text, "changeTT",QList<int>());
         RouteComments rc;
         rc.route = _rd.route();
         rc.commentKey = commentKey;
         rc.routeAlpha = _rd.alphaRoute();
         rc.date= ui->dateEdit->date();
         rc.companyKey = _rd.companyKey();
         if(!sql->updateRouteComment(rc))
         {
             //sql->rollbackTransaction("ModifyRouteTractionType");
             this->reject();
         }
      }
  }
  done(QDialog::Accepted);
}
