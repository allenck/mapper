#include "splitroute.h"
#include "ui_splitroute.h"
#include "data.h"

SplitRoute::SplitRoute(Configuration *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SplitRoute)
{
    config = cfg;
    //sql->setConfig(config);
    sql = SQL::instance();
    ui->setupUi(this);
    this->setWindowTitle(tr("Split route at date"));
    fillCompanies();
    ui->lblHelp->setText("");
    ui->chkDeleteOriginal->setChecked(true);
    connect(ui->txtNewRouteNbr1, SIGNAL(textChanged(QString)), this, SLOT(txtNewRouteNbr1_TextChanged(QString)));
    connect(ui->txtNewRouteNbr1, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteNbr1_Leave()));
    connect(ui->txtNewRouteNbr2, SIGNAL(textChanged(QString)), this, SLOT(txtNewRouteNbr2_TextChanged(QString)));
    connect(ui->txtNewRouteNbr2, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteNbr2_Leave()));
    connect(ui->txtNewRouteName1, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteName1_Leave()));
    connect(ui->txtNewRouteName2, SIGNAL(textEdited(QString)), this, SLOT(txtNewRouteName2_Leave()));
    connect(ui->dateFrom1, SIGNAL(editingFinished()), this, SLOT(dateFrom1_Leave()));
    connect(ui->dateFrom2, SIGNAL(editingFinished()), this, SLOT(dateFrom2_ValueChanged()));
    connect(ui->dateFrom2, SIGNAL(editingFinished()),this, SLOT(dateFrom2_Leave()));
    connect(ui->dateTo1, SIGNAL(editingFinished()), this, SLOT(dateTo1_Leave()));
    connect(ui->dateTo1, SIGNAL(editingFinished()), this, SLOT(dateTo1_ValueChanged()));
    connect(ui->dateTo2, SIGNAL(editingFinished()), this, SLOT(dateTo2_Leave()));
    connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(Cancel_Click()));
}

SplitRoute::~SplitRoute()
{
    delete ui;
}

/// <summary>
/// Sets a reference to the current configuration
/// </summary>
void SplitRoute::setConfiguration(Configuration *cfg) {  config = cfg; }

