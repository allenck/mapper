#include "modifyroutedialog.h"
#include "ui_modifyroutedialog.h"
#include "routenamewidget.h"

ModifyRouteDialog::ModifyRouteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRenameRoute)
{
    ui->setupUi(this);
    config = Configuration::instance();
    //sql->setConfig(config);
    sql = SQL::instance();
    ui->lblHelp->setText("");
    refreshRoutes();
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
    connect(ui->btnCancel,SIGNAL(clicked()),this, SLOT(reject()));
    //connect((ui->txtNewRouteNbr, SIGNAL(editingFinished()),this, SLOT()))
}

ModifyRouteDialog::~ModifyRouteDialog()
{
    delete ui;
}

//void ModifyRouteDialog::setConfig(Configuration * cfg)
//{
//    config = cfg;
//}

/// <summary>
/// Sets a reference to the current configuration
/// </summary>
//public configuration Configuration { set { config = value; } }

void ModifyRouteDialog::routeData(RouteData value)
{
//    if (value == null)
//        return;
    _rd = value;

//    ui->txtNewRouteNbr->setText( _rd.alphaRoute());
//    ui->txtNewRouteName->setText(_rd.routeName());
    ui->rnw->configure(&_rd,  ui->lblHelp);

    if (routeDataList.count()==0)
        refreshRoutes();
    //foreach (routeData rd in routeDataList)
    for(int i=0; i<routeDataList.count(); i++)
    {
        RouteData rd = (RouteData)routeDataList.at(i);
        if (rd.route() == _rd.route() && rd.routeName() == _rd.routeName() && rd.endDate() == _rd.endDate())
        {
            //cbRoutes.SelectedItem = rd;
            ui->cbRoutes->setCurrentIndex(i);
            //ui->rnw->newRouteName->setFocus();
            break;
        }
    }
}
RouteData ModifyRouteDialog::getRouteData()
{
    return _rd;
}
qint32 ModifyRouteDialog::newRoute()
{
    return _routeNbr;
}
QString ModifyRouteDialog::newName (){ return ui->rnw->newRouteName(); }

