#include "combineroutesdlg.h"
#include "ui_combineroutesdlg.h"

CombineRoutesDlg::CombineRoutesDlg(int companyKey, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombineRoutesDlg)
{
    ui->setupUi(this);
    config = Configuration::instance();
    sql = SQL::instance();
    //sql->setConfig(config);
    myParent = parent;
    ui->lblHelp->setText("");
    this->setWindowTitle(tr("Combine two routes"));

    connect(ui->endDate, SIGNAL(dateChanged(QDate)), this, SLOT(on_endDate_dateChanged(QDate)));
    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(on_dateEdit_dateChanged(QDate)));

   companyData =  sql->getCompany(companyKey);

    refreshRoutes();
}

CombineRoutesDlg::~CombineRoutesDlg()
{
    delete ui;
}
void CombineRoutesDlg::refreshRoutes()
{
    ui->cbRoute1->clear();
    ui->cbRoute2->clear();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.count()== 0)
        return;
    foreach (RouteData rd, routeDataList)
    {
        ui->cbRoute1->addItem(rd.toString());
        ui->cbRoute2->addItem(rd.toString());
    }
}
void CombineRoutesDlg::on_txtNewRouteNbr_editingFinished()
{
    ui->lblHelp->setText("");
    if(ui->txtNewRouteNbr->text() == "")
    {
        ui->lblHelp->setText(tr("Enter a route number!"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->txtNewRouteNbr->setFocus();
        return;
    }
    bool bAlphaRoute = false;
    //bRouteChanging = false;
    int companyKey = _rd1.companyKey;

    qint32 newRoute = sql->getNumericRoute(ui->txtNewRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);

    _routeNbr = newRoute;
    if (!config->currCity->bAlphaRoutes && bAlphaRoute)
    {
        ui->lblHelp->setText(tr( "Must be a number!"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->txtNewRouteNbr->setFocus();
        return;
    }
    ui->lblHelp->setText("");

}
void CombineRoutesDlg::on_txtNewRouteName_editingFinished()
{
    ui->lblHelp->setText("");

    if(ui->txtNewRouteName->text().isEmpty())
    {
        ui->lblHelp->setText(tr("Enter a name for the new route"));
        return;
    }
}
void CombineRoutesDlg::on_buttonBox_clicked(QAbstractButton *button)
{
    ui->lblHelp->setText("");

    if(ui->buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
        return;
    if(_rd1.route <0 || _rd2.route < 0 || ui->cbRoute1->currentIndex() == ui->cbRoute2->currentIndex())
    {
        ui->lblHelp->setText(tr("Select 2 valid routes"));
        return;
    }
    if(ui->txtNewRouteName->text().isEmpty())
    {
        ui->lblHelp->setText(tr("Enter a new route name"));
        return;
    }
    if(ui->txtNewRouteNbr->text() == "")
    {
        ui->lblHelp->setText(tr("Enter a route number"));
        return;
    }
    if(ui->endDate->date() < ui->dateEdit->date())
    {
        ui->lblHelp->setText(tr("end date cannot be before effective date"));
        return;
    }

    sql->beginTransaction("combineRoutes");
    _routeNbr = sql->addAltRoute(ui->txtNewRouteNbr->text(), companyData->routePrefix);
    _alphaRoute = ui->txtNewRouteNbr->text();

    QList<SegmentData> myArray;

//    // modify the end date for Route 1 to the date previous to the date control.
//    if (sql->modifyRouteDate(_rd1, false, ui->dateEdit->dateTime().addDays(-1)))
//    {
//        _rd1.endDate = ui->dateEdit->dateTime().addDays(-1);;

//    }
//    else
//    {
//        ui->lblHelp->setText(tr("modifyRouteDate failed"));
//        sql->RollbackTransaction("combineRoutes");
//        return;
//    }
//    // modify the end date for Route 2 to the date previous to the date control.
//    if (sql->modifyRouteDate(_rd2, false, ui->dateEdit->dateTime().addDays(-1)))
//    {
//        _rd2.endDate = ui->dateEdit->dateTime().addDays(-1);;

//    }
//    else
//    {
//        ui->lblHelp->setText(tr("modifyRouteDate failed"));
//        sql->RollbackTransaction("combineRoutes");
//        return;
//    }

    myArray = sql->getRouteSegmentsForDate(_rd1.route, _rd1.name, _rd1.endDate.toString("yyyy/MM/dd"));

    foreach(SegmentData sd1, myArray)
    {
        if (sql->addSegmentToRoute(_routeNbr, ui->txtNewRouteName->text().trimmed(), ui->dateEdit->date(),
                                   ui->endDate->date(), sd1.segmentId(), sd1.companyKey(),
                                   sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(),
                                   sd1.normalEnter(), sd1.normalLeave(), sd1.reverseEnter(), sd1.reverseLeave(),
                                   sd1.oneWay(), sd1.trackUsage()) == false)
        {
            ui->lblHelp->setText(tr("add failed: route ")+ QString("%1").arg(_routeNbr));
            //System.Media.SystemSounds.Asterisk.Play();
            sql->rollbackTransaction("combineRoutes");
            return;
        }
    }
    myArray = sql->getRouteSegmentsForDate(_rd2.route, _rd2.name, _rd2.endDate.toString("yyyy/MM/dd"));

    foreach(SegmentData sd1, myArray)
    {
        if (sql->addSegmentToRoute(_routeNbr, ui->txtNewRouteName->text().trimmed(), ui->dateEdit->date(),
                                   ui->endDate->date(), sd1.segmentId(), sd1.companyKey(),
                                   sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(), sd1.normalEnter(), sd1.normalLeave(),
                                   sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
        {
            ui->lblHelp->setText(tr("add failed"));
            //System.Media.SystemSounds.Asterisk.Play();
            sql->rollbackTransaction("combineRoutes");
            return;
        }
    }
    sql->commitTransaction("combineRoutes");

    this->accept();
    this->close();

}
void CombineRoutesDlg:: on_cbRoute1_currentIndexChanged(int sel)
{
    ui->lblHelp->setText("");

//    if( sel == ui->cbRoute1->currentIndex())
//    {
//        ui->lblHelp->setText(tr("Routes cannot be the same"));
//        return;
//    }
    _rd1 = routeDataList.at(sel);
    ui->dateEdit->setMinimumDate(_rd1.startDate);
    ui->dateEdit->setMaximumDate(_rd1.endDate.addDays(1));

    ui->endDate->setDate(_rd1.endDate);
}
void CombineRoutesDlg:: on_cbRoute2_currentIndexChanged(int sel)
{
    ui->lblHelp->setText("");
    _rd2 = routeDataList.at(sel);

    if( sel == ui->cbRoute1->currentIndex())
    {
        ui->lblHelp->setText(tr("Routes cannot be the same"));
        return;
    }
}
void CombineRoutesDlg::on_dateEdit_editingFinished()
{

}

void CombineRoutesDlg::on_dateEdit_dateChanged(QDate date)
{
    ui->lblHelp->setText("");
    if(date > ui->endDate->date())
    {
        ui->lblHelp->setText(tr("must be less than end date"));
    }
}
void CombineRoutesDlg::on_endDate_dateChanged(QDate date)
{
    ui->lblHelp->setText("");
    if(date < ui->dateEdit->date())
    {
        ui->lblHelp->setText(tr("must be >= than start date"));
    }
}
