#include "city.h"
#include <QMessageBox>
#include "webviewbridge.h"
#include "overlay.h"
#include "sql.h"

City::City(QObject *parent) : QObject(parent)
{
 setObjectName(name());
}
//QList<Overlay*> City::overlayList() {return overlays;}
void City::addOverlay(Overlay* ov)
{
 if(!ov->name.isEmpty())
 {
  city_overlayMap->insert(ov->name, ov);
  bDirty = true;
 }
}

void City::addConnection(Connection* connection)
{
    if(connection->description().contains("\n"))
        connection->description().remove("\n");
   if(!connectionMap.contains(connection->uniqueId().toString()))
   {
       connectionMap.insert(connection->uniqueId().toString(), connection);
       connections.append(connection);
       connection->setId(connections.count()-1);
       //curConnectionId = connections.count()-1;
       //curConnectionId = connection->id();
       _connectionUniqueId = connection->uniqueId();
   }
}

void City::removeOverlay(Overlay* ov)
{
 while(city_overlayMap->contains(ov->name))
 {
  city_overlayMap->remove(ov->name);
  bDirty = true;
 }
}

void City::setDirty(bool b) {bDirty = b;}
bool City::isDirty() {return bDirty;}

void City::setCityBounds(WebViewBridge* m_bridge)
{
 QMessageBox::question(nullptr, "Set City Bounds", "Zoom in or out until the entire 'region in the display covers the region. Then click on the Set Bounds' button");
 m_bridge->processScript("addCityBoundsButton");
}

void City::setCenter(LatLng center){this->center = center;}

void City::setId(int id){this->id = id;}
