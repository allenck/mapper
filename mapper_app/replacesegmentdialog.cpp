#include "replacesegmentdialog.h"
#include "ui_replacesegmentdialog.h"
#include "segmentselectionwidget.h"

bool compareSegmentInfo2(const SegmentData & s1, const SegmentData & s2)
{
    return s1.description() < s2.description();
}

ReplaceSegmentDialog::ReplaceSegmentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ReplaceSegmentDialog)
{
 ui->setupUi(this);
 sql = SQL::instance();
 ui->ssw->initialize();

 connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(Process(QAbstractButton *)));
 connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
// ui->oldSegments->setPlainText("196, 389,879,390");
// ui->newSegments->setPlainText("135,1304,1059");
 ui->lblHelp->setText("");
 connect(ui->oldSegments, &QPlainTextEdit::textChanged, [=]{
 });

 enterGrp = new QButtonGroup(this);
 enterGrp->addButton(ui->rbAdd);
 enterGrp->addButton(ui->rbDelete);
 ui->rbDelete->setChecked(true);
 ui->ignoreDate->setDate(QDate::currentDate());
 ui->ssw->refresh();

 connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(SegmentData)));

 connect(sql, &SQL::details, [=](QString txt){
  ui->details->append(txt);
 });
}

ReplaceSegmentDialog::~ReplaceSegmentDialog()
{
 delete ui;
}

void ReplaceSegmentDialog::Process(QAbstractButton *button)
{
 ui->details->clear();
 try{
 if(!SQL::instance()->replaceSegmentsInRoutes(ui->oldSegments->toPlainText().split(","), ui->newSegments->toPlainText().split(","),ui->ignoreDate->date()))
 {
  reject();
 }
 if(button->text() == tr("Save All"))
  accept();
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
#if 0
void ReplaceSegmentDialog::refreshSegmentCB()
{
 QStringList tokens;
 QStringList tokens2;
 QStringList streets;
 QString description;

    bRefreshingSegments = true;
    ui->cbSegments->clear();
    QString selectedStreet = "";
    if(ui->cbStreets->currentIndex() >0)
     selectedStreet = ui->cbStreets->currentText().trimmed();
    ui->cbStreets->clear();
    ui->cbStreets->addItem("");
    cbSegmentInfoList = sql->getSegmentInfo();
    qSort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfo2);
    //foreach (segmentInfo sI in cbSegmentInfoList)
    for(int i=0; i < cbSegmentInfoList.count(); i++)
    {
     SegmentInfo sI = cbSegmentInfoList.at(i);
     description = sI.description;
     // populate streets
     tokens = description.split(",");
     if(tokens.count() > 1)
     {
      QString street = tokens.at(0).trimmed();
      if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
      //if(street.indexOf(" ")) street= street.mid(0, street.indexOf(" "));
      if(ui->cbStreets->findText(street)<0)
      {
       ui->cbStreets->addItem(street);
       streets.append(street);
      }
      if( selectedStreet == street)
      {
       if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
          (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
        ui->cbSegments->addItem(sI.toString(), sI.segmentId);
        continue;
      }
      else if(selectedStreet == "" )
      {
       if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
          (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
        ui->cbSegments->addItem(sI.toString(), sI.segmentId);
       continue;
      }
      tokens2 = tokens.at(1).split("to");
      {
       for(int i=0; i < tokens2.count(); i++)
       {
        QString street2 = tokens2.at(i);
        street2 = tokens.at(0).trimmed();
        if(street2.indexOf("(")) street2= street.mid(0, street2.indexOf("("));
        //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
        if(ui->cbStreets->findText(street2)<0)
        {
         ui->cbStreets->addItem(street2);
         streets.append(street2);
        }
        if(selectedStreet == street2)
        {
         if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
            (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
            ui->rbBoth->isChecked())
          ui->cbSegments->addItem(sI.toString(), sI.segmentId);
         continue;
        }
        else if(selectedStreet == "" )
        {
         if((sI.tracks == 2 && ui->rbDouble->isChecked() ) ||
            (sI.tracks == 1 && ui->rbSingle->isChecked() )  ||
            ui->rbBoth->isChecked())
          ui->cbSegments->addItem(sI.toString(), sI.segmentId);
         continue;
        }
       }
      }
     }
     else
     {
      tokens = sI.description.split(" ");
     }
    }
    ui->cbStreets->model()->sort(0);
    ui->cbStreets->setCurrentText(selectedStreet);
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
#endif
void ReplaceSegmentDialog::segmentSelected(SegmentData sd)
{
 QString sSegment = QString("%1").arg(sd.segmentId());
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
