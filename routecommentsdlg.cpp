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
    ui->setupUi(this);
    this->routeList = routeList;
    proxyModel = (QSortFilterProxyModel*)ui->tableView->model();
    _sourceModel = (RouteSelectorTableModel*)proxyModel->sourceModel();
    _sourceModel->createList(routeList, QDate());
    config = Configuration::instance();
    if(!config->rcd.geometry.isEmpty())
        restoreGeometry(config->rcd.geometry);

    // _rc.route = -1;
    // _rc.companyKey = companyKey;
    _ci.alphaRoute = "";
    _ci.route=-1;
    _ci.companyKey = companyKey;
    _ci.date = QDate(1800,1,1);
    routes = new QList<int>();
    aRoutes = new QStringList();
    //_date.setYMD(1800,1,1);
    //sql->setConfig(config);
    sql = SQL::instance();
    ui->txtComments->setReadOnly(false);
    ui->btnIgnore->setVisible(false);
    ui->btnDelete->setEnabled(false);

    ui->lblInfo->clear();
    SQL::instance()->setForeignKeyCheck(false);
    MainWindow::_instance->foreignKeyCheckAct->setChecked(false);
    //config->setForeignKeyCheck(false);

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
    connect(ui->btnChangeDate, SIGNAL(clicked(bool)),this, SLOT(onChgDate()));
    connect(ui->tableView, &RouteSelector::selections_changed,this, [=](QModelIndexList selected, QModelIndexList deselected){
        if(!bSettingSelections)
            onSelectionsChanged(selected, deselected);
    });

    connect(ui->txtComments, &QTextEdit::textChanged,this, [=]{
        _ci.comments = ui->txtComments->toHtml();
        _ci.date = ui->dateEdit->date();
         ui->lblInfo->clear();
        // _rc.commentKey = -1;
        // _rc.ci.commentKey = -1;
        MainWindow::instance()->displayRouteComment(_ci);
        enableButtons();
    });

    ui->btnApply->setEnabled(false);
    ui->btnOK->setEnabled(false);
    connect(ui->tableView, &RouteSelector::selections_changed, this, [=]{
     ui->routesSelected->setText(QString::number(routes->count()));
    });
    connect(ui->tableView, &RouteSelector::routeSelected,this, [=](int route, QString aRoute,int row){
        onRouteSelected(route, aRoute, row);
    });

    connect(sql, &SQL::commentChange, this, [=] (CommentInfo ci, SQL::CHANGETYPE t){
        onCommentChange(ci, t);
    });

    config->rv.hiddenColumns.clear();
    // connect(ui->tableView->horizontalHeader(), &QHeaderView::sectionResized, this,
    //         [=](int logicalIndex, int oldSize, int newSize){
    //             config->rcd.state = ui->tableView->horizontalHeader()->saveState();
    //             if(!config->rcd.hiddenColumns.isEmpty())
    //                 config->rcd.hiddenColumns.replace(logicalIndex,newSize);
    // });

    // ui->tableView->hideColumn(_sourceModel->ROUTEID);
    // config->rcd.hiddenColumns.append(_sourceModel->ROUTEID);
    // ui->tableView->hideColumn(_sourceModel->ROUTEPREFIX);
    // config->rcd.hiddenColumns.append(_sourceModel->ROUTEPREFIX);
    // ui->tableView->hideColumn(_sourceModel->COMPANY);
    // config->rcd.hiddenColumns.append(_sourceModel->COMPANY);

    for(int i=0; i < _sourceModel->columnCount(QModelIndex()); i++)
    {
        if(((RouteSelector*)ui->tableView)->horizontalHeader()->isSectionHidden(i))
        {
            if(!config->rcd.hiddenColumns.contains(QVariant(i)))
                config->rcd.hiddenColumns.append(QVariant(i));
        }
        else
        {
            if(config->rcd.hiddenColumns.contains(QVariant(i)))
                config->rcd.hiddenColumns.removeOne(QVariant(i));
        }
    }
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableView->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(hdr_customContextMenu(QPoint)));

    hideColumnAct = new QAction(tr("Hide Column"),this);

    connect(hideColumnAct, &QAction::triggered, [=]{
        int logicalIndex =hideColumnAct->data().toInt();
        ui->tableView->hideColumn(logicalIndex);
        if(!config->rcd.hiddenColumns.contains(logicalIndex))
            config->rcd.hiddenColumns.append(logicalIndex);
    });

}

