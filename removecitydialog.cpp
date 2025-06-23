#include "removecitydialog.h"
#include "ui_removecitydialog.h"
#include "configuration.h"
#include "city.h"
#include "connection.h"
#include "vptr.h"
#include <QPushButton>
#include <QMessageBox>
#include <QFileInfo>
RemoveCityDialog::RemoveCityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveCityDialog)
{
    ui->setupUi(this);
    ui->comboBox->clear();
    Configuration* config = Configuration::instance();
    foreach (City* city, Configuration::instance()->cityList) {
      if(city != config->currCity)
        ui->comboBox->addItem(city->name(), VPtr<City>::asQVariant(city));
    }
    connect(ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, [=]{
       City * city = VPtr<City>::asPtr(ui->comboBox->currentData());
       if(QMessageBox::question(this, tr("Delete %1").arg(city->name()),
                                tr("Are you sure that you want to remove %1 and all its's connections?").arg(city->name())) == QMessageBox::No)
         reject();

       foreach (Connection* connection, city->connections) {
          if(connection->connectionType()=="Local" && ui->checkBox->isChecked())
          {
              QFileInfo info(connection->sqlite_fileName());
              if(info.exists())
              {
                QFile f(info.absoluteFilePath());
                f.moveToTrash();
              }
          }
       }
       config->cityNames().removeOne(city->name());
       config->cityList.removeOne(city);
       config->cityMap.remove(city->name());
       config->cityBounds.remove(city->name());
       City* currCity = config->currCity;
       for(int i= 0; i < config->cityList.count(); i++)
       {
          City* c = config->cityList.at(i);
          c->id = i;
          if(c == currCity)
              config->currentCityId = i;
       }
       config->saveSettings();
       accept();
    });
}

RemoveCityDialog::~RemoveCityDialog()
{
    delete ui;
}
