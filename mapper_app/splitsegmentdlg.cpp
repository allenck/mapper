#include "splitsegmentdlg.h"
#include "ui_splitsegmentdlg.h"
#include "data.h"

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
 refreshSegments();
 setSegment(segmentId);
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);

 connect(ui->cbSegments, &QComboBox::currentTextChanged, [=]{
  sd = cbSegmentDataList.values().at(ui->cbSegments->currentIndex());
  setSegment(sd.segmentId());

 });


 //QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbTrackType1->addItems(SegmentData::ROUTETYPES);
 ui->cbTrackType1->setCurrentIndex(sd.routeType());
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
  cbSegmentDataList = SQL::instance()->getSegmentInfoList();
  //std::sort(cbSegmentDataList.values().begin(), cbSegmentDataList.values().end(),compareSegmentData1);
  //foreach (segmentInfo sI in cbSegmentInfoList)
  for(int i=0; i < cbSegmentDataList.count(); i++)
  {
   SegmentData sd = cbSegmentDataList.values().at(i);
   ui->cbSegments->addItem(sd.toString(), sd.segmentId());
  }
}

void SplitSegmentDlg::setupDates(SegmentData sd){
 ui->dateFrom1->setEnabled(true);
 ui->dateTo2->setEnabled(true);
 ui->dateFrom1->setDate( sd.startDate());
 ui->dateTo1->setMinimumDate(ui->dateFrom1->date().addDays(1));
 ui->dateTo1->setDate( sd.startDate().addDays(1));
 ui->dateTo1->setMaximumDate(ui->dateTo2->date().addDays(-2));
 ui->dateFrom2->setDate( sd.startDate().addDays(2));
 ui->dateFrom2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateTo2->setDate( sd.endDate());
 ui->dateTo2->setMinimumDate(ui->dateFrom1->date().addDays(2));
 ui->dateFrom2->setMaximumDate(ui->dateTo2->date().addDays(-1));
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);
}

void SplitSegmentDlg::setSegment(int segmentId)
{
 sd = SQL::instance()->getSegmentInfo(segmentId);
 SQL::instance()->updateSegmentDates(sd.segmentId()); // update segment's begin and end dates.
 ui->cbSegments->setCurrentIndex(ui->cbSegments->findData(sd.segmentId()));
 ui->txtNewSegmentName1->setText(sd.description());


 setupDates(sd);
 ui->cbTrackType1->setCurrentIndex(sd.routeType());
 ui->cbTrackType2->setCurrentIndex(sd.routeType());

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
 SegmentData sdNew = SegmentData(sd);
 sd.setEndDate(ui->dateTo1->date());
 sd.setRouteType((RouteType)ui->cbTrackType1->currentIndex());
   sdNew.setEndDate(ui->dateTo1->date());
 sd.setDescription(ui->txtNewSegmentName1->text());
 sdNew.setStartDate(ui->dateFrom2->date());
 sdNew.setRouteType((RouteType)ui->cbTrackType1->currentIndex());
 sdNew.setDescription(ui->txtNewSegmentName2->text());
#endif

 // find all the routes using this segment
 QList<SegmentData> routes = SQL::instance()->getRouteSegmentsBySegment(sd.segmentId());

 // update the database with changes
 SQL::instance()->beginTransaction("splitSegment");

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
  SQL::instance()->rollbackTransaction("splitSegment");
   return false;
 }

 if(!SQL::instance()->updateSegment(&sd))
 {
  SQL::instance()->rollbackTransaction("splitSegment");
  return false;
 }

 int routesChanged = 0;
 if(ui->chkUpdateAffectedRoutes)
 {
  foreach(SegmentData rd, routes)
  {
   if(rd.segmentId() != sd.segmentId())
    continue;

   // If split date is between route segment's start and en dates
   if( rd.endDate() > sd.endDate()
       &&  rd.startDate() < sd.startDate())
   {
    if(!SQL::instance()->updateRouteDate(sd.segmentId(), rd.startDate().toString("yyyy/MM/dd"), sd.endDate().toString("yyyy/MM/dd")))
    {
     SQL::instance()->rollbackTransaction("splitSegment");
     return false;
    }
    // remainder of route segment > solit date, add new segment to route
#if 0
    siNew.endDate = rd.endDate.toString("yyyy/MM/dd");
    rd.lineKey = siNew.segmentId;
#else
    sdNew.setEndDate(rd.endDate());
    rd.setSegmentId(newSegmentId);
#endif
    if(!SQL::instance()->addSegmentToRoute(rd))
    {
     SQL::instance()->rollbackTransaction("splitSegment");
     return false;
    }
    routesChanged++;

    continue;
   }

   // route segment's end date is before split date, nothing to do
   if(rd.endDate() < sd.endDate())
    continue;

   // if route segment's start date is after split date, use new segment
   if(rd.startDate()> sd.endDate())
   {
    //if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), newSegmentId, rd.next, rd.prev))
    if(!SQL::instance()->updateRouteSegment(rd.segmentId(), rd.startDate().toString("yyyy/MM/dd"),
                                            rd.endDate().toString("yyyy/MM/dd"),newSegmentId))
    {
     SQL::instance()->rollbackTransaction("splitSegment");
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
 SQL::instance()->commitTransaction("splitSegment");
 return true;
}
