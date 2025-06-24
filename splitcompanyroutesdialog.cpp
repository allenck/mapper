#include "splitcompanyroutesdialog.h"
#include "ui_splitcompanyroutesdialog.h"
#include "companyview.h"
#include <QApplication>

SplitCompanyRoutesDialog::SplitCompanyRoutesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitCompanyRoutesDialog)
{
 ui->setupUi(this);
 sql = SQL::instance();
 connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
 connect(ui->btnApply, SIGNAL(clicked(bool)), this, SLOT(btnApply_clicked()));
 ui->lblHelp->setText("");
 ui->lblHelp->setStyleSheet("color: red");
 connect(CompanyView::instance()->model(), SIGNAL(companyChange()), this, SLOT(fillCompanies()));
 fillCompanies();
 connect(ui->cbCompany1, &QComboBox::currentTextChanged, sql,[=](QString){
     cd1 = sql->getCompany(ui->cbCompany1->currentData().toInt());
     ui->dateEdit->setDate(cd1->endDate);
     segmentList = sql->getRouteSegmentsForDate(ui->dateEdit->date(), ui->cbCompany1->currentData().toInt());
     checkParameters();
 });
 connect(ui->cbCompany2, &QComboBox::currentTextChanged, sql, [=](QString){
     cd2 = sql->getCompany(ui->cbCompany2->currentData().toInt());
     ui->newEndDate->setDate(cd2->endDate);
     checkParameters();
 });
}

SplitCompanyRoutesDialog::~SplitCompanyRoutesDialog()
{
 delete ui;
}

void SplitCompanyRoutesDialog::fillCompanies()
{
    ui->cbCompany1->clear();
    ui->cbCompany2->clear();

    //cbCompany.Text = " ";
    _companyList = sql->getCompanies();
    if (_companyList.count() == 0)
        return;
    //int count = 0;
    //foreach (companyData cd in _companyList)
    for(int i=0; i < _companyList.count(); i++)
    {
        CompanyData* cd = (CompanyData*)_companyList.at(i);
        ui->cbCompany1->addItem(cd->toString(), cd->companyKey);
        ui->cbCompany2->addItem(cd->toString(), cd->companyKey);

        //count++;
    }
}

void SplitCompanyRoutesDialog::setOrginalCompany(int companyKey)
{
    cd1 = sql->getCompany(ui->cbCompany1->currentData().toInt());
    ui->cbCompany1->setCurrentIndex(ui->cbCompany1->findData(companyKey));
    ui->dateEdit->setDate(cd1->endDate);
    segmentList = sql->getRouteSegmentsForDate(ui->dateEdit->date(), ui->cbCompany1->currentData().toInt());

}

bool SplitCompanyRoutesDialog::checkParameters()
{
    cd1 = sql->getCompany(ui->cbCompany1->currentData().toInt());

    ui->lblHelp->setText("");
    if(ui->cbCompany1->currentIndex() < 0)
    {
        ui->lblHelp->setText(tr("select the original company."));
        return false;
    }
    if(ui->cbCompany2->currentIndex() < 0)
    {
        ui->lblHelp->setText(tr("select the target company."));
        return false;
    }

    if(ui->cbCompany1->currentIndex() == ui->cbCompany2->currentIndex())
    {
        ui->lblHelp->setText(tr("select a different target company."));
        return false;
    }
    cd2 = sql->getCompany(ui->cbCompany2->currentData().toInt());
    if(ui->newEndDate->date() > cd2->endDate  || ui->newEndDate->date() < ui->dateEdit->date())
    {
        ui->lblHelp->setText(tr("select a new date valid for company!."));
        return false;

    }
    if(ui->dateEdit->date() < cd1->startDate  || ui->dateEdit->date() > cd1->endDate)
    {
        ui->lblHelp->setText(tr("date invalid for company1"));
        return false;
    }
    if(ui->newEndDate->date() < cd2->startDate  || ui->newEndDate->date() > cd2->endDate)
    {
        ui->lblHelp->setText(tr("date invalid for company2"));
        return false;
    }

    if(segmentList.isEmpty())
    {
        ui->lblHelp->setText(tr("No route segments for '%1'.").arg(ui->cbCompany1->currentText()));
        return false;
    }
    ui->btnApply->setEnabled(true);
    return true;
}

void SplitCompanyRoutesDialog::btnApply_clicked()
{
    if(!checkParameters())
        return;

    sql->beginTransaction("company split");
    QString curName;
    foreach (SegmentData* sd, segmentList)
    {
        qApp->processEvents();
        SegmentData sdOld = SegmentData(*sd);
        SegmentData sdNew = SegmentData(*sd);
        sdNew.setStartDate(ui->dateEdit->date());
        sdNew.setCompanyKey(ui->cbCompany2->currentData().toInt());
        sdNew.setEndDate(ui->newEndDate->date());
        if(sql->doesRouteSegmentExist(sdNew))
        {
            if(sd->routeName()!=curName)
            {
                ui->lblHelp->setStyleSheet("color: #FFFF00");
                ui->lblHelp->setText("bypassing: "+ sd->routeName());
                curName = sd->routeName();
            }

            continue;
        }
        if(sd->routeName() != curName)
        {
            ui->lblHelp->setStyleSheet("color: green");
            ui->lblHelp->setText("copying: "+ sd->routeName());
            curName = sd->routeName();
        }
        // if(sql->doesRouteSegmentExist(sdNew))
        // {
        //     qDebug() << "segment " << sdNew.segmentId() << " already present, route " << sdNew.route();
        //     continue;
        // }
        if(!sql->addSegmentToRoute(&sdNew, false))
        {
            ui->lblHelp->setStyleSheet("color: red");
            ui->lblHelp->setText("insert failed");
            sql->rollbackTransaction("company split");
            return;
        }
    }
    sql->commitTransaction("company split");
    accept();
}
