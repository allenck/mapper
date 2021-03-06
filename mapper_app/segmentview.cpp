#include "segmentview.h"
#include "editsegmentdialog.h"

SegmentView::SegmentView(Configuration *cfg, QObject *parent) :
    QObject(parent)
{
    m_parent = parent;
    config = Configuration::instance();
    //sql->setConfig(config)
    sql = SQL::instance();
    mainWindow* myParent = qobject_cast<mainWindow*>(m_parent);
    ui = myParent->ui->tblSegmentView;
    connect(ui->verticalHeader(), SIGNAL(sectionCountChanged(int,int)), this, SLOT(Resize(int,int)));

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

    addToRouteAct = new QAction(tr("Add in UpdateRoute"),this);
    addToRouteAct->setStatusTip(tr("Add this segment to the current route."));
    connect(addToRouteAct, SIGNAL(triggered()),this, SLOT(addToRoute()));

    editSegmentAct = new QAction(tr("Edit segment"), this);
    editSegmentAct->setStatusTip(tr("Edit this segment's properties"));
    connect(editSegmentAct, SIGNAL(triggered(bool)), this, SLOT(editSegment()));

    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(tablev_customContextMenu( const QPoint& )));
    sourceModel = new segmentViewTableModel();
    proxymodel = new segmentViewSortProxyModel(this);
    proxymodel->setSourceModel(sourceModel);
    ui->setModel(proxymodel);
    //connect(this, SIGNAL(sendRows(int, int)), proxymodel, SLOT(getRows(int,int)));

    connect(ui, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelectionChanged(QModelIndex)));

    myParent->ui->tabWidget->setTabText(1, "Intersecting Segments");
}
void SegmentView::Resize (int oldcount,int newcount)
{
    Q_UNUSED(oldcount)
    Q_UNUSED(newcount)

    ui->resizeColumnsToContents ();
    ui->resizeRowsToContents ();
}
//create table input context menu
void SegmentView::tablev_customContextMenu( const QPoint& pt)
{
    curRow = ui->rowAt(pt.y());
    curCol = ui->columnAt(pt.x());
    // check is item in QTableView exist or not
    if(boolGetItemTableView(ui))
    {
        //menu = QMenu(m_parent*);

        menu.addAction(copyAction);
        menu.addAction(pasteAction);
        //int row = ui->rowAt(pt.y());
        QItemSelectionModel * model = ui->selectionModel();
        QModelIndexList indexes = model->selectedIndexes();
        if(indexes.size() > 0)
        {
        QModelIndex ix = indexes.at(0);
         if(!(ix.data(Qt::CheckStateRole) == Qt::Checked))
            menu.addAction(addToRouteAct);
        }
        menu.addAction(editSegmentAct);
        menu.exec(QCursor::pos());
    }
}
//get QTableView selected item
bool SegmentView::boolGetItemTableView(QTableView *table)
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
void SegmentView::aCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    //QItemSelectionModel model = ui->selectionModel();
    //QModelIndex index =currentIndex();
    //QAbstractItemModel* item = index.model();
    if(currentIndex.isValid())
        clipboard->setText(currentIndex.data().toString());

}
void SegmentView::aPaste()
{

}

