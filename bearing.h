#ifndef BEARING_H
#define BEARING_H

#include <QObject>
#include "latlng.h"

class Bearing
{
public:
    explicit Bearing();
    Bearing(LatLng start, LatLng end);
    Bearing(double startLat, double startLon, double endLat, double endLon);
    ~Bearing() {}
    Bearing(const Bearing& other);
    qint32 getDirection()
    {
        return direction;
    }
    qint32 getReverseDirection()
    {
        return reverseDirection[direction];
    }
    double angle()
    {
        return brng;
    }
    /// <summary>
    /// gets distance in km
    /// </summary>
    double Distance() {return d;}
    /// <summary>
    /// gets the direction string, e.g. "N", "NE", etc.
    /// </summary>
    QString strDirection()
    {
        if(direction < 0 || direction > 7)
            return "?";
        return strNormal.at(direction);
    }
    /// <summary>
    ///  gets the reverse direction string, e.g. "S", "SW", etc.
    /// </summary>
    QString strReverseDirection()
    {
        if(direction < 0 || direction > 7)
            return "?";
        return strReverse.at(direction);
    }
    /// <summary>
    /// set a direction string, e.g. "N", "NE", etc.
    /// </summary>
    void DirectionString(QString value)
    {
        _strDirection = value;
    }
    /// <summary>
    /// return the reverse direction for DirectionString
    /// </summary>
    QString ReverseDirectionString()
    {
        int i;
        for (i = 0; i < strNormal.size(); i++)
        {
            if (_strDirection == strNormal[i])
                return strReverse[i];
        }
        return "";
    }

signals:
private:
    double R; // RADIUS OF THE EARTH IN KM

    double dToRad;
    double lat1;
    double lon1;
    double lat2;
    double lon2;
    double dLat;
    double dLon;
    double brng;
    QStringList strNormal;
    QStringList strReverse;
    QString _strDirection;
    QList<qint32> reverseDirection;
    qint32 direction;
    double d; // distance
    void common();
    void calculate();
};

#endif // BEARING_H
