#ifndef SEGMENTVIEW_H
#define SEGMENTVIEW_H

#include <QObject>
#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include "segmentviewtablemodel.h"
#include "segmentviewsortproxymodel.h"

class SegmentView : public QObject
{
    Q_OBJECT
public:
    SegmentView(Configuration *cfg, QObject *parent = 0);
    QObject* m_parent;
    QMenu menu;

    void showSegmentsAtPoint(double lat, double lon, qint32 SegmentId);
    SegmentViewTableModel *sourceModel;
    segmentViewSortProxyModel *proxymodel;
    QModelIndex currentIndex;
signals:
    void sendRows(int, int);
public slots:
    void editSegment();
private:
    QAction *copyAction;
    QAction *pasteAction;
    QAction *addToRouteAct;
    QAction *editSegmentAct;
    qint32 curRow, curCol;
    QStringList headers;
    QTableView* ui;
    QList<SegmentData> myArray;
    Configuration *config;
    SQL* sql;

    bool boolGetItemTableView(QTableView *table);

private slots:
    void tablev_customContextMenu( const QPoint& pt);

    void aCopy();
    void aPaste();
    void addToRoute();
    void itemSelectionChanged(QModelIndex index );
    void Resize (int oldcount,int newcount);

};

#endif // SEGMENTVIEW_H
