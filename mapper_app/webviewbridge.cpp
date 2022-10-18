#include "webviewbridge.h"
#include "mainwindow.h"
#include <QDebug>

webViewBridge* webViewBridge::_instance = NULL;

webViewBridge::webViewBridge(QObject *parent)
 : QObject(parent)
{
 m_parent = parent;
 _instance = this;
}

webViewBridge::webViewBridge(LatLng latLng, int zoom, QString maptype, QObject *parent)
 : QObject(parent)
{
 this->_latLng = latLng;
 this->_lat = latLng.lat();
 this->_lon = latLng.lon();
 this->_zoom = zoom;
 this->maptype = maptype;
 _instance = this;
}

webViewBridge::~webViewBridge()
{
    //
}

webViewBridge* webViewBridge::instance()
{
 if(_instance == NULL)
  _instance = new webViewBridge();
 return _instance;
}

float webViewBridge::curLat() const {return _lat;}
float webViewBridge::curLon(){return _lon;}
LatLng webViewBridge::curLatLng(){return _latLng;}
void webViewBridge::setLatLng(LatLng latlng){this->_latLng = latlng; emit latlngChanged(latlng);}
int webViewBridge::curZoom(){return _zoom;}
QVariant webViewBridge::getRslt(){return myRslt;}
QString webViewBridge::curMaptype(){return maptype;}
//int webViewBridge::curBrowseWindowWidth(){return browseWindowWidth;}
//int webViewBridge::curBrowseWindowHeight(){return browseWindowHeight;}
void webViewBridge::processScript(QString func, QString parms)
{
 bResultReceived = false;
 emit executeScript( func,  parms);
 if(func == "loadOverlay")
  qDebug()<<func + " " + parms + "\n";
}
void webViewBridge::processScript(QString func)
{
 bResultReceived = false;
 myRslt = QVariant();
 myList = QVariantList();
 emit executeScript( func, "");
}

void webViewBridge::processScript(QString func, QString parms, QString name, QString value)
{
 bResultReceived = false;
 emit executeScript2( func,  parms, name, value);
 if(func == "loadOverlay")
  qDebug()<<func + " " + parms + "\n";
}

void webViewBridge::processScript(QString func, QVariantList objArray)
{
 bResultReceived = false;
 emit executeScript3(func, objArray, objArray.count() );
 if(func == "loadOverlay")
  qDebug()<<func + " + " + QString("%1").arg(objArray.count()) + " parameters";
}

void webViewBridge::selectSegment(qint32 i, qint32 SegmentId)
{
//    //MainWindow::selectSegment(i, SegmentId);
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->selectSegment(i, SegmentId);
 emit segmentSelected(i, SegmentId);
}

// Display a route comment for date.
void webViewBridge::getInfoWindowComments(double lat, double lon, int route, QString date, int func)
{
 MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
 parent->getInfoWindowComments(lat, lon, route, date, func);
}

// receive result of function
void webViewBridge::scriptResult(QVariant value)
{
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

void webViewBridge::scriptArrayResult(QVariantList value)
{
    if(value.isEmpty()) return;
 myList = value;
 bResultReceived = true;
 emit on_scriptArrayResult(value);
}

bool webViewBridge::isResultReceived()
{
 return bResultReceived;
}

void webViewBridge::setPoint(qint32 i, double lat, double lon)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->SetPoint(i, lat, lon);
}

void webViewBridge::segmentStatus(QString txt, QString color)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->segmentStatus(txt, color);
 bResultReceived = true;
 emit segmentStatusSignal(txt, color);
}

void webViewBridge::setLat(double lat)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->setLat(lat);
}
void webViewBridge::setLon(double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->setLat(lon);
}
void webViewBridge::setDebug(QString str)
{
 emit outputSetDebug(str);
}
void webViewBridge::setLen(qint32 len)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->setLen(len);
}
void webViewBridge::setCenter(double lat, double lon, int zoom, QString maptype)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->m_latitude = lat;
    parent->m_longitude = lon;
    parent->m_zoom = zoom;
    parent->m_maptype = maptype;
    bResultReceived = true;
}

void webViewBridge::addPoint(int pt, double lat, double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->addPoint();
 emit addPointSignal(pt, lat, lon);
}
//TODO
void webViewBridge::moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteStartMarker(lat, lon, segmentId, i);
}
void webViewBridge::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteEndMarker(lat, lon, segmentId, i);
}
QString webViewBridge::getImagePath(qint32 i)
{
    Q_UNUSED(i)
    //mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//TODO:    return parent->getImagePath(i);
    return "";  // remove when implemented.
}

void webViewBridge::movePoint(qint32 segmentId, qint32 i, double lat, double lng)
{
 emit movePointSignal(segmentId, i, lat, lng);
}

void webViewBridge::insertPoint(int SegmentId, qint32 i, double newLat, double newLon)
{
 emit insertPointSignal(SegmentId, i, newLat, newLon);
}

void webViewBridge::updateIntersection(qint32 i, double newLat, double newLon)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    return parent->updateIntersection( i, newLat, newLon);
}
void webViewBridge::displayZoom(int zoom)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->getZoom(zoom);
}

void webViewBridge::showSegmentsAtPoint(double lat, double lon, qint32 segmentId)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->segmentView->showSegmentsAtPoint(lat, lon, segmentId);
}
void webViewBridge::queryOverlay()
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    parent->queryOverlay();
 emit queryOverlaySignal();
}
void webViewBridge::opacityChanged(QString name, qint32 opacity)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->opacityChanged(name, opacity);

}

void webViewBridge::getGeocoderResults(QString text)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->getGeocoderResults(text);
}

void webViewBridge::setStation(double lat, double lon, qint32 SegmentId, qint32 i)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->setStation(lat, lon, SegmentId, i);
}
void webViewBridge::updateStation(qint32 stationKey, qint32 segmentId)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->updateStation(stationKey, segmentId);
}
void webViewBridge::moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveStationMarker(stationKey, segmentId, lat, lng);
}
void webViewBridge::moveRouteComment(qint32 route, QString date, double lat, double lng, int companyKey)
{
    MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
    parent->moveRouteComment(route, date, lat, lng, companyKey);
}


void webViewBridge::mapInit()
{
 MainWindow * parent = qobject_cast<MainWindow*>(this->parent());
 parent->mapInit();
}

void webViewBridge::debug(QString text)
{
 qDebug() << text;
}
