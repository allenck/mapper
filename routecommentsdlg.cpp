#include "routecommentsdlg.h"
#include "ui_routecommentsdlg.h"
#include <QCompleter>
#include <QFontDialog>
#include <QClipboard>
#include <QTextDocumentFragment>
#include "htmltextedit.h"

RouteCommentsDlg::RouteCommentsDlg(Configuration *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RouteCommentsDlg)
{
    ui->setupUi(this);
    _route = -1;
    _companyKey = -1;
    //_date.setYMD(1800,1,1);
    _date = QDate(1800,1,1);
    config = cfg;
    //sql->setConfig(config);
    sql = SQL::instance();
    ui->txtComments->setReadOnly(false);
    connect(ui->txtTags, SIGNAL(editingFinished()), this, SLOT(OnTagsLeave()));

    connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOK_Clicked()));
    connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(OnBtnApply_clicked()));
    connect(ui->btnCancel, SIGNAL(clicked()),this, SLOT(btnCancel_Clicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_Clicked()));
    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(OnDateChanged()));
    connect(ui->dateEdit, SIGNAL(editingFinished()), this, SLOT(OnDateLeave()));
    connect(ui->txtComments, SIGNAL(dirtySet(bool)), this, SLOT(OnDirtySet(bool)));
//    connect(ui->txtAdditionalRoutes, SIGNAL(editingFinished()), this, SLOT(OnAdditionalRoutesLeave()));

    setWindowTitle(tr("Route Comments"));


    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(OnBtnNext()));
    connect(ui->btnPrev, SIGNAL(clicked()), this, SLOT(OnBtnPrev()));
//    connect(ui->txtRoute, SIGNAL(textChanged(QString)), this, SLOT(OnRouteTextChanged(QString)));
//    connect(ui->txtRoute, SIGNAL(editingFinished()), this, SLOT(OnRouteLeave()));
//    connect(ui->txtRouteAlpha,SIGNAL(textChanged(QString)), this, SLOT(OnAlphaRouteTextChanged(QString)));
//    connect(ui->txtRouteAlpha, SIGNAL(editingFinished()), this, SLOT(OnAlphaRouteLeave()));
    connect(ui->tableView, &RouteSelector::selections_changed, [=](QModelIndexList /*added*/, QModelIndexList deleted){
     for(QModelIndex deletedIndex : deleted)
     {
      if(deletedIndex.isValid())
      {
       int deletedRow = deletedIndex.row();
       RouteComments rc;
       QList<RouteName*> routeNameList =  ui->tableView->getList();
       rc.route = routeNameList.at(deletedRow)->route();
       rc.date = ui->dateEdit->date();
       sql->deleteRouteComment(rc);
      }
     }
    });


 ui->btnApply->setEnabled(false);
 ui->btnOK->setEnabled(false);
}

RouteCommentsDlg::~RouteCommentsDlg()
{
    delete ui;
}
void RouteCommentsDlg::setRoute(qint32 r)
{
    _route = r;
//    ui->txtRoute->setText(QString("%1").arg(r));
//    ui->txtRouteAlpha->setText(sql->getAlphaRoute(r, _companyKey));
    QList<int>* routes = new QList<int>();
    routes->append(r);
    ui->tableView->setSelections(routes);
}

void RouteCommentsDlg::setCompanyKey(qint32 cc)
{
    _companyKey = cc;
}

void RouteCommentsDlg::setDate(QDate dt)
{
    _date = dt;
    ui->dateEdit->setDate(dt);

    readComment(0);

    bDateChanged = false;

}

void RouteCommentsDlg::btnOK_Clicked()
{
 outputChanges();

 this->accept();
 this->close();
}

void RouteCommentsDlg::OnBtnApply_clicked()
{
 outputChanges();
}

void RouteCommentsDlg::btnCancel_Clicked()
{
    this->reject();
    this->close();
}

void RouteCommentsDlg::btnDelete_Clicked()
{
    sql->deleteRouteComment(_rc);
    this->close();
}

