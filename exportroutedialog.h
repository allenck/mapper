#ifndef EXPORTROUTEDIALOG_H
#define EXPORTROUTEDIALOG_H

#include <QDialog>
#include "sql.h"
#include "configuration.h"
#include "exportsql.h"

namespace Ui {
class ExportRouteDialog;
}

class ExportRouteDialog : public QDialog
{
 Q_OBJECT

public:
 explicit ExportRouteDialog(RouteInfo rd, Configuration *cfg, QWidget *parent = 0);
 ~ExportRouteDialog();

public slots:
 void btnCancel_clicked();
 void btnOK_clicked();
 void quickProcess();
 void newProgressMsg(QString msg);
 void On_cbConnections_currentIndexChanged(QString);

private:
 Ui::ExportRouteDialog *ui;
 Configuration * config;
 QTimer *timer;
 RouteInfo ri;
};

#endif // EXPORTROUTEDIALOG_H
