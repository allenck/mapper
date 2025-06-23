#include "routeview.h"
#include "sql.h"
#include "mainwindow.h"
#include "editsegmentdialog.h"
#include "webviewbridge.h"
#include "otherrouteview.h"
#include "rtitemdelegate.h"
#include "usagedelegate.h"
#include "splitsegmentdlg.h"
#include "turndelegate.h"
#include "ttitemdelegate.h"
#include "dateeditdelegate.h"

RouteView::RouteView(QObject* parent )
{
    m_parent = parent;

    config = Configuration::instance();

    //sql.setConfig(config);
    myParent = qobject_cast<MainWindow*>(m_parent);
    ui = myParent->ui->tblRouteView;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));
    connect(myParent->ui->tab, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(tab1CustomContextMenu(QPoint)));
    if(auto cornerButton = ui->findChild<QAbstractButton*>(QString(), Qt::FindDirectChildrenOnly)) {
        //this button is not a normal button, it doesn't paint text or icon
        //so it is not easy to show text on it, the simplest way is tooltip
        cornerButton->setToolTip("Sort Description");
        //disconnect the connected slots to the tableview (the "selectAll" slot)
        disconnect(cornerButton, Q_NULLPTR, ui, Q_NULLPTR);
//        //connect "clear" slot to it, here I use QTableWidget's clear, you can connect your own
//        connect(cornerButton, &QAbstractButton::clicked, ui->tableWidget, &QTableWidget::clear);
        connect(cornerButton, &QAbstractButton::clicked, [=]{
         sortNameAct->trigger();
        });
//        connect(cornerButton, &QAbstractButton::setText, [=](QString txt){
//         cornerButton->setText("Sort");
//        });
    }
    connect(ui, &QTableView::activated, [=](QModelIndex index){
        ui->blockSignals(true);
    });
    connect(WebViewBridge::instance(), &WebViewBridge::segmentSelectedX, [=] (qint32 pt, qint32 segmentId, QList<LatLng> list){
        on_segmentSelected(pt, segmentId,list);
    });

    // connect(SQL::instance(), &SQL::segmentChanged, this, [=](const SegmentInfo si, SQL::CHANGETYPE t){
    //     // SegmentInfo has changed, refresh the SegmentData record/
    //     SegmentData* sd = SQL::instance()->getSegmentData(rd.route(), si.segmentId(),
    //                                                       rd.startDate().toString("yyyy/MM/dd"),
    //                                                       rd.endDate().toString("yyyy/MM/dd"));
    //     if(sd  && sd->segmentId()  == si.segmentId())
    //     {
    //         sourceModel->segmentChanged(si.segmentId());
    //     }
    // });
    ui->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->horizontalHeader()->restoreState(config->rv.state);
    connect(ui->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(hdr_customContextMenu(QPoint)));

    ui->setAlternatingRowColors(true);
    ui->horizontalHeader()->setSectionsMovable(true);
    connect(ui->horizontalHeader(), &QHeaderView::sectionMoved, [=](int logicalIndex, int oldVisualIndex, int newVisualIndex){
     config->rv.movedColumns.clear();
     for(int i = 0; i < ui->model()->columnCount(); i++)
     {
      config->rv.movedColumns.append(ui->horizontalHeader()->logicalIndex(i));
     }
     config->rv.state = ui->horizontalHeader()->saveState();
    });

    //m_myParent = myParent;
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    reSequenceFromStartAct = new QAction(tr("&Start route at beginning"), this);
    reSequenceFromStartAct->setStatusTip(tr("Start route at this segment"));
    reSequenceFromStartAct->setShortcut(tr("Alt_Ctrl+S"));
    connect(reSequenceFromStartAct, SIGNAL(triggered()), this, SLOT(reSequenceRouteFromStart()));

    reSequenceFromEndAct = new QAction(tr("&Start route at end"), this);
    reSequenceFromEndAct->setStatusTip(tr("Start route at this segment"));
    reSequenceFromEndAct->setShortcut(tr("Alt_Ctrl+S"));
    connect(reSequenceFromEndAct, SIGNAL(triggered()), this, SLOT(reSequenceRouteFromEnd()));

    startTerminalStartAct = new QAction(tr("Start at beginning"), this);
    startTerminalStartAct->setStatusTip(tr("Start terminal is at start of segment"));
    connect(startTerminalStartAct, SIGNAL(triggered()), this, SLOT(StartRoute_S()));

    startTerminalEndAct = new QAction(tr("Start at end"), this);
    startTerminalEndAct->setStatusTip(tr("Start terminal is at end of segment"));
    connect(startTerminalEndAct, SIGNAL(triggered()), this, SLOT(StartRoute_E()));

    endTerminalStartAct = new QAction(tr("End terminal at beginning"), this);
    endTerminalStartAct->setStatusTip(tr("End terminal is at start of segment"));
    connect(endTerminalStartAct, SIGNAL(triggered()), this, SLOT(EndRoute_S()));

    endTerminalEndAct = new QAction(tr("End terminal at end"), this);
    endTerminalEndAct->setStatusTip(tr("End terminal is at end of segment"));
    connect(endTerminalEndAct,SIGNAL(triggered()), this, SLOT(EndRoute_E()));

    removeSegmentAct = new QAction(tr("Remove from route"), this);
    removeSegmentAct->setStatusTip(tr("Delete the segment from the route"));
    connect(removeSegmentAct, SIGNAL(triggered()), this, SLOT(removeSegment()));

    deleteSelectedRowsAct = new QAction(tr("Remove selected rows"),this);
    connect(deleteSelectedRowsAct, &QAction::triggered, [=]{
     for(SegmentData* sd : selectedSegments())
     {
      SQL::instance()->deleteRoute(*sd);
      MainWindow::instance()->m_bridge->processScript("clearPolyline", QString("%1").arg(sd->segmentId()));
     }
     model()->_selectedSegments.clear();
    });

    selectSegmentAct = new QAction(tr("Select segment"),this);
    selectSegmentAct->setToolTip(tr("Select segment on map."));
    connect(selectSegmentAct, SIGNAL(triggered()), this, SLOT(on_selectSegment_triggered()));

    editSegmentAct = new QAction(tr("Edit segment"), this);
    editSegmentAct->setStatusTip(tr("Edit this segment's properties"));
    connect(editSegmentAct, SIGNAL(triggered(bool)), this, SLOT(editSegment()));

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
    sourceModel = new RouteViewTableModel(route, name, companyKey, startDate,
                                          endDate, QList<SegmentData*>(),this);

    sortNameAct = new QAction(tr("Sort Description"),this);
    sortNameAct->setStatusTip(tr("Sort table by description"));
    connect(sortNameAct, &QAction::triggered, [=]{
     Qt::SortOrder order = ui->horizontalHeader()->sortIndicatorOrder();
     switch (order) {
     case Qt::AscendingOrder:
      order = Qt::DescendingOrder;
      break;
     case Qt::DescendingOrder:
      order = Qt::AscendingOrder;
      break;
     default:
      order = Qt::AscendingOrder;
     }
     ui->sortByColumn(model()->NAME, order);
     ui->hideColumn(model()->NAME);
    });
    connect(ui, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tablev_customContextMenu(QPoint)));

    hideColumnAct = new QAction(tr("Hide Column"),this);
    connect(hideColumnAct, &QAction::triggered, [=]{
     int logicalIndex =hideColumnAct->data().toInt();
     ui->hideColumn(logicalIndex);
     if(!config->rv.hiddenColumns.contains(logicalIndex))
      config->rv.hiddenColumns.append(logicalIndex);
    });


    updateRouteAct = new QAction(tr("Update route"),this);
    connect(updateRouteAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     qint32 segmentId = indexes.at(RouteViewTableModel::SEGMENTID).data().toInt();
     QModelIndex ix = proxymodel->mapToSource(indexes.at(RouteViewTableModel::SEGMENTID));
     SegmentData* sd = sourceModel->listOfSegments.at(ix.row());

     myParent->selectSegment(segmentId);
     myParent->updateRoute(sd);
    });

    addToAnotherRouteAct = new QAction(tr("Add selected segments to Another Route"));
    connect(addToAnotherRouteAct, &QAction::triggered, myParent,[=]{
     QComboBox* cbCombo = new QComboBox();
     QList<RouteData> routeList = myParent->routeList;
     for(int i=0; i<routeList.count(); i++)
     {
       RouteData rd = (RouteData)routeList.at(i);
       QString rdStartDate = rd.startDate().toString("yyyy/MM/dd");
       cbCombo->addItem(rd.toString(),QVariant::fromValue(rd));
//          if( rd.toString() == currText)
//             cbCombo->setCurrentIndex(i);
     }
     QMessageBox* box = new QMessageBox(QMessageBox::Question, tr("Select Route"),
                                          tr("Select the route to add the selected segments to."),
                                        QMessageBox::Ok| QMessageBox::Cancel);
//     box->setInformativeText("Routes");
//     QList<QWidget*> list = box->findChildren<QWidget*>("qt_msgbox_informativelabel");

     QLayout* layout = box->layout();
     if(qobject_cast<QGridLayout*>(layout))
     {
       //layout->addWidget(cbCombo);
      ((QGridLayout*)layout)->addWidget(cbCombo,1,2);

       if(box->exec() == QMessageBox::Ok)
       {
        RouteData rd = cbCombo->currentData().value<RouteData>();
        if(cbCombo->currentIndex() < 0)
         return;
        for(SegmentData* sd : sourceModel->_selectedSegments)
        {
         sd->setRouteName(rd.routeName());
         sd->setStartDate(rd.startDate());
         sd->setEndDate(rd.endDate());
         sd->setCompanyKey(rd.companyKey());
         if(SQL::instance()->doesRouteSegmentExist(*sd))
             continue;
         SQL::instance()->addSegmentToRoute(sd);
        }
        sourceModel->_selectedSegments.clear();
       }
      }
    });

    convertToSingleTrackAct = new QAction(tr("Convert to single track"),this);
    connect(convertToSingleTrackAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     qint32 segmentId = indexes.at(RouteViewTableModel::SEGMENTID).data().toInt();
     QModelIndex ix = proxymodel->mapToSource(indexes.at(RouteViewTableModel::SEGMENTID));
     SegmentData* sd = sourceModel->listOfSegments.at(ix.row());
     SegmentInfo si = SQL::instance()->convertSegment(segmentId, 1);
     if(si.segmentId() > 0 && si.segmentId() != sd->segmentId()){
      bool ok = SQL::instance()->deleteRouteSegment(sd->route(), sd->routeId(),sd->segmentId(),
                                                    sd->startDate().toString("yyyy/MM/dd"),
                                                    sd->endDate().toString("yyyy/MM/dd"));
      qDebug() << "old segment deleted " << sd->segmentId() << ok;
      sd->setOneWay("N");
      ok = SQL::instance()->addSegmentToRoute(sd);
      qDebug() << "new segment added " << si.segmentId() << ok;
     }
    });
    splitSegmentAct = new QAction(tr("Split segment at a date"),this);
    splitSegmentAct->setStatusTip(tr("Split segment into two separate route segments on a date"));
    connect(splitSegmentAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     QModelIndex ix = proxymodel->mapToSource(indexes.at(RouteViewTableModel::SEGMENTID));
     SegmentData* sd = sourceModel->listOfSegments.at(ix.row());
     qint32 segmentId = indexes.at(RouteViewTableModel::SEGMENTID).data().toInt();
     SplitSegmentDlg* splitSegment = new SplitSegmentDlg(sd);
     int ret = splitSegment->exec();
     if(ret == QDialog::Accepted)
     {
      //sourceModel->listOfSegments.removeAt(ix.row());
     }
      myParent-> ui->ssw->refresh();
    });

    proxymodel = new RouteViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->setModel(proxymodel);
    connect(this, SIGNAL(sendRows(int, int)), sourceModel, SLOT(getRows(int,int)));

    //connect(ui, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemChanged(QTableWidgetItem*)));
    //connect(ui, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(handleSelectionChanged(QTableWidgetItem *)));
    connect(ui, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelectionChanged(QModelIndex)));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), ui,
            SLOT(dataChanged(QModelIndex,QModelIndex)));

    ui->setColumnWidth(0,8);
    ui->setColumnWidth(RouteViewTableModel::TYPE, 20);
    ui->setColumnWidth(RouteViewTableModel::DISTANCE, 29);
    ui->setColumnWidth(RouteViewTableModel::TRACKS, 5);
    ui->hideColumn(sourceModel->NAME);
    //ui->horizontalHeader()->restoreState(config->rv.state);
    config->rv.hiddenColumns.clear();
    for(int i=0; i < sourceModel->columnCount(QModelIndex()); i++)
    {
     if(ui->horizontalHeader()->isSectionHidden(i))
     {
      if(!config->rv.hiddenColumns.contains(QVariant(i)))
       config->rv.hiddenColumns.append(QVariant(i));
     }
     else
     {
      if(config->rv.hiddenColumns.contains(QVariant(i)))
       config->rv.hiddenColumns.removeOne(QVariant(i));
     }
    }

    //connect(WebViewBridge::instance(), SIGNAL(segmentSelected(qint32,qint32)), this, SLOT(on_segmentSelected(int,int)));
    connect(WebViewBridge::instance(), SIGNAL(segmentSelectedX(qint32,qint32,QList<LatLng>)), this, SLOT(on_segmentSelected(int,int,QList<LatLng>)));

    myParent->ui->tabWidget->setTabText(0, "Route Segments");
    startTerminal = NULL;
    startSegment = -1;
    endSegment = -1;
}

