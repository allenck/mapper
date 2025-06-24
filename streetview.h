#ifndef STREETVIEW_H
#define STREETVIEW_H

#include <QTableView>
#include "data.h"
#include "mainwindow.h"
#include "sql.h"

class SQL;
class Configuration;
class StreetsTableModel;
class QSortFilterProxyModel;
class StreetView : public QObject
{
    Q_OBJECT
public:
    StreetView(QObject* parent = 0);
    StreetsTableModel* model();

public slots:
    void tablev_CustomContextMenu(const QPoint &pt);


private:
    StreetsTableModel* sourceModel = nullptr;
    QSortFilterProxyModel* proxyModel = nullptr;
    int curRow, curCol;
    Configuration* config = nullptr;
    SQL* sql = nullptr;
    void segmentChanged(SegmentInfo si, SQL::CHANGETYPE t);
    QObject* m_parent;
    MainWindow* myParent;
    QTableView* ui;
    StreetInfo sti0;

    friend class StreetsTableModel;
};

#endif // STREETVIEW_H
