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
#include "overlay.h"
#include <QModelIndexList>
#include <QItemSelection>
#include <QMenu>
#include <QClipboard>
#include "webviewbridge.h"
#include "mytextedit.h"
#include "lineeditdelegate.h"

EditCityDialog::EditCityDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditCityDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 cityOverlays = new QMap<QString, Overlay*>();
 foreach(Overlay* ov, *config->currCity->city_overlayMap)
  cityOverlays->insert(ov->name, ov);

 dirty = false;

 model = new OverlayTableModel(config->currentCityId);

 sorter = new QSortFilterProxyModel();
 sorter->setSourceModel(model = new OverlayTableModel(config->currentCityId));
 connect(model, SIGNAL(setDirty()), this, SLOT(on_setDirty()));
 //connect(model, SIGNAL(overlaySelectionChanged(Overlay*,bool)), this, SLOT(overlaySelectionChanged(Overlay*,bool)));
 ui->tableView->setModel(sorter);
 ui->tableView->setSortingEnabled(true);
 ui->tableView->setAlternatingRowColors(true);
 ui->tableView->resizeColumnsToContents();
 ui->tableView->setColumnWidth(OverlayTableModel::NAME, 500);
 ui->tableView->setColumnWidth(OverlayTableModel::DESCRIPTION, 300);
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::DESCRIPTION, new HtmlDelegate());
 ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
 ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::URLS, new LineEditDelegate());
 //connect(ui->tableView, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
 connect(ui->tableView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tablev_customContextMenu(QPoint)));

 bRefreshing = false;

 QPushButton* btnAddOverlay = new QPushButton(tr("Add Overlay"));
 ui->buttonBox->addButton(btnAddOverlay, QDialogButtonBox::ActionRole);
 connect(btnAddOverlay, &QPushButton::clicked, [=]{
  AddGeoreferencedDialog* dlg = new AddGeoreferencedDialog(this);
  int result = dlg->exec();
  if(result == QDialog::Accepted)
  {
   Overlay* ov = dlg->overlay();
   model->addOverlay(ov);
  }
 });
 QPushButton* okButton = new QPushButton(tr("Ok"));
 ui->buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
 connect(okButton, SIGNAL(clicked()), this, SLOT(ok_clicked()));
 QPushButton* applyButton =new QPushButton(tr("Apply"));
 ui->buttonBox->addButton(applyButton, QDialogButtonBox::ActionRole);
 connect(applyButton, SIGNAL(clicked()), this, SLOT(apply_clicked()));


 //connect(ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(rowSelected(QModelIndex)));
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));
 connect(ui->cbCity, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitysSelectionChanged(int)));

 for(int i=0; i < config->cityList.count(); i++)
 {
  City* city = config->cityList.at(i);
  ui->cbCity->addItem(city->name());
  if(config->currCity->name() == city->name())
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

 //Overlay::exportXml("./overlays.xml", config->overlayMap->values());
 pasteLatLng = new QAction(tr("Paste LatLng from Google Maps"),this);
 pasteLatLng->setToolTip(tr("paste latitude and longitude for city. Right clip on Google maps at point."));
 connect(pasteLatLng, SIGNAL(triggered(bool)), this, SLOT(on_pasteLatLng()));
 deleteConnection = new QAction(tr("delete city connection."),this);
 connect(deleteConnection, SIGNAL(triggered(bool)), this, SLOT(on_deleteConnection()));
 ui->cbCity->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(ui->cbCity, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(cbCity_customContextMenu(QPoint)));
}

EditCityDialog::~EditCityDialog()
{
 delete ui;
}

void EditCityDialog::newCity(int i)
{
 cityOverlays->clear();
 foreach(Overlay*ov, *config->currCity->city_overlayMap)
 {
  if(ov->bounds().contains( config->currCity->bounds())
     || config->currCity->bounds().contains(ov->bounds()))
  {
   cityOverlays->insert(ov->name, ov);
//   qDebug() << config->currCity->name() << " bounds: " << config->currCity->bounds().toString();
//   qDebug() << ov->name << " bounds: " << ov->bounds().toString();
  }
  else
  {
   qDebug() << "bypass:";
//   qDebug() << config->currCity->name() << " bounds: " << config->currCity->bounds().toString();
//   qDebug() << ov->name << " bounds: " << ov->bounds().toString();

  }
 }

 model->setCity(i);
}

