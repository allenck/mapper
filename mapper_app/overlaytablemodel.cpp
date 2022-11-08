#include "overlaytablemodel.h"
#include "configuration.h"
#include "sql.h"

OverlayTableModel::OverlayTableModel(int cityId, QObject *parent) : QAbstractTableModel(parent)
{
 config = Configuration::instance();
 currCityId = cityId;
 overlayMap = QMap<QString, Overlay*>();
 foreach(Overlay* ov, config->overlayList.values())
 {
  overlayMap.insert(ov->name, ov);
  ov->isSelected = config->currCity->overlayMap.contains(ov->name);
 }
}

int OverlayTableModel::rowCount(const QModelIndex &parent) const
{
 return overlayMap.count();
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
 Overlay* ov = overlayMap.values().at(row);
 City* c = config->cityList.at(currCityId);
 if(!ov->bounds.contains(c->center) && SQL::distance(ov->bounds.center(), c->center) > 10)
 {
  qDebug() << c->name << " center: " << c->center.toString();
  qDebug() << ov->name << " bounds: " << ov->bounds.toString() << "\n";
  return  0;
 }

 if(index.column() == SELECTED  || index.column() == NAME || index.column() == DESCRIPTION)
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
   return overlayMap.values().at(row)->name;
   break;
  case DESCRIPTION:
   return overlayMap.values().at(row)->description;
  case MINZOOM:
   return overlayMap.values().at(row)->minZoom;
  case MAXZOOM:
   return overlayMap.values().at(row)->maxZoom;
  case OPACITY:
   return overlayMap.values().at(row)->opacity;
//  case LOCAL:
//   return overlayList.at(row)->source == "mbtiles" || overlayList.at(row)->source == "tileserver";
  case SOURCE:
      return overlayMap.values().at(row)->source;
  case URLS:
      return overlayMap.values().at(row)->urls.at(0);

  default:
   break;
  }
 }
 if(role == Qt::CheckStateRole)
 {
  QString name = overlayMap.values().at(index.row())->name;
  Overlay* ov = overlayMap.values().at(index.row());
  if(index.column() == SELECTED)
  {
   if(ov->isSelected)
    return Qt::Checked;
   else
    return Qt::Unchecked;
  }
 }
 return QVariant();
}

bool OverlayTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 Overlay* ov = overlayMap.values().at(index.row());
 if(role == Qt::CheckStateRole)
 {
  if(index.column() == SELECTED)
  {
    ov->isSelected = value.toBool();
   emit overlaySelectionChanged(index, value.toBool());
  }
  if(index.column() == LOCAL)
  {
   if(value.toBool())
   {
#ifdef WIN32
    overlayMap.values().at(index.row())->source = "mbtiles";
#else
    overlayMap.values().at(index.row())->source = "tileserver";
#endif
   }
   else
    overlayMap.values().at(index.row())->source = "acksoft";
  }
  return true;
 }
 if(role == Qt::EditRole)
 {
  if(index.column() == NAME)
  {
   ov->name = value.toString();
  }
  if(index.column() == DESCRIPTION)
  {
   ov->description = value.toString();
  }
  return true;
 }
 return false;
}

void OverlayTableModel::setCity(int c)
{
 currCityId = c;
}

void OverlayTableModel::addOverlay(Overlay* ov)
{
 if(!overlayMap.contains(ov->name))
 {
  beginResetModel();
  overlayMap.insert(ov->name, ov);
  endResetModel();
 }
}
