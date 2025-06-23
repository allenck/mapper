#ifndef TURNDELEGATE_H
#define TURNDELEGATE_H

#include <QStyledItemDelegate>

class TurnDelegate : public QStyledItemDelegate
{
 public:
  TurnDelegate(QString center);
  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
 private:
  QString center;
};

#endif // TURNDELEGATE_H
