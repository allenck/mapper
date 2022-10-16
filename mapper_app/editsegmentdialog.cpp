#include "editsegmentdialog.h"
#include "ui_editsegmentdialog.h"
#include "configuration.h"
#include <sql.h>
#include "mainwindow.h"
#include <QMessageBox>
#include "webviewbridge.h"

EditSegmentDialog::EditSegmentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
}

EditSegmentDialog::EditSegmentDialog(int segmentId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
 for(int i=0; i< segmentlist.count(); i++)
 {
  SegmentInfo sI = (SegmentInfo)segmentlist.at(i);

  if (sI.segmentId == segmentId)
  {
   //cbSegments.SelectedItem = sI;
   ui->cbSegments->setCurrentIndex(i);
   break;
  }
 }
}

void EditSegmentDialog::common()
{
 ui->setupUi(this);
 this->config = Configuration::instance();
 sql = SQL::instance();
 b_cbSegments_TextChanged = false;
 bStartDateEdited = false;
 bEndDateEdited = false;
 m_bridge = webViewBridge::instance();
 connect(m_bridge, SIGNAL(segmentStatusSignal(QString,QString)), this, SLOT(On_segmentStatusSignal(QString,QString)));

 QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbRouteType->addItems(routeTypes);

// btnUpdate = new QPushButton(tr("&Update"));
// btnUpdate->setEnabled(false);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
// ui->buttonBox->addButton(btnUpdate, QDialogButtonBox::ActionRole);
// connect(btnUpdate, SIGNAL(clicked()), this, SLOT(On_btnUpdate_clicked()));
 connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]{
  On_btnSave_clicked();
  accept();
 });
 btnVerifyDates = new QPushButton(tr("Verify dates"));
 ui->buttonBox->addButton(btnVerifyDates, QDialogButtonBox::ButtonRole::ApplyRole);
 connect(btnVerifyDates, &QPushButton::clicked, [=]{
  if(sql->updateSegmentDates(si))
  {
   ui->dtBegin->setDate(QDate::fromString(si->startDate, "yyyy/MM/dd"));
   ui->dtEnd->setDate(QDate::fromString(si->endDate, "yyyy/MM/dd"));
  }
 });

 connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbSegments_currentIndexChanged(int)));
 fillSegments();

 connect(ui->cbRouteType, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbRouteType_currentIndexChanged(int)));
 connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(On_chkOneWay_toggled(bool)));
 connect(ui->txtDescription, SIGNAL(editingFinished()), this, SLOT(On_txtDescription_editingFinished()));
 connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(On_sbTracks_valueChanged(int)));
 connect(ui->dtBegin, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtBegin_dateChanged(QDate)));
 connect(ui->dtBegin, SIGNAL(editingFinished()), this, SLOT(On_dtBegin_editingFinished()));
 connect(ui->dtEnd, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtEnd_dateChanged(QDate)));
 connect(ui->dtEnd, SIGNAL(editingFinished()), this, SLOT(On_dtEnd_editingFinished()));
 connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(On_cbSegmentsTextChanged(QString)));
 connect(ui->cbSegments,SIGNAL(signalFocusOut()), this, SLOT(On_cbSegments_Leave()));
 connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(On_buttonBox_accepted()));
}

EditSegmentDialog::~EditSegmentDialog()
{
 delete ui;
}

bool compareSegmentInfoByName1(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description < s2.description;
}

void EditSegmentDialog::fillSegments()
{
 segmentlist = sql->getSegmentInfo();
 ui->cbSegments->clear();
 qSort(segmentlist.begin(), segmentlist.end(),compareSegmentInfoByName1);
 foreach (SegmentInfo si, segmentlist)
 {
  ui->cbSegments->addItem(si.toString(), si.segmentId);
 }
}

