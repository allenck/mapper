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
//class RouteInfo;
class RouteData;
class Kml : public QObject
{
 Q_OBJECT
public:
 explicit Kml(QString routeName, QList<SegmentData> segmentDataList, QObject *parent = 0);
 bool createKml(QString fileName, QString color);
 /*public*/ QDomDocument newDocument(QDomElement root);
 /*public*/ void addDefaultInfo(QDomElement root);
 /*public*/ void writeXML(QFile* file, QDomDocument doc);

signals:

public slots:

private:
 //RouteInfo ri;
 SegmentData sd;
 SQL* sql;
 QDomDocument doc;
 QDomElement root;
 QDomElement createPlacemark(QString oneWay);
 QDomElement createArrow(SegmentData sg);
 LatLng pointRadialDistance(LatLng start, double bearing, double inDistance);
 QString routeName;
 QList<SegmentData> segmentDataList;
};

#endif // KML_H
