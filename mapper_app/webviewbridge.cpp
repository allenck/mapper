#include "webviewbridge.h"
#include "mainwindow.h"
#include <QDebug>

WebViewBridge* WebViewBridge::_instance = NULL;

WebViewBridge::WebViewBridge(QObject *parent)
 : QObject(parent)
{
 m_parent = parent;
 _instance = this;
 config = Configuration::instance();
}

WebViewBridge::WebViewBridge(LatLng latLng, int zoom, QString maptype, QObject *parent)
 : QObject(parent)
{
 this->_latLng = latLng;
 this->_lat = latLng.lat();
 this->_lon = latLng.lon();
 this->_zoom = zoom;
 this->maptype = maptype;
 _instance = this;
 config = Configuration::instance();
}

WebViewBridge::~WebViewBridge()
{
    //
}

WebViewBridge* WebViewBridge::instance()
{
 if(_instance == NULL)
  _instance = new WebViewBridge();
 return _instance;
}

float WebViewBridge::curLat() const {return _lat;}
float WebViewBridge::curLon(){return _lon;}
LatLng WebViewBridge::curLatLng(){return _latLng;}
void WebViewBridge::setLatLng(LatLng latlng){this->_latLng = latlng; emit latlngChanged(latlng);}
int WebViewBridge::curZoom(){return _zoom;}
QVariant WebViewBridge::getRslt(){return myRslt;}
QString WebViewBridge::curMaptype(){return maptype;}
//int webViewBridge::curBrowseWindowWidth(){return browseWindowWidth;}
//int webViewBridge::curBrowseWindowHeight(){return browseWindowHeight;}
void WebViewBridge::processScript(QString func, QString parms)
{
 qDebug() << "processScript " << func;

 bResultReceived = false;
 emit executeScript( func,  parms);
 if(func == "loadOverlay")
  qDebug()<<func + " " + parms + "\n";
}
void WebViewBridge::processScript(QString func)
{
 qDebug() << "processScript " << func;
 bResultReceived = false;
 myRslt = QVariant();
 myList = QVariantList();
 emit executeScript( func, "");
}

void WebViewBridge::processScript(QString func, QString parms, QString name, QString value)
{
 qDebug() << "processScript " << func;
 bResultReceived = false;
 emit executeScript2( func,  parms, name, value);
 if(func == "loadOverlay")
  qDebug()<<func + " " + parms + "\n";
}

void WebViewBridge::processScript(QString func, QVariantList objArray)
{
 qDebug() << "processScript " << func;

 bResultReceived = false;
 emit executeScript3(func, objArray, objArray.count() );
 if(func == "loadOverlay")
  qDebug()<<func + " + " + QString("%1").arg(objArray.count()) + " parameters";
}

void WebViewBridge::selectSegment(qint32 i, qint32 SegmentId)
{
//    //MainWindow::selectSegment(i, SegmentId);
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->selectSegment(i, SegmentId);
 emit segmentSelected(i, SegmentId);
}

// Display a route comment for date.
void WebViewBridge::getInfoWindowComments(double lat, double lon, int route, QString date, int func)
{
 MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
 parent->getInfoWindowComments(lat, lon, route, date, func);
}

// receive result of function
void WebViewBridge::scriptResult(QVariant value)
{
 try {
  if(value == 0)
   //value = QVariant();
   return;
  qDebug() << "scriptResult" << value;
  if(value.isNull())
   return;
  myRslt = value;
  bResultReceived = true;
  emit on_scriptResult(value);
 }
 catch(std::exception)
 {
  qDebug() << "bad script result";
 }
}

void WebViewBridge::scriptArrayResult(QVariantList value)
{
    if(value.isEmpty()) return;
 myList = value;
 bResultReceived = true;
 emit on_scriptArrayResult(value);
}

bool WebViewBridge::isResultReceived()
{
 return bResultReceived;
}

void WebViewBridge::setPoint(qint32 i, double lat, double lon)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->SetPoint(i, lat, lon);
}

void WebViewBridge::segmentStatus(QString txt, QString color)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->segmentStatus(txt, color);
 bResultReceived = true;
 emit segmentStatusSignal(txt, color);
}

void WebViewBridge::setLat(double lat)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->setLat(lat);
}
void WebViewBridge::setLon(double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->setLat(lon);
}
void WebViewBridge::setDebug(QString str)
{
 emit outputSetDebug(str);
}
void WebViewBridge::setLen(qint32 len)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->setLen(len);
}
void WebViewBridge::setCenter(double lat, double lon, int zoom, QString maptype)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->m_latitude = lat;
    parent->m_longitude = lon;
    parent->m_zoom = zoom;
    parent->m_maptype = maptype;
    bResultReceived = true;
}

void WebViewBridge::addPoint(int pt, double lat, double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->addPoint();
 emit addPointSignal(pt, lat, lon);
}
//TODO
void WebViewBridge::moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteStartMarker(lat, lon, segmentId, i);
}
void WebViewBridge::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteEndMarker(lat, lon, segmentId, i);
}
QString WebViewBridge::getImagePath(qint32 i)
{
    Q_UNUSED(i)
    //mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//TODO:    return parent->getImagePath(i);
    return "";  // remove when implemented.
}

void WebViewBridge::movePoint(qint32 segmentId, qint32 i, double lat, double lng)
{
 emit movePointSignal(segmentId, i, lat, lng);
}

void WebViewBridge::insertPoint(int SegmentId, qint32 i, double newLat, double newLon)
{
 emit insertPointSignal(SegmentId, i, newLat, newLon);
}

void WebViewBridge::updateIntersection(qint32 i, double newLat, double newLon)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    return parent->updateIntersection( i, newLat, newLon);
}
void WebViewBridge::displayZoom(int zoom)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->getZoom(zoom);
}

void WebViewBridge::showSegmentsAtPoint(double lat, double lon, qint32 segmentId)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->segmentView->showSegmentsAtPoint(lat, lon, segmentId);
}
void WebViewBridge::queryOverlay()
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->queryOverlay();
 emit queryOverlaySignal();
}
void WebViewBridge::opacityChanged(QString name, qint32 opacity)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->opacityChanged(name, opacity);

}

void WebViewBridge::getGeocoderResults(QString text)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->getGeocoderResults(text);
}

void WebViewBridge::setStation(double lat, double lon, qint32 SegmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->setStation(lat, lon, SegmentId, i);
}
void WebViewBridge::updateStation(qint32 stationKey, qint32 segmentId)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->updateStation(stationKey, segmentId);
}
void WebViewBridge::moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveStationMarker(stationKey, segmentId, lat, lng);
}
void WebViewBridge::moveRouteComment(qint32 route, QString date, double lat, double lng, int companyKey)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteComment(route, date, lat, lng, companyKey);
}


void WebViewBridge::mapInit()
{
 MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
 parent->mapInit();
}

void WebViewBridge::debug(QString text)
{
 qDebug() << text;
}

void WebViewBridge::cityBounds(double neLat, double neLng, double swLat, double swLng)
{
 Bounds bounds = Bounds(LatLng(swLat, swLng), LatLng(neLat, neLng));
 qDebug() << "city bounds" << bounds.toString() << "valid=" << bounds.isValid();
 config->currCity->setBounds(bounds);
 LatLng center = bounds.center();
 config->currCity->setCenter(center);
 config->saveSettings();
 processScript("closeCityBoundsButton");
}

void WebViewBridge::rightClicked(LatLng rightClickLoc)
{
    _rightClickLoc = rightClickLoc;
}
