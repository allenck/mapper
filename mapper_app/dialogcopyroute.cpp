#include "dialogcopyroute.h"
#include "ui_dialogcopyroute.h"

DialogCopyRoute::DialogCopyRoute(Configuration *cfg,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCopyRoute)
{
    config = cfg;
    //sql->setConfig(cfg);
    sql = SQL::instance();
    ui->setupUi(this);
    bRouteChanged = false;
    refreshRoutes();
    fillCompanies();
    fillTractionTypes();
    ui->lblHelp->setText("");

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(btnOK_Click()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(btnCancel_Click()));
    connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompany_SelectedIndexChanged(int)));
    connect(ui->txtRouteNbr, SIGNAL(textChanged(QString)), this, SLOT(txtRouteNbr_TextChanged(QString)));
    connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbr_Leave()));
    connect(ui->dateStart, SIGNAL(dateChanged(QDate)), this, SLOT(dateStart_ValueChanged()));
    connect(ui->dateEnd, SIGNAL(dateChanged(QDate)), this, SLOT(dateEnd_ValueChanged()));
    ui->txtRouteNbr->setMaxLength(9);
}

DialogCopyRoute::~DialogCopyRoute()
{
    delete ui;
}

void DialogCopyRoute::refreshRoutes()
{
    //SQL mySql;
    ui->cbRoutes->clear();
    routeDataList = sql->getRoutesByEndDate();
    if (routeDataList.count()== 0)
        return;
    //foreach (routeData rd in routeDataList)
    for(int i=0; i<routeDataList.count(); i++)
    {
        ui->cbRoutes->addItem(((RouteData)routeDataList.at(i)).toString());
    }
    ui->cbRoutes->currentText() = "";
}
/// <summary>
/// Sets a reference to the current configuration
/// </summary>
void DialogCopyRoute::setConfiguration(Configuration *value) { config = value; }

