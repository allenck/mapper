#include "segmentselectionwidget.h"
#include "ui_segmentselectionwidget.h"
#include "webviewbridge.h"

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

}

void SegmentSelectionWidget::initialize()
{
 refreshLocations();

 refreshSegmentCB();

 connect(SQL::instance(), &SQL::segmentsChanged, [=](int segmentId){
  if(ui->cbSegments->findData(segmentId) >= 0)
  {
   SegmentInfo sd = sql->getSegmentInfo(segmentId);
   int  ix = ui->cbSegments->findData(segmentId);
   if(ui->cbSegments->itemText(ix) != sd.toString())
    ui->cbSegments->setItemText(ix, sd.toString());
  }
 });
 connect(WebViewBridge::instance(), SIGNAL(segmentSelected(int, int)), this, SLOT(segmentSelected(int, int)));
 //connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
 connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
 //connect(ui->cbSegments, SIGNAL(signalFocusOut()), this, SLOT(cbSegments_editingFinished()));
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
 _locations = sql->getLocations();
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

 QMutexLocker locker(&mutex);
 bRefreshingSegments = true;
 if(!bCbStreetsRefreshing)
  refreshStreetsCb();
 ui->cbSegments->clear();
 mapDescriptions.clear();
 cbSegmentDataMap = sql->getSegmentInfoList(ui->cbLocation->currentText());
 //qSort(cbSegmentDataList.begin(), cbSegmentDataList.end(),compareSegmentDataByName);
 //foreach (segmentInfo sI in cbSegmentInfoList)
 foreach(SegmentData sd, cbSegmentDataMap.values())
 {
  description = sd.description();
  if(!sd.location().isEmpty())
  {
   if(!_locations.contains(sd.location()))
    _locations.append(sd.location());
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
   if(street == selectedStreet && ui->cbLocation->currentText() == sd.location())
   {
    // populate streets
    if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
       (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
       ui->rbBoth->isChecked())
    {
     if(!mapDescriptions.contains(sd.toString()))
     {
      mapDescriptions.insert(sd.toString(), sd.segmentId());
      continue;
     }
    }
   }
   else
   {
    if(selectedStreet.trimmed().isEmpty())
    {
     // populate streets
     if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
        (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
        ui->rbBoth->isChecked())
     {
      if(!mapDescriptions.contains(sd.toString()))
      {
       mapDescriptions.insert(sd.toString(), sd.segmentId());
       continue;
      }
     }
    }
   }
   tokens2 = tokens.at(1).split("to");
   {
    for(int i=0; i < tokens2.count(); i++)
    {
     QString street = tokens2.at(i);
     street = tokens.at(0).trimmed();
     if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
     //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
     if(street == selectedStreet && ui->cbLocation->currentText() == sd.location())
     {
      // populate streets
      if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
         (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
         ui->rbBoth->isChecked())
      {
       if(!mapDescriptions.contains(sd.toString()))
       {
        mapDescriptions.insert(sd.toString(), sd.segmentId());
        continue;
       }
      }
     }
     else
     {
      if(selectedStreet.trimmed().isEmpty())
      {
       // populate streets
       if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
          (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
       {
        if(!mapDescriptions.contains(sd.toString()))
        {
         mapDescriptions.insert(sd.toString(), sd.segmentId());
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
      if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
      (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
      ui->rbBoth->isChecked())
      {
       if(!mapDescriptions.contains(sd.toString()))
       {
        mapDescriptions.insert(sd.toString(), sd.segmentId());
        continue;
       }
      }
    }
   }
   qDebug() << "bypass " << sd.toString();
  }
  if(sd.needsUpdate())
   sql->updateSegment(&sd);
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
  ui->cbSegments->addItem(iter.key(), iter.value());
 }
// ui->cbLocation->clear();
// ui->cbLocation->addItems(_locations);
 bRefreshingSegments = false;
}

void SegmentSelectionWidget::refreshStreetsCb()
{
 //QMutexLocker locker(&mutex2);

 QStringList streets;
 QStringList tokens;
 QStringList tokens2;
 QString description;
 QString selectedStreet = ui->cbStreets->currentText();
 bCbStreetsRefreshing = true;

 ui->cbStreets->clear();
 streets.clear();
 ui->cbStreets->addItem("");
 cbSegmentDataMap = sql->getSegmentInfoList(ui->cbLocation->currentText());
 foreach(SegmentData sd, cbSegmentDataMap.values())
 {
#if 0
  description = sd.description();
  tokens = description.split(",");
  if(tokens.count() > 1)
  {
   QString street = tokens.at(0).trimmed();
   if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
   if(!streets.contains(street))
   {
    streets.append(street);
   }
   tokens2 = tokens.at(1).split("to");
   {
    for(int i=0; i < tokens2.count(); i++)
    {
     QString street2 = tokens2.at(i);
     street2 = tokens.at(0).trimmed();
     if(street2.indexOf("(")) street2= street.mid(0, street2.indexOf("("));
     //if(street2.indexOf(" ")) street2= street2.mid(0, street2.indexOf(" "));
     if(!streets.contains(street2))
     {
      streets.append(street2);
     }
    }
   }
  }
  else
  {
   tokens = description.split(" ");
  }
#else
  if(!streets.contains(sd.streetName()))
  {
   streets.append(sd.streetName());
  }
#endif
 } // end for
 streets.sort();
 ui->cbStreets->addItems(streets);
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

void SegmentSelectionWidget::segmentSelected(int pt, int segmentId)
{
 m_SegmentId = segmentId;
 int ix = ui->cbSegments->findData(segmentId);
 if(ix >=0)
  ui->cbSegments->setCurrentIndex(ix);
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
   currSd = cbSegmentDataMap.value(segmentId);
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
 currSd = sql->getSegmentInfo(m_SegmentId);
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
}

void SegmentSelectionWidget::cbSegments_editingFinished()
{
 if(b_cbSegments_TextChanged ==true)
 {
  qint32 segmentId = -1;
  QString text = ui->cbSegments->currentText();

  bool bOk=false;
  segmentId = text.toInt(&bOk, 10);

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
