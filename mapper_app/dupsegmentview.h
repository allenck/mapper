#ifndef DUPSEGMENTVIEW_H
#define DUPSEGMENTVIEW_H

#include <QObject>
#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QList>
#include "data.h"




class dupSegmentViewSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit dupSegmentViewSortProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data ( const QModelIndex & index, int role ) const;
private:
    //qint32 startRow, endRow;
private slots:
   //void getRows(int, int); // to get the row numbers that need to be highlighted
};

class dupSegmentViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit dupSegmentViewTableModel(QObject *parent = 0);
    dupSegmentViewTableModel(QList<QPair<SegmentInfo, SegmentInfo>> dupSegmentList, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList<QPair<SegmentInfo, SegmentInfo>> getList();
    void reset();
    void deleteRow(int row);

signals:

public slots:
private:
    QList<QPair<SegmentInfo, SegmentInfo>> listOfSegments;

private slots:

};

class DupSegmentView : public QObject
{
    Q_OBJECT
public:
    DupSegmentView(QObject *parent = 0);
    void showDupSegments(QList<QPair<SegmentInfo, SegmentInfo> > dupSegmentList);


signals:
 void  selectSegment(int segmentId);

public slots:
    void tablev_customContextMenu( const QPoint& pt);

private:
    QObject *m_parent;
    Configuration *config;
    SQL* sql;
    QTableView* ui;
    dupSegmentViewTableModel *sourceModel;
    dupSegmentViewSortProxyModel *proxymodel;
    qint32 segmentId, dupSegmentId;
    SegmentInfo si1;
    SegmentInfo si2;
    int curRow, curCol;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;
    QMenu menu;
    QAction* selectSegmentsAct;
    QAction* deleteDuplicateAct;
    SegmentData currSd;
    qint32 modelRow = -1;
private slots:
    //void Resize (int oldcount,int newcount);
    void On_selectSegmentAct(bool);
};
#endif // DUPSEGMENTVIEW_H
