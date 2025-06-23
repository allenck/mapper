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
    QString startDate = "1800/01/01";
    QString endDate = "2050/12/31";
    QString alphaRoute;
    QStringList headers;
    QTableView* ui;
    TerminalInfo ti;
    //QList<SegmentInfo> segmentInfoList;  // list of segmentInfo items in cbSegments
    QList<SegmentData> segmentDataList;  // list of segmentInfo items in cbSegments

    QList<SegmentData> saveSegmentDataList;  // list of segmentInfo items in cbSegments
    void populateList();
    QAction *copyAction;
    QAction *pasteAction;
    QAction *reSequenceFromStartAct;
    QAction *reSequenceFromEndAct;
    QAction *startTerminalStartAct;
    QAction *startTerminalEndAct;
    QAction *endTerminalStartAct;
    QAction *endTerminalEndAct;
    QAction *deleteSegmentAct;
    QAction *unDeleteSegmentAct;
    QAction *saveChangesAct;
    QAction *discardChangesAct;
    QAction* selectSegmentAct;
    QAction *editSegmentAct;
    QAction* showColumnsAct;
    QAction* convertToSingleTrackAct;
    QAction* updateRouteAct;
    QAction* splitSegmentAct;

    QMenu *startTerminal;
    QMenu *endTerminal;
    //SQL sql;
    Configuration *config;
    bool bIsSequenced;

    bool boolGetItemTableView(QTableView *table);
    RouteViewTableModel *sourceModel;
    RouteViewSortProxyModel *proxymodel;
    QModelIndex currentIndex;
    CheckRoute *chk;
    RouteData rd;
    MainWindow* myParent;
    void reSequenceRoute(QString whichEnd);

private slots:
    void tablev_customContextMenu( const QPoint& );
    void aCopy();
    void aPaste();
    void itemSelectionChanged(QModelIndex index );
    void reSequenceRouteFromStart();
    void reSequenceRouteFromEnd();
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