void ModifyRouteDialog::btnOK_Click()
{
    RouteData rd = (RouteData)routeDataList.at(ui->cbRoutes->currentIndex());
    if(ui->rnw->newRoute()<=0)
    {
     ui->lblHelp->setText(tr("route number invalid"));
     return;
    }
    _routeNbr = ui->rnw->newRoute();
    if(ui->rnw->newRouteName().isEmpty())
    {
     ui->lblHelp->setText(tr("route name invalid"));
     return;
    }

    //QList<RouteData> myArray = sql->getRouteDatasForDate(rd.route(), rd.routeName(), rd.endDate().toString("yyyy/MM/dd"));
    QList<SegmentData*> myArray = sql->getSegmentDataList(rd);
    if (myArray.count()==0)
    {
        ui->lblHelp->setText(tr("No routes"));
        //System.Media.SystemSounds.Asterisk.Play();
        return;
    }
    sql->beginTransaction("RenameRoute");
    CompanyData* cd = sql->getCompany(rd.companyKey());

    if(ui->rnw->routeNbrMustBeAdded())
    {
      //sql->addAltRoute(ui->rnw->alphaRoute().toInt(), ui->rnw->alphaRoute());
     _routeNbr = sql->addAltRoute(ui->rnw->newRoute(),ui->rnw->alphaRoute());
    }
    //foreach (routeData rd1 in myArray)
    for(int i=0; i < myArray.count(); i++)
    {
//        SegmentData d1 = myArray.at(i);
//        if (!sql->deleteRouteSegment(rd1.route(), rd1.routeName(), rd1.segmentId(),rd1.startDate().toString("yyyy/MM/dd"),
//                                     rd1.endDate().toString("yyyy/MM/dd")))
//        {
//            ui->lblHelp->setText(tr("Delete failed"));
//            //System.Media.SystemSounds.Asterisk.Play();
//            return;
//        }
//        if (!sql->addSegmentToRoute(_routeNbr, ui->txtNewRouteName->text(), rd1.startDate(),
//            rd1.endDate(), rd1.segmentId(),
//            rd1.companyKey(), rd1.tractionType(), rd1.direction(), rd1.next(), rd1.prev(),
//            rd1.normalEnter(), rd1.normalLeave(), rd1.reverseEnter(), rd1.reverseLeave(), rd1.oneWay(), rd1.trackUsage()))
//        {
//            ui->lblHelp->setText(tr("Update failed"));
//            //System.Media.SystemSounds.Asterisk.Play();
//            return;
//        }
     SegmentData* sd = myArray.at(i);
     SegmentData* sdNew = new SegmentData(*sd);
     //sdNew.setRouteName(newName().trimmed());
     sdNew->setRoute(_routeNbr);
     sdNew->setRouteName(ui->rnw->newRouteName());
     sdNew->setAlphaRoute(ui->rnw->alphaRoute());
     if(!sql->updateRoute(*sd, *sdNew))
     {
      sql->rollbackTransaction("RenameRoute");
      this->setResult(QDialog::Rejected);
     }

     TerminalInfo ti = sql->getTerminalInfo(rd.route(), rd.routeName(), rd.endDate());
     if(ti.route == -1)
     {
         //if(!sql->deleteTerminalInfo(rd.route, rd.name, sql->formatDate(rd.endDate)))
         //    throw (new ApplicationException("delete terminal info failed"));
         if (!sql->updateTerminals(_routeNbr, ui->rnw->newRouteName(), ti.startDate,
                                   ti.endDate, ti.startSegment, ti.startWhichEnd,
                                   ti.endSegment, ti.endWhichEnd))
             //throw (new ApplicationException("update terminal failed!"));
             ui->lblHelp->setText(tr("Update terminal failed"));
     }
     RouteSeq routeSeq = sql->getRouteSeq(rd);
     if(routeSeq.route()> 0)
     {
      if(sql->deleteRouteSeq(routeSeq))
      {
       routeSeq.setRouteName(newName());
       if(! SQL::instance()->addRouteSeq(routeSeq))
       {
        sql->rollbackTransaction("RenameRoute");
        this->setResult(QDialog::Rejected);
       }
      }
      else
      {
       sql->rollbackTransaction("RenameRoute");
       this->setResult(QDialog::Rejected);

      }
      sql->deleteRouteSegment(*sd);
     }
    }
    sql->commitTransaction("RenameRoute");
    //sql->myConnection.Close();

    _rd.setAlphaRoute(_alphaRoute);
    _rd.setRouteName(ui->rnw->newRouteName());
    _rd.setRoute(_routeNbr);

//    this.DialogResult = DialogResult.OK;
//    this.Close();
    this->setResult(QDialog::Accepted);
    accept();
}

void ModifyRouteDialog::refreshRoutes()
{
    QString text = ui->cbRoutes->currentText();
    //ui->cbRoutes.Text = "";
    ui->cbRoutes->clear();

    //mySql = new Sql();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.count()==0)
        return;

    //foreach (routeData rd in routeDataList)
    for(int i=0; i< routeDataList.count(); i++)
    {
        RouteData rd = (RouteData)routeDataList.at(i);
        ui->cbRoutes->addItem(rd.toString());
    }
    //ui->cbRoutes->setCurrentIndex();t = text;
    int count = 0;
    for(int i=0; i< routeDataList.count(); i++)
    {
        RouteData rd = (RouteData)routeDataList.at(i);
        if (text == rd.toString())
        {
            //cbRoutes.SelectedIndex = count;
            ui->cbRoutes->setCurrentIndex(count);
            break;
        }
        count++;
    }

}

void ModifyRouteDialog::cbRoutes_SelectedIndexChanged()
{
    if (ui->rnw->newRouteName() == "")
        //ui->txtNewRouteName->setText (ui->cbRoutes->currentText());
     ui->rnw->setRouteName(ui->cbRoutes->currentText());
}


