#include "streetview.h"
#include "dateeditdelegate.h"
#include "qclipboard.h"
#include "qheaderview.h"
#include "qmenu.h"
#include "sql.h"
#include "streetstablemodel.h"

StreetView::StreetView() {
    config = Configuration::instance();
    sql = SQL::instance();
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::SingleSelection );
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(true);
    resizeColumnsToContents();
    setItemDelegateForColumn(StreetsTableModel::STARTDATE, new DateEditDelegate());
    setItemDelegateForColumn(StreetsTableModel::ENDDATE, new DateEditDelegate());
    sourceModel = new StreetsTableModel();
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(sourceModel);
    setModel(proxyModel);

    //connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tablev_CustomContextMenu(QPoint)));
    connect(this, &QTableView::customContextMenuRequested, this,[=](const QPoint& pt){
        tablev_CustomContextMenu(pt);
    });
    // connect(sourceModel, &StreetsTableModel::streetUpdated, this, [=](int row, QString street){
    //     sourceModel->streetChanged(street);
    // });
    connect(sourceModel, &StreetsTableModel::streetInfoChanged, this,[=](StreetInfo sti){
        sourceModel->streetChanged(sti);
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
    QAction* act = new QAction(tr("update %1 bounds").arg(street),this);
    //StreetInfo* sti = sourceModel->getStreet(street);
    StreetInfo* sti = sourceModel->getStreetName(street, location);
    if(!sti)
        StreetInfo* sti = sourceModel->getStreetDef(streetId);
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
        // if(!sti->newerName.isEmpty())
        // {
        //     StreetInfo* sti2 = sourceModel->getStreet(sti->newerName);
        // }
        // sourceModel->updateStreet(*sti);

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
    tablMenu.exec(QCursor::pos());
}