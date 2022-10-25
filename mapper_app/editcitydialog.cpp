#include "editcitydialog.h"
#include "ui_editcitydialog.h"
#include "configuration.h"
//#include "addoverlaydialog.h"
#include "QMessageBox"
#include "overlaytablemodel.h"
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QDebug>
#include "htmldelegate.h"
#include <QCloseEvent>
#include <QPushButton>
#include "addgeoreferenceddialog.h"

EditCityDialog::EditCityDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditCityDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 cityOverlays = new QMap<QString, Overlay*>();
 foreach(Overlay* ov, config->currCity->overlayMap)
  cityOverlays->insert(ov->name, ov);

 dirty = false;

 sorter = new QSortFilterProxyModel();
 sorter->setSourceModel(model = new OverlayTableModel());
 connect(model, SIGNAL(setDirty()), this, SLOT(on_setDirty()));
 connect(model, SIGNAL(overlaySelectionChanged(QModelIndex,bool)), this, SLOT(overlaySelectionChanged(QModelIndex,bool)));
 ui->tableView->setModel(sorter);
 ui->tableView->setSortingEnabled(true);
 ui->tableView->setAlternatingRowColors(true);
 ui->tableView->resizeColumnsToContents();
 ui->tableView->setColumnWidth(OverlayTableModel::NAME, 500);
 ui->tableView->setColumnWidth(OverlayTableModel::DESCRIPTION, 300);
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::DESCRIPTION, new HtmlDelegate());
 bRefreshing = false;

 QPushButton* btnAddOverlay = new QPushButton(tr("Add Overlay"));
 ui->buttonBox->addButton(btnAddOverlay, QDialogButtonBox::ActionRole);
 connect(btnAddOverlay, &QPushButton::clicked, [=]{
  AddGeoreferencedDialog* dlg = new AddGeoreferencedDialog(this);
  connect(dlg, &AddGeoreferencedDialog::overlayAdded, [=](Overlay* ov){
   model->addOverlay(ov);
  });
  dlg->exec();
 });
 QPushButton* okButton = new QPushButton(tr("Ok"));
 ui->buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
 connect(okButton, SIGNAL(clicked()), this, SLOT(ok_clicked()));
 QPushButton* applyButton =new QPushButton(tr("Apply"));
 ui->buttonBox->addButton(applyButton, QDialogButtonBox::ActionRole);
 connect(applyButton, SIGNAL(clicked()), this, SLOT(apply_clicked()));


 //connect(ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));
 connect(ui->cbCity, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitysSelectionChanged(int)));

 for(int i=0; i < config->cityList.count(); i++)
 {
  City* city = config->cityList.at(i);
  ui->cbCity->addItem(city->name);
  if(config->currCity->name == city->name)
  {
   ui->cbCity->setCurrentIndex(i);
   cbCitysSelectionChanged(i);
  }
 }
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));
 connect(ui->edDescription, SIGNAL(textChanged()), this, SLOT(edDescriptionTextChanged()));
 connect(ui->sbMaxZoom, SIGNAL(valueChanged(int)), this, SLOT(sbMaxZoomValueChanged(int)));
 connect(ui->sbMinZoom, SIGNAL(valueChanged(int)), this, SLOT(sbMinZoomValueChanged(int)));
 connect(ui->sbOpacity, SIGNAL(valueChanged(int)), this,SLOT(sbOpacityValueChanged(int)));
 connect(ui->editLatitude, SIGNAL(editingFinished()), this, SLOT(onLatitudeChanged()));
 connect(ui->editLongitude, SIGNAL(editingFinished()), this, SLOT(onLongitudeChanged()));
 setControls(false); // will be set to true when table selected.

 QSettings settings;
 QSize sz = settings.value("EditCityDialog:size",QSize(800,600)).toSize();
 resize(sz);
}

EditCityDialog::~EditCityDialog()
{
 delete ui;
}

void EditCityDialog::newCity(int i)
{
 cityOverlays->clear();
 foreach(Overlay*ov, config->currCity->overlayMap)
 {
  if(ov->bounds.contains( config->currCity->bounds) || config->currCity->bounds.contains(ov->bounds))
  {
   cityOverlays->insert(ov->name, ov);
   qDebug() << config->currCity->name << " bounds: " << config->currCity->bounds.toString();
   qDebug() << ov->name << " bounds: " << ov->bounds.toString();
  }
  else
  {
   qDebug() << "bypass:";
   qDebug() << config->currCity->name << " bounds: " << config->currCity->bounds.toString();
   qDebug() << ov->name << " bounds: " << ov->bounds.toString();

  }
 }

 model->setCity(i);
}

