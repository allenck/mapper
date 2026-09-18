#include "geoserver.h"
#include "filedownloader.h"
#include "qeventloop.h"

Geoserver* Geoserver::_instance = nullptr;
Geoserver* Geoserver::instance()
{
    if(_instance == NULL)
        _instance = new Geoserver();
    return _instance;
}


Geoserver::Geoserver(QObject *parent)
    : QObject{parent}
{ _instance = this;}


Geoserver::Geoserver(QString url, QObject *parent)
: QObject{parent}
{
    this->url = url;
    _instance = this;}

void Geoserver::getCapabilities(QString url)
{
    QEventLoop loop;
    m_resource = new FileDownloader(url + "geoserver/wms?service=WMS&version=1.3.0&request=GetCapabilities");
    connect(m_resource, SIGNAL(downloaded(QString)), this, SLOT(processResource()));
    loop.exec();

}

void Geoserver::processResource()
{
    QString str = m_resource->downloadedData();
    if(str != "")
    {
        QDomDocument doc;
        QString title;
        Bounds bounds;
        doc.setContent(str);
        QDomElement root = doc.documentElement();
        QString rootName = root.tagName();
        if(rootName == "WMS_Capabilities")
        {
            QDomElement contents = root.firstChildElement("Capability");
            if(!contents.isNull())
            {
                QDomNodeList list = contents.elementsByTagName("Layer");
                for(int i=0; i < list.count(); i++)
                {
                    QDomElement layer = list.at(i).toElement();
                    Layer* l = new Layer();

                    l->name = layer.firstChildElement("Name").text();
                    l->title = layer.firstChildElement("Title").text();
                    QDomElement bounds = layer.firstChildElement("EX_GeographicBoundingBox");
                    if(!bounds.isNull())
                    {
                        double westBoundLongitude = bounds.firstChildElement("westBoundLongitude").text().toDouble();
                        double eastBoundLongitude = bounds.firstChildElement("eastBoundLongitude").text().toDouble();
                        double westBoundLatitude = bounds.firstChildElement("westBoundLatitude").text().toDouble();
                        double eastBoundLatitude = bounds.firstChildElement("eastBoundLatitude").text().toDouble();
                        LatLng sw = LatLng(westBoundLatitude, westBoundLongitude);
                        LatLng ne = LatLng(eastBoundLatitude, eastBoundLongitude);
                        l->bounds = Bounds(sw,ne);
                    }
                    titleMap.insert(l->title, l);
                    nameMap.insert(l->title, l);
                    continue;
                }
            }
        }
    }
    emit xmlFinished();

}