#ifndef CITY_H
#define CITY_H

#include <QObject>
#include "latlng.h"
#include "data.h"
#include "exceptions.h"
class Overlay;
class Connection;
class WebViewBridge;
class City : public QObject
{
 Q_OBJECT
public:
 explicit City(QObject *parent = nullptr);
 qint32 id;
 QList<Connection*> connections;
 qint32 curConnectionId = 0;
 qint32 curExportConnId;
 qint32 curOverlayId;
 LatLng center;
 QString mapType;
 qint32 zoom;
 bool bAlphaRoutes = false;
 bool bNoPanOpt = false;
 bool bGeocoderRequest = false;
 qint32 lastRoute = 0;
 QString lastRouteName;
 QString lastRouteEndDate;
 bool bDisplayStationMarkers = false;
 bool bDisplayTerminalMarkers = false;
 bool bDisplayRouteComments = false;
 bool bShowOverlay = false;
 qint32 companyKey;
 qint32 routeSortType = 0;
 QString savedClipboard;
 bool bUserMap = false;
 QMap<QString, Overlay*>* city_overlayMap = new QMap<QString, Overlay*>();
 void addOverlay(Overlay* ov);
 void removeOverlay(Overlay* ov);
 bool isDirty();
 void setDirty(bool = true);
 //QList<Overlay*> selectedOverlays() {return overlays;}
 Bounds bounds() {return _bounds;}
 void setBounds(Bounds bounds){_bounds = bounds;}
 void setCityBounds(WebViewBridge *m_bridge);
 void setName(QString name){
  if(_name.isEmpty())
  _name = name;
  else
   throw IllegalArgumentException("city name is protected");
 }
 QString name() {return _name;}
 private:
 Bounds _bounds;
signals:

public slots:

private:
 bool bDirty;
 QString _name;

};

#endif // CITY_H
