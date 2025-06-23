#ifndef SEGMENTVIEWSORTPROXYMODEL_H
#define SEGMENTVIEWSORTPROXYMODEL_H

#include <QSortFilterProxyModel>

class segmentViewSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit segmentViewSortProxyModel(QObject *parent = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data ( const QModelIndex & index, int role ) const;
private:
    qint32 startRow, endRow;
private slots:
   void getRows(int, int); // to get the row numbers that need to be highlighted
};

#endif // SEGMENTVIEWSORTPROXYMODEL_H
