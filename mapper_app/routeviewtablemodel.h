#ifndef ROUTEVIEWTABLEMODEL_H
#define ROUTEVIEWTABLEMODEL_H

#include <QAbstractTableModel>
//#include <QPair>
#include <QList>
#include "data.h"

class RouteViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RouteViewTableModel(QObject *parent = 0);
    RouteViewTableModel(qint32 route, QString name, QDate dtStart, QDate dtEnd, QList<SegmentInfo> segmentInfoList, QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList< SegmentInfo > getList();
    void reset();
    void setSequenced(bool b);

    QList<RowChanged*> changedRows;
    void deleteRow(qint32 segmentId, const QModelIndex &index);
    void unDeleteRow(qint32 segmentId, const QModelIndex &index);
    bool isSegmentMarkedForDelete(qint32 segmentId);
    enum COLUMNS
    {
     SEGMENTID,
     NAME,
     TRACKS,
     ONEWAY,
     USAGE,
     NEXT,
     PREV,
     DIR,
     SEQ,
     RSEQ,
     STARTDATE,
     ENDDATE,
     DISTANCE
    };
    int getRow(int segmentId);
    void commitChanges();

signals:
    void rowChange(qint32 row, qint32 segmentid, bool bDeleted, bool bChanged);
    void refreshRoutes();

public slots:
    void getRows(int, int); // to get the row numbers that need to be highlighted

private:
     QList<SegmentInfo> listOfSegments;
     bool bIsSequenced;
     qint32 selectedRow;
     bool bSelectedRowChanged;
     QDate dtStart;
     QDate dtEnd;
     QString startDate, endDate;
     QList<SegmentInfo> saveSegmentInfoList;
     qint32 route;
     QString name;
     qint32 startRow, endRow;

     friend class RouteView;
};

#endif // ROUTEVIEWTABLEMODEL_H
