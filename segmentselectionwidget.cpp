#include "segmentselectionwidget.h"
#include "qcompleter.h"
#include "segmentdescription.h"
#include "ui_segmentselectionwidget.h"
#include "webviewbridge.h"
#include "otherrouteview.h"
#include "clipboard.h"
#include "streetstablemodel.h"

SegmentSelectionWidget::SegmentSelectionWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SegmentSelectionWidget)
{
 ui->setupUi(this);

 cbSegmentsGrp = new QButtonGroup(this);
 cbSegmentsGrp->addButton(ui->rbSingle);
 cbSegmentsGrp->addButton(ui->rbDouble);
 cbSegmentsGrp->addButton(ui->rbBoth);
 ui->rbBoth->setChecked(true);
 sql = SQL::instance();
 QCompleter* completer = ui->cbStreets->completer();

}

void SegmentSelectionWidget::initialize()
{
 refreshLocations();

 refreshSegmentCB();

 connect(SQL::instance(), &SQL::segmentChanged, [=](const SegmentInfo si){
     if(ui->cbSegments->findData(si.segmentId()) >= 0)
  {
   //SegmentInfo si = sql->getSegmentInfo(segmentId);
         int  ix = ui->cbSegments->findData(si.segmentId());
   if(ui->cbSegments->itemText(ix) != si.toString())
    ui->cbSegments->setItemText(ix, si.toString());
  }
 });

 //connect(WebViewBridge::instance(), SIGNAL(segmentSelected(int,int)), this, SLOT(segmentSelected(int,int)));
 connect(WebViewBridge::instance(), SIGNAL(segmentSelectedX(int,int,QList<LatLng>)), this, SLOT(segmentSelected(int,int,QList<LatLng>)));
 connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
 connect(ui->cbSegments->lineEdit(), SIGNAL(editingFinished()), this, SLOT(cbSegments_editingFinished()));
 connect(ui->cbStreets->lineEdit(), &QLineEdit::editingFinished, this, [=]{
     QString text = ui->cbStreets->currentText();
     QString fullName = SegmentDescription::updateToken(text);
     if(text != fullName)
         ui->cbStreets->setCurrentText(fullName);
     refreshSegmentCB();
 });
 connect(ui->rbSingle, &QRadioButton::clicked, [=]{
  saveStreet = ui->cbStreets->currentText();
  refreshSegmentCB();
 });
 connect(ui->rbDouble, &QRadioButton::clicked, [=]{
  saveStreet = ui->cbStreets->currentText();
  refreshSegmentCB();
 });
 connect(ui->rbBoth, &QRadioButton::clicked, [=]{
  saveStreet = ui->cbStreets->currentText();
  refreshSegmentCB();
 });
 connect(ui->cbLocation, &QComboBox::currentTextChanged, [=]{
  if(!bCbStreetsRefreshing)
   refreshStreetsCb();
 });

 QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->cbStreets);
 proxy->setSourceModel(ui->cbStreets->model());
 // combo's current model must be reparented,
 // otherwise QComboBox::setModel() will delete it
 ui->cbStreets->model()->setParent(proxy);
 ui->cbStreets->setModel(proxy);

 Clipboard::instance()->setContextMenu(ui->cbStreets);


// QSortFilterProxyModel* proxy2 = new QSortFilterProxyModel(ui->cbSegments);
// proxy2->setSourceModel(ui->cbSegments->model());
// // combo's current model must be reparented,
// // otherwise QComboBox::setModel() will delete it
// ui->cbSegments->model()->setParent(proxy2);
// ui->cbSegments->setModel(proxy2);

 connect(ui->cbStreets, &QComboBox::editTextChanged, [=]{
  bCbStreets_text_changed = true;
 });
 //connect(ui->cbStreets, SIGNAL(editingFinished()), this, SLOT(cbStreets_editingFinished()));
 connect(ui->cbStreets, SIGNAL(currentIndexChanged(int)), this, SLOT(cbStreets_currentIndexChanged(int)));
 connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegments_currentIndexChanged(int)));

 refreshSegmentCB();
 initialized = true;
}

SegmentSelectionWidget::~SegmentSelectionWidget()
{
 delete ui;
}

bool compareSegmentDataByName(const SegmentData & s1, const SegmentData & s2)
{
 return s1.description() < s2.description();
}

void SegmentSelectionWidget::refreshLocations()
{
    try {
         _locations = sql->getLocations();
    }
    catch(SQLException ex)
    {
        _locations.clear();
    }

 ui->cbLocation->clear();
 ui->cbLocation->addItems(_locations);
}

