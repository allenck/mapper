#ifndef ROUTEVIEW_H
#define ROUTEVIEW_H

#include <QObject>
#include <QtGui>
#include <QTableWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
#include "mainwindow.h"
#include <QMenu>
#include <QClipboard>
#include "routeviewtablemodel.h"
#include "routeviewsortproxymodel.h"
#include "sql.h"
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
    void checkChanges();
    RouteViewTableModel* model();

signals:
    void sendRows(int, int);
    void selectSegment(int);
    void refreshRoutes();

public slots:
//      void itemChanged(QTableWidgetItem* item);
      //void itemSelectionChanged(QTableWidgetItem * item );
    void commitChanges();
    void editSegment();
    void on_segmentSelected(int,int);

private:
    qint32 curRow, curCol;
    qint32 route;
    qint32 startSegment, endSegment;
    QString name;
    QString startDate;
    QString endDate;
    QString alphaRoute;
    QStringList headers;
    QTableView* ui;
    TerminalInfo ti;
    QList<SegmentInfo> segmentInfoList;  // list of segmentInfo items in cbSegments
    QList<SegmentInfo> saveSegmentInfoList;  // list of segmentInfo items in cbSegments
    void populateList();
    QAction *copyAction;
    QAction *pasteAction;
    QAction *reSequenceAction;
    QAction *startTerminalStartAct;
    QAction *startTerminalEndAct;
    QAction *endTerminalStartAct;
    QAction *endTerminalEndAct;
    QAction *deleteSegmentAct;
    QAction *unDeleteSegmentAct;
    QAction *saveChangesAct;
    QAction* selectSegmentAct;
    QAction *editSegmentAct;

    QMenu *startTerminal;
    QMenu *endTerminal;
    //SQL sql;
    Configuration *config;
    bool bIsSequenced;

    bool boolGetItemTableView(QTableView *table);
    RouteViewTableModel *sourceModel;
    RouteViewSortProxyModel *proxymodel;
    QModelIndex currentIndex;
    checkRoute *chk;

private slots:
    void tablev_customContextMenu( const QPoint& );
    void aCopy();
    void aPaste();
    void itemSelectionChanged(QModelIndex index );
    void reSequenceRoute();
    void StartRoute_S();
    void StartRoute_E();
    void EndRoute_S();
    void EndRoute_E();
    bool dataChanged(QModelIndex,QModelIndex);
    void Resize (int oldcount,int newcount);
    void deleteSegment();
    void unDeleteSegment();
    void on_selectSegment_triggered();

};

#endif // ROUTEVIEW_H
