#include "ttitemdelegate.h"
#include <QSortFilterProxyModel>

TTItemDelegate::TTItemDelegate()
{
 tractionTypes = SQL::instance()->getTractionTypes();
}


QWidget* TTItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

 QComboBox* editor;

 editor = new QComboBox(parent);
 foreach(TractionTypeInfo tti , tractionTypes.values())
 {
  editor->addItem(tti.description, tti.tractionType);
 }
 return editor;
}

void TTItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QModelIndex mIndex = index;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = mIndex.model()->data(mIndex, Qt::EditRole).toUInt();
    comboBox->setCurrentText(mIndex.data().toString());
}

void TTItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QModelIndex mIndex = index;
    QAbstractItemModel* mModel = model;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
    {
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
        mModel = (QAbstractItemModel *)mIndex.model();
    }
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    mModel->setData(mIndex, comboBox->currentData().toInt(), Qt::EditRole);
}

//void TTItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
// QComboBox* widget = new QComboBox();
// setEditorData(widget, index);
// widget->resize(option.rect.size());
// QPixmap pixmap(option.rect.size());
// widget->render(&pixmap);
// painter->drawPixmap(option.rect,pixmap);
//}
