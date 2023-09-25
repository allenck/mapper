#include "kml.h"

Kml::Kml(QString routeName, QList<SegmentData>segmentDataList, QObject *parent) :
  QObject(parent)
{
 this->routeName = routeName;
 this->segmentDataList = segmentDataList;
}



bool Kml::createKml(QString fileName, QString color)
{
 doc = QDomDocument();
 QDomProcessingInstruction xmlProcessingInstruction = doc.createProcessingInstruction("xml", "version=\"1.0\"  encoding=\"UTF-8\"");
 doc.appendChild(xmlProcessingInstruction);
 root = doc.createElement("kml");
 root.setAttribute("xmlns","http://www.opengis.net/kml/2.2");
 QDomElement document = doc.createElement("Document");
 document.setAttribute("name", routeName);
 root.appendChild(document);
 doc.appendChild(root);

 QDomElement style = doc.createElement("Style");
 style.setAttribute("id", "wideRed");
 QDomElement lineStyle = doc.createElement("LineStyle");
 QDomElement c = doc.createElement("color");
 c.appendChild(doc.createTextNode(color));
 lineStyle.appendChild(c);
 QDomElement w = doc.createElement("width");
 w.appendChild(doc.createTextNode("3"));
 lineStyle.appendChild(w);
 style.appendChild(lineStyle);
 document.appendChild(style);

 QDomElement style2 = doc.createElement("Style");
 style2.setAttribute("id", "narrowRed");
 QDomElement lineStyle2 = doc.createElement("LineStyle");
 QDomElement c2 = doc.createElement("color");
 c2.appendChild(doc.createTextNode(color));
 lineStyle2.appendChild(c2);
 QDomElement w2 = doc.createElement("width");
 w2.appendChild(doc.createTextNode("1"));
 lineStyle2.appendChild(w2);
 style2.appendChild(lineStyle2);
 document.appendChild(style2);

 QDomElement arrowStyle = doc.createElement("Style");
 arrowStyle.setAttribute("id", "arrow");

 QDomElement lineStyle3 = doc.createElement("LineStyle");
 QDomElement c3 = doc.createElement("color");
 c3.appendChild(doc.createTextNode(color));
 lineStyle3.appendChild(c3);
 arrowStyle.appendChild(lineStyle3);
 QDomElement polyStyle = doc.createElement("PolyStyle");
 QDomElement c4 = doc.createElement("color");
 c4.appendChild(doc.createTextNode(color));
 QDomElement lineStyle4 = doc.createElement("LineStyle");
 lineStyle4.appendChild(doc.createTextNode(color));
 polyStyle.appendChild(c4);
 arrowStyle.appendChild(polyStyle);
 document.appendChild(arrowStyle);

 for(int i = 0; i< segmentDataList.count(); i++)
 {
  sd = segmentDataList.at(i);
  document.appendChild(createPlacemark(sd.oneWay()));
  if(sd.oneWay().toLower() == "y")
  {
   document.appendChild(createArrow(sd));
  }


 }
 root.appendChild(document);
 writeXML(new QFile(fileName), doc);
 return true;
}

