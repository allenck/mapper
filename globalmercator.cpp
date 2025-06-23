#include "globalmercator.h"
#include <qmath.h>

//class GlobalMercator(object):
/*
TMS Global Mercator Profile
---------------------------
Functions necessary for generation of tiles in Spherical Mercator projection,
EPSG:900913 (EPSG:gOOglE, Google Maps Global Mercator), EPSG:3785, OSGEO:41001.
Such tiles are compatible with Google Maps, Microsoft Virtual Earth, Yahoo Maps,
UK Ordnance Survey OpenSpace API, ...
and you can overlay them on top of base maps of those web mapping applications.

Pixel and tile coordinates are in TMS notation (origin [0,0] in bottom-left).
What coordinate conversions do we need for TMS Global Mercator tiles::
  LatLon      <->       Meters      <->     Pixels    <->       Tile
WGS84 coordinates   Spherical Mercator  Pixels in pyramid  Tiles in pyramid
  lat/lon            XY in metres     XY pixels Z zoom      XYZ from TMS
 EPSG:4326           EPSG:900913
  .----.              ---------               --                TMS
 /      \     <->     |       |     <->     /----/    <->      Google
 \      /             |       |           /--------/          QuadTree
  -----               ---------         /------------/
KML, public         WebMapService         Web Clients      TileMapService
What is the coordinate extent of Earth in EPSG:900913?
[-20037508.342789244, -20037508.342789244, 20037508.342789244, 20037508.342789244]
Constant 20037508.342789244 comes from the circumference of the Earth in meters,
which is 40 thousand kilometers, the coordinate origin is in the middle of extent.
In fact you can calculate the constant as: 2 * M_PI * 6378137 / 2.0
$ echo 180 85 | gdaltransform -s_srs EPSG:4326 -t_srs EPSG:900913
Polar areas with abs(latitude) bigger then 85.05112878 are clipped off.
What are zoom level constants (pixels/meter) for pyramid with EPSG:900913?
whole region is on top of pyramid (zoom=0) covered by 256x256 pixels tile,
every lower zoom level resolution is always divided by two
initialResolution = 20037508.342789244 * 2 / 256 = 156543.03392804062
What is the difference between TMS and Google Maps/QuadTree tile name convention?
The tile raster itself is the same (equal extent, projection, pixel size),
there is just different identification of the same raster tile.
Tiles in TMS are counted from [0,0] in the bottom-left corner, id is XYZ.
Google placed the origin [0,0] to the top-left corner, reference is XYZ.
Microsoft is referencing tiles by a QuadTree name, defined on the website:
http://msdn2.microsoft.com/en-us/library/bb259689.aspx
The lat/lon coordinates are using WGS84 datum, yeh?
Yes, all lat/lon we are mentioning should use WGS84 Geodetic Datum.
Well, the web clients like Google Maps are projecting those coordinates by
Spherical Mercator, so in fact lat/lon coordinates on sphere are treated as if
the were on the WGS84 ellipsoid.

From MSDN documentation:
To simplify the calculations, we use the spherical form of projection, not
the ellipsoidal form. Since the projection is used only for map display,
and not for displaying numeric coordinates, we don't need the extra precision
of an ellipsoidal projection. The spherical projection causes approximately
0.33 percent scale distortion in the Y direction, which is not visually noticable.
How do I create a raster in EPSG:900913 and convert coordinates with PROJ.4?
You can use standard GIS tools like gdalwarp, cs2cs or gdaltransform.
All of the tools supports -t_srs 'epsg:900913'.
For other GIS programs check the exact definition of the projection:
More info at http://spatialreference.org/ref/user/google-projection/
The same projection is degined as EPSG:3785. WKT definition is in the official
EPSG database.
Proj4 Text:
 +proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0
 +k=1.0 +units=m +nadgrids=@null +no_defs
Human readable WKT format of EPGS:900913:
  PROJCS["Google Maps Global Mercator",
      GEOGCS["WGS 84",
          DATUM["WGS_1984",
              SPHEROID["WGS 84",6378137,298.2572235630016,
                  AUTHORITY["EPSG","7030"]],
              AUTHORITY["EPSG","6326"]],
          PRIMEM["Greenwich",0],
          UNIT["degree",0.0174532925199433],
          AUTHORITY["EPSG","4326"]],
      PROJECTION["Mercator_1SP"],
      PARAMETER["central_meridian",0],
      PARAMETER["scale_factor",1],
      PARAMETER["false_easting",0],
      PARAMETER["false_northing",0],
      UNIT["metre",1,
          AUTHORITY["EPSG","9001"]]]
*/
GlobalMercator::GlobalMercator(QObject *parent) : QObject(parent)
{
__init__(256);
}

void GlobalMercator::__init__(int tileSize)
{
 //"Initialize the TMS Global Mercator pyramid"
 this->tileSize = tileSize;
 this->initialResolution = 2 * M_PI * 6378137 / this->tileSize;
 //# 156543.03392804062 for tileSize 256 pixels
 this->originShift = 2 * M_PI * 6378137 / 2.0;
 //# 20037508.342789244
}

