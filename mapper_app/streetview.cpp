#include "streetview.h"
#include "dateeditdelegate.h"
#include "qclipboard.h"
#include "qheaderview.h"
#include "qmenu.h"
#include "sql.h"
#include "streetstablemodel.h"
#include "webviewbridge.h"

StreetView::StreetView() {
    config = Configuration::instance();
    sql = SQL::instance();
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::SingleSelection );
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(true);

    setItemDelegateForColumn(StreetsTableModel::STARTDATE, new DateEditDelegate());
    setItemDelegateForColumn(StreetsTableModel::ENDDATE, new DateEditDelegate());
    sourceModel = new StreetsTableModel(this);
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(sourceModel);

    setModel(proxyModel);
    resizeColumnsToContents();
    horizontalHeader()->stretchLastSection();
    //connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tablev_CustomContextMenu(QPoint)));
    connect(this, &QTableView::customContextMenuRequested, this,[=](const QPoint& pt){
        tablev_CustomContextMenu(pt);
    });
    connect(sourceModel, &StreetsTableModel::streetInfoChanged, this,[=](StreetInfo sti, StreetsTableModel::Action act){
        //sourceModel->streetChanged(sti);
        int row =sourceModel->findRow(sti.rowid);
        QModelIndex srcIndex = sourceModel->index(row,0);
        QModelIndex srtIndex = proxyModel->mapFromSource(srcIndex);
        scrollTo(srtIndex);
    });
    if(config->sv.colWidths.count() != sourceModel->columnCount(QModelIndex()))
    {
        config->sv.colWidths.clear();
        for(int i=0; i < sourceModel->columnCount(QModelIndex()); i++)
        {
            config->sv.colWidths.append(columnWidth(i));
        }
    }

    connect(horizontalHeader(), &QHeaderView::sectionResized, this,
            [=](int logicalIndex, int oldSize, int newSize){
        config->sv.state = horizontalHeader()->saveState();
        config->sv.colWidths.replace(logicalIndex,newSize);
    });
    qApp->processEvents();
    horizontalHeader()->restoreState(config->sv.state);

    if(!config->sv.colWidths.isEmpty())
    {
        for(int i=0; i < config->sv.colWidths.count(); i++)
        {
            setColumnWidth(i, config->sv.colWidths.at(i));
        }
    }

}

StreetsTableModel* StreetView::model()
{
    return sourceModel;
}

void StreetView::tablev_CustomContextMenu(const QPoint &pt)
{
    QMenu tablMenu;
    curRow = rowAt(pt.y());
    curCol = columnAt(pt.x());
    int srcRow = proxyModel->mapToSource(proxyModel-> index(curRow, curCol)).row();
    QItemSelectionModel * selModel = selectionModel();
    QModelIndexList indexes = selModel->selectedIndexes();
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
        QList<SegmentInfo> segments = sourceModel->getSegmentsForStreet(names);
        foreach (SegmentInfo si , segments) {
            if(!sti->segments.contains(si.segmentId()))
                sti->segments.append(si.segmentId());
            sti->updateBounds(si);
        }
        sourceModel->updateStreetName(*sti);

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
        QModelIndex index = currentIndex();
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
              QString street, int streetid, int seq){
            if(streetId < 0)
                return;
            if(pinId == 0)
            {
                sti->startLatLng = latLng;
            }
            else
            {
                sti->endLatLng = latLng;
            }
            sti->bounds = Bounds();
            sti->updateBounds();
            sourceModel->updateStreetName(*sti);
        });

        connect(act, &QAction::triggered,this, [=]{

            WebViewBridge::instance()->processScript("clearPinMarker");
            QVariantList objArray;
            objArray << sti->startLatLng.lat() << sti->startLatLng.lon() << sti->endLatLng.lat()
                     << sti->endLatLng.lon() << sti->street << sti->streetId << sti->sequence;
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
    }
    tablMenu.exec(QCursor::pos());
}
