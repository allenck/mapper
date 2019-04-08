#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QDialog>
#include "configuration.h"
#include "querymodel.h"
#include <QTableView>

namespace Ui {
class QueryDialog;
}

class QueryDialog : public QDialog
{
 Q_OBJECT
    
public:
 explicit QueryDialog(Configuration* cfg, QWidget *parent = 0);
 ~QueryDialog();
 void setMaxTabResults(int num);
    
private:
 Ui::QueryDialog *ui;
 Configuration* config;
 QString currQueryFilename;
 QSqlDatabase db;
 QTimer* timer;
 int i_Max_Tab_Results;
 bool boolGetItemTableView(QTableView *view);
 QModelIndex currentIndexQueryView;
// int queryViewCurrColumn;
 int currentColQueryView;
 void resizeEvent(QResizeEvent *e);
 void closeEvent(QCloseEvent *e);
 Connection* tgtConn;
 QString tgtDbType;
 bool bChanging;

private slots:
 void on_go_QueryButton_clicked();
 void on_clear_QueryButton_clicked();
 void on_load_QueryButton_clicked();
 void on_save_QueryButton_clicked();
 void quickProcess();
 void slot_queryView_row_DoubleClicked(QModelIndex index);
 void slot_QueryView_horizontalHeader_sectionDoubleClicked(int logicalIndex);
 void tablev_customContextMenu( const QPoint& pt); //query view
// void queryViewHeaderContextMenuRequested(const QPoint &pt);
// void on_queryView_hide_column();
// void on_queryView_show_columns();
 void on_copyCellText();
 void on_cb_stop_query_on_error_toggled(bool b_checked);
 void on_cb_sql_execute_after_loading_toggled(bool b_checked);
// void onMoveOrRezize_columns();
// void onResizeToData();
 void On_cbConnections_CurrentIndexChanged(int);
};

#endif // QUERYDIALOG_H
