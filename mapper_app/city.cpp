#include "city.h"
#include "configuration.h"

City::City(QObject *parent) : QObject(parent)
{

}
//QList<Overlay*> City::overlayList() {return overlays;}
void City::addOverlay(Overlay* ov)
{
 if(!ov->name.isEmpty())
 {
  overlayMap.insert(ov->name, ov);
  bDirty = true;
 }
}

void City::removeOverlay(Overlay* ov)
{
 while(overlayMap.contains(ov->name))
 {
  overlayMap.remove(ov->name);
  bDirty = true;
 }
}

void City::setDirty(bool b) {bDirty = b;}
bool City::isDirty() {return bDirty;}
