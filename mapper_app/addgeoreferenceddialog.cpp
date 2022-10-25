#include "addgeoreferenceddialog.h"
#include "ui_addgeoreferenceddialog.h"
#include "configuration.h"
#include "data.h"
#include <qcompleter.h>
#include <QPushButton>
#include <QUrl>
#include <QDomDocument>

AddGeoreferencedDialog::AddGeoreferencedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AddGeoreferencedDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 bounds = new Bounds();

 //connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_buttonBoxAccepted()));
 connect(ui->edName, SIGNAL(editingFinished()), this, SLOT(on_nameEditingFinished()));
 connect(ui->edName, SIGNAL(textEdited(QString)), this, SLOT(on_nameTextEdited(QString)));
 ui->swLat->setValidator(new QDoubleValidator(-90, 90, 11));
 ui->swLon->setValidator(new QDoubleValidator(-180, +180, 11));
 ui->neLat->setValidator(new QDoubleValidator(-90, 90, 11));
 ui->neLon->setValidator(new QDoubleValidator(-180, +180, 11));
 connect(ui->swLat, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->swLon, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->neLat, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->neLon, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->sbMinZoom, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->sbMaxZoom, SIGNAL(editingFinished()), this, SLOT(validateValues()));
 connect(ui->edUrl, SIGNAL(textChanged()), this, SLOT(validateValues()));
 connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_sourceChanged(QString)));
 connect(ui->rbWMTS, &QRadioButton::clicked, [=]{
  if(wmtsUrl.isValid())
  {
   ui->edUrl->setPlainText(wmtsUrl.toString());
   validateValues();
  }
 });
 connect(ui->rbXYZ, &QRadioButton::clicked, [=]{
  if(!url.isEmpty())
  {
   ui->edUrl->setPlainText(url);
   validateValues();
  }
 });
 overlayNames = config->overlayList.keys();
 foreach(QString s, config->localOverlayList)
 {
  if(!overlayNames.contains(s))
   overlayNames.append(s);
 }
 ui->edUrl->setPlaceholderText(tr("enter list of tile server urls."));
#ifndef WIN32
 ui->comboBox->addItem("tileserver");
#endif
}

AddGeoreferencedDialog::~AddGeoreferencedDialog()
{
 delete ui;
}

void AddGeoreferencedDialog::on_buttonBoxAccepted()
{
 QString name = ui->edName->text();
 Overlay* ov;
 if(config->overlayList.keys().contains(name))
  ov = config->overlayList.value(name);
 else
 {
  ov = new Overlay(name);
  config->overlayList.insert(name, ov);
 }
 ov->cityName = config->currCity->name;
 ov->bounds = Bounds(LatLng(ui->swLat->text().toDouble(), ui->swLon->text().toDouble()), LatLng(ui->neLat->text().toDouble(), ui->neLon->text().toDouble()));
 ov->minZoom = ui->sbMinZoom->value();
 ov->maxZoom = ui->sbMaxZoom->value();
 ov->source = ui->comboBox->currentText();
 ov->description = ui->description->toHtml();
 QString s = ui->edUrl->toPlainText();
 QStringList l = s.split("\n");
 ov->urls = l;
 config->currCity->overlayMap.insert(ov->name, ov);
}

void AddGeoreferencedDialog::checkBounds()
{
 double swLat = ui->swLat->text().toDouble();
 double swLon = ui->swLon->text().toDouble();

 double neLat = ui->neLat->text().toDouble();
 double neLon = ui->neLon->text().toDouble();
 bounds = new Bounds(LatLng(swLat, swLon), LatLng(neLat, neLon));
 if(!bounds->swPt().isValid())
 {
  ui->lblErr->setText(tr("SW latitude and longitude not valid"));
  return;
 }
 if(!bounds->nePt().isValid())
 {
  ui->lblErr->setText(tr("NE latitude and longitude not valid"));
  return;
 }

 if(bounds->isValid())
  qDebug() << "bounds valid";
}