RouteCommentsDlg::~RouteCommentsDlg()
{
    delete ui;
}

void RouteCommentsDlg::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    config->rcd.geometry = saveGeometry();
}

void RouteCommentsDlg::hdr_customContextMenu( const QPoint pt)
{
    // curRow = ui->rowAt(pt.y());
    //curCol = ui->columnAt(pt.x());
    QMenu menu;
    hideColumnAct->setData(ui->tableView->horizontalHeader()->logicalIndexAt(pt));
    menu.addAction(hideColumnAct);
    if(config->rcd.hiddenColumns.count()>0)
    {
        QMenu* m = new QMenu(tr("Show column"));
        menu.addMenu(m);
        foreach(QVariant col, config->rcd.hiddenColumns)
        {
            QAction* a = new QAction(_sourceModel->headerData(col.toInt(),Qt::Horizontal, Qt::DisplayRole).toString(),this);
            m->addAction(a);
            connect(a, &QAction::triggered, [=]{
                ui->tableView->showColumn(col.toInt());
                config->rcd.hiddenColumns.removeOne(col.toInt());
            });
        }
    }
    menu.exec(QCursor::pos());
}

QMap<QString, RouteName*>* RouteCommentsDlg::createList(QList<RouteData>* rdList, QDate dt)
{
    aList = new QMap<QString, RouteName*>();
    foreach(RouteData rd, *rdList)
    {
        //if(dt >= rd.startDate().addDays(-700) && dt <= rd.endDate())
        {
            RouteName* rn = new RouteName();
            rn->setRoute(rd.route());
            rn->setRouteName(rd.routeName());
            rn->setRoutePrefix(rd.routePrefix());
            rn->setRouteAlpha(rd.alphaRoute());
            rn->setBaseRoute(rd.baseRoute());
            rn->setCompanyKey(rd.companyKey());
            rn->setCompanyName(rd.companyName());
            rn->setRouteId(rd.routeId());
            rn->setDate(rd.startDate());
            aList->insert(rd.alphaRoute(), rn);
        }
    }
    return aList;
}

void RouteCommentsDlg::setRouteData(RouteData rd)
{
    _rd = rd;
    _ci.date = rd.startDate();
    _ci.route = rd.route();
    _ci.alphaRoute = rd.alphaRoute();
    _ci.commentKey = rd.companyKey();
    _ci.routeId = rd.routeId();
    _ci.routeName = rd.routeName();
    _ci.companyName = rd.companyName();
    comments = sql->commentsForAlphaRoute(_ci.alphaRoute, _ci.date, &currIx);
    //    ui->txtRoute->setText(QString("%1").arg(r));
    //    ui->txtRouteAlpha->setText(sql->getAlphaRoute(r, _companyKey));
    ui->lblInfo2->setText(tr("Route <B>%3</B> comment <B><I>%1 of %2</I></B>").arg(currIx +1).arg(comments->count()).arg(_ci.alphaRoute));
    ui->btnPrev->setEnabled(currIx > 0);
    ui->btnNext->setEnabled(currIx < comments->count()-1);
    routes->clear();
    aRoutes->clear();
    routes->append(rd.route());
    aRoutes->append(rd.alphaRoute());
    //bool rslt = readRouteComment(0);
    if(currIx >=0)
    {
        _ci = comments->at(currIx);
        ui->dateEdit->setDate(_ci.date);
        displayComment(_ci);
    }
    _sourceModel->createList(routeList, _ci.date);
    bSettingSelections = true;
    ui->tableView->setSelections(aRoutes);
    bSettingSelections = false;

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
    // ui->txtComments->clear();
    // ui->txtTags->clear();
    setDirty(false);
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
    ui->btnChangeDate->setEnabled(false);
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
    // sql->deleteRouteComment(_rc);
    // this->close();
    if(sql->deleteComment(_ci.commentKey))
    {
        ui->lblInfo->setStyleSheet("color:green");
        ui->lblInfo->setText(tr("comment %1 deleted").arg(_ci.commentKey));
        ui->btnDelete->setEnabled(false);
    }
}

