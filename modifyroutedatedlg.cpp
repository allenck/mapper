#include "modifyroutedatedlg.h"
#include "myapplication.h"
#include "ui_modifyroutedatedlg.h"
#include <QPushButton>

ModifyRouteDateDlg::ModifyRouteDateDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyRouteDateDlg)
{
 Q_UNUSED(parent)
 ui->setupUi(this);
 _dtFormat = ((MyApplication*)QApplication::instance())->dateFormat();

 QPushButton* btnOK = new QPushButton(tr("OK"));
 ui->buttonBox->addButton(btnOK, QDialogButtonBox::ApplyRole);
 this->setWindowTitle(tr("Modify Route dates"));
 connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),this, SLOT(btnOK_Click()));
 connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(btnCancel_Click()));
 connect(ui->rbStart, SIGNAL(toggled(bool)), this, SLOT(rbStartToggled(bool)));
 connect(ui->dateTimePicker1, SIGNAL(dateChanged(QDate)), this, SLOT(dateTimePicker1_ValueChanged(QDate)));

 _ri = NULL;
 _otherRi = NULL;
 routeList = QList<RouteInfo>();
 _currentIx= -1;
 sql = SQL::instance();
}

ModifyRouteDateDlg::~ModifyRouteDateDlg()
{
 delete ui;
}

//void ModifyRouteDateDlg::setConfiguration(Configuration *cfg)
//{
// config = cfg;
//}

void ModifyRouteDateDlg::setRouteData(RouteInfo* rd)
{
// this->routeList = routeList;
// _currentIx = currentIx;
// rd = routeList.at(currentIx);
 _ri = rd;

 ui->lblError->setText("");
 ui->lblRoute->setText(_ri->toString());
 //ui->dateTimePicker1->setDateTime(_rd->endDate);
 ui->rbEnd->setChecked(true);
 rbStartToggled(false);
 cd = sql->getCompany(_ri->companyKey());
 if(cd)
 {
     maxEndDate = cd->endDate;
 }
 else {
     throw IllegalArgumentException("invalid company");
 }
 QDate nextStartDate = sql->getNextStartOrEndDate(_ri->route(), _ri->startDate(),  0, true);
 if(nextStartDate.isValid() && nextStartDate < maxEndDate )
 {
     maxEndDate = nextStartDate.addDays(-1);
     ui->dateTimePicker1->setDate(maxEndDate);
     ui->lblError->setText(tr("Default end date set to %1 because \n"
                              "routes are already present after that date").arg(maxEndDate.toString(_dtFormat)));
 }
}

RouteInfo* ModifyRouteDateDlg::getRouteData()
{
 return _ri;
}

void ModifyRouteDateDlg::dateTimePicker1_ValueChanged(QDate date) //SLOT
{
 ui->lblError->setText("");
 if (ui->rbStart->isChecked())
 {
  if (date >= _ri->endDate())
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
  if (date <= _ri->startDate())
  {
   ui->lblError->setText(tr("Invalid date"));
   //System.Media.SystemSounds.Exclamation.Play();
   QApplication::beep();

   ui->dateTimePicker1->setFocus();
   return;
  }
  if(maxEndDate.isValid() && date > maxEndDate)
  {
      ui->lblError->setText(tr("Invalid date: must be less or equal than %1")
                                .arg(maxEndDate.toString(_dtFormat)));
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
  if(ui->dateTimePicker1->date() > _ri->endDate())
  {
      ui->lblError->setText(tr("New start date > route end date!"));
      QApplication::beep();
      return;
  }
 }
 else
 {
  if(ui->dateTimePicker1->date() < _ri->startDate())
  {
      ui->lblError->setText(tr("New end date < route start date!"));
      QApplication::beep();
      return;
  }
 }
 // Modify later routes' dates?
 if(!ui->checkBox->isChecked())
 {
  if(ui->rbStart->isChecked())
  {
   // see if there is a prior route.
   if(_currentIx > 0)
   {
    oRi  = routeList.at(_currentIx - 1);
   _otherRi = &oRi;
   }
  }
  else
  {
   // see if there is a later route
   if(_currentIx < routeList.size() - 1)
   {
    ri = routeList.at(_currentIx + 1);
    _otherRi = &ri;
   }
  }
 }
 else
  _otherRi = NULL;

 if(ui->rbStart->isChecked())
 {
  if(ui->dateTimePicker1->date() > _ri->endDate())
  {
   qDebug() << "can't set start date to later than end date!";
   ui->lblError->setText(tr("can't set start date %1 to later than end date: %2")
                         .arg(ui->dateTimePicker1->date().toString(_dtFormat),
                              ri.endDate().toString("yyy/MM/dd")));
   return;
  }
 }
 else
 {
  if(ui->dateTimePicker1->date() < _ri->startDate())
  {
   qDebug() << "can't set end date to before start date!";
   ui->lblError->setText(tr("can't set end date %1 to before start date:%2")
                         .arg(ui->dateTimePicker1->date().toString(_dtFormat),
                              ri.startDate().toString(_dtFormat)));

   return;
  }
 }

 setCursor(Qt::WaitCursor);
 if (sql->modifyRouteDate(_ri, ui->rbStart->isChecked(), ui->dateTimePicker1->date()/*,
                          ui->txtName1->text(), ui->txtName2->text()*/))
 {
  if (ui->rbStart->isChecked())
   _ri->startDate() = ui->dateTimePicker1->date();
  else
   _ri->endDate() = ui->dateTimePicker1->date();;

  this->accept();
  this->close();
 }
 else
 {
  ui->lblError->setText(tr("Nothing changed!"));
  ui->dateTimePicker1->setFocus();
  setCursor(Qt::ArrowCursor);
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
  ui->dateTimePicker1->setDate(_ri->startDate());
  ui->checkBox->setText(tr("Modify prior route end dates"));
  // changing startDate, get the name of the route before this.
  ui->lblName1->setText("PriorName:");
  ui->txtName1->setText(sql->getPrevRouteName(ri.startDate()));
  ui->lblName2->setText("CurrentName:");
  ui->txtName2->setText(ri.routeName());
 }
 else
 {
  ui->dateTimePicker1->setDate(_ri->endDate());
  ui->checkBox->setText(tr("Modify subsequent route start dates"));
  ui->lblName1->setText("CurrName:");
  ui->txtName1->setText(ri.routeName());
  ui->lblName2->setText("NextName:");
  ui->txtName2->setText(sql->getNextRouteName(ri.endDate()));

 }
 ui->checkBox->setChecked(false);
 if(ui->rbStart->isChecked() && _currentIx > 0)
 {
  RouteInfo rd = routeList.at(_currentIx - 1);
  if(_ri->startDate().addDays(-1) == rd.endDate())
   ui->checkBox->setChecked(true);
 }
 if(ui->rbEnd->isChecked() &&  _currentIx < routeList.size()-1)
 {
  RouteInfo rd = routeList.at(_currentIx + 1);
  if(_ri->endDate().addDays(+1) == rd.startDate())
   ui->checkBox->setChecked(true);
 }
}
