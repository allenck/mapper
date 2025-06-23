#include "queryeditmodel.h"
#include "mainwindow.h"

QueryEditModel::QueryEditModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent, db)
{

}

QueryEditModel::~QueryEditModel(){
 if(isDirty())
  submit();
}

QVariant QueryEditModel::data ( const QModelIndex & index, int role ) const
{
 if (!index.isValid())
     return QVariant();

 // let the base class handle all other cases
 return QSqlTableModel::data( index, role );
}

bool QueryEditModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 QVariant hdr;
 if(role == Qt::EditRole)
 {
  hdr = headerData(index.column(), Qt::Horizontal, Qt::DisplayRole);
 }
 bool rslt =  QSqlTableModel::setData( index, value, role );

 if(table.compare("routes", Qt::CaseInsensitive) ==0)
 {
  QSqlRecord record = this->record(index.row());
  SegmentData sd;
  sd.setRoute(record.value("Route").toInt());
  sd.setRouteName(record.value("Name").toString());
  sd.setStartDate(record.value("StartDate").toDate());
  sd.setEndDate(record.value("EndDate").toDate());
  sd.setSegmentId(record.value("LineKey").toInt());
  sd.setOneWay(record.value("OneWay").toString());
  sd.setTrackUsage(record.value("TrackUsage").toString());
  sd.setCompanyKey(record.value("CompanyKey").toInt());
  sd.setTractionType(record.value("TractionType").toInt());
  NotifyRouteChange rc = NotifyRouteChange(SQL::MODIFYSEG, &sd);
  emit routeChange(rc);

  MainWindow::instance()->refreshRoutes();
 }

 return rslt;
}

void QueryEditModel::setTabName(QString name)
{
 tabName = name;
}

QVariant QueryEditModel::edit(const QModelIndex &index) const
{
 return QVariant();
}

void QueryEditModel::setTable(const QString& table)
{
 this->table = table;
 QSqlTableModel::setTable(table);
}