void RouteCommentsDlg::OnBtnNext()
{
    outputChanges();

    //readRouteComment(+1);
    if(comments && currIx >= 0 && currIx < comments->count()-1)
    {
        _ci = comments->at(++currIx);
        ui->btnPrev->setEnabled(currIx > 0);
        ui->btnNext->setEnabled(currIx < comments->count()-1);
        displayComment(_ci);
    }

}

void RouteCommentsDlg::onRouteSelected(int route, QString alphaRoute, int row)
{
    //RouteComments rc = sql->getRouteComment(route, _rc.date, -1);
    CommentInfo ci = sql->getComment(alphaRoute,_ci.date, 0);
    if(ci.commentKey >=0)
    {
        if(!bIsDirty)
        {
            ui->txtComments->setHtml(ci.comments);
            ui->txtTags->setText(ci.tags);
            ui->txtCommentId->setText(QString::number(ci.commentKey));
        }
        else
        {
            if(ui->txtComments->toHtml() == ci.comments)
            {
                //QMessageBox::warning(this, tr("Warning"), tr("A comment for this route already exits! "));
                ui->lblInfo->setStyleSheet("color: rgb(255, 170, 0)");
                ui->lblInfo->setText(tr("A comment for this route already exits! "));
            }
        }
    }
    else
    {
        if(!ui->txtComments->toPlainText().isEmpty())
            setDirty(true);
    }
}

void RouteCommentsDlg::onCommentChange(CommentInfo ci, SQL::CHANGETYPE t)
{
    CommentInfo oldCi;
    if(comments && currIx >=0)
    {
        oldCi = comments->at(currIx);
    }
    comments = sql->commentsForAlphaRoute(_ci.alphaRoute,_ci.date,&currIx);
    ui->lblInfo2->setText(tr("Route <B>%3</B> comment <B><I>%1 of %2</I></B>").arg(currIx +1).arg(comments->count()).arg(_ci.alphaRoute));
    ui->btnPrev->setEnabled(currIx > 0);
    ui->btnNext->setEnabled(currIx < comments->count()-1);
}

bool RouteCommentsDlg::readRouteComment(int pos)
{
    //RouteComments rc;
    CommentInfo ci;
    if(pos < 0)
        ci = sql->getPrevComment(_ci);
    else if(pos > 0)
            ci = sql->getNextComment(_ci);
    else
         ci = sql->getComment(_ci.alphaRoute, _ci.date, -1);
    if(ci.commentKey == -1)
         return false;

    if(ci.commentKey != -1) // is there a comment for the  date?
    {
        bDateChanged = true;
        //_rc = rc;
        _ci = ci;
        //_date = _rc.date;
        ui->dateEdit->setDate(ci.date);
        displayComment(ci);

        bDateChanged = false;
        setDirty(false);
        return true;
    }
    _ci = ci;
    setDirty(false);

    return true;
}

