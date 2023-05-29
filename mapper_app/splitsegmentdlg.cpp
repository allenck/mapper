#include "splitsegmentdlg.h"
#include "ui_splitsegmentdlg.h"
#include "data.h"

bool compareSegmentInfo(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description < s2.description;
}


SplitSegmentDlg::SplitSegmentDlg(int segmentId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitSegmentDlg)
{
 ui->setupUi(this);
 refreshSegments();
 setSegment(segmentId);
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);

 connect(ui->cbSegments, &QComboBox::currentTextChanged, [=]{
  si = cbSegmentInfoList.at(ui->cbSegments->currentIndex());
  setSegment(si.segmentId);

 });


 //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbTrackType1->addItems(SegmentData::ROUTETYPES);
 ui->cbTrackType1->setCurrentIndex(si.routeType);
 ui->cbTrackType2->addItems(SegmentData::ROUTETYPES);

 connect(ui->dateFrom1, &QDateTimeEdit::dateChanged, [=]{

 });
 connect(ui->dateTo1, &QDateTimeEdit::editingFinished, [=]{
  ui->dateFrom2->setDate(ui->dateTo1->date().addDays(1));
  ui->btnOK->setEnabled(true);
  ui->btnApply->setEnabled(true);

 });

 connect(ui->dateFrom2, &QDateTimeEdit::editingFinished, [=]{
  ui->dateTo1->setDate(ui->dateFrom2->date().addDays(-1));
  ui->btnOK->setEnabled(true);
  ui->btnApply->setEnabled(true);
 });

 connect(ui->btnCancel, &QPushButton::clicked, [=]{
  reject();
 });
 connect(ui->btnOK, &QPushButton::clicked, [=]{
  bool result = processChanges();
  if(result)
   accept();
 });
 connect(ui->btnApply, &QPushButton::clicked, [=]{
  bool result = processChanges();
  if(result)
  {
   ui->btnOK->setEnabled(false);
   ui->btnApply->setEnabled(false);
  }
  refreshSegments();
 });
 ui->btnOK->setEnabled(false);
 ui->btnApply->setEnabled(false);

}


SplitSegmentDlg::~SplitSegmentDlg()
{
 delete ui;
}

void SplitSegmentDlg::refreshSegments()
{
  ui->cbSegments->clear();
  cbSegmentInfoList = SQL::instance()->getSegmentInfo();
  std::sort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfo);
  //foreach (segmentInfo sI in cbSegmentInfoList)
  for(int i=0; i < cbSegmentInfoList.count(); i++)
  {
   SegmentInfo si = cbSegmentInfoList.at(i);
   ui->cbSegments->addItem(si.toString(), si.segmentId);
  }
}

void SplitSegmentDlg::setupDates(SegmentInfo si){
 ui->dateFrom1->setEnabled(true);
 ui->dateTo2->setEnabled(true);
 ui->dateFrom1->setDate( QDate::fromString( si.startDate, "yyyy/MM/dd"));
 ui->dateTo1->setMinimumDate(ui->dateFrom1->date().addDays(1));
 ui->dateTo1->setDate( QDate::fromString( si.startDate, "yyyy/MM/dd").addDays(1));
 ui->dateTo1->setMaximumDate(ui->dateTo2->date().addDays(-2));
 ui->dateFrom2->setDate( QDate::fromString( si.startDate, "yyyy/MM/dd").addDays(2));
 ui->dateFrom2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateTo2->setDate( QDate::fromString( si.endDate, "yyyy/MM/dd"));
 ui->dateTo2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateFrom2->setMaximumDate(ui->dateTo2->date().addDays(-1));
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);
}

void SplitSegmentDlg::setSegment(int segmentId)
{
 si = SQL::instance()->getSegmentInfo(segmentId);
 SQL::instance()->updateSegmentDates(&si); // update segment's begin and end dates.
 ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(si.segmentId));
 ui->txtNewSegmentName1->setText(si.description);


 setupDates(si);
 ui->cbTrackType1->setCurrentIndex(si.routeType);
 ui->cbTrackType2->setCurrentIndex(si.routeType);

 ui->btnOK->setEnabled(true);
 ui->btnApply->setEnabled(true);

}

