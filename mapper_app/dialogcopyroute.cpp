#include "dialogcopyroute.h"
#include "ui_dialogcopyroute.h"
#include "clipboard.h"

DialogCopyRoute::DialogCopyRoute(RouteData rd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCopyRoute)
{
    setCursor(Qt::WaitCursor);
    config = Configuration::instance();
    //sql->setConfig(cfg);
    sql = SQL::instance();
    ui->setupUi(this);
    bRouteChanged = false;
    fillCompanies();
    _rd = rd;
    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(rd.companyKey()));
    cd = sql->getCompany(ui->cbCompany->currentData().toInt());
    if(cd->companyKey > 0)
    {
        ui->dateEnd->setDate(cd->endDate);
    }
    ui->dateStart->setDate(_rd.endDate().addDays(1));
    QDate nextEndDate = sql->getNextStartOrEndDate(_rd.route(), ui->dateStart->date(), _rd.segmentId(),true);

    if(nextEndDate < cd->endDate)
    {
        maxEndDate = nextEndDate.addDays(-1);
        ui->dateEnd->setDate(maxEndDate);
    }
    _rd2 = new RouteData(_rd);
    _rd2->setCompanyKey(cd->companyKey);
    ui->rnw->configure(_rd2, ui->lblHelp);
    ui->rnw->setRouteName(_rd.routeName());
    refreshRoutes();
    // int ix = ui->cbRoutes->findData(QVariant::fromValue(_rd));
    // ui->cbRoutes->setCurrentIndex(ix);
    ui->txtRouteName->setText(_rd.toString());
    fillTractionTypes();
    ui->lblHelp->setText("");

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(btnOK_Click()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(btnCancel_Click()));
    connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompany_SelectedIndexChanged(int)));
    connect(ui->dateStart, SIGNAL(dateChanged(QDate)), this, SLOT(dateStart_ValueChanged()));
    connect(ui->dateEnd, SIGNAL(dateChanged(QDate)), this, SLOT(dateEnd_ValueChanged()));
    connect(ui->rnw, SIGNAL(rdSelected(RouteData)), this, SLOT(on_rdSelected(RouteData)));
    connect(ui->rnw, &RouteNameWidget::routeNumberChange, this, [=](int newRoute){
        maxEndDate = cd->endDate;
        QDate nextStartDate = sql->getNextStartOrEndDate(newRoute, ui->dateStart->date(),0, true);
        if(nextStartDate < cd->endDate)
        {
            maxEndDate = nextStartDate.addDays(-1);
            ui->dateEnd->setDate(maxEndDate);
        }

    });
    setCursor(Qt::ArrowCursor);
}

DialogCopyRoute::~DialogCopyRoute()
{
    delete ui;
}


void DialogCopyRoute::refreshRoutes()
{
    //SQL mySql;
    //ui->cbRoutes->clear();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.count()== 0)
        return;
    //foreach (routeData rd in routeDataList)

    int companyKey = ui->cbCompany->currentData().toInt();
    QString companyName = ui->cbCompany->currentText();
    ui->rnw->clear();
    for(int i=0; i<routeDataList.count(); i++)
    {
        RouteData rd = routeDataList.at(i);
        if(rd.companyKey() == companyKey)
            //ui->cbRoutes->addItem(rd.toString(),QVariant::fromValue(rd));
            ui->rnw->setRouteName(rd);
    }
    //ui->cbRoutes->currentText() = "";

}
/// <summary>
/// Sets a reference to the current configuration
/// </summary>
void DialogCopyRoute::setConfiguration(Configuration *value) { config = value; }

void DialogCopyRoute::fillCompanies()
{
    ui->cbCompany->clear();
    ui->cbNewCompany->clear();
    //ui->cbCompany->setcurrentText = " ";
    _companyList = sql->getCompanies();
    if (_companyList.count()==0)
        return;
    //foreach (companyData cd in _companyList)
    for(int i = 0; i<_companyList.count(); i++)
    {
     ui->cbCompany->addItem(((CompanyData*)_companyList.at(i))->toString(), _companyList.at(i)->companyKey);
     ui->cbNewCompany->addItem(((CompanyData*)_companyList.at(i))->toString(), _companyList.at(i)->companyKey);
    }
    //cbCompany.Text = "";
}

void DialogCopyRoute::setCompany(int companyKey)
{
    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(companyKey));
    refreshRoutes();
}

