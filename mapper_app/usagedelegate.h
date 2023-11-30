#ifndef USAGEDELEGATE_H
#define USAGEDELEGATE_H

#include <QStyledItemDelegate>
#include "sql.h"

class UsageDelegate : public QStyledItemDelegate
{
 public:
  UsageDelegate();
  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

 private:
  QStringList combos1 = {" ","OneWay"}; // single track
  QStringList combos2 = {"Bidirectional", "OneWay(normal)","OneWay(reversed" };
};

#endif // USAGEDELEGATE_H
