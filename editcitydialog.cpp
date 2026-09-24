#include "editcitydialog.h"
#include "dialogselectlist.h"
#include "mainwindow.h"
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
#include "itemdelegate.h"
#include <QStatusBar>
#include "vptr.h"

EditCityDialog::EditCityDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EditCityDialog)
{
 ui->setupUi(this);
 config = Configuration::instance();
 cityOverlays = new QMap<QString, Overlay*>();
 foreach(Overlay* ov, *config->currCity->city_overlayMap)
  cityOverlays->insert(ov->name, ov);

 bDirty = false;

 geoserver = Geoserver::instance();
 //geoserver->getCapabilities("http://localhost:8080/geoserver");

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

 ui->tableView->setItemDelegateForColumn(OverlayTableModel::NAME, new LineEditDelegate());
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::LAYER, new LineEditDelegate());
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::URLS, new LineEditDelegate());
 ui->tableView->setItemDelegateForColumn(OverlayTableModel::CITYNAME, new ItemDelegate(config->cityNames()));
 QStringList sources;
 sources << "acksoft" << "acksoft2" << "georeferencer" << "georeferencer2" << "geoserver";

 ui->tableView->setItemDelegateForColumn(OverlayTableModel::SOURCE, new ItemDelegate(sources));
 //connect(ui->tableView, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
 connect(ui->tableView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tablev_customContextMenu(QPoint)));

 connect(model, &QAbstractItemModel::rowsInserted,
         this, [this](const QModelIndex &parent, int start, int end) {
             QModelIndex idx = model->index(end, 0);
             ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtBottom);
         });
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
 connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [=](QAbstractButton *button){
     QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
     if(role == QDialogButtonBox::RejectRole)
     {
         if(bDirty)
         {
             int rslt = QMessageBox::question(nullptr, tr("Cancel"), tr("Changes have been made. Are you sure you want to close and lose the changes"),
                                                                        QMessageBox::Yes | QMessageBox::No);
             if(rslt == QMessageBox::Yes)
             {
                 reject();
                 close();
             }
             accept();
         }
     }
 });

 //connect(ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
 connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(rowSelected(QModelIndex)));
 connect(ui->edDescription, SIGNAL(dirtySet(bool)), this, SLOT(OnDescriptionChanged(bool)));
 connect(ui->cbCity, SIGNAL(currentIndexChanged(int)),this, SLOT(cbCitysSelectionChanged(int)));
 //connect(model, SIGNAL(columnChanged(int,int,Overlay*,Overlay*,QVariant)), this, SLOT(onColumnChanged(int,int,Overlay*,Overlay*,QVariant)));
 connect(model, &OverlayTableModel::columnChanged,this, [=](int row,int column,Overlay* ovOld,Overlay* ov,QVariant value){
     onColumnChanged(row, column, ovOld,ov,value);
 });
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

 connect(qApp, &QCoreApplication::aboutToQuit, this, []() {
     qDebug() << "Application is about to close. Perform cleanup here.";
 });
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
 if(bDirty)
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
 bDirty = false;
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
  bDirty = true;
 }
}

void EditCityDialog::sbOpacityValueChanged(int value)
{
 if(ov == nullptr) return;
 ov->opacity = value;
 bDirty = true;
}
void EditCityDialog::sbMinZoomValueChanged(int value)
{
 if(ov == nullptr) return;

 bDirty = true;
}

void EditCityDialog::sbMaxZoomValueChanged(int value)
{
 if(ov == nullptr) return;
 bDirty = true;
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

  Overlay::exportXml("./Resources/overlays.xml",config->overlayMap->values());

  // rebuild each city's overlay list
  foreach (City* c, config->cityList) {
      c->city_overlayMap->clear();
      foreach (Overlay* ov, config->overlayMap->values()) {
          if(ov->isSelected)
              c->city_overlayMap->insert(ov->name, ov);
      }
  }
  bDirty = false;
 }

 accept();
}

