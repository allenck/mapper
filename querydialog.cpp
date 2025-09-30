#include "querydialog.h"
#include "querymodel.h"
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
#include <QInputMethod>
#include "findreplacewidget.h"

/*static*/ QueryDialog* QueryDialog::instance(){return _instance;}
/*static*/ QueryDialog* QueryDialog::_instance = nullptr;

QueryDialog::QueryDialog(Configuration* cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryDialog)
{
  _instance = this;
  ui->setupUi(this);
  frw = new FindReplaceWidget(ui->editQuery, this);
  ui->query_splitter->insertWidget(1,frw);
  config = cfg;
  config->changeFonts(this, config->font);
  // currQueryFilename = "";
  setTitle();
  bChanging = false;
  tgtConn = config->currCity->connections.at(config->currCity->curConnectionId);
  ui->chkShowOnly->setText(tr("Show only %1 connections").arg(config->currCity->name()));
  connect(ui->chkShowOnly, &QCheckBox::checkStateChanged,this,[=]{
      fill_cbConnections();
  });
  db = QSqlDatabase::database();
  tgtConn->setConnectionName(db.connectionName());
  connect(ui->editQuery, SIGNAL(textChanged()), this, SLOT(textChanged()));

  ui->editQuery->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->editQuery,SIGNAL(customContextMenuRequested(QPoint)),
          this,SLOT(showContextMenu(QPoint)));
  clearAct = new QAction("clear", this);
  connect(clearAct, &QAction::triggered, [=]{
   ui->editQuery->clear();
  });

  // ui->cbConnections->setContextMenuPolicy(Qt::CustomContextMenu);
  // connect(ui->cbConnections, &QComboBox::customContextMenuRequested, this,[=](QPoint pt){
  //     QMenu menu;
  //     QAction* act = new QAction(tr("Refresh connections"),this);
  //     menu.addAction(act);
  //     menu.exec(QCursor::pos());
  //     connect(act, &QAction::triggered,this,[=]{
  //         ui->cbConnections->clear();
  //         for(int i=0; i<config->currCity->connections.count(); i++)
  //         {
  //             Connection* c = config->currCity->connections.at(i);
  //             ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
  //             if(c->id() == config->currConnection->id())
  //                 ui->cbConnections->setCurrentIndex(i);
  //         }
  //     });
  // });
  connect(config->currCity, &City::connectionAdded,this,[=]{
      fill_cbConnections();
  });
  makeSelectedIncludeAct = new QAction(tr("Make include file of selection"), this);
  connect(makeSelectedIncludeAct, &QAction::triggered, [=]{
   makeSelectedInclude();
  });
  replaceWithIncludeAct = new QAction(tr("Replace selection with included file"),this);
  connect(replaceWithIncludeAct, &QAction::triggered, [=]{
   replaceWithInclude();
  });

  ui->widget_query_view->setTabsClosable(true);
  connect(ui->widget_query_view, &QTabWidget::tabCloseRequested, this,[=](int tab){
      ui->widget_query_view->removeTab(tab);
  });

  menuBar = new QMenuBar();
  layout()->setMenuBar(menuBar);

  createMenus();

  tgtDbType = tgtConn->servertype();
  i_Max_Tab_Results = 10;
  ui->cb_stop_query_on_error->setChecked(config->q.b_stop_query_on_error);
  ui->cb_sql_execute_after_loading->setChecked(config->q.b_sql_execute_after_loading);
  restoreGeometry(config->q.geometry);

  fill_cbConnections();
  connect(ui->cbConnections, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cbConnections_CurrentIndexChanged(int)));
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
      // ui->cbConnections->clear();
      // for(int i=0; i<config->currCity->connections.count(); i++)
      // {
      //  Connection* c = config->currCity->connections.at(i);
      //  ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
      //  if(c->id() == config->currConnection->id())
      //   ui->cbConnections->setCurrentIndex(i);

      // }
      ui->chkShowOnly->setText(tr("Show only %1 connections").arg(config->currCity->name()));
      fill_cbConnections();
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

