#include "newcitydialog.h"
#include "editconnectionsdlg.h"
#include "qpushbutton.h"
#include "ui_newcitydialog.h"
#include "webviewbridge.h"
#include "mymessagebox.h"
#include <QApplication>
#include "webviewbridge.h"

NewCityDialog::NewCityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewCityDialog)
{
    ui->setupUi(this);
    webViewBridge = WebViewBridge::instance();
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    connect(webViewBridge, &WebViewBridge::on_cityBounds, [=](Bounds bounds){
        city = new City();
        city->setName(ui->name->text());
        city->setBounds(bounds);
        city->setCenter(bounds.center());
        Connection* nc = new Connection(); //default connection, uninitialized
        nc->setCityName(ui->name->text());
        city->addConnection(nc);
        if(ui->title->text().isEmpty())
        {
         QMessageBox::critical(this, tr("Title blank"), tr("Enter a title for this database"));
         return;
        }
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
    });
    connect(ui->name, &QLineEdit::editingFinished, [=]{
        on_cityNameEntered();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, [=]{
     if(ui->title->text().isEmpty())
     {
      QMessageBox::critical(this, tr("Title blank"), tr("Enter a title for this database"));
      return;
     }
     Configuration::instance()->addCity(city);
       EditConnectionsDlg* dlg = new EditConnectionsDlg(nullptr);
       Parameters parms;
       parms.title = ui->title->text();
       parms.lat = city->center.lat();
       parms.lon = city->center.lon();
       parms.city = city->name();
       parms.maxDate = QDate::fromString("1850/01/01","yyyy/MM/dd");
       parms.minDate = QDate::fromString("1950/12/31","yyyy/MM/dd");
       parms.bAlphaRoutes = true;
       dlg->setParameter(parms);
       dlg->setCity(city);
       int ret =dlg->exec();
       emit finished(ret);
    });
}

NewCityDialog::~NewCityDialog()
{
    delete ui;
}

void NewCityDialog::on_cityNameEntered()
{
    QString name = ui->name->text().trimmed();
    Configuration* config = Configuration::instance();
    if(config->cityNames().contains(name))
    {
        MyMessageBox::critical(this, tr("Name not unique"),
                               tr("The city name already exists! Please enter a new one."));
        ui->name->setFocus();
    }

    //webViewBridge->processScript("closeCityBoundsButton");
    webViewBridge->processScript("addCityBoundsButton");

}
