#ifndef SEGMENTVIEWTABLEMODEL_H
#define SEGMENTVIEWTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "data.h"


class SegmentViewTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SegmentViewTableModel(QObject *parent = 0);
    SegmentViewTableModel(QList<SegmentData> segmentDataList, double Lat, double Lon, qint32 route, QString date, QObject *parent=0);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QList<SegmentData> getList();
    void reset();
    enum COLUMNS {SEGMENTID, DESCRIPTION, TRACKS, STREETNAME, DIRECTION, LAT, LON};
    int getRow(int segmentId);
    SegmentData selectedSegment(int row);

signals:

public slots:
private:
    QList<SegmentData> listOfSegments;
    double lat, lon;
    double angleDiff(double A1, double A2);
    qint32 m_routeNbr;
    QString m_date;
};

#endif // SEGMENTVIEWTABLEMODEL_H
