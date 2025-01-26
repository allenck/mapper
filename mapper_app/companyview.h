#ifndef COMPANYVIEW_H
#define COMPANYVIEW_H

#include <QObject>
#include <QAbstractTableModel>
#include "data.h"
#include "configuration.h"
#include <QTableView>
#include "mainwindow.h"


class MyCompanyTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MyCompanyTableModel(QObject *parent = 0);
    bool isDirty() {return bDirty;}
    void setDirty(bool b){bDirty = b;}
    void insertRecord(CompanyData*cd);
    void removeRecord(int row);
    void reset();


    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const {return companyList.count();}
    int columnCount(const QModelIndex &parent) const {return LASTUPDATED+1;}
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    CompanyData* getCompanyAtRow(int row);

 signals:
    void companySelectionsChanged();

 private:
    enum COLS
    {
        SELECT,
        KEY,
        MNEMONIC,
        ROUTEPREFIX,
        NAME,
        STARTDATE,
        ENDDATE,
        FIRSTROUTE,
        LASTROUTE,
        INFO,
        URL,
        LASTUPDATED
    };
     QMap<int, QString> hdrMap = {{SELECT,"Select"},{KEY, "Company Key"},{MNEMONIC, "Mnemonic"}, {NAME, "Company Name"},
                                     {ROUTEPREFIX, "Prefix"}, {INFO, "Information"},{STARTDATE, "Start"},
                                     {ENDDATE, "End"}, {URL,"Url"},
                                     {FIRSTROUTE, "First Route"}, {LASTROUTE, "Last Route"},
                                     {LASTUPDATED, "Last updated"}
                                    };
    Configuration * config;
    SQL* sql = nullptr;
    QList<CompanyData*> companyList;
    bool bDirty = false;
    friend class CompanyView;
};

class CompanyView : public QObject
{
    Q_OBJECT
public:
    CompanyView(QObject *parent = 0);
    QObject* m_parent;
    void clear();

    bool bUncomittedChanges();
    MyCompanyTableModel* model() {return _model;}
    static CompanyView* instance();

signals:
    //void dataChanged();

public slots:
    void newRecord();
    void delRecord();
    //void On_primeInsert(int, QSqlRecord&);
    void refresh();

private:
    MyCompanyTableModel *_model;
    Configuration * config;
    SQL* sql;
    QTableView* tableView;
    QMenu* menu;
    QAction* addAct;
    QAction* delAct;
    QAction* refreshAct;
    QAction* urlAct;
    int curRow, curCol;
    bool boolGetItemTableView(QTableView *table);
    QModelIndex currentIndex;
    bool bNeedsRefresh;
    static CompanyView* _instance;
    QSortFilterProxyModel* proxyModel;

private slots:
    void Resize (int oldcount,int newcount);
    void tablev_customContextMenu( const QPoint& pt);

};

#endif // COMPANYVIEW_H
