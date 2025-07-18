#include "webviewbridge.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include "segmentview.h"
#include <QFileDialog>
#include "exceptions.h"
#include "mainwindow.h"

WebViewBridge* WebViewBridge::_instance = NULL;

WebViewBridge::WebViewBridge(MainWindow *parent)
 : QObject()
{
 m_parent = parent;
 _instance = this;
 config = Configuration::instance();
}

WebViewBridge::WebViewBridge(LatLng latLng, int zoom, QString maptype, QString mapId, MainWindow *parent)
 : QObject()
{
 this->_latLng = latLng;
 this->_lat = latLng.lat();
 this->_lon = latLng.lon();
 this->_zoom = zoom;
 this->maptype = maptype;
 this->mapId = mapId;
 m_parent = parent;


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
void WebViewBridge::setLatLng(LatLng latlng){
    this->_latLng = latlng;
    emit latlngChanged(latlng);
}
int WebViewBridge::curZoom(){return _zoom;}
QVariant WebViewBridge::getRslt(){return myRslt;}
QString WebViewBridge::curMaptype(){return maptype;}
QString WebViewBridge::curMapId(){return mapId;}
void WebViewBridge::setMapId(QString mapid){this->mapId = mapid;}

void WebViewBridge::processScript(QString func, QString parms)
{
 qDebug() << "processScript " << func << " " << parms;

 bResultReceived = false;
 emit executeScript( func,  parms);
 if(func == "loadOverlay")
  qDebug()<<func + " " + parms + "\n";
}
void WebViewBridge::processScript(QString func)
{
 qDebug() << "processScript " << func;
 bResultReceived = false;
 //myRslt = QVariant();
 myList = QVariantList();
 emit executeScript( func, "");
}

void WebViewBridge::processScript(QString func, QString parms, QString name, QString value)
{
 //qDebug() << "processScript " << func;
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
    m_parent->segmentSelected(i, SegmentId);
 emit segmentSelected(i, SegmentId);
}

void WebViewBridge::selectSegmentX(qint32 i, qint32 SegmentId, QVariantList array)
{
//    //MainWindow::selectSegment(i, SegmentId);
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
    m_parent->segmentSelected(i, SegmentId);
 emit segmentSelectedX(i, SegmentId, buildPoints(array));
}

// Display a route comment for date.
void WebViewBridge::getInfoWindowComments(double lat, double lon, int route, QString date, int func)
{

 m_parent->getInfoWindowComments(lat, lon, route, date, func);
}

// receive result of function
void WebViewBridge::scriptResult(QVariant value)
{
 try {
  // if(value != QVariant())
  //  qDebug() << "scriptResult" << value;
  myRslt = value;
  bResultReceived = true;
  emit on_scriptResult(value);
 }
 catch(Exception)
 {
  qDebug() << "bad script result";
 }
}

// receive result of function
void WebViewBridge::scriptFunctionResult(QVariant function, QVariant value)
{
    try {
        qDebug() << "scriptFunctionResult" << function << value;
        //  if(value.isNull())
        //   return;
        myRslt = value;
        bResultReceived = true;
        emit on_scriptFunctionResult(function,value);
    }
    catch(Exception)
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

    m_parent->SetPoint(i, lat, lon);
}

void WebViewBridge::segmentStatus(QString txt, QString color)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    m_parent->segmentStatus(txt, color);
 bResultReceived = true;
 emit segmentStatusSignal(txt, color);
}

void WebViewBridge::setLat(double lat)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    m_parent->setLat(lat);
}

void WebViewBridge::setLon(double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    m_parent->setLat(lon);
}
void WebViewBridge::setDebug(QString str)
{
 emit outputSetDebug(str);
}
void WebViewBridge::setLen(qint32 len)
{

    m_parent->setLen(len);
}
void WebViewBridge::setCenter(double lat, double lon, int zoom, QString maptype)
{
    m_parent->m_latitude = lat;
    m_parent->m_longitude = lon;
    m_parent->m_zoom = zoom;
    m_parent->m_maptype = maptype;
    bResultReceived = true;
}

void WebViewBridge::addPoint(int pt, double lat, double lon)
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    m_parent->addPoint();
 emit addPointSignal(pt, lat, lon);
}

void WebViewBridge::addPointX(int pt, QVariantList array)
{
    //    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
    //    m_parent->addPoint();
    emit addPointSignalX(pt, buildPoints(array));
}

void WebViewBridge::addPointMode(bool bOn)
{
 m_parent->m_bAddMode = bOn;
}

//TODO
void WebViewBridge::moveRouteStartMarker(double lat, double lon, qint32 segmentId, qint32 i)
{

    m_parent->moveRouteStartMarker(lat, lon, segmentId, i);
}
void WebViewBridge::moveRouteEndMarker(double lat, double lon, qint32 segmentId, qint32 i)
{

    m_parent->moveRouteEndMarker(lat, lon, segmentId, i);
}
QString WebViewBridge::getImagePath(qint32 i)
{
    Q_UNUSED(i)
    //mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//TODO:    return m_parent->getImagePath(i);
    return "";  // remove when implemented.
}
void WebViewBridge::clickPoint(double lat, double lng)
{
    LatLng latlng = LatLng(lat, lng);
    emit clickLatLng(latlng );
    // QClipboard *clip = QApplication::clipboard();
    // QVariant v = QVariant::fromValue(latlng);
    // QMimeData* mimeData = new QMimeData;
    // QByteArray ba = v.toByteArray();
    // mimeData->setData("latlng", ba);
    // clip->setMimeData(mimeData);
    // clip->setText(latlng.str());
}