void RouteView::setList(QList<SegmentData*> segmentDataList)
{
 sourceModel->setList(segmentDataList);
}


RouteViewTableModel* RouteView::model() { return sourceModel;}

void RouteView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)
    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
}

void RouteView::hdr_customContextMenu( const QPoint pt)
{
    // curRow = ui->rowAt(pt.y());
    //curCol = ui->columnAt(pt.x());
    menu.clear();
    hideColumnAct->setData(ui->horizontalHeader()->logicalIndexAt(pt));
    menu.addAction(hideColumnAct);
    if(config->rv.hiddenColumns.count()>0)
    {
     QMenu* m = new QMenu(tr("Show column"));
     menu.addMenu(m);
     foreach(QVariant col, config->rv.hiddenColumns)
     {
      QAction* a = new QAction(sourceModel->headerData(col.toInt(),Qt::Horizontal, Qt::DisplayRole).toString(),this);
      m->addAction(a);
      connect(a, &QAction::triggered, [=]{
       ui->showColumn(col.toInt());
       config->rv.hiddenColumns.removeOne(col.toInt());
      });
     }
    }
    menu.exec(QCursor::pos());
}

//create table input context menu
void RouteView::tablev_customContextMenu( const QPoint& pt)
{
    curRow = ui->rowAt(pt.y());
    curCol = ui->columnAt(pt.x());
    // check is item in QTableView exist or not
    if(boolGetItemTableView(ui))
    {
        //menu = QMenu(m_parent*);
     menu.clear();
//     menu.addAction(copyAction);
//     menu.addAction(pasteAction);
//     menu.addSeparator();

     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     //qint32 row = model->currentIndex().row();
     //qint32 col =model->currentIndex().column();
     if(indexes.count()==0)
      return;
     QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
     QString txtSegmentId = Index.data().toString();
     txtSegmentId.replace("!", "");
     txtSegmentId.replace("*", "");
     qint32 segmentId = txtSegmentId.toInt();
     SegmentData* sd = sourceModel->segmentData(proxymodel->mapToSource(Index).row());
     //if(sourceModel->isSegmentMarkedForDelete(segmentId))
//     if(sd->markedForDelete())
//         menu.addAction(unDeleteSegmentAct);
//     else
     menu.addAction(removeSegmentAct);
     if(selectedSegments().count())
      menu.addAction(deleteSelectedRowsAct);
     //if(curRow == 0)
     //menu.addAction(saveChangesAct);
     //menu.addAction(discardChangesAct);
     menu.addAction(selectSegmentAct);
     QMenu* resequenceMenu = new QMenu(tr("Resequence route"));
     resequenceMenu->addAction(reSequenceFromStartAct);
     resequenceMenu->addAction(reSequenceFromEndAct);
     bool enable = (sd->tracks() == 1 && sd->oneWay() != "Y")
       || (sd->tracks() == 2 && sd->oneWay() == "Y" && sd->trackUsage() != "L");
     reSequenceFromEndAct->setEnabled(enable);
     menu.addMenu(resequenceMenu);
//        if(!startTerminal)
//        {
     TerminalInfo ti = SQL::instance()->getTerminalInfo(route,name, endDate);
     startTerminal = menu.addMenu(tr("Set start terminal..."));
     QActionGroup* startGroup = new QActionGroup(this);
     startTerminal->addAction(startTerminalStartAct);
     startTerminalStartAct->setCheckable(true);
     startTerminal->addAction(startTerminalEndAct);
     startGroup->addAction(startTerminalStartAct);
     startGroup->addAction(startTerminalEndAct);
     startTerminalEndAct->setCheckable(true);
     if(ti.startSegment == segmentId && ti.startWhichEnd=="S")
      startTerminalStartAct->setChecked(true);
     else
      startTerminalEndAct->setChecked(true);

     endTerminal =menu.addMenu(tr("Set end terminal..."));
     QActionGroup* endGroup = new QActionGroup(this);
     endTerminal->addAction(endTerminalStartAct);
     endTerminalStartAct->setCheckable(true);
     endTerminal->addAction(endTerminalEndAct);
     endTerminalEndAct->setCheckable(true);
     endGroup->addAction(endTerminalStartAct);
     endGroup->addAction(endTerminalEndAct);
     if(ti.endSegment == segmentId && ti.endWhichEnd=="S")
      endTerminalStartAct->setChecked(true);
     else
      endTerminalEndAct->setChecked(true);

//        }     menu.addAction(saveChangesAct);

     menu.addAction(editSegmentAct);
     //menu.addAction(showColumnsAct);
     menu.addAction(updateRouteAct);
     menu.addAction(splitSegmentAct);
     if(selectedSegments().count())
      menu.addAction(addToAnotherRouteAct);
//     addToAnotherRouteAct->setEnabled(sourceModel->_selectedSegments.count());
//     {
//      QItemSelectionModel * model = ui->selectionModel();
//      QModelIndexList indexes = model->selectedIndexes();
//      qint32 segmentId = indexes.at(RouteViewTableModel::SEGMENTID).data().toInt();
//      SegmentInfo sd = SQL::instance()->getSegmentInfo(segmentId);
//      if(sd.tracks()==2)
//       menu.addAction(convertToSingleTrackAct);
//     }
     //if(sourceModel->changedMap.values().count() > 0)
     if(sourceModel->bChangesMade)
     {
         menu.addSeparator();
//         menu.addAction(saveChangesAct);
//         menu.addAction(discardChangesAct);
         menu.addAction(sortNameAct);
     }
     menu.addSeparator();
     menu.exec(QCursor::pos());
 }
}

