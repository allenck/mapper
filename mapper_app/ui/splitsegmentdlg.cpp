#include "splitsegmentdlg.h"
#include "ui_splitsegmentdlg.h"

bool compareSegmentInfo(const SegmentInfo & s1, const SegmentInfo & s2)
{
    return s1.description < s2.description;
}


SplitSegmentDlg::SplitSegmentDlg(int segmentId, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SplitSegmentDlg)
{
 ui->setupUi(this);
 ui->cbSegments->clear();
 cbSegmentInfoList = SQL::instance()->getSegmentInfo();
 qSort(cbSegmentInfoList.begin(), cbSegmentInfoList.end(),compareSegmentInfo);
 //foreach (segmentInfo sI in cbSegmentInfoList)
 for(int i=0; i < cbSegmentInfoList.count(); i++)
 {
  SegmentInfo si = cbSegmentInfoList.at(i);
  ui->cbSegments->addItem(si.toString(), si.segmentId);
 }
 setSegment(segmentId);
 ui->dateFrom1->setEnabled(false);
 ui->dateTo2->setEnabled(false);

 connect(ui->cbSegments, &QComboBox::currentTextChanged, [=]{
  si = cbSegmentInfoList.at(ui->cbSegments->currentIndex());
  setSegment(si.segmentId);
 });


 QStringList routeTypes = QStringList() << "Surface" << "Surface PRW" << "Rapid Transit" << "Subway" << "Rail"  << "Incline" << "Other";
 ui->cbTrackType1->addItems(routeTypes);
 ui->cbTrackType2->addItems(routeTypes);

 connect(ui->dateFrom1, &QDateTimeEdit::dateChanged, [=]{

 });
 connect(ui->dateTo1, &QDateTimeEdit::dateChanged, [=]{
  ui->dateFrom2->setDate(ui->dateTo1->date().addDays(1));
 });

 connect(ui->dateFrom2, &QDateTimeEdit::dateChanged, [=]{
  ui->dateTo1->setDate(ui->dateTo1->date().addDays(-1));
 });

 connect(ui->btnCancel, &QPushButton::clicked, [=]{
  reject();
 });
 connect(ui->btnOK, &QPushButton::clicked, [=]{
  if(processChanges())
  accept();
 });

}


SplitSegmentDlg::~SplitSegmentDlg()
{
 delete ui;
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
 ui->txtNewSegmentName2->setText(si.description);

 setupDates(si);
 ui->cbTrackType1->setCurrentIndex(si.routeType);
 ui->cbTrackType2->setCurrentIndex(si.routeType);

}

bool SplitSegmentDlg::processChanges()
{

 SegmentInfo siNew = SegmentInfo(si);
 si.endDate = ui->dateTo1->date().toString("yyyy/MM/dd");
 siNew.startDate = ui->dateFrom2->date().toString("yyyy/MM/dd");
 si.routeType = (RouteType)ui->cbTrackType1->currentIndex();
 siNew.routeType = (RouteType)ui->cbTrackType2->currentIndex();
 si.description = ui->txtNewSegmentName1->text();
 siNew.description = ui->txtNewSegmentName2->text();

 // find all the routes using this segment
 QList<RouteData> routes = SQL::instance()->getRouteSegmentsBySegment(si.segmentId);

 // update the database with changes
 SQL::instance()->BeginTransaction("splitSegment");

 bool exists = false;
 int newSegmentId = SQL::instance()->addSegment(siNew.description, siNew.oneWay, siNew.tracks, siNew.routeType, siNew.pointList, &exists);
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

 foreach(RouteData rd, routes)
 {
  if(rd.lineKey != si.segmentId)
   continue;
  if(si.endDate <= rd.endDate.toString("yyyy/MM/dd") && si.startDate >= rd.startDate.toString("yyyy/MM/dd"))
  {
   if(!SQL::instance()->updateRouteDate(si.segmentId, rd.startDate.toString("yyyy/MM/dd"), si.endDate))
   {
    SQL::instance()->RollbackTransaction("splitSegment");
    return false;
   }
   siNew.endDate = rd.endDate.toString("yyyy/MM/dd");
   if(!SQL::instance()->addSegmentToRoute(rd, siNew))
   {
    SQL::instance()->RollbackTransaction("splitSegment");
    return false;
   }
   continue;
  }
  if(rd.startDate.toString("yyyy/MM/dd") > si.endDate)
  {
   if(!SQL::instance()->updateRoute(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"), newSegmentId, rd.next, rd.prev))
   {
    SQL::instance()->RollbackTransaction("splitSegment");
    return false;
   }
  }
 }
 SQL::instance()->CommitTransaction("splitSegment");
 return true;
}
