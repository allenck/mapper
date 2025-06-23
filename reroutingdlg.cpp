#include "reroutingdlg.h"
#include "ui_reroutingdlg.h"

ReroutingDlg::ReroutingDlg(RouteData rd, Configuration *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReroutingDlg)
{
    ui->setupUi(this);
    myParent = parent;
    config = cfg;
    _rd = rd;
    //sql->setConfig(cfg);
    sql = SQL::instance();

    ui->lblHelp->setText("");
    ui->lblRoute->setText(_rd.name);
//    ui->fromDate->setMinimumDate(_rd.startDate.date().addDays(1));
//    ui->fromDate->setMaximumDate(_rd.endDate.date().addDays(-1));
//    ui->toDate->setMinimumDate(_rd.startDate.date().addDays(1));
//    ui->toDate->setMaximumDate(_rd.endDate.date().addDays(-1));
    ui->fromDate->setDate(_rd.startDate.addDays(1));
    ui->fromDate->setToolTip(tr("Enter start date of rerouting."));
    ui->toDate->setDate(_rd.endDate.addDays(-1));
    ui->toDate->setToolTip(tr("Enter date that route resumes original routing."));
    ui->txtRoute->setText(_rd.name);

//    connect(ui->fromDate, SIGNAL(dateChanged(QDate)), this, SLOT(on_fromDate_changed(QDate)));
//    connect(ui->toDate, SIGNAL(dateChanged(QDate)), this, SLOT(on_toDate_changed(QDate)));
    //connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_buttonbox_accepted()));
    this->setWindowTitle(tr("Temporary Re-route"));

}

ReroutingDlg::~ReroutingDlg()
{
    delete ui;
}

void ReroutingDlg::on_fromDate_dateChanged(QDate date)
{
    ui->lblHelp->setText("");
    if(date > ui->toDate->date())
        //ui->toDate->setDate(date);
        ui->lblHelp->setText(tr("check to date; must be before!"));
}
void ReroutingDlg::on_fromDate_editingFinished()
{
    ui->lblHelp->setText("");
    if(ui->fromDate->date() < _rd.startDate.addDays(1))
        ui->fromDate->setDate(_rd.startDate.addDays(1));
}

void ReroutingDlg::on_toDate_dateChanged(QDate date)
{
    ui->lblHelp->setText("");
    if(date < ui->fromDate->date())
        //ui->fromDate->setDate(date);
        ui->lblHelp->setText(tr("check from date; must be later!"));

}
void ReroutingDlg::on_toDate_editingFinished()
{
    ui->lblHelp->setText("");
    if(ui->toDate->date() > _rd.endDate.addDays(-1))
        ui->toDate->setDate(_rd.endDate.addDays(-1));
}

void ReroutingDlg::on_btnOk_clicked()
{
    ui->lblHelp->setText("");

    if(ui->txtRoute->text() == "")
    {
        ui->lblHelp->setText(tr("Route name cannot be blank"));
        ui->txtRoute->setFocus();
        return;
    }

    setCursor(Qt::WaitCursor);
    sql->beginTransaction("reroute");
    QList<SegmentData> myArray = sql->getRouteSegmentsForDate(_rd.route, _rd.name, _rd.endDate.toString("yyyy/MM/dd"));

    foreach(SegmentData sd, myArray)
    {
        // copy original route before from date
        if (sql->addSegmentToRoute(sd.route(), sd.routeName(), sd.startDate(),
                                   ui->fromDate->date().addDays(-1), sd.segmentId(), sd.companyKey(),
                                   sd.tractionType(), sd.direction(), sd.next(), sd.prev(),
                                   sd.normalEnter(), sd.normalLeave(), sd.reverseEnter(), sd.reverseLeave(),
                                   sd.oneWay(), sd.trackUsage()) == false)
        {
            ui->lblHelp->setText(tr("add failed"));
            //System.Media.SystemSounds.Asterisk.Play();
            QApplication::beep();
            setCursor(Qt::ArrowCursor);
            return;
        }
        // create new route uing new name and fromDate to toDate
        if (sql->addSegmentToRoute(sd.route(), ui->txtRoute->text(), ui->fromDate->date(),
                                   ui->toDate->date().addDays(-1), sd.segmentId(), sd.companyKey(),
                                   sd.tractionType(), sd.direction(), sd.next(), sd.prev(), sd.normalEnter(),
                                   sd.normalLeave(), sd.reverseEnter(), sd.reverseLeave(), sd.oneWay(), sd.trackUsage()) == false)
        {
            ui->lblHelp->setText(tr("add failed"));
            //System.Media.SystemSounds.Asterisk.Play();
            QApplication::beep();
            setCursor(Qt::ArrowCursor);

            return;
        }
        // copy old route back after toDate
        if (sql->addSegmentToRoute(sd.route(), sd.routeName(),  ui->toDate->date(), sd.endDate(),
                                   sd.segmentId(), sd.companyKey(), sd.tractionType(), sd.direction(), sd.next(), sd.prev(),
                                   sd.normalEnter(), sd.normalLeave(), sd.reverseEnter(), sd.reverseLeave(), sd.oneWay(), sd.trackUsage()) == false)
        {
            ui->lblHelp->setText(tr("add failed"));
            //System.Media.SystemSounds.Asterisk.Play();
            QApplication::beep();
            setCursor(Qt::ArrowCursor);

            return;
        }
        if(!sql->deleteRouteSegment(sd.route(), sd.routeName(), sd.segmentId(), sd.startDate().toString("yyyy/MM/dd"),
                                                                                             sd.endDate().toString("yyyy/MM/dd")))
        {
            ui->lblHelp->setText(tr("delete failed"));
            //System.Media.SystemSounds.Asterisk.Play();
            QApplication::beep();
            setCursor(Qt::ArrowCursor);

            return;
        }
    }
    sql->commitTransaction("reroute");

    this->accept();
    setCursor(Qt::ArrowCursor);

}
void ReroutingDlg::on_btnCancel_clicked()
{
    this->reject();
}
