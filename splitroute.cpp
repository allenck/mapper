#include "splitroute.h"
#include "ui_splitroute.h"
#include "data.h"
#include "routecommentsdlg.h"

SplitRoute::SplitRoute( QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SplitRoute)
{
    config = Configuration::instance();
    sql = SQL::instance();
    ui->setupUi(this);
    this->setWindowTitle(tr("Split route at date"));
    //fillCompanies();
    ui->lblHelp->setText("");
    ui->chkDeleteOriginal->setChecked(true);
    connect(ui->dateFrom1, SIGNAL(editingFinished()), this, SLOT(dateFrom1_Leave()));
    //connect(ui->dateFrom2, SIGNAL(editingFinished()), this, SLOT(dateFrom2_ValueChanged()));
    connect(ui->dateFrom2, SIGNAL(editingFinished()),this, SLOT(dateFrom2_Leave()));
    connect(ui->dateTo1, SIGNAL(editingFinished()), this, SLOT(dateTo1_Leave()));
    //connect(ui->dateTo1, SIGNAL(editingFinished()), this, SLOT(dateTo1_ValueChanged()));
    connect(ui->dateTo2, SIGNAL(editingFinished()), this, SLOT(dateTo2_Leave()));
    connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(Cancel_Click()));
    // connect(ui->cbCompany1, &QComboBox::currentTextChanged, [=] ()
    // {
    //     int companyKey = ui->cbCompany1->currentData().toInt();
    //     if(companyKey > 0)
    //     {
    //         CompanyData* cd = sql->getCompany(companyKey);
    //         if(ui->dateTo1->date() > cd->endDate)
    //         {
    //             ui->dateTo1->setDate(cd->endDate);
    //             ui->dateFrom2->setDate(cd->endDate.addDays(-1));
    //         }
    //         if(_rd.endDate() < cd->endDate)
    //             ui->dateTo2->setDate(_rd.endDate() );
    //         else
    //             ui->dateTo2->setDate(cd->endDate);
    //     }
    // });

    fillTractionTypes();
}

SplitRoute::~SplitRoute()
{
    delete ui;
}