void DialogCopyRoute::fillTractionTypes()
{
    ui->cbTractionType->clear();
//    cbTractionType.Text = " ";
    tractionList = sql->getTractionTypes();
    if ( tractionList.count() == 0)
        return;
    //foreach (tractionTypeInfo tti in tractionList)
    for(int i = 0; i < tractionList.count(); i++)
    {
        ui->cbTractionType->addItem(((TractionTypeInfo)tractionList.values().at(i)).ToString());
    }
    //cbTractionType.Text = "";
}

//routeData RouteData
void DialogCopyRoute::setRouteData(RouteData rd)
{
    _rd = rd;
//    ui->txtRouteNbr->setText(_rd.alphaRoute());
//    ui->txtRouteName->setText(_rd.routeName());
    ui->rnw->configure(&rd, ui->lblHelp);
    bRouteChanged = true;

    ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(rd.companyKey()));

    if (routeDataList.count()== 0)
        refreshRoutes();
    //foreach (routeData rd in routeDataList)
    for(int i=0; i < routeDataList.count(); i++)
    {
        RouteData rd = (RouteData)routeDataList.at(i);
        if (rd.route() == _rd.route() && rd.routeName() == _rd.routeName() && rd.endDate() == _rd.endDate())
        {
            //ui->cbRoutes->setCurrentIndex(i);
            //ui->txtRouteName->setFocus();
            ui->dateStart->setDate( rd.startDate());
            ui->dateEnd->setDate( rd.endDate());
            int j=0;
            foreach(CompanyData* cd, _companyList)
            {
                if( cd->companyKey == rd.companyKey())
                {
                    ui->cbCompany->setCurrentIndex(j);
                    break;
                }
                j++;
            }

            break;
        }
    }
}