bool SplitSegmentDlg::processChanges()
{
#if 0
 SegmentInfo siNew = SegmentInfo(si);
 si.endDate = ui->dateTo1->date().toString("yyyy/MM/dd");
 siNew.startDate = ui->dateFrom2->date().toString("yyyy/MM/dd");
 si.routeType = (RouteType)ui->cbTrackType1->currentIndex();
 siNew.routeType = (RouteType)ui->cbTrackType2->currentIndex();
 si.description = ui->txtNewSegmentName1->text();
 siNew.description = ui->txtNewSegmentName2->text();
#else
 SegmentData sdNew = SegmentData(si);
 si.endDate = ui->dateTo1->date().toString("yyyy/MM/dd");
 si.routeType = (RouteType)ui->cbTrackType1->currentIndex();sdNew.setEndDate(ui->dateTo1->date());
 si.description = ui->txtNewSegmentName1->text();
 sdNew.setStartDate(ui->dateFrom2->date());
 sdNew.setRouteType((RouteType)ui->cbTrackType1->currentIndex());
 sdNew.setDescription(ui->txtNewSegmentName2->text());
#endif

 // find all the routes using this segment
 QList<RouteData> routes = SQL::instance()->getRouteSegmentsBySegment(si.segmentId);

 // update the database with changes
 SQL::instance()->BeginTransaction("splitSegment");

 bool exists = false;
 int newSegmentId;
#if 0
  newSegmentId = SQL::instance()->addSegment(siNew.description, siNew.oneWay, siNew.tracks, siNew.routeType,
                                                siNew.pointList, &exists, true);
#else
 newSegmentId = SQL::instance()->addSegment(sdNew, &exists, true);
#endif
 if(exists || newSegmentId <= 0)
 {
  SQL::instance()->RollbackTransaction("splitSegment");
   return false;
 }

 if(!SQL::instance()->updateSegment(&si))
 {
  SQL::instance()->RollbackTransaction("splitSegment");
  return false;
 }

 int routesChanged = 0;
 if(ui->chkUpdateAffectedRoutes)
 {
  foreach(RouteData rd, routes)
  {
   if(rd.lineKey != si.segmentId)
    continue;

   // If split date is between route segment's start and en dates
   if( rd.endDate.toString("yyyy/MM/dd") > si.endDate
       &&  rd.startDate.toString("yyyy/MM/dd") < si.startDate)
   {
    if(!SQL::instance()->updateRouteDate(si.segmentId, rd.startDate.toString("yyyy/MM/dd"), si.endDate))
    {
     SQL::instance()->RollbackTransaction("splitSegment");
     return false;
    }
    // remainder of route segment > solit date, add new segment to route
#if 0
    siNew.endDate = rd.endDate.toString("yyyy/MM/dd");
    rd.lineKey = siNew.segmentId;
#else
    sdNew.setEndDate(rd.endDate);
    rd.lineKey = newSegmentId;
#endif
    if(!SQL::instance()->addSegmentToRoute(rd))
    {
     SQL::instance()->RollbackTransaction("splitSegment");
     return false;
    }
    routesChanged++;

    continue;
   }

   // route segment's end date is before split date, nothing to do
   if(rd.endDate.toString("yyyy/MM/dd") < si.endDate)
    continue;

   // if route segment's start date is after split date, use new segment
   if(rd.startDate.toString("yyyy/MM/dd") > si.endDate)
   {
    //if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), newSegmentId, rd.next, rd.prev))
    if(!SQL::instance()->updateRouteSegment(rd.lineKey, rd.startDate.toString("yyyy/MM/dd"),rd.endDate.toString("yyyy/MM/dd"),newSegmentId))
    {
     SQL::instance()->RollbackTransaction("splitSegment");
     return false;
    }
   }

   routesChanged++;
  }
 }
 QPalette palette = ui->lblHelp->palette();
 QBrush brush(QColor(0, 255, 127, 255));
 palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
 ui->lblHelp->setPalette(palette);

 ui->lblHelp->setText(tr("new segment %1 created; %2 routes changed").arg(newSegmentId).arg(routesChanged));
 SQL::instance()->CommitTransaction("splitSegment");
 return true;
}
