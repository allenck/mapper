#include "city.h"
#include "configuration.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "webviewbridge.h"

City::City(QObject *parent) : QObject(parent)
{

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
