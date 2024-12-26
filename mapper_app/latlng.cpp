#include "latlng.h"

LatLng::LatLng() : QPointF()
{
 bValid=false;
 setLon(0);
 setLat(0);
}
LatLng::~LatLng() {}
LatLng::LatLng(const LatLng& other) {
 setY(other.y());
 setX(other.x());
 latitude = other.latitude;
 longitude= other.longitude;
 bValid = other.bValid;
}


// <summary>
// Create new LatLng
// </summary>
// <param name="Lat"></param>
// <param name="Lon"></param>
LatLng::LatLng(double Lat, double Lon) : QPointF(Lon, Lat)
{
    latitude = Lat;
    longitude = Lon;
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
    latitude = val;
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
    longitude = val;
    setX(val);
    bValid=checkValid();
}

bool LatLng::isValid(){
 bValid = checkValid();
 return bValid;
}

bool LatLng::checkValid()
{
 if(x() == 0 && y() == 0)
  return false;
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
 QString strLat = QString("%1").arg(/*latitude*/y(),0,'f',6);
 QString strLon = QString("%1").arg(/*longitude*/x(),0,'f',6);
 return strLat + "," + strLon;
}

/*static*/ LatLng LatLng::fromString(QString str)
{
    LatLng rslt;
    QStringList sl;
    sl = str.split(",");
    bool ok;
    double lat;
    double lon;
    if(sl.count()==2)
    {
        lat = sl.at(0).toDouble(&ok);
        if(!ok)
            return rslt;
        lon = sl.at(1).toDouble(&ok);
        if(!ok)
            return rslt;
        rslt = LatLng(lat,lon);
    }
    return rslt;
}