void EditCityDialog::apply_clicked()
{
 if(bDirty)
 {
  QSettings settings;
  settings.setValue("EditCityDialog:size", size());
  bDirty = false;
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
 bDirty = true;
}

void EditCityDialog::onLatitudeChanged()
{
 QString txt = ui->editLatitude->text();
 double val = txt.toDouble();
 city->center.setLat(val);
 bDirty = true;
}

void EditCityDialog::onLongitudeChanged()
{
 QString txt = ui->editLongitude->text();
 double val = txt.toDouble();
 city->center.setLon(val);
 bDirty = true;
}

void EditCityDialog::closeEvent(QCloseEvent * event)
{
 QMessageBox msgBox;
 if(bDirty)
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

#if 1
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

 QMap<QString, Overlay *>* map = model->getOverlayMap();

 currentIndexTableView = proxy->mapToSource(view->indexAt(pt));
 Overlay* currOv = map->values().at(currentIndexTableView.row() );

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
  QAction* newLine = new QAction(tr("Add new row"),this);
  connect(newLine, SIGNAL(triggered(bool)),this, SLOT(onNewLine()));
  menu.addAction(newLine);
  QAction* showOverlay = new QAction(tr("Display overlay"),this);
  QActionGroup* ag = new QActionGroup(this);
  showOverlay->setData(VPtr<Overlay>::asQVariant(currOv));
  ag->addAction(showOverlay);
  //connect(ag, SIGNAL(triggered()),this, SLOT(displayOverlay(QAction*)));
  connect(ag, &QActionGroup::triggered,[=](QAction* act){
      displayOverlay(act);
  });
  QStringList cityNames = config->cityNames();
  if(currOv && cityNames.contains(currOv->cityName))
    menu.addAction(showOverlay);
  if(currOv->source == "geoserver")
  {
      QAction* queryWmts = new QAction(tr("request Wmts capabilities"),this);
       queryWmts->setData(VPtr<Overlay>::asQVariant(currOv));
      ag = new QActionGroup(this);
      ag->addAction(queryWmts);
      connect(ag, &QActionGroup::triggered,[=](QAction* act){
          currOv->importWmsCapabilities(currOv->url(), wmtsList);
      });
      connect(currOv, &Overlay::wmtsFinished,this,[=] (QList<Overlay*>* list)
      {
          if(list)
              qDebug() << "wmtsList count: " << list->count();
          QStringList workspaces;
          foreach (Overlay* o, *list) {
              if(o->_layerName.contains(':'))
              {
                  QString ws = o->_layerName.mid(0,o->_layerName.indexOf(':')+1);
                  if(!workspaces.contains(ws))
                    workspaces.append(ws);
              }
          }
          DialogSelectList dlg = DialogSelectList();
          dlg.setInstructions(tr("Select the workspace geoserver is using."));
          dlg.setList(workspaces);
          int rslt = dlg.exec();
          if(rslt == QDialog::Accepted)
          {
              QString workspace = dlg.getResult();
              QList<QPair<QString,bool>> pairs = QList<QPair<QString,bool>>();
              QList<Overlay*> wsOverlays = QList<Overlay*>();
              foreach (Overlay* o, *list) {
                  if(o->_layerName.contains(workspace))
                  {
                    pairs.append(QPair<QString,bool>(o->_layerName,false));
                    wsOverlays.append(o);
                  }
              }
              dlg.setInstructions(tr("Check the overlays you wish to update or add"));
              dlg.setCheckList(pairs);
              if(dlg.exec()== QDialog::Accepted)
              {
                  pairs = dlg.getCheckList();
                  for(int ii=0; ii < pairs.count(); ii)
                  {
                      QPair<QString,bool> pair = pairs.at(ii);
                      Overlay* o = wsOverlays.at(ii++);
                      if(pair.second) // is checked?
                      {
                          bool bExists = false;
                          QList<Overlay*> oList = model->getOverlayMap()->values();
                          for (int jj = 0; jj < oList.count(); ++jj)
                          {
                              Overlay* o2 = oList.at(jj);
                              if(o->source == o2->source && o->_layerName == o2->_layerName &&
                                  o->url() == o2->url() && o->cityName == o2->cityName)
                              {
                                  // update the current item
                                  o2->setBounds(o->bounds());
                                  o2->minZoom = o->minZoom;
                                  o2->maxZoom = o->maxZoom;
                                  o2->description = o->description;
                                  o2->name = o->name;
                                  bExists = true;
                                  bDirty = true;
                                  qDebug() << o2->_layerName << " updated";
                              }
                          }
                          if(!bExists)
                          {
                              model->getOverlayMap()->insert(o->name,o);
                              qDebug() << o->_layerName << " added";
                              bDirty = true;
                          }
                      }
                  }
              }
              else return;
          }
          dlg.close();
      });
      menu.addAction(queryWmts);
  }
  // more actions can be added here
  menu.exec(QCursor::pos());
// }
}//get QTableView selected item

void EditCityDialog::onNewLine()
{
    Overlay* newOverlay = new Overlay(config->currCity->name(),"");
    model->addOverlay(newOverlay);
    QTimer::singleShot(0, this, [this]() {
        ui->tableView->scrollToBottom();
    });
}

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
    Overlay* ov  = model->getOverlayMap()->values().at(currentIndexTableView.row());
    int rslt = QMessageBox::question(nullptr, tr("OK to Delete"),tr("Confirm you want to delete %1 ").arg(ov->name),
                                     QMessageBox::Yes|QMessageBox::No);
    if(rslt == QMessageBox::Yes)
        model->deleteRow(currentIndexTableView.row());
}

void EditCityDialog::onUpdateProperties()
{
 Overlay* ov = model->selectedOverlay(currentIndexTableView.row());
 setCursor(Qt::WaitCursor);
 if(ov->source == "geoserver")
     updateGeoserverProperties(ov);
 connect(ov, &Overlay::xmlFinished, [=]{setCursor(Qt::ArrowCursor);});
 ov->getTileMapResource();
}