void RouteView::tab1CustomContextMenu(const QPoint &)
{
 QMenu tab1Menu;
    tab1Menu.addAction(sortNameAct);
    tab1Menu.exec(QCursor::pos());
}

//get QTableView selected item
bool RouteView::boolGetItemTableView(QTableView *table)
{
    // get model from tableview
    QItemSelectionModel * model = table->selectionModel();
    if(model)
    {
        currentIndex = model->currentIndex();
        return (true);
    }
    else                //QTableView doesn't have selected data
        return (false);

}

void RouteView::updateRouteView()
{
    //SQL sql;
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    TerminalInfo ti = SQL::instance()->getTerminalInfo(myParent->m_routeNbr, myParent->m_routeName,
                                                       QDate::fromString(myParent->m_currRouteEndDate, "yyyy/MM/dd"));
    startSegment = ti.startSegment;
    endSegment = ti.endSegment;
    // Check for uncomitted changes
//    checkChanges();
    route = myParent->m_routeNbr;
    name = myParent->m_routeName;
    companyKey = myParent->_rd.companyKey();
    startDate = QDate::fromString(myParent->m_currRouteStartDate, "yyyy/MM/dd");
    endDate = QDate::fromString(myParent->m_currRouteEndDate, "yyyy/MM/dd");
    alphaRoute = myParent->m_alphaRoute;

    rd = myParent->ui->cbRoute->currentData().value<RouteData>();
    myParent->ui->routeViewLabel->setText(tr("Route segments for route %1").arg(rd.toString()));


    ui->setSortingEnabled(false);
    if(!sourceModel)
     sourceModel = new RouteViewTableModel(route, name, companyKey, startDate, endDate,
                                           SQL::instance()->getRouteSegmentsInOrder(route, name, companyKey, endDate),this);
    else
     sourceModel->setList(SQL::instance()->getRouteSegmentsInOrder(route, name, companyKey, endDate));
    sourceModel->setSequenced(bIsSequenced);

    proxymodel->setSourceModel(sourceModel);

    if(!(ui->contextMenuPolicy() == Qt::CustomContextMenu))
        ui->setContextMenuPolicy(Qt::CustomContextMenu);

    // get the row of the start segment and the end segment
    int numRows = sourceModel->rowCount(QModelIndex());
    qint32 startRow =-1, endRow = -1;
    ti = SQL::instance()->getTerminalInfo(route,name, endDate);
    for(int i = 0; i < numRows; i++)
    {
     if(ui->isRowHidden(i))
      continue;
     int segmentId = sourceModel->index(i, RouteViewTableModel::SEGMENTID).data(Qt::DisplayRole).toInt();
     if(segmentId == ti.startSegment)
      startRow = i;
     if(segmentId == ti.endSegment)
      endRow = i;
    }
    if(startRow != -1 && endRow != -1)
     emit sendRows (startRow, endRow);
    //proxyModel->setTerminals(startRow, endRow);
    ui->setModel(proxymodel);
    ui->setSortingEnabled(true);

    ui->horizontalHeader()->setStretchLastSection(false);
    ui->horizontalHeader()->resizeSection(0,40);
    ui->horizontalHeader()->resizeSection(2,35);
    ui->horizontalHeader()->resizeSection(3,35);
    ui->horizontalHeader()->resizeSection(4,35);
    ui->horizontalHeader()->resizeSection(5,35);
    ui->horizontalHeader()->resizeSection(6,35);
    ui->horizontalHeader()->resizeSection(7,35);
    ui->horizontalHeader()->resizeSection(8,65);
    ui->horizontalHeader()->resizeSection(9,65);
    ui->resizeColumnsToContents();
    ui->setItemDelegateForColumn(sourceModel->TRACTIONTYPE, new TTItemDelegate());
    ui->setItemDelegateForColumn(sourceModel->TYPE, new RTItemDelegate());
    ui->setItemDelegateForColumn(sourceModel->NE, new TurnDelegate("back"));
    ui->setItemDelegateForColumn(sourceModel->NL, new TurnDelegate("ahead"));
    ui->setItemDelegateForColumn(sourceModel->RE, new TurnDelegate("back"));
    ui->setItemDelegateForColumn(sourceModel->RL, new TurnDelegate("ahead"));
    ui->setItemDelegateForColumn(sourceModel->COMBO, new UsageDelegate());
    ui->setItemDelegateForColumn(sourceModel->STARTDATE, new DateEditDelegate());
    ui->setItemDelegateForColumn(sourceModel->ENDDATE, new DateEditDelegate());
    ui->setItemDelegateForColumn(sourceModel->DOUBLEDATE, new DateEditDelegate());

    //populateList();
}

