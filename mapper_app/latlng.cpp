#include "latlng.h"

LatLng::LatLng()
{
 bValid=false;

}
LatLng::~LatLng() {}
LatLng::LatLng(const LatLng& other) { setY(other.y()); setX(other.x());}
/// <summary>
/// Create new LatLng
/// </summary>
/// <param name="Lat"></param>
/// <param name="Lon"></param>
LatLng::LatLng(double Lat, double Lon) : QPointF(Lon, Lat)
{
//    latitude = Lat;
//    longitude = Lon;
    if(Lat==0 && Lon ==0)
     bValid = false;
    else
     bValid=checkValid();
}
/// <summary>
/// get or set latitude
/// </summary>
double LatLng::lat() const
{
    return y();
}
void LatLng::setLat(double val)
{
    //latitude = val;
    setY(val);
    bValid = checkValid();
}

/// <summary>
/// get longitude
/// </summary>
double LatLng::lon() const
{
    return x(); //longitude;
}
void LatLng::setLon(double val)
{
    //longitude = val;
    setX(val);
    bValid=checkValid();
}

bool LatLng::isValid(){return bValid;}

bool LatLng::checkValid()
{
 if((y() > 90.0) || (y() < -90.))
  return false;
 if((x() < -180.) || (x() > 180.))
  return false;
 return true;
}

QString LatLng::toString()
{
    QString strLat = QString("%1").arg(/*latitude*/y());
    QString strLon = QString("%1").arg(/*longitude*/x());
    return "lat: " + strLat + " lon: " + strLon;
}

QString LatLng::str()
{
 QString strLat = QString("%1").arg(/*latitude*/y(),0,'f',8);
 QString strLon = QString("%1").arg(/*longitude*/x(),0,'f',8);
 return strLat + "," + strLon;
}

bool LatLng::operator ==(const LatLng pt)
{
    //return pt.latitude == this->latitude && pt.longitude == this->longitude;
    return pt.y() == this->y() && pt.x() == this->x();
}
