#include "itemdelegate.h"
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QLineEdit>

ItemDelegate::ItemDelegate(QStringList values, int column)
{
    this->values = values;
    this->column = column;
}


QWidget* ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

    QWidget* editor = new QLineEdit();
    if(column == -1 || column == index.column())
    {
        editor = new QComboBox(parent);
        ((QComboBox*)editor)->addItems(values);
    }
    return editor;
}

void ItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QModelIndex mIndex = index;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
    if(column == -1 || column == index.column())
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = mIndex.model()->data(mIndex, Qt::EditRole).toUInt();
        comboBox->setCurrentText(mIndex.data().toString());
    }
    else
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(mIndex.data().toString());
    }
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QModelIndex mIndex = index;
    QAbstractItemModel* mModel = model;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
    {
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
        mModel = (QAbstractItemModel *)mIndex.model();
    }
    if(column == -1 || column == index.column())
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        mModel->setData(mIndex, comboBox->currentText(), Qt::EditRole);
    }
    else
    {
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        mModel->setData(mIndex, lineEdit->text(), Qt::EditRole);
    }
}
