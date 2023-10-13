#include "exportdlg.h"
#include "ui_exportdlg.h"
#include <QCloseEvent>
#include "vptr.h"

ExportDlg::ExportDlg(Configuration *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDlg)
{
 ui->setupUi(this);
 this->setWindowTitle(tr("Export Database"));
 config = cfg;

 timer = new QTimer(this);
 timer->setInterval(1000);
 connect(timer, SIGNAL(timeout()), this, SLOT(quickProcess()));
 timer->start();

 connect(ui->chkAll, SIGNAL(clicked(bool)), this, SLOT(chkAll_changed(bool)));
 connect(ui->btnGo, SIGNAL(clicked()), this, SLOT(btnGo_clicked()));
 connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(btnCancel_clicked()) );
 connect(ui->btnFinish, &QPushButton::clicked, [=]{
     timer->stop();
     close();
 });
 ui->progressBar->setMinimum(0);
 ui->progressBar->setMaximum(100);
 ui->progressBar->setValue(0);
 ui->chkOverride->setChecked(false);
 ui->exportDate->setVisible(false);
 connect(ui->chkOverride, SIGNAL(clicked(bool)),this, SLOT(chkOverrideToggled(bool)));
 //connect(ui->chkAltRoute, SIGNAL(toggled(bool)), this, SLOT(on_chkAltRoute_toggled(bool)));


 ui->cbConnections->clear();
 for(int i=0; i<config->currCity->connections.count(); i++)
 {
     Connection* c = config->currCity->connections.at(i);
     if(c->id() == config->currConnection->id())
         continue;
     ui->cbConnections->addItem(c->description(), VPtr<Connection>::asQVariant(c));
 }

 for(int i=0; i<config->currCity->connections.count(); i++)
 {
     Connection* c = config->currCity->connections.at(i);
     if(c->id() != config->currCity->curConnectionId)
     {
         ui->cbConnections->setCurrentIndex(i);
         break;
     }
 }
// connect(ui->cbConnections, &QComboBox::currentTextChanged, [=]{
//     Connection* c = VPtr<Connection>::asPtr(ui->cbConnections->currentData());
//     c->configure();
// });
 on_chkAll_toggled(ui->chkAll->isChecked());
}

void ExportDlg::on_chkAll_toggled(bool)
{
 on_chkAltRoute_toggled(ui->chkAltRoute->isChecked());
 on_chkCompanies_toggled(ui->chkCompanies->isChecked());
 on_chkSegments_toggled(ui->chkSegments->isChecked());
 on_chkTractionTypes_toggled(ui->chkTractionTypes->isChecked());
}

ExportDlg::~ExportDlg()
{
    delete ui;
}

void ExportDlg::chkAll_changed(bool isChecked)
{
    ui->chkAltRoute->setChecked(isChecked);
    ui->chkComments->setChecked(isChecked);
    ui->chkCompanies->setChecked(isChecked);
    ui->chkIntersections->setChecked(isChecked);
    ui->chkLineSegments->setChecked(isChecked);
    ui->chkParameters->setChecked(isChecked);
    ui->chkRoutes->setChecked(isChecked);
    ui->chkSegments->setChecked(isChecked);
    ui->chkStations->setChecked(isChecked);
    ui->chkTerminals->setChecked(isChecked);
    ui->chkTractionTypes->setChecked(isChecked);
    ui->chkRouteComments->setChecked(isChecked);
    on_chkAll_toggled(isChecked);
}

void ExportDlg::btnCancel_clicked()
{
    this->close();
}