void SplitRoute::setRouteData(RouteData rd)
{
    _rd = rd;
    if (rd.route < 1)
        return;
    ui->txtNewRouteNbr1->setText( _rd.alphaRoute);
    ui->txtNewRouteNbr2->setText(_rd.alphaRoute);
    ui->txtNewRouteName1->setText( _rd.name);
    ui->txtNewRouteName2->setText( _rd.name);
    bRoute1Changed = true;
    bRoute2Changed = true;

    if (routeDataList.isEmpty())
        refreshRoutes();
    //foreach (routeData rd in routeDataList)
    for(int i = 0; i < routeDataList.count(); i++)
    {
        RouteData rd = routeDataList.at(i);
        if (rd.route == _rd.route && rd.name == _rd.name && rd.endDate == _rd.endDate)
        {
            ui->cbRoutes->setCurrentIndex(i);
            ui->txtNewRouteName1->setFocus();
            ui->dateFrom1->setDate( rd.startDate);
            ui->dateTo1->setDate(rd.startDate.addDays(1));
            ui->dateFrom2->setDate(rd.startDate.addDays(2));
            ui->dateTo2->setDate( rd.endDate);
            break;
        }
    }
    CompanyData* cd = sql->getCompany(_rd.companyKey);
    if (cd->companyKey > 0)
    {
        //foreach (companyData cd1 in _companyList)
        for(int i= 0; i < _companyList.count(); i++)
        {
            CompanyData* cd1 = _companyList.at(i);
            if (cd->companyKey == cd1->companyKey)
            {
                ui->cbCompany1->setCurrentIndex(i);
                ui->cbCompany2->setCurrentIndex(i);
                break;
            }
        }
    }
}
void SplitRoute::refreshRoutes()
{
    ui->cbRoutes->clear();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.isEmpty())
        return;
    //foreach (routeData rd in routeDataList)
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
    _companyList = sql->getCompanies();
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
    if (ui->txtNewRouteName1->text() == "")
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
    if (ui->txtNewRouteName1->text() == "")
    {
        ui->lblHelp->setText (tr("Enter a new Route name"));
        ui->txtNewRouteName1->setFocus();;
    }
}

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
        ui->dateTo1->setDateTime( ui->dateFrom1->dateTime().addDays(1));

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
    CompanyData* cd = _companyList.at(ui->cbCompany2->currentIndex());
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
    QList<SegmentData> routes;
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
//    if (ui->dateFrom1->dateTime() >= ui->dateTo1->dateTime())
//    {
//        ui->lblHelp->setText (tr("Date #1 from must be less than or equal To date"));
//        QApplication::beep();
//        ui->dateFrom1->setFocus();;
//        return;
//    }
    if (ui->dateTo1->dateTime() >= ui->dateFrom2->dateTime())
    {
        ui->lblHelp->setText (tr("#1 dates overlap #2 dates"));
        QApplication::beep();
        ui->dateFrom1->setFocus();;
        return;
    }
    CompanyData* cd = _companyList.at(ui->cbCompany1->currentIndex());
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
    cd =  _companyList.at(ui->cbCompany2->currentIndex());
    if (cd->companyKey < 0)
    {
        ui->lblHelp->setText (tr("Select a company 2"));
        QApplication::beep();
        return;
    }
    if (ui->dateTo2->date() < cd->startDate || ui->dateFrom2->date() > cd->endDate)
    {
        ui->lblHelp->setText (tr("Company 2 not valid for specified dates!"));
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
    setCursor(Qt::WaitCursor);
    sql->beginTransaction("SplitRoute");
    //cd = sql->getCompany(ui->cbCompany1->itemData(ui->cbCompany1->currentIndex()).toInt());
    cd = _companyList.at(ui->cbCompany1->currentIndex());
    if(cd->companyKey < 0)
    {
     sql->rollbackTransaction("SplitRoute");
     setCursor(Qt::ArrowCursor);
     return;
    }

    _routeNbr1 = sql->addAltRoute(ui->txtNewRouteNbr1->text(), cd->routePrefix);
    RouteData rd =  RouteData();
    rd = routeDataList.at(ui->cbRoutes->currentIndex());

    routes = sql->getRouteDataForRouteName(_routeNbr1, ui->txtNewRouteName1->text());
    if (!routes.isEmpty())
    {
     //foreach (routeData rd1 in routes)
     for(int i = 0; i <routes.count(); i ++)
     {
      SegmentData sd1 = routes.at(i);
      if (!(ui->dateFrom1->date() > sd1.endDate() || ui->dateTo1->date() < sd1.startDate()))
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

    _routeNbr2 = sql->addAltRoute(ui->txtNewRouteNbr2->text(),cd->routePrefix);
    routes = sql->getRouteDataForRouteName(_routeNbr2, ui->txtNewRouteName2->text());
    if (!routes.isEmpty())
    {
     //foreach (routeData rd2 in routes)
     for(int i = 0; i <routes.count(); i ++)
     {
      SegmentData sd2 = routes.at(i);
      if (!(ui->dateFrom2->date() > sd2.endDate() || ui->dateTo1->date() < sd2.startDate()))
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

    QList<SegmentData> myArray = sql->getRouteSegmentsForDate(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"));

    //sql->BeginTransaction("SplitRoute");
    for(int i = 0; i <myArray.count(); i ++)
    {
     SegmentData sd1 = myArray.at(i);
     if(ui->chkDeleteOriginal->isChecked())
     {
      if(!sql->deleteRouteSegment(sd1.route(), sd1.routeName(), sd1.segmentId(), sd1.startDate().toString("yyyy/MM/dd"),
                                  sd1.endDate().toString("yyyy/MM/dd")))
      {
       ui->lblHelp->setText (tr("delete failed"));
       QApplication::beep();
       setCursor(Qt::ArrowCursor);
       sql->rollbackTransaction("SplitRoute");
       return;
      }
     }

     // add back if original has an earlier start date.
     if(sd1.startDate() < ui->dateFrom1->date()  && sd1.endDate() < ui->dateFrom2->date())
     {
      if (sql->addSegmentToRoute(_routeNbr1, ui->txtNewRouteName1->text(),
                                 sd1.startDate(), ui->dateTo1->date(),
                                 sd1.segmentId(), _companyList.at(ui->cbCompany1->currentIndex())->companyKey,
                                 sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(), sd1.normalEnter(), sd1.normalLeave(),
                                 sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
      {
       ui->lblHelp->setText (tr("add failed"));
       QApplication::beep();
       setCursor(Qt::ArrowCursor);
       sql->rollbackTransaction("SplitRoute");
       return;
      }
     }

     // add back any after end date
     if(sd1.endDate() > ui->dateTo2->date() )
     {
      if (sql->addSegmentToRoute(_routeNbr1, ui->txtNewRouteName1->text(),
                                 ui->dateTo2->date(), sd1.endDate(),
                                 sd1.segmentId(), _companyList.at(ui->cbCompany1->currentIndex())->companyKey,
                                 sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(), sd1.normalEnter(), sd1.normalLeave(),
                                 sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
      {
       ui->lblHelp->setText (tr("add failed"));
       QApplication::beep();
       setCursor(Qt::ArrowCursor);
       sql->rollbackTransaction("SplitRoute");
       return;
      }

     }

     if (sql->addSegmentToRoute(_routeNbr1, ui->txtNewRouteName1->text(),
                                ui->dateFrom1->date(), ui->dateTo1->date(),
                                sd1.segmentId(), _companyList.at(ui->cbCompany1->currentIndex())->companyKey,
                                sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(), sd1.normalEnter(), sd1.normalLeave(),
                                sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
     {
      ui->lblHelp->setText (tr("add failed"));
      QApplication::beep();
      setCursor(Qt::ArrowCursor);
      sql->rollbackTransaction("SplitRoute");
      return;
     }

     if (sql->addSegmentToRoute(_routeNbr2, ui->txtNewRouteName2->text(),
                                ui->dateFrom2->date(), ui->dateTo2->date(),
                                sd1.segmentId(), _companyList.at(ui->cbCompany2->currentIndex())->companyKey,
                                sd1.tractionType(), sd1.direction(), sd1.next(), sd1.prev(), sd1.normalEnter(), sd1.normalLeave(),
                                sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
     {
      ui->lblHelp->setText (tr("add failed"));
      QApplication::beep();
      setCursor(Qt::ArrowCursor);
      sql->rollbackTransaction("SplitRoute");
      return;
     }
     _newRoute.route = _routeNbr2;
     _newRoute.name = ui->txtNewRouteName2->text();
     _newRoute.startDate = ui->dateFrom2->date();
     _newRoute.endDate = ui->dateTo2->date();
     _newRoute.companyKey = _companyList.at(ui->cbCompany2->currentIndex())->companyKey;
     _newRoute.tractionType = sd1.tractionType();

    }
    sql->commitTransaction("SplitRoute");

    this->accept();
    setCursor(Qt::ArrowCursor);
    this->close();
}

void SplitRoute::Cancel_Click()
{
    this->reject();
    this->close();
}

void SplitRoute::txtNewRouteNbr2_TextChanged(QString text)
{
    Q_UNUSED(text)
    bRoute2Changed = true;

}
