#ifndef KML_H
#define KML_H

#include <QObject>
#include "data.h"
#include "sql.h"
#include <QtXml/QDomDocument>

class QFile;
class segmentGroup;
class QDomElement;
class QDomDocument;
class SQL;
class routeInfo;
class RouteData;
class Kml : public QObject
{
 Q_OBJECT
public:
 explicit Kml(routeInfo ri, QObject *parent = 0);
 bool createKml(QString fileName, QString color);
 /*public*/ QDomDocument newDocument(QDomElement root);
 /*public*/ void addDefaultInfo(QDomElement root);
 /*public*/ void writeXML(QFile* file, QDomDocument doc);

signals:

public slots:

private:
 routeInfo ri;
 SegmentInfo si;
 SQL* sql;
 QDomDocument doc;
 QDomElement root;
 QDomElement createPlacemark(QString oneWay);
 QDomElement createArrow(SegmentInfo sg);
 LatLng pointRadialDistance(LatLng start, double bearing, double inDistance);

};

#endif // KML_H
