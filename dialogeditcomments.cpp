#include "dialogeditcomments.h"
#include "ui_dialogeditcomments.h"
#include "sql.h"
#include "mainwindow.h"
#include <QModelIndexList>

DialogEditComments::DialogEditComments(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogEditComments)
{
    common();
    setWindowTitle(tr("Edit comment"));
    ui->label_5->setVisible(false);
    ui->label_6->setVisible(false);
    ui->txtRoute->setVisible(false);
    ui->dateEdit->setVisible(false);
}
DialogEditComments::DialogEditComments(CommentInfo* ci, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogEditComments)
{
    common();
    setWindowTitle(tr("Add Route Comment"));
    ui->tableView->setVisible(false);
    ui->label_5->setVisible(true);
    ui->label_6->setVisible(true);
    ui->txtRoute->setVisible(true);
    ui->dateEdit->setVisible(true);
    ui->textEdit->setHtml(ci->comments);
    ui->tags->setText(ci->tags);
    info = ci;
}

void DialogEditComments::common()
{
    ui->setupUi(this);
    config = Configuration::instance();
    ui->commentKey->setReadOnly(true);
    routes = new QList<int>();
    aRoutes = new QStringList();
    _model = (RouteSelectorTableModel*)ui->tableView->model();

    connect(ui->htmlText, &QPlainTextEdit::textChanged, this, [=]{
        if(!textChangeing)
        {
            textChangeing = true;
            htmlText = ui->htmlText->toPlainText();
            ui->textEdit->setHtml(htmlText);
            textChangeing=false;
        }
    });
    connect(ui->textEdit, &QTextEdit::textChanged, this, [=]{
        if(!textChangeing)
        {
            textChangeing = true;
            htmlText = ui->textEdit->toHtml();
            ui->htmlText->setPlainText(htmlText);
            textChangeing=false;
        }
    });

    list = SQL::instance()->commentsList();
    model = new CommentSelectorTableModel(list);
    ui->tableView->setModel(model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    //ui->tableView->setMultiSelection(true);

    connect(ui->tableView, &QTableView::clicked, this, [=](QModelIndex index){
        info = list->at(index.row());
        curRow = index;
        ui->textEdit->setHtml(info->comments);
        ui->commentKey->setText(QString::number(info->commentKey));
        rcList = SQL::instance()->getRouteComments(info->commentKey);
    });
    connect(ui->tableView, &RouteSelector::selections_changed,this, [=](QModelIndexList added, QModelIndexList deleted)
    {
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

     QItemSelectionModel* sm = ui->tableView->selectionModel();
     modelIndexList = sm->selectedRows();
     routes->clear();
     aRoutes->clear();
     QString txtRoutes;
     foreach (QModelIndex ix, modelIndexList)
     {
         int selectedRoute = ix.data().toInt();
         if(!routes->contains(selectedRoute))
             routes->append(selectedRoute);
         QModelIndex aix = _model->index(ix.row(), RouteSelectorTableModel::ROUTEALPHA);
         aRoutes->append(aix.data().toString());
         txtRoutes.append(aix.data().toString() + ",");
     }
     txtRoutes.chop(1);
    });

    connect(ui->btnClose,&QPushButton::clicked, this, [=]{
        close();
    });

    connect(ui->btnSave,&QPushButton::clicked, this, [=]{  // ie Save
        info->comments = htmlText;
        if(ui->label_5->isVisible())
        {
            RouteComments rc;
            bool ok;
            rc.route = ui->txtRoute->text().toInt(&ok);
            rc.date = ui->dateEdit->date();
            if(ok)
            {
                rc.commentKey= info->commentKey;
                rc.ci.commentKey = info->commentKey;
                rc.ci.comments = info->comments;
                rc.ci.tags = info->tags;
                rc.ci.routesUsed.append(rc.route);
                if(!SQL::instance()->addRouteComment(rc))
                {
                    qDebug() << "add route comment failed!";
                    ui->lblInfo->setText("addrouteComment failed");
                    return;
                }
                accept();
            }
            reject();
        }
    });

        ui->tags->clear();
    connect(ui->btnDelete,&QPushButton::clicked, this, [=]{  // ie delete
        SQL::instance()->deleteComment(info->commentKey);
        model->deleteteRow(curRow);
        ui->textEdit->clear();
        ui->commentKey->clear();

        foreach (RouteComments* rc , rcList) {
            SQL::instance()->deleteRouteComment(*rc);
        }
    });

};


DialogEditComments::~DialogEditComments()
{
    delete ui;
}
void DialogEditComments::setData(QString text)
{
    ui->htmlText->setPlainText(text);
    htmlText = text;
    ui->textEdit->setHtml(text);
}

QString DialogEditComments::getSource()
{
    return ui->htmlText->toPlainText();
}


//**************************************************************************


CommentSelector::CommentSelector(QWidget* parent) : QTableView(parent)
{
    list = SQL::instance()->commentsList();
}

//**************************************************************************

CommentSelectorTableModel::CommentSelectorTableModel(QList<CommentInfo *> *list, QObject* parent) : QAbstractTableModel(parent)
{
    this->list = list;
}

int CommentSelectorTableModel::rowCount(const QModelIndex &parent) const
{
    return list->count();
}

int CommentSelectorTableModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant CommentSelectorTableModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        CommentInfo* info = list->at(index.row());
        switch(index.column())
        {
        case COMMENTKEY:
            return info->commentKey;
        case TAGS:
            return info->tags;
        case COMMENTS:
            return info->comments;
        case ROUTES:
            return info->routeCount;
        // case STATIONS:
        //     return info->stationCount;
        }

    }
    return QVariant();
}

QVariant CommentSelectorTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
        case COMMENTKEY:
            return tr("Key");
        case TAGS:
            return tr("Tags");
        case COMMENTS:
            return tr("Comments");
        case ROUTES:
            return tr("Routes");
        // case STATIONS:
        //     return tr("Stations");
        }
    }
    return QVariant();
}

Qt::ItemFlags CommentSelectorTableModel::flags(const QModelIndex &index) const
{
    switch(index.column())
    {
    case TAGS:
    case COMMENTS:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    case COMMENTKEY:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    default:
        return Qt::ItemIsEnabled;
    }
}

bool CommentSelectorTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::EditRole)
    {
        CommentInfo* cmt = list->at(index.row());
        switch(index.column())
        {
        case COMMENTS:
            // routeName->setRoutePrefix(value.toString());
            // SQL::instance()->updateAltRoute(routeName->route(), value.toString());
            break;
        }
    }
    return false;
}

void CommentSelectorTableModel::deleteteRow(QModelIndex index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    list->removeAt(index.row());
    endRemoveRows();
}