void EditCityDialog::updateGeoserverProperties(Overlay *ov)
{
    QString url = ov->url();
    QString host = url.mid(0, url.indexOf("/geoserver")+10);
    geoserver->getCapabilities(host);
    QList<Layer*> layers = geoserver->getLayerByTitle(ov->name);
    if(layers.isEmpty())
        return;
    ov->_layerName = layers.at(0)->name;
    ov->setBounds(layers.at(0)->bounds);
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


void EditCityDialog::onColumnChanged(int row,int column , Overlay* ovOld, Overlay* ovNew, QVariant value)
{
    ui->lblInfo->clear();
    switch((OverlayTableModel::COLUMNS)column)
    {
        case OverlayTableModel::SOURCE:
        {
            if(value.toString() == "geoserver")
            {
                if(ovOld->source == ovNew->source)
                    return;
                if(ovNew->url().isEmpty())
                {
                    ui->lblInfo->setStyleSheet(tr("color: red"));
                    ui->lblInfo->setText(tr("host url of geoserver required"));
                    ovNew->source = ovOld->source;
                    bDirty = true;
                    return;
                }
                QString url = ovNew->url();
                if( (geoserver->getHost() == url))
                    return;
                geoserver->getCapabilities(url);


                if(ovOld->source == "georeferencer")
                {
                    QString url = ovNew->url();
                    if(!url.contains("geoserver"))
                    {
                        ui->lblInfo->setStyleSheet("color: red");
                        ui->lblInfo->setText(tr("not a geoserver url"));
                        ov->source = ovOld->source;
                        return;
                    }
                    QString host = url.mid(0, url.indexOf("/geoserver")+10);
                    geoserver->getCapabilities(host);
                    int begin = url.indexOf("my_maps:");
                    int last = url.indexOf("@");
                    QString layerName = url.mid(begin, last-begin);
                    ovNew->_layerName = layerName;
                    // QStringList sl = QStringList();
                    // sl.append(host);
                    ovNew->setUrl(host);
                    bDirty = true;
                }
                if(ovNew->name.isEmpty())
                {
                    ui->lblInfo->setStyleSheet("color: #FFBF00");
                    ui->lblInfo->setText(tr("select a name"));

                }
            }
        }
        break;
        case OverlayTableModel::NAME:
            if(geoserver)
            {
                QList<Layer*> list = geoserver->getLayerByName(value.toString());
                if(list.count() == 1)
                {
                    ovNew->setBounds(list.at(0)->bounds);
                    break;
                }
            }
            break;
        case OverlayTableModel::SELECTED:
        case OverlayTableModel::CITYNAME:
        case OverlayTableModel::YEAR:
        case OverlayTableModel::DESCRIPTION:
        case OverlayTableModel::BOUNDS:
        case OverlayTableModel::MINZOOM:
        case OverlayTableModel::MAXZOOM:
        case OverlayTableModel::OPACITY:
        case OverlayTableModel::LOCAL:
        case OverlayTableModel::LAYER:
            if(ovNew->source == "geoserver")
            {
                if(!ovNew->name.isEmpty())
                {
                    geoserver->getCapabilities(ovNew->url());
                    QList<Layer*> layer = geoserver->getLayerByTitle(ovNew->name);
                    if(layer.count()>0)
                    {
                        ovNew->_layerName = layer.at(0)->name;
                        ovNew->setBounds(layer.at(0)->bounds);
                    }
                }
            }
            break;
        case OverlayTableModel::URLS:
        {
            if(ovNew->url().isEmpty())
            {
                return;
            }
            QString url = ovNew->url();
            QString newUrl =value.toString();
            if(newUrl.contains("geoserver"))
            {
                QString host = value.toString().mid(0, url.indexOf("/geoserver")+10);
                geoserver->getCapabilities(host);
                ovNew->source = "geoserver";
                ui->lblInfo->setStyleSheet("color: #FFBF00");
                ui->lblInfo->setText(tr("select a name"));
                if(!geoserver->titles().isEmpty())
                {
                    DialogSelectList* dlg = new DialogSelectList();
                    dlg->setList(geoserver->titles());
                    int rslt = dlg->exec();
                    if(rslt == QDialog::Accepted)
                    {
                        ovNew->name = dlg->getResult();
                        QList<Layer*> layers = geoserver->getLayerByTitle(ovNew->name);
                        ovNew->_layerName = layers.at(0)->name;
                        for(City* city : config->cityList)
                        {
                            if(city->bounds().contains(layers.at(0)->bounds))
                            {
                                ovNew->cityName = city->name();
                                ovNew->setBounds(layers.at(0)->bounds);
                            }
                            break;
                        }

                        ui->lblInfo->setStyleSheet("color: #00DD00");
                        ui->lblInfo->setText(tr("%1 selected").arg(ovNew->name));

                    }
                }
                else
                {
                    ui->lblInfo->setStyleSheet("color: #FF0000");
                    ui->lblInfo->setText(tr("url may be invalid"));
                }
            }
        }
        break;
        case OverlayTableModel::NUMCOLUMNS:
            break;
    }
}
void EditCityDialog::displayOverlay(QAction* act)
{
    Overlay* ov = VPtr<Overlay>::asPtr(act->data());
    qDebug() << "overlay "    << ov->name;
    MainWindow::instance()->loadOverlay(ov);
}
/*************************************************************************************************/
