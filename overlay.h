#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include "configuration.h"
#include <QtXml>
#include "data.h"
#include "filedownloader.h"
#include "qlist.h"
#include <QUrl>
class Overlay : public QObject
{
  Q_OBJECT

public:
  Overlay(QObject* parent = nullptr);
  Overlay(QString cityName, QString name, int opacity = 65);
  ~Overlay() {}
  Overlay(const Overlay& other) {
  cityName = other.cityName;
  name = other.name;
  description  = other.description;
  opacity = other.opacity;
  minZoom = other.minZoom;
  maxZoom = other.maxZoom;
  source = other.source;
  _bounds = other._bounds; // west longitude, south Latitude, east longitude, north latitude
  sCenter = other.sCenter; // longitude, latitude, zoom level
  _center = other._center;
  //_urls = other._urls;
  _url = other._url;
  wmtsUrl = other.wmtsUrl;
  _year = other._year;
  layerName = other.layerName;
 }
 //  QStringList urls() {return _urls;}
 // void setUrls(QStringList urls){
 //     if(urls.isEmpty())
 //         qWarning() << "urls is empty!";
 //     _urls = urls;
 // }

 bool operator==(const Overlay &ov)
 {
  if(this->name == ov.name && this->source == ov.source && ov.cityName == this->cityName) {return true;}
  return false;
 }
 static bool importXml(QString);
 static bool exportXml(QString, QList<Overlay *> overlayList);
 static QList<Overlay*> getList(City* city =nullptr);
 void getTileMapResource();
 QString url(){return _url;}
 void setUrl(QString txt){
     _url = txt;
     // if the url contains x,y,z templates replace them in a temp string with '0' to insure that rest of url validates
     if(txt.contains('{'))
         txt=txt.replace("{x}", "0").replace("{y}", "0").replace("{z}", "0");
     _qurl.setUrl(txt);
     Q_ASSERT(_qurl.isValid());
 }
 signals:
 void xmlFinished();
public:

 //qint32 id;
 QString cityName;
 QString name;
 QString layerName;
 QString description;
 qint32 opacity;
 int minZoom;
 int maxZoom;
 QString source;
 bool bLocal;
 QString sCenter; // longitude, latitude, zoom level
 //QStringList urls;
 QString wmtsUrl;
 bool isSelected = false;

 Overlay operator=(const Overlay& other)
 {
  cityName = other.cityName;
  name = other.name;
  description  = other.description;
  opacity = other.opacity;
  minZoom = other.minZoom;
  maxZoom = other.maxZoom;
  source = other.source;
  _bounds = other._bounds; // west longitude, south Latitude, east longitude, north latitude
  sCenter = other.sCenter; // longitude, latitude, zoom level
  _center = other._center;
  //urls = other.urls;
  _url = other._url;
  wmtsUrl = other.wmtsUrl;
  _year = other._year;
 }
 QString year() {return _year;}
 void setYear(QString year){_year = year;}
// LatLng center() {return _center;}
// void setCenter(LatLng center){_center = center;}
 Bounds bounds(){return _bounds;}
 LatLng setBounds(Bounds bounds) {_bounds = bounds; return _bounds.center();}
 bool checkValid();

 private:
 static QList<Overlay*> overlayList;
 QString _year;
 LatLng _center;
 Bounds _bounds; // west longitude, south Latitude, east longitude, north latitude
 FileDownloader* m_tilemapresource;
 //QStringList _urls;
 QUrl _qurl;
 QString _url;

 private slots:
    void processTileMapResource();

 friend class Configuration;
};
Q_DECLARE_METATYPE(Overlay)
#endif // OVERLAY_H
