#include "dialogchangeroute.h"
#include "ui_dialogchangeroute.h"
#include "sql.h"
#include <QPushButton>
#include <QIntValidator>

DialogChangeRoute::DialogChangeRoute(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogChangeRoute)
{
 ui->setupUi(this);
 ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
 ui->lowRange->setValidator(new QIntValidator(1,999));
 ui->highRange->setValidator(new QIntValidator(1,999));
 connect(ui->highRange, &QLineEdit::editingFinished, [=]{
  lowRange =ui->lowRange->text().toInt();
  highRange = ui->highRange->text().toInt();
  if(highRange - lowRange  > 1)
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  else
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
 });
 connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [=]{
  int result = SQL::instance()->nextRouteNumberInRange(lowRange, highRange);
  if(result > 0 && result < highRange)
  {
   number = result+1;
   accept();
  }
 });
}

DialogChangeRoute::~DialogChangeRoute()
{
 delete ui;
}

int DialogChangeRoute::getNumber()
{
 return number;
}
