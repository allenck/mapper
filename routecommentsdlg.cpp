#include "routecommentsdlg.h"
#include "dialogeditcomments.h"
#include "ui_routecommentsdlg.h"
#include <QCompleter>
#include <QFontDialog>
#include <QClipboard>
#include <QTextDocumentFragment>
#include "htmltextedit.h"
#include "mainwindow.h"

RouteCommentsDlg::RouteCommentsDlg(QList<RouteData> *routeList, int companyKey, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RouteCommentsDlg)
{
    this->routeList = routeList;
    ui->setupUi(this);
    _rc.route = -1;
    _rc.companyKey = companyKey;
    routes = new QList<int>();
    aRoutes = new QStringList();
    //_date.setYMD(1800,1,1);
    _rc.date = QDate(1800,1,1);
    config = Configuration::instance();
    //sql->setConfig(config);
    sql = SQL::instance();
    ui->txtComments->setReadOnly(false);
    ui->btnIgnore->setVisible(false);
    ui->lblInfo->clear();
    _model = (RouteSelectorTableModel*)ui->tableView->model();
    setWindowTitle(tr("Route Comments"));
    setDirty(false);

    connect(ui->txtTags, SIGNAL(editingFinished()), this, SLOT(OnTagsLeave()));

    connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOK_Clicked()));
    connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(OnBtnApply_clicked()));
    connect(ui->btnCancel, SIGNAL(clicked()),this, SLOT(btnCancel_Clicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_Clicked()));
    connect(ui->btnIgnore, SIGNAL(clicked()),this, SLOT(btnIgnore_clicked()));
    connect(ui->btnScan, &QPushButton::clicked,this,[=]{
        scan();
    });
    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(OnDateChanged()));
    connect(ui->dateEdit, SIGNAL(editingFinished()), this, SLOT(OnDateLeave()));
    connect(ui->txtComments, SIGNAL(dirtySet(bool)), this, SLOT(OnDirtySet(bool)));
//    connect(ui->txtAdditionalRoutes, SIGNAL(editingFinished()), this, SLOT(OnAdditionalRoutesLeave()));
    connect(ui->txtComments, &QTextBrowser::anchorClicked,this,[=](const QUrl &link){
        qDebug() << link.toDisplayString();
    });
    connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(OnBtnNext()));
    connect(ui->btnPrev, SIGNAL(clicked()), this, SLOT(OnBtnPrev()));
