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

EditSegmentDialog::EditSegmentDialog(SegmentInfo sd, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
 segmentSelected(sd);

 //ui->ssw->initialize();
 //connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(SegmentData)));
}

EditSegmentDialog::EditSegmentDialog(SegmentData *sd, SegmentInfo si, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 this->sd = sd;
 this->si = si;
 common();
 segmentSelected(si);

// ui->ssw->initialize();
// connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(SegmentData)));
}

void EditSegmentDialog::common()
{
 ui->setupUi(this);
 sql = SQL::instance();

 _locations = sql->getLocations();
 ui->cbLocation->clear();
 ui->cbLocation->addItems(_locations);
// if(sd && sd->route() < 0)
// {
//  ui->chkOneWay->setVisible(false);
//  ui->trackUsage->setVisible(sd->tracks()==2);
// }
// else
// {
//  //ui->chkOneWay->setChecked(sd->oneWay() == "Y");
//  ui->chkOneWay->setDisabled(true);
// }
 this->config = Configuration::instance();
 b_cbSegments_TextChanged = false;
 bStartDateEdited = false;
 bEndDateEdited = false;
 m_bridge = WebViewBridge::instance();
 connect(m_bridge, SIGNAL(segmentStatusSignal(QString,QString)), this, SLOT(On_segmentStatusSignal(QString,QString)));

 QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbRouteType->addItems(routeTypes);
 ui->txtPoints->setText(QString::number(si.pointList().count()));
 ui->txtKm->setText(QString::number(si.length()));
 ui->txtMiles->setText(QString::number(si.length()* 0.621371192));
 ui->txtBearing->setText(QString("%1°").arg(si.bearing().getBearing()));
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
 ui->txtDirection->setText(si.direction());
 ui->chkOneWay->setVisible(sd);
 ui->usageLabel->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si.tracks()==2 );
 ui->trackUsage->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si.tracks()==2);


