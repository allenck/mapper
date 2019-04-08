#include "overlaytablemodel.h"
#include "configuration.h"

OverlayTableModel::OverlayTableModel()
{
 config = Configuration::instance();
 currCityId = config->currentCityId;
 _selected = new QHash<QString, Overlay*>();
 overlayList = QList<Overlay*>(config->overlayList.values());
 foreach(Overlay* ov, config->cityList.at(currCityId)->overlays)
 {
  foreach(Overlay* ov1, overlayList)
  {
   if(ov->name == ov1->name)
   {
    _selected->insert(ov->name, ov);
   }
  }
 }
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
 if(index.column() == SELECTED )
 {
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
 }
 if(index.column() == LOCAL)
 {
  if(overlayList.at(index.row())->bLocal)
   return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
  else
   return Qt::NoItemFlags;
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
//  case SELECTED:
//   return _selected->contains(overlayList.at(row)->name);
//  case LOCAL:
//   return overlayList.at(row)->source == "mbtiles" || overlayList.at(row)->source == "tileserver";
  default:
   break;
  }
 }
 if(role == Qt::CheckStateRole)
 {
  QString name = overlayList.at(index.row())->name;
  if(index.column() == SELECTED)
  {
   if(_selected->contains(name))
   {
    overlayList.at(index.row())->source = _selected->value(name)->source;
    return Qt::Checked;
   }\
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
 if(role == Qt::CheckStateRole)
 {
  if(index.column() == SELECTED)
  {
   QString name = overlayList.at(index.row())->name;
   if(value.toBool())
   {
    if(!_selected->contains(name))
     _selected->insert(name, overlayList.at(index.row()));
   }
   else
   {
    if(_selected->contains(name))
     _selected->remove(name);
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

void OverlayTableModel::setCity(int c, QHash<QString, Overlay*>* hash)
{
 currCityId = c;
 _selected = hash;
}

