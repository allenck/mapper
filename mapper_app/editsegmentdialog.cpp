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

EditSegmentDialog::EditSegmentDialog(SegmentData sd, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
 segmentSelected(sd);

 ui->ssw->initialize();
 connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(segmentData)));
}

void EditSegmentDialog::common()
{
 ui->setupUi(this);
 this->config = Configuration::instance();
 sql = SQL::instance();
 b_cbSegments_TextChanged = false;
 bStartDateEdited = false;
 bEndDateEdited = false;
 m_bridge = WebViewBridge::instance();
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
  QPair<QDate,QDate> pair= sql->getStartAndEndDates(sd.segmentId());
  if(sd.segmentId() >=0)
  {
   ui->dtBegin->setDate(pair.first);
   ui->dtEnd->setDate(pair.second);
  }
 });

// connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(segmentSelected(int)));
// fillSegments();

 connect(ui->cbRouteType, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbRouteType_currentIndexChanged(int)));
 connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(On_chkOneWay_toggled(bool)));
 connect(ui->txtDescription, SIGNAL(editingFinished()), this, SLOT(On_txtDescription_editingFinished()));
 connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(On_sbTracks_valueChanged(int)));
 connect(ui->dtBegin, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtBegin_dateChanged(QDate)));
 connect(ui->dtBegin, SIGNAL(editingFinished()), this, SLOT(On_dtBegin_editingFinished()));
 connect(ui->dtEnd, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtEnd_dateChanged(QDate)));
 connect(ui->dtEnd, SIGNAL(editingFinished()), this, SLOT(On_dtEnd_editingFinished()));
// connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(On_cbSegmentsTextChanged(QString)));
// connect(ui->cbSegments,SIGNAL(signalFocusOut()), this, SLOT(On_cbSegments_Leave()));
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

//void EditSegmentDialog::fillSegments()
//{
// segmentlist = sql->getSegmentInfo();
// ui->cbSegments->clear();
// qSort(segmentlist.begin(), segmentlist.end(),compareSegmentInfoByName1);
// foreach (SegmentInfo si, segmentlist)
// {
//  ui->cbSegments->addItem(si.toString(), si.segmentId);
// }
//}

void EditSegmentDialog::segmentSelected(SegmentData sd)
{
 if(sd.segmentId()<0)
  return;
 this->sd = sd;
 SegmentData newSd = SegmentData(sd);

 // btnUpdate->setEnabled(false);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
 bStartDateEdited = false;
 bEndDateEdited = false;

 ui->label_segmentId->setText(QString::number(sd.segmentId()));
 ui->txtDescription->setText(sd.description());
 ui->chkOneWay->setChecked(sd.oneWay() == "Y");
 ui->sbTracks->setValue(sd.tracks());
 ui->cbRouteType->setCurrentIndex(sd.routeType());
 ui->dtBegin->setDate(sd.startDate());
 ui->dtEnd->setDate(sd.endDate());
 ui->txtPoints->setText(QString::number(sd.pointList().count()));
 ui->txtKm->setText(QString::number(sd.length(),'g', 3));
 ui->txtMiles->setText(QString::number(sd.length()*0.621371192,'g',3));
 ui->txtDirection->setText(sd.direction());
 ui->txtBearing->setText(QString::number(sd.bearing().getBearing(),'g',4));

// sql->getSegmentDates(si);
// if(sd.bNeedsUpdate)
//  btnUpdate->setEnabled(true);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
 ui->txtRoutes->setText(QString::number(/*sd.routeCount*/sql->getCountOfRoutesUsingSegment(sd.segmentId())));
}

void EditSegmentDialog::On_cbRouteType_currentIndexChanged(int i)
{
 sd.setRouteType((RouteType)i);
}

void EditSegmentDialog::On_sbTracks_valueChanged(int v)
{
 sd.setTracks(v);
}

void EditSegmentDialog::On_chkOneWay_toggled(bool b)
{
 if(!b)
 {
  sd.setOneWay("N");
 }
 else
 {
  sd.setOneWay("Y");
  //On_sbTracks_valueChanged(2);
 }
 setUpdate();
}