void RouteCommentsDlg::OnBtnNext()
{
    outputChanges();

    readComment(+1);

}
bool RouteCommentsDlg::readComment(int pos)
{
    RouteComments rc;
    if(pos < 0)
        rc = sql->getPrevRouteComment(_route, _date, _companyKey);
    else
        if(pos > 0)
            rc = sql->getNextRouteComment(_route, _date, _companyKey);
    else
            rc = sql->getRouteComment(_route, _date, _companyKey);

    if(rc.commentKey != -1)
    {
        _rc = rc;
        _date = _rc.date;
        ui->txtComments->setHtml(_rc.ci.comments);
        ui->txtTags->setText(_rc.ci.tags);
        ui->dateEdit->setDate(_rc.date);
        if(_rc.companyKey == 0 && _companyKey > 0)
            _rc.companyKey = _companyKey;

        bDateChanged = false;
        bIsDirty = false;
        setWindowTitle(tr("Route Comments"));
        return true;
    }
    _rc = rc;
    return false;
}

void RouteCommentsDlg::OnBtnPrev()
{
    outputChanges();

    readComment(-1);
}
void RouteCommentsDlg::OnDateChanged()
{
    bDateChanged = true;
}

void RouteCommentsDlg::OnDateLeave()
{
 QDate date = ui->dateEdit->date();
 if(!bDateChanged)
     return;
 outputChanges();

 _date = date;
 ui->dateEdit->setDate(date);
 bDateChanged = false;
 ui->txtComments->clear();
 ui->txtComments->setFontPointSize(9);

 ui->txtTags->clear();
 setDirty(false);

 readComment(0);
}

//void RouteCommentsDlg::OnRouteTextChanged(QString text)
//{
// if(!text.isEmpty())
//  bRouteChanged = true;
//}

void RouteCommentsDlg::outputChanges()
{
 if(bIsDirty)
 {
  _rc.ci.comments = ui->txtComments->toHtml();
  _rc.ci.tags = ui->txtTags->text();
  //qDebug()<< _rc.ci.comments;
  sql->updateRouteComment( _rc);

  QList<int>* selectedRoutes = ui->tableView->selectedRoutes();

//  if(!ui->txtAdditionalRoutes->text().isEmpty())
//  {
//   QStringList routes = ui->txtAdditionalRoutes->text().split(",");
   foreach(int route, *selectedRoutes)
   {
     _rc.route = route;
     sql->updateRouteComment( _rc);
   }
//  }
  bIsDirty = false;
  setWindowTitle(tr("Route Comments"));
  setDirty(false);
 }
}

//void RouteCommentsDlg::OnRouteLeave()
//{
// if(bRouteChanged)
// {
//  outputChanges();

////  _route = ui->txtRoute->text().toInt();
////  ui->txtRouteAlpha->setText(sql->getAlphaRoute(_route, _companyKey));
//  ui->txtComments->clear();

//  if(readComment(0))
//  {
//   setDirty(true);
//  }
// }
// bRouteChanged = false;
//}

void RouteCommentsDlg::OnAdditionalRoutesLeave()
{
 setDirty(true);
}

//void RouteCommentsDlg::OnAlphaRouteTextChanged(QString text)
//{
//    if(text.length()>0)
//    {
//        QStringList list = sql->getAlphaRoutes(text);
//        QCompleter *completer = new QCompleter(list);
//        ui->txtRouteAlpha->setCompleter(completer);
//    }
//}

//void RouteCommentsDlg::OnAlphaRouteLeave()
//{
//    QString newRoute;
//    bool bAlphaRoute;
//    int route = sql->getNumericRoute(ui->txtRouteAlpha->text(),&newRoute, &bAlphaRoute,_companyKey);
//    if(route > 0)
//    {
//        this->setRoute(route);
//    }
//}

void RouteCommentsDlg::OnTagsLeave()
{
    bIsDirty = true;
}

void RouteCommentsDlg::setDirty(bool b)
{
 bIsDirty = b;
}

void RouteCommentsDlg::OnDirtySet(bool bDirty)
{
 if(bDirty && !ui->txtComments->toPlainText().isEmpty())
 {
  ui->btnOK->setEnabled(true);
  ui->btnApply->setEnabled(true);
 }
 else
 {
  ui->btnOK->setEnabled(false);
  ui->btnApply->setEnabled(false);

 }
 setDirty(bDirty);
}

void RouteCommentsDlg::OnTextChanged()
{
 setDirty();
}
