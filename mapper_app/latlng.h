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
 bool operator ==(const LatLng pt);

private:
 bool bValid;

};
Q_DECLARE_METATYPE(LatLng)

#endif // LATLNG_H