void EditSegmentDialog::On_txtDescription_editingFinished()
{
 QString txt = ui->txtDescription->text();
 sd.setDescription(txt);
 int ix = txt.indexOf(",");
 if(ix > 0)
  sd.setStreetName(txt.mid(0,ix));
 else
  sd.setStreetName("");
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
 if(dt > sd.startDate())
 {
  QList<RouteData> list = sql->getRouteSegmentsForDate(sd.segmentId(), dt.toString("yyyy/MM/dd"));
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
   sd.setStartDate(dt);
   setUpdate();
  }
  else
   ui->dtBegin->setDate(sd.startDate());
  msg.close();
 }
}

void EditSegmentDialog::On_dtEnd_dateChanged(QDate /*dt*/)
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
 if(dt < sd.endDate())
 {
  QList<RouteData> list = sql->getRouteSegmentsForDate(sd.segmentId(), dt.toString("yyyy/MM/dd"));
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
   sd.setEndDate(dt);
   setUpdate();
  }
  else
   ui->dtEnd->setDate(sd.endDate());
  msg.close();
 }
}

void EditSegmentDialog::setUpdate()
{
 if(!sd.needsUpdate())
 {
  sd.setNeedsUpdate(true);
//  btnUpdate->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
 }
}

//void EditSegmentDialog::On_cbSegmentsTextChanged(QString )
//{
// b_cbSegments_TextChanged = true;
//}

//void EditSegmentDialog::On_cbSegments_Leave()
//{
// if(b_cbSegments_TextChanged ==true)
// {
//  qint32 segmentId = -1;
//  QString text = ui->cbSegments->currentText();

//  bool bOk=false;
//  segmentId = text.toInt(&bOk, 10);

//  if (bOk)
//  {
//   //foreach (segmentInfo sI in segmentInfoList)
//   for(int i=0; i< segmentlist.count(); i++)
//   {
//    SegmentInfo sI = (SegmentInfo)segmentlist.at(i);

//    if (sI.segmentId == segmentId)
//    {
//     //cbSegments.SelectedItem = sI;
//     ui->cbSegments->setCurrentIndex(i);
//     break;
//    }
//   }
//  }
// }
// b_cbSegments_TextChanged =false;
//}

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

 sd.setEndDate(ui->dtEnd->date());
 sd.setStartDate(ui->dtBegin->date());

 sql->BeginTransaction("updateSegment");
 QList<RouteData> list = sql->getRouteSegmentsForDate(sd.segmentId(), sd.startDate().toString("yyyy/MM/dd"));
 foreach(RouteData rd, list)
 {
  if(rd.startDate < QDate::fromString(sd.startDate().toString("yyy/MM/dd")))
  {
   if(list.count() != sql->updateRouteDate(sd.segmentId(), sd.startDate().toString("yyyy/MM/dd"), sd.endDate().toString("yyyy/MM/dd")))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }

 list = sql->getRouteSegmentsForDate(sd.segmentId(), sd.endDate().toString("yyyy/MM/dd"));
 foreach(RouteData rd, list)
 {
  if(rd.endDate > sd.endDate())
  {
   if(list.count() != sql->updateRouteDate(sd.segmentId(), sd.startDate().toString("yyyy/MM/dd"), sd.endDate().toString("yyyy/MM/dd")))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }
 if(!sql->updateSegment(&sd))
 {
  sql->RollbackTransaction("updateSegment");
  return;
 }
 sql->CommitTransaction("updateSegment");

// btnUpdate->setEnabled(false);
// ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

 m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(sd.segmentId()));
 while(!m_bridge->isResultReceived())
 {qApp->processEvents();}
 if (m_segmentStatus == "Y")
 {
 //displaySegment(sd.SegmentId, si.description, si.oneWay, m_segmentColor, true);
  sd.displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, "", true);
 }
 ui->ssw->refresh();
}

void EditSegmentDialog::On_segmentStatusSignal(QString txt, QString color)
{
 m_segmentStatus = txt;
 m_segmentColor = color;
}
