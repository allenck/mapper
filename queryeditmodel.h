#ifndef QUERYEDITMODEL_H
#define QUERYEDITMODEL_H

#include <QStandardItemModel> // Changed from QSqlQueryModel
#include "sql.h"
#include "data.h"

class QueryEditModel : public QStandardItemModel // Inherit QStandardItemModel
{
    Q_OBJECT
public:
    QueryEditModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase());
    ~QueryEditModel();

    void setTabName(QString name);
    void setTable(const QString& table);
    void setFilter(const QString& filter);
    bool select();

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QString tabName;
    QSqlError lastError() const; // <-- ADD THIS METHOD

signals:
    void routeChange(NotifyRouteChange rc);

public slots:
    bool submit();

protected:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    QString table;
    QString filterStr;
    QSqlDatabase m_db;
    QSqlRecord m_recordStructure; // Caches database column definitions securely in memory
    QSqlError m_lastError;       // <-- ADD THIS VARIABLE
};

#endif // QUERYEDITMODEL_H
