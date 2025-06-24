#include "dialogpreferences.h"
#include "qpushbutton.h"
#include "ui_dialogpreferences.h"
#include "mainwindow.h"
#include "filedownloader.h"

DialogPreferences::DialogPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogPreferences)
{
    ui->setupUi(this);
    config = Configuration::instance();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    ui->edTileServer->setText(config->tileServerUrl);
    tileServerUrl = new QUrl(config->tileServerUrl);

    MainWindow::instance()->initRouteSortCb(ui->cbRouteSort);

    ui->chkDisplayInBrowser->setChecked(config->bRunInBrowser);
    connect(ui->chkDisplayInBrowser, &QCheckBox::stateChanged,this,[=](int state){
        sDisplayInBrowser = (Qt::CheckState)state;
        bDirty = true;
        switch(sDisplayInBrowser)
        {
        case Qt::CheckState::Checked:
            ui->lblError->setText(tr("will run in browser after restart"));
            break;
        case Qt::CheckState::Unchecked:
            ui->lblError->setText(tr("will run in app after restart"));
            break;
        case Qt::CheckState::PartiallyChecked:
            break;
        }
        ui->lblError->setStyleSheet("color: green");

    });

    connect(ui->edTileServer, &QLineEdit::editingFinished, this,[=]{
        QString  txt = ui->edTileServer->text().trimmed();
        QUrl* url = new QUrl(txt);
        check_all();
        bDirty = true;
        m_dataCtrl = new FileDownloader(url->toString(), this);
        connect (m_dataCtrl, &FileDownloader::downloaded,this,[=](QString err){
            ui->lblError->setText(err);
            if(!err.isEmpty())
                ui->lblError->setStyleSheet("color: red");
        });
    });

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this,[=](QAbstractButton* button){
        if(!check_all())
            return;
        QString txt = button->text();
        if(ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok)
        {
            if(tileServerUrl->isValid())
            {
                config->tileServerUrl = tileServerUrl->toString();
            }
            switch(sDisplayInBrowser)
            {
            case Qt::CheckState::Checked:
                config->bRunInBrowser = true;
                break;
            case Qt::CheckState::Unchecked:
                config->bRunInBrowser = false;
                break;
            case Qt::CheckState::PartiallyChecked:
                break;
        }
        //connect(cbSort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSortSelectionChanged(int)));
            config->currCity->routeSortType = ui->cbRouteSort->currentIndex();

            accept();
            return;
        }
        else if(ui->buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
        {
            reject(); // cancel
        }
        else if(ui->buttonBox->standardButton(button) == QDialogButtonBox::Save)
        {
            if(bDirty)
                config->saveSettings();
            bDirty = false;
        }
        else
            return;
    });
}

DialogPreferences::~DialogPreferences()
{
    delete ui;
}


bool DialogPreferences::checkUrl()
{
    tileServerUrl = new QUrl(ui->edTileServer->text().trimmed());
    if(tileServerUrl->isValid())
    {
        return true;
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    return false;
}

bool DialogPreferences::check_all()
{
    if(!checkUrl())
        return false;

    // enable Save and Ok buttons
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    return true;
}
