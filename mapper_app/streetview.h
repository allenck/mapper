#ifndef STREETVIEW_H
#define STREETVIEW_H

#include <QTableView>

class SQL;
class Configuration;
class StreetsTableModel;
class QSortFilterProxyModel;
class StreetView : public QTableView
{
public:
    StreetView();
    StreetsTableModel* model();

public slots:
    void tablev_CustomContextMenu(const QPoint &pt);


private:
    StreetsTableModel* sourceModel = nullptr;
    QSortFilterProxyModel* proxyModel = nullptr;
    int curRow, curCol;
    Configuration* config = nullptr;
    SQL* sql = nullptr;
};

#endif // STREETVIEW_H