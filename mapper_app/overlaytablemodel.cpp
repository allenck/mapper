#include "overlaytablemodel.h"
#include "configuration.h"
#include "sql.h"

OverlayTableModel::OverlayTableModel()
{
 config = Configuration::instance();
 currCityId = config->currentCityId;
 overlayList = QList<Overlay*>(config->overlayList.values());
}

int OverlayTableModel::rowCount(const QModelIndex &parent) const
{
 return overlayList.count();
}

int OverlayTableModel::columnCount(const QModelIndex &parent) const
{
 return (int)NUMCOLUMNS;
}
QVariant OverlayTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
 if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
 {
  switch(section)
  {
   case NAME:
    return tr("Overlay Name");
  case SELECTED:
    return tr("Selected");
   case DESCRIPTION:
    return tr("Description");
   case MINZOOM:
    return tr("Min Zoom");
  case MAXZOOM:
    return tr("Max Zoom");
  case OPACITY:
   return tr("Default Opacity");
  case LOCAL:
   return tr("Local file");
  default:
   break;
  }
 }
 return QVariant();
}

Qt::ItemFlags OverlayTableModel::flags(const QModelIndex &index) const
{
 int row = index.row();
 Overlay* ov = overlayList.at(row);
 City* c = config->cityList.at(currCityId);
 if(!ov->bounds.contains(c->center) && SQL::distance(ov->bounds.center(), c->center) > 10)
 {
  qDebug() << c->name << " center: " << c->center.toString();
  qDebug() << ov->name << " bounds: " << ov->bounds.toString() << "\n";
  return  0;
 }

 if(index.column() == SELECTED )
 {
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
 }
 if(index.column() == LOCAL)
 {
//  if(overlayList.at(index.row())->bLocal)
//   return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
//  else
//   return Qt::NoItemFlags;
     return Qt::ItemIsEnabled;
 }
 return Qt::ItemIsEnabled;
}

QVariant OverlayTableModel::data(const QModelIndex &index, int role) const
{
 if(role == Qt::DisplayRole)
 {
  int row = index.row();
  switch(index.column()) {
  case NAME:
   return overlayList.at(row)->name;
   break;
  case DESCRIPTION:
   return overlayList.at(row)->description;
  case MINZOOM:
   return overlayList.at(row)->minZoom;
  case MAXZOOM:
   return overlayList.at(row)->maxZoom;
  case OPACITY:
   return overlayList.at(row)->opacity;
//  case LOCAL:
//   return overlayList.at(row)->source == "mbtiles" || overlayList.at(row)->source == "tileserver";
  case SOURCE:
      return overlayList.at(row)->source;
  case URLS:
      return overlayList.at(row)->urls.at(0);

  default:
   break;
  }
 }
 if(role == Qt::CheckStateRole)
 {
  QString name = overlayList.at(index.row())->name;
  Overlay* ov = overlayList.at(index.row());
  if(index.column() == SELECTED)
  {
   if(config->cityList.at(currCityId)->overlayList().contains(ov))
    return Qt::Checked;
   else
    return Qt::Unchecked;
  }
  if(index.column() == LOCAL && overlayList.at(index.row()))
  {
   if(overlayList.at(index.row())->source == "acksoft" || overlayList.at(index.row())->source == "")
    return Qt::Unchecked;
   else
    return Qt::Checked;
  }
 }
 return QVariant();
}

bool OverlayTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 Overlay* ov = overlayList.at(index.row());
 if(role == Qt::CheckStateRole)
 {
  if(index.column() == SELECTED)
  {
   QString name = overlayList.at(index.row())->name;
   if(value.toBool())
   {
    if(!config->cityList.at(currCityId)->overlayList().contains(ov))
     config->cityList.at(currCityId)->addOverlay(ov);
   }
   else
   {
    if(config->cityList.at(currCityId)->overlayList().contains(ov))
     config->cityList.at(currCityId)->removeOverlay(ov);
   }
   emit overlaySelectionChanged(index, value.toBool());
  }
  if(index.column() == LOCAL)
  {
   if(value.toBool())
   {
#ifdef WIN32
    overlayList.at(index.row())->source = "mbtiles";
#else
    overlayList.at(index.row())->source = "tileserver";
#endif
   }
   else
    overlayList.at(index.row())->source = "acksoft";
  }
  return true;
 }
 return false;
}

void OverlayTableModel::setCity(int c)
{
 currCityId = c;
}

