#ifndef BOUNDS_H
#define BOUNDS_H

#include <QObject>
#include "latlng.h"

class LatLng;
class Bounds : public QObject
{
    Q_OBJECT
public:
    Bounds(QObject* parent =0);
    Bounds(LatLng sw, LatLng ne);
    Bounds(QString bnds);
    ~Bounds();
    Bounds(const Bounds& other);
    static Bounds fromPointList(QList<LatLng>);
    bool isValid();
    bool checkValid();
    bool updateBounds(LatLng pt);
    bool updateBounds(Bounds bnds);
    void setBottomLeft(LatLng pt) {_swPt = pt;}
    void setTopRight(LatLng pt){_nePt = pt;};
    bool intersects(Bounds o);
    void operator=(const Bounds& other){
        _swPt = other._swPt;
        _nePt = other._nePt;
        _neLat = other._neLat;
        _neLon = other._neLon;
        _swLat = other._swLat;
        _swLon = other._swLon;
        bBoundsValid = other.bBoundsValid;
    }

    LatLng swPt() const;
    LatLng nePt() const;
    QString toString();
    bool contains(const LatLng &p) const;
    bool contains(const Bounds &b) const;
    LatLng center();

private:
    LatLng _swPt;
    LatLng _nePt;
    bool bBoundsValid;
    //float _swLat, _swLon, _neLat, _neLon;
    double _neLat, _neLon, _swLat, _swLon;
    QStringList sl;
};
Q_DECLARE_METATYPE(Bounds)
#endif // BOUNDS_H
