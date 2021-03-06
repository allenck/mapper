#include "querydialog.h"
#include "ui_querydialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QSqlRecord>
#include <QClipboard>
#include <QResizeEvent>
#include <QCloseEvent>
#include "sql.h"

QueryDialog::QueryDialog(Configuration* cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryDialog)
{
  ui->setupUi(this);
  config = cfg;
  currQueryFilename = "";
  bChanging = false;
  tgtConn = config->currCity->connections.at(config->currCity->curConnectionId);
  db = QSqlDatabase::database();
  if(tgtConn->driver() == "QODBC" || tgtConn->driver() == "QODBC3")
  {
   tgtDbType = "MsSql";
  }
  else
  {
   if(tgtConn->driver() == "QSQLITE")
    tgtDbType= "Sqlite";
   else
    tgtDbType = "MySql";
  }

  i_Max_Tab_Results = 10;
  ui->cb_stop_query_on_error->setChecked(config->q.b_stop_query_on_error);
  ui->cb_sql_execute_after_loading->setChecked(config->q.b_sql_execute_after_loading);
  restoreGeometry(config->q.geometry);

  ui->cbConnections->clear();
  for(int i=0; i<config->currCity->connections.count(); i++)
  {
   Connection* c = config->currCity->connections.at(i);
   ui->cbConnections->addItem(c->description());
   if(c->id() == config->currConnection->id())
    ui->cbConnections->setCurrentIndex(i);

  }
  setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));

  for(int i=0; i<config->currCity->connections.count(); i++)
  {
   Connection* c = config->currCity->connections.at(i);
   if(c->id() == config->currCity->curConnectionId)
   {
    ui->cbConnections->setCurrentIndex(i);
    break;
   }
  }
  connect(ui->cbConnections, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbConnections_CurrentIndexChanged(int)));
}

QueryDialog::~QueryDialog()
{
 delete ui;
}
void QueryDialog::resizeEvent(QResizeEvent *e)
{
 Q_UNUSED(e)
 config->q.geometry = saveGeometry();
}
void QueryDialog::closeEvent(QCloseEvent *e)
{
 Q_UNUSED(e)
 config->q.geometry = saveGeometry();
}

void QueryDialog::on_cb_sql_execute_after_loading_toggled(bool b_checked)
{
 config->q.b_sql_execute_after_loading = b_checked;
}

void QueryDialog::on_cb_stop_query_on_error_toggled(bool b_checked)
{
 config->q.b_stop_query_on_error = b_checked;
}

void QueryDialog::on_clear_QueryButton_clicked()
{
 ui->editQuery->clear();
 currQueryFilename = "";
}

