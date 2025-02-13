#include "dialogeditparameters.h"
#include "ui_dialogeditparameters.h"
#include "sql.h"
#include "data.h"
#include "configuration.h"

DialogEditParameters::DialogEditParameters(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogEditParameters)
{
 ui->setupUi(this);
 parms = SQL::instance()->getParameters();
 ui->txtName->setText(parms.city);
 ui->txtTitle->setText(parms.title);
 ui->txtLatLng->setText(QString("%1,%2").arg(parms.lat).arg(parms.lon));
 ui->chkAlpha->setChecked(parms.bAlphaRoutes);
 connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(btnOk()));
 connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
 LatLng center(parms.lat, parms.lon);
 if(Configuration::instance()->currCity->bounds().isValid())
  ui->bndsLabel->setText(tr("Bounds are valid."));
 else
  ui->bndsLabel->setText(tr("Bounds are invalid."));
 connect(ui->txtLatLng,&QLineEdit::editingFinished,this,[=]{
     QStringList sl = ui->txtLatLng->text().split(",");
     bool b1,b2;
     double lat,lng;
     LatLng latlng;
     if(sl.count()==2 )
     {
         lat = sl.at(0).toDouble(&b1);
         lng = sl.at(1).toDouble(&b2);
         if(b1 && b2)
         {
             latlng = LatLng(lat,lng);
             if(!latlng.isValid())
                 ui->txtLatLng->setStyleSheet("color: red");
             else
             {
                 ui->txtLatLng->setStyleSheet("color: black");
                 parms.lat=lat;
                 parms.lon = lng;
             }
         }
     }
 });
}

DialogEditParameters::~DialogEditParameters()
{
 delete ui;
}

void DialogEditParameters::btnOk()
{
 if(ui->txtName->text().isEmpty())
  return;
 if(ui->txtTitle->text().isEmpty())
  return;
 parms.city = ui->txtName->text();
 parms.title = ui->txtTitle->text();
 parms.bAlphaRoutes = ui->chkAlpha->isChecked()?"Y":"N";
 if(SQL::instance()->updateParameters(parms))
  accept();
}
