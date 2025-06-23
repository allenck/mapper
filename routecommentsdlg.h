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
    explicit RouteCommentsDlg(Configuration *cfg, QWidget *parent = 0);
    ~RouteCommentsDlg();
    void setRoute(qint32);
    void setDate(QDate);
    void setCompanyKey(qint32);
    
private:
    Ui::RouteCommentsDlg *ui;
    qint32 _route;
    qint32 _companyKey;
    QDate _date;
    SQL* sql;
    RouteComments _rc;
    Configuration *config;

    bool bIsDirty;
    bool bDateChanged;
    bool bRouteChanged;
    bool bTagsChanged;
    void outputChanges();
    bool readComment(int pos);
    void setDirty(bool = true);


private slots:
    void btnOK_Clicked();
    void btnCancel_Clicked();
    void btnDelete_Clicked();

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
};

#endif // ROUTECOMMENTSDLG_H
