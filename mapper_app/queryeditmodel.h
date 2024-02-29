#ifndef QUERYEDITMODEL_H
#define QUERYEDITMODEL_H

#include <QSqlTableModel>
#include "sql.h"
#include "data.h"

class QueryEditModel : public QSqlTableModel
{
  Q_OBJECT
 public:
  QueryEditModel(QObject* parent =0, QSqlDatabase db = QSqlDatabase());
  ~QueryEditModel();
  void setTabName(QString name);
  QVariant edit(const QModelIndex &index) const;
  QVariant data(const QModelIndex &index, int role) const;
  void setTable(const QString& table);
  QString tabName;

 signals:
  void routeChange(NotifyRouteChange rc);

 protected:
  bool setData(const QModelIndex &index, const QVariant &value, int role);
 private:
  QString table;
};

#endif // QUERYEDITMODEL_H
