#ifndef TTITEMDELEGATE_H
#define TTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "sql.h"

class TTItemDelegate : public QStyledItemDelegate
{
 public:
  TTItemDelegate();
  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

 private:
  QMap<int, TractionTypeInfo> tractionTypes;
};

#endif // TTITEMDELEGATE_H
