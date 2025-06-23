#include "segmentviewsortproxymodel.h"
#include <QColor>

segmentViewSortProxyModel::segmentViewSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}
bool segmentViewSortProxyModel::lessThan(const QModelIndex &left,
const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if(left.data().type() == QVariant::Int)
    {
        return (left.data().toInt() < right.data().toInt());
    }
    else if(leftData.type() == QVariant::String)
    {
        return (leftData.toString() < rightData.toString());
    }
    return false;
}
QVariant segmentViewSortProxyModel::data ( const QModelIndex & index, int role ) const
{
    QModelIndex sourceIndex;
    if (!index.isValid())
        return QVariant();

    // We only wish to override the background role
    if (role == Qt::BackgroundRole )
    {
        sourceIndex = mapToSource(index);
        qint32 row = sourceIndex.row();
        if ( row == startRow)
        {
            return QVariant( QColor(Qt::green) );
        }

        if ( row == endRow)
        {
            return QVariant( QColor(Qt::red) );
        }
    }
    // let the base class handle all other cases
    return QSortFilterProxyModel::data( index, role );
}
void segmentViewSortProxyModel::getRows(qint32 start, qint32 end)
{
    startRow = start;
    endRow = end;
}
