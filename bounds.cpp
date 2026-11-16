#include "bounds.h"
#include <qpaintdevice.h>

Bounds::Bounds(QObject *parent)
    : QObject(parent)
{
    _swPt = LatLng(90,180);
    _nePt = LatLng(-90,-180);
    bBoundsValid = false;
}

Bounds::Bounds(LatLng sw, LatLng ne) :  QObject()
{
    _swPt = sw;
    _swLat = sw.lat();
    _swLon = sw.lon();
    _nePt = ne;
    _neLat = ne.lat();
    _neLon = ne.lon();
    if(sw.lon() < ne.lon() && sw.lat() < ne.lat())
    {
        bBoundsValid = true;
    }
    else
        bBoundsValid = false;
}

Bounds::Bounds(QString bnds): QObject()
{
    _swPt = LatLng(90,180);
    _nePt = LatLng(-90,-180);
    bBoundsValid = false;
    QStringList sl = bnds.split(",");
    if(sl.count()>=4)
    {
        bool ok;
        _swLon = sl.at(0).toDouble(&ok);
        if(!(ok && _swLon >= -180.0 && _swLon <= 180.0))
            return;
        _swLat = sl.at(1).toDouble(&ok);
        if(!(ok && _swLat >= -90.0 && _swLat <= 90.0))
            return;
        _neLon = sl.at(2).toDouble(&ok);
        if(!(ok && _neLon >= -180.0 && _neLon <= 180.0))
            return;
        _neLat = sl.at(3).toDouble(&ok);
        if(!(ok && _neLat >= -90.0 && _neLat <= 90.0))
            return;
        _swPt = LatLng(_swLat, _swLon);
        _nePt = LatLng(_neLat, _neLon);
        setBottomLeft(_swPt);
        setTopRight(_nePt);
        bBoundsValid = isValid();
    }
}
Bounds::~Bounds(){}
Bounds::Bounds(const Bounds &other) : QObject()
{
    _swPt = other._swPt;
    _nePt = other._nePt;
    _neLat = other._neLat;
    _neLon = other._neLon;
    _swLat = other._swLat;
    _swLon = other._swLon;
    setBottomLeft(_swPt);
    setTopRight(_nePt);
    bBoundsValid = other.bBoundsValid;
}

Bounds Bounds::fromPointList(QList<LatLng> list)
{
    Bounds b = Bounds();
    double neLat = b.nePt().lat();
    double neLon = b.nePt().lon();
    double swLat = b.swPt().lat();
    double swLon = b.swPt().lon();
    foreach(LatLng latLng, list)
    {
        if(latLng.lat() > neLat)
            neLat = latLng.lat();
        if(latLng.lat() < swLat)
            swLat = latLng.lat();

        if(latLng.lon() > neLon)
            neLon = latLng.lon();
        if(latLng.lon() < swLon)
            swLon = latLng.lon();
        b.setBottomLeft(LatLng(swLat, swLon));
        b.setTopRight(LatLng(neLat, neLon));
    }
    return b;
}

bool Bounds::isValid() {bBoundsValid = checkValid(); return bBoundsValid;}
bool Bounds::checkValid()
{
    // if(_nePt.lon() == 0)
    //  return false;
    if(!_swPt.isValid())
        return false;
    if(!_nePt.isValid())
        return false;
    if(_swPt.lat() < _nePt.lat() && _swPt.lon() < _nePt.lon())
        return true;
    return false;
}
bool Bounds::updateBounds(LatLng pt)
{
    if (pt.lat() < _swPt.lat())
        _swPt.setLat(pt.lat());
    if (pt.lat() > _nePt.lat())
        _nePt.setLat( pt.lat());
    if (pt.lon() < _swPt.lon())
        _swPt.setLon( pt.lon());
    if (pt.lon() > _nePt.lon())
        _nePt.setLon( pt.lon());
    bBoundsValid = true;
    return bBoundsValid;
}
bool Bounds::updateBounds(Bounds bnds)
{
    updateBounds(bnds.nePt());
    updateBounds(bnds.swPt());
    return bBoundsValid;
}

LatLng Bounds::swPt() const { return _swPt;}
LatLng Bounds::nePt() const { return _nePt;}

QString Bounds::toString()
{
    return QString("%1,%2, %3,%4").arg(_swPt.lon(),14,'g',11).arg(_swPt.lat(),14,'g',11).arg(_nePt.lon(),14,'g',11).arg(_nePt.lat(),14,'g',11);
}

bool Bounds::contains(const LatLng &p) const
{
    // if((p.lon() >= _swPt.lon()) && (p.lon() <= _nePt.lon())
    //    && (p.lat() >= _swPt.lat()) && (p.lat()<= _nePt.lat())) return true;
    if(!(p.lon() >= _swPt.lon())) return false;
    if(!(p.lon() <= _nePt.lon())) return false;
    if(!(p.lat() >= _swPt.lat())) return false;
    if( (p.lat() <= _nePt.lat())) return true;

    return false;
}

bool Bounds::contains(const Bounds &b) const
{
    if(this->contains(b._swPt) || this->contains(b._nePt) || b.contains(this->_swPt) || b.contains(this->_nePt))
        return true;
    return false;
}

LatLng Bounds::center() {
    // QPoint topLeft(_swPt.lon(), _nePt.lat());
    // QPoint bottomRight(_nePt.lon(), _swPt.lat());
    QRectF rect(QPointF(_swPt.lon(), _nePt.lat()),
                QPointF(_nePt.lon(), _swPt.lat())); // top left pt, bottom right pt
    QPointF ctr(rect.center());
    return LatLng(ctr.y(), ctr.x());
}

bool Bounds::intersects(Bounds o)
{
    if(!isValid() || !o.isValid())
        return false;
    QRectF thisRect(QPoint(_swPt.lon(), _nePt.lat()), QPoint(_nePt.lon(), _swPt.lat()));
    QRectF otherRect(QPoint(o._swPt.lon(), o._nePt.lat()), QPoint(o._nePt.lon(), o._swPt.lat()));
    return thisRect.intersects(otherRect);
}