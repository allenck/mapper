#include "editsegmentdialog.h"
#include "qpushbutton.h"
#include "ui_editsegmentdialog.h"
#include "configuration.h"
#include <sql.h>
#include <QMessageBox>
#include "webviewbridge.h"
#include "mainwindow.h"

EditSegmentDialog::EditSegmentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 common();
}

EditSegmentDialog::EditSegmentDialog(SegmentInfo si, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 this->si = si;
 common();
 segmentSelected(this->si);

 QList<SegmentData*> listOfSegments = SQL::instance()->getRouteSegmentsInOrder(rd->route(),
                                                                               rd->routeName(), rd->companyKey(), rd->endDate());
 foreach (SegmentData* sd, listOfSegments) {
  if(sd->segmentId() == si.segmentId())
  {
   _sd = sd;
   oneWay = sd->oneWay();
   direction = sd->direction();
  break;


  }
 }


 //ui->ssw->initialize();
 //connect(ui->ssw, SIGNAL(segmentSelected(SegmentData)), this, SLOT(segmentSelected(SegmentData)));
}

EditSegmentDialog::EditSegmentDialog(SegmentData *sd, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditSegmentDialog)
{
 this->si = SegmentInfo(*sd);
 common();
 //this->si = sql->getSegmentInfo(sd->segmentId());
 segmentSelected(si);

 _sd = sd;
 oneWay = sd->oneWay();
 direction = sd->direction();
}

void EditSegmentDialog::common()
{
 ui->setupUi(this);
 sql = SQL::instance();

 RouteData rd = MainWindow::instance()->routeList.at(MainWindow::instance()->ui->cbRoute->currentIndex());
 this->rd = new RouteData(rd);

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
 ui->txtPoints->setText(QString::number(si.pointList().count()));
 ui->txtKm->setText(QString::number(si.length()));
 ui->txtMiles->setText(QString::number(si.length()* 0.621371192));
 ui->txtBearing->setText(QString("%1°").arg(si.bearing().angle()));
 ui->btnSave->setEnabled(false);
 ui->txtDirection->setText(si.direction());
 ui->txtNewerName->setText(si.newerName());
 //ui->chkOneWay->setVisible(sd);
// ui->usageLabel->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si->tracks()==2 );
// ui->trackUsage->setVisible(sd != nullptr && sd->oneWay()==  "Y" && si->tracks()==2);

connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
connect(ui->btnSave, &QPushButton::clicked, [=]{
  On_btnSave_clicked();
  accept();
 });

 connect(ui->cbRouteType, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbRouteType_currentIndexChanged(int)));
 //connect(ui->chkOneWay, SIGNAL(toggled(bool)), this, SLOT(On_chkOneWay_toggled(bool)));
 connect(ui->txtDescription, SIGNAL(editingFinished()), this, SLOT(On_txtDescription_editingFinished()));
 connect(ui->sbTracks, SIGNAL(valueChanged(int)), this, SLOT(On_sbTracks_valueChanged(int)));
 connect(ui->dtBegin, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtBegin_dateChanged(QDate)));
 connect(ui->dtBegin, SIGNAL(editingFinished()), this, SLOT(On_dtBegin_editingFinished()));
 connect(ui->dtEnd, SIGNAL(dateChanged(QDate)),this, SLOT(On_dtEnd_dateChanged(QDate)));
 connect(ui->dtEnd, SIGNAL(editingFinished()), this, SLOT(On_dtEnd_editingFinished()));
 connect(ui->doubleTracked, SIGNAL(dateChanged(QDate)), this, SLOT(On_doubleTracked_dateChanged(QDate)));
 connect(ui->doubleTracked, SIGNAL(editingFinished()), this, SLOT(On_doubleTrackedDate_editingFinished()) );
 //connect(ui->trackUsage, SIGNAL(currentIndexChanged(int)), this, SLOT(On_trackUsageChanged(int)));

 connect(ui->btnRecalc, &QPushButton::clicked, [=]{
     bool rslt = sql->recalculateSegmentDates(&si);
     if(rslt && (si.startDate() < ui->dtBegin->date()))
     {
         if(ui->dtBegin->date() == ui->doubleTracked->date())
             ui->doubleTracked->setDate(si.startDate());
         ui->dtBegin->setDate(si.startDate());
         ui->dtEnd->setDate(si.endDate());
     }
 });

 oldestStartDate = si.endDate();
 latestEndDate = si.startDate();
 oldestDoubleTrackDate = si.endDate();

 if(si.tracks() == 1)
  ui->doubleTracked->setVisible(false);
 ui->doubleTracked->setDate(si.doubleDate());
 //ui->chkOneWay->setChecked(sd->oneWay()=="Y");

 QList<SegmentInfo> dups = sql->getSegmentsInSameDirection(si);
 foreach(SegmentInfo dup, dups)
 {
  dupSegments.append(dup);
  ui->listWidget->addItem(dup.toString());
  if(dup.segmentId()>0 && dup.routeType() == si.routeType())
  {
    QList<SegmentData*> routes = sql->getRouteSegmentsBySegment(dup.segmentId());
    foreach(SegmentData* sd, routes)
    {
      if(sd->startDate() < oldestStartDate)
       oldestStartDate = sd->startDate();
      if(sd->endDate() > latestEndDate)
       latestEndDate = dup.endDate();
      if(sd->tracks() ==2 && sd->startDate() < oldestDoubleTrackDate)
        oldestDoubleTrackDate = sd->startDate();
    }
  }
 }
 reversed = sql->getSegmentsInOppositeDirection(si);
 foreach(SegmentInfo dup, reversed)
 {
  if(dup.segmentId()>0 && dup.routeType() == si.routeType())
  {
   dupSegments.append(dup);
   ui->listWidget->addItem(dup.toString());
   if(dup.segmentId()>0 && dup.routeType() == si.routeType())
   {
     QList<SegmentData*> routes = sql->getRouteSegmentsBySegment(dup.segmentId());
     foreach(SegmentData* sd, routes)
     {
       if(sd->startDate() < oldestStartDate)
        oldestStartDate = sd->startDate();
       if(dup.endDate() > latestEndDate)
        latestEndDate = sd->endDate();
       if(sd->tracks() ==2 && sd->startDate() < oldestDoubleTrackDate)
         oldestDoubleTrackDate = sd->startDate();
     }
   }
  }
 }
 if(dupSegments.count() >0)
 {
  ui->lblHelp->setText(tr("Duplicate segments exist"));
  ui->lblHelp->setStyleSheet("color:#FFA500");
  si.setStartDate(oldestStartDate);
  si.setEndDate(latestEndDate);
  si.setDoubleDate(oldestDoubleTrackDate);
 }
 ui->dtBegin->setDate(si.startDate());
 ui->dtEnd->setDate(si.endDate());
}

