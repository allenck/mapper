#include "exportroutedialog.h"
#include "ui_exportroutedialog.h"
#include <QPushButton>

ExportRouteDialog::ExportRouteDialog(RouteData rd, Configuration *cfg, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ExportRouteDialog)
{
 ui->setupUi(this);
 this->setWindowTitle(tr("Export Route"));
 config = cfg;
 this->rd = rd;
 ui->lblRoute->setText(rd.toString());
 timer = new QTimer(this);
 timer->setInterval(1000);
 connect(timer, SIGNAL(timeout()), this, SLOT(quickProcess()));

 connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(btnOK_clicked()));
 connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(btnCancel_clicked()) );
 ui->progressBar->setMinimum(0);
 ui->progressBar->setMaximum(100);
 ui->progressBar->setValue(0);

 ui->cbConnections->clear();
 for(int i=0; i<config->currCity->connections.count(); i++)
 {
     Connection* c = config->currCity->connections.at(i);
     if(c->id() == config->currConnection->id())
         continue;
     ui->cbConnections->addItem(c->description());
 }
 for(int i=0; i<config->currCity->connections.count(); i++)
 {
     Connection* c = config->currCity->connections.at(i);
     if(c->id() == config->currCity->curConnectionId)
     {
         ui->cbConnections->setCurrentIndex(i);
         break;
     }
 }
 connect(ui->cbConnections, SIGNAL(currentIndexChanged(QString)), this, SLOT(On_cbConnections_currentIndexChanged(QString)));
 if(ui->cbConnections->currentText() == "")
  ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

ExportRouteDialog::~ExportRouteDialog()
{
 delete ui;
}

void ExportRouteDialog::quickProcess()
{
 QApplication::processEvents();
}

void ExportRouteDialog::btnCancel_clicked()
{
    this->close();
}

void ExportRouteDialog::btnOK_clicked()
{
 ExportSql exprt(config, this);
 timer->start();

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

 connect(&exprt, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
 connect(&exprt,SIGNAL(progressMsg(QString)), this, SLOT(newProgressMsg(QString)));

 // Routes
 ui->lblHelp->setText(tr("Routes"));
 qApp->processEvents();
 ui->progressBar->setValue(0);
 exprt.exportRoute( rd);
 //ui->chkRoutes->setChecked(false);
 qApp->processEvents();

}

void ExportRouteDialog::newProgressMsg(QString msg)
{
    ui->lblHelp->setText(msg);
}

void ExportRouteDialog::On_cbConnections_currentIndexChanged(QString txt)
{
 if(txt.isEmpty())
  ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
 else
  ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}
