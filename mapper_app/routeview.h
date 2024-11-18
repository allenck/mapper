#ifndef ROUTEVIEW_H
#define ROUTEVIEW_H

#include <QObject>
#include <QtGui>
#include <QTableWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
//#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include "routeviewtablemodel.h"
#include "routeviewsortproxymodel.h"
#include "configuration.h"
#include "webviewbridge.h"
#include "checkroute.h"

class RouteView : public QObject
{
    Q_OBJECT
public:
    RouteView(QObject *parent = 0);
    QObject* m_parent;
    QMenu menu;

    void updateRouteView();
    void updateTerminals();
    bool isSequenced();
    bool bUncomittedChanges();
//    void checkChanges();
    RouteViewTableModel* model();
    void setList(QList<SegmentData *> segmentDataList);
    void clear();
    QList<SegmentData *> selectedSegments();

signals:
    void sendRows(int, int);
    void selectSegment(int);
    void refreshRoutes();

public slots:
//      void itemChanged(QTableWidgetItem* item);
      //void itemSelectionChanged(QTableWidgetItem * item );
//    void commitChanges();
    void editSegment();
    void on_segmentSelected(int, int, QList<LatLng>);

private:
    qint32 curRow, curCol;
    qint32 route=-1;
    qint32 startSegment, endSegment;
    int companyKey;
    QString name;
    QDate startDate = QDate::fromString("1800/01/01", "yyyy/MM/dd");
    QDate endDate = QDate::fromString("2050/12/31", "yyyy/MM/dd");
    QString alphaRoute;
    QStringList headers;
    QTableView* ui;
    TerminalInfo ti;
    void populateList();
    // QAction *copyAction;
    // QAction *pasteAction;
    QAction *reSequenceFromStartAct;
    QAction *reSequenceFromEndAct;
    QAction *startTerminalStartAct;
    QAction *startTerminalEndAct;
    QAction *endTerminalStartAct;
    QAction *endTerminalEndAct;
    QAction *removeSegmentAct;
    QAction* selectSegmentAct;
    QAction *editSegmentAct;
    //QAction* showColumnsAct;
    QAction* convertToSingleTrackAct;
    QAction* updateRouteAct;
    QAction* splitSegmentAct;
    QAction* sortNameAct;
    QAction* hideColumnAct;
    QAction* showColumnAct;
    QAction* addToAnotherRouteAct;
    QAction* deleteSelectedRowsAct;
    QAbstractButton* cornerButtonAct;

    QMenu *startTerminal;
    QMenu *endTerminal;
    //SQL sql;
    Configuration *config;
    bool bIsSequenced;

    bool boolGetItemTableView(QTableView *table);
    RouteViewTableModel *sourceModel = nullptr;
    RouteViewSortProxyModel *proxymodel = nullptr;
    QModelIndex currentIndex;
    CheckRoute *chk;
    RouteData rd;
    MainWindow* myParent;
    void reSequenceRoute(QString whichEnd);

private slots:
    void hdr_customContextMenu(const QPoint pt );
    void tablev_customContextMenu( const QPoint& );
    void tab1CustomContextMenu(const QPoint &);
    // void aCopy();
    // void aPaste();
    void itemSelectionChanged(QModelIndex index );
    void reSequenceRouteFromStart();
    void reSequenceRouteFromEnd();
    void StartRoute_S();
    void StartRoute_E();
    void EndRoute_S();
    void EndRoute_E();
    bool dataChanged(QModelIndex,QModelIndex);
    void Resize (int oldcount,int newcount);
    void removeSegment();
    //void unDeleteSegment();
    void on_selectSegment_triggered();

    friend class RouteViewTableModel;
};

#endif // ROUTEVIEW_H
