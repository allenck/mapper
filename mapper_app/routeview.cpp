//#include "routeview.h"
#include "sql.h"
//#include <QHeaderView>
#include "mainwindow.h"
#include "editsegmentdialog.h"
#include "webviewbridge.h"

RouteView::RouteView(QObject* parent )
{
    m_parent = parent;

    config = Configuration::instance();

    //sql.setConfig(config);
    //headers << "" << "Item" << "Name" << "1 way" << "Next" << "Prev" << "Dir" << "Seq" << "RSeq" << "StartDate" << "EndDate";
    mainWindow* myParent = qobject_cast<mainWindow*>(m_parent);
    ui = myParent->ui->tblRouteView;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

    //ui->setColumnCount(headers.count());
    //ui->setHorizontalHeaderLabels(headers);

    ui->resizeColumnsToContents();

    ui->setAlternatingRowColors(true);
    ui->setColumnWidth(0,25);
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

    reSequenceAction = new QAction(tr("&Start route here"), this);
    reSequenceAction->setStatusTip(tr("Start route at this segment"));
    reSequenceAction->setShortcut(tr("Alt_Ctrl+S"));
    connect(reSequenceAction, SIGNAL(triggered()), this, SLOT(reSequenceRoute()));

    startTerminalStartAct = new QAction(tr("Start at start"), this);
    startTerminalStartAct->setStatusTip(tr("Start terminal is at start of segment"));
    connect(startTerminalStartAct, SIGNAL(triggered()), this, SLOT(StartRoute_S()));

    startTerminalEndAct = new QAction(tr("Start at end"), this);
    startTerminalEndAct->setStatusTip(tr("Start terminal is at end of segment"));
    connect(startTerminalEndAct, SIGNAL(triggered()), this, SLOT(StartRoute_E()));

    endTerminalStartAct = new QAction(tr("End terminal at start"), this);
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
    sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentInfoList);
    saveChangesAct = new QAction(tr("Commit changes"),this);
    saveChangesAct->setStatusTip(tr("Save any uncommitted changes"));
    //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
    connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));

    myParent->proxyModel = proxymodel = new RouteViewSortProxyModel(this);
    myParent->proxyModel->setSourceModel(sourceModel);
    ui->setModel(myParent->proxyModel);
    connect(this, SIGNAL(sendRows(int, int)), sourceModel, SLOT(getRows(int,int)));

    //connect(ui, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemChanged(QTableWidgetItem*)));
    //connect(ui, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(handleSelectionChanged(QTableWidgetItem *)));
    connect(ui, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelectionChanged(QModelIndex)));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), ui,
            SLOT(dataChanged(QModelIndex,QModelIndex)));
    //connect(myParent->saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));

    connect(webViewBridge::instance(), SIGNAL(segmentSelected(qint32,qint32)), this, SLOT(on_segmentSelected(int,int)));


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
        if(sourceModel->isSegmentMarkedForDelete(segmentId))
            menu.addAction(unDeleteSegmentAct);
        else
            menu.addAction(deleteSegmentAct);
        //if(curRow == 0)
        menu.addAction(selectSegmentAct);
        menu.addAction(reSequenceAction);
//        if(!startTerminal)
//        {
        startTerminal = menu.addMenu(tr("Set start terminal..."));
        startTerminal->addAction(startTerminalStartAct);
        startTerminal->addAction(startTerminalEndAct);
        endTerminal =menu.addMenu(tr("Set end terminal..."));
        endTerminal->addAction(endTerminalStartAct);
        endTerminal->addAction(endTerminalEndAct);
