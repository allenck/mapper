#include "editstation.h"
#include "ui_editstation.h"
#include "editcomments.h"
#include "webviewbridge.h"
#include "stationview.h"
#include <QCompleter>

EditStation::EditStation(StationInfo sti, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editStation)
{
    ui->setupUi(this);
    _sti = sti;
    bDirty = false;
    config = Configuration::instance();
    sql = SQL::instance();
    _stationKey = sti.stationKey;
    _stationName = "";
    _infoKey = -1;
    ui->lblErrorText->setText("");
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_Click()));
    connect(ui->btnEditText, SIGNAL(clicked()), this, SLOT(btnEditText_Click()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
    connect(ui->txtLatitude, SIGNAL(editingFinished()),this, SLOT(txtLatLng_Leave()));
    connect(ui->txtLongitude, SIGNAL(editingFinished()),this, SLOT(txtLatLng_Leave()));
    connect(ui->txtStationName, SIGNAL(editingFinished()), this, SLOT(txtStationName_Leave()));
    connect(ui->txtStationName, SIGNAL(textEdited(QString)), this, SLOT(txtStationName_edited(QString)));
    this->setWindowTitle("Edit Station");

    ui->cbIcons->clear();
    ui->cbIcons->addItem(QIcon(":/darkred-dot.png"), "Default", "default");
    ui->cbIcons->addItem(QIcon(":/greenblank.png"), "Green bubble", "green");
    ui->cbIcons->addItem(QIcon(":/blueblank.png"), "Blue bubble", "blue");
    ui->cbIcons->addItem(QIcon(":/orange.png"), "Orange bubble", "orange");
    ui->cbIcons->addItem(QIcon(":/redblank.png"), "Red bubble", "red");
    ui->cbIcons->addItem(QIcon(":/white.png"), "White bubble", "white");
    ui->cbIcons->addItem(QIcon(":/yellowblank.png"), "Yellow bubble", "yellow");
    ui->cbIcons->addItem(QIcon(":/blue-red-blank.png"), "Blue & red bubble", "blue-red");
    ui->cbIcons->addItem(QIcon(":/tram.png"), "Tram icon", "tram");
    ui->cbIcons->addItem(QIcon(":/subway.png"), "Subway icon", "subway");
    ui->cbIcons->addItem(QIcon(":/S-Bahn-Logo.svg"), "U-Bahn", "sbahn");
    ui->cbIcons->addItem(QIcon(":/U-Bahn.svg"), "S-Bahn", "ubahn");
    ui->cbIcons->addItem(QIcon(":/Strassenbahn-Haltestelle.svg"), "Haltestelle", "haltestelle");
    ui->cbIcons->addItem(QIcon(":/sl-metro-logo.svg"),"StL Metro", "slmetro");
    int iconIndex = ui->cbIcons->findData(QVariant(sti.markerType));
    ui->cbIcons->setCurrentIndex(iconIndex);

    connect(ui->cbIcons, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbIcons_selectionChanged(int)));
    if(_stationKey > 0)
    {
     setStationId(_stationKey);
    }
    ui->txtStationName->setText(sti.stationName);
    _latLng = LatLng(sti.latitude,sti.longitude);
    ui->txtLatitude->setText( QString::number(sti.latitude,'g',8));
    ui->txtLongitude->setText(QString::number(sti.longitude,'g',8));
    ui->txtRoutes->setText(_sti.routes.join(","));
    ui->cbIcons->setCurrentIndex(ui->cbIcons->findData(markerType));
    ui->dateStart->setDate(sti.startDate);
    ui->dateEnd->setDate(sti.endDate);
}

EditStation::~EditStation()
{
    delete ui;
}

qint32 EditStation::segmentId()
{
   return _segmentId;
}