void QueryDialog::on_load_QueryButton_clicked()
{
 QString s_File_Name = QFileDialog::getOpenFileName(this, "Choose a SQL text file",
                       config->q.s_query_path, "SQL text files (*.sql *.txt);;All Files (*.*)");
 // QFileDialog take a long time to close, this should tak care of this - but does not.
 QCoreApplication::processEvents();
 if (s_File_Name.isEmpty()) return;
 QFile this_file(s_File_Name);
 QFileInfo this_fi(s_File_Name);
 if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  QMessageBox::critical(this,tr("Error"), "Could not load sql query text file");
  return;
 }
 config->q.s_query_path = this_fi.dir().absolutePath();
 currQueryFilename = s_File_Name;

 if(this_fi.size() > 1000000)
 {
  // large file
  QMessageBox msgBox;
  msgBox.setText(tr("The query is very large"));
  msgBox.setInformativeText(tr("The query is too large to display. Select 'No' to load it anyway, 'Yes' to process in the background or 'Cancel'.\nDo you wish to process the queries in the background?"));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.setIcon(QMessageBox::Warning);
  switch(msgBox.exec())
  {
   case QMessageBox::Cancel:
    return;
   case QMessageBox::No:
    goto loadIt;
   case QMessageBox::Yes:
    break;
  }
  for (int ix =ui->widget_query_view->count()-1; ix > 0; ix--)
   ui->widget_query_view->removeTab(ix);
  ui->queryResultText->clear();

  //DbConnection *dbc = this_dblist->getDbConnection(this_dblist->current_index);
  QTextStream in(&this_file);
  int linesRead=0, recordsProcessed=0, errors=0;
  this->setCursor(Qt::WaitCursor);
  qApp->processEvents();

  while(!in.atEnd())
  {
   QString queryStr;
   do
   {
    QString line = in.readLine();
    linesRead++;
    //qDebug()<<line;
    queryStr += line;
   } while (!queryStr.endsWith(";"));
   //qDebug()<<queryStr;
   QSqlQuery query = QSqlQuery(queryStr, db);
   //QStringList sa_Message_Text;
   if (query.lastError().isValid())
   {
    errors++;
    ui->queryResultText->append(QString("%1\n%2").arg(query.lastError().driverText()).arg(query.lastError().databaseText())+"\n");
    ui->queryResultText->append(QString(tr("Line %1 ")).arg(linesRead--)+" " +query.lastQuery()+"\n");
    if(ui->cb_stop_query_on_error->isChecked())
    {
     ui->queryResultText->append(tr("Query stopped because of errors\n"));
     this->setCursor(Qt::ArrowCursor);
     return;
    }
   }
   recordsProcessed++;
   if(recordsProcessed%1000 == 0)
    ui->queryResultText->append(QString(tr("Records processed: %1\n")).arg(recordsProcessed));

   qApp->processEvents();
  } // !while.atEnd()
  this->setCursor(Qt::ArrowCursor);
  ui->queryResultText->append(QString(tr("Records processed: %1\n")).arg(recordsProcessed));
  ui->queryResultText->append(QString(tr("There were errors: %1\n")).arg(errors));
  return;
 }
loadIt:
 QTextStream in(&this_file);
 ui->editQuery->setPlainText(in.readAll());
 if (ui->cb_sql_execute_after_loading->checkState() == Qt::Checked)
 {
  ui->cb_sql_execute_after_loading->setChecked(false);
  on_go_QueryButton_clicked();
 }
}