void AddGeoreferencedDialog::on_nameEditingFinished()
{
 QString name = ui->edName->text();
 if(config->overlayList.keys().contains(name))
 {
  Overlay* ov = config->overlayList.value(name);
  if(ov->name == name)
  {
   ui->sbMinZoom->setValue(ov->minZoom);
   ui->sbMaxZoom->setValue(ov->maxZoom);

   ui->swLat->setText(QString::number(ov->bounds.swPt().lat(), 'g',11));
   ui->swLon->setText(QString::number(ov->bounds.swPt().lon(), 'g',11));
   ui->neLat->setText(QString::number(ov->bounds.nePt().lat(), 'g',11));
   ui->neLon->setText(QString::number(ov->bounds.nePt().lon(), 'g',11));
   checkBounds();
   ui->comboBox->setCurrentText(ov->source);
   if(ov->source == "georeferencer")
    ui->edUrl->setEnabled(true);
   QString txt;
   foreach (QString s, ov->urls)
   {
    txt.append(s + "\n");
   }
   ui->edUrl->setText(txt);
   ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Update"));
   ui->description->setHtml(ov->description);
   ui->comboBox->setCurrentIndex(ui->comboBox->findText(ov->source));
   bUpdate = true;
  }
  else
  {
   ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Add"));
   bUpdate = false;
  }
 }
// else
// {
//     ui->sbMinZoom->setValue(0);
//     ui->sbMaxZoom->setValue(15);

//     ui->swLat->setText("");
//     ui->swLon->setText("");
//     ui->neLat->setText("");
//     ui->neLon->setText("");
//     checkBounds();
//     ui->comboBox->setCurrentText("georeferencer");
//     ui->edUrl->setEnabled(true);
//     QString txt = "";
//     ui->edUrl->setText(txt);
     ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Add"));
//     ui->description->setHtml("");
// }
 validateValues();
}

void AddGeoreferencedDialog::on_nameTextEdited(QString txt)
{
 QCompleter* completer = new QCompleter(overlayNames);
 ui->edName->setCompleter(completer);
}

