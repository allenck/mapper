#include "editcitydialog.h"
#include "ui_editcitydialog.h"
#include "configuration.h"
//#include "addoverlaydialog.h"
#include "QMessageBox"
#include "overlaytablemodel.h"
#include <QSortFilterProxyModel>
#include <QMessageBox>


EditCityDialog::EditCityDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditCityDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 cityOverlays = new QHash<QString, Overlay*>();
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
 bRefreshing = false;

 //connect(ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));

 for(int i=0; i < config->cityList.count(); i++)
 {
  City* c = config->cityList.at(i);
  ui->cbCity->addItem(c->name);
  if(config->currCity->name == c->name)
  {
   ui->cbCity->setCurrentIndex(i);
   cbCitysSelectionChanged(i);
  }
 }
 connect(ui->cbCity, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitysSelectionChanged(int)));
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));
 connect(ui->edDescription, SIGNAL(textChanged()), this, SLOT(edDescriptionTextChanged()));
 connect(ui->sbMaxZoom, SIGNAL(valueChanged(int)), this, SLOT(sbMaxZoomValueChanged(int)));
 connect(ui->sbMinZoom, SIGNAL(valueChanged(int)), this, SLOT(sbMinZoomValueChanged(int)));
 connect(ui->sbOpacity, SIGNAL(valueChanged(int)), this,SLOT(sbOpacityValueChanged(int)));
 connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(ok_clicked()));
 setControls(false); // will be set to true when table selected.
}

EditCityDialog::~EditCityDialog()
{
 delete ui;
}
void EditCityDialog::newCity(int i)
{
 cityOverlays->clear();
 foreach(Overlay*ov, config->currCity->overlays)
 {
  cityOverlays->insert(ov->name, ov);
 }

 model->setCity(i, cityOverlays);
}

void EditCityDialog::cbCitysSelectionChanged(int i)
{
 if(dirty)
 {
  if(QMessageBox::question(this, tr("Save City?"), tr("One or more overlays have been added or modified. Do you wish to save %1").arg(c->name), QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
  {
   c->overlays = cityOverlays->values();
  }
 }
 c = config->cityList.at(i);
 cityId = i;
 newCity(i);
 model->setCity(i, cityOverlays);
 dirty = false;
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
 foreach (Overlay* ov1, c->overlays) {
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
 foreach (Overlay* ov1, c->overlays)
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
   cityOverlays->insert(ov->name, ov);
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
  for(int i = 0; i < c->overlays.count(); i++)
  {
   if(c->overlays.at(i)->name == newOv->name)
   {
    QMessageBox::critical(this, tr("Error"), tr("Overlay %1 is already used").arg(newOv->name));
    return;
   }
  }
  c->overlays.append(newOv);
  if(c->curOverlayId < 0)
   c->curOverlayId = 0;
  refreshOverlayCb();
  //cbOverlaySelectionChanged(c->overlays.count()-1);
 }
}

void EditCityDialog::btnDeleteFromCityClicked()
{
 for(int i = 0; i < c->overlays.count(); i++)
 {
//  if(c->overlays.at(i)->name == ui->cbOverlay->currentText())
//  {
//   c->overlays.removeAt(i);
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
 if(b && c->curOverlayId >=0)
  c->overlays.at(c->curOverlayId)->description = ui->edDescription->toHtml();
}

void EditCityDialog::ok_clicked()
{
 if(dirty)
 {
  c->overlays = cityOverlays->values();
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