void QueryDialog::fill_cbConnections()
{
    ui->cbConnections->clear();
    for(int i=0; i<config->currCity->connections.count(); i++)
    {
        Connection* c = config->currCity->connections.at(i);
        ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
        if(c->id() == config->currConnection->id())
            ui->cbConnections->setCurrentIndex(i);
    }
    if(ui->chkShowOnly->isChecked())
        return;
    // add other cities as well
    for(City* city : config->cityList)
    {
        if(city->name() == config->currCity->name())
            continue; // aleady got these!
        for(int i=0; i<city->connections.count(); i++)
        {
            Connection* c = city->connections.at(i);
            ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
            if(c->id() == config->currConnection->id())
                ui->cbConnections->setCurrentIndex(i);
        }
    }
    setWindowTitle(tr("Manual Sql Query (%1)").arg(ui->cbConnections->currentText()));
}

QueryDialog::~QueryDialog()
{
 delete ui;
}

void QueryDialog::showContextMenu(const QPoint &pt)
{
    QMenu *menu = ui->editQuery->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(clearAct);
    QString selected;
    QTextCursor cur = ui->editQuery->textCursor();
    if (cur.hasSelection())
    {
     selected = cur.selectedText();
     menu->addAction(makeSelectedIncludeAct);
     menu->addAction(replaceWithIncludeAct);
    }
    menu->exec(ui->editQuery->mapToGlobal(pt));
    delete menu;
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
 setTitle();

 for (int ix =ui->widget_query_view->count()-1; ix > 0; ix--)
  ui->widget_query_view->removeTab(ix);
 ui->queryResultText->clear();
 sa_Message_Text.clear();
 s_Search="";

 saveFileAct->setEnabled(false);
}

