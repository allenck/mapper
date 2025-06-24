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
    QModelIndex mIndex = index;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentText(mIndex.data().toString());
}

void UsageDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QModelIndex mIndex = index;
    QAbstractItemModel* mModel = model;
    if(qobject_cast<const QSortFilterProxyModel*>(index.model()))
    {
        mIndex = qobject_cast<const QSortFilterProxyModel*>(index.model())->mapToSource(index);
        mModel = (QAbstractItemModel *)mIndex.model();
    }
 QComboBox *comboBox = static_cast<QComboBox*>(editor);
 //model->setData(index, comboBox->currentData().toInt(), Qt::EditRole);

 QString combo = comboBox->currentText();
 if(combo == " ")
 {
  mModel->setData(mIndex, "  ");
 }
 else if(combo == "1Way")
 {
  mModel->setData(mIndex, "Y ");
 }
 else if(combo == "2Way")
 {
  mModel->setData(mIndex, "  ");
 }
 else if(combo == "1Way(normal)")
 {
  mModel->setData(mIndex, "YR");
 }
 else
 {
  mModel->setData(mIndex, "YL");
 }
}