QPointF GlobalMercator::LatLonToMeters(double lat, double lon )
{
 //"Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913"

 mx = lon * this->originShift / 180.0;
 my = qLn( qTan((90 + lat) * M_PI / 360.0 )) / (M_PI / 180.0);

 my = my * this->originShift / 180.0;
 return QPointF(mx, my);
}

QPointF GlobalMercator::MetersToLatLon(double mx, double my )
{
 //"Converts XY point from Spherical Mercator EPSG:900913 to lat/lon in WGS84 Datum"

 lon = (mx / this->originShift) * 180.0;
 lat = (my / this->originShift) * 180.0;

 lat = 180 / M_PI * (2 * qAtan( qExp( lat * M_PI / 180.0)) - M_PI / 2.0);
 //return lat, lon
 return QPointF(lon, lat);
}

QPointF GlobalMercator::PixelsToMeters(double px, double py, int zoom)
{
 //"Converts pixel coordinates in given zoom level of pyramid to EPSG:900913"

 res = this->Resolution( zoom );
 mx = px * res - this->originShift;
 my = py * res - this->originShift;
 //return mx, my
 return QPointF(mx, my);
}

QPointF GlobalMercator::MetersToPixels(double mx, double my, int zoom)
{
 //"Converts EPSG:900913 to pyramid pixel coordinates in given zoom level"

 res = this->Resolution( zoom );
 px = (mx + this->originShift) / res;
 py = (my + this->originShift) / res;
 //return px, py
return QPointF(px, py);
}

QPoint GlobalMercator::PixelsToTile(double px, double py)
{
 //"Returns a tile covering region in given pixel coordinates"

 tx = int( qCeil( px / float(this->tileSize) ) - 1 );
 ty = int( qCeil( py / float(this->tileSize) ) - 1 );
 //return tx, ty
 return QPoint(tx, ty);
}

QPoint GlobalMercator::PixelsToRaster(int px, int py, int zoom)
{
 //"Move the origin of pixel coordinates to top-left corner"

 mapSize = this->tileSize << zoom;
 //return px, mapSize - py
 return QPoint(px, mapSize - py);
}

QPoint GlobalMercator::MetersToTile(double mx, double my, int zoom)
{
 //"Returns tile for given mercator coordinates"

 /*px, py =*/ this->MetersToPixels( mx, my, zoom);
 return this->PixelsToTile( px, py);
}

QRectF GlobalMercator::TileBounds(double tx, double ty, int zoom)
{
 //"Returns bounds of the given tile in EPSG:900913 coordinates"

 /*minx, miny =*/QPointF min = this->PixelsToMeters( tx*this->tileSize, ty*this->tileSize, zoom );
 /*maxx, maxy*/ QPointF max = this->PixelsToMeters( (tx+1)*this->tileSize, (ty+1)*this->tileSize, zoom );
 //return ( minx, miny, maxx, maxy )
 return QRectF(QPointF(min.x(), max.y()),QPointF(max.x(), min.y()));
}
#if 0
QRectF GlobalMercator::TileLatLonBounds(double tx, double ty, int zoom )
{
 //"Returns bounds of the given tile in latutude/longitude using WGS84 datum"

 bounds = this->TileBounds( tx, ty, zoom);
 /*minLat, minLon*/QPointF min = this->MetersToLatLon(bounds[0], bounds[1]);
 /*maxLat, maxLon*/QPointF max = this->MetersToLatLon(bounds[2], bounds[3]);

 //return ( minLat, minLon, maxLat, maxLon )
 return QRectF(min, max);
}
#endif
double GlobalMercator::Resolution(int zoom )
{
 //"Resolution (meters/pixel) for given zoom level (measured at Equator)"

 //# return (2 * M_PI * 6378137) / (this->tileSize * 2**zoom)
 return this->initialResolution / qPow(2,zoom);
}

int GlobalMercator::ZoomForPixelSize(int pixelSize )
{
 //"Maximal scaledown zoom of the pyramid closest to the pixelSize."

 //for i in range(30):
 for(int i=0; i < 30; i++)
 {
     if (pixelSize > this->Resolution(i))
         //return i-1 if i!=0 else 0 // We don't want to scale up
      if(i != 0)
       return i-1;
     else
       return 0;
 }
}

QPoint GlobalMercator::GoogleTile(int tx, int ty, int zoom)
{
 //"Converts TMS tile coordinates to Google Tile coordinates"

 //# coordinate origin is moved from bottom-left to top-left corner of the extent
 //return tx, (2**zoom - 1) - ty
  return QPoint(tx, qPow(2,zoom - 1) - ty);
}
#if 0
def QuadTree(self, tx, ty, zoom ):
 "Converts TMS tile coordinates to Microsoft QuadTree"

 quadKey = ""
 ty = (2**zoom - 1) - ty
 for i in range(zoom, 0, -1):
     digit = 0
     mask = 1 << (i-1)
     if (tx & mask) != 0:
         digit += 1
     if (ty & mask) != 0:
         digit += 2
     quadKey += str(digit)

 return quadKey
#endif
