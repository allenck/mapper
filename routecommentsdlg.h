#ifndef ROUTECOMMENTSDLG_H
#define ROUTECOMMENTSDLG_H

#include <QDialog>
#include <QDate>
#include "data.h"
#include "sql.h"
#include <QMenu>

namespace Ui {
class RouteCommentsDlg;
}

class RouteCommentsDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit RouteCommentsDlg(QList<RouteData> *routeList, int companyKey, QWidget *parent = 0);
    ~RouteCommentsDlg();
    //void setRoute(qint32);
    //void setDate(QDate);
    //void setCompanyKey(qint32);
    void scan();
    void setDirty(bool = true);
    void displayComment(CommentInfo newCi);
    static bool upgrade();
    void setRouteData(RouteData rd);
    void onRouteSelected(int route, QString alphaRoute, int row);
    QMap<QString, RouteData *> *createList(QList<RouteData>* rdList, QDate dt);

private:
    Ui::RouteCommentsDlg *ui;
    // qint32 _route;
    // qint32 _companyKey;
    // QDate _date;
    SQL* sql;
    //RouteComments _rc;
    RouteData _rd;
    CommentInfo _ci;
    RouteComments _rc;
    Configuration *config;
    QList<RouteData>* routeList = nullptr;
    QSortFilterProxyModel* proxyModel = nullptr;
    RouteSelectorTableModel* _sourceModel = nullptr;
    QList<int>* routes = nullptr;
    QStringList* aRoutes = nullptr;
    QList<int>* dRoutes = nullptr;
    QModelIndexList modelIndexList;
    QItemSelectionModel* selectionModel=nullptr;
    QList<CommentInfo>* orphans = nullptr;
    QList<CommentInfo>* comments = nullptr;
    int currIx = -1;
    QMap<QString, RouteData*>* aList = nullptr;

    int ixOrphan=-1;
    // int commentsUpdated = 0;
    int commentsDeleted = 0;
    // int routeCommentsDeleted = 0;
    // int routeCommentsAdded =0;
    int htmlCorrected = 0;
    // int invalidDates = 0;
    // int invalidRoutes = 0 ;
    // int routesDeleted = 0;
    int linksFixed =0;
    // int invalidRouteComments = 0;
    int orphansDeleted = 0;
    int orphansUsed = 0;
    int dup_emptyOrphans =0;

    bool bIsDirty = false;
    bool bDateChanged = false;
    bool bRouteChanged = false;
    bool bTagsChanged = false;
    QString scanLog;
    bool outputChanges();
    QT_DEPRECATED bool readRouteComment(int pos);
    bool bScanInProgress = false;
    bool processOrphan();
    bool finishScan(int rslt);
    bool scanResult = false;
    void enableButtons();
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    QAction* hideColumnAct;
    QAction* showColumnAct;

private slots:
    void btnOK_Clicked();
    void btnCancel_Clicked();
    void btnDelete_Clicked();
    void btnIgnore_clicked();
    void OnBtnNext();
    void OnBtnPrev();
    void OnDateChanged();
    void OnTextChanged();
    void OnTagsLeave();
    void OnDateLeave();
//    void OnRouteTextChanged(QString text);
//    void OnAlphaRouteTextChanged(QString text);
//    void OnRouteLeave();
//    void OnAlphaRouteLeave();
    void OnBtnApply_clicked();
    void OnDirtySet(bool);
    void OnAdditionalRoutesLeave();
    void onChgDate();
    void onSelectionsChanged(QModelIndexList added, QModelIndexList deleted);
    void onCommentChange(CommentInfo ci, SQL::CHANGETYPE t);
    void hdr_customContextMenu( const QPoint pt);
    void tablev_customContextMenu(const QPoint& pt);

protected:
    bool bSettingSelections = false;
};

#endif // ROUTECOMMENTSDLG_H