bool SplitRoute::setRouteData(RouteData rd)
{
    _rd = rd;
    if (rd.route() < 1)
        return false;
    CompanyData* cd = sql->getCompany(_rd.companyKey());
    if(!cd)
        throw IllegalArgumentException("invalid company");
    maxEndDate = cd->endDate;
    QDate nextStartDate = sql->getNextStartOrEndDate(_rd.route(), _rd.startDate(), _rd.segmentId(), true);
    if(nextStartDate < maxEndDate)
        maxEndDate = nextStartDate.addDays(-1);
    if(maxEndDate.isValid())
    {
        if(rd.endDate() > maxEndDate)
        {
            if(QMessageBox::critical(this, tr("Error"),
                                  tr("The end date for this route %1\n"
                                     "overlaps a succeding route starting on %2\n"
                                     "This route's end date must be less than or equal to %3")
                                        .arg(_rd.endDate().toString("yyyy/MM/dd"),
                                        nextStartDate.toString("yyyy/MM/dd"),
                                        maxEndDate.toString("yyyy/MM/dd")),QMessageBox::Cancel |
                                          QMessageBox::Ignore) == QMessageBox::Cancel)

                return false;
        }
    }
    bIgnoreDateCheck = true;

    ui->dateFrom1->setDate( rd.startDate());
    QDate test = rd.startDate().addYears(1);
    test =test.addDays(-1);
    if(test > rd.endDate())
    {
     test = rd.startDate().addMonths(1);
     test =test.addDays(-1);
     if(test > rd.endDate())
      test = rd.startDate().addDays(1);
    }
    ui->dateTo1->setDate(test);
    ui->dateFrom2->setDate(test.addDays(1));
    ui->dateTo2->setDate( rd.endDate());

    bRoute1Changed = true;
    bRoute2Changed = true;

    if (routeDataList.isEmpty())
        refreshRoutes();
    ui->cbRoutes->setCurrentIndex(ui->cbRoutes->findText(rd.toString()));
#if 0
    //foreach (routeData rd in routeDataList)
    for(int i = 0; i < routeDataList.count(); i++)
    {
        RouteData rd = routeDataList.at(i);
        if (rd.route() == _rd.route()
            && rd.routeName() == _rd.routeName()
            && rd.endDate() == _rd.endDate())
        {
            ui->cbRoutes->setCurrentIndex(i);
            //ui->txtNewRouteName1->setFocus();
            ui->dateFrom1->setDate( rd.startDate());
            ui->dateTo1->setDate(rd.startDate().addDays(1));
            ui->dateFrom2->setDate(rd.startDate().addDays(2));
            ui->dateTo2->setDate( rd.endDate());
            break;
        }
    }
#endif
    //CompanyData* cd = sql->getCompany(_rd.companyKey());
    fillCompanies();
    if(rd.companyKey() > 0)
    {
        CompanyData* cd = sql->getCompany(rd.companyKey());
        int ix = ui->cbCompany1->findData(cd->companyKey);
        if(ix < 0)
        {
            ui->cbCompany1->addItem(cd->name, cd->companyKey);
            ui->cbCompany2->addItem(cd->name, cd->companyKey);
        }
    }
    ui->rnw1->configure(&rd, ui->lblHelp);
    ui->rnw1->setCompanyKey(ui->cbCompany1->currentData().toInt());
    ui->rnw2->configure(&rd, ui->lblHelp);
    ui->rnw2->setCompanyKey(ui->cbCompany2->currentData().toInt());

    ui->cbCompany1->setCurrentIndex(ui->cbCompany1->findData(_rd.companyKey()));
    ui->cbCompany2->setCurrentIndex(ui->cbCompany2->findData(_rd.companyKey()));
    connect(ui->cbCompany1, &QComboBox::currentTextChanged, [=](){
     ui->rnw1->setCompanyKey(ui->cbCompany1->currentData().toInt());
     CompanyData* cd = sql->getCompany(ui->cbCompany1->currentData().toInt());
    if(ui->dateTo1->date() > cd->endDate)
    {
     ui->dateTo1->setDate(cd->endDate);
     ui->dateFrom2->setDate(cd->endDate.addDays(1));
    }
    });
    connect(ui->cbCompany2, &QComboBox::currentTextChanged, [=](){
     ui->rnw2->setCompanyKey(ui->cbCompany2->currentData().toInt());
     CompanyData* cd = sql->getCompany(ui->cbCompany2->currentData().toInt());
     if(cd->companyKey != ui->cbCompany1->currentData().toInt())
         ui->dateFrom2->setDate(cd->startDate);
    });

    ui->cbTractionType->setCurrentIndex(ui->cbTractionType->findData(_rd.tractionType()));
    return true;
}

void SplitRoute::refreshRoutes()
{
    ui->cbRoutes->clear();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.isEmpty())
        return;
    //foreach (routeData rd in routeDataList)
    int ix = -1;
    for(int i = 0; i < routeDataList.count(); i++)
    {
        RouteData rd = routeDataList.at(i);
        ui->cbRoutes->addItem(rd.toString());
    }
    //cbRoutes.SelectedText = "";
}
RouteData SplitRoute::getRoute()
{
    return _rd;
}
RouteData SplitRoute::getNewRoute()
{
    return _newRoute;
}

void SplitRoute::fillCompanies()
{
    ui->cbCompany1->clear();
    ui->cbCompany2->clear();
    _companyList = sql->getCompaniesInDateRange(_rd.startDate(), _rd.endDate());
    if (_companyList.isEmpty())
        return;
    //foreach (companyData cd in _companyList)
    for(int i = 0; i < _companyList.count(); i++)
    {
        CompanyData* cd = _companyList.at(i);
        ui->cbCompany1->addItem(cd->toString(), cd->companyKey);
        ui->cbCompany2->addItem(cd->toString(), cd->companyKey);
    }
}

