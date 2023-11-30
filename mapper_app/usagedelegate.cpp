#include "usagedelegate.h"
#include "routeviewtablemodel.h"

UsageDelegate::UsageDelegate()
{

}


QWidget* UsageDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

 QComboBox* editor;

 editor = new QComboBox(parent);
 QModelIndex ix = index.model()->index(index.row(), RouteViewTableModel::TRACKS);
 int tracks = index.model()->data(ix).toInt();
 if(tracks==1)
  editor->addItems(combos1);
 else
  editor->addItems(combos2);
 return editor;
}

void UsageDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 int value = index.model()->data(index, Qt::EditRole).toUInt();
 comboBox->setCurrentText(index.data().toString());
}

void UsageDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 //model->setData(index, comboBox->currentData().toInt(), Qt::EditRole);

 QString combo = comboBox->currentText();
 if(combo == " ")
 {
  model->setData(index, "  ");
 }
 else if(combo == "OneWay")
 {
  model->setData(index, "Y ");
 }
 else if(combo == "Bidirectional")
 {
  model->setData(index, "  ");
 }
 else if(combo == "OneWay(normal)")
 {
  model->setData(index, "YR");
 }
 else
 {
  model->setData(index, "YL");
 }
}