void RouteView::populateList()
{
 ui->setSortingEnabled(false);

    //routeViewTableModel *sourceModel = new routeViewTableModel(segmentInfoList);
    //myParent->proxyModel= new routeViewSortProxyModel(this);
    //proxyModel->setSourceModel(sourceModel);
   // ui->setModel(proxyModel);


 ui->setSortingEnabled(true);
}
bool ascending_si_segmentId( const SegmentData s1 , const SegmentData s2 )
{
 //cout<<"\n" << __FUNCTION__;
 return s1.segmentId() < s2.segmentId();
}

void RouteView::reSequenceRouteFromStart()
{
 reSequenceRoute("S");
}

void RouteView::reSequenceRouteFromEnd()
{
 reSequenceRoute("E");
}

void RouteView::reSequenceRoute(QString whichEnd)
{
 //SQL sql;
 MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 //qint32 row = model->currentIndex().row();
 //qint32 col =model->currentIndex().column();
 QModelIndex proxyIndex = indexes.at(RouteViewTableModel::SEGMENTID);
 QModelIndex srcIndex = proxymodel->mapToSource(proxyIndex);
 qint32 segmentId;
 //if(col==0)
 {
  //bool bOk=false;
  segmentId = this->model()->getList().at(srcIndex.row())->segmentId();
  qint32 endSegment = -1;
  endSegment = SQL::instance()->sequenceRouteSegments(segmentId, sourceModel->getList(), &rd, whichEnd);
  sourceModel->bChangesMade = true;
#if 0
  QList<SegmentData> old = segmentDataList;
  //compareSequenceClass comparer = new compareSequenceClass();
  //segmentList.Sort(comparer);
  std::sort(segmentDataList.begin(),segmentDataList.end(),ascending_si_segmentId);
  bIsSequenced = true;
  for(int i=0; i < segmentDataList.count(); i++)
  {
   SegmentData sd = segmentDataList.at(i);
   if(sd.needsUpdate())
   {
    sourceModel->changedMap.values().append(new RowChanged( i, sd));
   }
   if(sd.sequence() == -1 && segmentDataList.at(i).returnSeq() ==-1)
   {
       bIsSequenced = false;
       break;
   }
  }
#endif
#if 0
  //populateList();
  sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentDataList);
  myParent->proxyModel->setSourceModel(sourceModel);
  //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
  connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));

  ui->setModel(myParent->proxyModel);
  //sourceModel->reset();
  sourceModel->setSequenced(bIsSequenced);
