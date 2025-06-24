#include "streetview.h"
#include "dateeditdelegate.h"
#include "qclipboard.h"
#include "qheaderview.h"
#include "qmenu.h"
#include "sql.h"
#include "streetstablemodel.h"
#include "webviewbridge.h"

StreetView::StreetView(QObject *parent) {
    m_parent = parent;
    config = Configuration::instance();
    sql = SQL::instance();
    myParent = qobject_cast<MainWindow*>(m_parent);

    ui = myParent->ui->tblStreetView;
    ui->setAlternatingRowColors(true);
    ui->setSelectionBehavior(QAbstractItemView::SelectRows );
    ui->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->setSortingEnabled(true);

    ui->setItemDelegateForColumn(StreetsTableModel::STARTDATE, new DateEditDelegate());
    ui->setItemDelegateForColumn(StreetsTableModel::ENDDATE, new DateEditDelegate());
    sourceModel =  StreetsTableModel::instance();
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(sourceModel);

    ui->setModel(proxyModel);
    ui->resizeColumnsToContents();
    ui->horizontalHeader()->stretchLastSection();
    //connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tablev_CustomContextMenu(QPoint)));
    connect(ui, &QTableView::customContextMenuRequested, this,[=](const QPoint& pt){
        tablev_CustomContextMenu(pt);
    });
    connect(sourceModel, &StreetsTableModel::streetInfoChanged, this,[=](StreetInfo sti, StreetsTableModel::Action act){
        //sourceModel->streetChanged(sti);
        int row =sourceModel->findRow(sti.rowid);
        QModelIndex srcIndex = sourceModel->index(row,0);
        QModelIndex srtIndex = proxyModel->mapFromSource(srcIndex);
        ui->scrollTo(srtIndex);
    });
    if(config->sv.colWidths.count() != sourceModel->columnCount(QModelIndex()))
    {
        config->sv.colWidths.clear();
        for(int i=0; i < sourceModel->columnCount(QModelIndex()); i++)
        {
            config->sv.colWidths.append(ui->columnWidth(i));
        }
    }

    connect(ui->horizontalHeader(), &QHeaderView::sectionResized, this,
            [=](int logicalIndex, int oldSize, int newSize){
        config->sv.state = ui->horizontalHeader()->saveState();
        config->sv.colWidths.replace(logicalIndex,newSize);
    });
    qApp->processEvents();
    ui->horizontalHeader()->restoreState(config->sv.state);

    if(!config->sv.colWidths.isEmpty())
    {
        for(int i=0; i < config->sv.colWidths.count(); i++)
        {
            ui->setColumnWidth(i, config->sv.colWidths.at(i));
        }
    }

    connect(SQL::instance(), &SQL::segmentChanged,this,[=](SegmentInfo si, SQL::CHANGETYPE t){
        segmentChanged(si, t);
    });

}

StreetsTableModel* StreetView::model()
{
    return sourceModel;
}