void SegmentSelectionWidget::refreshSegmentCB()
{
 QStringList streets;
 QStringList tokens;
 QStringList tokens2;
 QString description;
 QString selectedStreet =ui->cbStreets->currentText();
 QList<SegmentInfo> segList;
 try{
  segList = sql->getSegmentsForStreet(selectedStreet,ui->cbLocation->currentText() );
 }
 catch(SQLException ex)
 {}

 QMutexLocker locker(&mutex);
 bRefreshingSegments = true;
 if(!bCbStreetsRefreshing)
  refreshStreetsCb();
 ui->cbSegments->clear();
 mapDescriptions.clear();
 cbSegmentInfoMap = sql->getSegmentInfoList(ui->cbLocation->currentText());
 if(!segList.isEmpty())
 {
     foreach(SegmentData si, segList)
     {
         if(!si.toString().isEmpty())
            ui->cbSegments->addItem(si.toString(), si.segmentId());
     }
     return;
 }
 foreach(SegmentData si, cbSegmentInfoMap.values())
 {
  description = si.description();
  if(!si.location().isEmpty())
  {
   if(!_locations.contains(si.location()))
    _locations.append(si.location());
  }
  tokens = description.split(",");
  if(tokens.count() > 1)
  {
   QString street = tokens.at(0).trimmed();
   int locStart = street.indexOf("(");
   if(locStart > 0) {
    int locEnd = street.indexOf(")");
    if(locEnd >0)
    {
     QString loc = street.mid(locStart+1, locEnd-locStart-1);
     if(!_locations.contains(loc))
      _locations.append(loc);
//     sd.setLocation(loc);
    }
    street= street.mid(0, street.indexOf("("));
   }
   QString searchLoc = ui->cbLocation->currentText();

   if((!searchLoc.isEmpty() && street == selectedStreet && ui->cbLocation->currentText() == si.location())
       || street == selectedStreet)
   {
    // populate streets
    if((si.tracks() == 2 && ui->rbDouble->isChecked() ) ||
       (si.tracks() == 1 && ui->rbSingle->isChecked() )  ||
       ui->rbBoth->isChecked())
    {
     if(!mapDescriptions.contains(si.toString()))
     {
      mapDescriptions.insert(si.toString(), si.segmentId());
      continue;
     }
    }
   }
   else
   {
    if(selectedStreet.trimmed().isEmpty())
    {
     // populate streets
     if((si.tracks() == 2 && ui->rbDouble->isChecked() ) ||
        (si.tracks() == 1 && ui->rbSingle->isChecked() )  ||
        ui->rbBoth->isChecked())
     {
      if(!mapDescriptions.contains(si.toString()))
      {
       mapDescriptions.insert(si.toString(), si.segmentId());
       continue;
      }
     }
    }
   }

   QString split;
   if(tokens.at(1).contains("to"))
    split = "to";
   else if(tokens.at(1).contains("zur"))
    split = "zur";
   else
    split = "-";
   tokens2 = tokens.at(1).split(split);
   {
    for(int i=0; i < tokens2.count(); i++)
    {
     QString street = tokens2.at(i);
     street = tokens.at(0).trimmed();
     if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
     //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
     if(street == selectedStreet && ui->cbLocation->currentText() == si.location())
     {
      // populate streets
      if((si.tracks() == 2 && ui->rbDouble->isChecked() ) ||
         (si.tracks() == 1 && ui->rbSingle->isChecked() )  ||
         ui->rbBoth->isChecked())
      {
       if(!mapDescriptions.contains(si.toString()))
       {
        mapDescriptions.insert(si.toString(), si.segmentId());
        continue;
       }
      }
     }
     else
     {
      if(selectedStreet.trimmed().isEmpty())
      {
       // populate streets
       if((si.tracks() == 2 && ui->rbDouble->isChecked() ) ||
          (si.tracks() == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
       {
        if(!mapDescriptions.contains(si.toString()))
        {
         mapDescriptions.insert(si.toString(), si.segmentId());
         continue;
        }
       }
      }
     }
    }
   }
  }
  else
  {
   tokens = description.split(" ");
   if(tokens.count() > 1)
   {
    QString street = tokens.at(0).trimmed();
    if(street == selectedStreet || selectedStreet.isEmpty())
    {
      if((si.tracks() == 2 && ui->rbDouble->isChecked() ) ||
      (si.tracks() == 1 && ui->rbSingle->isChecked() )  ||
      ui->rbBoth->isChecked())
      {
       if(!mapDescriptions.contains(si.toString()))
       {
        mapDescriptions.insert(si.toString(), si.segmentId());
        continue;
       }
      }
    }
   }
   qDebug() << "bypass " << si.toString();
  }
  if(si.needsUpdate())
   sql->updateSegment(new SegmentData(si));
 }
 if(m_SegmentId >0)
  ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(m_SegmentId));
// m_bridge->processScript("addModeOff");
// addPointModeAct->setChecked(false);
 //ui->cbSegments->model()->sort(0);
 QMapIterator<QString, int> iter(mapDescriptions);
 while(iter.hasNext())
 {
  iter.next();
     if(iter.key().isEmpty())
      continue;
     if(!iter.key().isEmpty())
       ui->cbSegments->addItem(iter.key(), iter.value());
 }
// ui->cbLocation->clear();
// ui->cbLocation->addItems(_locations);
 bRefreshingSegments = false;
}

void SegmentSelectionWidget::refreshStreetsCb()
{
    bCbStreetsRefreshing = true;
    QString selectedStreet = ui->cbStreets->currentText();
    ui->cbStreets->clear();
    QStringList streets;
#if 1
    streets = StreetsTableModel::instance()->getStreetnamesList(ui->cbLocation->currentText());
//#else
    QStringList tokens;
    QStringList tokens2;
    QString description;
    //selectedStreet = ui->cbStreets->currentText();

    //streets.clear();
    cbSegmentInfoMap = sql->getSegmentInfoList(ui->cbLocation->currentText());
    foreach(SegmentData sd, cbSegmentInfoMap.values())
    {
      if(!streets.contains(sd.streetName()))
      {
        if(!sd.streetName().isEmpty())
            streets.append(sd.streetName());
      }
    } // end for
    streets.sort();
#endif
    ui->cbStreets->addItems(streets);
    ui->cbStreets->setCompleter(new QCompleter(streets,this));
    ui->cbStreets->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(selectedStreet));
    bCbStreetsRefreshing = false;
}