void EditCityDialog::cbCitysSelectionChanged(int i)
{
 if(dirty)
 {
  if(QMessageBox::question(this, tr("Save City?"), tr("One or more overlays have been added or modified. Do you wish to save %1").arg(city->name), QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
  {
   //city->overlayMap = cityOverlays->values();
   city->setDirty(true);
  }
 }
 city = config->cityList.at(i);
 cityId = i;
 newCity(i);
 model->setCity(i);
 dirty = false;
 ui->editLatitude->setText(QString::number(city->center.lat()));
 ui->editLongitude->setText(QString::number(city->center.lon()));
}

void EditCityDialog::onClicked(QModelIndex index)
{
 int row = sorter->mapToSource(index).row();
 ui->tableView->selectRow(row);
 setControls(true);
 ov = config->overlayList.values().at(row);
 ui->edDescription->clear();
 ui->edDescription->setHtml(ov->description);
 ui->sbOpacity->setValue(ov->opacity);

 ui->sbMinZoom->setMinimum(ov->minZoom);
 ui->sbMinZoom->setMaximum(ov->maxZoom);
 ui->sbMinZoom->setValue(ov->minZoom);

 ui->sbMaxZoom->setMinimum(ov->minZoom);
 ui->sbMaxZoom->setMaximum(ov->maxZoom);
 ui->sbMaxZoom->setValue(ov->maxZoom);
 foreach (Overlay* ov1, city->overlayMap) {
  if(ov->name == ov1->name)
  {
   if(ov1->description != "")
   {
    ui->edDescription->setHtml(ov1->description);
    if(ui->edDescription->toPlainText() == "")
     ui->edDescription->setHtml(ov->description);

    dirty = true;
   }
  }
  ui->sbOpacity->setValue(ov->opacity);
  ui->sbMinZoom->setValue(ov->minZoom);
  ui->sbMaxZoom->setValue(ov->maxZoom);
 }

}

void EditCityDialog::overlaySelectionChanged(QModelIndex mindex, bool bCheck)
{
 //QModelIndex ix = sorter->mapToSource(mindex);

 int row = mindex.row();
 ui->tableView->selectRow(row);
 bRefreshing = true;
 QString name = model->data(model->index(row,OverlayTableModel::NAME),Qt::DisplayRole).toString();
 //bool checked = model->data(model->index(row,OverlayTableModel::SELECTED),Qt::DisplayRole).toBool();
 if(!bCheck)
 {
  ui->edDescription->clear();
  setControls(false);
 }
 setControls(true);
 ov = config->overlayList.values().at(row);
 ui->edDescription->clear();
 ui->edDescription->setHtml(ov->description);
 ui->sbOpacity->setValue(ov->opacity);

 ui->sbMinZoom->setMinimum(ov->minZoom);
 ui->sbMinZoom->setMaximum(ov->maxZoom);
 ui->sbMinZoom->setValue(ov->minZoom);

 ui->sbMaxZoom->setMinimum(ov->minZoom);
 ui->sbMaxZoom->setMaximum(ov->maxZoom);
 ui->sbMaxZoom->setValue(ov->maxZoom);
 foreach (Overlay* ov1, city->overlayMap.values())
 {
  if(ov->name == ov1->name)
  {
   if(ov1->description != "")
   {
    ui->edDescription->setHtml(ov1->description);
    if(ui->edDescription->toPlainText() == "")
     ui->edDescription->setHtml(ov->description);
    dirty = true;
   }
  }
  ui->sbOpacity->setValue(ov->opacity);
  ui->sbMinZoom->setValue(ov->minZoom);
  ui->sbMaxZoom->setValue(ov->maxZoom);
 }
 if(bCheck)
 {
  if(!cityOverlays->contains(ov->name))
  {
       //cityOverlays->insert(ov->name, ov);
      if(ov->bounds.contains( config->currCity->center))
      {
       cityOverlays->insert(ov->name, ov);
       qDebug() << config->currCity->name << " center: " << config->currCity->center.toString();
       qDebug() << ov->name << " bounds: " << ov->bounds.toString();
      }
  }
 }
 else
 {
  if(cityOverlays->contains(ov->name))
   cityOverlays->remove(ov->name);
 }
 bRefreshing = false;
}

void EditCityDialog::edDescriptionTextChanged()
{
 if(bRefreshing) return;
 if(ui->edDescription->toPlainText() != "")
 {
  ov->description = ui->edDescription->toHtml();
  dirty = true;
 }
}

void EditCityDialog::sbOpacityValueChanged(int value)
{
 ov->opacity = value;
 dirty = true;
}
void EditCityDialog::sbMinZoomValueChanged(int value)
{
 ov->minZoom = value;
 dirty = true;
}

void EditCityDialog::sbMaxZoomValueChanged(int value)
{
 ov->maxZoom = value;
 dirty = true;
}
#if 0
void EditCityDialog::btnAddToCityClicked()
{
 AddOverlayDialog* dlg = new AddOverlayDialog();
 if(dlg->exec() == QDialog::Accepted)
 {
  Overlay* newOv = dlg->overlay();
  for(int i = 0; i < city->overlays.count(); i++)
  {
   if(city->overlays.at(i)->name == newOv->name)
   {
    QMessageBox::critical(this, tr("Error"), tr("Overlay %1 is already used").arg(newOv->name));
    return;
   }
  }
  city->overlays.append(newOv);
  if(city->curOverlayId < 0)
   city->curOverlayId = 0;
  refreshOverlayCb();
  //cbOverlaySelectionChanged(city->overlays.count()-1);
 }
}

void EditCityDialog::btnDeleteFromCityClicked()
{
 for(int i = 0; i < city->overlays.count(); i++)
 {
//  if(city->overlays.at(i)->name == ui->cbOverlay->currentText())
//  {
//   city->overlays.removeAt(i);
//   break;
//  }
 }
 refreshOverlayCb();
 ui->btnDeletefromCity->setEnabled(false);
 ui->btnAddToCity->setEnabled(true);
}
#endif
void EditCityDialog::OnDescriptionChanged(bool b)
{
 if(bRefreshing) return;
 if(city->curOverlayId >= city->overlayMap.count())
 if(b && city->curOverlayId >= 0)
  city->curOverlayId = 0;
 city->overlayMap.values().at(city->curOverlayId)->description = ui->edDescription->toHtml();
}

void EditCityDialog::ok_clicked()
{
 if(dirty)
 {
  QSettings settings;
  settings.setValue("EditCityDialog:size", size());
  dirty = false;
 }

 accept();
}

void EditCityDialog::apply_clicked()
{
 if(dirty)
 {
  QSettings settings;
  settings.setValue("EditCityDialog:size", size());
  dirty = false;
 }

}

void EditCityDialog::setControls(bool enabled)
{
 ui->edDescription->setEnabled(enabled);
 ui->sbMinZoom->setEnabled(enabled);
 ui->sbMaxZoom->setEnabled(enabled);
 ui->sbOpacity->setEnabled(enabled);
}

void EditCityDialog::on_setDirty()
{
 dirty = true;
}

void EditCityDialog::onLatitudeChanged()
{
 QString txt = ui->editLatitude->text();
 double val = txt.toDouble();
 city->center.setLat(val);
 dirty = true;
}

void EditCityDialog::onLongitudeChanged()
{
 QString txt = ui->editLongitude->text();
 double val = txt.toDouble();
 city->center.setLon(val);
 dirty = true;
}

void EditCityDialog::closeEvent(QCloseEvent * event)
{
 QMessageBox msgBox;
 if(dirty)
 {
  if(QMessageBox::question(this, tr("Save City?"), tr("One or more overlays have been added or modified. Do you wish to save %1").arg(city->name), QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
  {
   //city->overlayMap = cityOverlays->values();
   city->setDirty(true);
  }
 }

 QSettings settings;
 settings.setValue("EditCityDialog:size", size());

 config->saveSettings();

#if 0
 msgBox.setText("Are you sure you want to close?");
 msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
 msgBox.setDefaultButton(QMessageBox::Close);
 int result = msgBox.exec();
 switch (result) {
   case QMessageBox::Close:
       event->accept();
       break;
   case QMessageBox::Cancel:
       event->ignore();
       break;
   default:
       QDialog::closeEvent(event);
       break;
 }
#else
 event->accept();
#endif
}