EditSegmentDialog::~EditSegmentDialog()
{
 delete ui;
}

bool compareSegmentInfoByName1(const SegmentData & s1, const SegmentData & s2)
{
    return s1.description() < s2.description();
}

void EditSegmentDialog::segmentSelected(SegmentInfo si)
{
 if(si.segmentId()<0)
  return;
 ui->btnSave->setEnabled(false);
 bStartDateEdited = false;
 bEndDateEdited = false;

 ui->cbLocation->setCurrentText(si.location());
 connect(ui->cbLocation, &QComboBox::editTextChanged, [=]
 {
   if(!_locations.contains( ui->cbLocation->currentText()))
   {
    qDebug() << "location " << " added";
   }
 });
 if(si.segmentId() > 0)
 {
  ui->label_segmentId->setText(QString::number(si.segmentId()));
  ui->txtDescription->setText(si.description());
  //ui->chkOneWay->setVisible(sd);
 }
 ui->sbTracks->setValue(si.tracks());
 ui->cbRouteType->setCurrentIndex(si.routeType());
 ui->dtBegin->setDate(si.startDate());
 ui->doubleTracked->setDate(si.doubleDate());
 ui->dtEnd->setDate(si.endDate());
 ui->txtPoints->setText(QString::number(si.pointList().count()));
 ui->txtKm->setText(QString::number(si.length(),'g', 3));
 ui->txtMiles->setText(QString::number(si.length()*0.621371192,'g',3));
 ui->txtDirection->setText(si.direction());
 ui->txtBearing->setText(QString::number(si.bearing().angle(),'g',4));

// sql->getSegmentDates(si);
// if(sd.bNeedsUpdate)
//  btnUpdate->setEnabled(true);
 ui->btnSave->setEnabled(true);
 ui->txtRoutes->setText(QString::number(/*sd.routeCount*/sql->getCountOfRoutesUsingSegment(si.segmentId())));
}

