#include "overlaytablemodel.h"
#include "configuration.h"
#include "sql.h"
#include <QTextDocument>
#include <overlay.h>

OverlayTableModel::OverlayTableModel(int cityId, QObject *parent) : QAbstractTableModel(parent)
{
 config = Configuration::instance();
 currCityId = cityId;
 overlayMap = new QMap<QString, Overlay*>();
 for(Overlay* ov : config->overlayMap->values())
 {
  if(ov->opacity > 65)
   ov->opacity = 65;
   overlayMap->insert(ov->cityName + "|" + ov->name, ov);
 }
}

int OverlayTableModel::rowCount(const QModelIndex &parent) const
{
 return overlayMap->count();
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
  case CITYNAME:
   return tr("City");
  case BOUNDS:
   return tr("Bounds");
  case SOURCE:
   return tr("Source");
  case YEAR:
   return tr("Year");
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
 Overlay* ov = overlayMap->values().at(row);
 City* c = config->cityList.at(currCityId);
// if(!ov->bounds().contains(c->center) && SQL::distance(ov->bounds().center(), c->center) > 10)
// {
//  qDebug() << c->name << " center: " << c->center.toString();
//  qDebug() << ov->name << " bounds: " << ov->bounds().toString() << "\n";
//  return  0;
// }

 if(index.column() == SELECTED )
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
 if( index.column() == NAME || index.column() == DESCRIPTION
    || index.column() == CITYNAME || index.column() == YEAR
    || index.column() == MINZOOM || index.column() == MAXZOOM || index.column() == URLS)
 {
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
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
 int row = index.row();
 Overlay* ov = overlayMap->values().at(row);
 if(role == Qt::ToolTipRole)
 {
  if(index.column() == DESCRIPTION)
   return ov->description;
  if(index.column() == BOUNDS)
   return ov->bounds().toString();
 }

 if(role == Qt::BackgroundRole)
 {
  QVariant background = QVariant();
  switch(index.column())
  {
   case NAME:
   if(ov->name.isEmpty())
   {
    background = QVariant( QColor(Qt::red) );
   }
   break;
  case CITYNAME:
   if(ov->cityName.isEmpty() )
   {
    background = QVariant( QColor(Qt::red) );
    break;
   }
   if(!config->cityNames().contains(ov->cityName))
   {
    background = QVariant( QColor(Qt::yellow) );
    break;

   }
   break;
  case OPACITY:
   if(ov->opacity > 65) background = QVariant( QColor(Qt::red) );
   break;
  case BOUNDS:
   if(!ov->bounds().isValid())
    background = QVariant( QColor(Qt::red) );
   break;
//  case CENTER:
//   if(!ov->center().isValid())
//    background = QVariant( QColor(Qt::red) );
//   break;
  case MINZOOM:
   if(ov->minZoom < 1) background = QVariant( QColor(Qt::red) );
   break;
  case MAXZOOM:
   if(ov->maxZoom > 21 || ov->maxZoom <1) background = QVariant( QColor(Qt::red) );
   break;
  case URLS:
   if(ov->urls.isEmpty()) background = QVariant( QColor(Qt::red) );
   break;
  }
  return background;
 }
 if(role == Qt::DisplayRole)
 {
  switch(index.column()) {
  case NAME:
   return ov->name;
   break;
  case CITYNAME:
   return ov->cityName;
  case DESCRIPTION:
  {
   QString text = ov->description;
   if(!text.isEmpty())
   {
      QTextDocument td(text);
      return td.toPlainText();
   }
   else
    return "";
  }
  case BOUNDS:
     return ov->bounds().isValid()?"valid":"invalid";
//  case CENTER:
//   return ov->center().isValid()?"valid":"invalid";
  case MINZOOM:
   return ov->minZoom;
  case MAXZOOM:
   return ov->maxZoom;
  case OPACITY:
   return ov->opacity;
//  case LOCAL:
//   return overlayList.at(row)->source == "mbtiles" || overlayList.at(row)->source == "tileserver";
  case SOURCE:
      return ov->source;
  case URLS:
   if(ov->urls.count())
      return ov->urls.at(0);
   else return "";
  case YEAR:
   return ov->year();
  default:
   break;
  }
 }
 if(role == Qt::CheckStateRole)
 {
  QString name = overlayMap->values().at(index.row())->name;
  Overlay* ov = overlayMap->values().at(index.row());
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
 Overlay* ov = overlayMap->values().at(index.row());
 QString oldName =ov->cityName +"." + ov->name;
 QString newName;
 if(role == Qt::CheckStateRole)
 {
  if(index.column() == SELECTED)
  {
    ov->isSelected = value.toBool();
    emit overlaySelectionChanged(ov, ov->isSelected);
    setDirty();
  }
  if(index.column() == LOCAL)
  {
   if(value.toBool())
   {
#ifdef WIN32
    overlayMap->values().at(index.row())->source = "mbtiles";
#else
    overlayMap->values().at(index.row())->source = "tileserver";
#endif
   }
   else
    overlayMap->values().at(index.row())->source = "acksoft";
  }
  return true;
 }
 if(role == Qt::EditRole)
 {
  oldName = ov->cityName +"." + ov->name;
  QString oldCityKey =ov->name;
  newName = ov->cityName +"." + ov->name;
  QString newCityKey = ov->name;
  if(index.column() == NAME)
  {
   overlayMap->remove(oldName);
   ov->name = newCityKey =value.toString();
   newName = ov->cityName + "." +ov->name;
  }
  if(index.column() == CITYNAME)
  {
   newName = value.toString()+"."+ov->name;
   newCityKey = value.toString();
   newName = value.toString()+"." + ov->name;
   QStringList cityList = config->cityNames();
   if(!cityList.contains(newCityKey))
    return false; // invalid city name
   if(overlayMap->contains(newName))
    return false;
   overlayMap->remove(oldName);
   ov->cityName = value.toString();
   overlayMap->insert(newName,ov);
   return true;
  }
  if(index.column() == DESCRIPTION)
  {
   ov->description = value.toString();
  }
  if(index.column()== YEAR)
  {
   ov->setYear(value.toString());
  }
  if(index.column() == URLS)
  {
   QString text = value.toString();
   if(text.contains(","))
   {
    QStringList sl = text.split(",");
    for(QString _url : sl){
     QUrl url(_url);
     if(!url.isValid())
      return false;
    }
    ov->urls = sl;
   }
   else
   {
    QStringList sl;
    sl.append(text);
    QUrl url(text);
    if(!url.isValid())
     return false;
    ov->urls = sl;
   }
  }
  if(index.column() == MINZOOM)
   ov->minZoom = value.toInt();
  if(index.column() == MAXZOOM)
   ov->maxZoom = value.toInt();

  emit overlayChanged(oldName, newName, ov);
  if(oldName == newName)
  {
   // do nothing if neither name or cityName changed!
  }
  else
  {
   if(oldName != newName)
   {
    overlayMap->remove(oldName);
    overlayMap->insert(newName, ov);
   }
   else
   {
    config->currCity->city_overlayMap->remove(oldCityKey);
    config->currCity->city_overlayMap->insert(newCityKey, ov);
   }
  }
  setDirty();
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
 if(!overlayMap->contains(ov->cityName +"|" + ov->name))
 {
  beginResetModel();
  overlayMap->insert(ov->cityName +"|" + ov->name, ov);
  endResetModel();
 }
}

QMap<QString, Overlay*>* OverlayTableModel::getOverlayMap()
{
 return overlayMap;
}

void OverlayTableModel::deleteRow(int row)
{
 Overlay* ov = overlayMap->values().at(row);
 beginRemoveRows(QModelIndex(), row, row);
 overlayMap->remove(ov->cityName+"|"+ov->name);
 endRemoveRows();
}

Overlay* OverlayTableModel::selectedOverlay(int row)
{
 return overlayMap->values().at(row);
}