//void EditStation::setSegmentId(qint32 value)
//{
// _segmentId = value;
// sd = sql->getSegmentInfo(_segmentId);
// if (_latLng.isValid() && _pt != -1)
//     setRadioButtons();
// if(ui->dateStart->date() < sd.startDate())
// {
//  qDebug() <<"start date change from " << ui->dateStart->date().toString("yyyy/MM/dd") << " to " << sd.startDate();
//  ui->dateStart->setDate(sd.startDate());
//  bDirty = true;
// }
// if(ui->dateEnd->date() > sd.endDate())
// {
//  qDebug() <<"end date change from " << ui->dateEnd->date().toString("yyyy/MM/dd") << " to " << sd.endDate();
//  ui->dateEnd->setDate(sd.endDate());
//  bDirty = true;
// }
//}

qint32 EditStation::StationId()
{ return _stationKey; }

void EditStation::setMarkerType(QString markerType)
{
 this->markerType = markerType;
 ui->cbIcons->setCurrentIndex(ui->cbIcons->findData(markerType));
 this->markerType = markerType;
}

void EditStation::On_cbIcons_selectionChanged(int i)
{
    if(markerType != ui->cbIcons->currentData().toString())
 {
  qDebug() << "marker type change from " << markerType << " to " << ui->cbIcons->itemData(i).toString();
  bDirty = true;
 }
 markerType = ui->cbIcons->currentData().toString();
}

void EditStation::setStationId(qint32 value)
{
 _stationKey = value;
 StationInfo sti = sql->getStationInfo(_stationKey);
 setStationId(sti);

}
void EditStation::setStationId(StationInfo sti)
{
 if (sti.stationKey>=0)
 {
  ui->btnDelete->setEnabled(true);
  ui->btnEditText->setEnabled(true);
  _stationName = sti.stationName;
  ui->txtStationName->setText( _stationName);
  _infoKey = sti.infoKey;
  _latLng =  LatLng(sti.latitude, sti.longitude);
  if(sti.segmentId >= 0)
  {
   _segmentId = sti.segmentId;
   SegmentInfo sd = sql->getSegmentInfo(_segmentId);
   if(sd.segmentId() > 0)
   {
    if(ui->dateStart->date() < sd.startDate())
    {
     qDebug() <<"start date change from " << ui->dateStart->date().toString("yyyy/MM/dd") << " to " << sd.startDate();
     ui->dateStart->setDate(sd.startDate());
     //bDirty = true;
    }
    if(ui->dateEnd->date() > sd.endDate())
    {
     qDebug() <<"end date change from " << ui->dateEnd->date().toString("yyyy/MM/dd") << " to " << sd.endDate();
     ui->dateEnd->setDate(sd.endDate());
     //bDirty = true;
    }
   }
  }
//  _bGeodb_loc_id = sti.geodb_loc_id;
//  if(sti.geodb_loc_id > 0)
//      ui->txtGeodbLocId->setText(QString("%1").arg(sti.geodb_loc_id));
  ui->dateStart->setDate( sti.startDate);
  ui->dateEnd->setDate(sti.endDate);
//  QList<SegmentData> pointList = sql->getSegmentData(_segmentId);
//  if (pointList.isEmpty())
//      _pt = 0;
//  else
//  {
//   _pt = 0;
//   //foreach (segmentData sd in pointList)
//   for(int i=0; i < pointList.count(); i ++)
//   {
//    SegmentData sd = pointList.at(i);
//    if (_latLng ==  LatLng(sd.startLat, sd.startLon))
//    {
//     break;
//    }
//    _pt++;
//   }
//  }
  markerType = sti.markerType;
  ui->cbIcons->setCurrentIndex(ui->cbIcons->findData(sti.markerType));
  ui->cbIcons->setCurrentIndex(ui->cbIcons->findData(markerType));
 }
}
QString EditStation::StationName()
{
     return _stationName;
}
void EditStation::setStationName(QString value)
{
    _stationName = value;
}

//LatLng EditStation::Point()
//{
//     return _latLng;
//}
//void EditStation::setPoint(LatLng value)
//{
//    _latLng = value;
//    if (_segmentId != -1 && _pt !=-1)
//        setRadioButtons();
//}
//int EditStation::Index() { return _pt; }
//void EditStation::setIndex(qint32 value)
//{
//    _pt = value;
//    if (_latLng.isValid() && _segmentId != -1)
//        setRadioButtons();
//}

