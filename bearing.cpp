#include "bearing.h"

Bearing::Bearing()
{}
void Bearing::common()
{
    strNormal <<"N"<< "NE"<< "E"<< "SE"<< "S"<< "SW"<< "W"<< "NW";
    strReverse <<"S"<< "SW"<< "W"<< "NW"<< "N"<< "NE"<< "E"<< "SE";
    reverseDirection <<4<<5<<6<<7<<0<<1<<2<<3;
    R = 6371.0;
    dToRad = 0.0174532925;
    direction = 0;
    d = 0;
    brng = 0;
    lat1=0;
    lat2=0;
    lon1=0;
    lon2=0;
    dLat=0;
    dLon=0;
}

Bearing::Bearing(LatLng start, LatLng end)
{
    common();

    lat1 = start.lat() * dToRad;
    lon1 = start.lon() * dToRad;
    lat2 = end.lat() * dToRad;
    lon2 = end.lon() * dToRad;
    dLat = dToRad * (end.lat() - start.lat());
    dLon = dToRad * (end.lon() - start.lon());

    calculate();
}
Bearing::Bearing(const Bearing& other)
{
    double R; // RADIUS OF THE EARTH IN KM

    dToRad = other.dToRad;
    lat1 = other.lat1;
    lon1 = other.lon1;
    lat2 = other.lat2;
    lon2 = other.lon2;
    dLat = other.dLat;
    dLon = other.dLon;
    brng = other.brng;
    strNormal = other.strNormal;
    strReverse = other.strReverse;
    _strDirection = other._strDirection;
    reverseDirection = other.reverseDirection;
    direction = other.direction;
    d = other.d; // distance

}

/// <summary>
/// Constructor
/// </summary>
/// <param name="startLat"></param>
/// <param name="startLon"></param>
/// <param name="endLat"></param>
/// <param name="endLon"></param>
Bearing::Bearing(double startLat, double startLon, double endLat, double endLon)
{
    common();

    lat1 = startLat * dToRad;
    lon1 = startLon * dToRad;
    lat2 = endLat * dToRad;
    lon2 = endLon * dToRad;
    dLat = dToRad * (endLat - startLat);
    dLon = dToRad * (endLon - startLon);
    calculate();
}
void Bearing::calculate()
{
    double y = qSin(dLon) * qCos(lat2);
    double x = qCos(lat1) * qSin(lat2) -
               qSin(lat1) * qCos(lat2) * qCos(dLon);
    brng = qAtan2(y, x) / dToRad;
    // save values for writing out later
    if (brng < 0)
        brng = 360.0 + brng;
    direction = (int)((brng + 22.5) / 45.0) ;
    if(direction >= 8)
        direction = 0;

    // calculate distance
    double a = qSin(dLat / 2) * qSin(dLat / 2)
               + qCos(lat1) * qCos(lat2)
                     * qSin(dLon / 2) * qSin(dLon / 2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));
    d = R * c;
}