void RouteCommentsDlg::displayComment(CommentInfo newCi)
{
    //_rc= newRc;
    _ci = newCi;
    ui->lblInfo2->setText(tr("Route <B>%3</B> comment <B><I>%1 of %2</I></B>").arg(currIx +1).arg(comments->count()).arg(_ci.alphaRoute));

    ui->txtCommentId->setText(QString::number( _ci.commentKey));
    ui->txtComments->setHtml(_ci.comments);
    ui->txtTags->setText(_ci.tags);

    bSettingSelections = true;
    ui->tableView->setSelections(&_ci.aRoutesList);
    bSettingSelections = false;

    ui->txtRoutesUsed->setText(_ci.aRoutesList.join(','));
    ui->dateEdit->setDate(newCi.date);
    ui->btnChangeDate->setEnabled(true);
    ui->lblInfo->clear();
    // if(routes->isEmpty())
    //     routes->append(_ci.route);
    // if(!_ci.routesUsed.isEmpty())
    // {
    //     routes = &_ci.routesUsed;
    // }
    // if(_rc.routeId == -1 )
    // {
    //     _ci.routeId = ((RouteSelectorTableModel*)ui->tableView->model())->getRouteId(_rc.route);
    //     if(_rc.routeId > 0)
    //     {
    //         setDirty();
    //         outputChanges();
    //     }
    // }
    ui->btnDelete->setEnabled(true);

    setDirty(false);
}

void RouteCommentsDlg::onChgDate()
{
    QDateEdit* de = new QDateEdit(ui->dateEdit->date());
    de->setDisplayFormat("yyyy/MM/dd");

    QMessageBox box = QMessageBox(QMessageBox::Question, tr("Move Date"), tr("Enter a new date for this comment"), QMessageBox::Apply|QMessageBox::Cancel);
    QLayout* layout = box.layout();
    if(qobject_cast<QGridLayout*>(layout))
    {
        //layout->addWidget(cbCombo);
        ((QGridLayout*)layout)->addWidget(de,1,2);

        int rtn = box.exec();
        if(rtn == QMessageBox::Cancel)
            return;
    }
    _ci.date =  de->date();
    if(!sql->updateComment(_ci))
    {
        ui->lblInfo->setStyleSheet("color:red");
        ui->lblInfo->setText(tr("update failed"));
        return;
    }
    ui->txtComments->clear();
    ui->txtTags->clear();
    ui->btnChangeDate->setEnabled(false);

    OnDateLeave();
}

void RouteCommentsDlg::OnBtnPrev()
{
    outputChanges();

    //readRouteComment(-1);
    if(comments && currIx > 0 )
    {
        _ci = comments->at(--currIx);
        ui->btnPrev->setEnabled(currIx > 0);
        ui->btnNext->setEnabled(currIx < comments->count()-1);
        displayComment(_ci);
    }
}

void RouteCommentsDlg::OnDateChanged()
{
    bDateChanged = true;
}


void RouteCommentsDlg::OnDateLeave()
{
    QDate date = ui->dateEdit->date();
    ui->lblInfo->clear();
    if(!bDateChanged)
        return;
    if(bIsDirty && !ui->txtComments->toPlainText().isEmpty())
        outputChanges();
    if(date.isValid() && date.year()>=1800)
        ui->dateEdit->setStyleSheet("color: black");
    _ci.date = date;
    //setDirty(true);
    QList<RouteData> list = sql->getRoutesByStartDate(date, 700); // get routes 700 days after date
    ((RouteSelectorTableModel*)ui->tableView->model())->createList(&list,_ci.date);
    bDateChanged = false;
    if(bScanInProgress)
        return;
    ui->txtComments->clear();
    ui->btnChangeDate->setEnabled(false);
    ui->txtComments->setFontPointSize(9);
    ui->txtCommentId->clear();
    _ci.commentKey = -1;
    if(_ci.route < 1)
        _ci.route = _rd.route();
    ui->txtTags->clear();
    if(!routes->isEmpty())
    {
        CommentInfo ci = sql->getComment(_ci.alphaRoute, date, -1);
        if(_ci.commentKey > 0)
            displayComment(_ci);
    }

    setDirty(false);

    readRouteComment(0);
}

