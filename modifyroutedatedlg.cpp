#include "modifyroutedatedlg.h"
#include "ui_modifyroutedatedlg.h"


ModifyRouteDateDlg::ModifyRouteDateDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyRouteDateDlg)
{
 Q_UNUSED(parent)
 ui->setupUi(this);
 this->setWindowTitle(tr("Modify Route dates"));
 connect(ui->buttonBox, SIGNAL(accepted()),this, SLOT(btnOK_Click()));
 connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(btnCancel_Click()));
 connect(ui->rbStart, SIGNAL(toggled(bool)), this, SLOT(rbStartToggled(bool)));
 connect(ui->dateTimePicker1, SIGNAL(dateChanged(QDate)), this, SLOT(dateTimePicker1_ValueChanged(QDate)));
 _rd = NULL;
 _otherRd = NULL;
 routeList = QList<RouteData>();
 _currentIx= -1;
 sql = SQL::instance();
}

ModifyRouteDateDlg::~ModifyRouteDateDlg()
{
 delete ui;
}

void ModifyRouteDateDlg::setConfiguration(Configuration *cfg)
{
 config = cfg;
}

void ModifyRouteDateDlg::setRouteData(QList<RouteData> routeList, int currentIx)
{
 this->routeList = routeList;
 _currentIx = currentIx;
 rd = routeList.at(currentIx);
 _rd = &rd;

 ui->lblError->setText("");
 ui->lblRoute->setText(_rd->toString());
 //ui->dateTimePicker1->setDateTime(_rd->endDate);
 ui->rbEnd->setChecked(true);
 rbStartToggled(false);
}

RouteData* ModifyRouteDateDlg::getRouteData()
{
 return _rd;
}

void ModifyRouteDateDlg::dateTimePicker1_ValueChanged(QDate date) //SLOT
{
 ui->lblError->setText("");
 if (ui->rbStart->isChecked())
 {
  if (date >= _rd->endDate)
  {
   ui->lblError->setText(tr("Invalid date"));
   //System.Media.SystemSounds.Exclamation.Play();
   ui->dateTimePicker1->setFocus();
   QApplication::beep();

   return;
  }
 }
 else
 if (ui->rbEnd->isChecked())
 {
  if (date <= _rd->startDate)
  {
   ui->lblError->setText(tr("Invalid date"));
   //System.Media.SystemSounds.Exclamation.Play();
   QApplication::beep();

   ui->dateTimePicker1->setFocus();
   return;
  }
 }
 else
 {
  ui->lblError->setText(tr("Select date"));
  //System.Media.SystemSounds.Exclamation.Play();
  ui->groupBox1->setFocus();
  return;
 }
}

void ModifyRouteDateDlg::btnOK_Click()      //SLOT
{
  ui->lblError->setText("");
 if(!ui->rbStart->isChecked() && !ui->rbEnd->isChecked())
 {
  ui->lblError->setText(tr("Select date"));
  //System.Media.SystemSounds.Exclamation.Play();
  ui->groupBox1->setFocus();
  return;
 }

 if(ui->rbStart->isChecked())
 {
  if(ui->dateTimePicker1->date() > _rd->endDate)
  {
      ui->lblError->setText(tr("New start date > route end date!"));
      QApplication::beep();
      return;
  }
 }
 else
 {
  if(ui->dateTimePicker1->date() < _rd->startDate)
  {
      ui->lblError->setText(tr("New end date < route start date!"));
      QApplication::beep();
      return;
  }
 }
 if(ui->checkBox->isChecked())
 {
  if(ui->rbStart->isChecked())
  {
   // see if there is a prior route.
   if(_currentIx > 0)
   {
    oRd  = routeList.at(_currentIx - 1);
   _otherRd = &oRd;
   }
  }
  else
  {
   // see if there is a later route
   if(_currentIx < routeList.size() - 1)
   {
    rd = routeList.at(_currentIx + 1);
    _otherRd = &rd;
   }
  }
 }
 else
  _otherRd = NULL;

 if (sql->modifyRouteDate(_rd, ui->rbStart->isChecked(), ui->dateTimePicker1->date(),
                          ui->txtName1->text(), ui->txtName2->text()))
 {
  if (ui->rbStart->isChecked())
   _rd->startDate = ui->dateTimePicker1->date();
  else
   _rd->endDate = ui->dateTimePicker1->date();;

  this->accept();
  this->close();
 }
 else
 {
  ui->lblError->setText(tr("Nothing changed!"));
  ui->dateTimePicker1->setFocus();
  return;
 }
}
void ModifyRouteDateDlg::btnCancel_Click()  //SLOT
{
    this->reject();
    this->close();
}

void ModifyRouteDateDlg::rbStartToggled(bool state)
{

 if(state)
 {
  ui->dateTimePicker1->setDate(_rd->startDate);
  ui->checkBox->setText(tr("Modify prior route end dates"));
  // changing startDate, get the name of the route before this.
  ui->lblName1->setText("PriorName:");
  ui->txtName1->setText(sql->getPrevRouteName(rd.startDate));
  ui->lblName2->setText("CurrentName:");
  ui->txtName2->setText(rd.name);
 }
 else
 {
  ui->dateTimePicker1->setDate(_rd->endDate);
  ui->checkBox->setText(tr("Modify subsequent route start dates"));
  ui->lblName1->setText("CurrName:");
  ui->txtName1->setText(rd.name);
  ui->lblName2->setText("NextName:");
  ui->txtName2->setText(sql->getNextRouteName(rd.endDate));

 }
 ui->checkBox->setChecked(false);
 if(ui->rbStart->isChecked() && _currentIx > 0)
 {
  RouteData rd = routeList.at(_currentIx - 1);
  if(_rd->startDate.addDays(-1) == rd.endDate)
   ui->checkBox->setChecked(true);
 }
 if(ui->rbEnd->isChecked() &&  _currentIx < routeList.size()-1)
 {
  RouteData rd = routeList.at(_currentIx + 1);
  if(_rd->endDate.addDays(+1) == rd.startDate)
   ui->checkBox->setChecked(true);
 }
}