qint32 EditStation::infoKey (){ return _infoKey; }
bool EditStation::WasStationDeleted() {  return _bStationDeleted;  }
QDate EditStation::StartDate() {  return ui->dateStart->date(); }

void EditStation::setStartDate(QDate value)
{
    ui->dateStart->setDate(value);
    //qDebug()<< "editStation: start date -" + ui->dateStart->dateTime().toString();
}

QDate EditStation::EndDate() { return ui->dateEnd->date(); }
void EditStation::setEndDate(QDate value){
    ui->dateEnd->setDate(value);
    //qDebug()<< "editStation: end date -" + ui->dateEnd->dateTime().toString();
}

LatLng EditStation::latLng() {return _latLng;}

void EditStation::setLatLng(LatLng value)
{
   _latLng = value;
    ui->txtLatitude->setText( QString::number(_latLng.lat(),'g',8));
    ui->txtLongitude->setText(QString::number(_latLng.lon(),'g',8));
}


/// <summary>
/// Sets a reference to the current configuration
/// </summary>
//void editStation::setConfiguration (Configuration * cfg)
//{
//    config = cfg;
//    sql->setConfig(config);
//}
//void EditStation::setRadioButtons()
//{
//    StationInfo sti = StationInfo();
//    // Closest end check
//    if (sql->Distance(sd.startLat(), sd.startLon(), _latLng.lat(), _latLng.lon()) <
//            sql->Distance(sd.endLat(), sd.endLon(), _latLng.lat(), _latLng.lon()))
//    {
//     sti = sql->getStationAtPoint( LatLng(sd.startLat(), sd.startLon()));
//     if (sti.stationKey >=0)
//         ui->rbClosestEnd->setEnabled(false);
//    }
//    else
//    {
//     sti = sql->getStationAtPoint( LatLng(sd.endLat(), sd.endLon()));
//     if (sti.stationKey >= 0)
//         ui->rbClosestEnd->setEnabled(false);
//    }

//    // closest point check
//    sd = sql->getSegmentInfo(/*_pt,*/ _segmentId);
//    sti = sql->getStationAtPoint( sd.getStartLatLng());
//    if (sti.stationKey >= 0)
//        ui->rbClosestPoint->setEnabled(false);
//}