QDomElement Kml::createPlacemark(QString oneWay)
{
 QDomElement placemark = doc.createElement("Placemark");
 QDomElement name = doc.createElement("name" );
 name.appendChild(doc.createTextNode(sd.description()));
 placemark.appendChild(name);
 if(oneWay.toLower() != "y")
 {
  QDomElement styleUrl = doc.createElement("styleUrl");
  styleUrl.appendChild(doc.createTextNode("#wideRed"));
  placemark.appendChild(styleUrl);
 }
 else
 {
  QDomElement styleUrl = doc.createElement("styleUrl");
  styleUrl.appendChild(doc.createTextNode("#narrowRed"));
  placemark.appendChild(styleUrl);
 }
 QDomElement lineString = doc.createElement("LineString");
 QDomElement altitudeMode = doc.createElement("altitudeMode");
 altitudeMode.appendChild(doc.createTextNode("relative"));
 lineString.appendChild(altitudeMode);
 QDomElement coordinates = doc.createElement("coordinates");
 QString coords;

  double lat = 0, lon = 0;
  bool bFirst = true;
  LatLng startPt =  LatLng();
  LatLng endPt =  LatLng();
//  for(int i=0; i < sg.data.count(); i++)
//  {
//   segmentData sd = sg.data.at(i);
//   startPt =  LatLng(sd.startLat, sd.startLon);
////   if (startPt.lat() < swPt.lat())
////    swPt.setLat(startPt.lat());
////   if (startPt.lat() > nePt.lat())
////    nePt.setLat( startPt.lat());
////   if (startPt.lon() < swPt.lon())
////    swPt.setLon( startPt.lon());
////   if (startPt.lon() > nePt.lon())
////    nePt.setLon( startPt.lon());
////   bBoundsValid = true;
//   endPt =  LatLng(sd.endLat, sd.endLon);
//   lat = sd.endLat;
//   lon = sd.endLon;
//   if(bFirst)
//   {
////    infoLat = lat;
////    infoLon = lon;
//    bFirst = false;
//   }
////   if (lat < swPt.lat())
////    swPt.setLat( lat);
////   if (lat > nePt.lat())
////    nePt.setLat(lat);
////   if (lon < swPt.lon())
////    swPt.setLon( lon);
////   if (lon > nePt.lon())
////    nePt.setLon( lon);
//   //points.append(sd.startLat);
//   //points.append(sd.startLon);
//   coords.append(QString("%1,%2,0\n").arg(sd.startLon,0,'f',14).arg(sd.startLat,0,'f',14));
//  }
//  points.append(lat);
//  points.append(lon);
//  coords.append(QString("%1,%2,0\n").arg(lon,0,'f',14).arg(lat,0,'f',14));

  coordinates.appendChild(doc.createTextNode(sd.pointsString()));
  placemark.appendChild(lineString);
  lineString.appendChild(coordinates);


 return placemark;
}

/**
 * Create the Document object to store a particular root Element.
 *
 * @param root Root element of the final document
 * @param dtd name of an external DTD
 * @return new Document, with root installed
 */
 /*public*/ QDomDocument Kml::newDocument(QDomElement root)
{
 doc = QDomDocument(root.tagName());
 //doc.setDocType(new DocType(root.getName(), dtd));
 QDomProcessingInstruction xmlProcessingInstruction = doc.createProcessingInstruction("xml", "version=\"1.0\"  encoding=\"UTF-8\"");
 doc.appendChild(xmlProcessingInstruction);
//    xmlProcessingInstruction =  doc.createProcessingInstruction("xml-stylesheet","type=\"text/xsl\" href=\"/xml/XSLT/panelfile-2-9-6.xsl");
//    doc.appendChild(xmlProcessingInstruction);
 addDefaultInfo(root);
 return doc;
}

/**
 * Add default information to the XML before writing it out. <P> Currently,
 * this is identification information as an XML comment. This includes: <UL>
 * <LI>The JMRI version used <LI>Date of writing <LI>A CVS id string, in
 * case the file gets checked in or out </UL> <P> It may be necessary to
 * extend this to check whether the info is already present, e.g. if
 * re-writing a file.
 *
 * @param root The root element of the document that will be written.
 */
/*static*/ /*public*/ void Kml::addDefaultInfo(QDomElement root) {
    QString content = tr("Written by JMRI version ") + /*jmri.Version.name()*/ "3.11"
            + " on " + QDate::currentDate().toString();
            + " $Id: XmlFile.java 22548 2013-01-19 20:26:03Z rhwood $";
    //Comment comment = new Comment(content);
    QDomElement comment = doc.createElement("comment");
    comment.appendChild(doc.createTextNode(content));
    root.appendChild(comment);
}
/**
 * Write a File as XML.
 *
 * @throws org.jdom.JDOMException
 * @throws FileNotFoundException
 * @param file File to be created.
 * @param doc Document to be written out. This should never be NULL.
 */
