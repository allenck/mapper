#ifndef GLOBALMERCATOR_H
#define GLOBALMERCATOR_H

#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QRectF>

class GlobalMercator : public QObject
{
 Q_OBJECT
public:
 explicit GlobalMercator(QObject *parent = 0);
 QPointF LatLonToMeters(double lat, double lon );
 QPointF MetersToLatLon(double mx, double my );
 QPointF PixelsToMeters(double px, double py, int zoom);
 QPointF MetersToPixels(double mx, double my, int zoom);
 QPoint PixelsToTile(double px, double py);
 QPoint PixelsToRaster(int px, int py, int zoom);
 QPoint MetersToTile(double mx, double my, int zoom);
 QRectF TileBounds(double tx, double ty, int zoom);
 //QRectF TileLatLonBounds(double tx, double ty, int zoom);
 double Resolution(int zoom );
 int ZoomForPixelSize(int pixelSize );
 QPoint GoogleTile(int tx, int ty, int zoom);

signals:

public slots:

private:
 void __init__(int tileSize=256);
 int tileSize;
 double initialResolution;
 double originShift;
 double lat;
 double lon;
 double mx;
 double my;
 int px;
 int py;
 int tx;
 int ty;
 int res;
 int mapSize;
 QRectF bounds;
};

#endif // GLOBALMERCATOR_H
