#ifndef LATLNG_H
#define LATLNG_H
#include <QPointF>
#include <QObject>

class LatLng : public QPointF
{
public:
 LatLng();
 ~LatLng();
 LatLng(const LatLng& other);
 LatLng(double Lat, double Lon);
 double lat() const;
 void setLat(double val);
 double lon() const;
 void setLon(double val);
 bool isValid();
 bool checkValid();
 QString toString();
 QString str();
 static LatLng fromString(QString str);

 void operator=(const LatLng& other)
 {
  setY(other.y());
  setX(other.x());
  latitude = other.latitude;
  longitude= other.longitude;
  bValid = other.bValid;
 }

 bool operator==(const LatLng &other)
 {
     return other.latitude == latitude && other.longitude == longitude;
 }

private:
 bool bValid=false;
 double latitude=0, longitude=0;

};
Q_DECLARE_METATYPE(LatLng)

#endif // LATLNG_H
