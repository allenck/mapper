#ifndef SEGMENTVIEWTABLEMODEL_H
#define SEGMENTVIEWTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "data.h"

class SegmentDescription;
class SegmentViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SegmentViewTableModel(QObject *parent = 0);
    SegmentViewTableModel(QList<SegmentInfo> segmentDataList, double Lat, double Lon, qint32 route, QString date, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList<SegmentInfo> *getList();
    void reset();
    enum COLUMNS {SEGMENTID, DESCRIPTION, TRACKS, STREETNAME, LOCATION, STREETID, DIRECTION, LATLNG, WHICHEND};
    int getRow(int segmentId);
    SegmentInfo selectedSegment(int row);
    void setList(QList<SegmentInfo> list);
    void checkList(QList<SegmentInfo> segmentDataList);
    void updateRow(int row, SegmentInfo si);

signals:

public slots:
private:
    QList<SegmentInfo> listOfSegments;
    double lat, lon;
    double angleDiff(double A1, double A2);
    qint32 m_routeNbr=-1;
    QString m_date;
    SegmentDescription* sd = nullptr;
    void common();
};

#endif // SEGMENTVIEWTABLEMODEL_H
