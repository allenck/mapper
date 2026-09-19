#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include "qstyleditemdelegate.h"
#include <QObject>

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QStringList values, int column = -1);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    QStringList values;
    int column = -1;
};


#endif // ITEMDELEGATE_H
