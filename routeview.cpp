#include "routeview.h"
#include "sql.h"
#include "mainwindow.h"
#include "editsegmentdialog.h"
#include "webviewbridge.h"
#include "otherrouteview.h"
#include "ttitemdelegate.h"
#include "rtitemdelegate.h"
#include "splitsegmentdlg.h"
#include "checkroute.h"
#include "turndelegate.h"

RouteView::RouteView(QObject* parent )
{
    m_parent = parent;

    config = Configuration::instance();

    //sql.setConfig(config);
    myParent = qobject_cast<MainWindow*>(m_parent);
    ui = myParent->ui->tblRouteView;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    //ui->setColumnCount(headers.count());
    //ui->setHorizontalHeaderLabels(headers);

    //ui->resizeColumnsToContents();

    ui->setAlternatingRowColors(true);

    //m_myParent = myParent;
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );

    //create contextmenu
    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setStatusTip(tr("Copy Table Location"));
    copyAction->setShortcut(tr("Ctrl+C"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(aCopy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setStatusTip(tr("Paste"));
    pasteAction->setShortcut(tr("Ctrl+V"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(aPaste()));

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

    deleteSegmentAct = new QAction(tr("Remove from route"), this);
    deleteSegmentAct->setStatusTip(tr("Delete the segment from the route"));
    connect(deleteSegmentAct, SIGNAL(triggered()), this, SLOT(deleteSegment()));

    unDeleteSegmentAct = new QAction(tr("Undo delete of segment"), this);
    unDeleteSegmentAct->setStatusTip(tr("Don't delete the segment from the route"));
    connect(unDeleteSegmentAct, SIGNAL(triggered()), this, SLOT(unDeleteSegment()));

    selectSegmentAct = new QAction(tr("Select segment"),this);
    selectSegmentAct->setToolTip(tr("Select segment on map."));
    connect(selectSegmentAct, SIGNAL(triggered()), this, SLOT(on_selectSegment_triggered()));

    editSegmentAct = new QAction(tr("Edit segment"), this);
    editSegmentAct->setStatusTip(tr("Edit this segment's properties"));
    connect(editSegmentAct, SIGNAL(triggered(bool)), this, SLOT(editSegment()));

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
    sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentDataList);

    saveChangesAct = new QAction(tr("Commit changes"),this);
    saveChangesAct->setStatusTip(tr("Save any uncommitted changes"));
    discardChangesAct = new QAction(tr("Abandon changes"),this);
    discardChangesAct->setStatusTip(tr("Discard any changes"));

    showColumnsAct = new QAction(tr("Hide extra columns"),this);
    showColumnsAct->setCheckable(true);
    connect(showColumnsAct, &QAction::toggled, [=]{
     bool b = showColumnsAct->isChecked();
     ui->setColumnHidden(RouteViewTableModel::NEXT, b);
     ui->setColumnHidden(RouteViewTableModel::PREV, b);
     ui->setColumnHidden(RouteViewTableModel::SEQ, b);
     ui->setColumnHidden(RouteViewTableModel::RSEQ, b);
    });

    updateRouteAct = new QAction(tr("Update route"),this);
    connect(updateRouteAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     qint32 segmentId = indexes.at(0).data().toInt();
     QModelIndex ix = proxymodel->mapToSource(indexes.at(0));
     SegmentData sd = sourceModel->listOfSegments.at(ix.row());

     myParent->selectSegment(segmentId);
     myParent->updateRoute(&sd);

    });

    convertToSingleTrackAct = new QAction(tr("Convert to single track"),this);
    connect(convertToSingleTrackAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     qint32 segmentId = indexes.at(0).data().toInt();
     QModelIndex ix = proxymodel->mapToSource(indexes.at(0));
     SegmentData sd = sourceModel->listOfSegments.at(ix.row());
     SegmentInfo si = SQL::instance()->convertSegment(segmentId, 1);
     if(si.segmentId() > 0 && si.segmentId() != sd.segmentId()){
      bool ok = SQL::instance()->deleteRouteSegment(sd.route(), sd.routeName(),sd.segmentId(),
                                                    sd.startDate().toString("yyyy/MM/dd"),
                                                    sd.endDate().toString("yyyy/MM/dd"));
      qDebug() << "old segment deleted " << sd.segmentId() << ok;
      ok = SQL::instance()->addSegmentToRoute(sd.route(),sd.routeName(), sd.startDate(),sd.endDate(),
                                              si.segmentId(),sd.companyKey(),
                                         sd.tractionType(),sd.direction(), -1, -1, 0,0,0,0,"N", " ");
      qDebug() << "new segment added " << si.segmentId() << ok;
     }
    });
    //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
    connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));
    connect(discardChangesAct, &QAction::triggered, [=]{
     segmentDataList = saveSegmentDataList;
     sourceModel->reset();
    });
    splitSegmentAct = new QAction(tr("Split segment at a date"),this);
    splitSegmentAct->setStatusTip(tr("Split segment into two separate route segments on a date"));
    connect(splitSegmentAct, &QAction::triggered, [=]{
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     qint32 segmentId = indexes.at(0).data().toInt();
     SplitSegmentDlg* splitSegment = new SplitSegmentDlg(segmentId);
     int ret = splitSegment->exec();
     if(ret == QDialog::Accepted)
      //refreshSegmentCB();
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

    connect(WebViewBridge::instance(), SIGNAL(segmentSelected(qint32,qint32)), this, SLOT(on_segmentSelected(int,int)));

    myParent->ui->tabWidget->setTabText(0, "Route Segments");
    startTerminal = NULL;
    startSegment = -1;
    endSegment = -1;
}

RouteViewTableModel* RouteView::model() { return sourceModel;}

void RouteView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)
    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
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
     menu.addAction(copyAction);
     menu.addAction(pasteAction);
     QItemSelectionModel * model = ui->selectionModel();
     QModelIndexList indexes = model->selectedIndexes();
     //qint32 row = model->currentIndex().row();
     //qint32 col =model->currentIndex().column();
     QModelIndex Index = indexes.at(0);
     QString txtSegmentId = Index.data().toString();
     txtSegmentId.replace("!", "");
     txtSegmentId.replace("*", "");
     qint32 segmentId = txtSegmentId.toInt();
     SegmentData sd = sourceModel->segmentData(proxymodel->mapToSource(Index).row());
     if(sourceModel->isSegmentMarkedForDelete(segmentId))
         menu.addAction(unDeleteSegmentAct);
     else
         menu.addAction(deleteSegmentAct);
     //if(curRow == 0)
     menu.addAction(saveChangesAct);
     menu.addAction(discardChangesAct);
     menu.addAction(selectSegmentAct);
     QMenu* resequenceMenu = new QMenu(tr("Resequence route"));
     resequenceMenu->addAction(reSequenceFromStartAct);
     resequenceMenu->addAction(reSequenceFromEndAct);
     bool enable = (sd.tracks() == 1 && sd.oneWay() != "Y")
       || (sd.tracks() == 2 && sd.oneWay() == "Y" && sd.trackUsage() != "L");
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
     menu.addAction(showColumnsAct);
     menu.addAction(updateRouteAct);
     menu.addAction(splitSegmentAct);

     {
      QItemSelectionModel * model = ui->selectionModel();
      QModelIndexList indexes = model->selectedIndexes();
      qint32 segmentId = indexes.at(0).data().toInt();
      SegmentInfo sd = SQL::instance()->getSegmentInfo(segmentId);
      if(sd.tracks()==2)
       menu.addAction(convertToSingleTrackAct);
     }
     if(sourceModel->changedMap.values().count() > 0)
     {
         menu.addSeparator();
         menu.addAction(saveChangesAct);
         menu.addAction(discardChangesAct);
     }
     menu.exec(QCursor::pos());

 }
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