void EditSegmentDialog::On_cbSegments_currentIndexChanged(int i)
{
 if(i<0)
  return;
 SegmentInfo newSi = segmentlist.at(i);
 this->si = new SegmentInfo(newSi);
 si->tracks = newSi.tracks;
 this->saveSi = new SegmentInfo(newSi);
// btnUpdate->setEnabled(false);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
 bStartDateEdited = false;
 bEndDateEdited = false;


 ui->txtDescription->setText(si->description);
 ui->chkOneWay->setChecked(si->oneWay == "Y");
 ui->sbTracks->setValue(si->tracks);
 ui->cbRouteType->setCurrentIndex(si->routeType);
 ui->dtBegin->setDate(QDate::fromString(si->startDate, "yyyy/MM/dd"));
 ui->dtEnd->setDate(QDate::fromString(si->endDate, "yyyy/MM/dd"));
 ui->txtPoints->setText(QString::number(si->points));
 ui->txtKm->setText(QString::number(si->length,'g', 3));
 ui->txtMiles->setText(QString::number(si->length*0.621371192,'g',3));
 ui->txtDirection->setText(si->direction);
 ui->txtBearing->setText(QString::number(si->bearing.getBearing(),'g',4));

// sql->getSegmentDates(si);
// if(si->bNeedsUpdate)
//  btnUpdate->setEnabled(true);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
 ui->txtRoutes->setText(QString::number(si->routeCount));
}

void EditSegmentDialog::On_cbRouteType_currentIndexChanged(int i)
{
 si->routeType = (RouteType)i;
}

void EditSegmentDialog::On_sbTracks_valueChanged(int v)
{
 si->tracks = v;
}

void EditSegmentDialog::On_chkOneWay_toggled(bool b)
{
 if(!b)
 {
  si->oneWay = "N";
 }
 else
 {
  si->oneWay = "Y";
  //On_sbTracks_valueChanged(2);
 }
 setUpdate();
}

void EditSegmentDialog::On_txtDescription_editingFinished()
{
 QString txt = ui->txtDescription->text();
 si->description = txt;
 int ix = txt.indexOf(",");
 if(ix > 0)
  si->streetName = txt.mid(0,ix);
 else
  si->streetName = "";
 setUpdate();
}

void EditSegmentDialog::On_dtBegin_dateChanged(QDate dt)
{
 bStartDateEdited = true;
}

void EditSegmentDialog::On_dtBegin_editingFinished()
{
 QDate dt = ui->dtBegin->date();
 if(dt > ui->dtEnd->date())
 {
  QMessageBox::critical(this, tr("date error"), "The Begin date must be before the end date");
  return;
 }
 if(dt > QDate::fromString(si->startDate, "yyyy/MM/dd"))
 {
  QList<RouteData> list = sql->getRouteSegmentsForDate(si->segmentId, dt.toString("yyyy/MM/dd"));
  QString detail;
  foreach (RouteData rd, list)
  {
   detail.append(rd.toString() +"\n");
  }
  QMessageBox msg;
  msg.setWindowTitle(tr("Date Warning"));
  msg.setText(tr("%1 routes are using this segment on this date.").arg(list.count()));
  msg.setStandardButtons( QMessageBox::Yes| QMessageBox::No);
  msg.setInformativeText("Click on 'Yes' to alter all the routes using this segment. Enter 'No'' to cancel");
  msg.setIcon(QMessageBox::Warning);
  msg.setDetailedText(detail);
  if(msg.exec() == QMessageBox::Yes)
  {
   si->startDate = dt.toString("yyyy/MM/dd");
   setUpdate();
  }
  else
   ui->dtBegin->setDate(QDate::fromString(saveSi->startDate, "yyyy/mm/dd"));
  msg.close();
 }
}

void EditSegmentDialog::On_dtEnd_dateChanged(QDate dt)
{
 bEndDateEdited = true;
}

