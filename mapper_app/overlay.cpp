#include "overlay.h"
#include <QFile>
#include <QTextStream>
#include "exceptions.h"
#include "configuration.h"

Overlay::Overlay(QObject *parent) : QObject(parent)
{
 source="acksoft";
 bLocal = false;
 _bounds = Bounds();
}

Overlay::Overlay(QString cityName, QString name, int opacity)
{
 this->cityName = cityName;
 this->name = name;
 this->opacity = opacity;
 source="acksoft";
 bLocal = false;
 minZoom = 8;
 maxZoom = 16;
 _bounds = Bounds();
}


bool Overlay::importXml(QString fileName)
{
 QDir cwd = QDir::currentPath();
 QFile* file = new QFile(fileName);
 if(file->open(QIODevice::ReadOnly))
 {
  QDomDocument doc("Overlay");
  doc.setContent(file->readAll());
  QDomElement root = doc.documentElement();
  QDomNodeList list = root.elementsByTagName("overlay");
  for(int i=0; i < list.count(); i++)
  {
   QDomElement elem = list.at(i).toElement();
   Overlay* ov = new Overlay();
   ov->name = elem.attribute("name");
   ov->cityName = elem.attribute("cityName");
   //ov->description = elem.attribute("description");
   QDomElement description = elem.firstChildElement("description");
   ov->description = description.text();
   ov->opacity = elem.attribute("opacity", "65").toInt();
   ov->minZoom = elem.attribute("minZoom", "8").toInt();
   ov->maxZoom = elem.attribute("maxZoom", "20").toInt();
   ov->source = elem.attribute("source");
   ov->_year = elem.attribute("year");
   ov->isSelected = elem.attribute("selected")=="yes";
   double westLongitude, southLatitude, eastLongitude, northLatitude;
   westLongitude = elem.attribute("westLongitude").toDouble();
   southLatitude = elem.attribute("southLatitude").toDouble();
   eastLongitude = elem.attribute("eastLongitude").toDouble();
   northLatitude = elem.attribute("northLatitude").toDouble();
   ov->_bounds = Bounds(LatLng(southLatitude, westLongitude), LatLng(northLatitude, eastLongitude));
//   if(ov->bounds().isValid())
//   {
//     ov->setCenter(ov->bounds().center());
//     qDebug() << "center: "<< ov->center().toString();
//   }
   QDomElement wmtsUrl = elem.firstChildElement("wmtsUrl");
   ov->wmtsUrl = wmtsUrl.text();
   QDomElement urls = elem.firstChildElement("url");
//   QStringList urlList;
//   for(int i =0; i < urls.count(); i++)
//   {
//    QDomElement url = urls.at(i).toElement();
//    urlList.append(url.text());
//   }
   if(urls.text().isEmpty())
   {
    if(ov->source == "acksoft" && ov->urls.isEmpty())
     ov->urls.append("http://ubuntu-2:1080/public/map_tiles/");
    if(ov->source == "mbtiles"&& ov->urls.isEmpty()) // Windows
     ov->urls.append("http://localhost/map_tiles/mbtiles.php");
    if(ov->source == "tileserver" && ov->urls.isEmpty()) // Linux
     ov->urls.append("http://localhost/tileserver.php");
   }
   else
    ov->urls = urls.text().split(",");

   overlayList.append(ov);
  }
  file->close();
 }
 else{
  qDebug() << tr("error importing %1 %2").arg(fileName).arg(file->errorString());
  throw FileNotFoundException(tr("error importing %1 %2").arg(fileName).arg(file->errorString()));
 }
 return true;
}
QList<Overlay*> Overlay::overlayList = QList<Overlay*>();

