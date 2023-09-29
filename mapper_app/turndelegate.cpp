#include "turndelegate.h"
#include "turncombo.h"

TurnDelegate::TurnDelegate(QString center)
{
 this->center = center;
}

QWidget* TurnDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

 TurnCombo* editor;

 editor = new TurnCombo(center, parent);
 return editor;
}

void TurnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
 TurnCombo *comboBox = static_cast<TurnCombo*>(editor);
 QString value = index.model()->data(index, Qt::EditRole).toString();
 comboBox->setCurrentText(value);
}

void TurnDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
 TurnCombo *comboBox = static_cast<TurnCombo*>(editor);
 model->setData(index, comboBox->currentData().toInt(), Qt::EditRole);
}
