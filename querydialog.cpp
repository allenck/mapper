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
#include "vptr.h"
#include "mainwindow.h"
#include "queryeditmodel.h"

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
  tgtConn->setConnectionName(db.connectionName());

  menuBar = new QMenuBar();
  toolsMenu = new QMenu(tr("Tools"));
  layout()->setMenuBar(menuBar);

  QMenu* fileMenu= new Menu(tr("File"));
  menuBar->addMenu(fileMenu);
  QAction* loadFileAct = new QAction(tr("Load query"), this);
  fileMenu->addAction(loadFileAct);
  connect(loadFileAct, &QAction::triggered, [=]{on_load_QueryButton_clicked();});
  saveFileAct = new QAction(tr("Save query"), this);
  saveFileAct->setEnabled(false);
  fileMenu->addAction(saveFileAct);
  connect(saveFileAct, &QAction::triggered, [=]{on_save_QueryButton_clicked(currQueryFilename);});
  saveAsFileAct = new QAction(tr("Save query as ..."), this);
  fileMenu->addAction(saveAsFileAct);
  connect(saveAsFileAct, &QAction::triggered, [=]{on_saveAs_QueryButton_clicked();});

  menuBar->addMenu(toolsMenu);
  //QMenu* tablesMenu = new QMenu(tr("Tables"));
  connect(toolsMenu, &QMenu::aboutToShow, [=]{
   Connection* c = VPtr<Connection>::asPtr(ui->cbConnections->currentData());
   QSqlDatabase db = QSqlDatabase::database(c->connectionName());
   QStringList tableList = db.tables(QSql::Tables);
   QStringList sysTableList = db.tables(QSql::SystemTables);
   toolsMenu->clear();
   QMenu* tablesMenu = new QMenu(tr("Tables"));
   toolsMenu->addMenu(tablesMenu);
   foreach(QString tableName, tableList)
   {
    if(sysTableList.contains(tableName))
     continue;
    if(c->servertype() == "Sqlite" && tableName.startsWith("sqlite_"))
     continue;
    QMenu* tableMenu = new QMenu(tableName);
    tablesMenu->addMenu(tableMenu);
    QAction* act = new QAction(tr("show table"),this);
    act->setData(tableName);
    tableMenu->addAction(act);
    connect(act, &QAction::triggered, [=]{
     QString txt;
     if(c->servertype() == "Sqlite")
      txt = QString("pragma table_info('%1')").arg(tableName);
     else if(c->servertype() == "MySql")
      txt = "describe " + tableName;
     else // SQL Server
      txt = "EXEC sp_help " + tableName;
     processALine(txt, tableName);
    });
   }
  });

  tgtDbType = tgtConn->servertype();
  i_Max_Tab_Results = 10;
  ui->cb_stop_query_on_error->setChecked(config->q.b_stop_query_on_error);
  ui->cb_sql_execute_after_loading->setChecked(config->q.b_sql_execute_after_loading);
  restoreGeometry(config->q.geometry);

  ui->cbConnections->clear();
  for(int i=0; i<config->currCity->connections.count(); i++)
  {
   Connection* c = config->currCity->connections.at(i);
   ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
   if(c->id() == config->currConnection->id())
    ui->cbConnections->setCurrentIndex(i);

  }
  setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));