void WebViewBridge::movePoint(qint32 segmentId, qint32 i, double lat, double lng)
{
 emit movePointSignal(segmentId, i, lat, lng);
 //m_parent->movePoint(segmentId, i, lat, lng);
}

void WebViewBridge::movePointX(qint32 segmentId, qint32 i, double lat, double lon, QVariantList array)
{
    LatLng pt = LatLng(lat,lon);
    emit movePointSignalX(segmentId, i, pt, buildPoints(array));
    //m_parent->movePoint(segmentId, i, lat, lng);
}

void WebViewBridge::insertPoint(int SegmentId, qint32 i, double newLat, double newLon)
{
 emit insertPointSignal(SegmentId, i, newLat, newLon);
}

void WebViewBridge::insertPointX(int SegmentId, qint32 i, QVariantList array)
{
    emit insertPointSignalX(SegmentId, i, buildPoints(array));
}

void WebViewBridge::updateIntersection(qint32 i, double newLat, double newLon)
{

    return m_parent->updateIntersection( i, newLat, newLon);
}
void WebViewBridge::displayZoom(int zoom)
{

    m_parent->getZoom(zoom);
}

void WebViewBridge::showSegmentsAtPoint(double lat, double lon, qint32 segmentId)
{

    m_parent->segmentView->showSegmentsAtPoint(lat, lon, segmentId);
}

void WebViewBridge::queryOverlay()
{
//    mainWindow * parent = qobject_cast<mainWindow*>(this->parent());
//    m_parent->queryOverlay();
 emit queryOverlaySignal();
}

// void WebViewBridge::initialized()
// {
//  //m_parent->showGoogleMapFeaturesAct->trigger();
// }

void WebViewBridge::opacityChanged(QString name, qint32 opacity)
{
    m_parent->opacityChanged(name, opacity);
}

void WebViewBridge::getGeocoderResults(QString text)
{
    m_parent->getGeocoderResults(text);
}

void WebViewBridge::setStation(double lat, double lon, qint32 SegmentId, qint32 i)
{
    m_parent->setStation(lat, lon, SegmentId, i);
}
void WebViewBridge::updateStation(qint32 stationKey, qint32 segmentId)
{

    m_parent->updateStation(stationKey, segmentId);
}
void WebViewBridge::moveStationMarker(qint32 stationKey, qint32 segmentId, double lat, double lng)
{

    m_parent->moveStationMarker(stationKey, segmentId, lat, lng);
}
void WebViewBridge::moveRouteComment(qint32 route, QString date, double lat, double lng, int companyKey)
{

    m_parent->moveRouteComment(route, date, lat, lng, companyKey);
}


void WebViewBridge::mapInit()
{

 m_parent->mapInit();
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
 emit on_cityBounds(bounds);

 Parameters parms = SQL::instance()->getParameters();
 parms.lat = center.lat();
 parms.lon = center.lon();
 SQL::instance()->updateParameters(parms);
}

void WebViewBridge::rightClicked(double lat, double lon)
{
    //_rightClickLoc = rightClickLoc;
    qDebug() << "LatLng:"<< lat << " " << lon;
    emit on_rightClicked(LatLng(lat, lon));
}

void WebViewBridge::pinClicked(int pinId, double lat, double lon, QString street, int streetId, QString location, int seq)
{
    emit on_pinClicked(pinId, LatLng(lat, lon), street, streetId, location, seq);
}

void WebViewBridge::pinMarkerMoved(double lat, double lon)
{
    LatLng latlng = LatLng(lat,lon);
    emit on_pinMarkerMoved(latlng);
    QClipboard *clip = QApplication::clipboard();
    QVariant v = QVariant::fromValue(latlng);
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = v.toByteArray();
    mimeData->setData("latlng", ba);
    clip->setMimeData(mimeData);
    clip->setText(latlng.str());}

void WebViewBridge::screenshot(QString base64image)
{
 QString saveFilename = QFileDialog::getSaveFileName(nullptr, "Save as", "Choose a filename", "PNG(*.png);; TIFF(*.tiff *.tif);; JPEG(*.jpg *.jpeg)");

 QString saveExtension = "PNG";
 int pos = saveFilename.lastIndexOf('.');
 if (pos >= 0)
     saveExtension = saveFilename.mid(pos + 1);
 QString ext = "." + saveExtension.toLower();
 if(!saveFilename.endsWith(ext))
  saveFilename.append(ext);

 QByteArray base64Data = base64image.toLatin1().mid(base64image.indexOf(",")+1);
 QImage image;
 if(image.loadFromData(QByteArray::fromBase64(base64Data)))
 {
  image.save(saveFilename, "PNG");
 }
}

QList<LatLng> WebViewBridge::buildPoints(QVariantList array)
{
    QList<LatLng> points;
    for(int i=0; i < array.count(); i+=2)
    {
        points.append(LatLng(array.at(i).toDouble(), array.at(i+1).toDouble()));
    }
    return points;
}

