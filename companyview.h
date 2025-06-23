#ifndef COMPANYVIEW_H
#define COMPANYVIEW_H

#include <QObject>
#include <QSqlTableModel>
#include "data.h"
#include "configuration.h"
#include "mainwindow.h"
#include "sql.h"

class MyCompanyTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
  MyCompanyTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()) : QSqlTableModel(parent){}

protected:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
 private:
    QMap<QString, QString> hdrMap = {{"key", "Company Key"},{"Description", "Company Name"},
                                     {"routePrefix", "Prefix"}, {"startDate", "Start"}, {"endDate", "End"},
                                     {"firstRoute", "First Route"}, {"lastRoute", "Last Route"},
                                     {"lastUpdate", "Last updated"}, {"info", "Information"}
                                    };
};
class CompanyView : public QObject
{
    Q_OBJECT
public:
    CompanyView(Configuration *cfg, QObject *parent = 0);
    QObject* m_parent;
    void clear();
    enum COLUMNS
    {
     KEY,
     NAME,
     ROUTEPREFIX,
     STARTDATE,
     ENDDATE,
     FIRSTROUTE,
     LASTROUTE,
     LASTUPDATED,
     INFO
    };
    bool bUncomittedChanges();


signals:
    void dataChanged();

public slots:
    void newRecord();
    void delRecord();
    void On_primeInsert(int, QSqlRecord&);
    void refresh();

private:
    QSqlTableModel *model;
    Configuration * config;
    SQL* sql;
    QTableView* tableView;
    QMenu* menu;
    QAction* addAct;
    QAction* delAct;
    QAction* refreshAct;
    int curRow, curCol;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;
    bool bNeedsRefresh;

private slots:
    void Resize (int oldcount,int newcount);
    void tablev_customContextMenu( const QPoint& pt);

};

#endif // COMPANYVIEW_H
