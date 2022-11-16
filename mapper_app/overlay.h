#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include "configuration.h"
#include <QtXml>
#include "data.h"
#include "filedownloader.h"

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
  urls = other.urls;
  wmtsUrl = other.wmtsUrl;
  _year = other._year;
 }

 bool operator==(const Overlay &ov)
 {
  if(this->name == ov.name && this->source == ov.source && ov.cityName == this->cityName) {return true;}
  return false;
 }
 static bool importXml(QString);
 static bool exportXml(QString, QList<Overlay *> overlayList);
 void getTileMapResource();

 signals:
 void xmlFinished();
public:

 //qint32 id;
 QString cityName;
 QString name;
 QString description;
 qint32 opacity;
 int minZoom;
 int maxZoom;
 QString source;
 bool bLocal;
 QString sCenter; // longitude, latitude, zoom level
 QStringList urls;
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
  urls = other.urls;
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

 private slots:
void processTileMapResource();

 friend class Configuration;
};
Q_DECLARE_METATYPE(Overlay)
#endif // OVERLAY_H
