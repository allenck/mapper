#ifndef QUERYEDITMODEL_H
#define QUERYEDITMODEL_H

#include <QSqlTableModel>

class QueryEditModel : public QSqlTableModel
{
  Q_OBJECT
 public:
  QueryEditModel(QObject* parent =0, QSqlDatabase db = QSqlDatabase());
  ~QueryEditModel();
  void setTabName(QString name);
  QVariant edit(const QModelIndex &index) const;
  QVariant data(const QModelIndex &index, int role) const;

  QString tabName;

 protected:
  bool setData(const QModelIndex &index, const QVariant &value, int role);
 private:

};

#endif // QUERYEDITMODEL_H