void DialogCopyRoute::fillCompanies()
{
    ui->cbCompany->clear();
    //ui->cbCompany->setcurrentText = " ";
    _companyList = sql->getCompanies();
    if (_companyList.count()==0)
        return;
    //foreach (companyData cd in _companyList)
    for(int i = 0; i<_companyList.count(); i++)
    {
     ui->cbCompany->addItem(((CompanyData*)_companyList.at(i))->toString(), _companyList.at(i)->companyKey);
    }
    //cbCompany.Text = "";

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
void DialogCopyRoute::setRouteData(RouteData value)
{
    _rd = value;
    ui->txtRouteNbr->setText(_rd.alphaRoute);
    ui->txtRouteName->setText(_rd.name);
    bRouteChanged = true;

    if (routeDataList.count()== 0)
        refreshRoutes();
    //foreach (routeData rd in routeDataList)
    for(int i=0; i < routeDataList.count(); i++)
    {
        RouteData rd = (RouteData)routeDataList.at(i);
        if (rd.route == _rd.route && rd.name == _rd.name && rd.endDate == _rd.endDate)
        {
            ui->cbRoutes->setCurrentIndex(i);
            ui->txtRouteName->setFocus();
            ui->dateStart->setDate( rd.startDate);
            ui->dateEnd->setDate( rd.endDate);
            int j=0;
            foreach(CompanyData* cd, _companyList)
            {
                if( cd->companyKey == rd.companyKey)
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
void DialogCopyRoute::txtRouteNbr_TextChanged(QString text)      // SLOT
{
    Q_UNUSED(text)
    bRouteChanged = true;
}
void DialogCopyRoute::txtRouteNbr_Leave()      // SLOT
{
    //try
    //{
    //    _routeNbr = Convert.ToInt32(txtRouteNbr.Text);
    //}
    //catch (Exception e1)
    //{
    //    lblHelp.Text = e1.Message;
    //    return;
    //}
    ui->lblHelp->setText("");
    bool bAlphaRoute = false;
    int companyKey = ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    qint32 newRoute = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);

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
            ui->txtRouteName->setText(rd.name);
            if (bRouteChanged)
            {
                ui->dateStart->setDate( rd.startDate);
                ui->dateEnd->setDate( rd.endDate);
                //cbTractionType.SelectedIndex = rd.tractionType;
                //int ix = cbTractionType.FindString(rd.tractionType.ToString(), -1);
                //cbTractionType.SelectedIndex = ix;
                for(int j =0; j< tractionList.count(); j++)
                {
                    TractionTypeInfo tti = tractionList.values().at(j);
                    if(tti.tractionType == rd.tractionType)
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

void DialogCopyRoute::btnOK_Click()      // SLOT
{
    ui->lblHelp->setText("");
    QList<SegmentData> myArray;
    ui->lblHelp->setText("");
    bool bAlphaRoute = false;
    int companyKey = ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    qint32 newRoute = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
    if(newRoute <=0)
     return;
    if(ui->txtRouteName->text().isEmpty())
    {
        ui->lblHelp->setText(tr("Enter a route name"));
        ui->txtRouteName->setFocus();
        QApplication::beep();
        return;
    }
    if (ui->cbRoutes->currentIndex() == -1)
    {
         ui->lblHelp->setText(tr("Select a route"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->cbRoutes->setFocus();
        QApplication::beep();

        return;
    }
    if(ui->txtRouteNbr->text() == "")
    {
        ui->lblHelp->setText(tr("Enter a route number"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->cbRoutes->setFocus();
        QApplication::beep();

        return;
    }
    if(ui->dateStart->date() > ui->dateEnd->date())
    {
        ui->lblHelp->setText(tr("Date error. Start date after end date"));
        //System.Media.SystemSounds.Asterisk.Play();
        ui->dateStart->setFocus();
        QApplication::beep();

        return;
    }
    _routeNbr = sql->addAltRoute(ui->txtRouteNbr->text(), cd->routePrefix);
    RouteData rd;
    qint32 ix = ui->cbRoutes->currentIndex();
    rd = routeDataList.at(ix);

    //if (sql->doesRouteExist(_routeNbr, txtRouteName.Text, dateStart.Text, dateEnd.Text))
    //{
    //    lblHelp.Text = "Route already exists";
    //    System.Media.SystemSounds.Asterisk.Play();
    //    return;
    //}

    myArray = sql->getRouteSegmentsForDate(rd.route, rd.name, rd.endDate.toString("yyyy/MM/dd"));

//    sql->OpenConnection();
//    sql->BeginTransaction("CopyRoute");
    qint32  tractionType = (tractionList.values().at(ui->cbTractionType->currentIndex())).tractionType;
    //foreach (routeData rd1 in myArray)

    if(ui->dateStart->date() > _rd.endDate || ui->dateEnd->date() < _rd.startDate)
    {
        for (int i =0; i < myArray.count(); i++)
        {
            SegmentData sd1 = myArray.at(i);
            if (sql->addSegmentToRoute(_routeNbr, ui->txtRouteName->text(), ui->dateStart->date(),
                                       ui->dateEnd->date(), sd1.segmentId(),
                                       ((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey,
                                       /*cbTractionType.SelectedIndex + 1*/tractionType, sd1.direction(), sd1.next(), sd1.prev(),
                                       sd1.normalEnter(), sd1.normalLeave(), sd1.reverseEnter(), sd1.reverseLeave(),
                                       sd1.oneWay(), sd1.trackUsage()) == false)
            {
                ui->lblHelp->setText(tr("add failed"));
                //System.Media.SystemSounds.Asterisk.Play();
                QApplication::beep();

                return;
            }
        }
    }
    else
    for (int i =0; i < myArray.count(); i++)
    {
        SegmentData sd1 = myArray.at(i);
        SegmentData sd2 = sql->getSegmentInfoForRouteDates(rd.route, ui->txtRouteName->text(), sd1.segmentId(),
                                                           sd1.startDate().toString("yyyy/MM/dd"),
                                                           sd1.endDate().toString("yyyy/MM/dd"));
        if (sd2.route() < 0)
        {
            if (sql->doesRouteSegmentExist(_routeNbr, ui->txtRouteName->text(), sd2.segmentId(), ui->dateStart->date(), ui->dateEnd->date()))
            {
                sql->deleteRouteSegment(_routeNbr, ui->txtRouteName->text(), sd2.segmentId(), ui->dateStart->text(), ui->dateEnd->text());
            }

            if (sql->addSegmentToRoute(_routeNbr, ui->txtRouteName->text(), ui->dateStart->date(), ui->dateEnd->date(),
                                       sd1.segmentId(),
                                       ((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey,
                                       /*cbTractionType.SelectedIndex + 1*/tractionType, sd1.direction(),
                                       sd1.next(), sd1.prev(),
                                       sd1.normalEnter(), sd1.normalLeave(), sd1.reverseEnter(), sd1.reverseLeave(),
                                       sd1.oneWay(), sd1.trackUsage()) == false)
            {
                ui->lblHelp->setText(tr("add failed"));
                //System.Media.SystemSounds.Asterisk.Play();
                return;
            }
        }
        else
        {
            // an existing segment exists for this date range.
            // first, delete the existing segment from the route
            if (!sql->deleteRouteSegment(_routeNbr, ui->txtRouteName->text(), sd2.segmentId(),
                                         sd2.startDate().toString("yyyy/MM/dd"), sd2.endDate().toString("yyyy/MM/dd")))
            {
                ui->lblHelp->setText(tr("deleteRoute failed!"));
                //System.Media.SystemSounds.Beep.Play();
                //sql->myConnection.Close();
                return;
            }
            // now add back the portion of the route before the current date range
            if (sd2.startDate() < ui->dateStart->date())
            {
                if (sql->addSegmentToRoute(_routeNbr, ui->txtRouteName->text(),
                    sd2.startDate(), (ui->dateStart->date().addDays(-1)),
                    sd2.segmentId(), sd2.companyKey(), sd2.tractionType(), sd2.direction(),
                                           sd1.next(), sd1.prev(), sd2.normalEnter(),
                    sd2.normalLeave(), sd2.reverseEnter(), sd2.reverseLeave(), sd2.oneWay(), sd2.trackUsage()) == false)
                {
                    ui->lblHelp->setText(tr("add failed"));
                    //System.Media.SystemSounds.Asterisk.Play();
                    return;
                }
            }
            // now add back any portion of the route after the current date range
            if (sd2.endDate() > ui->dateEnd->date())
            {
                if (sql->addSegmentToRoute(_routeNbr, ui->txtRouteName->text(),
                    (ui->dateEnd->date().addDays(1)), sd2.endDate(),
                    sd2.segmentId(), sd2.companyKey(), sd2.tractionType(), sd2.direction(), sd2.next(), sd2.prev(),
                    sd2.normalEnter(), sd2.normalLeave(), sd2.reverseEnter(), sd2.reverseLeave(), sd2.oneWay(), sd2.trackUsage()) == false)
                {
                    ui->lblHelp->setText(tr("add failed"));
                    //System.Media.SystemSounds.Asterisk.Play();
                    return;
                }
            }
            // now add the segment for the current date range
            if (sql->doesRouteSegmentExist(_routeNbr, ui->txtRouteName->text(), sd1.segmentId(), ui->dateStart->date(), ui->dateEnd->date()))
            {
                sql->deleteRouteSegment(_routeNbr, ui->txtRouteName->text(), sd2.segmentId(), ui->dateStart->text(), ui->dateEnd->text());
            }
            if (sql->addSegmentToRoute(_routeNbr, ui->txtRouteName->text(), ui->dateStart->date(), ui->dateEnd->date(), sd1.segmentId(),
                     ((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey,
                     /*cbTractionType.SelectedIndex + 1*/tractionType, sd1.direction(), sd1.next(), sd1.prev(),
                     sd1.normalEnter(), sd1.normalLeave(), sd1.reverseEnter(), sd1.reverseLeave(), sd1.oneWay(), sd1.trackUsage()) == false)
            {
                ui->lblHelp->setText(tr("add failed"));
                //System.Media.SystemSounds.Asterisk.Play();
                return;
            }

        }

    }
    //sql->CommitTransaction("CopyRoute");
    //sql->myConnection.Close();

    _rd.route = _routeNbr;
    _rd.name = ui->txtRouteName->text();
    _rd.startDate = ui->dateStart->date();
    _rd.endDate = ui->dateEnd->date();
    _rd.companyKey = ((CompanyData*)_companyList.at(ui->cbCompany->currentIndex()))->companyKey;
    _rd.tractionType = ((TractionTypeInfo)tractionList.values().at(ui->cbTractionType->currentIndex())).tractionType;

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

//    if (ui->dateEnd->date() < cd->startDate || ui->dateStart->date() > cd->endDate)
//    {
//        ui->lblHelp->setText(tr("Company not valid for specified dates!"));
//        //System.Media.SystemSounds.Question.Play();
//        ui->dateStart->setFocus();
//    }
//    if (ui->dateStart->date() < cd->startDate)
     ui->dateStart->setDate(cd->startDate);
//    if (ui->dateEnd->date() > cd->endDate)
     ui->dateEnd->setDate(cd->endDate);
}

void DialogCopyRoute::dateStart_ValueChanged()   //SLOT
{
    ui->lblHelp->setText("");
    cd = _companyList.at(ui->cbCompany->currentIndex());
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
    cd = _companyList.at(ui->cbCompany->currentIndex());
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