void RouteCommentsDlg::onSelectionsChanged(QModelIndexList selected, QModelIndexList deselected)
{
    //QSortFilterProxyModel* proxyModel = (QSortFilterProxyModel*)ui->tableView->model();
    dRoutes = new QList<int>();
    ui->lblInfo->clear();
    for(QModelIndex deletedIndex : deselected)
    {
        QModelIndex sIndex = proxyModel->mapToSource(deletedIndex);
        if(sIndex.isValid())
        {
            if(sIndex.column()== RouteSelectorTableModel::ROUTE)
            {
                dRoutes->append(sIndex.data().toInt());
                ui->lblInfo->setText(QString("unselect %1").arg(sIndex.data().toInt()));
            }
        }
    }
    selectionModel = ui->tableView->selectionModel();
    modelIndexList = selectionModel->selectedRows();
    routes->clear();
    aRoutes->clear();
    ui->lblInfo->clear();
    QString txtRoutes;
    foreach (QModelIndex index, modelIndexList) {
        QModelIndex ix = proxyModel->mapToSource(index);
        int selectedRoute = ix.data().toInt();
        if(!routes->contains(selectedRoute))
            routes->append(selectedRoute);
        QModelIndex aix = _sourceModel->index(ix.row(), RouteSelectorTableModel::ROUTEALPHA);
        QModelIndex rix = _sourceModel->index(ix.row(), RouteSelectorTableModel::ROUTE);
        aRoutes->append(aix.data().toString());
        txtRoutes.append(aix.data().toString() + ",");
    }
#if 0
        // _rc.commentKey = -1;
        // _rc.routeAlpha = aix.data().toString();
        // _rc.companyKey = _model->index(ix.row(),RouteSelectorTableModel::COMPANY).data().toInt();
        // _rc.companyName = _model->index(ix.row(),RouteSelectorTableModel::COMPANYNAME).data().toString();
        // _rc.routeName = _model->index(ix.row(),RouteSelectorTableModel::NAME).data().toString();
        _ci.commentKey = -1;
        _ci.alphaRoute = aix.data().toString();
        _ci.route = rix.data().toInt();
        _ci.companyKey = _model->index(ix.row(),RouteSelectorTableModel::COMPANY).data().toInt();
        _ci.companyName = _model->index(ix.row(),RouteSelectorTableModel::COMPANYNAME).data().toString();
        _ci.routeName = _model->index(ix.row(),RouteSelectorTableModel::NAME).data().toString();
        // if(bScanInProgress)
        // {
        //     _rc.route = _model->index(ix.row(),RouteSelectorTableModel::ROUTE).data().toInt();
        // }
        // else
        {
            //RouteComments newRc =sql->getRouteComment(selectedRoute, ui->dateEdit->date(),-1);
            CommentInfo newCi = sql->getComment(_ci.alphaRoute, _ci.date, -1);
            {
                if(newCi.commentKey > 0)
                {
                    if(!bIsDirty)
                    {
                        displayComment(newCi);
                    }
                }
            }
        }
    }
    txtRoutes.chop(1);
    ui->txtRoutesUsed->setText(txtRoutes);
    qDebug() << routes->count() << " routes selected";
    foreach(int r, *routes)
        qDebug() << " " << r;

    enableButtons();
#endif
    _ci.routesUsed = *routes;
    _ci.aRoutesList = *aRoutes;
    _ci.jRoutesListString = _ci.jRoutesTableToString(_ci.aRoutesList);
    ui->txtRoutesUsed->setText(_ci.aRoutesList.join(','));
}

