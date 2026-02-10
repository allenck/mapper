#include "dialogeditcomments.h"
#include "ui_dialogeditcomments.h"
#include "sql.h"
#include "mainwindow.h"

DialogEditComments::DialogEditComments(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogEditComments)
{
    ui->setupUi(this);
    config = Configuration::instance();
    ui->commentKey->setReadOnly(true);
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

    connect(ui->btnClose,&QPushButton::clicked, this, [=]{
        close();
    });

    connect(ui->btnSave,&QPushButton::clicked, this, [=]{  // ie Save
        info->comments = htmlText;
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
