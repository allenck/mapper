#include "editsegmentdialog.h"
#include "qpushbutton.h"
#include "ui_editsegmentdialog.h"
#include "configuration.h"
#include <sql.h>
#include <QMessageBox>
#include "webviewbridge.h"

EditSegmentDialog::EditSegmentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
}

EditSegmentDialog::EditSegmentDialog(SegmentInfo *si, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 this->si = si;
 common();
 segmentSelected(si);

 //ui->ssw->initialize();
 //connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(SegmentData)));
}

EditSegmentDialog::EditSegmentDialog(SegmentData *sd, SegmentInfo* si, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 this->sd = sd;
 this->osd = sd;
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
 if(sd)
 {
  if(!si)
   si = new SegmentInfo(*sd);
 }
 else
 {
  if(!si)
   si = new SegmentInfo();
  else
   sd = new SegmentData(*si);
 }

 ui->lblHelp->setText("");
 _locations = sql->getLocations();
 ui->cbLocation->clear();
 ui->cbLocation->addItems(_locations);
 this->config = Configuration::instance();
 b_cbSegments_TextChanged = false;
 bStartDateEdited = false;
 bEndDateEdited = false;
 m_bridge = WebViewBridge::instance();
 connect(m_bridge, SIGNAL(segmentStatusSignal(QString,QString)), this, SLOT(On_segmentStatusSignal(QString,QString)));

 QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbRouteType->addItems(routeTypes);
 ui->txtPoints->setText(QString::number(si->pointList().count()));
 ui->txtKm->setText(QString::number(si->length()));
 ui->txtMiles->setText(QString::number(si->length()* 0.621371192));
 ui->txtBearing->setText(QString("%1°").arg(si->bearing().angle()));
 ui->btnSave->setEnabled(false);
 ui->txtDirection->setText(si->direction());
 ui->chkOneWay->setVisible(sd);
 ui->usageLabel->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si->tracks()==2 );
 ui->trackUsage->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si->tracks()==2);

connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
connect(ui->btnSave, &QPushButton::clicked, [=]{
  On_btnSave_clicked();
  accept();
 });
 btnVerifyDates = new QPushButton(tr("Verify dates"));
 //ui->buttonBox->addButton(btnVerifyDates, QDialogButtonBox::ButtonRole::ApplyRole);
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
 connect(ui->doubleTracked, SIGNAL(dateChanged(QDate)), this, SLOT(On_doubleTracked_dateChanged(QDate)));
 connect(ui->doubleTracked, SIGNAL(editingFinished()), this, SLOT(On_doubleTrackedDate_editingFinished()) );
 connect(ui->trackUsage, SIGNAL(currentIndexChanged(int)), this, SLOT(On_trackUsageChanged(int)));

 if(sd)
 {
  if(!si)
   si = new SegmentInfo(*sd);
 }
 else
 {
  if(!si)
   si = new SegmentInfo();
  else
   sd = new SegmentData(*si);
 }
 if(si->tracks() == 1)
  ui->doubleTracked->setVisible(false);

 SegmentInfo dup = sql->getSegmentInSameDirection(*si);
 if(dup.segmentId()>0 && dup.routeType() == si->routeType())
  dupSegments.append(dup);
 dup = sql->getSegmentInOppositeDirection(*si);
 if(dup.segmentId()>0&& dup.routeType() == si->routeType())
  dupSegments.append(dup);
 if(dupSegments.count() >0)
 {
  ui->lblHelp->setText(tr("Duplicate segments exist"));
  ui->lblHelp->setStyleSheet("color:#FFA500");
 }
}

EditSegmentDialog::~EditSegmentDialog()
{
 delete ui;
}

bool compareSegmentInfoByName1(const SegmentData & s1, const SegmentData & s2)
{
    return s1.description() < s2.description();
}