void RouteView::aCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    if(currentIndex.isValid())
        clipboard->setText(currentIndex.data().toString());

}
void RouteView::aPaste()
{

}

void RouteView::updateRouteView()
{
    //SQL sql;
    MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
    TerminalInfo ti = SQL::instance()->getTerminalInfo(myParent->m_routeNbr, myParent->m_routeName, myParent->m_currRouteEndDate);
    startSegment = ti.startSegment;
    endSegment = ti.endSegment;
    // Check for uncomitted changes
    checkChanges();
    route = myParent->m_routeNbr;
    name = myParent->m_routeName;
    startDate = myParent->m_currRouteStartDate;
    endDate = myParent->m_currRouteEndDate;
    alphaRoute = myParent->m_alphaRoute;

    //segmentInfoList = SQL::instance()->getRouteSegmentsInOrder2(route, name, endDate);
    segmentDataList = SQL::instance()->getRouteSegmentsInOrder(route, name, endDate);
    //rd = myParent->routeList.at(myParent->ui->cbRoute->currentIndex());
    rd = myParent->ui->cbRoute->currentData().value<RouteData>();
    myParent->ui->routeViewLabel->setText(tr("Route segments for route %1").arg(rd.toString()));

    qDebug()<<"checkRoute: "+alphaRoute + " '"+name+"' "+endDate;
    chk = new CheckRoute(segmentDataList, config, this);
    chk->setStart(startSegment);
    chk->setEnd(endSegment);
    bIsSequenced = chk->setSeqNbrs();

    if(segmentDataList.count()== 0)
        return;
#if 0
    bIsSequenced = true;
    for(int i=0; i <segmentInfoList.count(); i++)
    {
        segmentInfo si = segmentInfoList.at(i);
        if(si.next < 1 && si.prev < 1 )
        {
            bIsSequenced=false;
            break;
        }
    }
    if(bIsSequenced && ti.startSegment > 0)
    {
        // assign sequence numbers
        bool bWorking = true;
        qint32 nextSequence = ti.startSegment;
        qint32 currSeq = -1;
        int i=0;

        while(bWorking)
        {
            for( i=0; i < segmentInfoList.count(); i ++)
            {
                segmentInfo * si = (segmentInfo*)&segmentInfoList.at(i);
                if(segmentInfoList.at(i).SegmentId == nextSequence)
                {
                    nextSequence = segmentInfoList.at(i).next;
                    si->sequence = ++currSeq;
                    //if(segmentInfoList.at(i).SegmentId == ti.endSegment)
                    if(si->SegmentId == ti.endSegment)
                    {
                        bWorking = false;
                    }
                    break;
                }
            }
            if(i >= segmentInfoList.count())
                break;
            if(currSeq > segmentInfoList.count())
                break;
        }
        nextSequence = ti.endSegment;
        currSeq = -1;
        bWorking = true;
        while(bWorking)
        {
            for( i=0; i < segmentInfoList.count(); i ++)
            {
                segmentInfo * si = (segmentInfo*)&segmentInfoList.at(i);
                if(segmentInfoList.at(i).SegmentId == nextSequence)
                {
                    nextSequence = segmentInfoList.at(i).prev;
                    si->returnSeq = ++currSeq;
                    //if(segmentInfoList.at(i).SegmentId == ti.startSegment)
                    if(si->SegmentId == ti.startSegment)
                    {
                        bWorking = false;
                    }
                    break;
                }
            }
            if(i >= segmentInfoList.count())
                break;
            if(currSeq > segmentInfoList.count())
                break;

        }
    }
#endif

    ui->setSortingEnabled(false);
    sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentDataList);
    //saveSegmentInfoList = segmentInfoList;  // added 5/6/2012 ack
    //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
    connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));

    saveSegmentDataList.clear();
    foreach(SegmentData sd, segmentDataList)
        saveSegmentDataList.append(sd);
    sourceModel->setSequenced(bIsSequenced);

    //routeViewFilterProxyModel* filterProxy = new routeViewFilterProxyModel(this);
    //routeViewSortProxyModel *proxyModel= new routeViewSortProxyModel(this);
    //filterProxy->setTerminals(startRow, endRow);
    //filterProxy->setSourceModel(sourceModel);
    proxymodel->setSourceModel(sourceModel);

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
    ui->setItemDelegateForColumn(sourceModel->TRACTIONTYPE, new TTItemDelegate());
    ui->setItemDelegateForColumn(sourceModel->TYPE, new RTItemDelegate());
    ui->setItemDelegateForColumn(sourceModel->NE, new TurnDelegate("back"));
    ui->setItemDelegateForColumn(sourceModel->NL, new TurnDelegate("ahead"));
    ui->setItemDelegateForColumn(sourceModel->RE, new TurnDelegate("back"));
    ui->setItemDelegateForColumn(sourceModel->RL, new TurnDelegate("ahead"));
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
 QModelIndex Index = indexes.at(0);
 //if(col==0)
 {
  //bool bOk=false;
  qint32 segmentId = Index.data().toInt();
  qint32 endSegment = -1;
  endSegment = SQL::instance()->sequenceRouteSegments(segmentId, segmentDataList, route, name,
                                                      endDate, whichEnd);

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
 sourceModel->reset();
 myParent->setCursor(Qt::ArrowCursor);
}

