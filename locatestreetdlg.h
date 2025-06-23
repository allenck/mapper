#ifndef LOCATESTREETDLG_H
#define LOCATESTREETDLG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QList>
#include "geodbsql.h"

class GeodbViewSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit GeodbViewSortProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data ( const QModelIndex & index, int role ) const;
private:
    //qint32 startRow, endRow;
private slots:
   //void getRows(int, int); // to get the row numbers that need to be highlighted
};

class GeodbViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit GeodbViewTableModel(QObject *parent = 0);
    GeodbViewTableModel(QList<geodbObject> objectList, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList< geodbObject> getList();

    void reset();

signals:

public slots:

private:
    QList<geodbObject> listOfObjects;

private slots:


};

namespace Ui {
    class LocateStreetDlg;
}

class LocateStreetDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LocateStreetDlg(QWidget *parent = 0);
    ~LocateStreetDlg();

private:
    Ui::LocateStreetDlg *ui;
    QObject *m_parent;
    GeodbViewTableModel *sourceModel;
    GeodbViewSortProxyModel *proxymodel;

private slots:
    void txtStreet_Leave();
    void Resize(int oldcount,int newcount);
    void itemSelectionChanged(QModelIndex index );

};

#endif // LOCATESTREETDLG_H
