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

 connect(webViewBridge::instance(), SIGNAL(segmentSelected(int, int)), this, SLOT(segmentSelected(int, int)));
 connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegmentsSelectedValueChanged(int)));
 connect(ui->cbSegments, SIGNAL(editTextChanged(QString)), this, SLOT(cbSegmentsTextChanged(QString)));
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
 connect(ui->cbStreets, &QComboBox::editTextChanged, [=]{
  bCbStreets_text_changed = true;
 });
 connect(ui->cbStreets, SIGNAL(editingFinished()), this, SLOT(cbStreets_editingFinished()));
 connect(ui->cbStreets, SIGNAL(currentIndexChanged(int)), this, SLOT(cbStreets_currentIndexChanged(int)));
 connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegments_currentIndexChanged(int)));

 refreshSegmentCB();

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

 bRefreshingSegments = true;
 if(!bCbStreetsRefreshing)
  refreshStreetsCb();
 ui->cbSegments->clear();
 cbSegmentDataList = sql->getSegmentDataList();
 qSort(cbSegmentDataList.begin(), cbSegmentDataList.end(),compareSegmentDataByName);
 //foreach (segmentInfo sI in cbSegmentInfoList)
 for(int i=0; i < cbSegmentDataList.count(); i++)
 {
  SegmentData sd = cbSegmentDataList.at(i);
  description = sd.getDescription();
  tokens = description.split(",");
  if(tokens.count() > 1)
  {
   QString street = tokens.at(0).trimmed();
   if(street.indexOf("(")) street= street.mid(0, street.indexOf("("));
   if(street == selectedStreet)
   {
    // populate streets
    if((sd.getTracks() == 2 && ui->rbDouble->isChecked() ) ||
       (sd.getTracks() == 1 && ui->rbSingle->isChecked() )  ||
       ui->rbBoth->isChecked())
     ui->cbSegments->addItem(sd.toString(), sd.getSegmentId());
    continue;
   }
   else
   {
    if(selectedStreet.trimmed().isEmpty())
    {
     // populate streets
     if((sd.getTracks() == 2 && ui->rbDouble->isChecked() ) ||
        (sd.getTracks() == 1 && ui->rbSingle->isChecked() )  ||
        ui->rbBoth->isChecked())
      ui->cbSegments->addItem(sd.toString(), sd.getSegmentId());
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
      if((sd.getTracks() == 2 && ui->rbDouble->isChecked() ) ||
         (sd.getTracks() == 1 && ui->rbSingle->isChecked() )  ||
         ui->rbBoth->isChecked())
       ui->cbSegments->addItem(sd.toString(), sd.getSegmentId());
      continue;
     }
     else
     {
      if(selectedStreet.trimmed().isEmpty())
      {
       // populate streets
       if((sd.getTracks() == 2 && ui->rbDouble->isChecked() ) ||
          (sd.getTracks() == 1 && ui->rbSingle->isChecked() )  ||
          ui->rbBoth->isChecked())
        ui->cbSegments->addItem(sd.toString(), sd.getSegmentId());
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
 bRefreshingSegments = false;
}

void SegmentSelectionWidget::refreshStreetsCb()
{
 QStringList streets;
 QStringList tokens;
 QStringList tokens2;
 QString description;
 QString selectedStreet = ui->cbStreets->currentText();
 bCbStreetsRefreshing = true;

 ui->cbStreets->clear();
 ui->cbStreets->addItem("");
 cbSegmentDataList = sql->getSegmentDataList();
 for(int i=0; i < cbSegmentDataList.count(); i++)
 {
  SegmentData sd = cbSegmentDataList.at(i);
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

void SegmentSelectionWidget::cbSegments_currentIndexChanged(int segmentId)
{
 emit segmentSelected(segmentId);
}
