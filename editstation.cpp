#include "editstation.h"
#include "ui_editstation.h"
#include "editcomments.h"
#include "webviewbridge.h"
#include "stationview.h"
#include <QCompleter>

EditStation::EditStation(qint32 stationKey, bool bDisplayStationMarkers, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editStation)
{
    ui->setupUi(this);
    bDirty = false;
    config = Configuration::instance();
    this->bDisplayStationMarkers = bDisplayStationMarkers;
    this->markerType = markerType;
    //sql->setConfig(config);
    sql = SQL::instance();
    _segmentId = -1;
    //_stationKey = -1;
    _lineSegmentId = -1;
    _stationKey = stationKey;
    _stationName = "";
    _latLng = LatLng();
    _pt = -1;
    _infoKey = -1;
    _bStationDeleted = false;
    bUpdateExisting = false;
    _bGeodb_loc_id = -1;
    ui->lblErrorText->setText("");
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_Click()));
    connect(ui->btnEditText, SIGNAL(clicked()), this, SLOT(btnEditText_Click()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
    connect(ui->txtGeodbLocId, SIGNAL(editingFinished()),this, SLOT(txtGeodbLocId_Leave()));
    connect(ui->txtStationName, SIGNAL(editingFinished()), this, SLOT(txtStationName_Leave()));
    connect(ui->txtStationName, SIGNAL(textEdited(QString)), this, SLOT(txtStationName_edited(QString)));
    this->setWindowTitle("Edit Station");
    ui->rbClosestPoint->setChecked(true);

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
    connect(ui->cbIcons, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbIcons_selectionChanged(int)));
    if(_stationKey > 0)
    {
     setStationId(stationKey);
     bUpdateExisting = true;

    }

}

EditStation::~EditStation()
{
    delete ui;
}
qint32 EditStation::SegmentId()
{
   return _segmentId;
}

void EditStation::setSegmentId(qint32 value)
{
 _segmentId = value;
 sd = sql->getSegmentInfo(_segmentId);
 if (_latLng.isValid() && _pt != -1)
     setRadioButtons();
 if(ui->dateStart->date() < sd.startDate())
 {
  qDebug() <<"start date change from " << ui->dateStart->date().toString("yyyy/MM/dd") << " to " << sd.startDate();
  ui->dateStart->setDate(sd.startDate());
  bDirty = true;
 }
 if(ui->dateEnd->date() > sd.endDate())
 {
  qDebug() <<"end date change from " << ui->dateEnd->date().toString("yyyy/MM/dd") << " to " << sd.endDate();
  ui->dateEnd->setDate(sd.endDate());
  bDirty = true;
 }
}

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
 if(markerType != ui->cbIcons->itemData(i).toString())
 {
  qDebug() << "marker type change from " << markerType << " to " << ui->cbIcons->itemData(i).toString();
  bDirty = true;
 }
 markerType = ui->cbIcons->itemData(i).toString();
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
  ui->groupBox1->setEnabled(false);
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
  _bGeodb_loc_id = sti.geodb_loc_id;
  if(sti.geodb_loc_id > 0)
      ui->txtGeodbLocId->setText(QString("%1").arg(sti.geodb_loc_id));
  ui->dateStart->setDateTime( sti.startDate);
  ui->dateEnd->setDateTime(sti.endDate);
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
  bUpdateExisting = true;

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

LatLng EditStation::Point()
{
     return _latLng;
}
void EditStation::setPoint(LatLng value)
{
    _latLng = value;
    if (_segmentId != -1 && _pt !=-1)
        setRadioButtons();
}
int EditStation::Index() { return _pt; }
void EditStation::setIndex(qint32 value)
{
    _pt = value;
    if (_latLng.isValid() && _segmentId != -1)
        setRadioButtons();
}

qint32 EditStation::infoKey (){ return _infoKey; }
bool EditStation::WasStationDeleted() {  return _bStationDeleted;  }
int EditStation::LineSegmentId() {  return _lineSegmentId; }
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
qint32 EditStation::Geodb_Loc_Id() {return _bGeodb_loc_id;}
void EditStation::setGeodb_Loc_Id(qint32 value)
{
   _bGeodb_loc_id = value;
    ui->txtGeodbLocId->setText( QString("%1").arg(_bGeodb_loc_id));
}


