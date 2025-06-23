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
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
    });
    connect(ui->name, &QLineEdit::editingFinished, [=]{
        on_cityNameEntered();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, [=]{
       EditConnectionsDlg* dlg = new EditConnectionsDlg(nullptr);
//       Configuration::instance()->addCity(city);
//       qApp->processEvents();
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
        MyMessageBox::critical(this, tr("Name not unique"), tr("The city name already exists! Please enter a new one."));
        ui->name->setFocus();
    }

    //webViewBridge->processScript("closeCityBoundsButton");
    webViewBridge->processScript("addCityBoundsButton");

}
