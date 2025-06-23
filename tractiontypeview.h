#ifndef TRACTIONTYPEVIEW_H
#define TRACTIONTYPEVIEW_H

#include <QObject>
#include <QSqlTableModel>
#include "data.h"
#include "configuration.h"
#include "mainwindow.h"
#include "sql.h"

class MyTractionTypesTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    MyTractionTypesTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

protected:
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
};

class TractionTypeView : public QObject
{
    Q_OBJECT
public:
    TractionTypeView(Configuration * cfg, QObject *parent = 0);
    QObject* m_parent;
    void clear();

signals:

public slots:
    void newRecord();
    void delRecord();

private:
    QSqlTableModel *model;
    Configuration * config;
    SQL* sql;
    QTableView* tableView;
    QMenu* menu;
    QAction* addAct;
    QAction* delAct;
    int curRow, curCol;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;

private slots:
    void Resize (int oldcount,int newcount);
    void tablev_customContextMenu( const QPoint& pt);


};

#endif // TRACTIONTYPEVIEW_H