void RouteView::itemSelectionChanged(QModelIndex index )
{
  QString value = index.model()->index(index.row(),0).data().toString();
  if(value.contains(" "))
  {
    qint32 i = value.indexOf(' ');
    value.truncate(i);
  }

  qint32 segmentId = value.toInt();

  MainWindow * parent = qobject_cast<MainWindow*>(this->m_parent);
  parent->setCursor(QCursor(Qt::WaitCursor));
  if(parent->selectedSegment() == segmentId)
   return; // already selected
  parent->ProcessScript("selectSegment", QString("%1").arg(segmentId));
  parent->setCursor(QCursor(Qt::ArrowCursor));

  OtherRouteView::instance(NULL)->showRoutesUsingSegment(segmentId);
}

void RouteView::StartRoute_S()         //SLOT
{
    //SQL sql;
    MainWindow * myParent = qobject_cast<MainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    //qint32 row = model->currentIndex().row();
    //qint32 col =model->currentIndex().column();
    QModelIndex Index = indexes.at(0);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    SQL::instance()->updateTerminals(route, name, startDate, endDate, segmentId, "S",  ti.route < 0 ? segmentId : ti.endSegment, ti.route < 0 ? "?" : ti.endWhichEnd);
    ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    startSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.startLatLng.lat, ti.startLatLng.lon, myParent->getRouteMarkerImagePath(myParent->m_alphaRoute, true) };
