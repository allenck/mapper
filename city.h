#ifndef CITY_H
#define CITY_H

#include <QObject>
#include "latlng.h"
#include "data.h"
#include "exceptions.h"
#include <QUuid>

class Overlay;
class Connection;
class WebViewBridge;
class City : public QObject
{
 Q_OBJECT
public:
 explicit City(QObject *parent = nullptr);
    qint32 id =0;
 QList<Connection*> connections;
 QT_DEPRECATED QStringList connectionNames;
 QMap<QString, Connection*> connectionMap; // connections by Uuid
 QMap<QString, Connection*> connectionMap2; // connections by description
 QMap<Connection*, QString> connectionMap3; // descriptions by connection
 qint32 curConnectionId = 0;
 qint32 curExportConnId =-1;
 qint32 curOverlayId =0;
 LatLng center;
 QString mapType = "roadmap";
 qint32 zoom =8;
 bool bAlphaRoutes = true;
 bool bNoPanOpt = false;
 bool bGeocoderRequest = false;
 qint32 lastRoute = 0;
 QString lastRouteName;
 QString lastRouteEndDate;
 bool bDisplayStationMarkers = false;
 bool bDisplayTerminalMarkers = false;
 bool bDisplayRouteComments = false;
 bool bDisplayRoutesForSelectedCompanies = false;
 bool bShowOverlay = false;
 qint32 companyKey=-1;
 //QString selectedCompanies; // comma separated list of companies selected to show in routes
 QList<int> selectedCompaniesList;
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
 //QList<QPair<QString,QString>> abbreviationsList;

 void setNameOverride(QString name)
 {
     _name = name;
 }
 QString name() {return _name;}
 void setCenter(LatLng center);
 void setId(int id);
 void setConnectionUniqueId(QUuid connectionUniqueId){_connectionUniqueId = connectionUniqueId;}
 QUuid connectionUniqueId() {return _connectionUniqueId;}
 void addConnection(Connection*connection);
 private:
 Bounds _bounds;
 QUuid _connectionUniqueId;

signals:
 void connectionAdded(Connection* connection);

public slots:

private:
 bool bDirty;
 QString _name;

};

#endif // CITY_H