/// <summary>
/// Sets a reference to the current configuration
/// </summary>
//void editStation::setConfiguration (Configuration * cfg)
//{
//    config = cfg;
//    sql->setConfig(config);
//}
void EditStation::setRadioButtons()
{
    StationInfo sti = StationInfo();
    // Closest end check
    if (sql->Distance(sd.startLat(), sd.startLon(), _latLng.lat(), _latLng.lon()) <
            sql->Distance(sd.endLat(), sd.endLon(), _latLng.lat(), _latLng.lon()))
    {
     sti = sql->getStationAtPoint( LatLng(sd.startLat(), sd.startLon()));
     if (sti.stationKey >=0)
         ui->rbClosestEnd->setEnabled(false);
    }
    else
    {
     sti = sql->getStationAtPoint( LatLng(sd.endLat(), sd.endLon()));
     if (sti.stationKey >= 0)
         ui->rbClosestEnd->setEnabled(false);
    }

    // closest point check
    sd = sql->getSegmentInfo(/*_pt,*/ _segmentId);
    sti = sql->getStationAtPoint( sd.getStartLatLng());
    if (sti.stationKey >= 0)
        ui->rbClosestPoint->setEnabled(false);
}

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

 StationInfo sti = sql->getStationAtPoint(_latLng);

 if(ui->rbAddPoint->isChecked())
 {
  //sql->insertPoint((qint32)_pt, _segmentId, _latLng.lat(), _latLng.lon(), & _lineSegmentId);
  SegmentInfo sd = sql->getSegmentInfo(_segmentId);
  sd.insertPoint(_pt, _latLng);
  sql->updateSegment(&sd);
 }
 else if (ui->groupBox1->isEnabled())
 {
  if (ui->rbClosestEnd->isChecked())
  {
   if (sql->Distance(sd.startLat(), sd.startLon(), _latLng.lat(), _latLng.lon()) <
           sql->Distance(sd.endLat(), sd.endLon(), _latLng.lat(), _latLng.lon()))
   {
//    _latLng = LatLng(si.startLat, si.startLon);
//    sd = sql->getSegmentData(0, _segmentId);
//    _lineSegmentId = sd.key;
//   }
//   else
//   {
//    _latLng =  LatLng(si.endLat, si.endLon);
//    sd = sql->getSegmentData(si.lineSegments - 1, _segmentId);
//    _lineSegmentId = sd.key;
    sd = sql->getSegmentInfo(_segmentId);
   }
  }
  else if (ui->rbClosestPoint->isChecked())
  {
   sd = sql->getSegmentInfo(/*_pt,*/ _segmentId);
   //_latLng = new LatLng(sd.startLat, sd.startLon);
   //sd = sql->getSegmentData(_pt, _segmentId);
   _lineSegmentId = sd.segmentId();
   if (sql->Distance(sd.startLat(), sd.startLon(), _latLng.lat(), _latLng.lon()) <
           sql->Distance(sd.endLat(), sd.endLon(), _latLng.lat(), _latLng.lon()))
       _latLng =  sd.getStartLatLng();
   else
       _latLng =  sd.getEndLatLng();
  }
  else if(!ui->rbAddPoint->isChecked())
  {
   ui->lblErrorText->setText(tr("Radio button not set"));
   QApplication::beep();
   return;
  }
 }
 else
 {
 _stationName = ui->txtStationName->text();
 }
 if (bUpdateExisting)
 {
  StationInfo   sti = sql->getStationInfo(_stationKey);
  if(StationView::instance() != NULL)
   StationView::instance()->changeStation("chg", sti);

  sql->updateStation(_stationKey, _infoKey,_stationName, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"),markerType);
  QVariantList objArray;
  objArray << _stationKey << markerType;
  WebViewBridge::instance()->processScript("updateStationMarker", objArray);
 }
 else
 {
 //qint32 stationKey = sql->addStation(_stationName, _latLng, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"), 0, si.routeType, markerType);
  qint32 stationKey = sql->addStation(_stationName, _latLng, _segmentId, ui->dateStart->dateTime().toString("yyyy/MM/dd"),ui->dateEnd->dateTime().toString("yyyy/MM/dd"),-1, -1,sd.routeType(),markerType, 0);
 if (stationKey >= 0)
 {
   QString str = _stationName;
   StationInfo sti = sql->getStationInfo(stationKey);
   sti.segmentId = _segmentId;
   sti.route = 0;
   sti.alphaRoute= "";

   StationView* _stationView = StationView::instance();
   if(_stationView != NULL)
   {
    _stationView->changeStation("add", sti);
   }
   QVariantList objArray;
//                if (sti.infoKey > 0)
//                {
   CommentInfo ci = sql->getComments(sti.infoKey);
   //str = ci.comments;
   //m_bridge->processScript("addStationMarker", QString("%1").arg(form.Point().lat(),0,'f',8)+","+QString("%1").arg(form.Point().lon(),0,'f',8)+","+(bDisplayStationMarkers?"true":"false")+","+QString("%1").arg(form.SegmentId())+",'"+form.StationName()+"',"+QString("%1").arg(stationKey)+","+QString("%1").arg(sti.infoKey)+",comments,'"+QString("%1").arg(markerType)+"'", "comments", ci.comments);
   objArray << _latLng.lat() << _latLng.lon()
            << QString("%1").arg(_latLng.lat(),0,'f',8)+","+QString("%1").arg(_latLng.lon(),0,'f',8)
               +","+(bDisplayStationMarkers?"true":"false") << _segmentId << _stationName
            << _stationKey << sti.infoKey<<ci.comments << markerType;
   WebViewBridge::instance()->processScript("addStationMarker",objArray);
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
 if(!bUpdateExisting)
 {
  StationInfo sti = sql->getStationInfo(ui->txtStationName->text().trimmed());
  if(sti.stationName == ui->txtStationName->text().trimmed())
  {
   // Station already exists
   sti.segmentId = _segmentId;
   sti.latitude = _latLng.lat();
   sti.longitude = _latLng.lon();
   sql->updateStation(sti.stationKey, _latLng, _segmentId);
   _stationKey = sti.stationKey;
   //ui->cbIcons->
   setStationId(sti);
  }
  else
   bUpdateExisting = false;
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

void EditStation::txtGeodbLocId_Leave()
{
    if (ui->txtGeodbLocId->text().length() != 0 && ui->txtGeodbLocId->text().length()!=10)
    {
        ui->lblErrorText->setText(tr("must be 0 or 10 bytes"));
        ui->txtGeodbLocId->setFocus();
        return;
    }
    bool bOk = false;
    _bGeodb_loc_id = ui->txtGeodbLocId->text().toInt(&bOk, 10);
    if(!bOk)
    {
        ui->lblErrorText->setText(tr( "must be numeric"));
        ui->txtGeodbLocId->setFocus();
        return;
    }
    ui->lblErrorText->setText( "");
    bDirty = true;
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