//    webBrowser1.Document.InvokeScript("addRouteStartMarker", objArray);
    QVariantList objArray;
    objArray << ti.startLatLng.lat()<< ti.startLatLng.lon()<<myParent->getRouteMarkerImagePath(alphaRoute, false);
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
    QModelIndex Index = indexes.at(0);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    //SQL::instance()->updateTerminals(route, name,ti.route < 0 ? "1800/01/01" : ti.startDate.toString("yyyy/MM/dd"), endDate, ti.route < 0?-1:ti.startSegment, ti.route < 0?"?":ti.startWhichEnd,        segmentId, "S");
    SQL::instance()->updateTerminals(route, name,ti.route < 0 ? "1800/01/01" : startDate, endDate, ti.route < 0?-1:ti.startSegment, ti.route < 0?"?":ti.startWhichEnd,        segmentId, "S");
    ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    endSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.endLatLng.lat, ti.endLatLng.lon, myParent->getRouteMarkerImagePath(myParent->m_alphaRoute, false) };
//    webBrowser1.Document.InvokeScript("addRouteEndMarker", objArray);
    QVariantList objArray;
    objArray << ti.endLatLng.lat()<< ti.endLatLng.lon()<<myParent->getRouteMarkerImagePath(alphaRoute, false);
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
    QModelIndex Index = indexes.at(0);
    qint32 segmentId = Index.data().toInt();

    TerminalInfo ti = SQL::instance()->getTerminalInfo(route, name, endDate);
    //SQL::instance()->updateTerminals(route, name, ti.route < 0?"1800/01/01":ti.startDate.toString("yyyy/MM/dd"), endDate, segmentId, "E", ti.route < 0 ? -1 : ti.endSegment, ti.route < 0 ? "?" : ti.endWhichEnd);
    SQL::instance()->updateTerminals(route, name, ti.route < 0?"1800/01/01":startDate, endDate, segmentId, "E", ti.route < 0 ? -1 : ti.endSegment, ti.route < 0 ? "?" : ti.endWhichEnd);
    ti = SQL::instance()->getTerminalInfo(route,myParent-> m_routeName, endDate);
    startSegment = segmentId;
    updateRouteView();
