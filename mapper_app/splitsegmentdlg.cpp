#include "splitsegmentdlg.h"
#include "ui_splitsegmentdlg.h"
#include "data.h"
#include "mainwindow.h"

bool compareSegmentInfo1(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description() < s2.description();
}
bool compareSegmentData1(const SegmentData & s1, const SegmentData & s2)
{
    return s1.description() < s2.description();
}


SplitSegmentDlg::SplitSegmentDlg(int segmentId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitSegmentDlg)
{
 ui->setupUi(this);

 common(segmentId);
}

void SplitSegmentDlg::common(int segmentId)
{
 //refreshSegments();
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);

 //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbTrackType1->addItems(SegmentData::ROUTETYPES);
 ui->cbTrackType1->setCurrentIndex(sd->routeType());
 ui->cbTrackType2->addItems(SegmentData::ROUTETYPES);
 ui->cbTrackType2->setCurrentIndex(sd->routeType());
 connect(ui->cbTrackType1, &QComboBox::currentTextChanged,[=](){
  canBeChanged();
 });
 connect(ui->cbTrackType2, &QComboBox::currentTextChanged,[=](){
  canBeChanged();
 });


 QList<CompanyData*> companies;
 if(sd)
  companies = SQL::instance()->getCompaniesInDateRange(sd->startDate(), sd->endDate());
 else
  companies = SQL::instance()->getCompanies();
 companyMap.clear();
 foreach (CompanyData* cd, companies) {
  companyMap.insert(cd->companyKey, cd);
  ui->cbCompany1->addItem(cd->toString(), cd->companyKey);
  ui->cbCompany2->addItem(cd->toString(), cd->companyKey);
 }
 if(sd)
 {
  ui->cbCompany1->setCurrentIndex(ui->cbCompany1->findData(sd->companyKey()));
  ui->cbCompany2->setCurrentIndex(ui->cbCompany2->findData(sd->companyKey()));
 }

 connect(ui->cbCompany1, &QComboBox::currentTextChanged, [=](){
  // if(ix < 0)
  //  return;
  CompanyData* cd = companyMap.value(ui->cbCompany1->currentData().toInt());
  //if(sd->startDate() >= cd->startDate && ui->dateFrom1->date()<= cd->endDate)
  if(ui->dateTo1->date() < cd->endDate)
  {
   ui->dateTo1->setDate(cd->endDate);
   ui->dateFrom2->setDate(cd->endDate.addDays(1));
  }

  if(company1Valid())
  {
   ui->lblHelp->clear();
   canBeChanged();
   return;
  }
  ui->btnOK->setEnabled(false);
  ui->lblHelp->setText("First company is invalid!");
 });

 connect(ui->cbCompany2, &QComboBox::currentTextChanged, [=](/*int ix*/){
  // if(ix < 0)
  //  return;
  CompanyData* cd = companyMap.value(ui->cbCompany2->currentData().toInt());
  //if(sd->startDate() >= ui->dateFrom2->date() && sd->endDate() <= cd->endDate)
  if(ui->dateFrom2->date() < cd->startDate)
  {
   ui->dateFrom2->setDate(cd->startDate);
   ui->dateTo1->setDate(cd->startDate.addDays(-1));
  }
  if(company2Valid())
  {
   ui->lblHelp->clear();
   canBeChanged();
   return;
  }
  ui->lblHelp->setText("Second company is invalid!");
 });

 QMap<int,TractionTypeInfo> ttypes = SQL::instance()->getTractionTypes();
 foreach(TractionTypeInfo ttype, ttypes.values())
 {
  ui->cbTractionType1->addItem(ttype.description, ttype.tractionType);
  ui->cbTractionType2->addItem(ttype.description, ttype.tractionType);
 }
 connect(ui->cbTractionType1, &QComboBox::currentTextChanged,[=](){
  canBeChanged();
 });
 connect(ui->cbTractionType2, &QComboBox::currentTextChanged,[=](){
  canBeChanged();
 });

 connect(ui->dateFrom1, &QDateTimeEdit::dateChanged, [=]{

 });
 connect(ui->dateTo1, &QDateTimeEdit::editingFinished, [=]{
  ui->dateFrom2->setDate(ui->dateTo1->date().addDays(1));
  ui->btnOK->setEnabled(true);
  canBeChanged();

 });

 connect(ui->dateFrom2, &QDateTimeEdit::editingFinished, [=]{
  ui->dateTo1->setDate(ui->dateFrom2->date().addDays(-1));
  ui->btnOK->setEnabled(true);
  canBeChanged();
 });

 connect(ui->btnCancel, &QPushButton::clicked, [=]{
  reject();
 });
 connect(ui->btnOK, &QPushButton::clicked, [=]{
  if(!  canBeChanged())
   return;

  bool result = processChanges();
  if(result)
   accept();
 });
  //refreshSegments();
 setSegment(segmentId);

 ui->btnOK->setEnabled(false);
 canBeChanged();


}
SplitSegmentDlg::SplitSegmentDlg(SegmentData* sd, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitSegmentDlg)
{
 ui->setupUi(this);
 this->sd = sd;
 sdSpecified = true;
 common(sd->segmentId());
}

