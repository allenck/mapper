#ifndef CITY_H
#define CITY_H

#include <QObject>
#include "latlng.h"
#include "data.h"

class Overlay;
class Connection;
class City : public QObject
{
 Q_OBJECT
public:
 explicit City(QObject *parent = nullptr);
 qint32 id;
 QString name;
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
 QMap<QString, Overlay*> overlayMap;
 void addOverlay(Overlay* ov);
 void removeOverlay(Overlay* ov);
 bool isDirty();
 void setDirty(bool = true);
 //QList<Overlay*> selectedOverlays() {return overlays;}
 Bounds bounds;
signals:

public slots:

private:
 bool bDirty;
};

#endif // CITY_H