//    connect(ui->txtRoute, SIGNAL(textChanged(QString)), this, SLOT(OnRouteTextChanged(QString)));
//    connect(ui->txtRoute, SIGNAL(editingFinished()), this, SLOT(OnRouteLeave()));
//    connect(ui->txtRouteAlpha,SIGNAL(textChanged(QString)), this, SLOT(OnAlphaRouteTextChanged(QString)));
//    connect(ui->txtRouteAlpha, SIGNAL(editingFinished()), this, SLOT(OnAlphaRouteLeave()));
    connect(ui->tableView, &RouteSelector::selections_changed,this, [=](QModelIndexList added, QModelIndexList deleted){
     // for(QModelIndex deletedIndex : deleted)
     // {
     //  if(deletedIndex.isValid())
     //  {
     //   int deletedRow = deletedIndex.row();
     //   RouteComments rc;
     //   QList<RouteName*> routeNameList =  ui->tableView->getList();
     //   rc.route = routeNameList.at(deletedRow)->route();
     //   rc.date = ui->dateEdit->date();
     //   sql->deleteRouteComment(rc);
     //  }
     // }
     dRoutes = new QList<int>();
     ui->lblInfo->clear();
     for(QModelIndex deletedIndex : deleted)
     {
         if(deletedIndex.isValid())
         {
            if(deletedIndex.column()== RouteSelectorTableModel::ROUTE)
            {
                 dRoutes->append(deletedIndex.data().toInt());
                ui->lblInfo->setText(QString("unselect %1").arg(deletedIndex.data().toInt()));
            }
         }
     }
     selectionModel = ui->tableView->selectionModel();
     modelIndexList = selectionModel->selectedRows();
     routes->clear();
     aRoutes->clear();
     QString txtRoutes;
     foreach (QModelIndex ix, modelIndexList) {
         int selectedRoute = ix.data().toInt();
         if(!routes->contains(selectedRoute))
             routes->append(selectedRoute);
         QModelIndex aix = _model->index(ix.row(), RouteSelectorTableModel::ROUTEALPHA);
         aRoutes->append(aix.data().toString());
         txtRoutes.append(aix.data().toString() + ",");
         _rc.routeAlpha = aix.data().toString();
         _rc.companyKey = _model->index(ix.row(),RouteSelectorTableModel::COMPANY).data().toInt();
         _rc.companyName = _model->index(ix.row(),RouteSelectorTableModel::COMPANYNAME).data().toString();
         _rc.routeName = _model->index(ix.row(),RouteSelectorTableModel::NAME).data().toString();
         if(bScanInProgress)
         {
             _rc.route = _model->index(ix.row(),RouteSelectorTableModel::ROUTE).data().toInt();
         }
     }
     txtRoutes.chop(1);
     ui->txtRoutesUsed->setText(txtRoutes);
     qDebug() << routes->count() << " routes selected";
     foreach(int r, *routes)
         qDebug() << " " << r;

     enableButtons();
    });

    connect(ui->txtComments, &QTextEdit::textChanged,this, [=]{
         _rc.ci.comments = ui->txtComments->toHtml();
         _rc.date = ui->dateEdit->date();
        // _rc.commentKey = -1;
        // _rc.ci.commentKey = -1;
        MainWindow::instance()->displayRouteComment(_rc);
         enableButtons();
    });

 ui->btnApply->setEnabled(false);
 ui->btnOK->setEnabled(false);
 connect(ui->tableView, &RouteSelector::selections_changed, this, [=]{
     ui->routesSelected->setText(QString::number(routes->count()));
 });
 connect(ui->tableView, &RouteSelector::routeSelected,this, [=](int route, int row){
     RouteComments rc = sql->getRouteComment(route, _rc.date, -1);
     if(rc.commentKey >=0)
     {
         if(!bIsDirty)
         {
             ui->txtComments->setHtml(rc.ci.comments);
             ui->txtTags->setText(rc.ci.tags);
             ui->txtCommentId->setText(QString::number(rc.ci.commentKey));
         }
         else
         {
             if(ui->txtComments->toHtml() == rc.ci.comments)
             {
                 QMessageBox::warning(this, tr("Warning"), tr("A comment for this route already exits! "));
             }
         }
     }
     else
     {
         if(!ui->txtComments->toPlainText().isEmpty())
             setDirty(true);
     }
 });
}

RouteCommentsDlg::~RouteCommentsDlg()
{
    delete ui;
}

void RouteCommentsDlg::setRoute(qint32 r)
{
    _rc.route = r;
//    ui->txtRoute->setText(QString("%1").arg(r));
//    ui->txtRouteAlpha->setText(sql->getAlphaRoute(r, _companyKey));
    routes->clear();
    aRoutes->clear();
    routes->append(r);
    ui->tableView->setSelections(routes);
}

void RouteCommentsDlg::setCompanyKey(qint32 cc)
{
    _rc.companyKey = cc;
}

void RouteCommentsDlg::setDate(QDate dt)
{
    bDateChanged = true;
    _rc.date = dt;
    ui->dateEdit->setDate(dt);

    bool rslt = readRouteComment(0);

    bDateChanged = false;

    _model->createList(routeList, dt);
    ui->tableView->setSelections(routes);

}

void RouteCommentsDlg::btnOK_Clicked()
{
 outputChanges();

 this->accept();
 this->close();
}