#endif
  ti = SQL::instance()->getTerminalInfo(route,name, endDate);
  qint32 startRow=-1, endRow=-1;
  for(int i = 0; i < sourceModel->rowCount(QModelIndex()); i++)
  {
   if(ui->isRowHidden(i))
       continue;
   int segmentId = sourceModel->index(i, 0).data(Qt::DisplayRole).toInt();
   if(segmentId == ti.startSegment)
       startRow = i;
   if(segmentId == ti.endSegment)
       endRow = i;
  }
  emit sendRows (startRow, endRow);
 }
 SQL::instance()->saveRouteSequence(rd, segmentId, whichEnd);
 sourceModel->reset();
 myParent->setCursor(Qt::ArrowCursor);
}

void RouteView::itemSelectionChanged(QModelIndex index )
{
    if(!index.isValid())
        return;
  QString value = index.model()->index(index.row(),0).data().toString();
  if(value.contains(" "))
  {
    qint32 i = value.indexOf(' ');
    value.truncate(i);
  }

  qint32 segmentId = value.toInt();

  MainWindow * parent = qobject_cast<MainWindow*>(this->m_parent);
  if(parent->m_segmentId == segmentId)
      return;
  parent->setCursor(QCursor(Qt::WaitCursor));
//  if(parent->selectedSegment() == segmentId)
//   return; // already selected
  parent->ProcessScript("selectSegment", QString("%1").arg(segmentId));
  parent->setCursor(QCursor(Qt::ArrowCursor));

  OtherRouteView::instance()->showRoutesUsingSegment(segmentId);
}