//    Object[] objArray = new Object[] { ti.startLatLng.lat, ti.startLatLng.lon, getRouteMarkerImagePath(m_alphaRoute, true) };
//    webBrowser1.Document.InvokeScript("addRouteStartMarker", objArray);
    QVariantList objArray;
    objArray << ti.startLatLng.lat()<< ti.startLatLng.lon()<<myParent->getRouteMarkerImagePath(alphaRoute, false);
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
    QModelIndex Index = indexes.at(0);
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
    objArray << ti.endLatLng.lat()<< ti.endLatLng.lon()<<myParent->getRouteMarkerImagePath(alphaRoute, false);
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
    for(int i =0; i < segmentDataList.count(); i++)
    {
        SegmentData sd = segmentDataList.at(i);
        if(sd.sequence() == 0)
        {
            startSeg = sd.segmentId();
            startRow = i;
            SegmentInfo sd1 = SQL::instance()->getSegmentInfo(sd.next());
            if(SQL::instance()->Distance(sd.startLat(), sd.startLon(), sd1.startLat(), sd1.startLon()) < .02
               ||SQL::instance()->Distance(sd.startLat(), sd.startLon(), sd1.endLat(), sd1.endLon()) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(sd.returnSeq() == 0)
        {
            endSeg = sd.segmentId();
            endRow = i;
            SegmentInfo sd1 = SQL::instance()->getSegmentInfo(sd.prev());
            if(SQL::instance()->Distance(sd.startLat(), sd.startLon(), sd1.startLat(), sd1.startLon()) < .02
               || SQL::instance()->Distance(sd.startLat(), sd.startLon(), sd1.endLat(), sd1.endLon()) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(sd.sequence() == -1 && sd.returnSeq()==-1)
        {
            bIsSequenced = false;
            myParent->setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
    }
    if(!bIsSequenced)
        return;
    for(int i =0; i < segmentDataList.count(); i++)
    {
        SegmentData sd = segmentDataList.at(i);
        SQL::instance()->updateRoute(route, name, endDate, sd.segmentId(), sd.next(), sd.prev(), sd.trackUsage());
    }

    SQL::instance()->updateTerminals(route, name, startDate, endDate, startSeg, startWhichEnd, endSeg, endWhichEnd);
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

void RouteView::deleteSegment()
{
    //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    QModelIndex ix = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = ix.data().toInt();

    sourceModel->deleteRow(segmentId, ix);

}

void RouteView::unDeleteSegment()
{
    //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    QModelIndex ix = indexes.at(RouteViewTableModel::SEGMENTID);
    qint32 segmentId = ix.data().toInt();

    sourceModel->unDeleteRow(segmentId, ix);
}

void RouteView::commitChanges()
{
 bool rslt = sourceModel->commitChanges();
 if(rslt)
 {
  myParent->refreshRoutes();
  myParent->btnDisplayRouteClicked();
 }
 myParent-> setCursor(Qt::ArrowCursor);
}

bool RouteView::bUncomittedChanges()
{
 if(sourceModel->changedMap.values().count()>0)
  return true;
 else
  return false;
}

void RouteView::on_selectSegment_triggered()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 QModelIndex Index = indexes.at(0);
 qint32 segmentId = Index.data().toInt();
 MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
 myParent->segmentSelected(0,segmentId);
 emit selectSegment(segmentId);
}

void RouteView::editSegment()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 qint32 segmentId = indexes.at(0).data().toInt();
 SegmentData sd = segmentDataList.at(proxymodel->mapToSource(indexes.at(0)).row());
 SegmentInfo si = SQL::instance()->getSegmentInfo(segmentId);
 EditSegmentDialog* dlg = new EditSegmentDialog(&sd, si);
 dlg->exec();
}

void RouteView::on_segmentSelected(int, int segmentId)
{
 int row = sourceModel->getRow(segmentId);
 if(row >= 0)
 {
  QModelIndex modelIndex = proxymodel->mapFromSource(sourceModel->index(row,1));
  //ui->setCurrentIndex(modelIndex);
  ui->selectRow(modelIndex.row());
 }
}

void RouteView::checkChanges()
{
 if(sourceModel->changedMap.values().count() > 0)
 {
  QMessageBox::StandardButtons rslt;
  MainWindow* myParent = qobject_cast<MainWindow*>(m_parent);
  rslt = QMessageBox::warning(myParent,tr("Commit changes"), tr("There are uncommited changes to the current route. Do you wish to save them?") , QMessageBox::Save | QMessageBox::Discard|QMessageBox::Cancel);
  switch (rslt)
  {
  case QMessageBox::Save:
   //commitChanges();
   sourceModel->commitChanges();
   break;
  case QMessageBox::Discard:
   sourceModel->changedMap.values().clear();
   break;
  default:
   break;
  }
 }
}
