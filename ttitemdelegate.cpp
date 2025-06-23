#include "ttitemdelegate.h"

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
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 int value = index.model()->data(index, Qt::EditRole).toUInt();
 comboBox->setCurrentText(index.data().toString());
}

void TTItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 model->setData(index, comboBox->currentData().toInt(), Qt::EditRole);
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