void QueryDialog::createMenus()
{
    menuBar->clear();
    toolsMenu = new QMenu(tr("Tools"));

    fileMenu= new Menu(tr("File"));
    menuBar->addMenu(fileMenu);
    selectMenu = new QMenu(tr("Load Resource script"));
    wAct = createWidgetAction();
    this->selectMenu->addAction(wAct);
    this->selectMenu->setStatusTip(tr("load a read-only resource script"));
    fileMenu->addMenu(selectMenu);
    QAction* loadFileAct = new QAction(tr("Load query"), this);
    fileMenu->addAction(loadFileAct);
    connect(loadFileAct, &QAction::triggered, this,[=]{
        on_load_QueryButton_clicked();
    });
    saveFileAct = new QAction(tr("Save query"), this);
    saveFileAct->setEnabled(false);
    fileMenu->addAction(saveFileAct);
    connect(saveFileAct, &QAction::triggered, this,[=]{
      on_save_QueryButton_clicked(currQueryFilename);
    });
    saveAsFileAct = new QAction(tr("Save query as ..."), this);
    fileMenu->addAction(saveAsFileAct);
    connect(saveAsFileAct, &QAction::triggered,this, [=]{
      on_saveAs_QueryButton_clicked();
    });
    refreshRoutesAct = new QAction(tr("Refresh routes"),this);
    refreshRoutesAct->setCheckable(true);
    connect(refreshRoutesAct, &QAction::triggered, this,[=]{
    MainWindow::instance()->refreshRoutes();
    if(refreshRoutesAct->isChecked())
    MainWindow::instance()->btnDisplayRouteClicked();
    });

    menuBar->addMenu(toolsMenu);
    toolsMenu->addAction(refreshRoutesAct);
    //QMenu* tablesMenu = new QMenu(tr("Tables"));
    connect(toolsMenu, &QMenu::aboutToShow, this,[=]{
        Connection* c = tgtConn;
        QSqlDatabase db;
        //if(c->connectionName().isEmpty())
        db = QSqlDatabase::database(tgtConn->connectionName());
        // else
        //  db = QSqlDatabase::database(c->connectionName());
        QStringList tableList = db.tables(QSql::Tables);
        QStringList sysTableList = db.tables(QSql::SystemTables);
        toolsMenu->clear();
        toolsMenu->addAction(refreshRoutesAct);

        QMenu* tablesMenu = new QMenu(tr("Tables"));
        toolsMenu->addMenu(tablesMenu);
        foreach(QString tableName, tableList)
        {
            if(c->servertype()!="PostgreSQL")
            {
             if(sysTableList.contains(tableName))
              continue;
            }
            if(c->servertype() == "Sqlite" && tableName.startsWith("sqlite_"))
             continue;
            QMenu* tableMenu = new QMenu(tableName);
            tablesMenu->addMenu(tableMenu);
            QAction* act = new QAction(tr("show table"),this);
            act->setData(tableName);
            tableMenu->addAction(act);
            connect(act, &QAction::triggered,this, [=]{
                QString txt;
                if(c->servertype() == "Sqlite")
                txt = QString("pragma table_info('%1')").arg(tableName);
                else if(c->servertype() == "MySql")
                txt = "describe " + tableName;
                else if(c->servertype() == "MsSql")// SQL Server
                txt = "select *"
                " from INFORMATION_SCHEMA.COLUMNS"
                " where TABLE_NAME='" + tableName + "'";
                else
                txt = QString("Select * from INFORMATION_SCHEMA.COLUMNS"
                      "where TABLE_NAME='%1'").arg(tableName);

                processALine(txt, tableName);
            });
            act = new QAction(tr("select table"),this);
            act->setData(tableName);
            tableMenu->addAction(act);
            connect(act, &QAction::triggered,this, [=]{
                QString txt = "select * from " + tableName;
                //processALine(txt, tableName);
                processSelect(tableName, txt);
            });
        }
        toolsMenu->addSeparator();
        QMenu* viewsMenu = new QMenu(tr("Views"));
        QStringList views = SQL::instance()->listViews();
        foreach(QString v, views)
        {
            QMenu* viewMenu = new QMenu(v);
            viewsMenu->addMenu(viewMenu);
            QAction* act = new QAction(tr("show View"),this);
            act->setData(v);
            viewMenu->addAction(act);
            connect(act, &QAction::triggered,this, [=]{
                QString txt;
                if(c->servertype() == "Sqlite")
                txt = QString("pragma table_info('%1')").arg(v);
                else if(c->servertype() == "MySql")
                txt = "describe " + v;
                else if(c->servertype() == "PostgreSQL")
                 txt = QString("select column_name, data_type, character_maximum_length, column_default, is_nullable\
                               from INFORMATION_SCHEMA.COLUMNS where table_name = '&1';").arg(v);
                else // SQL Server
                txt = "SELECT * "
                       "   FROM INFORMATION_SCHEMA.COLUMNS"
                       "       WHERE TABLE_NAME = '" + v+ "'";

                processALine(txt, v);
            });
        }
        toolsMenu->addMenu(viewsMenu);
        QAction* act = new QAction(tr("Show find/replace"),this);
        act->setCheckable(true);
        act->setChecked(frw->isVisible());
        connect(act, &QAction::triggered,this,[=](bool bChecked){
           frw->setVisible(bChecked);
        });
        toolsMenu->addAction(act);
    });
}

void QueryDialog::on_load_QueryButton_clicked()
{
    setCursor(Qt::WaitCursor);
    on_clear_QueryButton_clicked();
    QString s_File_Name = QFileDialog::getOpenFileName(this, "Choose a SQL text file",
                       config->q.s_query_path, "SQL text files (*.sql *.txt);;All Files (*.*)");
    // QFileDialog take a long time to close, this should take care of this - but does not.
    QCoreApplication::processEvents();
    setCursor(Qt::ArrowCursor);
    if (s_File_Name.isEmpty()) return;
    loadFile(s_File_Name);
}

void QueryDialog::loadFile(QString s_File_Name)
{
    QFile this_file(s_File_Name);
    QFileInfo this_fi(s_File_Name);

    if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Error"), "Could not load sql query text file");
        return;
    }
    config->q.s_query_path = this_fi.dir().absolutePath();
    currQueryFilename = s_File_Name;
    setTitle();
    saveFileAct->setEnabled(true);

    if(this_fi.size() > 1000000)
    {
        // large file
        QMessageBox msgBox;
        msgBox.setText(tr("The query is very large"));
        msgBox.setInformativeText(tr("The query is too large to display.\n"
                                   "Select 'No' to load it anyway, 'Yes' to process "
                                   "in the background or 'Cancel'.\n"
                                   "Do you wish to process the queries in the background?"));
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
        QTextStream* in = new QTextStream(&this_file);
        this->setCursor(Qt::WaitCursor);
        qApp->processEvents();

       if(!processStream(in))
       {
            return;
       }
        this->setCursor(Qt::ArrowCursor);
        ui->queryResultText->append(QString(tr("Records processed: %1<BR>")).arg(recordsProcessed));
        ui->queryResultText->append(QString(tr("There were errors: %1<BR>")).arg(errors));
        return;
    }