bool Overlay::exportXml(QString fileName, QList<Overlay*> overlayList)
{
 QFile* file = new QFile(fileName);
 if(file->open(QIODevice::WriteOnly))
 {

  QDomDocument doc("Overlays");
  QDomElement root = doc.createElement("Overlays");
  doc.appendChild(root);
  for(Overlay* ov : overlayList)
  {
//   if(ov->source != "georeferencer")
//    continue;
   QDomElement overlay = doc.createElement("overlay");
   overlay.setAttribute("name", ov->name);
   overlay.setAttribute("cityName", ov->cityName);
   //overlay.setAttribute("description", ov->description);
   QDomElement description = doc.createElement("description");
   description.appendChild(doc.createTextNode(ov->description));
   overlay.appendChild(description);
   overlay.setAttribute("opacity", ov->opacity);
   overlay.setAttribute("minZoom", ov->minZoom);
   overlay.setAttribute("maxZoom", ov->maxZoom);
   overlay.setAttribute("source", ov->source);
   overlay.setAttribute("year", ov->_year);
   overlay.setAttribute("selected", (ov->isSelected)?"yes":"no");
   QDomElement bounds = doc.createElement("bounds");
   bounds.setAttribute("southLatitude", QString::number(ov->_bounds.swPt().lat()));
   bounds.setAttribute("westLongitude", QString::number(ov->_bounds.swPt().lon()));
   bounds.setAttribute("northLatitude", QString::number(ov->_bounds.nePt().lat()));
   bounds.setAttribute("eastLongtude", QString::number(ov->_bounds.nePt().lon()));
   overlay.appendChild(bounds);
   QDomElement wmtsUrl = doc.createElement("wmtsUrl");
   wmtsUrl.appendChild(doc.createTextNode(ov->wmtsUrl));
   overlay.appendChild(wmtsUrl);
   QDomElement url = doc.createElement("url");
//   foreach(QString sUrl, ov->urls)
//   {
//    url.appendChild(doc.createTextNode(sUrl));
//   }
   url.appendChild(doc.createTextNode(ov->urls.join(",")));
   overlay.appendChild(url);

   root.appendChild(overlay);
  }
  QString text = doc.toString(2);
  QTextStream stream(file);
  stream << text;
  file->close();
  return true;
 }
 else{
  throw Exception(tr("error exporting %1 %2").arg(fileName).arg(file->errorString()));
 }

 return false;
}

void Overlay::getTileMapResource()
{
 QEventLoop loop;
 QUrl url = QUrl::fromUserInput(QString("http://ubuntu-2:1080/public/map_tiles/%1/tilemapresource.xml").arg(name));
 if(url.isValid())
  m_tilemapresource = new FileDownloader(url);
 else
 {
  return;
 }
 m_tilemapresource->setOverlay(this);
 connect(m_tilemapresource, SIGNAL(downloaded(QString)), this, SLOT(processTileMapResource()));
 loop.exec();

}

void Overlay::processTileMapResource()
{
 QString str = m_tilemapresource->downloadedData();
 if(str != "")
 {
  QDomDocument doc;
  QString title;
  Bounds bounds;
  doc.setContent(str);
  QDomElement root = doc.documentElement();
  if(root.tagName() == "TileMap")
  {
   QDomElement elem = root.firstChildElement("Title");
   if(!elem.isNull())
    title = elem.text();
   elem = root.firstChildElement("Abstract");
   if(!elem.isNull())
    description = elem.text();
   elem = root.firstChildElement("BoundingBox");
   if(!elem.isNull())
   {
    double minx, miny, maxx, maxy;
    minx = elem.attribute("minx").toDouble();
    miny = elem.attribute("miny").toDouble();
    maxx = elem.attribute("maxx").toDouble();
    maxy = elem.attribute("maxy").toDouble();
    bounds = Bounds(LatLng(miny, minx), LatLng(maxy, maxx));
    if(bounds.isValid())
    {
     //Overlay* ov = m_tilemapresource->overlay();
     setBounds(bounds);
    }
   }
   elem = root.firstChildElement("TileSets");
   if(!elem.isNull())
   {
    QDomNodeList nl = elem.elementsByTagName("TileSet");
    int minZoom = 30;
    int maxZoom = 0;
    int zoom;
    for(int i=0; i < nl.count(); i++)
    {
     bool bok;
     zoom = nl.at(i).toElement().attribute("href").toInt(&bok);
     if(bok)
     {
      if(zoom < minZoom)
       minZoom = zoom;
      if(zoom > maxZoom)
       maxZoom = zoom;
     }
    }
    if(minZoom != 30)
     this->minZoom =  minZoom;
    if(maxZoom > 0)
     this->maxZoom = maxZoom;
   }
   qDebug() <<"xml processed: " << name << "descr: " << description << " bounds: " << bounds.toString() << " minZoom: " << minZoom << " maxZoom: " << maxZoom;
  }
  emit xmlFinished();

 }
}

// check validity of various fields
bool Overlay::checkValid()
{
 if(name.isEmpty())return false;
 if(cityName.isEmpty()) return false;
 if(opacity > 65) opacity = 65;
 bool b = bounds().isValid();
 if(!bounds().isValid()) return false;
 //if(!center().isValid()) return false;
 if(minZoom < 1) return false;
 if(maxZoom > 21) return false;
 if(urls.isEmpty()) return false;
 return true;
}