void EditCityDialog::cbCitysSelectionChanged(int i)
{
 if(dirty)
 {
  if(QMessageBox::question(this, tr("Save City?"), tr("One or more overlays have been added or modified. Do you wish to save %1").arg(city->name()), QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
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


void EditCityDialog::overlaySelectionChanged(Overlay* ov, bool bCheck)
{
 if(ov == nullptr)
  throw NullPointerException("null pointer");
 if(!bCheck)
 {
  ui->edDescription->clear();
  setControls(false);
 }
 setControls(true);
 ui->edDescription->clear();
 ui->edDescription->setHtml(ov->description);
 if(!(ov->opacity>0 && ov->opacity <=65))
  ov->opacity = 65;
 ui->sbOpacity->setValue(ov->opacity);

 ui->sbMinZoom->setMinimum(ov->minZoom);
 ui->sbMinZoom->setMaximum(ov->maxZoom);
 ui->sbMinZoom->setValue(ov->minZoom);

 ui->sbMaxZoom->setMinimum(ov->minZoom);
 ui->sbMaxZoom->setMaximum(ov->maxZoom);
 ui->sbMaxZoom->setValue(ov->maxZoom);
// foreach (Overlay* ov1, city->overlayMap->values())
// {
//  if(ov->name == ov1->name)
//  {
//   if(ov1->description != "")
//   {
//    ui->edDescription->setHtml(ov1->description);
//    if(ui->edDescription->toPlainText() == "")
//     ui->edDescription->setHtml(ov->description);
//    dirty = true;
//   }
//  }
//  ui->sbOpacity->setValue(ov->opacity);
//  ui->sbMinZoom->setValue(ov->minZoom);
//  ui->sbMaxZoom->setValue(ov->maxZoom);
// }
 if(bCheck)
 {
  if(!cityOverlays->contains(ov->name))
  {
       //cityOverlays->insert(ov->name, ov);
      if(ov->bounds().contains( config->currCity->center))
      {
       cityOverlays->insert(ov->name, ov);
       qDebug() << config->currCity->name() << " center: " << config->currCity->center.toString();
       qDebug() << ov->name << " bounds: " << ov->bounds().toString();
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
 if(ov == nullptr) return;

 if(ui->edDescription->toPlainText() != "")
 {
  ov->description = ui->edDescription->toHtml();
  dirty = true;
 }
}

void EditCityDialog::sbOpacityValueChanged(int value)
{
 if(ov == nullptr) return;
 ov->opacity = value;
 dirty = true;
}
void EditCityDialog::sbMinZoomValueChanged(int value)
{
 if(ov == nullptr) return;

 dirty = true;
}

void EditCityDialog::sbMaxZoomValueChanged(int value)
{
 if(ov == nullptr) return;
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
 if(city->curOverlayId >= city->city_overlayMap->count())
 if(b && city->curOverlayId >= 0)
  city->curOverlayId = 0;
 if(city->curOverlayId >= 0)
  city->city_overlayMap->values().at(city->curOverlayId)->description = ui->edDescription->toHtml();
}

void EditCityDialog::ok_clicked()
{
 QMap<QString, Overlay*>* ovMap = model->getOverlayMap();

// for(int i= config->overlayMap->values().count()-1; i >= 0; i--)
// {
//  Overlay* ov = config->overlayMap->values().at(i);
//  if(ovMap->contains(ov->cityName + "|" + ov->name))
//  {
//   config->overlayMap->remove(ov->cityName + "|" + ov->name);
//   dirty = true;
//  }
//  if(!config->overlayMap->contains(ov->cityName + "|" + ov->name))
//  {
//   config->overlayMap->insert(ov->cityName + "|" + ov->name, ov);
//   dirty = true;
//  }
// }
 config->overlayMap = ovMap;
 //if(dirty)
 {
  QSettings settings;
  settings.setValue("EditCityDialog:size", size());
  config->saveSettings();
  Overlay::exportXml("./overlays.xml",config->overlayMap->values());
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
  if(QMessageBox::question(this, tr("Save City?"), tr("One or more overlays have been added or modified. Do you wish to save %1").arg(city->name()), QMessageBox::Yes | QMessageBox::No)== QMessageBox::Yes)
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
//create table input context menu
void EditCityDialog::tablev_customContextMenu( const QPoint& pt)
{
 // check is item in QTableView exist or not
 QTableView *view = qobject_cast<QTableView*>(ui->tableView);
 QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(view->model());

 currentIndexTableView = proxy->mapToSource(view->indexAt(pt));
// if(boolGetItemTableView(view))
// {
  //menu = QMenu(m_parent*);
  QAction * copyAction = new QAction("Copy cell text", this);
  connect(copyAction,SIGNAL(triggered()),this,SLOT(on_copyCellText()));
  QAction * pasteAction = new QAction("Paste",this);
  connect(pasteAction,SIGNAL(triggered()),this,SLOT(tableViewPaste()));

  //QClipboard *clip = QApplication::clipboard();
  QMenu menu;

  menu.addAction(copyAction);
  menu.addAction(pasteAction);
  QAction* deleteAction = new QAction(tr("Delete"), this);
  menu.addAction(deleteAction);
  connect(deleteAction, SIGNAL(triggered()), this, SLOT(onDeleteRow()));
  QAction* updateProperties = new QAction(tr("Update Properties"), this);
  connect(updateProperties, SIGNAL(triggered()), this, SLOT(onUpdateProperties()));
  menu.addAction(updateProperties);
  // more actions can be added here
  menu.exec(QCursor::pos());


// }
}//get QTableView selected item

bool EditCityDialog::boolGetItemTableView(QTableView *view)
{
 // get model from tableview
 QItemSelectionModel *selModel = view->selectionModel();
 if(selModel)
 {
  currentIndexTableView = selModel->currentIndex();
  return (true);
 }
 else                //QTableView doesn't have selected data
  return (false);
}

void EditCityDialog::on_copyCellText()
{

 QClipboard *clip = QApplication::clipboard();
 QString text = model->data(currentIndexTableView,Qt::DisplayRole).toString();
 clip->setText(text);
}

void ::EditCityDialog::tableViewPaste()
{
 QClipboard *clip = QApplication::clipboard();
 QString text = clip->text();
 model->setData(currentIndexTableView, text, Qt::EditRole );
}

void EditCityDialog::rowSelected(QModelIndex index)
{
 int row = sorter->mapToSource(index).row();
 Overlay* ov = model->getOverlayMap()->values().at(row);
 overlaySelectionChanged(ov,ov->isSelected);
}

void EditCityDialog::onDeleteRow()
{
 model->deleteRow(currentIndexTableView.row());

}

void EditCityDialog::onUpdateProperties()
{
 Overlay* ov = model->selectedOverlay(currentIndexTableView.row());
 setCursor(Qt::WaitCursor);
 connect(ov, &Overlay::xmlFinished, [=]{setCursor(Qt::ArrowCursor);});
 ov->getTileMapResource();
}
void EditCityDialog::cbCity_customContextMenu(QPoint pt)
{
    QMenu cityMenu;
    cityMenu.addAction(pasteLatLng);
    cityMenu.addAction(deleteConnection);
    cityMenu.exec(QCursor::pos());
}

void EditCityDialog::on_deleteConnection()
{

}

void EditCityDialog::on_pasteLatLng()
{
    const QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    QStringList sl = text.split(",");
    if(sl.count()== 2)
    {
        bool ok;
      double  latitude, longitude;
      latitude = sl.at(0).toDouble(&ok);
      if(!ok)   return;
      longitude = sl.at(1).toDouble(&ok);
      if(!ok)   return ;
      ui->editLatitude->setText(QString::number(latitude));
      ui->editLongitude->setText(QString::number(longitude));
      QVariantList objArray;
      objArray << latitude << longitude;
      WebViewBridge::instance()->processScript("setCenter", objArray);
      City* city = config->cityList.at(config->cityNames().indexOf(ui->cbCity->currentText()));
      if(city)
          city->setCenter(LatLng(latitude, longitude));
       WebViewBridge::instance()->processScript("addCityBoundsButton");
       objArray.clear();
       objArray << 12;
       WebViewBridge::instance()->processScript("setZoom", objArray); }
}
