#ifndef GEOSERVER_H
#define GEOSERVER_H

#include <QObject>
#include <QtXml>
#include "data.h"
#include <QMap>

class FileDownloader;
class Layer : public QObject
{
public:
    Layer(QObject *parent = nullptr){}
    QString title;
    QString name;
    Bounds bounds;
    QString abstract; // description
    QString keyword;
};

class Geoserver : public QObject
{
    Q_OBJECT
public:
    explicit Geoserver(QObject *parent = nullptr);
    ~Geoserver() {}
    Geoserver(QString url, QObject *parent = nullptr);
    static Geoserver* instance();
    void getCapabilities(QString url);
    QList<Layer*> getLayerByName(QString name) {return nameMap.values(name);}
    QList<Layer*> getLayerByTitle(QString name) {return titleMap.values(name);}
    QString getHost() {return url;}
    QStringList titles(){return titleMap.keys();}

signals:
    void xmlFinished();

private slots:
    void processResource();

private :
    QString url;
    FileDownloader* m_resource;
    QList<Layer> layers;
    static Geoserver* _instance;
    QMultiMap<QString, Layer*> titleMap;
    QMultiMap<QString, Layer*> nameMap;
    QEventLoop* loop = nullptr;;

};

#endif // GEOSERVER_H