//        }
        menu.addAction(editSegmentAct);
        if(sourceModel->changedRows.count() > 0)
        {
            menu.addSeparator();
            menu.addAction(saveChangesAct);
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
    mainWindow* myParent = qobject_cast<mainWindow*>(m_parent);
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

    segmentInfoList = SQL::instance()->getRouteSegmentsInOrder2(route, name, endDate);

    qDebug()<<"checkRoute: "+alphaRoute + " '"+name+"' "+endDate;
    chk = new checkRoute(segmentInfoList, config, this);
    chk->setStart(startSegment);
    chk->setEnd(endSegment);
    bIsSequenced = chk->setSeqNbrs();

    if(segmentInfoList.count()== 0)
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
    sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentInfoList);
    //saveSegmentInfoList = segmentInfoList;  // added 5/6/2012 ack
    //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
    connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));

    saveSegmentInfoList.clear();
    foreach(SegmentInfo si, segmentInfoList)
        saveSegmentInfoList.append(si);
    sourceModel->setSequenced(bIsSequenced);

    //routeViewFilterProxyModel* filterProxy = new routeViewFilterProxyModel(this);
    //routeViewSortProxyModel *proxyModel= new routeViewSortProxyModel(this);
    //filterProxy->setTerminals(startRow, endRow);
    //filterProxy->setSourceModel(sourceModel);
    myParent->proxyModel->setSourceModel(sourceModel);

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
    ui->setModel(myParent->proxyModel);
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
bool ascending_si_segmentId( const SegmentInfo & s1 , const SegmentInfo & s2 )
{
 //cout<<"\n" << __FUNCTION__;
 return s1.segmentId < s2.segmentId;
}
void RouteView::reSequenceRoute()
{
 //SQL sql;
 mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
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
  endSegment = SQL::instance()->sequenceRouteSegments(segmentId, segmentInfoList, route, name, endDate);

  QList<SegmentInfo> old = segmentInfoList;
  //compareSequenceClass comparer = new compareSequenceClass();
  //segmentList.Sort(comparer);
  qSort(segmentInfoList.begin(),segmentInfoList.end(),ascending_si_segmentId);
  bIsSequenced = true;
  for(int i=0; i < segmentInfoList.count(); i++)
  {
   if(segmentInfoList.at(i).sequence == -1 && segmentInfoList.at(i).returnSeq ==-1)
   {
       bIsSequenced = false;
       break;
   }
  }
  //populateList();
  sourceModel = new RouteViewTableModel(route, name, QDate::fromString(startDate, "yyyy/MM/dd"), QDate::fromString(endDate, "yyyy/MM/dd"), segmentInfoList);
  myParent->proxyModel->setSourceModel(sourceModel);
  //connect(saveChangesAct, SIGNAL(triggered()), sourceModel, SLOT(commitChanges()));
  connect(saveChangesAct, SIGNAL(triggered(bool)), this, SLOT(commitChanges()));

  ui->setModel(myParent->proxyModel);
  //sourceModel->reset();
  sourceModel->setSequenced(bIsSequenced);

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

  mainWindow * parent = qobject_cast<mainWindow*>(this->m_parent);
  parent->setCursor(QCursor(Qt::WaitCursor));
  parent->ProcessScript("selectSegment", QString("%1").arg(segmentId));
  parent->setCursor(QCursor(Qt::ArrowCursor));

  OtherRouteView::instance(NULL)->showRoutesUsingSegment(segmentId);
}