bool RouteCommentsDlg::outputChanges()
{
    if(routes->isEmpty())
    {
        //QMessageBox::warning(this, tr("No Route"), tr("No routes are selected. Please select one or more."));
        ui->lblInfo->setStyleSheet("color:magenta");
        ui->lblInfo->setText("No routes are selected. Please select one or more.");
        return false;
    }

    ui->lblInfo->clear();
    if(bIsDirty)
    {
        _ci.comments = ui->txtComments->toHtml();
        _ci.tags = ui->txtTags->text();
        _ci.date = ui->dateEdit->date();
        _ci.routesUsed = *routes;

        sql->beginTransaction("outputChanges");

        // foreach(int route, *routes)
        // {
        //  _rc.route = route;
        //  _rc.routeName = _model->getRouteName(route);
        //  _rc.routeId = _model->getRouteId(route);
        //  if(!sql->updateRouteComment( &_rc))
        //  {

        //     return false;
        //  }
        // }
        CommentInfo oldCi = sql->getComments(_ci.commentKey);
        if(oldCi.commentKey != _ci.commentKey)
        {
            if(!sql->addComment(&_ci))
            {
                ui->lblInfo->setStyleSheet("color:red");
                ui->lblInfo->setText(tr("add failed"));

                return false;
            }
            ui->lblInfo->setStyleSheet("color:green");
            ui->lblInfo->setText(tr("comment added key = &1").arg(_ci.commentKey));
        }
        else
        {
            if(_ci.commentKey ==-1)
            {
                if(!sql->addComment(&_ci))
                {
                    sql->rollbackTransaction("outputChanges");
                    ui->lblInfo->setStyleSheet("color:red");
                    ui->lblInfo->setText(tr("add failed"));
                    return false;
                }
            }
            else
            {
                if(!sql->updateComment(_ci, true))
                {
                    sql->rollbackTransaction("outputChanges");
                    ui->lblInfo->setStyleSheet("color:red");
                    ui->lblInfo->setText(tr("add failed"));
                    return false;
                }
            }
        }
        sql->commitTransaction("outputChanges");
        //  }
        setDirty(false);
    }
    ui->lblInfo->setStyleSheet("color:green");
    ui->lblInfo->setText(tr("comment added: key=%1").arg(_ci.commentKey));
    ui->txtCommentId->setText(QString::number(_ci.commentKey));
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
    // commentsUpdated = 0;
    commentsDeleted = 0;
    // routeCommentsDeleted = 0;
    // routeCommentsAdded =0;
    // invalidDates = 0;
    // invalidRoutes = 0 ;
    // routesDeleted = 0;
    // htmlCorrected = 0;
    // invalidRouteComments = 0;
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

    // // Get a list of all RouteComments referencing invalid Comments
    // QList<RouteComments*> invalid = sql->listInvalidRouteComments();
    // foreach(RouteComments* rc, invalid)
    // {
    //     if(sql->deleteRouteComment(*rc))
    //     {
    //         scanLog.append(tr("Delete invalid RouteComments route=%1 date=%2 referencing commentKey=%3").arg(rc->route).arg(rc->date.toString("yyyy/MM/dd").arg(rc->commentKey)));
    //         invalidRouteComments++;
    //     }
    // }

    // QList<RouteComments*> list = sql->listRouteComments();
    // scanResult = true;
    // foreach(RouteComments* rc, list)
    // {
    //     if(!rc->date.isValid())
    //     {
    //         scanLog.append( QString("- date invalid %1 route %2\n").arg(rc->date.toString()).arg(rc->route));
    //         invalidDates++;
    //     }
    //     QString routeAlpha = sql->getAlphaRoute(rc->route, "");
    //     if(routeAlpha.isEmpty() || rc->route < 1)
    //     {
    //         scanLog.append(QString("- route invalid %1 date %2\n").arg(rc->route).arg(rc->date.toString()));
    //         invalidRoutes++;
    //     }
    //     if(rc->ci.comments.isEmpty())
    //     {
    //         if(!sql->deleteComment(rc->ci.commentKey))
    //         {
    //             scanLog.append(QString("- Error: delete commentKey %1 failed\n").arg(rc->ci.commentKey));
    //                 scanResult = false;
    //             break;
    //         }
    //         if(!sql->deleteRouteComment(*rc))
    //         {
    //             scanLog.append( QString("- Error: delete routeComment  %1 %2 failed\n").arg(rc->route).arg(rc->date.toString()));
    //                 scanResult = false;
    //             break;
    //         }
    //         scanLog.append( QString("- delete comment %1 plaintext is empty route: %2 date: %3\n").arg(rc->ci.commentKey).arg(routeAlpha).arg(rc->date.toString()));
    //         commentsDeleted++;
    //         routesDeleted++;
    //         continue;
    //     }
    //     if(!HtmlTextEdit::isHtmlFragment(rc->ci.comments))
    //     {
    //         scanLog.append(QString("- not HTML commentKey: %1\n").arg(rc->ci.commentKey));
    //     }
    //     ui->txtComments->setHtml(rc->ci.comments);
    //     ui->txtCommentId->setText(QString::number(rc->ci.commentKey));
    //     qApp->processEvents();
    //     QString text = ui->txtComments->toPlainText();
    //     if(text.isEmpty())
    //     {
    //         RouteComments rc1 = sql->getRouteComment(rc->route,rc->date, rc->commentKey);
    //         if(rc1.commentKey < 1)
    //             continue; // already deleted!
    //         if(!sql->deleteRouteCommenUsingCommentKey(rc->ci.commentKey))
    //         {
    //             scanLog.append(QString("- Error: delete commentKey %1 failed\n").arg(rc->ci.commentKey));
    //             scanResult = false;
    //             break;
    //         }
    //         scanLog.append(QString("- delete comment %1 html is empty route: %2 date: %3\n").arg(rc->ci.commentKey).arg(routeAlpha).arg(rc->date.toString()));
    //         routeCommentsDeleted++;
    //         commentsDeleted++;
    //         routesDeleted++;
    //         continue;
    //     }
    //     if(HtmlTextEdit::isHtmlFragment(text))
    //     {
    //         ui->txtComments->setHtml(text);
    //         qApp->processEvents();
    //         rc->ci.comments = ui->txtComments->toHtml();
    //         if(!sql->updateComment(rc->ci))
    //         {
    //             scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
    //         }
    //         else
    //             htmlCorrected++;
    //     }
    //     if(text.startsWith("https://"))
    //     {
    //         if(!HtmlTextEdit::isLink(text))
    //         {
    //             QTextDocumentFragment frag = QTextDocumentFragment::fromHtml("<a href=" + text + "><span style=\" font-family:'Ubuntu'; text-decoration: underline; color:#6c7565;\">"
    //                                                                          + text + "</span></a></p>");
    //             ui->txtComments->clear();
    //             ui->btnChangeDate->setEnabled(true);
    //             ui->txtComments->textCursor().insertFragment(frag);
    //             rc->ci.comments = ui->txtComments->toHtml();
    //             if(rc->ci.routesUsed.isEmpty())
    //                 rc->ci.routesUsed.append(rc->route);
    //             if(!sql->updateComment(rc->ci))
    //             {
    //                 scanLog.append(tr("- Error: link fix failed %1 commentKey: %2\n").arg(text).arg(rc->commentKey));
    //             }
    //             else
    //             {
    //                 linksFixed++;
    //                 scanLog.append(tr("- link fixed %1 commentKey: %2\n").arg(text).arg(rc->commentKey));
    //             }
    //         }
    //     }
    //     if(rc->ci.routesUsed.isEmpty() || !rc->ci.routesUsed.contains(rc->route))
    //     {
    //         rc->ci.routesUsed.append(rc->route);
    //         if(!sql->updateComment(rc->ci))
    //         {
    //            scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
    //             scanResult = false;
    //             break;
    //         }
    //         commentsUpdated++;
    //     }

    //     // if(!rc->ci.routesUsed.contains(rc->route))
    //     // {
    //     //     rc->ci.routesUsed.append(rc->route);
    //     //     if(!\sql->updateComment(rc->ci))
    //     //     {
    //     //         scanLog.append(QString("- Error: update commentKey %1 failed\n").arg(rc->ci.commentKey));
    //     //             scanResult = false;
    //     //         break;
    //     //     }
    //     //     commentsUpdated++;
    //     // }
    // }

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

        if(selectionModel == nullptr)
            selectionModel = ui->tableView->selectionModel();
        else
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
    //                       "commentsUpdated: %1<br>\n"
                          "commentsDeleted: %2<br>\n"
    //                       "routeCommentsDeleted: %3<br>\n"
    //                       "routeCommentsAdded: %4<br>\n"
    //                       "invalidDates: %5<br>\n"
    //                       "invalidRoutes: %6<br>\n"
    //                       "routesDeleted: %7<br>\n"
    //                       "htmlCorrected:%8<br>\n"
    //                       "linksFixed:%9<br>\n"
    //                       "invalidRouteCommentsDeleted:%10<br>\n"
                          "orphansDeleted:%11<br>\n"
                          "orphansUsed:%12<br>\n"
                          "dup_emptyOrphans:%13<br>\n"
                          "*****************************************************************<br>\n")
    //                   .arg(commentsUpdated)
                      .arg(commentsDeleted)
                      //.arg(routeCommentsDeleted)
    //                   .arg(routeCommentsAdded).arg(invalidDates).arg(invalidRoutes).arg(routesDeleted).arg(htmlCorrected)
    //                   .arg(linksFixed).arg(invalidRouteComments)
                            .arg(orphansDeleted).arg(orphansUsed).arg(dup_emptyOrphans);
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
    ui->btnChangeDate->setEnabled(false);
    ui->txtTags->clear();
    //commentsUpdated = 0;
    commentsDeleted = 0;
    //routeCommentsDeleted = 0;
    //routeCommentsAdded =0;
    //invalidDates = 0;
    //invalidRoutes = 0 ;
    //routesDeleted = 0;
    //htmlCorrected=0;
    //linksFixed = 0;
    ixOrphan=-1;
    if(orphans)
        orphans->clear();
    return rtn;
}