loadIt:
    QTextStream* in = new QTextStream(&this_file);
    //ui->editQuery->setPlainText(in.readAll());
    if(!loadStream(in))
    return;
    if (ui->cb_sql_execute_after_loading->checkState() == Qt::Checked)
    {
    ui->cb_sql_execute_after_loading->setChecked(false);
    on_go_QueryButton_clicked();
    }
}

bool QueryDialog::loadStream(QTextStream* in)
{
 while(!in->atEnd())
 {
  QString line = in->readLine();
  if(line.startsWith("#"))
   handleComment(line);
  else
   ui->editQuery->append(line);
 }
 return true;
}

bool QueryDialog::handleComment(QString line)
{
 ui->editQuery->append(QString("<FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\">").arg(line));
 if(line.startsWith("#include",Qt::CaseInsensitive))
 {
  QString fn = line.mid(8).trimmed();
  QFile this_file(config->q.s_query_path+"/"+fn);
  if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
   QMessageBox::critical(this,tr("Error"), "Could not load sql query text file");
   return false;
  }
  else
  {
   QTextStream* in = new QTextStream(&this_file);
   if(!loadStream(in))
    return false;
   ui->editQuery->append(QString("<FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\">")
                         .arg("#end" + fn ));
  }
 }
 return true;
}