void RouteView::StartRoute_S()         //SLOT
{
    //SQL sql;
    mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
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
    webViewBridge::instance()->processScript("addRouteStartMarker", objArray);

}
void RouteView::EndRoute_S()           // SLOT
{
    //SQL sql;
    mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
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
    webViewBridge::instance()->processScript("addRouteEndMarker", objArray);

}
void RouteView::StartRoute_E()         // SLOT
{
    //SQL sql;
    mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
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
    webViewBridge::instance()->processScript("addRouteStartMarker", objArray);

}
void RouteView::EndRoute_E()       // SLOT
{
    //SQL sql;
    mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
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
    webViewBridge::instance()->processScript("addRouteEndMarker", objArray);

}
void RouteView::updateTerminals()
{
    mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);
    //QItemSelectionModel * model = ui->selectionModel();
    bIsSequenced = true;
    int startSeg = -1, endSeg = -1, startRow=-1, endRow=-1;
    QString startWhichEnd = "S", endWhichEnd = "S";
    myParent->setCursor(QCursor(Qt::WaitCursor));
    for(int i =0; i < segmentInfoList.count(); i++)
    {
        SegmentInfo si = segmentInfoList.at(i);
        if(si.sequence == 0)
        {
            startSeg = si.segmentId;
            startRow = i;
            SegmentInfo si1 = SQL::instance()->getSegmentInfo(si.next);
            if(SQL::instance()->Distance(si.startLat, si.startLon, si1.startLat, si1.startLon) < .02 ||SQL::instance()->Distance(si.startLat, si.startLon, si1.endLat, si1.endLon) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(si.returnSeq == 0)
        {
            endSeg = si.segmentId;
            endRow = i;
            SegmentInfo si1 = SQL::instance()->getSegmentInfo(si.prev);
            if(SQL::instance()->Distance(si.startLat, si.startLon, si1.startLat, si1.startLon) < .02 || SQL::instance()->Distance(si.startLat, si.startLon, si1.endLat, si1.endLon) < .02)
            {
                startWhichEnd = "E";
            }
        }
        if(si.sequence == -1 && si.returnSeq==-1)
        {
            bIsSequenced = false;
            myParent->setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
    }
    if(!bIsSequenced)
        return;
    for(int i =0; i < segmentInfoList.count(); i++)
    {
        SegmentInfo si = segmentInfoList.at(i);
        SQL::instance()->updateRoute(route, name, endDate, si.segmentId, si.next, si.prev);
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

//bool sortbyrow( const RowChanged & s1 , const RowChanged & s2 )
//{
//    return s1.row > s2.row;
//}

//void RouteView::commitChanges()
//{
// //mainWindow * myParent = qobject_cast<mainWindow*>(m_parent);

// // sort rows in descending sequence.
// qSort(*sourceModel->changedRows.begin(),*sourceModel->changedRows.end(), sortbyrow );

// for(int i=0; i < sourceModel->changedRows.count(); i++)
// {
//  int row = sourceModel->changedRows.at(i)->row;
//  //SegmentInfo si = sourceModel->getList().at(row);
//  SegmentInfo si = saveSegmentInfoList.at(row);
////  for(int j = 0; j < sourceModel->listOfSegments.count(); j++)
////  {
////   if(sourceModel->listOfSegments.at(j).segmentId == sourceModel->changedRows.at(i)->segmentId)
////   {
////    si = sourceModel->listOfSegments.at(j);
////    break;
////   }
////  }
//  if(si.segmentId != sourceModel->changedRows.at(i)->segmentId)
//  {
//      qDebug() << "Error, wrong segmentId found!";
//      return;
//  }
//  //segmentInfo siOld = segmentInfoList.at(row);
//  SegmentInfo siOld = saveSegmentInfoList.at(row);

//  RouteData rd = SQL::instance()->getRouteData(route, siOld.segmentId, siOld.startDate, siOld.endDate);
//  if(rd.route <= 0)
//  {
//      qDebug()<< "route data not found";
//      qDebug() << "route="+ QString("%1").arg(route)+ " segmentId="+ QString("%1").arg(siOld.segmentId)+ " "+ siOld.startDate + " "+siOld.endDate;
//      return;
//  }

//  SQL::instance()->BeginTransaction("updateRoute");
//  if(siOld.oneWay != si.oneWay || siOld.tracks != si.tracks || siOld.length != si.length  || siOld.bNeedsUpdate)
//  {
//   SQL::instance()->updateSegmentDescription(si.segmentId, si.description, si.oneWay, si.tracks, si.length);
//  }

//  if(!SQL::instance()->deleteRouteSegment(route, name, si.segmentId, siOld.startDate, siOld.endDate))
//      return;
//  if(siOld.startDate < startDate)
//  {
//   // add back segment used before route start date
//   QString newEndDate = QDate::fromString(startDate, "yyyy/MM/dd").addDays(-1).toString("yyyy/MM/dd");
//   if(!SQL::instance()->addSegmentToRoute(route, name, startDate, newEndDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave))
//       return;
//  }
//  if(siOld.endDate > endDate)
//  {
//   QString newStartDate = QDate::fromString(endDate, "yyyy/MM/dd").addDays(-1).toString("yyyy/MM/dd");
//   if(!SQL::instance()->addSegmentToRoute(route, name, newStartDate, endDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave))
//       return;
//  }
//  if(sourceModel->changedRows.at(i)->bDeleted)
//  {
//   //myParent->ProcessScript("clearPolyline", QString("%1").arg(si.segmentId));
//   webViewBridge::instance()->processScript("clearPolyline", QString("%1").arg(si.segmentId));
//  }
//  if(sourceModel->changedRows.at(i)->bChanged && !sourceModel->changedRows.at(i)->bDeleted)
//  {
//      if(!SQL::instance()->addSegmentToRoute(route, name, si.startDate, si.endDate, si.segmentId, rd.companyKey, rd.tractionType, si.bearing.strDirection(),si.next, si.prev, si.normalEnter, si.normalLeave, si.reverseEnter, si.reverseLeave))
//          return;
//  }
//  SQL::instance()->CommitTransaction("updateRoute");
//  sourceModel->removeRow(row);
// }
// sourceModel->changedRows.clear();
// //myParent->refreshRoutes();
// emit refreshRoutes();
//}
void RouteView::commitChanges()
{
 sourceModel->commitChanges();
}

bool RouteView::bUncomittedChanges()
{
 if(sourceModel->changedRows.count()>0)
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
 emit selectSegment(segmentId);
}

void RouteView::editSegment()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 qint32 segmentId = indexes.at(0).data().toInt();
 EditSegmentDialog* dlg = new EditSegmentDialog(segmentId);
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
 if(sourceModel->changedRows.count() > 0)
 {
  QMessageBox::StandardButtons rslt;
  mainWindow* myParent = qobject_cast<mainWindow*>(m_parent);
  rslt = QMessageBox::warning(myParent,tr("Save changes"), tr("There are uncommited changes to the current route. Do you wish to save them?") , QMessageBox::Save | QMessageBox::Discard|QMessageBox::Cancel);
  switch (rslt)
  {
  case QMessageBox::Save:
   //commitChanges();
   sourceModel->commitChanges();
   break;
  case QMessageBox::Discard:
   sourceModel->changedRows.clear();
   break;
  default:
   break;
  }
 }
}