/*public*/ void Kml::writeXML(QFile* file, QDomDocument doc) //throw (FileNotFoundException)
{
 QFileInfo info(file->fileName());
 // ensure parent directory exists
 if(!QDir(info.canonicalPath()).exists())
  return;
 // write the result to selected file
 //if(!file->isOpen())
  file->open(QIODevice::WriteOnly);
 QTextStream stream (file);
//    try {
//        XMLOutputter fmt = new XMLOutputter();
//        fmt.setFormat(Format.getPrettyFormat());
//        fmt.output(doc, o);
//    } finally {
//        o.close();
//    }
// stream.setCodec("UTF-8");
 QString sXml = doc.toString();
 doc.save(stream,2);
}

QDomElement Kml::createArrow(SegmentData si)
{
 QDomElement placemark = doc.createElement("Placemark");
 QDomElement name = doc.createElement("name" );
 name.appendChild(doc.createTextNode("arrow"));
 placemark.appendChild(name);

 QDomElement styleUrl = doc.createElement("styleUrl");
 styleUrl.appendChild(doc.createTextNode("#arrow"));
 placemark.appendChild(styleUrl);

 int len = sd.pointList().count();
 LatLng startPt;
 LatLng endPt;

 //segmentData sd = sg.data.at(len-1);
 //startPt =  LatLng(sd.startLat, sd.startLon);
 startPt = sd.pointList().last();
 //endPt =  LatLng(sd.endLat, sd.endLon);
 endPt = sd.pointList().at(len-2);

 Bearing b(startPt, endPt );
 LatLng left = pointRadialDistance(startPt, b.getBearing()-15, .020);
 LatLng right = pointRadialDistance(startPt, b.getBearing()+15, .020);

 QDomElement polygon = doc.createElement("Polygon");
 QDomElement extrude = doc.createElement("extrude");
 extrude.appendChild(doc.createTextNode("1"));
 polygon.appendChild(extrude);
 QDomElement altitudeMode = doc.createElement("altitudeMode");
 altitudeMode.appendChild(doc.createTextNode("RelativeToGround"));
 polygon.appendChild(altitudeMode);
 QDomElement outerBoundaryIs = doc.createElement("outerBoundaryIs");
 polygon.appendChild(outerBoundaryIs);
 QDomElement linearRing = doc.createElement("LinearRing");
 outerBoundaryIs.appendChild(linearRing);
 QDomElement coordinates = doc.createElement("coordinates");
 QString coord;
 coord.append(QString("            %1,%2,%3\n").arg(left.lon(),0,'f',14).arg(left.lat(),0,'f',14).arg(100));
 coord.append(QString("            %1,%2,%3\n").arg(startPt.lon(),0,'f',14).arg(startPt.lat(),0,'f',14).arg(100));
 coord.append(QString("            %1,%2,%3\n").arg(right.lon(),0,'f',14).arg(right.lat(),0,'f',14).arg(100));
 coord.append(QString("            %1,%2,%3\n").arg(left.lon(),0,'f',14).arg(left.lat(),0,'f',14).arg(100));
 coordinates.appendChild(doc.createTextNode(coord));
 linearRing.appendChild(coordinates);
 placemark.appendChild(polygon);
 return placemark;
}

LatLng Kml::pointRadialDistance(LatLng start, double bearing, double inDistance)
{
 double dToRad = 0.0174532925;
 double rEarth = 6371.01;  // Earth's average radius in km
 double epsilon = 0.000001;  // threshold for floating-point equality

 double degrees = bearing;
 while (degrees < -180) degrees += 360;
 while (degrees > 180) degrees -= 360;

 // convert the angle to radians
 double lat1 = start.lat() * dToRad;
 double lon1 = start.lon() * dToRad;
 double rbrng = degrees * dToRad;
 double rd = inDistance / rEarth;    // normalize linear distance to radian angle

 double rLat = 0, rLon = 0;
 // http://www.movable-type.co.uk/scripts/latlong.html
 rLat = qAsin(qSin(lat1) * qCos(rd) +
         qCos(lat1) * qSin(rd) * qCos(rbrng));
 rLon = lon1 + qAtan2(qSin(rbrng) * qSin(rd) * qCos(lat1),
                            qCos(rd) - qSin(lat1) * qSin(rLat));
 //alert("degrees " + degrees + " rLat " + rLat + " rLon " + rLon);
 return LatLng(rLat / dToRad, rLon / dToRad);
}
