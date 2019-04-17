#ifndef CITY_H
#define CITY_H

#include <QObject>
#include "latlng.h"

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
 bool bAlphaRoutes;
 bool bNoPanOpt;
 bool bGeocoderRequest;
 qint32 lastRoute = 0;
 QString lastRouteName;
 QString lastRouteEndDate;
 bool bDisplayStationMarkers;
 bool bDisplayTerminalMarkers;
 bool bDisplayRouteComments;
 bool bShowOverlay;
 qint32 companyKey;
 qint32 routeSortType = 0;
 QString savedClipboard;
 bool bUserMap;
 QList<Overlay*> overlayList();
 void addOverlay(Overlay* ov);
 void removeOverlay(Overlay* ov);
 bool isDirty();
 void setDirty(bool = true);
signals:

public slots:

private:
 QList<Overlay*> overlays;
 bool bDirty;
};

#endif // CITY_H
