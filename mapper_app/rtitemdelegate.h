#ifndef RTITEMDELEGATE_H
#define RTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <data.h>

class RTItemDelegate : public QStyledItemDelegate
{
 public:
  RTItemDelegate();
  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

 private:
  QMap<int, QString> routeTypes;

};

#endif // RTITEMDELEGATE_H