// ui->buttonBox->addButton(btnUpdate, QDialogButtonBox::ActionRole);
// connect(btnUpdate, SIGNAL(clicked()), this, SLOT(On_btnUpdate_clicked()));
 connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]{
  On_btnSave_clicked();
  accept();
 });
 btnVerifyDates = new QPushButton(tr("Verify dates"));
 ui->buttonBox->addButton(btnVerifyDates, QDialogButtonBox::ButtonRole::ApplyRole);
 connect(btnVerifyDates, &QPushButton::clicked, [=]{
  QPair<QDate,QDate> pair= sql->getStartAndEndDates(sd->segmentId());
  if(sd->segmentId() >=0)
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
 connect(ui->trackUsage, SIGNAL(currentIndexChanged()), this, SLOT(On_trackUsageChanged(int)));
}

EditSegmentDialog::~EditSegmentDialog()
{
 delete ui;
}

bool compareSegmentInfoByName1(const SegmentData & s1, const SegmentData & s2)
{
    return s1.description() < s2.description();
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

void EditSegmentDialog::segmentSelected(SegmentInfo si)
{
 if(si.segmentId()<0)
  return;
 this->si = si;
 //ui->ssw->initialize();

 SegmentInfo newSi = SegmentInfo(si);

 // btnUpdate->setEnabled(false);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
 bStartDateEdited = false;
 bEndDateEdited = false;

 ui->cbLocation->setCurrentText(si.location());
 connect(ui->cbLocation, &QComboBox::editTextChanged, [=]{
  if(!_locations.contains( ui->cbLocation->currentText()))
  {
   qDebug() << "location " << " added";
  }
 });
 ui->label_segmentId->setText(QString::number(si.segmentId()));
 ui->txtDescription->setText(si.description());
 ui->chkOneWay->setVisible(sd);
 if(sd)
 {
  if(rd->route > 0  && si.tracks()== 1)
  {
   ui->chkOneWay->setChecked(sd->oneWay() == "Y");
  }
  if(rd->route > 0  && sd->tracks()== 2)
  {
   if(sd->trackUsage() =="L")
    ui->trackUsage->setCurrentIndex(1); // left track
   else  if(sd->trackUsage() =="R")
    ui->trackUsage->setCurrentIndex(2); // right track
   else
    ui->trackUsage->setCurrentIndex(0); // both tracks
  }

  ui->usageLabel->setVisible(sd->tracks() ==2 && sd->oneWay()=="Y");
  ui->trackUsage->setVisible(sd->tracks() ==2 && sd->oneWay()=="Y");
 }
 ui->sbTracks->setValue(si.tracks());
 ui->cbRouteType->setCurrentIndex(si.routeType());
 ui->dtBegin->setDate(si.startDate());
 ui->dtEnd->setDate(si.endDate());
 ui->txtPoints->setText(QString::number(si.pointList().count()));
 ui->txtKm->setText(QString::number(si.length(),'g', 3));
 ui->txtMiles->setText(QString::number(si.length()*0.621371192,'g',3));
 ui->txtDirection->setText(si.direction());
 ui->txtBearing->setText(QString::number(si.bearing().getBearing(),'g',4));

// sql->getSegmentDates(si);
// if(sd.bNeedsUpdate)
//  btnUpdate->setEnabled(true);
 ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
 ui->txtRoutes->setText(QString::number(/*sd.routeCount*/sql->getCountOfRoutesUsingSegment(si.segmentId())));
}

void EditSegmentDialog::On_cbRouteType_currentIndexChanged(int i)
{
 si.setRouteType((RouteType)i);
}

void EditSegmentDialog::On_sbTracks_valueChanged(int v)
{
 si.setTracks(v);
 if(rd->route > 0 )
 {
  ui->chkOneWay->setChecked(v==1);
  ui->trackUsage->setVisible(v==2);
  ui->usageLabel->setVisible(v==2);
 }
}


void EditSegmentDialog::On_chkOneWay_toggled(bool b)
{
 if(!b)
 {
  //sd.setOneWay("N");
  sd->setOneWay("N");
 }
 else
 {
  //sd.setOneWay("Y");
  sd->setOneWay( "N");

 }
 sd->setNeedsUpdate(true);
}

void  EditSegmentDialog::On_trackUsageChanged(int i)
{
 switch(i)
 {
 case  1:
  sd->setTrackUsage("L");
  break;
 case 2:
  sd->setTrackUsage("R");
  break;
 default:
  sd->setTrackUsage(" ");
  break;
 }
 sd->setNeedsUpdate(true);
}

void EditSegmentDialog::On_txtDescription_editingFinished()
{
 QString txt = ui->txtDescription->text();
 sd->setDescription(txt);
 int ix = txt.indexOf(",");
 if(ix > 0)
  sd->setStreetName(txt.mid(0,ix));
 else
  sd->setStreetName("");
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
 if(dt > sd->startDate())
 {
  QList<SegmentData> list = sql->getRouteSegmentsForDate(sd->segmentId(), dt.toString("yyyy/MM/dd"));
  QString detail;
  foreach (SegmentData sd, list)
  {
   detail.append(sd.toString() +"\n");
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
   sd->setStartDate(dt);
   setUpdate();
  }
  else
   ui->dtBegin->setDate(sd->startDate());
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
 if(dt < sd->endDate())
 {
  QList<SegmentData> list = sql->getRouteSegmentsForDate(sd->segmentId(), dt.toString("yyyy/MM/dd"));
  QString detail;
  foreach (SegmentData sd, list)
  {
   detail.append(sd.toString() +"\n");
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
   sd->setEndDate(dt);
   setUpdate();
  }
  else
   ui->dtEnd->setDate(sd->endDate());
  msg.close();
 }
}

void EditSegmentDialog::setUpdate()
{
 if(!sd->needsUpdate())
 {
  sd->setNeedsUpdate(true);
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

 sd->setEndDate(ui->dtEnd->date());
 sd->setStartDate(ui->dtBegin->date());
 sd->setLocation(ui->cbLocation->currentText());

 sql->beginTransaction("updateSegment");
 QList<SegmentData> list = sql->getRouteSegmentsForDate(sd->segmentId(), sd->startDate().toString("yyyy/MM/dd"));
 foreach(SegmentData sd1, list)
 {
  if(sd1.startDate() < QDate::fromString(sd->startDate().toString("yyy/MM/dd")))
  {
   if(list.count() != sql->updateRouteDate(sd->segmentId(), sd->startDate().toString("yyyy/MM/dd"), sd->endDate().toString("yyyy/MM/dd")))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }

 list = sql->getRouteSegmentsForDate(sd->segmentId(), sd->endDate().toString("yyyy/MM/dd"));
 foreach(SegmentData sd1, list)
 {
  if(sd1.endDate() > sd->endDate())
  {
   if(list.count() != sql->updateRouteDate(sd->segmentId(), sd->startDate().toString("yyyy/MM/dd"),
                                           sd->endDate().toString("yyyy/MM/dd")))
   {
//    sql->RollbackTransaction("updateSegment");
//    return;
   }
  }
 }
 if(!sql->updateSegment(sd))
 {
  sql->rollbackTransaction("updateSegment");
  return;
 }
 if(sd->needsUpdate())
 {
  if(!sql->updateRoute(*osd, *sd))
  {
   sql->rollbackTransaction("updateSegment");
   return;
  }
 }
 sql->commitTransaction("updateSegment");
 sd->setNeedsUpdate(false);

// btnUpdate->setEnabled(false);
// ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

 m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(sd->segmentId()));
 while(!m_bridge->isResultReceived())
 {qApp->processEvents();}
 if (m_segmentStatus == "Y")
 {
 //displaySegment(sd.SegmentId, si.description, si.oneWay, m_segmentColor, true);
  sd->displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, "", true);
 }
 //ui->ssw->refresh();
}

void EditSegmentDialog::On_segmentStatusSignal(QString txt, QString color)
{
 m_segmentStatus = txt;
 m_segmentColor = color;
}
