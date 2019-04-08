#ifndef ROUTEVIEWSORTPROXYMODEL_H
#define ROUTEVIEWSORTPROXYMODEL_H

#include <QSortFilterProxyModel>

class RouteViewSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit RouteViewSortProxyModel(QObject *parent = 0);
    //void getRows(qint32 start, qint32 end);
signals:


public slots:

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    //QVariant data ( const QModelIndex & index, int role ) const;
private:

private slots:
};

class routeViewFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit routeViewFilterProxyModel(QObject *parent = 0);
    //void setTerminals(qint32 start, qint32 end);
protected:
    //QVariant data ( const QModelIndex & index, int role ) const;

private:

};

#endif // ROUTEVIEWSORTPROXYMODEL_H