void EditSegmentDialog::segmentSelected(SegmentInfo* si)
{
 if(si->segmentId()<0)
  return;
 this->si = si;
 //ui->ssw->initialize();

 //SegmentInfo newSi = SegmentInfo(si);

 // btnUpdate->setEnabled(false);
 ui->btnSave->setEnabled(false);
 bStartDateEdited = false;
 bEndDateEdited = false;

 ui->cbLocation->setCurrentText(si->location());
 connect(ui->cbLocation, &QComboBox::editTextChanged, [=]{
  if(!_locations.contains( ui->cbLocation->currentText()))
  {
   qDebug() << "location " << " added";
  }
 });
 if(si->segmentId() > 0)
 {
  ui->label_segmentId->setText(QString::number(si->segmentId()));
  ui->txtDescription->setText(si->description());
  //ui->chkOneWay->setVisible(sd);
 }
 if(sd)
 {
  if(rd == nullptr)
   rd = new RouteData(*sd);
  if(rd->route() > 0  && sd->tracks()== 1)
  {
   ui->chkOneWay->setChecked(sd->oneWay() == "Y");
  }
  if(rd->route() > 0  && sd->tracks()== 2)
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
 ui->sbTracks->setValue(si->tracks());
 ui->cbRouteType->setCurrentIndex(si->routeType());
 ui->dtBegin->setDate(si->startDate());
 ui->doubleTracked->setDate(si->doubleDate());
 ui->dtEnd->setDate(si->endDate());
 ui->txtPoints->setText(QString::number(si->pointList().count()));
 ui->txtKm->setText(QString::number(si->length(),'g', 3));
 ui->txtMiles->setText(QString::number(si->length()*0.621371192,'g',3));
 ui->txtDirection->setText(si->direction());
 ui->txtBearing->setText(QString::number(si->bearing().angle(),'g',4));

// sql->getSegmentDates(si);
// if(sd.bNeedsUpdate)
//  btnUpdate->setEnabled(true);
 ui->btnSave->setEnabled(true);
 ui->txtRoutes->setText(QString::number(/*sd.routeCount*/sql->getCountOfRoutesUsingSegment(si->segmentId())));
}

void EditSegmentDialog::On_cbRouteType_currentIndexChanged(int i)
{
 si->setRouteType((RouteType)i);
}

void EditSegmentDialog::On_sbTracks_valueChanged(int v)
{
 ui->lblHelp->setText("");
 si->setTracks(v);
 if(rd )
 {
  ui->chkOneWay->setChecked(v==1);
  ui->trackUsage->setVisible(v==2);
  ui->usageLabel->setVisible(v==2);
 }
 ui->doubleTracked->setVisible(v==2);

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
  if(txt.isEmpty())
  {
   ui->lblHelp->setText(tr("Description blank"));
   return;
  }
  SegmentInfo si1 = sql->getSegmentIdForDescription(txt);
  if(si->segmentId()==-1 && si1.segmentId() >0)
  {
   ui->lblHelp->setText(tr("description aready present for segment %1").arg(si1.segmentId()));
   return;
  }
  if(si->segmentId() > 0 && si1.segmentId()>0 && (si->segmentId()!= si1.segmentId()))
  {
   ui->lblHelp->setText(tr("description aready present for segment %1").arg(si1.segmentId()));
   return;
  }

  sd->setDescription(txt);
  si->setDescription(txt);
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
 ui->lblHelp->setText("");
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
  QList<SegmentData> list = sql->getRouteDatasForDate(sd->segmentId(), dt.toString("yyyy/MM/dd"));
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
 ui->lblHelp->setText("");
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
  QList<SegmentData> list = sql->getRouteDatasForDate(sd->segmentId(), dt.toString("yyyy/MM/dd"));
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

void EditSegmentDialog::On_doubleTracked_dateChanged(QDate dt)
{
 bDoubleTrackedDateEdited = true;
 ui->lblHelp->setText("");
}

void EditSegmentDialog::On_doubleTrackedDate_editingFinished()
{
 if(ui->doubleTracked->date().isValid())
 {
  if(ui->doubleTracked->date() < si->startDate() || ui->doubleTracked->date() > si->endDate())
  {
   ui->lblHelp->setText("Double Tracked Date must be after startDate or before end date!");
   return;
  }
  si->setDoubleDate(ui->doubleTracked->date());
  sd->setDoubleDate(ui->doubleTracked->date());
 }
}


void EditSegmentDialog::setUpdate()
{
 if(!sd->needsUpdate())
 {
  sd->setNeedsUpdate(true);
//  btnUpdate->setEnabled(true);
  ui->btnSave->setEnabled(true);
 }
}

void EditSegmentDialog::On_btnSave_clicked()
{
  ui->lblHelp->setText("");
  ui->lblHelp->setStyleSheet("color:red");

  if(ui->dtBegin->date() > ui->dtEnd->date())
  {
    ui->lblHelp->setText("End date must be after start date!");
    return;
  }
  if(si->segmentId() == -1)
    processAdd();

  si->setEndDate(ui->dtEnd->date());
  si->setStartDate(ui->dtBegin->date());
  si->setLocation(ui->cbLocation->currentText());

  sd->update(*si); // populate sd with shared fields

  if(si->tracks() == 2 && dupSegments.count() > 0)
  {
   QString infoText;
   foreach (SegmentInfo si, dupSegments) {
    infoText.append(tr("Segment %1 %2\n").arg(si.segmentId()).arg(si.description()));
   }
    QMessageBox* box = new QMessageBox(QMessageBox::Question, tr("Duplicates"),
                                       tr("Duplicate segments exist. Replace in routes with this segment?"),
                                       QMessageBox::Cancel | QMessageBox::Yes | QMessageBox::No);
    box->setInformativeText(infoText);
    int ret = box->exec();
    switch (ret) {
    case QMessageBox::Cancel:
     reject();
     break;
    case QMessageBox::Yes:
     bReplaceDups = true;
    default:
     break;
    }

  }

  sql->beginTransaction("updateSegment");
  if(!sql->updateSegment(si))
  {
   sql->rollbackTransaction("updateSegment");
   ui->lblHelp->setText(tr("update segment %1 failed").arg(si->segmentId()));
   return;
  }

//  if(osd && sd->needsUpdate())
//  {
//   if(!sql->updateRoute(*osd, *sd))
//   {
//    ui->lblHelp->setText(tr("update route failed"));
//    sql->rollbackTransaction("updateSegment");
//    return;
//   }
//  }
  if(bReplaceDups)
  {
   QDate oldestStartDate = si->startDate();
   foreach (SegmentInfo siDup, dupSegments)
   {
    if(siDup.startDate() < oldestStartDate)
     oldestStartDate = siDup.startDate();
   }
   foreach (SegmentInfo siDup, dupSegments) {
    if(!siDup.doubleDate().isValid() && ui->doubleTracked->date().isValid())
    {
     siDup.setDoubleDate(ui->doubleTracked->date());
     siDup.setTracks(2);
     if(!sql->updateSegment(&siDup))
     {
      sql->rollbackTransaction("updateSegment");
      ui->lblHelp->setText(tr("update segment %1 failed").arg(siDup.segmentId()));
      return;
     }

     // Keep the oldest segment, e.g the one with the lowest segment id.
     int fromId;
     int toId;
     if(si->segmentId()< siDup.segmentId())
     {
      toId = si->segmentId();
      fromId = siDup.segmentId();
     }
     else
     {
      toId = siDup.segmentId();
      fromId = si->segmentId();
     }
     QString commandText = QString("update Routes set lineKey = %1 where lineKey = %2")
       .arg(toId).arg(fromId);

     if(!sql->executeCommand(commandText))
     {
      {
       sql->rollbackTransaction("updateSegment");
       ui->lblHelp->setText(tr("update routes using segment %1 failed").arg(fromId));
       return;
      }
     }
     // adjust start date on oldest segment
     commandText = QString("update segments set startDate = '%1' where segmentid = %2")
       .arg(oldestStartDate.toString("yyyy/MM/dd")).arg(toId);
     if(!sql->executeCommand(commandText))
     {
      {
       sql->rollbackTransaction("updateSegment");
       ui->lblHelp->setText(tr("update  segment %1 failed").arg(toId));
       return;
      }
     }
     // TODO: delete fromId
    }
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
  si->displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, "", true);
 }
 //ui->ssw->refresh();
}

void EditSegmentDialog::On_segmentStatusSignal(QString txt, QString color)
{
 m_segmentStatus = txt;
 m_segmentColor = color;
}

void EditSegmentDialog::processAdd()
{
 QString txt = ui->txtDescription->text();
 if(txt.contains(","))
  si->setStreetName(txt.mid(0,txt.indexOf(",")));
 si->setDescription(txt);
 si->setLocation(ui->cbLocation->currentText());
 si->setTracks(ui->sbTracks->value());
 si->setEndDate(ui->dtEnd->date());
 si->setStartDate(ui->dtBegin->date());
 si->setRouteType((RouteType)ui->cbRouteType->currentIndex());
 sd = new SegmentData(*si);
 bool bAlreadyExists = false;
 int newSegment = sql->addSegment(*sd, &bAlreadyExists, false);
 if(bAlreadyExists)
 {
  ui->lblHelp->setText(tr("segment already exists"));
  return;
 }
 sql->updateSegmentDates(si);
 QVariantList objArray;
 objArray << newSegment;
 WebViewBridge::instance()->processScript("addModeOn", objArray);

}