void RouteCommentsDlg::OnBtnApply_clicked()
{
    if(!outputChanges())
    {
        ui->lblInfo->setStyleSheet("color: red");
        ui->lblInfo->setText(tr("add failed"));
        return;
    }

    if(bScanInProgress)
    {
        orphans->removeAt(ixOrphan);
        orphansUsed++;
        if(orphans->count())
        {
            processOrphan();
            return;
        }
        else
            finishScan(scanResult);
    }
    ui->txtComments->clear();
    ui->txtTags->clear();
}

void RouteCommentsDlg::btnIgnore_clicked()
{
    setDirty(false);
    if(bScanInProgress)
    {
        orphans->removeAt(ixOrphan);
        if(orphans->count())
        {
            processOrphan();
            return;
        }
    }
    ui->txtComments->clear();
    ui->txtTags->clear();
}

void RouteCommentsDlg::btnCancel_Clicked()
{
    if(bScanInProgress)
    {
        finishScan(scanResult);
        return;
    }
    this->reject();
    this->close();
}

void RouteCommentsDlg::btnDelete_Clicked()
{
    if(bScanInProgress)
    {
        CommentInfo info = orphans->at(ixOrphan);
        if(!sql->deleteComment(info.commentKey))
        {
            return;
        }
        orphans->removeAt(ixOrphan);
        orphansDeleted++;
        if(orphans->count())
        {
            processOrphan();
            return;
        }
        else
            finishScan(scanResult);

        return;
    }
    sql->deleteRouteComment(_rc);
    this->close();
}

void RouteCommentsDlg::OnBtnNext()
{
    outputChanges();

    readRouteComment(+1);

}
bool RouteCommentsDlg::readRouteComment(int pos)
{
    RouteComments rc;
    if(pos < 0)
        rc = sql->getPrevRouteComment(_rc.route, _rc.date, _rc.commentKey, _rc.companyKey);
    else if(pos > 0)
            rc = sql->getNextRouteComment(_rc.route, _rc.date, _rc.commentKey, _rc.companyKey);
    else
         rc = sql->getRouteComment(_rc.route, _rc.date, -1);
    if(rc.commentKey == -1)
         return false;

    if(rc.commentKey != -1)
    {
        bDateChanged = true;
        _rc = rc;
        //_date = _rc.date;
        ui->dateEdit->setDate(rc.date);
        ui->txtComments->setHtml(_rc.ci.comments);
        ui->txtTags->setText(_rc.ci.tags);
        ui->txtCommentId->setText(QString::number(rc.ci.commentKey));
        if(_rc.companyKey == 0 && _rc.companyKey > 0)
            _rc.companyKey = _rc.companyKey;
        ui->txtRoutesUsed->setText(_rc.ci.routesTableToString(_rc.ci.routesUsed));
        ui->tableView->setSelections(&rc.ci.routesUsed);

        bDateChanged = false;
        setDirty(false);
        return true;
    }
    _rc = rc;
    return true;
}

void RouteCommentsDlg::OnBtnPrev()
{
    outputChanges();

    readRouteComment(-1);
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
 if(bIsDirty && !ui->txtComments->toPlainText().isEmpty())
    outputChanges();
 if(date.isValid() && date.year()>=1800)
     ui->dateEdit->setStyleSheet("color: black");
 _rc.date = date;
 setDirty(true);
 QList<RouteData> list = sql->getRoutesByEndDate(0);
 ((RouteSelectorTableModel*)ui->tableView->model())->createList(&list,_rc.date);
 bDateChanged = false;
 if(bScanInProgress)
     return;
 ui->txtComments->clear();
 ui->txtComments->setFontPointSize(9);

 ui->txtTags->clear();
 setDirty(false);

 readRouteComment(0);
}

//void RouteCommentsDlg::OnRouteTextChanged(QString text)
//{
// if(!text.isEmpty())
//  bRouteChanged = true;
//}