#if 0
void SplitRoute::txtNewRouteNbr1_TextChanged(QString text)
{
    Q_UNUSED(text);

    bRoute1Changed = true;

}

void SplitRoute::txtNewRouteNbr1_Leave()
{
    if (ui->txtNewRouteNbr1->text() == "")
    {
        ui->lblHelp->setText(tr("Enter a route number!"));
        QApplication::beep();
        ui->txtNewRouteNbr1->setFocus();
        return;
    }
    bool bAlphaRoute = false;
    //bRouteChanging = false;
    int companyKey = ui->cbCompany1->itemData(ui->cbCompany1->currentIndex()).toInt();
    qint32  newRoute = sql->getNumericRoute(ui->txtNewRouteNbr1->text(), & _alphaRoute1, & bAlphaRoute, companyKey);

    _routeNbr1 = newRoute;
    if (!config->currCity->bAlphaRoutes && bAlphaRoute)
    {
        ui->lblHelp->setText(tr("Must be a number!"));
        QApplication::beep();
        ui->txtNewRouteNbr1->setFocus();
        return;
    }
    ui->lblHelp->setText("");

}

void SplitRoute::txtNewRouteName1_Leave()
{
    if (ui->rnw1->newRouteName() == "")
    {
        ui->lblHelp->setText(tr("Enter a new Route name"));
        QApplication::beep();
        ui->txtNewRouteName1->setFocus();
    }
}

void SplitRoute::txtNewRouteNbr2_Leave()
{
    if (ui->txtNewRouteNbr2->text() == "")
    {
        ui->lblHelp->setText (tr("Enter a route number!"));
        QApplication::beep();
        ui->txtNewRouteNbr2->setFocus();
        return;
    }
    bool bAlphaRoute = false;
    //bRouteChanging = false;
    int companyKey = ui->cbCompany2->itemData(ui->cbCompany2->currentIndex()).toInt();

    int newRoute = sql->getNumericRoute(ui->txtNewRouteNbr1->text(), & _alphaRoute2, & bAlphaRoute, companyKey);

    _routeNbr2 = newRoute;
    if (!config->currCity->bAlphaRoutes && bAlphaRoute)
    {
        ui->lblHelp->setText (tr("Must be a number!"));
        QApplication::beep();
        ui->txtNewRouteNbr2->setFocus();;
        return;
    }
    ui->lblHelp->setText ("");

}

void SplitRoute::txtNewRouteName2_Leave()
{
    if (ui->rnw1->newRouteName() == "")
    {
        ui->lblHelp->setText (tr("Enter a new Route name"));
        ui->txtNewRouteName1->setFocus();;
    }
}
#endif
void SplitRoute::dateFrom1_Leave()
{
    if(ui->dateTo1->dateTime() < ui->dateFrom1->dateTime())
        ui->dateTo1->setFocus();;
}

void SplitRoute::dateTo1_ValueChanged()
{
    if(!ui->chkAllowGap->isChecked())
        ui->dateFrom2->setDateTime( ui->dateTo1->dateTime().addDays(1));
}

void SplitRoute::dateTo1_Leave()
{
    if (ui->dateTo1->dateTime() < ui->dateFrom1->dateTime() && !ui->chkAllowGap->isChecked())
        //ui->dateTo1->setDateTime( ui->dateFrom1->dateTime().addDays(1));
        ui->dateFrom2->setDateTime( ui->dateTo1->dateTime().addDays(1));

}

void SplitRoute::dateFrom2_ValueChanged()
{
    if (ui->dateFrom2->dateTime() < ui->dateTo1->dateTime()&& !ui->chkAllowGap->isChecked())
        ui->dateFrom2->setDateTime(ui->dateTo1->dateTime().addDays( 1));

}