//  for(int i=0; i<config->currCity->connections.count(); i++)
//  {
//   Connection* c = config->currCity->connections.at(i);
//   if(c->id() == config->currCity->curConnectionId)
//   {
//    ui->cbConnections->setCurrentIndex(i);
//    break;
//   }
//  }
  connect(ui->cbConnections, SIGNAL(currentIndexChanged(int)), this, SLOT(On_cbConnections_CurrentIndexChanged(int)));
  connect(ui->rollback_toolButton, &QToolButton::clicked, [=]{
   try{
   SQL::instance()->rollbackTransaction("");
   }
   catch(Exception e)
   {

   }
   ui->rollback_toolButton->setEnabled(SQL::instance()->isTransactionActive());

  });
  connect(MainWindow::instance(), &MainWindow::newCitySelected, [=]
  {
      bChanging = true;
      ui->cbConnections->clear();
      for(int i=0; i<config->currCity->connections.count(); i++)
      {
       Connection* c = config->currCity->connections.at(i);
       ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
       if(c->id() == config->currConnection->id())
        ui->cbConnections->setCurrentIndex(i);

      }
      bChanging = false;
      setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));
  });
  connect(ui->cbEditStrategy, &QComboBox::currentTextChanged, [=]{
   if(ui->cbEditStrategy->currentIndex() ==2)
    ui->pbSubmit->setVisible(true);
   else
    ui->pbSubmit->setVisible(false);
  });
  ui->rollback_toolButton->setEnabled(SQL::instance()->isTransactionActive());

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

 for (int ix =ui->widget_query_view->count()-1; ix > 0; ix--)
  ui->widget_query_view->removeTab(ix);
 ui->queryResultText->clear();
 sa_Message_Text.clear();
 s_Search="";

 currQueryFilename = "";
 saveFileAct->setEnabled(false);

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
 currQueryFilename = s_File_Name;
 if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
 {
  QMessageBox::critical(this,tr("Error"), "Could not load sql query text file");
  return;
 }
 config->q.s_query_path = this_fi.dir().absolutePath();
 currQueryFilename = s_File_Name;
 saveFileAct->setEnabled(true);

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
    ui->queryResultText->append(QString("<FONT COLOR=\"#FF0000\">%1<BR>%2<FONT COLOR=\"#000000\"><BR>")
                                .arg(query.lastError().driverText(),query.lastError().databaseText())+"<BR>");
    ui->queryResultText->append(QString(tr("Line %1 ")).arg(linesRead--)+" " +query.lastQuery()+"<BR>");
    if(ui->cb_stop_query_on_error->isChecked())
    {
     ui->queryResultText->append(tr("Query stopped because of errors<BR>"));
     this->setCursor(Qt::ArrowCursor);
     return;
    }
   }
   recordsProcessed++;
   if(recordsProcessed%1000 == 0)
    ui->queryResultText->append(QString(tr("Records processed: %1<BR>")).arg(recordsProcessed));

   qApp->processEvents();
  } // !while.atEnd()
  this->setCursor(Qt::ArrowCursor);
  ui->queryResultText->append(QString(tr("Records processed: %1<BR>")).arg(recordsProcessed));
  ui->queryResultText->append(QString(tr("There were errors: %1<BR>")).arg(errors));
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
 i_Message_Error=0;
 i_Message_Result_Yes=0;
 i_Message_Result_No=0;
 i_Message_Rows_effected=0;
 i_Message_Total=0;
 i_Rows_Total=0;
 sa_Message_Text.clear();

 // remove all but the first tab.
 for (int ix =ui->widget_query_view->count()-1; ix > 0; ix--)
  ui->widget_query_view->removeTab(ix);
 ui->queryResultText->clear();

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
// else
//  text = ui->editQuery->toPlainText();
 QStringList list = text.split(";\n");
 foreach(QString txt, list)
 {
  if (txt.trimmed().isEmpty())
   continue;
  QStringList tokens = txt.split(" ");
  if(tokens.count() >= 4
     && tokens.at(0).compare("select", Qt::CaseInsensitive)  == 0
     && tokens.at(1) == "*"
     && tokens.at(2).compare("from", Qt::CaseInsensitive) == 0
     )
  {
   processSelect(tokens.at(3), txt);
  }
  else
  {

   sa_Message_Text.append("<I>"+txt+ "</I><BR>");

   if(!processALine(txt))
    break;
  }
  qApp->processEvents();
 }
 s_Search="";
 if (i_Message_Error > 0)
 {
  if (i_Message_Error > 1)
   s_Search="s";
  sa_Message_Text.append(QString("%1 Statement%2 - that produced errors<BR>").arg(i_Message_Error).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Error;
 }
 if (i_Message_Result_Yes > 0)
 {
  if (i_Message_Result_Yes > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - that produced results - were completed correctly.<BR>")).arg(i_Message_Result_Yes).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Result_Yes;
 }
 if (i_Message_Result_No > 0)
 {
  if (i_Message_Result_No > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - that produced no results - were completed correctly.<BR>")).arg(i_Message_Result_No).arg(s_Search));
  s_Search="";
  i_Message_Total+=i_Message_Result_No;
 }
 if (i_Message_Total > 0)
 {
  if (i_Message_Total > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Statement%2 - total<BR>")).arg(i_Message_Total).arg(s_Search));
  s_Search="";
 }
 if (i_Rows_Total > 0)
 {
  if (i_Rows_Total > 1)
   s_Search="s";
  sa_Message_Text.append(QString(tr("%1 Query-Row%2 returned.<BR>")).arg(i_Rows_Total).arg(s_Search));
  s_Search="";
 }
 //sa_Message_Text.append(QString(tr("Rows affected : %1 <BR>")).arg(i_Message_Rows_effected));
 for (int i=0; i<sa_Message_Text.count(); i++)
  s_Search+=sa_Message_Text[i]+"\n";
 ui->queryResultText->setText(s_Search);
 sa_Message_Text.clear();
 if (tab_First_Result != 0)
 {
  ui->widget_query_view->setCurrentWidget(tab_First_Result);
 }
 this->setCursor(Qt::ArrowCursor);
 timer->stop();
 ui->rollback_toolButton->setEnabled(SQL::instance()->isTransactionActive());
}

bool QueryDialog::processALine(QString txt, QString tabName)
{
 QueryModel *query_view_model = new QueryModel(this ,db, config->currConnection->servertype());
 //queryModelList.append(query_view_modell);
 query_view_model->setQuery(txt, db);
 if (query_view_model->lastError().isValid())
 {
  //query_View->hide();
  //queryResultText->show();
  sa_Message_Text.append(QString("<FONT COLOR=\"#FF0000\">%1<BR>%2<FONT COLOR=\"#000000\"><BR>").arg(query_view_model->lastError().driverText(),
                         query_view_model->lastError().databaseText()));
  sa_Message_Text.append(txt);
  i_Message_Error++;
  if(ui->cb_stop_query_on_error->isChecked())
  {
   sa_Message_Text.append(tr("Query stopped because of errors<BR>"));
   for (int i=0; i<sa_Message_Text.count(); i++)
    s_Search+=sa_Message_Text[i];
   ui->queryResultText->setText(s_Search);
   return false;
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
    QString tabTitle;
    if(!tabName.isEmpty())
     tabTitle = tabName;
    else
     tabTitle = tr("Result ") + QString("%1").arg(ui->widget_query_view->count());
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
   sa_Message_Text.append(QString("%1 rows affected.<BR>").arg(query_view_model->query().numRowsAffected()));
   i_Message_Rows_effected+=query_view_model->query().numRowsAffected();
   i_Message_Result_No++;
  }
 }
 return true;
}

void QueryDialog::processSelect(QString table, QString commandLine)
{
 QWidget *tab_First_Result=0;

 QSqlDatabase db = QSqlDatabase::database();
 QueryEditModel* model = new QueryEditModel(nullptr, db);
 model->setTable(table);
 QString whereClause;
 if(commandLine.contains("where", Qt::CaseInsensitive))
 {
  whereClause = commandLine.mid(commandLine.indexOf("where", Qt::CaseInsensitive)+5).trimmed();
  int ix;
  if((ix = whereClause.indexOf(";")) >= 0)
   whereClause = whereClause.mid(0,ix);
  if((ix = whereClause.indexOf("order by", Qt::CaseInsensitive))>= 0)
   whereClause = whereClause.mid(0,ix);
  if(!whereClause.isEmpty())
   model->setFilter(whereClause);
 }
 bool rslt = model->select();
 if(rslt)
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
  query_view->setSortingEnabled(false);
  QSortFilterProxyModel *sm = new QSortFilterProxyModel(query_view);
  sm->setSourceModel(model);
  model->setEditStrategy((QueryEditModel::EditStrategy)ui->cbEditStrategy->currentIndex());
  query_view->setModel(sm);
  query_view->setSortingEnabled(true);
  sm->sort(-1,Qt::AscendingOrder);
  QString tabTitle = tr("Result ") + QString("%1").arg(ui->widget_query_view->count());
  if (tab_First_Result == 0)
  {
   tab_First_Result = query_view;
  }
  int i_tab = ui->widget_query_view->addTab(query_view, tabTitle);
  //qDebug() << QString("tab %1 added").arg(i_tab);
  model->setTabName(tabTitle);
  query_view->resizeColumnsToContents();
  query_view->resizeRowsToContents();
  i_Rows_Total += model->rowCount();

  connect(ui->pbSubmit, &QPushButton::clicked, [=] {
   model->submit();
  });
  ui->pbSubmit->setVisible(false);
 }
 else
 {
  QSqlError err = model->lastError();
  if (err.isValid())
  {
   //query_View->hide();
   //queryResultText->show();
   sa_Message_Text.append(QString("%1\n%2").arg(model->lastError().driverText()).arg(model->lastError().databaseText()));
   sa_Message_Text.append(commandLine);
   i_Message_Error++;
   if(ui->cb_stop_query_on_error->isChecked())
   {
    sa_Message_Text.append(tr("Query stopped because of errors"));
    for (int i=0; i<sa_Message_Text.count(); i++)
     s_Search+=sa_Message_Text[i]+"\n";
    ui->queryResultText->setText(s_Search);
    return;
   }
  }
 }
}

void QueryDialog::quickProcess()
{
 QApplication::processEvents();
}

void QueryDialog::setMaxTabResults(int num)
{
 i_Max_Tab_Results = num;
}

void QueryDialog::on_saveAs_QueryButton_clicked()
{
 QString s_File_Name = QFileDialog::getSaveFileName(this, "Choose the name of a SQL text file to save to",
                       config->q.s_query_path,"SQL text files (*.sql *.txt);;All Files (*.*)");
 if (s_File_Name.isEmpty()) return;
 saveFile(s_File_Name);
}

void QueryDialog::on_save_QueryButton_clicked(QString s_File_Name)
{
 saveFile(currQueryFilename);
}

void QueryDialog::saveFile(QString s_File_Name)
{
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
 if(qobject_cast<QueryModel*>(proxyModel->sourceModel()))
 {
  QueryModel * model =  qobject_cast<QueryModel*>(proxyModel->sourceModel());
  qDebug()<< model->tabName;
  model->edit(proxyModel->mapToSource(index));
 }
 else
 {
  QueryEditModel * model =  qobject_cast<QueryEditModel*>(proxyModel->sourceModel());
  qDebug()<< model->tabName;
  model->edit(proxyModel->mapToSource(index));
 }
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
  if(qobject_cast<QueryModel*>(proxyModel->sourceModel()))
  {
   QueryModel * model = qobject_cast<QueryModel*>(proxyModel->sourceModel());
   //QItemSelectionModel *selmodel = view->selectionModel();
   //const QItemSelection &sellist = proxyModel->mapSelectionToSource (selmodel->selection());

   QClipboard *clip = QApplication::clipboard();
   clip->setText(model->data(currentIndexQueryView,Qt::DisplayRole).toString());
  }
  else
  {
   QueryEditModel * model = qobject_cast<QueryEditModel*>(proxyModel->sourceModel());
   QClipboard *clip = QApplication::clipboard();
   clip->setText(model->data(currentIndexQueryView,Qt::DisplayRole).toString());

  }
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

 void QueryDialog::On_cbConnections_CurrentIndexChanged(int ix)
 {
  if(bChanging) return;
  Connection* tgtConn = VPtr<Connection>::asPtr(ui->cbConnections->itemData(ix));
  if(ui->cbConnections->currentText() == config->currCity->connections.at(config->currCity->curConnectionId)->description())
  {
   // reverting to default (currrent) database!
   //
   // the docs recommed not setting QSqlDatabase as a member of a class but since
   // this class is always present once created there should be no problem.
   db = QSqlDatabase::database(tgtConn->connectionName()); // restore default connection
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


//  if(db.isOpen() && db.connectionName() == "testConnection")
//   db.close();

//   db=QSqlDatabase::addDatabase(tgtConn->driver(), "query");

   //tgtConn = config->currCity->connections.at(config->currCity->curExportConnId);
  //tgtConn = config->currCity->connections.at(ui->cbConnections->itemData(ix).toInt());
#if 1
  //if(tgtConn->connectionName().isEmpty())
  {
   db = QSqlDatabase::addDatabase(tgtConn->driver(), QString("query%1").arg(ix));
   tgtConn->setConnectionName(db.connectionName());
   Connection::configureDb(&db, tgtConn);
  if(!db.open())
  {
   ui->go_QueryButton->setEnabled(false);
   qDebug() << "Database not open: " + ui->cbConnections->currentText() + ", current databasename: " + db.databaseName() + " " + db.lastError().text();
   qDebug() << "current dir: " + QDir::currentPath();
  }
  else
  {
   ui->go_QueryButton->setEnabled(true);
   if(tgtDbType == "MsSql")
   {
#if 0
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
#endif
   }
  }
#endif
  QWidget::setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));
 }
}

 void QueryDialog::executeQuery(QString commandText)
 {
  ui->editQuery->clear();
  ui->editQuery->setText(commandText);
  on_go_QueryButton_clicked();
 }