void RouteView::StartRoute_S()         //SLOT
{
    //SQL sql;
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    //qint32 row = model->currentIndex().row();
    //qint32 col =model->currentIndex().column();
    QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    SQL::instance()->updateTerminals(route, name, startDate,
                                     endDate, segmentId, "S",
                                      ti.route < 0 ? segmentId : ti.endSegment,
                                      ti.route < 0 ? "?" : ti.endWhichEnd);
    ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    startSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.startLatLng.lat, ti.startLatLng.lon, myParent->getRouteMarkerImagePath(myParent->m_alphaRoute, true) };
//    webBrowser1.Document.InvokeScript("addRouteStartMarker", objArray);
    QVariantList objArray;
    objArray << ti.startLatLng.lat()<< ti.startLatLng.lon()<< //myParent->getRouteMarkerImagePath(alphaRoute, false);
        "./green00.png" << "'" + alphaRoute + "'";

    WebViewBridge::instance()->processScript("addRouteStartMarker", objArray);

}
void RouteView::EndRoute_S()           // SLOT
{
    //SQL sql;
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    //qint32 row = model->currentIndex().row();
    //qint32 col =model->currentIndex().column();
    QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    //SQL::instance()->updateTerminals(route, name,ti.route < 0 ? "1800/01/01" : ti.startDate), endDate, ti.route < 0?-1:ti.startSegment, ti.route < 0?"?":ti.startWhichEnd,        segmentId, "S");
    SQL::instance()->updateTerminals(route, name,ti.route < 0 ? QDate::fromString("1800/01/01", "yyyy/MM/dd") : startDate,
                                     endDate, ti.route < 0?-1:ti.startSegment, ti.route < 0?"?":ti.startWhichEnd,        segmentId, "S");
    ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    endSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.endLatLng.lat, ti.endLatLng.lon, myParent->getRouteMarkerImagePath(myParent->m_alphaRoute, false) };