void AddGeoreferencedDialog::validateValues()
{
 ui->lblErr->setText("");
 wmtsUrl = QUrl(ui->edUrl->toPlainText());
 if(wmtsUrl.isValid())
 {
  downloader = new FileDownloader(wmtsUrl);
  connect(downloader, SIGNAL(downloaded()), SLOT(validateWMTS()));
  setCursor(Qt::WaitCursor);
  return;
 }

 ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
 ui->lblErr->setText("");
 if(ui->edName->text() == "" )
 {
  ui->lblErr->setText(tr("A name must be specified!"));
  return;
 }
 if(ui->sbMinZoom->value() > ui->sbMaxZoom->value())
 {
  ui->lblErr->setText(tr("Minimum zoom must be less than max zoom!"));
  return;
 }

 checkBounds();
 if(!bounds->checkValid())
 {
  ui->lblErr->setText(tr("Bounds are not valid"));
  return;
 }
 QString txt = ui->edUrl->toPlainText();
 if(txt == "" && ui->comboBox->currentText()== "georeferencer")
 {
  ui->edUrl->setEnabled(true);
  ui->lblErr->setText(tr("One or more valid urls must be specified"));
  return;
 }
 if(ui->comboBox->currentText()== "georeferencer")
 {
  ui->edUrl->setEnabled(true);
  QStringList l = txt.split("\n");
  foreach (QString s, l)
  {
   s = s.trimmed();
   QUrl url(s);
   if(s == "") continue;
   if((s.contains("/{z}/{x}/{y}.png") || s.contains("/{z}/{x}/{y}.jpg"))
           && (s.startsWith("http://") || s.startsWith("https://"))&& url.isValid())
    continue;
   ui->lblErr->setText(tr("The url(s) are invalid." ));
   return;
  }
 }
 else
  ui->edUrl->setEnabled(false);

 ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void AddGeoreferencedDialog::validateWMTS()
{
 QDomDocument doc;
 QStringList points;
 Overlay* ov = new Overlay();

 doc.setContent(downloader->downloadedData());
 QDomElement root = doc.documentElement();
 if(root.isNull())
  return;
 if(root.tagName() == "Capabilities")
 {
  QDomElement contents = root.firstChildElement("Contents");
  if(!contents.isNull())
  {
   QDomElement layer = contents.firstChildElement("Layer");
   if(!layer.isNull())
   {
    ov->name = layer.firstChildElement("ows:Title").text();
    LatLng sw;
    LatLng ne;
    QDomElement bounds = layer.firstChildElement("ows:WGS84BoundingBox");
    if(!bounds.isNull())
    {
     QDomElement swCorner = bounds.firstChildElement("ows:LowerCorner");
     QString lon_lat = swCorner.text();
     points = lon_lat.split(" ");
     sw = LatLng(points.at(1).toDouble(), points.at(0).toDouble());
     QDomElement neCorner = bounds.firstChildElement("ows:UpperCorner");
     lon_lat = neCorner.text();
     points = lon_lat.split(" ");
     ne = LatLng(points.at(1).toDouble(), points.at(0).toDouble());
     ov->bounds = Bounds(sw, ne);

     // calculate min/max zoom
     ov->maxZoom = -1;
     ov->minZoom = 32767;
     QDomElement tileMatrixSetLink = layer.firstChildElement("TileMatrixSetLink");
     if(!tileMatrixSetLink.isNull())
     {
      QDomElement tileMatrixSetLimits = tileMatrixSetLink.firstChildElement("TileMatrixSetLimits");
      if(!tileMatrixSetLimits.isNull())
      {
       QDomNodeList list = tileMatrixSetLimits.elementsByTagName("TileMatrixLimits");
       if(list.count() > 0)
       {
         QDomElement tileMatrix_first = list.at(0).toElement();
         QString min = tileMatrix_first.firstChildElement("TileMatrix").text();
         ov->minZoom = min.mid(min.indexOf(":")+1).toInt();

         QDomElement tileMatrix_last = list.at(list.count()-1).toElement();

         QString max = tileMatrix_last.firstChildElement("TileMatrix").text();
         ov->maxZoom = max.mid(max.indexOf(":")+1).toInt();
        }
      }
     }
     QDomElement resourceUrl = layer.firstChildElement("ResourceURL");
     if(!resourceUrl.isNull())
     {
      //https://maps.georeferencer.com/georeferences/88523211-86cb-58d9-ae3a-ed9bf52a7cfe/2021-11-29T17:47:46.800140Z/map/{z}/{x}/{y}.png?key=caj1mpUbIDuRGkUmcxkG
      QString url = resourceUrl.attribute("template");
      url = url.replace("{TileMatrix}/{TileCol}/{TileRow}", "{z}/{x}/{y}");
      ov->urls.append(url);
     }
    }
   }
  }
 }
 ov->cityName = config->currCity->name;
 ov->wmtsUrl = wmtsUrl;
 ov->isSelected = false;
 ui->edName->setText(ov->name);
 ui->rbXYZ->setChecked(true);
 ui->neLat->setText(QString::number(ov->bounds.nePt().lat(),'g', 8));
 ui->neLon->setText(QString::number(ov->bounds.nePt().lon(),'g', 8));
 ui->swLat->setText(QString::number(ov->bounds.swPt().lat(),'g', 8));
 ui->swLon->setText(QString::number(ov->bounds.swPt().lon(),'g', 8));
 ui->edUrl->setText(ov->urls.at(0));
 ui->sbMinZoom->setValue(ov->minZoom);
 ui->sbMaxZoom->setValue(ov->maxZoom);
 ui->comboBox->setCurrentText("georeferencer");
 setCursor(Qt::ArrowCursor);
 ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
 config->currCity->addOverlay(ov);
 emit overlayAdded(ov);
 config->saveSettings();
}

void AddGeoreferencedDialog::on_sourceChanged(QString)
{
 validateValues();
}