RouteData DialogCopyRoute::getRouteData()
{
    return _rd;
}
#if 0
void DialogCopyRoute::txtRouteNbr_TextChanged(QString text)      // SLOT
{
    Q_UNUSED(text)
    bRouteChanged = true;
}
void DialogCopyRoute::txtRouteNbr_Leave()      // SLOT
{
    ui->lblHelp->setText("");
    bool bAlphaRoute = false;
    bNewRouteNbr = false;

    int companyKey = ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    if(ui->txtRouteNbr->text().contains(","))
    {
     int nxt = sql->findNextRouteInRange(ui->txtRouteNbr->text());
     if(nxt >= 0)
     {
      ui->txtRouteNbr->setText(QString::number(nxt));
     }
    }
    qint32 newRoute = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
    if(newRoute < 0)
    {
        QMessageBox::StandardButtons rslt;
        rslt = QMessageBox::warning(this,tr("Route number not found"),
                                    tr( "The route number was not found. Enter Yes to add it"), QMessageBox::Yes | QMessageBox::No);
        switch (rslt)
        {
            case QMessageBox::Yes:
                bNewRouteNbr = true;
                bool bok;
                newRoute = ui->txtRouteNbr->text().toInt(&bok);
                if(!bok)
                 newRoute = sql->findNextRouteInRange("1000,1099");
                break;
            case QMessageBox::No:
                break;
            default:
                return;
        }
    }
//    if (newRoute != _routeNbr)
//    {
//        ui->cbSegments->setCurrentIndex(-1);
//    }
     bAddMode = false;

    _routeNbr = newRoute;
    if (!config->currCity->bAlphaRoutes && bAlphaRoute)
    {
     ui->lblHelp->setText(tr( "Must be a number!"));
     //System.Media.SystemSounds.Asterisk.Play();
     ui->txtRouteNbr->setFocus();
     return;
    }
    ui->lblHelp->setText("");
    //           fillComboBox();
    QList<RouteData> myArray = sql->getRouteInfo(_routeNbr);
    if ( myArray.count() == 0)
    {
        ui->txtRouteName->setText( "");
        ui->txtRouteName->setFocus();
    }
    else
    {
        //foreach (routeData rd in myArray)
        for(int i=0; i < myArray.count(); i++)
        {
            RouteData rd =myArray.at(i);
            ui->txtRouteName->setText(rd.routeName());
            if (bRouteChanged)
            {
                ui->dateStart->setDate( rd.startDate());
                ui->dateEnd->setDate( rd.endDate());
                //cbTractionType.SelectedIndex = rd.tractionType;
                //int ix = cbTractionType.FindString(rd.tractionType.ToString(), -1);
                //cbTractionType.SelectedIndex = ix;
                for(int j =0; j< tractionList.count(); j++)
                {
                    TractionTypeInfo tti = tractionList.values().at(j);
                    if(tti.tractionType == rd.tractionType())
                    {
                        ui->cbTractionType->setCurrentIndex(j);
                        break;
                    }
                }

                qint32 companyKey = sql->getRouteCompany(_routeNbr);
                //foreach (companyData cd in _companyList)
                for(int j=0; j < _companyList.count(); j++)
                {
                    cd = _companyList.at(j);
                    if (companyKey == cd->companyKey)
                    {
                        ui->cbCompany->setCurrentIndex(j);
                        //cbCompany.SelectedText = cd.ToString();
                        break;
                    }
                }
            }
            bRouteChanged = false;
            break;
        }
    }
}
#endif
void DialogCopyRoute::btnOK_Click()      // SLOT
{
    ui->lblHelp->setText("");
    //QList<RouteData> myArray;
    QList<SegmentData*> segmentDataList;
    ui->lblHelp->setText("");
    bool bAlphaRoute = false;
    int companyKey = ui->cbNewCompany->itemData(ui->cbNewCompany->currentIndex()).toInt();
    CompanyData* cd = sql->getCompany(companyKey);
    if(ui->dateStart->date() < cd->startDate || ui->dateEnd->date() > cd->endDate)
    {
     ui->lblHelp->setText(tr("check that dates are valid for company!"));
        QApplication::beep();
     return;
    }
    //qint32 newRoute = sql->getNumericRoute(ui->rnw->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
    qint32 newRoute = ui->rnw->newRoute();
//    if(!bNewRouteNbr && newRoute <=0)
//     return;
    if(ui->rnw->newRouteName().isEmpty())
    {
        ui->lblHelp->setText(tr("Enter a route name"));
        //ui->txtRouteName->setFocus();
        QApplication::beep();
        return;
    }
    // if (ui->cbRoutes->currentIndex() == -1)
    // {
    //      ui->lblHelp->setText(tr("Select a route"));
    //     //System.Media.SystemSounds.Asterisk.Play();
    //     ui->cbRoutes->setFocus();
    //     QApplication::beep();

    //     return;
    // }
    // if(ui->rnw->newRouteName() == "")
    // {
    //     ui->lblHelp->setText(tr("Enter a route number"));
    //     //System.Media.SystemSounds.Asterisk.Play();
    //     ui->txtRouteName->setFocus();
    //     QApplication::beep();
    //     return;
    // }
    if(ui->dateStart->date() > ui->dateEnd->date())
    {
        ui->lblHelp->setText(tr("Date error. Start date after end date"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->dateStart->setFocus();
        QApplication::beep();
        return;
    }
    if(ui->dateEnd->date() > maxEndDate)
    {
        ui->lblHelp->setText(tr("Date error. End date must not be ;ater than %1").arg(maxEndDate.toString("yyyy/MM/dd")));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->dateStart->setFocus();
        QApplication::beep();
        return;
    }
    if(ui->rnw->routeNbrMustBeAdded())
     _routeNbr = sql->addAltRoute(ui->rnw->alphaRoute(), cd->routePrefix);
    else
     _routeNbr = ui->rnw->newRoute();

    RouteData rd;
    rd.setRoute(_routeNbr);
    rd.setRouteName(ui->rnw->newRouteName());
    rd.setStartDate(ui->dateStart->date());
    rd.setEndDate(ui->dateEnd->date());
    rd.setCompanyKey(ui->cbCompany->currentData().toInt());
    QList<SegmentData*> conflicts = sql->getConflictingRouteSegments(rd);
    if(!conflicts.isEmpty())
    {
        SegmentData* sd = conflicts.at(0);
        ui->lblHelp->setText(tr("dates conflict with route %1 %2 %3-%4").arg(sd->route())
                                 .arg(sd->routeName(), sd->startDate().toString("yyyy/MM/dd"),
                                     sd->endDate().toString("yyy/MM/dd")));
        qCritical() << ui->lblHelp->text();

        //System.Media.SystemSounds.Asterisk.Play();
        ui->dateStart->setFocus();
        QApplication::beep();
        return;
    }


    //qint32 ix = ui->cbRoutes->currentIndex();
    //rd = routeDataList.at(ix);

    //myArray = sql->getRouteDatasForDate(rd.route(), rd.routeName(), rd.endDate().toString("yyyy/MM/dd"));
    segmentDataList = sql->getSegmentDataList(_rd);

    int rslt = QMessageBox::question(nullptr, tr("Confirm copy"),
                                     tr("Click on Ok to copy %1 segments from:\n%2 to \n%3 %4-%5")
                                         .arg(segmentDataList.count())
                                         .arg(_rd.toString(),ui->rnw->newRouteName(),ui->dateStart->date().toString("yyyy/MM/dd"),
                                             ui->dateEnd->date().toString("yyyy/MM/dd") ),
                                     QMessageBox::Ok |QMessageBox::Cancel);
    if(rslt == QMessageBox::Cancel)
        this->reject();

//    sql->BeginTransaction("CopyRoute");
    setCursor(Qt::WaitCursor);
    qint32  tractionType = (tractionList.values().at(ui->cbTractionType->currentIndex())).tractionType;

    if(ui->dateStart->date() > _rd.endDate() || ui->dateEnd->date() < _rd.startDate())
    {
        setCursor(Qt::WaitCursor);
        //for (int i =0; i < myArray.count(); i++)
        for(SegmentData* sd : segmentDataList)
        {
            qApp->processEvents();
         sd->setRoute(_routeNbr);
         sd->setRouteName(ui->rnw->newRouteName());
         sd->setStartDate(ui->dateStart->date());
         sd->setEndDate(ui->dateEnd->date());
         sd->setTractionType(tractionType);
         sd->setCompanyKey(companyKey);
         if(sql->doesRouteSegmentExist(*sd))
             continue;
         if(!sql->addSegmentToRoute(sd))
         {
             ui->lblHelp->setText(tr("add failed"));
             //System.Media.SystemSounds.Asterisk.Play();
             QApplication::beep();
             setCursor(Qt::ArrowCursor);
             return;
         }
        }
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        setCursor(Qt::WaitCursor);
        for(SegmentData* sd : segmentDataList)
        {
            qApp->processEvents();
            QDate nextStartDate = sql->getNextStartOrEndDate(sd->route(), sd->startDate(), sd->segmentId(), true);
            if(nextStartDate.isValid() && nextStartDate> sd->endDate())
                sd->setEndDate(nextStartDate.addDays(-1));
            SegmentData* sd2 = sql->getConflictingSegmentDataForRoute(_routeNbr,ui->rnw->newRouteName(),
                                                        sd->segmentId(),
                                                        ui->dateStart->date().toString("yyyy/MM/dd"),
                                                        ui->dateEnd->date().toString("yyyy/MM/dd"));
            if (sd2 == nullptr)
            {
                if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->newRouteName(), sd->segmentId(),
                                               ui->dateStart->date(), ui->dateEnd->date()))
                {
                    sql->deleteRouteSegment(_routeNbr, /*ui->rnw->newRouteName()*/sd->routeId(), sd->segmentId(), ui->dateStart->text(), ui->dateEnd->text());
                }
                sd->setRoute(_routeNbr);
                sd->setRouteName(ui->rnw->newRouteName());
                sd->setStartDate(ui->dateStart->date());
                sd->setEndDate(ui->dateEnd->date());
                sd->setTractionType(tractionType);
                sd->setCompanyKey(((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey);
                if(!sql->addSegmentToRoute(sd))
                {
                    ui->lblHelp->setText(tr("add failed"));
                    QApplication::beep();
                    setCursor(Qt::ArrowCursor);
                    return;
                }
            }
            else
            {
                // an existing segment exists for this date range.
                // first, delete the existing segment from the route
                if (!sql->deleteRouteSegment(_routeNbr, /*ui->rnw->newRouteName()*/sd2->routeId(), sd2->segmentId(),
                                             sd2->startDate().toString("yyyy/MM/dd"),
                                             sd2->endDate().toString("yyyy/MM/dd")))
                {
                    ui->lblHelp->setText(tr("deleteRoute failed!"));
                    QApplication::beep();
                    setCursor(Qt::ArrowCursor);
                    return;
                }
                // now add back the portion of the route before the current date range
                if (sd2->startDate() < ui->dateStart->date())
                {
                    sd2->setRoute(_routeNbr);
                    sd2->setRouteName(ui->rnw->newRouteName());
                    sd2->setEndDate(ui->dateStart->date().addDays(-1));
                    if(!sql->addSegmentToRoute(sd2))
                    {
                        ui->lblHelp->setText(tr("add failed"));
                        QApplication::beep();
                        setCursor(Qt::ArrowCursor);
                        return;
                    }
                }
                // now add back any portion of the route after the current date range
                if (sd2->endDate() > ui->dateEnd->date())
                {
                    sd2->setRoute(_routeNbr);
                    sd2->setRouteName(ui->rnw->newRouteName());
                    sd2->setStartDate(ui->dateEnd->date().addDays(1));
                    if(!sql->addSegmentToRoute(sd2))
                    {
                        ui->lblHelp->setText(tr("add failed"));
                        QApplication::beep();
                        setCursor(Qt::ArrowCursor);
                        return;
                    }
                }
                // now add the segment for the current date range
                if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->newRouteName(), sd->segmentId(),
                                               ui->dateStart->date(), ui->dateEnd->date()))
                {
                    sql->deleteRouteSegment(_routeNbr, /*ui->rnw->newRouteName()*/sd2->routeId(), sd2->segmentId(),
                                            ui->dateStart->text(), ui->dateEnd->text());
                }
                sd->setRoute(_routeNbr);
                sd->setRouteName(ui->rnw->newRouteName());
                sd->setStartDate(ui->dateStart->date());
                sd->setEndDate(ui->dateEnd->date());
                sd->setCompanyKey(((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey);

                sd->setTractionType(tractionType);
                if(!sql->addSegmentToRoute(sd))
                {
                    ui->lblHelp->setText(tr("add failed"));
                    QApplication::beep();
                    setCursor(Qt::ArrowCursor);
                    return;
                }
            }
        }
        setCursor(Qt::ArrowCursor);
    }
    //sql->CommitTransaction("CopyRoute");
    //sql->myConnection.Close();

    _rd.setRoute(_routeNbr);
    _rd.setRouteName(ui->rnw->newRouteName());
    _rd.setStartDate(ui->dateStart->date());
    _rd.setEndDate(ui->dateEnd->date());
    _rd.setCompanyKey( ((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey);
    _rd.setTractionType( ((TractionTypeInfo)tractionList.values().at(ui->cbTractionType->currentIndex())).tractionType);

    this->accept();
    this->close();
}

void DialogCopyRoute::cbCompany_SelectedIndexChanged(int row) // SLOT
{
    Q_UNUSED(row)
    ui->lblHelp->setText("");
    //companyData cd = sql->getCompany(cbCompany.SelectedIndex);
    cd = _companyList.at(ui->cbCompany->currentIndex());
    if (cd->name == "")
        return;
    ui->rnw->setCompanyKey(cd->companyKey);

//    if (ui->dateStart->date() < cd->startDate)
     ui->dateStart->setDate(cd->startDate);
//    if (ui->dateEnd->date() > cd->endDate)
     ui->dateEnd->setDate(cd->endDate);
     maxEndDate = cd->endDate;

     refreshRoutes();
}

void DialogCopyRoute::dateStart_ValueChanged()   //SLOT
{
    ui->lblHelp->setText("");
    cd = _companyList.at(ui->cbNewCompany->currentIndex());
    if(ui->dateStart->date() < cd->startDate)
    {
        ui->lblHelp->setText(tr("start date is prior to company's start date"));
        QApplication::beep();
        ui->dateStart->setFocus();
    }
    if(ui->dateStart->date() > ui->dateEnd->date())
    {
        ui->lblHelp->setText(tr("start date is after end date"));
        QApplication::beep();
        ui->dateStart->setFocus();

    }
}
void DialogCopyRoute::dateEnd_ValueChanged()   //SLOT
{
    ui->lblHelp->setText("");
    cd = _companyList.at(ui->cbNewCompany->currentIndex());
    if(ui->dateStart->dateTime() > ui->dateEnd->dateTime())
    {
        ui->lblHelp->setText(tr("end date must be after company start date (")+ cd->startDate.toString("yyyy/MM/dd")+")");
        QApplication::beep();
        ui->dateStart->setFocus();
    }
    if(ui->dateEnd->date() > cd->endDate)
    {
        ui->lblHelp->setText(tr("end date is after to company's end date (")+ cd->endDate.toString("yyyy/MM/dd")+")");
        QApplication::beep();
        ui->dateEnd->setFocus();
    }
    if(ui->dateStart->date() > ui->dateEnd->date())
    {
        ui->lblHelp->setText(tr("start date is after end date"));
        QApplication::beep();
        ui->dateStart->setFocus();
    }

}

void DialogCopyRoute::btnCancel_Click()      // SLOT
{
    this->reject();
    this->close();
}

void DialogCopyRoute::on_rdSelected(RouteData rd)
{
    ui->dateStart->setDate(rd.startDate());
    ui->dateEnd->setDate(rd.endDate());
}