bool QueryDialog::processStream(QTextStream* in)
{
    _delimiter = ";";
 while(!in->atEnd())
 {
  QString queryStr;
  do
  {
   QString line = in->readLine();
   linesRead++;
   if(line.contains("DELIMITER", Qt::CaseInsensitive))
   {
       int ix = line.indexOf("DELIMITER",Qt::CaseInsensitive)+ 9;
       QString delim = line.mid(ix).trimmed();
       _delimiter = delim;
       continue;
   }
   //qDebug()<<line;
   queryStr += line;
  } while (!queryStr.endsWith(_delimiter));
  //qDebug()<<queryStr;
  QSqlQuery query = QSqlQuery(queryStr, db);
  //QStringList sa_Message_Text;
  query.setForwardOnly(false);
  if (query.lastError().isValid())
  {
   errors++;
   QSqlRecord rec = query.record();
   ui->queryResultText->append(QString("<FONT COLOR=\"#FF0000\">%1<BR>%2<FONT COLOR=\"#000000\"><BR>")
                               .arg(query.lastError().driverText(),query.lastError().databaseText())+"<BR>");
   ui->queryResultText->append(QString(tr("Line %1 ")).arg(linesRead--)+" " +query.lastQuery()+"<BR>");
   if(ui->cb_stop_query_on_error->isChecked())
   {
    ui->queryResultText->append(tr("Query stopped because of errors<BR>"));
    this->setCursor(Qt::ArrowCursor);
    return false;
   }
  }
  recordsProcessed++;
  if(recordsProcessed%1000 == 0)
   ui->queryResultText->append(QString(tr("Records processed: %1<BR>")).arg(recordsProcessed));

  qApp->processEvents();
 }
 return true;
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
        //text = cur.selection().toRawText();
    }
    else
        text = ui->editQuery->toPlainText(); // select all lines

    QStringList lines = text.split("\n");
    QString combined;
    _delimiter = ";";
    delimiters.push(_delimiter);
    QStringList statements;

    foreach(QString line, lines)
    {
        line = line.replace(QChar(8233)," ");
        if(line.startsWith("#"))
            continue;
        if(line.contains("DELIMITER", Qt::CaseInsensitive))
        {
            int ix = line.indexOf("DELIMITER",Qt::CaseInsensitive)+ 9;
            QString delim = line.mid(ix).trimmed();
            _delimiter = delim;
            delimiters.push(_delimiter);
            continue;
        }
        if(line.contains(_delimiter))
        {
            combined.append(line);
            // if(delimiters.count(0) > 1)
            //     combined.replace(_delimiter,"delimiters.at(1)");
            delimiters.pop();
            _delimiter = delimiters.top();
            if(combined.endsWith(_delimiter))
            {
                statements.append(combined);
                combined.clear();
            }
            continue;
        }
        if(line.contains("$"))
        {
            int ix = line.indexOf("$");
            QString delim = "$";
            int i =ix+1;
            while (i < line.length())
            {
                delim.append(line.at(i));
                if(line.at(i)== "$")
                {
                    _delimiter = delim;
                    delimiters.push(_delimiter);
                    break;
                }
                i++;
            }
        }
        combined.append(line + " ");
    }
    // if(delimiters.count()>1)
    // {
    //     processALine(combined);
    // }
    //else
    {
        QStringList viewList = SQL::instance()->listViews();
        foreach(QString txt, statements)
        {
            if (txt.trimmed().isEmpty())
                continue;
            QStringList tokens = txt.split(" ");
            if(tokens.count() >= 4
                 && tokens.at(0).compare("select", Qt::CaseInsensitive)  == 0
                 && tokens.at(1) == "*"
                 && tokens.at(2).compare("from", Qt::CaseInsensitive) == 0
                 && !viewList.contains(tokens.at(3),Qt::CaseInsensitive) && db.isValid())
            {
                processSelect(tokens.at(3), txt);
            }
            else
            {
                sa_Message_Text.append("<I>"+txt+ "</I><BR>");

                // if(!processALine(txt))
                //     break;
                QSqlQuery query = QSqlQuery(db);
                bool bQuery = query.exec(txt);
                if(!bQuery)
                {
                    sa_Message_Text.append(QString("<FONT COLOR=\"#FF0000\">%1<BR>%2<FONT COLOR=\"#000000\"></BR>").arg(query.lastError().driverText(),
                                                                                                                       query.lastError().databaseText()));
                    sa_Message_Text.append(txt);
                    i_Message_Error++;
                }
                else
                {
                    sa_Message_Text.append("<BR>Success!</BR>");
                    i_Rows_Total += 1;
                    i_Message_Result_Yes++;
                }
            }
            qApp->processEvents();
        }
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
        QString was_were = "was";
        if (i_Message_Result_Yes > 1)
        {
            s_Search="s";
            was_were = "were";
        }
        sa_Message_Text.append(QString(tr("%1 Statement%2 - that produced results - %3 completed correctly.<BR>")).arg(i_Message_Result_Yes).arg(s_Search,was_were));
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
            sa_Message_Text.append(tr("<BR>Query stopped because of errors<BR>"));
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
                MyHeaderView *hv = new MyHeaderView(Qt::Horizontal, query_view);
                query_view->setHorizontalHeader(hv);
                //     query_view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
                //     connect(query_view->horizontalHeader(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(queryViewHeaderContextMenuRequested(const QPoint)));
                //     query_view->horizontalHeader()->setMovable(true);

                query_view->setSortingEnabled(false);
                QSortFilterProxyModel *sm = new QSortFilterProxyModel(query_view);
                sm->setSourceModel(query_view_model);
                query_view->setModel(sm);
                query_view->setSortingEnabled(true);
                query_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
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
    _delimiter = ";";
 //QSqlDatabase db = QSqlDatabase::database();
 QueryEditModel* model = new QueryEditModel(nullptr, db);
 model->setTable(table);
 QString whereClause;
 if(commandLine.contains("where", Qt::CaseInsensitive))
 {
  whereClause = commandLine.mid(commandLine.indexOf("where", Qt::CaseInsensitive)+5).trimmed();
  int ix;
  if((ix = whereClause.indexOf(_delimiter)) >= 0)
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
  MyHeaderView *hv = new MyHeaderView(Qt::Horizontal, query_view);
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
 currQueryFilename = s_File_Name;
 saveFileAct->setEnabled(true);
 setTitle();
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
 //out << ui->editQuery->toPlainText();
 QString text; //= ui->editQuery->toPlainText();
 QTextCursor cur = ui->editQuery->textCursor();
 if (cur.hasSelection())
 {
     text = cur.selectedText();
     //text = cur.selection().toRawText();
 }
 else
     text = ui->editQuery->toPlainText(); // select all lines
 QStringList sl = text.split("\n");
 for(int i=0; i < sl.count(); i++)
 {
  QString s = sl.at(i);
  if(s.startsWith("#include", Qt::CaseInsensitive))
  {
   out<< s << "\n";
   i = handleOutFile(sl,i);
   continue;
  }
  out << s<< "\n";
 }
}

int QueryDialog::handleOutFile(QStringList sl, int i)
{
 QString line = sl.at(i);
 if(line.startsWith("#include",Qt::CaseInsensitive))
 {
  QString fn = line.mid(8).trimmed();
  QFile this_file(config->q.s_query_path+"/"+fn);
  if (!this_file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
   QMessageBox::critical(this,tr("Error"), "Could not open sql query text file");
   return false;
  }
  else
  {
   QTextStream out(&this_file);
   for(int j=i+1; j < sl.count(); j++)
   {
    QString line = sl.at(j);
    if(line.startsWith("#end"))
     return j;
    out << line<< "\n";
   }
  }
 }
 throw Exception();
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
   // QAction * pasteAction = new QAction("Paste",this);
   // connect(pasteAction,SIGNAL(triggered()),this,SLOT(queryViewPaste()));
//   QAction *copyBlobAct = new QAction(tr("Copy blob to clipboard"),this);
//   connect(copyBlobAct, SIGNAL(triggered()),this,SLOT(on_copyBlob()));
   QAction* insertAction = new QAction(tr("insert row"),this);
   connect(insertAction, &QAction::triggered, this,[=]{
    on_insertRow();
   });
   QAction* deleteAction = new QAction(tr("delete row"),this);
   connect(deleteAction, &QAction::triggered, this, [=]{
    on_deleteRow();
   });
   QAction* sortAction = new QAction(tr("sort on column"),this);
   connect(sortAction, &QAction::triggered, this, [=]{
       on_sortAction();
   });
   //QClipboard *clip = QApplication::clipboard();
   QMenu menu;

   menu.addAction(copyAction);
   // more actions can be added here
//   QSqlRecord this_record = model->record(currentIndexQueryView.row());
//   s_currBlobType = model->table_blobs->on_check_blob_list(model->rowCount(),&this_record,currentIndexQueryView.row(),currentIndexQueryView.column());
//   if(s_currBlobType != "")
//   {
//    qDebug()<<"Is blob "+ s_currBlobType;
//    menu.addAction(copyBlobAct);
//   }
   menu.addAction(sortAction);

   QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
   QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
   if(qobject_cast<QueryEditModel*>(proxyModel->sourceModel()))
   {
    QueryEditModel * model = qobject_cast<QueryEditModel*>(proxyModel->sourceModel());
    if(model)
    {
     //menu.addAction(pasteAction);
     menu.addAction(insertAction);
     menu.addAction(deleteAction);
    }
   }
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


 void QueryDialog::on_copyCellText()
 {
  qint32 currTabIndex = ui->widget_query_view->currentIndex();
  if(currTabIndex<1)
   return;   // no results present
  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
  view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
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
 void QueryDialog::on_sortAction()
 {
     qint32 currTabIndex = ui->widget_query_view->currentIndex();
     if(currTabIndex<1)
         return;   // no results present
     QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
     view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
     QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());

     ((QSqlTableModel*)proxyModel->sourceModel())->setSort(currentIndexQueryView.column(),Qt::SortOrder::AscendingOrder);
     ((QSqlTableModel*)proxyModel->sourceModel())->select();
 }



 void QueryDialog::on_deleteRow()
 {
  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
  QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
  if(qobject_cast<QueryEditModel*>(proxyModel->sourceModel()))
  {
   QueryEditModel * model = qobject_cast<QueryEditModel*>(proxyModel->sourceModel());
   int row = proxyModel->mapToSource(currentIndexQueryView).row();
   model->removeRow(row);
  }
 }

 void QueryDialog::on_insertRow()
 {
  QTableView *view = qobject_cast<QTableView*>(ui->widget_query_view->currentWidget());
  QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
  if(qobject_cast<QueryEditModel*>(proxyModel->sourceModel()))
  {
   QueryEditModel * model = qobject_cast<QueryEditModel*>(proxyModel->sourceModel());
   int row = currentIndexQueryView.row();
   model->insertRow(row);
  }
 }

 void QueryDialog::on_cbConnections_CurrentIndexChanged(int ix)
 {
  if(bChanging) return;
  setCursor(Qt::WaitCursor);
  Connection* tgt = VPtr<Connection>::asPtr(ui->cbConnections->itemData(ix));
  if(tgt== nullptr)
      return;
  this->tgtConn = tgt;
  if(ui->cbConnections->currentText() == config->currCity->connections.at(config->currCity->curConnectionId)->description())
  {
   // reverting to default (currrent) database!
   //
   // the docs recommend not setting QSqlDatabase as a member of a class but since
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

#if 1
  if(tgtConn->connectionName().isEmpty())
  {
      if(tgtConn->getDb().isOpen())
          db = tgtConn->getDb();
      else
      {
          db = QSqlDatabase::addDatabase(tgtConn->driver(), QString("query-%1").arg(tgtConn->description()));
          tgtConn->setConnectionName(db.connectionName());
          tgtConn->setDb(db);
          Connection::configureDb(db, tgtConn, config);
      }
//   if(tgtConn->servertype() == "Sqlite")
//   {
//    if(!tgtConn->isSqliteUserFunctionLoaded())
//     tgtConn->setSqliteUserFunctionLoaded(SQL::instance()->
//             loadSqlite3Functions(db));
//   }
   bool bOpen = db.isOpen();
   if(!bOpen)
   {
       if(tgtConn->connectionType() == "ODBC" && !tgtConn->connectString().isEmpty())
           bOpen = db.open();
       else
           bOpen = db.open(tgtConn->userId(), tgtConn->pwd());
   }
   if(!bOpen)
   {
    ui->go_QueryButton->setEnabled(false);
    qDebug() << "Database not open: " + ui->cbConnections->currentText() + ", current databasename: " + db.databaseName() + " " + db.lastError().text();
    qDebug() << "current dir: " + QDir::currentPath();
    return;
   }
   else
   {
    ui->go_QueryButton->setEnabled(true);
    createMenus();
    if(tgtConn->servertype() == "Sqlite")
    {
#ifndef NO_UDF
     if(!tgtConn->isSqliteUserFunctionLoaded())
      tgtConn->setSqliteUserFunctionLoaded( SQL::instance()->loadSqlite3Functions(db));
#endif
    }
    else if(tgtConn->servertype() == "MsSql")
    {
#if 1
        if(tgtConn->database() != "default" || tgtConn->database() != "")
        {
         QSqlQuery query = QSqlQuery(db);
            query.setForwardOnly(false);
         if(!query.exec(tr("use [%1]").arg(tgtConn->database())))
         {
          SQLERROR(std::move(query));
            db.close();
            return;
         }
        }
#endif
    }
   }
#endif
   setTitle();
  }
  else
  {
    db = tgtConn->getDb();
   qDebug() << "connection name:" << tgtConn->connectionName();
   qDebug() << "driver:" << db.driverName() << " database:" << db.databaseName();
   if(db.isOpen())
   {
       ui->go_QueryButton->setEnabled(true);
   }
  }
  wAct = createWidgetAction();
  setCursor(Qt::ArrowCursor);
}

void QueryDialog::setTitle()
{
    Connection c = VPtr<Connection>::asPtr(ui->cbConnections->currentData());
    QString database = SQL::instance()->getDatabase(c.servertype());
    if(currQueryFilename.isEmpty())
        QWidget::setWindowTitle(tr("Manual Sql Query (%1) %2")
                          .arg(ui->cbConnections->currentText(),database));
    else
    {
        QFileInfo info(currQueryFilename);
        QWidget::setWindowTitle(tr("Manual Sql Query (%1) - %2 %3")
                              .arg(ui->cbConnections->currentText(),info.fileName(),database));
    }
}

void QueryDialog::executeQuery(QString commandText)
{
    ui->editQuery->clear();
    ui->editQuery->setText(commandText);
    on_go_QueryButton_clicked();
}

QTextLine QueryDialog::currentTextLine(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    if (!block.isValid())
     return QTextLine();

    const QTextLayout *layout = block.layout();
    if (!layout)
     return QTextLine();

    const int relativePos = cursor.position() - block.position();
    return layout->lineForTextPosition(relativePos);
}

void QueryDialog::textChanged()
{
    QTextCursor cursor = ui->editQuery->textCursor();
    QTextBlock block = cursor.block();
    QString text = block.text();
    QTextLine textLine = currentTextLine(cursor);
}

void QueryDialog::replaceWithInclude()
{
    setCursor(Qt::WaitCursor);
    QString s_File_Name = QFileDialog::getOpenFileName(this, "Choose a SQL text file",
                        config->q.s_query_path, "SQL text files (*.sql *.txt);;All Files (*.*)");
    // QFileDialog take a long time to close, this should tak care of this - but does not.
    QCoreApplication::processEvents();
    setCursor(Qt::ArrowCursor);
    if (s_File_Name.isEmpty()) return;
    QFile this_file(s_File_Name);
    QFileInfo this_fi(s_File_Name);
    if (!this_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    QMessageBox::critical(this,tr("Error"), "Could not load sql query text file");
    return;
    }

    QTextCursor cur = ui->editQuery->textCursor();
    QTextStream in(&this_file);
    QString newText = in.readAll();
    cur.removeSelectedText();
    cur.insertHtml(QString("<FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\"><BR>")
                 .arg("#include " + s_File_Name ));
    cur.insertText(newText);
    cur.insertHtml(QString("<FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\">")
                 .arg("#end" + s_File_Name ));
}

void QueryDialog::makeSelectedInclude()
{
    QTextCursor cur = ui->editQuery->textCursor();
    QString text = cur.selectedText();
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

    //config->q.s_query_path = this_fi.dir().absolutePath();
    QTextStream out(&this_file);
    out << text;
    cur.removeSelectedText();
    cur.removeSelectedText();
    cur.insertHtml(QString("<FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\"><BR>")
                 .arg("#include " + s_File_Name ));
    cur.insertText(text);
    cur.insertHtml(QString("<BR><FONT COLOR=\"#A0A0A0\">%1<FONT COLOR=\"#000000\">")
                 .arg("#end" + s_File_Name ));
}

QWidgetAction* QueryDialog::createWidgetAction()
{
    QString dirName = tgtConn->servertype();
    if(dirName == "Sqlite")
     dirName = "Sqlite3";
    cbFile = new QComboBox(this);
    QDirIterator dirIterator(":/sql/" + dirName, QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        dirIterator.next();
        QFileInfo fileInfo = dirIterator.fileInfo();
        if (fileInfo.isFile())
        {
            QStringList parts = fileInfo.path().split("/");
            if(parts.count() == 2 || (parts.count() == 3 && parts.at(2)== dirName))
             // Do not add directories to the list
                cbFile->addItem(fileInfo.fileName(), fileInfo.filePath());
        }
    }
    cbFile->setVisible(true);
    cbFile->activateWindow();
    cbFile->model()->sort(0, Qt::AscendingOrder);

    connect(cbFile, &QComboBox::currentIndexChanged,this, [=](int ix){
        ui->editQuery->clear();
        loadFile(cbFile->itemData(ix).toString());
        qInfo() << "file selection changed:" << ix << " " << cbFile->currentText();
        selectMenu->close();
    });
    wAct = new QWidgetAction(this);
    wAct->setDefaultWidget(cbFile);
    return wAct;
}

void QueryDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_F)
    {
         QTextCursor cursor = ui->editQuery->textCursor();
         QString selection = cursor.selectedText();
         frw->setTextSelection(selection);
    }
    QWidget::keyPressEvent(event);
}
