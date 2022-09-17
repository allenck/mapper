#ifndef STATIONVIEW_H
#define STATIONVIEW_H

#include <QObject>
#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QList>
#include "data.h"




class StationViewSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit StationViewSortProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data ( const QModelIndex & index, int role ) const;
private:
    //qint32 startRow, endRow;
private slots:
   //void getRows(int, int); // to get the row numbers that need to be highlighted
};

class StationViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StationViewTableModel(QObject *parent = 0);
    StationViewTableModel(QList<StationInfo> stationList, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList< StationInfo > getList();
    void addStation(StationInfo sti);
    void changeStation(StationInfo sti);
    void setStationList(QList<StationInfo> stationList);

signals:

public slots:
    void reset();

private:
    QList<StationInfo> listOfStations;

};

class StationView : public QObject
{
    Q_OBJECT
public:
    StationView(Configuration *cfg, QObject *parent = 0);
    void showStations();
    static StationView* instance();


signals:

public slots:
    void itemSelectionChanged(QModelIndex index );
    void changeStation(QString typeOfChg, StationInfo sti);
    StationViewTableModel* model() {return sourceModel;}

private:
    QObject *m_parent;
    SQL* sql;
    Configuration * config;
    QTableView* ui;
    StationViewTableModel *sourceModel;
    StationViewSortProxyModel *proxymodel;
    bool boolGetItemTableView(QTableView *table);
    QAction *copyAction;
    QAction *pasteAction;
    QAction *displayAction;
    QAction *editAction;
    int curRow;
    int curCol;
    QMenu menu;
    QModelIndex currentIndex;
    static StationView* _instance;

private slots:
    void Resize (int oldcount,int newcount);
    void tablev_customContextMenu( const QPoint& pt);
    void aCopy();
    void aPaste();
    void on_DisplayTriggered(bool);
    void on_editTriggered();
};

#endif // STATIONVIEW_H