void EditSegmentDialog::On_cbRouteType_currentIndexChanged(int i)
{
 si.setRouteType((RouteType)i);
}

void EditSegmentDialog::On_sbTracks_valueChanged(int v)
{
 ui->lblHelp->setText("");
 if(reversed.count() == 0)
 {
  //QMessageBox::warning(this, tr("Warning"), "There are no segments defined in the opposite direction");
  ui->lblHelp->setText("There are no segments defined in the opposite direction");
 }
 si.setTracks(v);
 ui->doubleTracked->setVisible(v==2);

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
  if(si.segmentId()==-1 && si1.segmentId() >0)
  {
   ui->lblHelp->setText(tr("description aready present for segment %1").arg(si1.segmentId()));
   return;
  }
  if(si.segmentId() > 0 && si1.segmentId()>0 && (si.segmentId()!= si1.segmentId()))
  {
   ui->lblHelp->setText(tr("description aready present for segment %1").arg(si1.segmentId()));
   return;
  }

  //sd->setDescription(txt);
  si.setDescription(txt);
  int ix = txt.indexOf(",");
  if(ix > 0)
   si.setStreetName(txt.mid(0,ix));
  else
   si.setStreetName("");
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
 if(dt > si.startDate())
 {
  QList<SegmentData> list = sql->getRouteDatasForDate(si.segmentId(), dt.toString("yyyy/MM/dd"));
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
   si.setStartDate(dt);
   setUpdate();
  }
  else
   ui->dtBegin->setDate(si.startDate());
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
 if(dt < si.endDate())
 {
   QList<SegmentData> list = sql->getRouteDatasForDate(si.segmentId(), dt.toString("yyyy/MM/dd"));
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
     si.setEndDate(dt);
     setUpdate();
   }
   else
     ui->dtEnd->setDate(si.endDate());
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
  if(ui->doubleTracked->date() < ui->dtBegin->date() || ui->doubleTracked->date() > ui->dtEnd->date())
  {
   ui->lblHelp->setText("Double Tracked Date must be after startDate or before end date!");
   return;
  }
  si.setDoubleDate(ui->doubleTracked->date());
  ui->btnSave->setEnabled(true);
 }
}


