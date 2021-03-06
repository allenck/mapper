#include "city.h"

City::City(QObject *parent) : QObject(parent)
{

}
QList<Overlay*> City::overlayList() {return overlays;}
void City::addOverlay(Overlay* ov)
{
 overlays.append(ov);
 bDirty = true;
}
void City::removeOverlay(Overlay* ov)
{
 overlays.removeOne(ov);
 bDirty = true;
}

void City::setDirty(bool b) {bDirty = b;}
bool City::isDirty() {return bDirty;}