bool RouteCommentsDlg::outputChanges()
{
    if(routes->isEmpty())
    {
        QMessageBox::warning(this, tr("No Route"), tr("No routes are selected. Please select one or more."));
        return false;
    }

 if(bIsDirty)
 {
  _rc.ci.comments = ui->txtComments->toHtml();
  _rc.ci.tags = ui->txtTags->text();
  _rc.date = ui->dateEdit->date();
  _rc.ci.routesUsed = *routes;
  //_rc.commentKey =
  //qDebug()<< _rc.ci.comments;
  if(!sql->updateRouteComment( _rc))
  {
      return false;
  }


//  if(!ui->txtAdditionalRoutes->text().isEmpty())
//  {
//   QStringList routes = ui->txtAdditionalRoutes->text().split(",");
   foreach(int route, *routes)
   {
     _rc.route = route;
       _rc.routeName = _model->getRouteName(route);
     sql->updateRouteComment( _rc);
   }
//  }
  setDirty(false);
 }
 return true;
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
    if(!ui->txtComments->toPlainText().isEmpty())
        setDirty(true);
}

void RouteCommentsDlg::setDirty(bool b)
{
    if(b)
    {
        if(ui->txtComments->toPlainText().isEmpty())
            return;
    }
    bIsDirty = b;
    ui->btnApply->setEnabled(b);
    ui->btnOK->setEnabled(b);
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


void RouteCommentsDlg::scan()
{
    if(sql->isTransactionActive())
    {
        sql->rollbackTransaction("scan");
    }
    bScanInProgress = true;
    ui->btnNext->setVisible(false);
    ui->btnPrev->setVisible(false);
    ui->btnOK->setVisible(false);
    ui->btnIgnore->setVisible(true);
    ui->btnIgnore->setEnabled(false);
    ui->btnCancel->setText(tr("Finish"));
    commentsUpdated = 0;
    commentsDeleted = 0;
    routeCommentsDeleted = 0;
    routeCommentsAdded =0;
    invalidDates = 0;
    invalidRoutes = 0 ;
    routesDeleted = 0;
    htmlCorrected = 0;
    invalidRouteComments = 0;
    orphansDeleted = 0;
    orphansUsed = 0;
    dup_emptyOrphans =0;

    scanLog.clear();
    ui->lblInfo->setText(tr("Begin scan<br>"));

    if(sql->isTransactionActive())
    {
        int r = QMessageBox::question(this, tr("Uncommitted changes"), tr("There are uncommited changes. "
                                                                          "Click Yes to commit them, No to roll them back"
                                                                          " or cancel to do nothing"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        switch (r) {
        case QMessageBox::Yes:
            sql->commitTransaction("");
            break;
        case QMessageBox::No:
            sql->rollbackTransaction("");
            break;
        default:
            return;
        }
    }
    sql->beginTransaction("scan");

    // Get a list of all RouteComments referencing invalid Comments
    QList<RouteComments*> invalid = sql->listInvalidRouteComments();
    foreach(RouteComments* rc, invalid)
    {
        if(sql->deleteRouteComment(*rc))
        {
            scanLog.append(tr("Delete invalid RouteComments route=%1 date=%2 referencing commentKey=%3").arg(rc->route).arg(rc->date.toString("yyyy/MM/dd").arg(rc->commentKey)));
            invalidRouteComments++;
        }
    }

    QList<RouteComments*> list = sql->listRouteComments();
    scanResult = true;
    foreach(RouteComments* rc, list)
    {
        if(!rc->date.isValid())
        {
            scanLog.append( QString("- date invalid %1 route %2\n").arg(rc->date.toString()).arg(rc->route));
            invalidDates++;
        }
        QString routeAlpha = sql->getAlphaRoute(rc->route, "");
        if(routeAlpha.isEmpty() || rc->route < 1)
        {
            scanLog.append(QString("- route invalid %1 date %2\n").arg(rc->route).arg(rc->date.toString()));
            invalidRoutes++;
        }
        if(rc->ci.comments.isEmpty())
        {
            if(!sql->deleteComment(rc->ci.commentKey))
            {
                scanLog.append(QString("- Error: delete commentKey %1 failed\n").arg(rc->ci.commentKey));
                    scanResult = false;
                break;
            }
            if(!sql->deleteRouteComment(*rc))
            {
                scanLog.append( QString("- Error: delete routeComment  %1 %2 failed\n").arg(rc->route).arg(rc->date.toString()));
                    scanResult = false;
                break;
            }
            scanLog.append( QString("- delete comment %1 plaintext is empty route: %2 date: %3\n").arg(rc->ci.commentKey).arg(routeAlpha).arg(rc->date.toString()));
            commentsDeleted++;
            routesDeleted++;
            continue;
        }
        if(!HtmlTextEdit::isHtmlFragment(rc->ci.comments))
        {
            scanLog.append(QString("- not HTML commentKey: %1\n").arg(rc->ci.commentKey));
        }
        ui->txtComments->setHtml(rc->ci.comments);
        ui->txtCommentId->setText(QString::number(rc->ci.commentKey));
        qApp->processEvents();
        QString text = ui->txtComments->toPlainText();
        if(text.isEmpty())
        {
            RouteComments rc1 = sql->getRouteComment(rc->route,rc->date, rc->commentKey);
            if(rc1.commentKey < 1)
                continue; // already deleted!
            if(!sql->deleteRouteCommenUsingCommentKey(rc->ci.commentKey))
            {
                scanLog.append(QString("- Error: delete commentKey %1 failed\n").arg(rc->ci.commentKey));
                scanResult = false;
                break;
            }
            scanLog.append(QString("- delete comment %1 html is empty route: %2 date: %3\n").arg(rc->ci.commentKey).arg(routeAlpha).arg(rc->date.toString()));
            routeCommentsDeleted++;
            commentsDeleted++;
            routesDeleted++;
            continue;
        }
        if(HtmlTextEdit::isHtmlFragment(text))
        {
            ui->txtComments->setHtml(text);
            qApp->processEvents();
            rc->ci.comments = ui->txtComments->toHtml();
            if(!sql->updateComment(rc->ci))
            {
                scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
            }
            else
                htmlCorrected++;
        }
        if(text.startsWith("https://"))
        {
            if(!HtmlTextEdit::isLink(text))
            {
                QTextDocumentFragment frag = QTextDocumentFragment::fromHtml("<a href=" + text + "><span style=\" font-family:'Ubuntu'; text-decoration: underline; color:#6c7565;\">"
                                                                             + text + "</span></a></p>");
                ui->txtComments->clear();
                ui->txtComments->textCursor().insertFragment(frag);
                rc->ci.comments = ui->txtComments->toHtml();
                if(rc->ci.routesUsed.isEmpty())
                    rc->ci.routesUsed.append(rc->route);
                if(!sql->updateComment(rc->ci))
                {
                    scanLog.append(tr("- Error: link fix failed %1 commentKey: %2\n").arg(text).arg(rc->commentKey));
                }
                else
                {
                    linksFixed++;
                    scanLog.append(tr("- link fixed %1 commentKey: %2\n").arg(text).arg(rc->commentKey));
                }
            }
        }
        if(rc->ci.routesUsed.isEmpty() || !rc->ci.routesUsed.contains(rc->route))
        {
            rc->ci.routesUsed.append(rc->route);
            if(!sql->updateComment(rc->ci))
            {
               scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
                scanResult = false;
                break;
            }
            commentsUpdated++;
        }

        // if(!rc->ci.routesUsed.contains(rc->route))
        // {
        //     rc->ci.routesUsed.append(rc->route);
        //     if(!\sql->updateComment(rc->ci))
        //     {
        //         scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
        //             scanResult = false;
        //         break;
        //     }
        //     commentsUpdated++;
        // }
    }

    // now see if any orphans can be used.
    ui->btnIgnore->setEnabled(true);

    //QList<CommentInfo>* comments = \sql->getComments();
    orphans = sql->getOrphanComments();
    dup_emptyOrphans =0;
    if(orphans->isEmpty())
        finishScan(scanResult);
    // remove any empty or duplicate comments
    for(int ix = orphans->count()-1; ix>=0; ix --)
    {
        CommentInfo info = orphans->at(ix);
        if(info.comments.isEmpty())
        {
            if(sql->deleteComment(info.commentKey))
            {
                qDebug() << QString("delete %1 failed").arg(info.commentKey);
                continue;
            }
            orphans->removeLast();
            dup_emptyOrphans++;
            continue;
        }
        QTextEdit* edit = new QTextEdit(this);
        edit->setHtml(info.comments);
        if(ix >=1)
        {
            for(int i = ix-1; i >=0; i--)
            {
                CommentInfo info2 = orphans->at(i);
                QTextEdit* edit2 = new QTextEdit();
                edit2->setHtml(info2.comments);
                if(edit->toPlainText() == edit2->toPlainText())
                {
                    if(sql->deleteComment(info.commentKey))
                    {
                        qDebug() << QString("delete %1 failed").arg(info.commentKey);
                        continue;
                    }
                    orphans->removeLast();
                    dup_emptyOrphans++;
                    continue;
                }
            }
            break;
        }
        continue;
    }
    processOrphan();
    return;
}

bool RouteCommentsDlg::processOrphan()
{
    bool rslt = false;
    ui->lblInfo->setStyleSheet("color: rgb(255, 170, 0)");
    ui->lblInfo->setText(tr("Scanning of orphan comments"));
    ui->btnScan->setVisible(false);
    ui->btnIgnore->setEnabled(true);

    for(ixOrphan = orphans->count()-1; ixOrphan >= 0; ixOrphan-- )
    {
        CommentInfo info = orphans->at(ixOrphan);
        QTextEdit* edit = new QTextEdit();
        edit->setHtml(info.comments);
        setDirty(true);
        if(edit->toPlainText().isEmpty())
        {
            if(!sql->deleteComment(info.commentKey))
            {
                scanLog.append(QString("- delete commentKey %1, plaintext empty\n").arg(info.commentKey));
                rslt = false;
                break;
            }
            orphans->removeAt(ixOrphan);
            commentsDeleted++;
            continue;
        }
        QList<CommentInfo>* dups = sql->commentByText(info.comments);
        if(dups->count() > 1)
        {
            for(int i=dups->count()-1; i > 0; i--) // delete all except first!
            {
                CommentInfo ci = dups->at(i);
                if(!sql->deleteComment(ci.commentKey))
                {
                    scanLog.append(QString("- Error: delete commentKey %1 failed\n").arg(ci.commentKey));
                    rslt = false;
                    break;
                }
                commentsDeleted++;
                scanLog.append( QString("- deleted dup Comment %1\n").arg(ci.commentKey));

                dups->removeAt(i);
            }

        }
        else {
            qDebug() << "dup not found";
        }

        //     now present comment to see if it can be added to a new RouteComment
        ui->txtComments->setHtml(info.comments);
        ui->txtTags->setText(info.tags);
        ui->txtCommentId->setText(QString::number(info.commentKey));
        ui->txtRoutesUsed->clear();
        ui->dateEdit->setDate(QDate::fromString("1799/12/31","yyyy/MM/dd"));
        ui->dateEdit->setStyleSheet("color: red");
        _rc = RouteComments();
        _rc.route = -1;
        _rc.date = QDate();
        _rc.commentKey = info.commentKey;
        _rc.ci.commentKey = info.commentKey;
        _rc.ci.tags = info.tags;
        _rc.ci.comments = info.comments;

        selectionModel->clear();
        enableButtons();
        setDirty(false);

        ui->lblInfo->setStyleSheet("color: rgb(255, 170, 0)");
        ui->lblInfo->setText(tr("Click Apply to add comment %1 to route or Ignore to process next orphan").arg(info.commentKey));
        return rslt;
    }
    return rslt;
}

bool RouteCommentsDlg::finishScan(int rslt)
{
    bool rtn = false;
    ui->lblInfo->clear();
    QString msg = QString("Scan results:\n"
                          "commentsUpdated: %1<br>\n"
                          "commentsDeleted: %2<br>\n"
                          "routeCommentsDeleted: %3<br>\n"
                          "routeCommentsAdded: %4<br>\n"
                          "invalidDates: %5<br>\n"
                          "invalidRoutes: %6<br>\n"
                          "routesDeleted: %7<br>\n"
                          "htmlCorrected:%8<br>\n"
                          "linksFixed:%9<br>\n"
                          "invalidRouteCommentsDeleted:%10<br>\n"
                          "orphansDeleted:%11<br>\n"
                          "orphansUsed:%12<br>\n"
                          "dup_emptyOrphans:%13<br>\n"
                          "*****************************************************************<br>\n")
                      .arg(commentsUpdated).arg(commentsDeleted).arg(routeCommentsDeleted)
                      .arg(routeCommentsAdded).arg(invalidDates).arg(invalidRoutes).arg(routesDeleted).arg(htmlCorrected)
                      .arg(linksFixed).arg(invalidRouteComments).arg(orphansDeleted).arg(orphansUsed).arg(dup_emptyOrphans);
    if(rslt)
    {
        //int rtn = QMessageBox::question(this, tr("Commit changes"),msg + "Do you wish to commit changes?",QMessageBox::Yes | QMessageBox::No);
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Commit changes"));
        msgBox.setText(msg);
        msgBox.setInformativeText( "Do you wish to commit changes?");
        msgBox.setDetailedText(scanLog );
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setBaseSize(QSize(2000, 120));
        msgBox.setTextFormat(Qt::TextFormat::RichText);
        msgBox.setModal(true);
        int rs = msgBox.exec();

        if(rs == QMessageBox::Yes)
        {
            sql->commitTransaction("scan");
            rtn = true;
        }
        else
        {
            sql->rollbackTransaction("scan");
            rtn = false;
        }
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Errors occured"), msg + "Results will be rolled back");

        sql->rollbackTransaction("scan");
        rtn = false;
    }

    bScanInProgress = false;
    ui->btnNext->setVisible(true);
    ui->btnPrev->setVisible(true);
    ui->btnOK->setVisible(true);
    ui->btnIgnore->setVisible(false);
    ui->btnIgnore->setEnabled(false);
    ui->btnScan->setVisible(true);
    ui->btnCancel->setText(tr("Cancel"));
    ui->txtComments->clear();
    ui->txtTags->clear();
    commentsUpdated = 0;
    commentsDeleted = 0;
    routeCommentsDeleted = 0;
    routeCommentsAdded =0;
    invalidDates = 0;
    invalidRoutes = 0 ;
    routesDeleted = 0;
    htmlCorrected=0;
    linksFixed = 0;
    ixOrphan=-1;
    if(orphans)
        orphans->clear();
    return rtn;
}

// enable start and apply buttons if required input is present
void RouteCommentsDlg::enableButtons()
{
    if(_rc.route > 0 && _rc.commentKey >0 && !_rc.ci.comments.isEmpty() && _rc.date.isValid() && !ui->txtComments->toPlainText().isEmpty())
    {
        ui->btnApply->setEnabled(true);
        ui->btnOK->setEnabled(true);
    }
    else
    {
        ui->btnApply->setEnabled(false);
        ui->btnOK->setEnabled(false);


    }
}

void RouteCommentsDlg::closeEvent(QCloseEvent *e)
{
    if(bScanInProgress)
    {
        finishScan(scanResult);
        return;
    }
    this->reject();
    this->close();

}