void SplitRoute::dateFrom2_Leave()
{
    ui->lblHelp->setText ("");
    if(ui->dateFrom1->dateTime() > ui->dateFrom2->dateTime())
    {
        ui->lblHelp->setText (tr("From start date is after end date"));
        QApplication::beep();
        ui->dateFrom1->setFocus();;
        return;
    }
    if(ui->dateTo1->date()< ui->dateTo2->date()&& !ui->chkAllowGap->isChecked())
    {
        ui->dateTo1->setDateTime(ui->dateFrom2->dateTime().addDays(-1));
    }

}

void SplitRoute::dateTo2_Leave()
{
    ui->lblHelp->setText ("");
    if(ui->dateTo1->date() > ui->dateTo2->date())
    {
        ui->lblHelp->setText (tr("To start date is after end date"));
        QApplication::beep();
        ui->dateTo1->setFocus();;
        return;
    }
    CompanyData* cd = sql->getCompany(ui->cbCompany2->currentData().toInt());
    if(ui->dateTo2->date()> cd->endDate)
    {
        ui->dateTo2->setDate(cd->endDate);
    }
    if (ui->dateTo2->date() < ui->dateFrom2->date()&& !ui->chkAllowGap->isChecked())
    {
        ui->dateTo2->setDate( ui->dateFrom2->date().addDays(1));
    }
}

