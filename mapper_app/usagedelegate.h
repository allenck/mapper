#ifndef USAGEDELEGATE_H
#define USAGEDELEGATE_H

#include <QStyledItemDelegate>

class UsageDelegate : public QStyledItemDelegate
{
 public:
  UsageDelegate();
  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

 private:
  QStringList combos1 = {" ","1Way"}; // single track
  QStringList combos2 = {"2Way", "1Way(normal)","1Way(reversed" };
};

#endif // USAGEDELEGATE_H
