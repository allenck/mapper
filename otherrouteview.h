#ifndef OTHERROUTEVIEW_H
#define OTHERROUTEVIEW_H

#include <QObject>
#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QList>
#include "data.h"

class otherRouteViewSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit otherRouteViewSortProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data ( const QModelIndex & index, int role ) const;
private:
    //qint32 startRow, endRow;
private slots:
   //void getRows(int, int); // to get the row numbers that need to be highlighted
};

class OtherRouteViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit OtherRouteViewTableModel(QObject *parent = 0);
    OtherRouteViewTableModel(QList<SegmentData*> routeDataList, SegmentData* sd, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList<SegmentData *> getList();
    void reset();
signals:

public slots:
private:
    QList<SegmentData*> listOfRoutes;
    SegmentData* sd;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;

private slots:

};

class OtherRouteView : public QObject
{
    Q_OBJECT
public:
 static OtherRouteView* instance();
    void showRoutesUsingSegment(qint32 segmentId);


signals:
 void displayRoute(RouteData rd);

public slots:
 void On_displayRouteAct_triggered(bool);
 void Resize (int oldcount,int newcount);
 void tablev_customContextMenu( const QPoint& pt);
private:
    OtherRouteView( QObject *parent = 0);
    QObject *m_parent;
    Configuration *config;
    SQL* sql;
    QTableView* ui;
    OtherRouteViewTableModel *sourceModel;
    otherRouteViewSortProxyModel *proxymodel;
    static OtherRouteView* _instance;
    QMenu menu;
    QAction* displayRouteAct;
    SegmentData* sd;
    int curRow, curCol;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;

};
#endif // OTHERROUTEVIEW_H
