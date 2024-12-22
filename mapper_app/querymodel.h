#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QSqlQueryModel>
#include <QHeaderView>
#include <QTableView>
#include <QWidget>


class QueryModel : public QSqlQueryModel
{
 Q_OBJECT
public:
    explicit QueryModel(QObject *parent = 0, QSqlDatabase db=QSqlDatabase(), QString dbtype="Sqlite3");
 void sort( int column, Qt::SortOrder order);
 QVariant data(const QModelIndex &index, int role) const;
 QVariant edit(const QModelIndex &index) const;
 Qt::ItemFlags flags(const QModelIndex &index) const;
 QString driver;
 QString dbType;
 void setTabName(QString name);
 QString tabName;


private:

signals:
    
public slots:
    
};
class MyHeaderView : public QHeaderView
{
 Q_OBJECT
public:
 MyHeaderView(Qt::Orientation orientation, QWidget * parent = 0);
 virtual ~MyHeaderView();
 void setMoveAllowed(bool b);
 bool bAllowSortColumns;

private:
 void mousePressEvent(QMouseEvent *e);
 void mouseReleaseEvent(QMouseEvent *e);
 void mouseMoveEvent(QMouseEvent *e);

 int fromCol;
 int toCol;
 QWidget *myParent;
 QTableView *view;
 int queryViewCurrColumn;


private slots:
 void contextMenuRequested(const QPoint &pt);
 void on_queryView_hide_column();
 void on_queryView_show_columns();
 void onMoveOrRezize_columns();
 void onResizeToData();
 void on_sortAction();

};

#endif // QUERYMODEL_H
