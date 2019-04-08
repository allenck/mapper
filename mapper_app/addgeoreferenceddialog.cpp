#include "addgeoreferenceddialog.h"
#include "ui_addgeoreferenceddialog.h"
#include "configuration.h"
#include "data.h"
#include <qcompleter.h>
#include <QPushButton>

AddGeoreferencedDialog::AddGeoreferencedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AddGeoreferencedDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 bounds = new Bounds();

 connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_buttonBoxAccepted()));
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
  ov = new Overlay(name);
 ov->bounds = bounds->toString();
 ov->minZoom = ui->sbMinZoom->value();
 ov->maxZoom = ui->sbMaxZoom->value();
 ov->source = ui->comboBox->currentText();
 ov->description = ui->description->toHtml();
 QString s = ui->edUrl->toPlainText();
 QStringList l = s.split("\n");
 ov->urls = l;
 config->overlayList.insert(name, ov);
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
 validateValues();
}

void AddGeoreferencedDialog::on_nameTextEdited(QString txt)
{
 QCompleter* completer = new QCompleter(overlayNames);
 ui->edName->setCompleter(completer);
}

void AddGeoreferencedDialog::validateValues()
{
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
 if(!bounds->isValid())
 {
  ui->lblErr->setText(tr("Bounds are not valid"));
  return;
 }
 QString txt = ui->edUrl->toPlainText();
 if(txt == "" && ui->comboBox->currentText()== "georeferencer")
 {
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
   if(s == "") continue;
   if((s.endsWith("/{z}/{x}/{y}.png") || s.endsWith("/{z}/{x}/{y}.jpg"))  && s.startsWith("http://"))
    continue;
   ui->lblErr->setText(tr("The url(s) are invalid." ));
   return;
  }
 }
 else
  ui->edUrl->setEnabled(false);

 ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void AddGeoreferencedDialog::on_sourceChanged(QString)
{
 validateValues();
}