void EditStation::btnOK_Click()
{
 if (ui->txtStationName->text().length() == 0)
 {
  ui->lblErrorText->setText(tr("Station name cannot be blank!"));
  QApplication::beep();
  ui->txtStationName->setFocus();
  return;
 }
 _stationName = ui->txtStationName->text();
 _sti.stationName = ui->txtStationName->text();
 _sti.routes = ui->txtRoutes->text().split(",");
 if(_sti.routes.isEmpty())
 {
  ui->lblErrorText->setText(tr("Routes cannot be blank!"));
  QApplication::beep();
  ui->txtStationName->setFocus();
  return;
 }
 _sti.startDate = ui->dateStart->date();
 _sti.endDate = ui->dateEnd->date();
 if(_sti.endDate < _sti.startDate)
 {
  ui->lblErrorText->setText(tr("End date must br > start date!"));
  QApplication::beep();
  ui->txtStationName->setFocus();
  return;
 }
 _sti.latitude = ui->txtLatitude->text().toDouble();
 _sti.longitude = ui->txtLongitude->text().toDouble();
 _latLng = LatLng(_sti.latitude,_sti.longitude);
 if(!_latLng.isValid())
 {
  ui->lblErrorText->setText(tr("Latitude or longitude is invalid!"));
  QApplication::beep();
  ui->txtStationName->setFocus();
  return;
 }
 _sti.markerType = ui->cbIcons->currentData().toString();
 if(_sti.markerType.isEmpty())
 {
     ui->lblErrorText->setText(tr("Marker type must be selected!"));
     QApplication::beep();
     ui->cbIcons->setFocus();
     return;
 }

 QList<SegmentInfo> sList = sql->getIntersectingSegments(_sti.latitude, _sti.longitude,
                                                         .020, _sti.routeType);
 for(SegmentInfo si : sList)
 {
  QString sTxt = QString::number(si.segmentId());
  if(!_sti.segments.contains(sTxt))
   _sti.segments.append(sTxt);
 }

 if (_sti.stationKey > 0)
 {
  if(StationView::instance() != NULL)
   StationView::instance()->changeStation("chg", _sti);

  //sql->updateStation(_stationKey, _infoKey,_stationName, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"),markerType);
  sql->updateStation(_sti);
  QVariantList objArray;
  objArray << _stationKey << _sti.markerType;
  WebViewBridge::instance()->processScript("updateStationMarker", objArray);
 }
 else
 {
 //qint32 stationKey = sql->addStation(_stationName, _latLng, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"), 0, si.routeType, markerType);
  //qint32 stationKey = sql->addStation(_stationName, _latLng, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"),-1, -1,sd.routeType(),markerType, 0);
  qint32 stationKey= sql->addStation(_sti);
  if (stationKey >= 0)
  {
   StationView* _stationView = StationView::instance();
   if(_stationView != NULL)
   {
    _stationView->changeStation("add", _sti);
   }
   QVariantList objArray;
//                if (sti.infoKey > 0)
//                {
   CommentInfo ci = sql->getComments(_sti.infoKey);
   //str = ci.comments;
   //m_bridge->processScript("addStationMarker", QString("%1").arg(form.Point().lat(),0,'f',8)+","+QString("%1").arg(form.Point().lon(),0,'f',8)+","+(bDisplayStationMarkers?"true":"false")+","+QString("%1").arg(form.SegmentId())+",'"+form.StationName()+"',"+QString("%1").arg(stationKey)+","+QString("%1").arg(sti.infoKey)+",comments,'"+QString("%1").arg(markerType)+"'", "comments", ci.comments);
   objArray << _latLng.lat() << _latLng.lon()
            << QString("%1").arg(_latLng.lat(),0,'f',8)+","+QString("%1").arg(_latLng.lon(),0,'f',8)
               +","+(config->currCity->bDisplayStationMarkers?"true":"false") << _segmentId << _stationName
            << _stationKey << _sti.infoKey<<ci.comments << markerType;
   WebViewBridge::instance()->processScript("addStationMarker",objArray);
  }
 }
 // get other stations at this point and update segments field
 QList<StationInfo> stnList = sql->getStationAtPoint(LatLng(_sti.latitude, _sti.longitude));
 for(StationInfo sti : stnList)
 {
  if(_sti.stationKey == sti.stationKey)
   continue;
  bool bNeedsUpdate = false;
  for(QString sTxt :_sti.segments)
  {
   if(!sti.segments.contains(sTxt))
   {
    sti.segments.append(sTxt);
    bNeedsUpdate = true;
   }
   if(bNeedsUpdate)
   {
    sql->updateStation(sti);
   }
  }
 }
 bDirty = false;
 this->accept();
 this->close();

}

void EditStation::btnEditText_Click()
{

    EditComments commentsForm;
    commentsForm.setConfiguration( config);
    if (_infoKey != -1)
    {
        CommentInfo ci = sql->getComments(_infoKey);
        commentsForm.setHTMLText( ci.comments);
        commentsForm.setTags(ci.tags);
    }
    else
        commentsForm.setTags (ui->txtStationName->text());
    //connect(&commentsForm, SIGNAL(editComplete()), this, SLOT(commentsEditComplete()) );

    if (commentsForm.exec() == QDialog::Accepted)
    {
        if (_infoKey == -1)
        {
            _infoKey = sql->addComment(commentsForm.HTML(), commentsForm.Tags());
            sql->updateStation(_stationKey, _infoKey);
        }
        else
            sql->updateComment(_infoKey, commentsForm.HTML(), commentsForm.Tags());
    }

}