void QueryDialog::on_go_QueryButton_clicked()
{

 // ui->treeDbList->currentIndex()
 QWidget *tab_First_Result=0;
 QStringList sa_Message_Text;
 int i_Message_Error=0;
 int i_Message_Result_Yes=0;
 int i_Message_Result_No=0;
 int i_Message_Rows_effected=0;
 int i_Message_Total=0;
 int i_Rows_Total=0;
 //DbConnection *dbc = this_dblist->getDbConnection(this_dblist->current_index);
// if (!db)
// {
//  //query_View->hide();
//  //queryResultText->show();
//  ui->queryResultText->setPlainText("No database connection selected.");
//  return;
// }
// if (db.isOpen())
// {
//  QSqlError ce = dbc->connect(this_dblist);
//  if (ce.isValid())
//  {
//   //query_View->hide();
//   //queryResultText->show();
//   queryResultText->setPlainText(QString("%1\n%2").arg(ce.driverText()).arg(ce.databaseText()));
//   return;
//  }
// }
 // remove all but the first tab.
 for (int ix =ui->widget_query_view->count()-1; ix > 0; ix--)
  ui->widget_query_view->removeTab(ix);
 ui->queryResultText->clear();

// if ((QGeomColl::b_query_as_csv) || (QGeomColl::b_csv_to_file))
//  return query_as_csv(dbc->db);

 // initialize new sql query object
// QWidget * tab_search=(GeomCollTab*)(*(tab_GeomColl))->parentWidget();
// QWidget *tab_main_child;
QString s_Search; //=tab_search->objectName();
// for (int i=0; i<((GeomCollTab*)this_parent)->count(); ++i)
// {
//  tab_search = ((GeomCollTab*)this_parent)->widget(i);
//  if (s_Search == tab_search->objectName())
//   tab_main_child=tab_search;
// }
// if (!tab_main_child)
// {
//  qDebug()<<"-E-> GeomCollTab::on_go_QueryButton_clicked: searching for ["<<s_Search<<"] failed.";
// }
 this->setCursor(Qt::WaitCursor);
 timer = new QTimer(this);
 connect(timer,SIGNAL(timeout()),this,SLOT(quickProcess()));
 timer->start(1000);
 //query_View_modell = new QGeomCollQueryModel(this,dbc->db,this,tab_GeomColl,tab_main_child);
 this->setCursor(Qt::WaitCursor);
 QString text;
 QTextCursor cur = ui->editQuery->textCursor();
 if (cur.hasSelection())
 {
  text = cur.selectedText();
 }
 else
  text = ui->editQuery->toPlainText();
 QStringList list = text.split(";\n");
 foreach(QString txt, list)
 {
  if (txt.trimmed().isEmpty())
   continue;
  QueryModel *query_view_model = new QueryModel(this ,db, config->currConnection->servertype());
  //queryModelList.append(query_view_modell);
  query_view_model->setQuery(txt, db);
  if (query_view_model->lastError().isValid())
  {
   //query_View->hide();
   //queryResultText->show();
   sa_Message_Text.append(QString("%1\n%2").arg(query_view_model->lastError().driverText()).arg(query_view_model->lastError().databaseText()));
   sa_Message_Text.append(txt);
   i_Message_Error++;
   if(ui->cb_stop_query_on_error->isChecked())
   {
    sa_Message_Text.append(tr("Query stopped because of errors"));
    for (int i=0; i<sa_Message_Text.count(); i++)
     s_Search+=sa_Message_Text[i]+"\n";
    ui->queryResultText->setPlainText(s_Search);
    return;
   }
  }
  else
  {
   // query was successful
   if (query_view_model->query().isSelect())
   {
    i_Rows_Total += query_view_model->rowCount();
    i_Message_Result_Yes++;
    if (i_Message_Result_Yes <= i_Max_Tab_Results)
    {
     QTableView *query_view = new QTableView();
     query_view->setAlternatingRowColors(true);
     query_view->setSelectionMode(QAbstractItemView::ContiguousSelection);
     connect(query_view->horizontalHeader(),SIGNAL(sectionDoubleClicked(int)),this,SLOT(slot_QueryView_horizontalHeader_sectionDoubleClicked(int)));
     connect(query_view,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slot_queryView_row_DoubleClicked(QModelIndex)));
     query_view->setContextMenuPolicy(Qt::CustomContextMenu);
     connect(query_view,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tablev_customContextMenu(QPoint)));
     myHeaderView *hv = new myHeaderView(Qt::Horizontal, query_view);
     query_view->setHorizontalHeader(hv);
//     query_view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
//     connect(query_view->horizontalHeader(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(queryViewHeaderContextMenuRequested(const QPoint)));
//     query_view->horizontalHeader()->setMovable(true);
     query_view->setSortingEnabled(false);
     QSortFilterProxyModel *sm = new QSortFilterProxyModel(query_view);
     sm->setSourceModel(query_view_model);
     query_view->setModel(sm);
     query_view->setSortingEnabled(true);
     sm->sort(-1,Qt::AscendingOrder);
     QString tabTitle = tr("Result ") + QString("%1").arg(ui->widget_query_view->count());
     if (tab_First_Result == 0)
     {
      tab_First_Result=query_view;
     }
     int i_tab = ui->widget_query_view->addTab(query_view, tabTitle);
     //qDebug() << QString("tab %1 added").arg(i_tab);
     query_view_model->setTabName(tabTitle);
     query_view->resizeColumnsToContents();
     query_view->resizeRowsToContents();
    }
    if (i_Message_Result_Yes == i_Max_Tab_Results)
    {
     sa_Message_Text.append(QString(tr("No more Tab-Results will be shown. Maximum allowed: %1 .")).arg(i_Max_Tab_Results));
    }
   }
   else
   {
    //query_View->hide();
    //queryResultText->show();
    sa_Message_Text.append(QString("%1 rows affected.").arg(query_view_model->query().numRowsAffected()));
    i_Message_Rows_effected+=query_view_model->query().numRowsAffected();
    i_Message_Result_No++;
   }
  }
  qApp->processEvents();
 }
 s_Search="";
 if (i_Message_Error > 0)
 {
  if (i_Message_Error > 1)
   s_Search="s";
  sa_Message_Text.append(QString("%1 Statement%2 - that produced errors").arg(i_Message_Error).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Error;
 }
 if (i_Message_Result_Yes > 0)
 {
  if (i_Message_Result_Yes > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - that produced results - were completed correctly.")).arg(i_Message_Result_Yes).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Result_Yes;
 }
 if (i_Message_Result_No > 0)
 {
  if (i_Message_Result_No > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - that produced no results - were completed correctly.")).arg(i_Message_Result_No).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Result_No;
 }
 if (i_Message_Total > 0)
 {
  if (i_Message_Total > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - total")).arg(i_Message_Total).arg(s_Search));
  s_Search="";
 }
 if (i_Rows_Total > 0)
 {
  if (i_Rows_Total > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Query-Row%2 returned.")).arg(i_Message_Total).arg(s_Search));
  s_Search="";
 }
 sa_Message_Text.append(QString(tr("Rows effected : %1 ")).arg(i_Message_Rows_effected));
 for (int i=0; i<sa_Message_Text.count(); i++)
  s_Search+=sa_Message_Text[i]+"\n";
 ui->queryResultText->setPlainText(s_Search);
 sa_Message_Text.clear();
 if (tab_First_Result != 0)
 {
  ui->widget_query_view->setCurrentWidget(tab_First_Result);
 }
 this->setCursor(Qt::ArrowCursor);
 timer->stop();
}

void QueryDialog::quickProcess()
{
 QApplication::processEvents();
}

void QueryDialog::setMaxTabResults(int num)
{
 i_Max_Tab_Results = num;
}

void QueryDialog::on_save_QueryButton_clicked()
{
 QString s_File_Name = QFileDialog::getSaveFileName(this, "Choose the name of a SQL text file to save to",
                       config->q.s_query_path,"SQL text files (*.sql *.txt);;All Files (*.*)");
 if (s_File_Name.isEmpty()) return;
 QFileInfo this_fi(s_File_Name);
 if (this_fi.completeSuffix() == "")
  s_File_Name+=".sql";
 QFile this_file(s_File_Name);
 if (!this_file.open(QIODevice::WriteOnly | QIODevice::Text))
 {
  QMessageBox::critical(this,qApp->applicationName(), "Could not save sql query text file");
  return;
 }

 config->q.s_query_path = this_fi.dir().absolutePath();
 QTextStream out(&this_file);
 out << ui->editQuery->toPlainText();
}

void QueryDialog::slot_QueryView_horizontalHeader_sectionDoubleClicked(int logicalIndex)
{
 Q_UNUSED(logicalIndex);
 QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
 QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
 //QGeomCollQueryModel * model = qobject_cast<QGeomCollQueryModel*>(proxyModel->sourceModel());
 proxyModel->sort(-1, Qt::AscendingOrder);
}

void QueryDialog::slot_queryView_row_DoubleClicked(QModelIndex index)
{
 if(!index.isValid())
  return;
 qint32 currTabIndex = ui->widget_query_view->currentIndex();
 if(currTabIndex<1)
  return;
 // qDebug()<<"current tab #" + QString("%1").arg(currTabIndex);
 // Note:
 // The QModelIndex passed in refers to the model used in the active tab's QtableView.
 // Obtain the view used in the active widget (current active tab), then determine it's sortproxymodel and it's sourcemodel to get the model used for this query.
 QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
 QSortFilterProxyModel *proxyModel =  qobject_cast<QSortFilterProxyModel *>(view->model());
 QueryModel * model =  qobject_cast<QueryModel*>(proxyModel->sourceModel());
 qDebug()<< model->tabName;
 model->edit(proxyModel->mapToSource(index));
}

//create table input context menu
 void QueryDialog::tablev_customContextMenu( const QPoint& pt)
 {
  // check is item in QTableView exist or not
  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
  //QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
  //QueryModel * model = qobject_cast<QueryModel*>(proxyModel->sourceModel());
  //int curRow = view->rowAt(pt.y());
  int curCol = view->columnAt(pt.x());
  currentIndexQueryView = view->indexAt(pt);
  currentColQueryView = curCol;
  if(boolGetItemTableView(view))
  {
   //menu = QMenu(m_parent*);
   QAction * copyAction = new QAction("Copy cell text", this);
   connect(copyAction,SIGNAL(triggered()),this,SLOT(on_copyCellText()));
//   QAction * pasteAction = new QAction("Paste",this);
//   connect(pasteAction,SIGNAL(triggered()),this,SLOT(queryViewPaste()));
//   QAction *copyBlobAct = new QAction(tr("Copy blob to clipboard"),this);
//   connect(copyBlobAct, SIGNAL(triggered()),this,SLOT(on_copyBlob()));

   //QClipboard *clip = QApplication::clipboard();
   QMenu menu;

   menu.addAction(copyAction);
//   menu.addAction(pasteAction);
   // more actions can be added here
//   QSqlRecord this_record = model->record(currentIndexQueryView.row());
//   s_currBlobType = model->table_blobs->on_check_blob_list(model->rowCount(),&this_record,currentIndexQueryView.row(),currentIndexQueryView.column());
//   if(s_currBlobType != "")
//   {
//    qDebug()<<"Is blob "+ s_currBlobType;
//    menu.addAction(copyBlobAct);
//   }
   menu.exec(QCursor::pos());
  }
 }//get QTableView selected item

 bool QueryDialog::boolGetItemTableView(QTableView *view)
 {
  // get model from tableview
  QItemSelectionModel *selModel = view->selectionModel();
  if(selModel)
  {
   currentIndexQueryView = selModel->currentIndex();
   return (true);
  }
  else                //QTableView doesn't have selected data
   return (false);
 }

// void QueryDialog::queryViewHeaderContextMenuRequested(const QPoint &pt)
// {
//  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());

//  QAction *hideColumn = new QAction(tr("Hide Column"),this);
//  QAction *showHiddenColumns = new QAction(tr("Show Hidden Columns"),this);
//  myHeaderView* hv = (myHeaderView*)view->horizontalHeader();
//  QAction *moveOrResize = new QAction(hv->bAllowSortColumns?tr("Allow resize columns"):tr("Allow sort/drag/move columns"),this);
//  QAction *resizeToData = new QAction(tr("Resize to data"), this);
//  connect(hideColumn, SIGNAL(triggered()),this,SLOT(on_queryView_hide_column()));
//  connect(showHiddenColumns,SIGNAL(triggered()),this,SLOT(on_queryView_show_columns()));
//  connect(moveOrResize, SIGNAL(triggered()),this,SLOT(onMoveOrRezize_columns()));
//  connect(resizeToData, SIGNAL(triggered()),this,SLOT(onResizeToData()));

//  QMenu menu;
//  queryViewCurrColumn = view->columnAt(pt.x());
//  menu.addAction(moveOrResize);
//  menu.addAction(resizeToData);
//  menu.addAction(hideColumn);
//  for(int i=0; i < view->model()->columnCount(); i++)
//  {
//   if(view->isColumnHidden(i))
//   {
//    menu.addAction(showHiddenColumns);
//    break;
//   }
//  }
//  menu.exec(QCursor::pos());
// }

// void QueryDialog::on_queryView_hide_column()
// {
//  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
//  view->hideColumn(queryViewCurrColumn);
// }

// void QueryDialog::on_queryView_show_columns()
// {
//  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
//  for(int i=0; i < view->model()->columnCount(); i++)
//  {
//   if(view->isColumnHidden(i))
//   {
//    view->showColumn(i);
//   }
//  }
// }

 void QueryDialog::on_copyCellText()
 {
  qint32 currTabIndex = ui->widget_query_view->currentIndex();
  if(currTabIndex<1)
   return;   // no results present
  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
  QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
  QueryModel * model = qobject_cast<QueryModel*>(proxyModel->sourceModel());
  //QItemSelectionModel *selmodel = view->selectionModel();
  //const QItemSelection &sellist = proxyModel->mapSelectionToSource (selmodel->selection());

  QClipboard *clip = QApplication::clipboard();
  clip->setText(model->data(currentIndexQueryView,Qt::DisplayRole).toString());
 }
// void QueryDialog::onMoveOrRezize_columns()
// {
//  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
//  myHeaderView* hv = (myHeaderView*)view->horizontalHeader();
//  hv->bAllowSortColumns = !hv->bAllowSortColumns;
// }

// void QueryDialog::onResizeToData()
// {
//  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
//  myHeaderView* hv =(myHeaderView*) view->horizontalHeader();
//  hv->resizeSections(QHeaderView::ResizeToContents);
//  QVariantList colList;
//  QVariantList mapList;
//  for(int i=0; i < view->model()->columnCount(); i++)
//  {
//   if(view->isColumnHidden(i))
//    colList << -(view->columnWidth(i));
//   else
//    colList << view->columnWidth(i);
//   mapList << hv->logicalIndex(i);
//  }
// }

 void QueryDialog::On_cbConnections_CurrentIndexChanged(int )
 {
  if(bChanging) return;
  if(ui->cbConnections->currentText() == config->currCity->connections.at(config->currCity->curConnectionId)->description())
  {
   // reverting to default (currrent) database!
   if(db.isOpen() && db.connectionName() == "testConnection")
    db.close();
   db = QSqlDatabase();
   return;
  }

  for(int i=0; i<config->currCity->connections.count(); i++)
  {
   Connection* c = config->currCity->connections.at(i);
   if( c->description() == ui->cbConnections->currentText())
   {
    config->currCity->curExportConnId = c->id();
    config->saveSettings();
    break;
   }
  }
  if(db.isOpen() && db.connectionName() == "testConnection")
   db.close();


  tgtConn = config->currCity->connections.at(config->currCity->curExportConnId);
#if 1
  db = QSqlDatabase::addDatabase(tgtConn->driver(),"testConnection");
  if(tgtConn->driver() == "QODBC" || tgtConn->driver() == "QODBC3")
  {
   db.setHostName(tgtConn->host());
   db.setDatabaseName(tgtConn->dsn());
   tgtDbType = "MsSql";

  }
  else
  {
   db.setHostName(tgtConn->host());
   db.setDatabaseName(tgtConn->database());
   if(tgtConn->driver() == "QSQLITE")
   {
    tgtDbType= "Sqlite";
   }
   else
   {
    tgtDbType = "MySql";
   }
  }

  db.setUserName(tgtConn->uid());
  db.setPassword(tgtConn->pwd());
  if(!db.open())
  {
   ui->go_QueryButton->setEnabled(false);
   qDebug() << "Database not open: " + ui->cbConnections->currentText() + ", current databasename: " + db.databaseName() + " " + db.lastError().text();
   qDebug() << "current dir: " + QDir::currentPath();
  }
  else
  {
   ui->go_QueryButton->setEnabled(true);
   if(tgtConn->driver() == "QODBC" || tgtConn->driver() == "QODBC3" )
   {
    if(tgtConn->useDatabase() != "default" || tgtConn->useDatabase() != "")
    {
     QSqlQuery query = QSqlQuery(db);
     if(!query.exec(tr("use [%1]").arg(tgtConn->useDatabase())))
     {
      SQLERROR(query);
        db.close();
        return;
     }
    }
   }
  }
#endif
   setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));
//   bChanging = true;
//   if(!tgtConn->isOpen())
//    db = tgtConn->configure("testConnection");
//   db = QSqlDatabase::database("testConnection");
//   bChanging = false;
 }