void EditSegmentDialog::On_dtEnd_editingFinished()
{
 QDate dt = ui->dtEnd->date();

 if(dt < ui->dtBegin->date())
 {
  QMessageBox::critical(this, tr("date error"), "The Begin date must be before the end date");
  return;
 }
 if(dt < QDate::fromString(si->endDate, "yyyy/MM/dd"))
 {
  QList<RouteData> list = sql->getRouteSegmentsForDate(si->segmentId, dt.toString("yyyy/MM/dd"));
  QString detail;
  foreach (RouteData rd, list)
  {
   detail.append(rd.toString() +"\n");
  }
  QMessageBox msg;
  msg.setWindowTitle(tr("Date Warning"));
  msg.setText(tr("%1 routes are using this segment on this date.").arg(list.count()));
  msg.setStandardButtons( QMessageBox::Yes| QMessageBox::No);
  msg.setInformativeText("Click on 'Yes' to alter all the routes using this segment. Enter 'No'' to cancel");
  msg.setIcon(QMessageBox::Warning);
  msg.setDetailedText(detail);
  msg.exec();
  if(msg.exec() == QMessageBox::Yes)
  {
   si->endDate = dt.toString("yyyy/MM/dd");
   setUpdate();
  }
  else
   ui->dtEnd->setDate(QDate::fromString(saveSi->endDate, "yyyy/mm/dd"));
  msg.close();
 }
}

void EditSegmentDialog::setUpdate()
{
 if(!si->bNeedsUpdate)
 {
  si->bNeedsUpdate = true;
//  btnUpdate->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
 }
}

void EditSegmentDialog::On_cbSegmentsTextChanged(QString )
{
 b_cbSegments_TextChanged = true;
}

void EditSegmentDialog::On_cbSegments_Leave()
{
 if(b_cbSegments_TextChanged ==true)
 {
  qint32 segmentId = -1;
  QString text = ui->cbSegments->currentText();

  bool bOk=false;
  segmentId = text.toInt(&bOk, 10);

  if (bOk)
  {
   //foreach (segmentInfo sI in segmentInfoList)
   for(int i=0; i< segmentlist.count(); i++)
   {
    SegmentInfo sI = (SegmentInfo)segmentlist.at(i);

    if (sI.segmentId == segmentId)
    {
     //cbSegments.SelectedItem = sI;
     ui->cbSegments->setCurrentIndex(i);
     break;
    }
   }
  }
 }
 b_cbSegments_TextChanged =false;
}

void EditSegmentDialog::On_buttonBox_accepted()
{
 On_btnSave_clicked();
}

void EditSegmentDialog::On_btnSave_clicked()
{
 if(ui->dtBegin->date() > ui->dtEnd->date())
 {
  return;
 }

 si->endDate = ui->dtEnd->date().toString("yyyy/MM/dd");
 si->startDate = ui->dtBegin->date().toString("yyyy/MM/dd");

 sql->BeginTransaction("updateSegment");
 QList<RouteData> list = sql->getRouteSegmentsForDate(si->segmentId, si->startDate);
 foreach(RouteData rd, list)
 {
  if(rd.startDate < QDate::fromString(si->startDate, "yyyy/MM/dd"))
  {
   if(list.count() != sql->updateRouteDate(si->segmentId, si->startDate, si->endDate))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }

 list = sql->getRouteSegmentsForDate(si->segmentId, si->endDate);
 foreach(RouteData rd, list)
 {
  if(rd.endDate > QDate::fromString(si->endDate, "yyyy/MM/dd"))
  {
   if(list.count() != sql->updateRouteDate(si->segmentId, si->startDate, si->endDate))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }
 if(!sql->updateSegment(si))
 {
  sql->RollbackTransaction("updateSegment");
  return;
 }
 sql->CommitTransaction("updateSegment");

// btnUpdate->setEnabled(false);
// ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

 m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(si->segmentId));
 while(!m_bridge->isResultReceived())
 {qApp->processEvents();}
 if (m_segmentStatus == "Y")
 {
 //displaySegment(si->SegmentId, si.description, si.oneWay, m_segmentColor, true);
  si->displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, si->trackUsage, true);
 }
 fillSegments();
}

void EditSegmentDialog::On_segmentStatusSignal(QString txt, QString color)
{
 m_segmentStatus = txt;
 m_segmentColor = color;
}
