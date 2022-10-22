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
}

void SegmentSelectionWidget::initialize()
{
 refreshSegmentCB();

 connect(SQL::instance(), &SQL::segmentsChanged, [=]{
  refreshSegmentCB();
 });
 connect(webViewBridge::instance(), SIGNAL(segmentSelected(int, int)), this, SLOT(segmentSelected(int, int)));
 //connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
 //connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
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
 return s1.getDescription() < s2.getDescription();
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
 cbSegmentDataMap = sql->getSegmentDataList();
 //qSort(cbSegmentDataList.begin(), cbSegmentDataList.end(),compareSegmentDataByName);
 //foreach (segmentInfo sI in cbSegmentInfoList)
 foreach(SegmentData sd, cbSegmentDataMap.values())
 {
  description = sd.getDescription();
  tokens = description.split(",");
  if(tokens.count() > 1)
  {
   QString street = tokens.at(0).trimmed();
   if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
   if(street == selectedStreet)
   {
    // populate streets
    if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
       (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
       ui->rbBoth->isChecked())
//     if(ui->cbSegments->findText(sd.toString())<0)
//      ui->cbSegments->addItem(sd.toString(), sd.segmentId());
           if(!mapDescriptions.contains(sd.toString()))

     mapDescriptions.insert(sd.toString(), sd.segmentId());
    continue;
   }
   else
   {
    if(selectedStreet.trimmed().isEmpty())
    {
     // populate streets
     if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
        (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
        ui->rbBoth->isChecked())
//     if(ui->cbSegments->findText(sd.toString())<0)
//      ui->cbSegments->addItem(sd.toString(), sd.segmentId());
             if(!mapDescriptions.contains(sd.toString()))

       mapDescriptions.insert(sd.toString(), sd.segmentId());
     continue;
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
     if(street == selectedStreet)
     {
      // populate streets
      if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
         (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
         ui->rbBoth->isChecked())
//      if(ui->cbSegments->findText(sd.toString())<0)
//       ui->cbSegments->addItem(sd.toString(), sd.segmentId());
       if(!mapDescriptions.contains(sd.toString()))
        mapDescriptions.insert(sd.toString(), sd.segmentId());
      continue;
     }
     else
     {
      if(selectedStreet.trimmed().isEmpty())
      {
       // populate streets
       if((sd.tracks() == 2 && ui->rbDouble->isChecked() ) ||
          (sd.tracks() == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
//       if(ui->cbSegments->findText(sd.toString())<0)
//        ui->cbSegments->addItem(sd.toString(), sd.segmentId());
        if(!mapDescriptions.contains(sd.toString()))
         mapDescriptions.insert(sd.toString(), sd.segmentId());
       continue;
      }
     }
    }
   }
  }
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
 bRefreshingSegments = false;
}

void SegmentSelectionWidget::refreshStreetsCb()
{
 QMutexLocker locker(&mutex2);

 QStringList streets;
 QStringList tokens;
 QStringList tokens2;
 QString description;
 QString selectedStreet = ui->cbStreets->currentText();
 bCbStreetsRefreshing = true;

 ui->cbStreets->clear();
 streets.clear();
 ui->cbStreets->addItem("");
 cbSegmentDataMap = sql->getSegmentDataList();
 foreach(SegmentData sd, cbSegmentDataMap.values())
 {
  description = sd.getDescription();
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
}

void SegmentSelectionWidget::cbSegments_currentIndexChanged(int index)
{
 if(bRefreshingSegments)
  return;
 if(index <= 0)
  return;
 int segmentId = ui->cbSegments->itemData(index).toInt();
 if(initialized)
  emit segmentSelected(currSd = cbSegmentDataMap.value(segmentId));
}

QComboBox* SegmentSelectionWidget::cbSegments()
{
 return ui->cbSegments;
}

SegmentData SegmentSelectionWidget::segmentSelected()
{
// int segmentId = ui->cbSegments->currentData().toInt();
// currSd = sql->getSegmentData(segmentId);
 return currSd;
}

void SegmentSelectionWidget::refresh()
{
 refreshSegmentCB();
}

void SegmentSelectionWidget::setCurrentSegment(int segmentId)
{
 ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(segmentId));
 currSd = sql->getSegmentData(segmentId);
}
