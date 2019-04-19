#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include "data.h"
#include <QSettings>
#include <iostream>
#include <QDebug>
#include "settingsdb.h"
#include "city.h"
#include "connection.h"

class Configuration;
class SQL;

class Overlay
{
public:
 //qint32 id;
 QString name;
 QString description;
 qint32 opacity;
 int minZoom;
 int maxZoom;
 QString source;
 bool bLocal;
 Bounds bounds; // west longitude, south Latitude, east longitude, north latitude
 QString sCenter; // longitude, latitude, zoom level
 QStringList urls;
 Overlay()
 {
  source="acksoft";
  bLocal = false;
  bounds = Bounds();
 }

 Overlay(QString name, int opacity = 65)
 {
  this->name = name;
  this->opacity = opacity;
  source="acksoft";
  bLocal = false;
  minZoom = 8;
  maxZoom = 16;
  bounds = Bounds();
 }

 bool operator==(const Overlay &ov)
 {
  if(this->name == ov.name && this->source == ov.source) {return true;}
  return false;
 }
};


struct query
{
 bool b_stop_query_on_error;
 bool b_sql_execute_after_loading;
 QString s_query_path;
 QByteArray geometry;
};

class Configuration : public QObject
{
 Q_OBJECT
public:
 City* currCity;
 Connection* currConnection;
 void getSettings();
 void saveSettings();
 void setOverlay(Overlay* ov);
 QList<City*> cityList;
 QMap<QString, Overlay*> overlayList;
 qint32 currentCityId;
 query q;
 static Configuration* instance();
 bool bDisplayWebDebug;
 bool bRunInBrowser;
 QStringList localOverlayList;
 QStringList georeferencedList;
 QString path;

private:
 static Configuration* _instance;
 explicit Configuration(QObject *parent = 0);

signals:

public slots:

};

#endif // CONFIGURATION_H
