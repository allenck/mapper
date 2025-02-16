#ifndef ROUTEVIEWTABLEMODEL_H
#define ROUTEVIEWTABLEMODEL_H

#include <QAbstractTableModel>
//#include <QPair>
#include <QList>
#include "data.h"
#include "sql.h"

class RouteViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RouteViewTableModel(QObject *parent);
    RouteViewTableModel(qint32 route, QString name, int companyKey, QDate dtStart, QDate dtEnd, QList<SegmentData*> segmentDataList, QObject *parent);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList<SegmentData *> getList();
    void setList(QList<SegmentData *> segmentDataList);
    void reset();
    void setSequenced(bool b);

    //QMap<int, RowChanged*> changedMap;
    void deleteRow(qint32 segmentId, const QModelIndex &index);
    //void unDeleteRow(qint32 segmentId, const QModelIndex &index);
    //bool isSegmentMarkedForDelete(qint32 segmentId);
    enum COLUMNS
    {
     SEGMENTID,
     NAME,
     TRACKS,
     TYPE,
     ONEWAY,
     USAGE,
     COMBO,
     TRACTIONTYPE,
     DISTANCE,
     COMPANYKEY,
     STARTDATE,
     DOUBLEDATE,
     ENDDATE,
     ANGLES,
     ANGLEE,
     NE,
     NL,
     NEXT,
     PREV,
     NEXTR,
     PREVR,
     RE,
     RL,
     DIR,
     SEQ,
     RSEQ,
        STREET,
        NEWERSTREET
    };
    int getRow(int segmentId);
    SegmentData *segmentData(int row);
    void clear();

signals:
    void rowChange(qint32 row, qint32 segmentid, bool bDeleted, bool bChanged);
    //void refreshRoutes();

public slots:
    void getRows(int, int); // to get the row numbers that need to be highlighted
    //bool commitChanges();
    void segmentChanged(SegmentInfo si, SQL::CHANGETYPE);
    void routeChange(NotifyRouteChange rc);

private:
     QList<SegmentData*> listOfSegments;
     bool bIsSequenced;
     qint32 selectedRow;
     bool bSelectedRowChanged;
     QDate dtStart;
     QDate dtEnd;
     QDate startDate, endDate;
     QList<SegmentData> saveSegmentDataList;
     qint32 route;
     QString name;
     int companyKey;
     qint32 startRow, endRow;
     QMap<int, TractionTypeInfo> tractionTypes;
     QMap<int, QString> turnMap = {{0, "back"}, {1,"Left"},{2,"Right"}};
     QMap<int, QString> turnMap2 = {{0, "ahead"}, {1,"Left"},{2,"Right"}};
     bool bChangesMade = false; // set when changes need to be comitted.
     QList<SegmentData*> _selectedSegments;
     bool bSegmentNeedsUpdate = false;

     friend class RouteView;
};

#endif // ROUTEVIEWTABLEMODEL_H