void SplitRoute::btnOK_Click()
{
    QList<RouteData> routes;
    ui->lblHelp->setText ("");
    if(ui->dateFrom1->dateTime() > ui->dateFrom2->dateTime())
    {
        ui->lblHelp->setText (tr("From start date1 is after end date1"));
        QApplication::beep();
        ui->dateFrom1->setFocus();;
        return;
    }
    if(ui->dateTo1->dateTime() > ui->dateTo2->dateTime())
    {
        ui->lblHelp->setText (tr("To end date1 is after end date2"));
        QApplication::beep();
        ui->dateTo1->setFocus();;
        return;
    }
    if(ui->dateFrom1->dateTime() > ui->dateTo1->dateTime())
    {
        ui->lblHelp->setText (tr("Date #1 from must be less or equal to than To date"));
        QApplication::beep();
        ui->dateFrom1->setFocus();;
        return;
    }
    CompanyData* cd = sql->getCompany(ui->cbCompany1->currentData().toInt());
    if (cd->companyKey < 0)
    {
        ui->lblHelp->setText (tr("Select a company 1"));
        QApplication::beep();
        return;
    }
    if (ui->dateTo1->date() < cd->startDate || ui->dateFrom1->date() > cd->endDate)
    {
        ui->lblHelp->setText (tr("Company 1 not valid for specified dates!"));
        QApplication::beep();
        ui->dateFrom1->setFocus();
        return;
    }
    cd =  sql->getCompany(ui->cbCompany2->currentData().toInt());
    if (cd->companyKey < 0)
    {
        ui->lblHelp->setText (tr("Select a company 2"));
        QApplication::beep();
        return;
    }
    if (ui->dateTo2->date() < cd->startDate || ui->dateFrom2->date() > cd->endDate)
    {
        ui->lblHelp->setText (tr("Company 2 not valid for specified dates! Must be > %1")
                                 .arg(cd->startDate.toString("yyyy/MM/dd")));
        QApplication::beep();
        ui->dateFrom2->setFocus();
        return;
    }
    if(ui->dateFrom1->dateTime() > ui->dateTo1->dateTime())
    {
        ui->lblHelp->setText (tr("Start date 2 after end date"));
        QApplication::beep();
        ui->dateFrom1->setFocus();
        return;
    }
    if(ui->dateFrom2->dateTime() > ui->dateTo2->dateTime())
    {
        ui->lblHelp->setText (tr("Start date 2 after end date"));
        QApplication::beep();
        ui->dateFrom2->setFocus();
        return;
    }
    if (ui->dateTo2->dateTime().date() > cd->endDate)
    {
        ui->lblHelp->setText (tr("end date 2 > company enddate"));
        QApplication::beep();
        ui->dateFrom1->setFocus();;
        return;
    }
    setCursor(Qt::WaitCursor);
    sql->beginTransaction("SplitRoute");
    //sql->executeCommand("PRAGMA defer_foreign_keys = 1");
    //cd = sql->getCompany(ui->cbCompany1->itemData(ui->cbCompany1->currentIndex()).toInt());
    cd = sql->getCompany(ui->cbCompany1->currentData().toInt());
    if(cd->companyKey < 0)
    {
     sql->rollbackTransaction("SplitRoute");
     setCursor(Qt::ArrowCursor);
     return;
    }

    //_routeNbr1 = sql->addAltRoute(ui->rnw1->alphaRoute(), cd->routePrefix);
    if(ui->rnw1->routeNbrMustBeAdded())
    {
     if(!sql->addAltRoute(ui->rnw1->newRoute(), ui->rnw1->alphaRoute(),cd->routePrefix))
     {
      QApplication::beep();
      setCursor(Qt::ArrowCursor);
      sql->rollbackTransaction("SplitRoute");
      return;
     }
    }

    _routeNbr1 = ui->rnw1->newRoute();
    _routeNbr2 = ui->rnw2->newRoute();
    routes = sql->getRouteDataForRouteName(_routeNbr1, ui->rnw1->newRouteName());
    if (!routes.isEmpty())
    {
     //foreach (routeData rd1 in routes)
     for(int i = 0; i< routes.count(); i++)
     {
      RouteData rd1 = routes.at(i);
      if (!(ui->dateFrom1->date() > rd1.endDate() || ui->dateTo1->date() < rd1.startDate()))
      {
       if (!ui->chkDeleteOriginal->isChecked())
       {
        ui->lblHelp->setText (tr("Route 1 already exists"));
        QApplication::beep();
        setCursor(Qt::ArrowCursor);
        sql->rollbackTransaction("SplitRoute");
        return;
       }
      }
     }
    }
    cd = sql->getCompany(ui->cbCompany2->itemData(ui->cbCompany2->currentIndex()).toInt());

    //_routeNbr2 = sql->addAltRoute(ui->rnw2->alphaRoute(),cd->routePrefix);
    if(ui->rnw2->routeNbrMustBeAdded())
    {
     if(!sql->addAltRoute(ui->rnw2->newRoute(), ui->rnw2->alphaRoute(),cd->routePrefix))
     {
      QApplication::beep();
      setCursor(Qt::ArrowCursor);
      sql->rollbackTransaction("SplitRoute");
      return;
     }
    }
    routes = sql->getRouteDataForRouteName(_routeNbr2, ui->rnw2->newRouteName());
    if (!routes.isEmpty())
    {
     //foreach (routeData rd2 in routes)
     for(int i = 0; i <routes.count(); i ++)
     {
      RouteData rd2 = routes.at(i);
      if (!(ui->dateFrom2->date() > rd2.endDate() || ui->dateTo1->date() < rd2.startDate()))
      {
       if (!ui->chkDeleteOriginal->isChecked())
       {
        ui->lblHelp->setText (tr("Route 2 already exists"));
        QApplication::beep();
        setCursor(Qt::ArrowCursor);
        sql->rollbackTransaction("SplitRoute");
        return;
       }
      }
     }
    }

    // QList<RouteData> myArray = sql->getRouteDatasForDate(_rd.route(), _rd.routeName(), _rd.companyKey(),
    //                                                      _rd.endDate().toString("yyyy/MM/dd"));
    QList<SegmentData*> myArray = sql->getSegmentDatasForDate(_rd.route(), _rd.routeName(), _rd.companyKey(),
                                                         _rd.endDate());
    if(!sql->executeCommand(QString("delete from routes where route = %1 and routeid = %2").arg(_rd.route()).arg( myArray.at(0)->routeId())))
    {
        qDebug() << "delete route failed!";
        ui->lblHelp->setText (tr("delete route failed"));
        QApplication::beep();
        setCursor(Qt::ArrowCursor);
        sql->rollbackTransaction("SplitRoute");
        return;
    }

    // if(!sql->executeCommand(QString("delete from RouteName where routeId = %1").arg(myArray.at(0)->routeId())))
    // {
    //     qDebug() << "delete routename failed!";
    //     ui->lblHelp->setText (tr("delete routename failed"));
    //     QApplication::beep();
    //     setCursor(Qt::ArrowCursor);
    //     sql->rollbackTransaction("SplitRoute");
    //     return;
    // }
    //sql->BeginTransaction("SplitRoute");

    for(int i = 0; i <myArray.count(); i ++)
    {
        qApp->processEvents();
        setCursor(Qt::WaitCursor);
         SegmentData* sd1 = myArray.at(i);
         if(ui->chkDeleteOriginal->isChecked())
         {
          // if(!sql->deleteRouteSegment(sd1->route(), sd1->routeId(), sd1->segmentId(),
          //                             sd1->startDate().toString("yyyy/MM/dd"),
          //                             sd1->endDate().toString("yyyy/MM/dd")))
          if(!sql->deleteRouteSegment(*sd1, false))
          {
           ui->lblHelp->setText (tr("delete failed"));
           QApplication::beep();
           setCursor(Qt::ArrowCursor);
           sql->rollbackTransaction("SplitRoute");
           return;
          }
     }

     // add back if original has an earlier start date.
     if(sd1->startDate() < ui->dateFrom1->date()  && sd1->endDate() < ui->dateFrom2->date())
     {
         // RouteInfo ri = RouteInfo(_routeNbr1,ui->rnw1->newRouteName(),rd1.startDate(),rd1.endDate(),
         //                          rd1.companyKey(), rd1.alphaRoute());
         // bool bAlreadyPresent;
         // ri.setRouteId(sql->addRouteName(ri, &bAlreadyPresent));

         // if (sql->addSegmentToRoute(_routeNbr1, ri.routeId(),
         //                         rd1.startDate(), ui->dateTo1->date(),
         //                         rd1.segmentId(), ui->cbCompany1->currentData().toInt(),
         //                         rd1.tractionType(), rd1.direction(), rd1.next(), rd1.prev(), rd1.normalEnter(),
         //                         rd1.normalLeave(), rd1.reverseEnter(), rd1.reverseLeave(),
         //                         rd1.sequence(), rd1.returnSeq(), rd1.oneWay(), rd1.trackUsage(),
         //                         rd1.doubleDate()) == false)
         SegmentData sd1a = SegmentData(*sd1);
         sd1a.setRoute(_routeNbr1);
         sd1a.setRouteName(ui->rnw1->newRouteName());
         //sd1.setRouteId(ri.routeId());
         sd1a.setEndDate(ui->dateTo1->date());
         sd1a.setCompanyKey(ui->cbCompany1->currentData().toInt());
         if (!sql->addSegmentToRoute(&sd1a))
         {
             ui->lblHelp->setText (tr("add failed: ")+sd1a.toString2());
           QApplication::beep();
           setCursor(Qt::ArrowCursor);
           sql->rollbackTransaction("SplitRoute");
           return;
         }
     }

     // add back any after end date
     if(sd1->endDate() > ui->dateTo2->date() )
     {
      // if (sql->addSegmentToRoute(_routeNbr1, ui->rnw1->newRouteName(),
      //                            ui->dateTo2->date(), rd1.endDate(),
      //                            rd1.segmentId(), ui->cbCompany2->currentData().toInt(),
      //                            ui->cbTractionType->currentData().toInt(), rd1.direction(), rd1.next(), rd1.prev(),
      //                            rd1.normalEnter(), rd1.normalLeave(),
      //                            rd1.reverseEnter(), rd1.reverseLeave(),
      //                            rd1.sequence(), rd1.returnSeq(),
      //                            rd1.oneWay(), rd1.trackUsage(), rd1.doubleDate()) == false)
         SegmentData sd1a = SegmentData(*sd1);
         sd1a.setRoute(_routeNbr1);
         sd1a.setRouteName(ui->rnw1->newRouteName());
         sd1a.setRouteId(-1);
         sd1a.setEndDate(ui->dateTo1->date());
         sd1a.setCompanyKey(ui->cbCompany1->currentData().toInt());
         sd1a.setTractionType(ui->cbTractionType->currentData().toInt());
         if (!sql->addSegmentToRoute(&sd1a))
         {
             ui->lblHelp->setText (tr("add failed: ")+sd1a.toString2());
           QApplication::beep();
           setCursor(Qt::ArrowCursor);
           sql->rollbackTransaction("SplitRoute");
           return;
         }

     }

     // set new enddate for existing route
     // if (sql->addSegmentToRoute(_routeNbr1, ui->rnw1->newRouteName(),
     //                            ui->dateFrom1->date(), ui->dateTo1->date(),
     //                            rd1.segmentId(), ui->cbCompany1->currentData().toInt(),
     //                            rd1.tractionType(), rd1.direction(), rd1.next(), rd1.prev(),
     //                            rd1.normalEnter(), rd1.normalLeave(),
     //                            rd1.reverseEnter(), rd1.reverseLeave(),
     //                            rd1.sequence(), rd1.returnSeq(),
     //                            rd1.oneWay(), rd1.trackUsage(), rd1.doubleDate()) == false)
     SegmentData sd1a = SegmentData(*sd1);
     sd1a.setRoute(_routeNbr1);
     sd1a.setRouteName(ui->rnw1->newRouteName());
     //sd1.setRouteId(ri.routeId());
     sd1a.setStartDate(ui->dateFrom1->date());
     sd1a.setEndDate(ui->dateTo1->date());
     sd1a.setCompanyKey(ui->cbCompany1->currentData().toInt());
     //sd1.setTractionType(ui->cbTractionType->currentData().toInt());
     RouteInfo ri = RouteInfo(sd1a);
     if(!sql->updateRouteName(ri))
     {
         ui->lblHelp->setText (tr("updateRouteName failed"));
         QApplication::beep();
         setCursor(Qt::ArrowCursor);
         sql->rollbackTransaction("SplitRoute");
         return;
     }
     bool bAlreadyPresent = false;
     int new_routeid = sql->addRouteName(ri, &bAlreadyPresent);
     if(new_routeid < 0)
     {
         qDebug() << tr("addRouteName failed for %1").arg(sd1a.routeId());
         if(!sql->insertRouteName(ri))
         {
            return;
         }
     }
     else
         sd1a.setRouteId(new_routeid);

     if (!sql->addSegmentToRoute(&sd1a))
     {
      ui->lblHelp->setText (tr("add failed: ")+sd1a.toString2());
      QApplication::beep();
      setCursor(Qt::ArrowCursor);
      sql->rollbackTransaction("SplitRoute");
      return;
     }

     // ri = RouteInfo(_routeNbr2,ui->rnw2->newRouteName(),ui->dateFrom2->date(),ui->dateTo2->date(),ui->cbCompany2->currentData().toInt());
     // ri.setRouteId(sql->addRouteName(ri, &bAlreadyPresent));

     // if (sql->addSegmentToRoute(_routeNbr2, ui->rnw2->newRouteName(),
     //                            ui->dateFrom2->date().addDays(0), ui->dateTo2->date(),
     //                            rd1.segmentId(), ui->cbCompany2->currentData().toInt(),
     //                            ui->cbTractionType->currentData().toInt(), rd1.direction(), rd1.next(), rd1.prev(),
     //                            rd1.normalEnter(), rd1.normalLeave(),
     //                            rd1.reverseEnter(), rd1.reverseLeave(),
     //                            rd1.sequence(), rd1.returnSeq(),
     //                            rd1.oneWay(), rd1.trackUsage(), rd1.doubleDate()) == false)
     sd1a = SegmentData(*sd1);
     sd1a.setRoute(_routeNbr2);
     sd1a.setRouteName(ui->rnw2->newRouteName());
     sd1a.setRouteId(-1);
     sd1a.setStartDate(ui->dateFrom2->date());
     sd1a.setEndDate(ui->dateTo2->date());
     sd1a.setCompanyKey(ui->cbCompany2->currentData().toInt());
     sd1a.setTractionType(ui->cbTractionType->currentData().toInt());
     new_routeid = sql->addRouteName(ri, &bAlreadyPresent);
     if(new_routeid < 0)
     {
         qDebug() << tr("addRouteName failed for %1").arg(sd1a.routeId());
         if(!sql->insertRouteName(ri))
         {
             return;
         }
     }
     else
         sd1a.setRouteId(new_routeid);
     if (!sql->addSegmentToRoute(&sd1a))
     {
          ui->lblHelp->setText (tr("add failed: ")+sd1a.toString2());
          QApplication::beep();
          setCursor(Qt::ArrowCursor);
          sql->rollbackTransaction("SplitRoute");
      return;
     }
     _newRoute.setRoute(_routeNbr2);
     _newRoute.setRouteName(ui->rnw2->newRouteName());
     _newRoute.setStartDate(ui->dateFrom2->date().addDays(0));
     _newRoute.setEndDate(ui->dateTo2->date());
     _newRoute.setCompanyKey(ui->cbCompany2->currentData().toInt());
     _newRoute.setTractionType(sd1->tractionType());
    }
    sql->commitTransaction("SplitRoute");

    if(ui->cbKeepOpen->isChecked())
    {
     int ix = ui->cbRoutes->currentIndex();
     if(ix == ui->cbRoutes->count()-1)
     {
      ui->cbKeepOpen->setChecked(false);
      return;
     }
     ui->cbRoutes->setCurrentIndex(++ix);
     _rd = routeDataList.at(ix);

//     disconnect(ui->txtNewRouteNbr1, SIGNAL(textChanged(QString)), this, SLOT(txtNewRouteNbr1_TextChanged(QString)));
//     disconnect(ui->txtNewRouteNbr1, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteNbr1_Leave()));
//     disconnect(ui->txtNewRouteNbr2, SIGNAL(textChanged(QString)), this, SLOT(txtNewRouteNbr2_TextChanged(QString)));
//     disconnect(ui->txtNewRouteNbr2, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteNbr2_Leave()));
//     ui->txtNewRouteNbr1->setText( _rd.alphaRoute());
//     ui->txtNewRouteNbr2->setText(_rd.alphaRoute());
//     ui->txtNewRouteName1->setText( _rd.routeName());
//     ui->txtNewRouteName2->setText( _rd.routeName());
     setCursor(Qt::ArrowCursor);
     return;
    }


    if(ui->chkAddComment->isChecked())
    {
     RouteCommentsDlg *dlg = new RouteCommentsDlg(&routeDataList, _rd.companyKey());
     dlg->setDate(ui->dateFrom2->date());
     dlg->setRoute(ui->rnw2->newRoute());
     dlg->exec();
    }

    this->accept();
    setCursor(Qt::ArrowCursor);
    this->close();
}

void SplitRoute::Cancel_Click()
{
    this->reject();
    this->close();
}
#if 0
void SplitRoute::txtNewRouteNbr2_TextChanged(QString text)
{
    Q_UNUSED(text)
    bRoute2Changed = true;

}
#endif
void SplitRoute::fillTractionTypes()
{
    ui->cbTractionType->clear();
    //cbTractionType.Text = " ";
    _tractionList = sql->getTractionTypes();
    if ( _tractionList.count() == 0)
        return;
    //int count = 0;
    //foreach (tractionTypeInfo tti in _tractionList)
    for(int i= 0; i < _tractionList.count(); i++)
    {
     //if (si.routeType == tti.routeType)
     {
      TractionTypeInfo tti  = (TractionTypeInfo)_tractionList.values().at(i);
      ui->cbTractionType->addItem(tti.ToString(),tti.tractionType);
      //count++;
     }
    }
    //cbTractionType.Text = "";
}