void EditSegmentDialog::setUpdate()
{
  ui->btnSave->setEnabled(true);
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
  if(si.segmentId() == -1)
  {
    ui->lblHelp->setText("invalid segmentId");
    return;
  }

  si.setEndDate(ui->dtEnd->date());
  si.setStartDate(ui->dtBegin->date());
  si.setLocation(ui->cbLocation->currentText());
  si.setDoubleDate(ui->doubleTracked->date());
  si.setRouteType((RouteType)ui->cbRouteType->currentIndex());
  si.setNewerName(ui->txtNewerName->text());
  if(si.tracks() == 2 && dupSegments.count() > 0)
  {
   QString infoText;
   foreach (SegmentInfo siDup, dupSegments)
   {
    if(this->si.segmentId() != siDup.segmentId() && siDup.tracks()==2)
    {
     // 2 track segment already exists!
     int ret = QMessageBox::question(nullptr, tr("Duplicate exists"), tr("A duplicate already exists. "
                                     "\nShould this route be changed to use it?"), QMessageBox::Yes | QMessageBox::No);
     if(ret == QMessageBox::Yes)
     {
      QString trackUsage = " ";
      if(_sd && _sd->tracks()==1 && _sd->oneWay()=="Y" && siDup.tracks()==2)
      {
       if(_sd->direction() == siDup.direction())
        trackUsage = "R";
       else
        trackUsage = "L";
      }
      QString txt = QString("update Routes set lineKey = %1, trackUsage = '%3' where lineKey = %2").arg(siDup.segmentId()).arg( this->si.segmentId()).arg(trackUsage);
      bool rslt = sql->executeCommand(txt);
      if(rslt)
      {
       if(!sql->deleteSegment(this->si.segmentId()))
        return;
      }
//      SegmentData sd = SegmentData(siDup);
//      sd.setRoute(rd->route());
//      sd.setRouteName(rd->routeName());
//      sd.setStartDate(rd->startDate());
//      sd.setEndDate(rd->endDate());
//      sd.displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, "", true);

     }
     break;
    }
    infoText.append(tr("Segment %1 %2\n").arg(siDup.segmentId()).arg(siDup.description()));
   }
   QMessageBox* box = new QMessageBox(QMessageBox::Question, tr("Duplicates"),
                                      tr("Duplicate segments exist. Replace in routes with this segment?"),
                                      QMessageBox::Cancel | QMessageBox::Yes | QMessageBox::No);
   box->setInformativeText(infoText);
   int ret = box->exec();
   switch (ret)
   {
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
  if(!sql->updateSegment(&si))
  {
   sql->rollbackTransaction("updateSegment");
   ui->lblHelp->setText(tr("update segment %1 failed").arg(si.segmentId()));
   return;
  }
  QString oneWay;
  QString trackUsage;
  QList<SegmentData*> routeList = sql->getRouteSegmentsBySegment(si.segmentId());
  foreach (SegmentData* sd, routeList)
  {
    SegmentData sd1 = SegmentData(*sd);
    sd1.setSegmentId(si.segmentId());
    sd1.setTracks(si.tracks());
    if(sd1.oneWay() == "Y" )
    {
     if(si.direction().at(0) == sd1.direction().at(0))
      sd1.setTrackUsage("R");
     else
      sd1.setTrackUsage("L");
    }
    if(!sql->updateRoute(*sd,sd1))
    {
     ui->lblHelp->setText(tr("update route failed"));
     // sql->rollbackTransaction("updateSegment");
     // return;
     continue;
    }

//    if(sd1.route()== rd->route() && sd1.routeName() == rd->routeName()
//       && sd1.startDate()== rd->startDate())
//    {
//     sd1.displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, sd1.trackUsage(), true);
//     bSegmentDisplayed = true;
//    }
   //}
  }
  if(bReplaceDups)
  {
    foreach (SegmentInfo siDup, dupSegments)
    {
 //     QString commandText = QString("update Routes set lineKey = %1 where lineKey = %2")
 //       .arg(toId).arg(fromId);
      QList<SegmentData*> routeList = sql->getRouteSegmentsBySegment(siDup.segmentId());
      foreach (SegmentData* sd, routeList)
      {
        SegmentData sd1 = SegmentData(*sd);
        sd1.setSegmentId(si.segmentId());
        sd1.setTracks(si.tracks());
        if(sd1.oneWay() == "Y" )
        {
         if(si.direction().at(0) == sd1.direction().at(0))
          sd1.setTrackUsage("R");
         else
          sd1.setTrackUsage("L");
        }
        if(!sql->deleteRoute(*sd))
        {
          sql->rollbackTransaction("updateSegment");
          ui->lblHelp->setText(tr("delete route %1 failed").arg(sd->route()));
          return;
        }
        if(sql->doesRouteSegmentExist(sd1))
            return;
        if(!sql->addSegmentToRoute(&sd1))
        {
          sql->rollbackTransaction("updateSegment");
          ui->lblHelp->setText(tr("add route %1 failed").arg(sd->route()));
          return;
        }
      }
       // TODO: delete fromId
      break; // only use the first dup!
    }
    if(ui->chkDeleteUnused)
    {
     foreach (SegmentInfo siDup, dupSegments) {
      if(sql->getRouteSegmentsBySegment(siDup.segmentId()).count() ==0)
      {
       if(!sql->deleteSegment(siDup.segmentId()))
       {
        sql->rollbackTransaction("updateSegment");
        ui->lblHelp->setText(tr("delete segment %1 failed").arg(siDup.segmentId()));
        return;
       }
      }
     }
    }
   }

   sql->commitTransaction("updateSegment");

   m_bridge->processScript("isSegmentDisplayed", QString("%1").arg(si.segmentId()));
   while(!m_bridge->isResultReceived())
   {qApp->processEvents();}
//   if (m_segmentStatus == "Y" && !bSegmentDisplayed)
//   {
//    SegmentData sd = SegmentData(si);
//     sd.displaySegment(ui->dtEnd->date().toString("yyyy/MM/dd"),m_segmentColor, "", true);
//   }
   MainWindow::instance()->On_displayRoute(*rd);
}

void EditSegmentDialog::On_segmentStatusSignal(QString txt, QString color)
{
 m_segmentStatus = txt;
 m_segmentColor = color;
}
#if 0
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
#endif