SplitSegmentDlg::~SplitSegmentDlg()
{
 delete ui;
}

//void SplitSegmentDlg::refreshSegments()
//{
//  ui->cbSegments->clear();
//  cbSegmentInfoList = SQL::instance()->getSegmentInfoList();
//  //std::sort(cbSegmentDataList.values().begin(), cbSegmentDataList.values().end(),compareSegmentData1);
//  //foreach (segmentInfo sI in cbSegmentInfoList)
//  for(int i=0; i < cbSegmentInfoList.count(); i++)
//  {
//   SegmentData sd = cbSegmentInfoList.values().at(i);
//   ui->cbSegments->addItem(sd.toString(), sd.segmentId());
//  }
//}

void SplitSegmentDlg::setupDates(SegmentData* sd){
 ui->dateFrom1->setEnabled(true);
 ui->dateTo2->setEnabled(true);
 ui->dateFrom1->setDate( sd->startDate());
 ui->dateTo1->setMinimumDate(ui->dateFrom1->date().addDays(1));
 ui->dateTo1->setDate( sd->startDate().addDays(1));
 ui->dateTo1->setMaximumDate(ui->dateTo2->date().addDays(-2));
 ui->dateFrom2->setDate( sd->startDate().addDays(2));
 ui->dateFrom2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateTo2->setDate( sd->endDate());
 ui->dateTo2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateFrom2->setMaximumDate(ui->dateTo2->date().addDays(-1));
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);
}

void SplitSegmentDlg::setSegment(int segmentId)
{
 if(sd == nullptr)
  sd = new SegmentData(SQL::instance()->getSegmentInfo(segmentId));
 SQL::instance()->updateSegmentDates(sd->segmentId()); // update segment's begin and end dates.
 //ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(sd->segmentId()));
 ui->txtSegment->setText(sd->description());
 //ui->txtNewSegmentName1->setText(sd->description());


 setupDates(sd);
 ui->cbTrackType1->setCurrentIndex(sd->routeType());
 ui->cbTrackType2->setCurrentIndex(sd->routeType());

 ui->cbTractionType1->setCurrentIndex(ui->cbTractionType1->findData(sd->tractionType()));
 ui->cbTractionType2->setCurrentIndex(ui->cbTractionType2->findData(sd->tractionType()));

 ui->cbCompany1->setCurrentIndex(ui->cbCompany1->findData(sd->companyKey()));
 ui->cbCompany2->setCurrentIndex(ui->cbCompany2->findData(sd->companyKey()));

 ui->btnOK->setEnabled(true);
 canBeChanged();

}