// enable start and apply buttons if required input is present
void RouteCommentsDlg::enableButtons()
{
    if(/*_ci.route > 0 &&*/ !_ci.alphaRoute.isEmpty() && !_ci.comments.isEmpty() && _ci.date.isValid() && !ui->txtComments->toPlainText().isEmpty())
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

/*static*/ bool RouteCommentsDlg::upgrade()
{
    QList<RouteComments*> list = SQL::instance()->listRouteComments();

    SQL::instance()->beginTransaction("upgrade");
    for(RouteComments* rc : list)
    {
        QVariantList vl;
        QSqlDatabase db = QSqlDatabase();
        if(rc->ci.date.isNull())
        {
            rc->ci.aRoutes = QJsonArray();
            rc->ci.date = rc->date;
            QStringList sl;
            if(rc->ci.routesUsed.isEmpty())
                rc->ci.routesUsed.append(rc->route);
            for(int route : rc->ci.routesUsed)
            {
                if(route < 1)
                    continue;
                if(SQL::instance()->executeCommand(QString("select routeAlpha from altRoute where route = %1").arg(route),db,&vl)){
                   rc->ci.aRoutes.append(vl.at(0).toString());
                    sl.append(vl.at(0).toString());
                }
            }
            rc->ci.jRoutesListString = rc->ci.jRoutesTableToString(sl);
            if(!SQL::instance()->updateComment(rc->ci,true))
            {
                qDebug() << tr("upgrade: commentKey %1 not updated").arg(rc->ci.commentKey);
                continue;
            }
        }
    }
    SQL::instance()->commitTransaction("upgrade");

    return true;
}
