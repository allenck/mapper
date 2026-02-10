#ifndef DIALOGEDITCOMMENTS_H
#define DIALOGEDITCOMMENTS_H

#include <QDialog>
#include "configuration.h"
#include <QTableView>

namespace Ui {
class DialogEditComments;
}

class CommentSelectorTableModel;
class DialogEditComments : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditComments(QWidget *parent = nullptr);
    ~DialogEditComments();
    void setData(QString text);
    QString getSource();

private:
    Ui::DialogEditComments *ui;
    QString htmlText;
    QModelIndex curRow;
    Configuration* config;
    bool textChangeing = false;
    QList<CommentInfo*> *list;
    CommentInfo* info;
    QList<RouteComments*> rcList;

protected:
    CommentSelectorTableModel* model = nullptr;
};

class CommentSelector : public QTableView
{
    Q_OBJECT
public:
    CommentSelector(QWidget* parent =0);
    ~CommentSelector() {}
    CommentSelector(const CommentSelector&) : QTableView() {}

private:
    QList<CommentInfo*> *list;

};
class CommentSelectorTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CommentSelectorTableModel(QList<CommentInfo*> *list, QObject *parent=0);
    enum COLUMNS{COMMENTKEY,TAGS,ROUTES,COMMENTS};
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    void deleteteRow(QModelIndex index);

private:
    QList<CommentInfo*> *list;
};
#endif // DIALOGEDITCOMMENTS_H