bool SplitSegmentDlg::processChanges()
{
 ui->lblHelp->clear();
 // if the track type is being changed, a new segment with the track
 // type.
 if(sdSpecified)
 {
  int newSegmentId = sd->segmentId();
  SQL::instance()->beginTransaction("splitSegment");
  if(ui->cbTractionType1->currentIndex() != ui->cbTractionType2->currentIndex())
  {
   SegmentInfo si = SegmentInfo(*sd);
   si.setRouteType((RouteType)ui->cbTractionType2->currentData().toInt());
   bool bExists = false;
   newSegmentId = SQL::instance()->addSegment(si,&bExists, true);
   if(newSegmentId < 0)
   {
    SQL::instance()->rollbackTransaction("splitSegment");
    ui->lblHelp->setText(tr("error creating segment"));
    return false;
   }
  }
  SegmentData* sdOld = new SegmentData(*sd);
  if(!SQL::instance()->deleteRoute(*sd))
  {
   ui->lblHelp->setText(tr("error deleting original segment"));
   SQL::instance()->rollbackTransaction("splitSegment");
   return false;
  }

  sd->setCompanyKey(ui->cbCompany1->currentData().toInt());
  sd->setTractionType(ui->cbTractionType1->currentData().toInt());
  sd->setEndDate(ui->dateTo1->date());
  if(!SQL::instance()->addSegmentToRoute(sd))
  {
   ui->lblHelp->setText(tr("error updating route 1"));
   SQL::instance()->rollbackTransaction("splitSegment");
   return false;
  }

  SegmentData* sdNew = new SegmentData(*sdOld);
  sdNew->setSegmentId(newSegmentId);
  sdNew->setStartDate(ui->dateFrom2->date());
  sdNew->setRouteType((RouteType)ui->cbTrackType1->currentIndex());
  sdNew->setCompanyKey(ui->cbCompany2->currentData().toInt());
  if(!SQL::instance()->addSegmentToRoute(sdNew))
  {
     ui->lblHelp->setText(tr("error updating route 2"));
     SQL::instance()->rollbackTransaction("splitSegment");
     return false;
  }
 }
 else
 {
  // find all the routes using this segment
  QList<SegmentData*> routes = SQL::instance()->getRouteSegmentsBySegment(sd->segmentId());

  // update the database with changes
  SQL::instance()->beginTransaction("splitSegment");

  bool exists = false;
  int newSegmentId;
#if 0
  newSegmentId = SQL::instance()->addSegment(siNew.description, siNew.oneWay, siNew.tracks, siNew.routeType,
                                                siNew.pointList, &exists, true);
#else
  newSegmentId = SQL::instance()->addSegment(*sd, &exists, true);
#endif
  if(exists || newSegmentId <= 0)
  {
   SQL::instance()->rollbackTransaction("splitSegment");
    return false;
  }

  if(!SQL::instance()->updateSegment(new SegmentData(*sd)))
  {
   SQL::instance()->rollbackTransaction("splitSegment");
   return false;
  }

  int routesChanged = 0;
  if(ui->chkUpdateAffectedRoutes)
  {
   foreach(SegmentData* sd1, routes)
   {
    if(sd1->segmentId() != sd->segmentId())
     continue;

    // If split date is between route segment's start and end dates
    if( sd1->endDate() > sd->endDate()
        &&  sd1->startDate() < sd->startDate())
    {
     // if(!SQL::instance()->updateRouteDate(sd->segmentId(), sd1->startDate().toString("yyyy/MM/dd"),
     //                                      sd->endDate().toString("yyyy/MM/dd")))
        SegmentData sd2 = SegmentData(*sd1);
     sd2.setEndDate(sd->endDate());
     if(!SQL::instance()->updateRoute(*sd1,sd2))
     {
      SQL::instance()->rollbackTransaction("splitSegment");
      return false;
     }
    // remainder of route segment > solit date, add new segment to route
#if 0
    siNew.endDate = rd.endDate.toString("yyyy/MM/dd");
    rd.lineKey = siNew.segmentId;
#else
      sd->setEndDate(sd1->endDate());
      sd1->setSegmentId(newSegmentId);
#endif
     if(!SQL::instance()->addSegmentToRoute(sd1))
     {
      SQL::instance()->rollbackTransaction("splitSegment");
      return false;
     }
     routesChanged++;

     continue;
    }

    // route segment's end date is before split date, nothing to do
    if(sd1->endDate() < sd->endDate())
     continue;

    // if route segment's start date is after split date, use new segment
    if(sd1->startDate()> sd->endDate())
    {
     //if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), newSegmentId, rd.next, rd.prev))
     // if(!SQL::instance()->updateRouteSegment(sd1->segmentId(), sd1->startDate().toString("yyyy/MM/dd"),
     //                                         sd1->endDate().toString("yyyy/MM/dd"),newSegmentId))
        SegmentData sd2 = SegmentData(*sd1);
     sd2.setSegmentId(newSegmentId);
     if(!SQL::instance()->updateRoute(*sd1,sd2))
     {
      SQL::instance()->rollbackTransaction("splitSegment");
      return false;
     }
    }

    routesChanged++;

  }
  QPalette palette = ui->lblHelp->palette();
  QBrush brush(QColor(0, 255, 127, 255));
  palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
  ui->lblHelp->setPalette(palette);

  ui->lblHelp->setText(tr("new segment %1 created; %2 routes changed").arg(newSegmentId).arg(routesChanged));
  }
 }

 SQL::instance()->commitTransaction("splitSegment");
 //MainWindow::instance()->refreshRoutes();
 return true;
}

bool SplitSegmentDlg::company1Valid()
{
 CompanyData* cd = companyMap.value(ui->cbCompany1->currentData().toInt());
 if(!cd)
  return false;
 if(sd->startDate() >= cd->startDate && ui->dateFrom1->date()<= cd->endDate)
 {
  ui->lblHelp->clear();
  return true;
 }
 return false;
}

bool SplitSegmentDlg::company2Valid()
{
 CompanyData* cd = companyMap.value(ui->cbCompany2->currentData().toInt());
 if(!cd)
  return false;
 //if(sd->startDate() >= ui->dateFrom2->date() && ui->dateTo2->date() <= cd->endDate)
 if(cd->startDate <= ui->dateFrom2->date() && ui->dateTo2->date() <= cd->endDate)
 {
  ui->lblHelp->clear();
  return true;
 }
 return false;
}

bool SplitSegmentDlg::canBeChanged()
{
 if(!company1Valid() || !company2Valid())
 {
  ui->btnOK->setEnabled(false);
 }
 if(ui->cbCompany1->currentIndex() == ui->cbCompany2->currentIndex()
    && ui->cbTrackType1->currentIndex() == ui->cbTrackType2->currentIndex()
    && ui->cbTractionType1->currentIndex() == ui->cbTractionType2->currentIndex())
 {
  ui->lblHelp->setText(tr("Nothing is changed!"));
  ui->btnOK->setEnabled(false);
  return false;
 }
 ui->lblHelp->clear();
 ui->btnOK->setEnabled(true);
 return true;
}