//    webBrowser1.Document.InvokeScript("addRouteEndMarker", objArray);
    QVariantList objArray;
    objArray << ti.endLatLng.lat()<< ti.endLatLng.lon()<< // <myParent->getRouteMarkerImagePath(alphaRoute, false);
        "./red00.png" << "'" + alphaRoute + "'";

    WebViewBridge::instance()->processScript("addRouteEndMarker", objArray);

}
void RouteView::StartRoute_E()         // SLOT
{
    //SQL sql;
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    //qint32 row = model->currentIndex().row();
    //qint32 col =model->currentIndex().column();
    QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    //SQL::instance()->updateTerminals(route, name, ti.route < 0?"1800/01/01":ti.startDate), endDate, segmentId, "E", ti.route < 0 ? -1 : ti.endSegment, ti.route < 0 ? "?" : ti.endWhichEnd);
    SQL::instance()->updateTerminals(route, name, ti.route < 0?QDate::fromString("1800/01/01","yyyy/MM/dd"):startDate,
                                     endDate, segmentId, "E", ti.route < 0 ? -1 : ti.endSegment, ti.route < 0 ? "?" : ti.endWhichEnd);
    ti = SQL::instance()->getTerminalInfo(route,myParent-> m_routeName, endDate);
    startSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.startLatLng.lat, ti.startLatLng.lon, getRouteMarkerImagePath(m_alphaRoute, true) };
