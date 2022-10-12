#include "replacesegmentdialog.h"
#include "ui_replacesegmentdialog.h"
#include "sql.h"

bool compareSegmentInfo2(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description < s2.description;
}

ReplaceSegmentDialog::ReplaceSegmentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ReplaceSegmentDialog)
{
 ui->setupUi(this);

 connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(Process(QAbstractButton *)));
 connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
 connect(SQL::instance(), SIGNAL(details(QString)), this, SLOT(updateDetails(QString)));
 // test paramaters
// ui->oldSegments->setPlainText("196, 389,879,390");
// ui->newSegments->setPlainText("135,1304,1059");
 ui->lblHelp->setText("");
 connect(ui->oldSegments, &QPlainTextEdit::textChanged, [=]{
 });

 cbSegmentsGrp = new QButtonGroup(this);
 cbSegmentsGrp->addButton(ui->rbSingle);
 cbSegmentsGrp->addButton(ui->rbDouble);
 cbSegmentsGrp->addButton(ui->rbBoth);
 ui->rbBoth->setChecked(true);

 enterGrp = new QButtonGroup(this);
 enterGrp->addButton(ui->rbAdd);
 enterGrp->addButton(ui->rbDelete);
 ui->rbDelete->setChecked(true);
 refreshSegmentCB();

 connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
 connect(ui->rbSingle, SIGNAL(toggled(bool)), this, SLOT(refreshSegmentCB()));
 connect(ui->rbDouble, SIGNAL(toggled(bool)), this, SLOT(refreshSegmentCB()));
 connect(ui->rbBoth, SIGNAL(toggled(bool)), this, SLOT(refreshSegmentCB()));

}

ReplaceSegmentDialog::~ReplaceSegmentDialog()
{
 delete ui;
}

void ReplaceSegmentDialog::Process(QAbstractButton *button)
{
 ui->details->clear();
 try{
 if(!SQL::instance()->replaceSegmentsInRoutes(ui->oldSegments->toPlainText().split(","), ui->newSegments->toPlainText().split(",")))
 {
  rejected();
 }
 if(button->text() == tr("Save All"))
  accepted();
 }

 catch(IllegalArgumentException e)
 {

 }
 catch(RecordNotFoundException e)
 {
  ui->lblHelp->setText(e.getMessage());
 }
 catch(Exception e)
 {
  ui->lblHelp->setText(e.getMessage());

 }
}

void ReplaceSegmentDialog::refreshSegmentCB()
{
 bRefreshingSegments = true;
    ui->cbSegments->clear();
    cbSegmentInfoList = SQL::instance()->getSegmentInfo();
    qSort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfo2);
    //foreach (segmentInfo sI in cbSegmentInfoList)
    for(int i=0; i < cbSegmentInfoList.count(); i++)
    {
     SegmentInfo sI = cbSegmentInfoList.at(i);
     if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
        (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
        ui->rbBoth->isChecked())
      ui->cbSegments->addItem(sI.toString(), sI.segmentId);
    }
    bRefreshingSegments = false;
}

void ReplaceSegmentDialog::cbSegmentsSelectedValueChanged(qint32 index)
{
    if(bRefreshingSegments)
        return;
    SegmentInfo sI;
    int segmentId = ui->cbSegments->currentData().toInt();
    int row = -1;
    for(int i = 0; i < cbSegmentInfoList.count(); i++)
    {
     if(cbSegmentInfoList.at(i).segmentId == segmentId)
     {
      row = i;
      break;
     }
    }
    if(row < 0)
        return;
    if(row >= cbSegmentInfoList.count())
        return;
    sI = (SegmentInfo)cbSegmentInfoList.at(row);
    QString sSegment = QString("%1").arg(sI.segmentId);
    if(ui->rbDelete->isChecked())
    {
     if(!ui->oldSegments->toPlainText().isEmpty())
      ui->oldSegments->insertPlainText(",");
     ui->oldSegments->insertPlainText(sSegment);
    }
    else
    {
     if(!ui->newSegments->toPlainText().isEmpty())
      ui->newSegments->insertPlainText(",");
     ui->newSegments->insertPlainText(sSegment);

    }
}

void ReplaceSegmentDialog::updateDetails(QString txt)
{
 ui->details->append(txt);
}