void ExportDlg::btnGo_clicked()
{
 ui->btnGo->setEnabled(false);
 ExportSql* exprt = new ExportSql(config, ui->chkDrop->isChecked(), this);
 if(ui->chkOverride->isChecked())
 {
  exprt->setOverride(ui->exportDate->dateTime());
 }
 exprt->setNoDelete(ui->chkNoDelete->checkState());
 timer->start();
 for(int i=0; i<config->currCity->connections.count(); i++)
 {
  Connection* c = config->currCity->connections.at(i);
  if( c->description() == ui->cbConnections->currentText())
  {
   config->currCity->curExportConnId = c->id();
   config->saveSettings();
   currConnection = c;
   break;
  }
 }

 connect(exprt, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
 connect(exprt,SIGNAL(progressMsg(QString)), this, SLOT(newProgressMsg(QString)));
 connect(exprt, SIGNAL(uncheck(QString)),  this, SLOT(uncheckControl(QString)));
 connect(exprt, &ExportSql::requestStop, [=] {
     stopEnabled = true;
     close();
 });

// if(ui->chkAll->isChecked())
//     exprt.exportAll();
// else
 stopEnabled = true;
 if(currConnection->servertype() == "MsSql")
     SQL::instance()->useDatabase(currConnection->defaultSqlDatabase(), exprt->targetDb());
 while(stopEnabled)
 {
  if(ui->chkParameters->isChecked())
  {
      ui->lblHelp->setText(tr("Parameters"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportParameters())
      {
       ui->lblHelp->setText("Export of Parameters has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkParameters->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkCompanies->isChecked())
  {
      ui->lblHelp->setText(tr("Companies"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->dropRoutes())
        ui->lblHelp->setText("Drop table Routes failed");
      if(!exprt->exportCompanies())
      {
       ui->lblHelp->setText("Export of Companies has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkCompanies->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkTractionTypes->isChecked())
  {
      ui->lblHelp->setText(tr("Traction Types"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->dropRoutes())
        ui->lblHelp->setText("Drop table Routes failed");
      if(!exprt->exportTractionTypes())
      {
       ui->lblHelp->setText("Export of TractionTypes has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkTractionTypes->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkAltRoute->isChecked())
  {
      ui->lblHelp->setText(tr("Alt Routes"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->dropRoutes())
        ui->lblHelp->setText("Drop table Routes failed");
      if(!exprt->exportAltRoute())
      {
       ui->lblHelp->setText("Export of altRoute has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkAltRoute->setChecked(false);
      qApp->processEvents(QEventLoop::AllEvents);
  }
  if(ui->chkComments->isChecked())
  {
      ui->lblHelp->setText(tr("Comments"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportComments())
      {
       ui->lblHelp->setText("Export of Comments has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkComments->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkIntersections->isChecked())
  {
      ui->lblHelp->setText(tr("Intersections"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportIntersections())
      {
       ui->lblHelp->setText("Export of Intersections has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkIntersections->setChecked(false);
      qApp->processEvents();
  }
//     if(ui->chkLineSegments->isChecked())
//     {
//         ui->label->setText(tr("Line Segments"));
//         qApp->processEvents();
//         ui->progressBar->setValue(0);
//         exprt->exportLineSegments();
//         ui->chkLineSegments->setChecked(false);
//         qApp->processEvents();
//     }
  if(ui->chkSegments->isChecked())
  {
      ui->lblHelp->setText(tr("Segments"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->dropRoutes())
       ui->lblHelp->setText("Drop Route failed");
      if(!exprt->exportSegments())
      {
       ui->lblHelp->setText("Export of Segments has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkSegments->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkRoutes->isChecked())
  {
      ui->lblHelp->setText(tr("Routes"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportRoute())
      {
       ui->lblHelp->setText("Export of Routes has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkRoutes->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkStations->isChecked())
  {
      ui->lblHelp->setText(tr("Stations"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportStations())
      {
       ui->lblHelp->setText("Export of Stations has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkStations->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkTerminals->isChecked())
  {
      ui->lblHelp->setText(tr("Terminals"));
      qApp->processEvents();
      ui->progressBar->setValue(0);
      if(!exprt->exportTerminals())
      {
       ui->lblHelp->setText("Export of Terminals has errors");
       ui->btnGo->setEnabled(true);
       return;
      }
      ui->chkTerminals->setChecked(false);
      qApp->processEvents();
  }
  if(ui->chkRouteComments->isChecked())
  {
   ui->lblHelp->setText(tr("RouteComments"));
   qApp->processEvents();
   ui->progressBar->setValue(0);
   if(!exprt->exportRouteComments())
   {
    ui->lblHelp->setText("Export of RouteComments has errors");
    ui->btnGo->setEnabled(true);
    return;
   }
   ui->chkRouteComments->setChecked(false);
   qApp->processEvents();
  }
  ui->lblHelp->setText(tr("Done"));

  exprt->export_geodb_geometry();
  if(stopEnabled)
      break;
 }
 timer->stop();
 this->close();
 ui->btnGo->setEnabled(true);
}

void ExportDlg::quickProcess()
{
    QApplication::processEvents();

    show();
}

void ExportDlg::chkOverrideToggled(bool checked)
{
    if(checked)
    {
        ui->exportDate->setVisible(true);
    }
    else
    {
        ui->exportDate->setVisible(false);
    }
}

void ExportDlg::newProgressMsg(QString msg)
{
    ui->lblHelp->setText(msg);
}

void ExportDlg::uncheckControl(QString control)
{
 QCheckBox * ctrl =findChild<QCheckBox*>( control);
 if(ctrl)
     ctrl->setChecked(false);
 else
     qDebug()<<tr("ExportDlg::uncheckControl: control '")+control+"' not found.";
}

void ExportDlg::on_chkDrop_toggled(bool bState)
{
 ui->chkOverride->setEnabled(!bState);
 ui->exportDate->setEnabled(!bState);
 ui->chkNoDelete->setEnabled(!bState);
}

void ExportDlg::on_chkCompanies_toggled(bool bChecked)
{
 if(bChecked)
 {
  if(!ui->chkRoutes->isChecked())
  {
   // foreign key refereces this table
   ui->chkRoutes->setChecked(true);
  }
 }
}
void ExportDlg::on_chkSegments_toggled(bool bChecked)
{
 if(bChecked)
 {
  if(!ui->chkRoutes->isChecked())
  {
   // foreign key refereces this table
   ui->chkRoutes->setChecked(true);
  }
 }
}

void ExportDlg::on_chkTractionTypes_toggled(bool bChecked)
{
 if(bChecked)
 {
  if(!ui->chkRoutes->isChecked())
  {
   // foreign key refereces this table
   ui->chkRoutes->setChecked(true);
  }
 }
}

void ExportDlg::on_chkAltRoute_toggled(bool bChecked)
{
 if(bChecked)
 {
  if(!ui->chkRoutes->isChecked())
  {
   // foreign key refereces this table
   ui->chkRoutes->setChecked(true);
  }
 }
}

//void ExportDlg::closeEvent(QCloseEvent* e)
//{
//    e->ignore();
//}