void StreetView::tablev_CustomContextMenu(const QPoint &pt)
{
    QMenu tablMenu;
    curRow = ui->rowAt(pt.y());
    curCol = ui->columnAt(pt.x());
    int srcRow = proxyModel->mapToSource(proxyModel-> index(curRow, curCol)).row();
    sti0 = sourceModel->getStreetInfo(srcRow);
    QItemSelectionModel * selModel = ui->selectionModel();
    QModelIndexList indexes = selModel->selectedIndexes();
    if(indexes.isEmpty())
        return;
    QModelIndex Index = indexes.at(StreetsTableModel::STREET);
    QString street = Index.data().toString();
    QString location =indexes.at(StreetsTableModel::LOCATION).data().toString();
    int streetId = indexes.at(StreetsTableModel::STREETID).data().toInt();
    QAction* act = new QAction(tr("Update %1 bounds").arg(street),this);
    //StreetInfo* sti = sourceModel->getStreet(street);
    StreetInfo* sti = nullptr;
    QList<StreetInfo*> stiList = sourceModel->getStreetName(street, location);
    if(!stiList.isEmpty())
        sti = stiList.at(0);
    if(!sti)
        return;

    connect(act, &QAction::triggered, this, [=]{
        QStringList names;
        names.append(street);
        //QList<SegmentInfo> segments = sourceModel->getSegmentsForStreet(names);
        QList<SegmentInfo*> segments = sourceModel->getStreetsSegments(sti0);
        foreach (SegmentInfo* si , segments) {
            // if(!sti0.segments.contains(si->segmentId()))
            //     sti0.segments.append(si->segmentId());
            if(!sti0.bounds.checkValid())
                sti0.bounds = Bounds();
            sti0.updateSegmentInfo(*si);
        }
        sourceModel->updateStreetName(sti0);

    });
    tablMenu.addAction(act);
    act = new QAction(tr("Refresh table"),this);
    tablMenu.addAction(act);
    connect(act, &QAction::triggered, this, [=]{
        //QList<StreetInfo> streets = sourceModel->getStreets();
        QList<StreetInfo> streets = sourceModel->getStreetInfoList();
        sourceModel->setList(streets);
    });
    act = new QAction(tr("Delete %1").arg(sti->street),this);
    tablMenu.addAction(act);
    connect(act, &QAction::triggered, this, [=]{
        //sourceModel->deleteStreet(street);
        sourceModel->deleteStreetDef(srcRow);
    });
    act = new QAction("Copy cell text", this);
    tablMenu.addAction(act);
    connect(act, &QAction::triggered, this, [=]{
        QModelIndex index = ui->currentIndex();
        QModelIndex srcIndex = proxyModel->mapToSource(index);
        QClipboard *clip = QApplication::clipboard();
        clip->setText(srcIndex.data().toString());
    });
    act = new QAction(tr("check dates"),this);
    tablMenu.addAction(act);
    connect(act, &QAction::triggered, this, [=]{
        QStringList names;
        if(sti)
        {
            QList<StreetInfo*>* list = sourceModel->getStreetNames(sti->streetId,&names);
        }
    });
    if(sti->startLatLng.isValid() && sti->endLatLng.isValid())
    {
        act = new QAction(tr("Show ends"), this);
        tablMenu.addAction(act);
        connect(WebViewBridge::instance(), &WebViewBridge::on_pinClicked,this,[=](int pinId, LatLng latLng,
              QString street, int streetid, QString location, int seq){
            if(streetId < 0)
                return;
            if(pinId == 0)
                sti->startLatLng = latLng;
            else
                sti->endLatLng = latLng;
            sti->bounds = Bounds();
            sti->updateBounds();
            sourceModel->updateStreetName(*sti);
        });
        connect(act, &QAction::triggered,this, [=]{

            WebViewBridge::instance()->processScript("clearPins");
            QString title = sti->street;
            if(!location.isEmpty())
                title.append(QString("(%1").arg(sti->location));
            QVariantList objArray;
            objArray << sti->startLatLng.lat() << sti->startLatLng.lon() << sti->endLatLng.lat()
                     << sti->endLatLng.lon() << title << sti->streetId << sti->location
                     << true << sti->sequence;
            WebViewBridge::instance()->processScript("showStreetPins",objArray);
            qApp->processEvents();
        });

        if(curCol == StreetsTableModel::ENDDATE)
        {
            QModelIndex ix = sourceModel->index(srcRow,StreetsTableModel::ENDDATE );
            int seq = ix.data().toInt();
            if(seq == 0)
            {
                tablMenu.addSeparator();
                act = new QAction(tr("remove end date"),this);
                connect(act, &QAction::triggered,this,[=]{
                    int streetid = sourceModel->index(srcRow,StreetsTableModel::STREETID).data().toInt();
                    StreetInfo* sti = sourceModel->getStreetDef(streetId);
                    sti->dateEnd = QDate();
                    sourceModel->updateStreetDef(*sti);
                });
                tablMenu.addAction(act);

            }
        }

        act = new QAction(tr("Show all segment's ends"), this);
        tablMenu.addAction(act);
        connect(act, &QAction::triggered,this, [=]{
            QStringList names;
            QList<StreetInfo*>* list = sourceModel->getStreetNames(sti->streetId,&names);
            QList<SegmentInfo> segments = sourceModel->getSegmentsForStreet(names);
            WebViewBridge::instance()->processScript("clearPins");
            QString title;
            QVariantList objArray;
            int seq =0;
            foreach (SegmentInfo si, segments) {
                title = si.streetName();
                if(!location.isEmpty())
                    title.append(QString("(%1").arg(si.location()));

                objArray.clear();
                objArray << si.startLat() << si.startLon() << si.endLat() << si.endLon()
                         << title << si.segmentId() << si.location() << false << seq++ ;
                WebViewBridge::instance()->processScript("showStreetPins",objArray);
                qApp->processEvents();
            }
        });
    }
    tablMenu.exec(QCursor::pos());
}

void StreetView::segmentChanged(SegmentInfo si, SQL::CHANGETYPE t)
{
    if(si.streetId()> 0)
    {
        QList<StreetInfo*>* streets = StreetsTableModel::instance()->getStreetNames(si.streetId());
        foreach (StreetInfo* sti, *streets) {
            switch (t) {
            case SQL::MODIFYSEG:
            case SQL::ADDSEG:
                sti->location = si.location();
                sti->startLatLng = si.getStartLatLng();
                sti->endLatLng =si.getEndLatLng();

                if(!sti->segments.contains(si.segmentId()))
                    sti->segments.append(si.segmentId());

                break;
            case SQL::DELETESEG:
                if(sti->segments.contains(si.segmentId()))
                    sti->segments.removeOne(si.segmentId());
                break;
            default:
                break;
            }
            sti->bounds = Bounds();
            sti->updateBounds();
            StreetsTableModel::instance()->updateStreetName(*sti);
        }
    }
}

