#include "queryeditmodel.h"
#include "mainwindow.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlError>
#include <QDebug>

QueryEditModel::QueryEditModel(QObject *parent, QSqlDatabase db)
    : QStandardItemModel(parent), m_db(db)
{
}

QueryEditModel::~QueryEditModel() {}

void QueryEditModel::setTable(const QString& table) {
    this->table = table;
}

void QueryEditModel::setFilter(const QString& filter) {
    this->filterStr = filter;
}

void QueryEditModel::setTabName(QString name) {
    tabName = name;
}

Qt::ItemFlags QueryEditModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags modelFlags = QStandardItemModel::flags(index);
    if (index.isValid()) {
        modelFlags |= Qt::ItemIsEditable; // Enable manual cell triggers
    }
    return modelFlags;
}

bool QueryEditModel::select() {
    clear(); // Wipe previous cached runs safely

    QSqlQuery query(m_db);
    query.setForwardOnly(true); // Direct, safe pipeline bypasses driver metadata faults

    QString queryString = QString("SELECT * FROM %1").arg(table);
    if (!filterStr.isEmpty()) {
        queryString += QString(" WHERE %1").arg(filterStr);
    }

    qDebug() << "QueryEditModel executing manual select query: " << queryString;

    if (!query.exec(queryString)) {
        m_lastError = query.lastError(); // FIXED: Cache the error state internally
        qCritical() << "Select failed inside QueryEditModel: " << m_lastError.text();
        return false;
    }
    m_lastError = QSqlError(); // Clear error state on successful execution

    // Capture layout column naming conventions cleanly from active memory buffer
    m_recordStructure = query.record();
    int colCount = m_recordStructure.count();

    // FIXED: Explicitly define the column width first so headers have empty slots to latch onto
    setColumnCount(colCount);

    // Map column naming fields securely to view headers
    for (int col = 0; col < colCount; ++col) {
        setHeaderData(col, Qt::Horizontal, m_recordStructure.fieldName(col));
    }

    // FIXED: Properly structured forward-only data ingestion loop
    int row = 0;
    while (query.next()) {
        QList<QStandardItem*> rowItems;
        for (int col = 0; col < colCount; ++col) {
            rowItems.append(new QStandardItem(query.value(col).toString()));
        }
        appendRow(rowItems);
        row++;
    }

    qDebug() << "QueryEditModel successfully loaded rows into memory: " << rowCount();
    return true;
}

bool QueryEditModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole || !index.isValid()) return false;

    // 1. Fetch Primary Key identity information (Using Column 0 as identity field reference)
    QModelIndex primaryKeyIndex = QStandardItemModel::index(index.row(), 0);
    QVariant primaryKeyValue = data(primaryKeyIndex, Qt::DisplayRole);
    QString primaryKeyName = m_recordStructure.fieldName(0);
    QString targetFieldName = m_recordStructure.fieldName(index.column());

    qDebug() << "Attempting memory-backed database update on table " << table
             << " WHERE " << primaryKeyName << " = " << primaryKeyValue;

    // 2. Fire direct database synchronization instruction
    QSqlQuery updateQuery(m_db);
    QString sql = QString("UPDATE %1 SET %2 = :val WHERE %3 = :id")
                      .arg(table)
                      .arg(targetFieldName)
                      .arg(primaryKeyName);

    updateQuery.prepare(sql);
    updateQuery.bindValue(":val", value);
    updateQuery.bindValue(":id", primaryKeyValue);

    if (!updateQuery.exec()) {
        qCritical() << "Direct update statement execution failure: " << updateQuery.lastError().text();
        return false;
    }

    // 3. Update the data value inside local cell memory layout instantly
    bool localResult = QStandardItemModel::setData(index, value, role);
    if (!localResult) return false;

    // Custom route notification check handles workspace canvas redraws safely
    if (table.compare("routes", Qt::CaseInsensitive) == 0) {
        // FIXED: Explicitly call record mapping with table name safely
        QSqlRecord activeRowRecord = m_db.driver()->record(table);

        SegmentData sd;
        sd.setRoute(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("Route")), Qt::DisplayRole).toInt());
        sd.setRouteName(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("Name")), Qt::DisplayRole).toString());
        sd.setStartDate(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("StartDate")), Qt::DisplayRole).toDate());
        sd.setEndDate(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("EndDate")), Qt::DisplayRole).toDate());
        sd.setSegmentId(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("LineKey")), Qt::DisplayRole).toInt());
        sd.setOneWay(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("OneWay")), Qt::DisplayRole).toString());
        sd.setTrackUsage(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("TrackUsage")), Qt::DisplayRole).toString());
        sd.setCompanyKey(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("CompanyKey")), Qt::DisplayRole).toInt());
        sd.setTractionType(data(QStandardItemModel::index(index.row(), activeRowRecord.indexOf("TractionType")), Qt::DisplayRole).toInt());

        NotifyRouteChange rc = NotifyRouteChange(SQL::MODIFY, &sd);
        emit routeChange(rc);

        MainWindow::instance()->refreshRoutes();
    }

    return true;
}

bool QueryEditModel::submit() {
    return true;
}

// FIXED: Returns cached errors to satisfy QueryDialog error checks
QSqlError QueryEditModel::lastError() const {
    return m_lastError;
}