void EditStation::btnDelete_Click()
{
    _bStationDeleted =  sql->deleteStation(_stationKey);
    _stationKey = -1;
    if (_infoKey > -1)
        sql->deleteComment(_infoKey);
    this->accept();
    this->close();

}

void EditStation::txtStationName_Leave()
{
 if (ui->txtStationName->text().length() > 0)
  ui->btnEditText->setEnabled(true);
 else
  ui->btnEditText->setEnabled(false);
 //if(!bUpdateExisting)
 {
  StationInfo sti = sql->getStationInfo(ui->txtStationName->text().trimmed());
  if(sti.stationName == ui->txtStationName->text().trimmed())
  {
   // Station already exists
   _sti.segmentId = sti.segmentId;
   _sti.latitude = sti.latitude;
   ui->txtLatitude->setText(QString::number(sti.latitude,'g',8));
   _sti.longitude = sti.longitude;
   ui->txtLongitude->setText(QString::number(sti.longitude,'g',8));
   ui->txtRoutes->setText(_sti.routes.join(","));
   ui->cbIcons->setCurrentIndex(ui->cbIcons->findData(sti.markerType));
   sql->updateStation(sti);
   _stationKey = sti.stationKey;
   //ui->cbIcons->
   setStationId(sti.stationKey);
  }
 }
}
void EditStation::txtStationName_edited(QString txt)
{
 if(txt.length() > 2)
 {
  QList<StationInfo> list = sql->getStationsLikeName(txt);
  if(!list.isEmpty())
  {
   QStringList cl;
   foreach(StationInfo sti, list)
   {
    cl.append(sti.stationName);
   }
   ui->txtStationName->setCompleter(new QCompleter(cl));
  }
 }
 bDirty = true;
}

void EditStation::dateTimePicker2_ValueChanged()
{
 bDirty = true;
}

void EditStation::txtLatLng_Leave()
{
//    if (ui->txtLatLng->text().length() != 0 && ui->txtLatLng->text().length()!=10)
//    {
//        ui->lblErrorText->setText(tr("must be 0 or 10 bytes"));
//        ui->txtLatLng->setFocus();
//        return;
//    }
//    bool bOk = false;
//    _bGeodb_loc_id = ui->txtLatLng->text().toInt(&bOk, 10);
//    if(!bOk)
//    {
//        ui->lblErrorText->setText(tr( "must be numeric"));
//        ui->txtLatLng->setFocus();
//        return;
//    }
//    ui->lblErrorText->setText( "");
  bool bOk;
  double latitude = ui->txtLatitude->text().toDouble(&bOk);
  if(!bOk)
  {
      ui->lblErrorText->setText(tr( "latitude must be numeric"));
      ui->txtLatitude->setFocus();
      return;
  }
  double longitude = ui->txtLongitude->text().toDouble(&bOk);
  if(!bOk)
  {
      ui->lblErrorText->setText(tr( "longitude must be numeric"));
      ui->txtLongitude->setFocus();
      return;
  }
  _latLng = LatLng(latitude,longitude);
  if(!_latLng.isValid())
  {
      ui->lblErrorText->setText(tr( "invalid LatLng"));
      ui->txtLatitude->setFocus();
      return;
  }

  _sti.latitude = latitude;
  _sti.longitude = longitude;

  _sti.markerType = ui->cbIcons->currentData().toString();
  _sti.stationName = ui->txtStationName->text();
  if(_sti.stationKey < 0)
  {
   if(sql->addStation(_sti)< 0)
   {
    ui->lblErrorText->setText(tr( "error adding station"));
    return;
   }
   else
   {
    if(!sql->updateStation(_sti))
    {
     ui->lblErrorText->setText(tr( "error updating station"));
     return;
    }
   }
  }
 close();
}

void EditStation::closeEvent(QCloseEvent *event)
{
 if(bDirty)
 {
  if(QMessageBox::question(this, tr("Save changes?"), tr("Changes were made to this station. Do you wish to save them?"),QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
  {
   event->ignore();
   return;
  }
 }
 event->accept();
}