void SegmentSelectionWidget::cbStreets_editingFinished()
{
 QString txt = ui->cbStreets->currentText();
 //ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(txt));
 if(bCbStreets_text_changed)
 {
  ui->cbStreets->setCurrentIndex(ui->cbStreets->findText(txt));
 }
 bCbStreets_text_changed = false;
}

void SegmentSelectionWidget::cbStreets_currentIndexChanged(int)
{
 saveStreet = ui->cbStreets->currentText();
 if(!bRefreshingSegments)
 refreshSegmentCB();
}

void SegmentSelectionWidget::segmentSelected(int pt, int segmentId, QList<LatLng> points)
{
 m_SegmentId = segmentId;
 SegmentInfo si = sql->getSegmentInfo(segmentId);
 ui->cbLocation->setCurrentText(si.location());
 ui->cbStreets->setCurrentText(si.streetName());
 int ix = ui->cbSegments->findData(segmentId);
 if(ix >=0)
  ui->cbSegments->setCurrentIndex(ix);
 else
 {

  bCbStreets_text_changed = true;
  cbStreets_editingFinished();
  ix = ui->cbSegments->findData(segmentId);
  if(ix >=0)
   ui->cbSegments->setCurrentIndex(ix);
  else
  {
    if(!si.toString().isEmpty())
        ui->cbSegments->addItem(si.toString(), segmentId);
    ui->rbBoth->setChecked(true);
  }
 }
}

void SegmentSelectionWidget::cbSegments_currentIndexChanged(int index)
{
 if(bRefreshingSegments)
  return;
 if(index < 0)
  return;
 int segmentId = ui->cbSegments->itemData(index).toInt();
 if(initialized)
 {
  if(segmentId != m_SegmentId){
   currSd = cbSegmentInfoMap.value(segmentId);
   emit segmentSelected(SegmentInfo(currSd));
  }
  m_SegmentId = segmentId;
 }
}

QComboBox* SegmentSelectionWidget::cbSegments()
{
 return ui->cbSegments;
}

SegmentData SegmentSelectionWidget::segmentSelected()
{
 int m_SegmentId = ui->cbSegments->currentData().toInt();
 OtherRouteView::instance()->showRoutesUsingSegment(m_SegmentId);
 currSd = SegmentData(sql->getSegmentInfo(m_SegmentId));
 return currSd;
}

void SegmentSelectionWidget::refresh()
{
 refreshSegmentCB();
}

void SegmentSelectionWidget::setCurrentSegment(int segmentId)
{
 currSd = sql->getSegmentInfo(segmentId);
 ui->cbLocation->findText(currSd.location());
 ui->cbStreets->findText(currSd.description());
 ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(segmentId));
}

void SegmentSelectionWidget::cbSegmentsTextChanged(QString txt)
{
    b_cbSegments_TextChanged = true;
    QString curText = ui->cbSegments->currentText();
}

void SegmentSelectionWidget::cbSegments_editingFinished()
{
 if(b_cbSegments_TextChanged ==true)
 {
  qint32 segmentId = -1;
  QString text = ui->cbSegments->currentText();


  bool bOk=false;
  segmentId = text.trimmed().toInt(&bOk, 10);
  if(!bOk)
  {
      b_cbSegments_TextChanged =false;
      return;
  }

  ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(segmentId));
  if(ui->cbSegments->currentIndex() < 0)
  {
   // is it valid?
   SegmentInfo sd = sql->getSegmentInfo(segmentId);
   if(sd.segmentId() >= 0)
   {
    ui->cbSegments->addItem(sd.toString(), segmentId);
   }
  }
 }
 b_cbSegments_TextChanged =false;

}