//    webBrowser1.Document.InvokeScript("addRouteStartMarker", objArray);
    QVariantList objArray;
    objArray << ti.startLatLng.lat()<< ti.startLatLng.lon()<< //myParent->getRouteMarkerImagePath(alphaRoute, false);
        "./green00.png" << "'" + alphaRoute + "'";

    WebViewBridge::instance()->processScript("addRouteStartMarker", objArray);

}
void RouteView::EndRoute_E()       // SLOT
{
    //SQL sql;
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    //qint32 row = model->currentIndex().row();
    //qint32 col =model->currentIndex().column();
    QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    SQL::instance()->updateTerminals(route, name, endDate, endDate,
        ti.route < 0 ? segmentId : ti.startSegment, ti.route < 0 ? "?" : ti.startWhichEnd,
        segmentId, "E");
    ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    endSegment = segmentId;
    updateRouteView();

//    Object[] objArray = new Object[] { ti.endLatLng.lat, ti.endLatLng.lon, getRouteMarkerImagePath(m_alphaRoute, false) };
//    webBrowser1.Document.InvokeScript("addRouteEndMarker", objArray);
    QVariantList objArray;
    objArray << ti.endLatLng.lat()<< ti.endLatLng.lon()<< //myParent->getRouteMarkerImagePath(alphaRoute, false);
        "./red00.png" << "'" + alphaRoute + "'";

    WebViewBridge::instance()->processScript("addRouteEndMarker", objArray);

}
void RouteView::updateTerminals()
{
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    //QItemSelectionModel * model = ui->selectionModel();
    bIsSequenced = true;
    int startSeg = -1, endSeg = -1, startRow=-1, endRow=-1;
    QString startWhichEnd = "S", endWhichEnd = "S";
    myParent->setCursor(QCursor(Qt::WaitCursor));
    for(int i =0; i < sourceModel->getList().count(); i++)
    {
        SegmentData* sd = sourceModel->getList().at(i);
        if(sd->sequence() == 0)
        {
            startSeg = sd->segmentId();
            startRow = i;
            SegmentInfo sd1 = SQL::instance()->getSegmentInfo(sd->next());
            if(SQL::instance()->Distance(sd->startLat(), sd->startLon(), sd1.startLat(), sd1.startLon()) < .02
               ||SQL::instance()->Distance(sd->startLat(), sd->startLon(), sd1.endLat(), sd1.endLon()) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(sd->returnSeq() == 0)
        {
            endSeg = sd->segmentId();
            endRow = i;
            SegmentInfo sd1 = SQL::instance()->getSegmentInfo(sd->prev());
            if(SQL::instance()->Distance(sd->startLat(), sd->startLon(), sd1.startLat(), sd1.startLon()) < .02
               || SQL::instance()->Distance(sd->startLat(), sd->startLon(), sd1.endLat(), sd1.endLon()) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(sd->sequence() == -1 && sd->returnSeq()==-1)
        {
            bIsSequenced = false;
            myParent->setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
    }
    if(!bIsSequenced)
        return;
    for(int i =0; i < sourceModel->getList().count(); i++)
    {
        SegmentData* sd = sourceModel->getList().at(i);
        //SQL::instance()->updateRoute(route, name, endDate, sd.segmentId(), sd.next(), sd.prev(), sd.trackUsage());
        SegmentData* newSd = new SegmentData(*sd);
        newSd->setRoute(route);
        newSd->setRouteName(name);
        SQL::instance()->updateRoute(*sd, *newSd);
    }

    SQL::instance()->updateTerminals(route, name, startDate,
                                     endDate, startSeg, startWhichEnd, endSeg, endWhichEnd);
    emit sendRows (startRow, endRow);

    updateRouteView();
    myParent->setCursor(QCursor(Qt::ArrowCursor));
}

bool RouteView::isSequenced()
{
    return bIsSequenced;
}
bool RouteView::dataChanged(QModelIndex oldIx,QModelIndex newIx )
{
    //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    //QItemSelectionModel * model = ui->selectionModel();
    return true;
}

void RouteView::removeSegment()
{
    //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    QModelIndex ix = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = ix.data().toInt();
    sourceModel->deleteRow(segmentId, proxymodel->mapToSource(ix));
    sourceModel->bChangesMade = true;
    ui->clearSelection();
    MainWindow::instance()->segmentChanged(segmentId, 0);
}

void RouteView::on_selectSegment_triggered()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 QModelIndex Index = indexes.at(RouteViewTableModel::SEGMENTID);
 qint32 segmentId = Index.data().toInt();
 MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
 //myParent->segmentSelected(0,segmentId);
 emit selectSegment(segmentId);
}

void RouteView::editSegment()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 qint32 segmentId = indexes.at(RouteViewTableModel::SEGMENTID).data().toInt();
 SegmentData* sd = sourceModel->getList().at(proxymodel->mapToSource(indexes.at(RouteViewTableModel::SEGMENTID)).row());
 SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId);
 EditSegmentDialog* dlg = new EditSegmentDialog(si);
 if(dlg->exec() == QDialog::Accepted)
  MainWindow::instance()->refreshRoutes();
}

void RouteView::on_segmentSelected(int, int segmentId, QList<LatLng>)
{
 int row = sourceModel->getRow(segmentId);
 if(row >= 0)
 {
  QModelIndex modelIndex = proxymodel->mapFromSource(sourceModel->index(row,1));
  ui->selectRow(modelIndex.row());
 }
}

void RouteView::clear()
{
    sourceModel->clear();
}

QList<SegmentData*> RouteView::selectedSegments()
{
 return sourceModel->_selectedSegments;
}