void SegmentView::showSegmentsAtPoint(double lat, double lon, qint32 SegmentId)
{
    SegmentInfo siIn;
    SegmentInfo si;
    //SQL sql;
    mainWindow* myParent = qobject_cast<mainWindow*>(m_parent);
    double a1 = 0;

    siIn = sql->getSegmentInfo(SegmentId);
    if (siIn.segmentId < 1)
    {
        qDebug() << "segmentID " + QString("%1").arg(SegmentId) + " not found";
        return;

    }
//    if (siIn.bearingStart == null || siIn.bearingEnd == null)
//        return;
    //if (lat == siIn.startLat && lon == siIn.startLon)
    if(qAbs(lat - siIn.startLat)< .00000001 && qAbs(lon - siIn.startLon) < .00000001)
    {
        siIn.whichEnd = "S";
        a1 = siIn.bearingStart.getBearing();
    }
    else
    {
        siIn.whichEnd = "E";
        a1 = siIn.bearingEnd.getBearing();
    }
    // get all the points within .020km
    myParent->setCursor(QCursor(Qt::WaitCursor));
    sourceModel->reset();
    myArray = sql->getIntersectingSegments(lat, lon, .020, siIn.routeType);
    myParent->setCursor(QCursor(Qt::ArrowCursor));

    if(myArray.count()== 0)
        return;

    ui->setSortingEnabled(false);

    sourceModel = new segmentViewTableModel(myArray, lat, lon, myParent->m_routeNbr, myParent->m_currRouteEndDate, this);
    proxymodel->setSourceModel(sourceModel);

    // get the row of the start segment and the end segment
    int numRows = proxymodel->rowCount();
    qint32 startRow =-1, endRow = -1;
    for(int i = 0; i < numRows; i++)
    {
        if(ui->isRowHidden(i))
            continue;
        //int segmentId = proxymodel->index(i, 0).data(Qt::DisplayRole).toInt();
//        if(segmentId == ti.startSegment)
//            startRow = i;
//        if(segmentId == ti.endSegment)
//            endRow = i;
    }
    emit sendRows (startRow, endRow);
    //proxyModel->setTerminals(startRow, endRow);
    ui->setModel(proxymodel);
    ui->setSortingEnabled(true);

    ui->horizontalHeader()->setStretchLastSection(false);
    ui->horizontalHeader()->resizeSection(0,60);
    //ui->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
    ui->horizontalHeader()->resizeSection(2,40);
    ui->horizontalHeader()->resizeSection(3,150);
    ui->horizontalHeader()->resizeSection(4, 40);
    ui->horizontalHeader()->resizeSection(5, 70);
    ui->horizontalHeader()->resizeSection(6, 70);
}
void SegmentView::addToRoute()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 qint32 SegmentId = indexes.at(0).data().toInt();
 mainWindow * parent = qobject_cast<mainWindow*>(this->m_parent);
 if(parent->routeDlg == 0)
 {
  parent->routeDlg = new RouteDlg(config, parent);
  //parent->routeDlg->Configuration ( config);
  //routeDlg->SegmentChanged += new segmentChangedEventHandler(segmentChanged);
  connect(parent->routeDlg, SIGNAL(SegmentChangedEvent(qint32, qint32)),parent, SLOT(segmentChanged(qint32,qint32)));
 }
 if(parent->m_segmentStatus == "Y")
   parent->ProcessScript("selectSegment", QString("%1").arg(SegmentId));
 else
 {
  SegmentInfo si = sql->getSegmentInfo(SegmentId);
  parent->displaySegment(SegmentId, si.description, si.oneWay, si.oneWay == "N" ? "#00FF00" : "#045fb4", true);
 }
 parent->routeDlg->setSegmentId(SegmentId); // do before setting route!
 int ix = parent->ui->cbRoute->currentIndex();
 if(ix >= 0)
 {
  RouteData rd = parent->routeList.at(ix);
  parent->routeDlg->setRouteData(rd);
 }
 parent->routeDlg->show();
 parent->routeDlg->raise();
 parent->routeDlg->activateWindow();
}
void SegmentView::editSegment()
{
 QItemSelectionModel * model = ui->selectionModel();
 QModelIndexList indexes = model->selectedIndexes();
 qint32 segmentId = indexes.at(0).data().toInt();
 EditSegmentDialog* dlg = new EditSegmentDialog(segmentId);
 dlg->exec();
}

void SegmentView::itemSelectionChanged(QModelIndex index)
{
    QItemSelectionModel * model = ui->selectionModel();
    QModelIndexList indexes = model->selectedIndexes();
    qint32 segmentId =indexes.at(0).data().toInt();
    mainWindow * parent = qobject_cast<mainWindow*>(this->m_parent);
    parent->ProcessScript("isSegmentDisplayed", QString("%1").arg(segmentId));
    if(parent->m_segmentStatus == "Y")
        parent->ProcessScript("selectSegment", QString("%1").arg(segmentId));
    else
    {
        SegmentInfo si = sql->getSegmentInfo(segmentId);
        parent->displaySegment(segmentId, si.description, si.oneWay, si.oneWay == "N" ? "#00FF00" : "#045fb4", true);
    }

}
