#include "splitcompanyroutesdialog.h"
#include "ui_splitcompanyroutesdialog.h"
#include "companyview.h"

SplitCompanyRoutesDialog::SplitCompanyRoutesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitCompanyRoutesDialog)
{
 ui->setupUi(this);
 sql = SQL::instance();
 connect(ui->cbCompany2, &QComboBox::currentIndexChanged, [=](int index){
  CompanyData* cd = sql->getCompany(ui->cbCompany2->currentData().toInt());
  ui->dateEdit->setDate(cd->startDate);
 });
 connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
 connect(ui->btnApply, SIGNAL(clicked(bool)), this, SLOT(btnApply_clicked()));
 ui->lblHelp->setText("");
 ui->lblHelp->setStyleSheet("color: red");
 connect(CompanyView::instance()->model(), SIGNAL(companyChange()), this, SLOT(fillCompanies()));
 fillCompanies();
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

void SplitCompanyRoutesDialog::btnApply_clicked()
{
 ui->lblHelp->setText("");
 QList<SegmentData*> segmentList = sql->getRoutSegmentsForDate(ui->dateEdit->date(),
                                                               ui->cbCompany1->currentData().toInt());
 if(segmentList.isEmpty())
 {
  ui->lblHelp->setText(tr("No route segments for '%1'.").arg(ui->cbCompany1->currentText()));
  return;
 }

 sql->beginTransaction("company split");
 foreach (SegmentData* sd, segmentList) {
  SegmentData sdOld = SegmentData(*sd);
  SegmentData sdNew = SegmentData(*sd);
  sdOld.setEndDate(ui->dateEdit->date().addDays(-1));
  if(!sql->updateSegment(&sdNew))
  {
   ui->lblHelp->setText("");
   sql->rollbackTransaction("company split");
   return;
  }
  sdNew.setStartDate(ui->dateEdit->date());
  sdNew.setCompanyKey(ui->cbCompany2->currentData().toInt());
 }
 sql->commitTransaction("company split");
 accepted();
}
