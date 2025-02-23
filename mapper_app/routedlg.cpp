#include "routedlg.h"
#include "ui_routedlg.h"
#include "segmentdescription.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "vptr.h"
#include "companyview.h"

RouteDlg::RouteDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RouteDlg)
{
    ui->setupUi(this);
    config = Configuration::instance();
    config->changeFonts(this, config->font);
    //sql->setConfig(config);
    sql = SQL::instance();
    this->setWindowTitle(tr("Update Route"));
    _routeNbr = -1;
    _segmentId = -1;
    bRouteChanged = false;
    formNotLoaded = true;
    //_normalEnter = 0, _normalLeave = 0, _reverseEnter = 0, _reverseLeave = 0;
    bSegmentChanging = false;
    bRouteChanging = false;
    bAddMode = false;
    _alphaRoute="";
    bNewRouteNbr=false;
    strNoRoute = tr("New Route Name");
    for(int i=0; i < SegmentData::ROUTETYPES.count(); i++)
    {
        ui->cbRouteType->addItem(SegmentData::ROUTETYPES.at(i),i);
    }
    //connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbr_Leave()) );
    //connect(ui->rnw, SIGNAL(routeNumberChange(int)), this, SLOT(txtRouteNbr_Leave()) );
    //connect(ui->cbRouteName, SIGNAL(signalFocusOut()), this, SLOT(txtRouteName_Leave()));
    //connect(ui->rnw, SIGNAL(routeNameChanged(QString)), this, SLOT(txtRouteName_Leave()));
    connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegments_SelectedIndexChanged(int)));
    connect(ui->gbNormalEnter, SIGNAL(toggled(bool)), this, SLOT(gbNormalEnter_Leave()));
    connect(ui->gbNormalLeave, SIGNAL(toggled(bool)), this, SLOT(gbNormalLeave_Leave()));
    connect(ui->gbReverseEnter, SIGNAL(toggled(bool)), this, SLOT(gbReverseEnter_Leave()));
    connect(ui->gbReverseLeave, SIGNAL(toggled(bool)), this, SLOT(gbReverseLeave_Leave()));
    connect(ui->btnClose,SIGNAL(clicked()), this, SLOT(btnClose_click()));
    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_Click()));
    connect(ui->btnUpdateTurn, SIGNAL(clicked()), this, SLOT(btnUpdateTurn_Click()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(btnDelete_Click()));
    connect(ui->rbNFromLeft, SIGNAL(toggled(bool)), this, SLOT(rbNFromLeft_CheckedChanged(bool)));
    connect(ui->rbNFromBack, SIGNAL(toggled(bool)), this,  SLOT(rbNFromBack_CheckedChanged(bool)));
    connect(ui->rbNFromRight, SIGNAL(toggled(bool)), this,  SLOT(rbNFromRight_CheckedChanged(bool)));
    connect(ui->rbNToLeft, SIGNAL(toggled(bool)), this,  SLOT(rbNToLeft_CheckedChanged(bool)));
    connect(ui->rbNAhead, SIGNAL(toggled(bool)), this,  SLOT(rbNAhead_CheckedChanged(bool)));
    connect(ui->rbNToRight, SIGNAL(toggled(bool)), this,  SLOT(rbNToRight_CheckedChanged(bool)));
    connect(ui->rbRFromLeft, SIGNAL(toggled(bool)), this,  SLOT(rbRFromLeft_CheckedChanged(bool)));
    connect(ui->rbRFromBack, SIGNAL(toggled(bool)), this,  SLOT(rbRFromBack_CheckedChanged(bool)));
    connect(ui->rbRFromRight, SIGNAL(toggled(bool)), this,  SLOT(rbRFromRight_CheckedChanged(bool)));
    connect(ui->rbRToLeft, SIGNAL(toggled(bool)), this,  SLOT(rbRToLeft_CheckedChanged(bool)));
    connect(ui->rbRAhead, SIGNAL(toggled(bool)), this,  SLOT(rbRAhead_CheckedChanged(bool)));
    connect(ui->rbRToRight, SIGNAL(toggled(bool)), this,  SLOT(rbRToRight_CheckedChanged(bool)));
    connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompany_SelectedIndexChanged(int )));
    connect(ui->dateEnd, SIGNAL(editingFinished()), this, SLOT(dateEnd_Leave()) );
    connect(ui->cbOneWay, SIGNAL(toggled(bool)), this, SLOT(cbOneWay_checkedChanged(bool)));
    connect(ui->rbLeft, &QRadioButton::clicked, [=]{
     ui->gbNormalEnter->setVisible(false);
     ui->gbNormalLeave->setVisible(false);
     ui->gbReverseEnter->setVisible(true);
     ui->gbReverseLeave->setVisible(true);
    });
    connect(ui->rbRight, &QRadioButton::clicked, [=]{
     ui->gbNormalEnter->setVisible(true);
     ui->gbNormalLeave->setVisible(true);
     ui->gbReverseEnter->setVisible(false);
     ui->gbReverseLeave->setVisible(false);
    });
    ui->gbUsage->setVisible(false);

//    connect(this, SIGNAL(setStartDate(QDate)), ui->dateStart, SLOT(setDate(QDate)));
//    connect(this,SIGNAL(setEndDate(QDate)), ui->dateEnd, SLOT(setDate(QDate)));
    connect(ui->dateStart, SIGNAL(editingFinished()), this, SLOT(dateStart_Leave()));
    myParent = (MainWindow*)parent;
    connect(myParent, SIGNAL(newCitySelected()), this, SLOT(OnNewCity()));
    fillCompanies();
    connect(myParent->companyView->model(), &MyCompanyTableModel::dataChanged,this,[=]{
        fillCompanies();
    });
    fillTractionTypes();

    ui->rnw->configure((SegmentData*)nullptr, ui->lblHelpText);
}

RouteDlg::~RouteDlg()
{
    delete ui;
}

//void RouteDlg::Configuration(Configuration *cfg)
//{
//    config = cfg;
//}

void RouteDlg::setSegmentId(qint32 segmentid)
{
 _segmentId = segmentid;
 bSegmentChanging = true;
 sql->updateSegment(_segmentId);
 si = sql->getSegmentInfo(_segmentId);
 if(qobject_cast<MainWindow*>(parent()))
 {
    MainWindow* main = qobject_cast<MainWindow*>(parent());
    this->sd = sql->getConflictingSegmentDataForRoute(main->m_routeNbr, main->m_routeName, _segmentId,
                                          main->m_currRouteStartDate, main->m_currRouteEndDate);
 }
 bSegmentChanging = false;

 if (sd==nullptr)
     return;
 //lblSegment.Text = sql->getSegmentDescription(_SegmentId);
 ui->lblSegmentText->setText(sd->toString());
 //fillCompanies();
 fillSegmentsComboBox(); // list of routes using this segment
 //fillTractionTypes();
 if (sd->route() >0)
 {
  //foreach (tractionTypeInfo tti in cbTractionType.Items)
  for(int i = 0; i < _tractionList.count(); i++ )
  {
   TractionTypeInfo tti = (TractionTypeInfo)_tractionList.values().at(i);
   if (tti.tractionType == sd->tractionType())
   {
       ui->cbTractionType->setCurrentIndex(i);
       break;
   }
  }
 }
 //ui->cbSegments->setFocus();
 ui->cbOneWay->setChecked(sd->oneWay() == "Y");
 ui->gbUsage->setVisible(si.tracks()==2 && sd->oneWay() == "Y");
 if (sd->oneWay() == "Y" && sd->tracks() == 1)
 {
     ui->gbDirection->setVisible(true);
     ui->gbReverseEnter->setVisible(false);
     ui->gbReverseLeave->setVisible(false);
     ui->rbNormal->setText(sd->bearing().strDirection());
     ui->rbReverse->setText( sd->bearing().strReverseDirection());
     ui->rbNormal->setChecked(true);
 }
 if(sd->oneWay() != "Y" && sd->tracks() == 1)
 {
  ui->gbDirection->setVisible(false);
  ui->gbReverseEnter->setVisible(true);
  ui->gbReverseLeave->setVisible(true);
 }
 if(sd->oneWay() == "Y" && sd->tracks() == 2)
 {
  ui->gbUsage->setVisible(true);
  ui->rbLeft->setChecked(sd->trackUsage() == "L");
  ui->rbRight->setChecked(sd->trackUsage() == "R");
 }
 else
 {
  ui->gbUsage->setVisible(false);
 }

 SegmentData* sd = sql->getSegmentData(_routeNbr, segmentid, ui->dateStart->date().toString("yyyy/MM/dd"), ui->dateEnd->date().toString("yyyy/MM/dd"));
 if(! sd || sd->segmentId() < 0)
 {
  ui->btnAdd->setText(tr("Add"));
  ui->btnAdd->setEnabled(true);
 }
 else
 {
  ui->dateStart->setDate(sd->startDate());
  ui->dateEnd->setDate(sd->endDate());
  bSegmentChanging = false;
  //ui->txtRouteNbr->setFocus();
 }
 btnUpdateTurn_Click();

 CalculateDates();
}

void RouteDlg::setRouteNbr(qint32 rt)
{
    bRouteChanging = true;
    _routeNbr = rt;
    int companyKey= ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    CompanyData* cd = sql->getCompany(companyKey);

    _alphaRoute = sql->getAlphaRoute(_routeNbr, cd->routePrefix);
    //ui->txtRouteNbr->setText(  _alphaRoute);
    _routeNamesList = sql->getRouteNames(_routeNbr);
    if (_routeNamesList.count()==0)
        return;
//    ui->cbRouteName->clear();
//    ui->cbRouteName->addItem("");
    //foreach (string str in myArray)
//    for(int i=0;i<_routeNamesList.count();i++)
//    {
//        QString str = (QString)_routeNamesList.at(i);
//        ui->cbRouteName->addItem(str);
//    }
    if (sd->route() >0  && sd->route() == _routeNbr)
    {
        companyKey = sd->companyKey();
        for(int j=0; j < _routeNamesList.count(); j++)
        {
            if(_routeNamesList.at(j)== sd->routeName())
            {
//                ui->cbRouteName->setCurrentIndex(j+1);
                break;
            }
        }
    }
    else
        companyKey = sql->getRouteCompany(_routeNbr);
    //foreach( companyData cd in _companyList)
    for(int i=0;i<_companyList.count();i++)
    {
        CompanyData* cd =(CompanyData*)_companyList.at(i);
        if(companyKey == cd->companyKey)
        {
            ui->cbCompany->setCurrentIndex(i);
           // cbCompany.SelectedText = cd.ToString();
            break;
        }
    }
    bRouteChanging = false;
}

void RouteDlg::setSegmentData(SegmentData* sd)
{
 bRouteChanging = true;
 this->sd = sd;
 this->oldSd = new SegmentData(*sd);
 _routeNbr = sd->route();
 _alphaRoute = sd->alphaRoute();
 _segmentId = sd->segmentId();
 //int count = 0;
 //bool bFound = false;
 ui->btnAdd->setEnabled(false);
// ui->txtRouteNbr->setText(sd->alphaRoute());
// ui->cbRouteName->clear();
// ui->cbRouteName->addItem(strNoRoute);
// ui->cbRouteName->setCurrentText(sd->routeName());
 ui->rnw->configure(sd, ui->lblHelpText);
 ui->rnw->setCompanyKey(sd->companyKey());
 cbOneWay_checkedChanged(sd->oneWay()=="Y");
 ui->cbOneWay->setChecked(sd->oneWay()=="Y");
 ui->gbUsage->setVisible(sd->tracks()==2 && sd->oneWay() == "Y");
 if(sd->trackUsage() == "L") ui->rbLeft->setChecked(true);
 //_segmentDataList = ((MainWindow*)parent())->segmentDataList;
 fillSegmentsComboBox();
 if(!_segmentDataList.contains(sd))
 {
  _segmentDataList.append(sd);
  ui->cbSegments->addItem(sd->toString(), VPtr<SegmentData>::asQVariant(sd));
 }
 ui->cbSegments->setCurrentIndex(ui->cbSegments->findText(sd->toString()));
 ui->lblSegmentText->setText(sd->toString2());

// for(int i=0; i < _segmentDataList.count(); i++)
// {
//  SegmentData sd1 = _segmentDataList.at(i);
//  if(sd1->startDate()== sd->startDate() && sd1->endDate() == sd->endDate())
//  {
//   cbSegments_SelectedIndexChanged(i);
//   checkUpdate(__FUNCTION__);
//   break;
//  }
// }
 _routeNamesList = sql->getRouteNames(_routeNbr);
 for(int i=0; i < _routeNamesList.count(); i++)
 {
  QString name = (QString)_routeNamesList.at(i);
//  ui->cbRouteName->addItem(name);
//  if(name == sd->routeName())
//  {
//   ui->cbRouteName->setCurrentIndex(i+1);
//  }
 }
 if(sd->companyKey()>0)
  ui->cbCompany->setCurrentIndex(ui->cbCompany->findData(sd->companyKey()));

 ui->dateStart->clearMaximumDateTime();
 ui->dateStart->clearMinimumDateTime();
 ui->dateEnd->clearMaximumDateTime();
 ui->dateEnd->clearMinimumDateTime();
 ui->dateStart->setDate(sd->startDate());
 ui->dateEnd->setDate(sd->endDate());
 displayDates(__FUNCTION__);

 ui->gbUsage->setVisible(sd->oneWay() == "Y" && sd->tracks() == 2);

 //foreach (tractionTypeInfo tti in cbTractionType.Items)
 for(int i=0; i < _tractionList.count(); i++)
 {
  TractionTypeInfo tti = (TractionTypeInfo)_tractionList.values().at(i);
  if (tti.tractionType == sd->tractionType())
  {
   ui->cbTractionType->setCurrentIndex(i);
   break;
  }
 }
 btnUpdateTurn_Click();
 bRouteChanging = false;
 bAddMode = false;
 checkUpdate(__FUNCTION__);
 ui->cbCompany->setFocus();
}

/// <summary>
/// Selects a specific route to edit.
/// </summary>
void RouteDlg::setSegmentData(RouteData rd)
{
 bRouteChanging = true;
 this->_rd = rd;
 _routeNbr = _rd.route();
 _alphaRoute = _rd.alphaRoute();
 //_segmentId = _rd.lineKey;;
 //int count = 0;
 //bool bFound = false;
// ui->txtRouteNbr->setText(rd.alphaRoute());
// ui->cbRouteName->clear();
// ui->cbRouteName->addItem(strNoRoute);
 ui->rnw->configure(&rd, ui->lblHelpText);
 _routeNamesList = sql->getRouteNames(_routeNbr);
 for(int i=0; i < _routeNamesList.count(); i++)
 {
  QString name = (QString)_routeNamesList.at(i);
//  ui->cbRouteName->addItem(name);
//  if(name == _rd.routeName())
//  {
//   ui->cbRouteName->setCurrentIndex(i+1);
//  }
 }
 ui->dateStart->clearMaximumDateTime();
 ui->dateStart->clearMinimumDateTime();
 ui->dateEnd->clearMaximumDateTime();
 ui->dateEnd->clearMinimumDateTime();
 ui->dateStart->setDate(_rd.startDate());
 ui->dateEnd->setDate(_rd.endDate());
 displayDates(__FUNCTION__);

 //if (ui->cbCompany->currentIndex() == -1)
 {
  int companyKey= 0;
  if(_rd.route() >= 1)
   companyKey = _rd.companyKey();
  else
   companyKey = sql->getDefaultCompany(_routeNbr, ui->dateEnd->dateTime().toString("yyyy/MM/dd"));
  //i = cbCompany.FindString(companyKey.ToString(), -1);
  //cbCompany.SelectedIndex = i;
  for(int i=0;i<_companyList.count();i++)
  {
   CompanyData* cd =(CompanyData*)_companyList.at(i);
   if(companyKey == cd->companyKey)
   {
    ui->cbCompany->setCurrentIndex(i);
      // cbCompany.SelectedText = cd.ToString();
    break;
   }
  }
 }
 //foreach (tractionTypeInfo tti in cbTractionType.Items)
 for(int i=0; i < _tractionList.count(); i++)
 {
  TractionTypeInfo tti = (TractionTypeInfo)_tractionList.values().at(i);
  if (tti.tractionType == _rd.tractionType())
  {
   ui->cbTractionType->setCurrentIndex(i);
   break;
  }
 }
 bRouteChanging = false;
 bAddMode = false;
 //checkUpdate(__FUNCTION__);
 ui->cbCompany->setFocus();
}

void RouteDlg::setAddMode (bool value)
{
    bAddMode = value;
    Parameters parms = sql->getParameters();
//    ui->dateStart->setMinimumDate( parms.minDate);
//    ui->dateStart->setMaximumDate( parms.maxDate);
//    ui->dateEnd->setMaximumDate( parms.maxDate);
//    ui->dateEnd->setMinimumDate(parms.minDate);
    displayDates(__FUNCTION__);

}
void RouteDlg::SegmentChanged(qint32 segmentId)
{
    Q_UNUSED(segmentId)
 si = sql->getSegmentInfo(segmentId);
}

void RouteDlg::routeChanged(RouteData rd)
{

}
void RouteDlg::Form_Load()
{
//    if (config )
//    {
//        this.Location = new Point(config.routeDlgLocation.X, config.routeDlgLocation.Y);
//    }
    formNotLoaded = false;

    Parameters parms = sql->getParameters();
//    dateStart.MinDate = parms.minDate;
//    dateStart.MaxDate = parms.maxDate;
//    dateEnd.MinDate = parms.minDate;
//    dateEnd.MaxDate = parms.maxDate;

    fillCompanies();

    fillSegmentsComboBox();

    //fillTractionTypes();

    checkUpdate(__FUNCTION__);
}
#if 0
void RouteDlg::txtRouteNbr_Leave()
{
    bool bAlphaRoute = false;
    bNewRouteNbr = false;
    int companyKey = ui->cbCompany->currentData().toInt();
    if(ui->txtRouteNbr->text().contains(","))
    {
     int nxt = sql->findNextRouteInRange(ui->txtRouteNbr->text());
     if(nxt >= 0)
     {
      ui->txtRouteNbr->setText(QString::number(nxt));
     }
    }
    int newRoute = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
    if(newRoute < 0)
    {
        QMessageBox::StandardButtons rslt;
        rslt = QMessageBox::warning(this,tr("Route number not found"),
                                    tr( "The route number was not found. Enter Yes to add it"),
                                    QMessageBox::Yes | QMessageBox::No);
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
                bRouteChanging=false;
                return;
        }
    }
    if (newRoute != _routeNbr)
    {
        ui->cbSegments->setCurrentIndex(-1);
    }

    _routeNbr = newRoute;
    sd->setRoute(newRoute);
    sd->setAlphaRoute(ui->txtRouteNbr->text());
    if (!config->currCity->bAlphaRoutes && bAlphaRoute)
    {
        ui->lblHelpText->setText(tr( "Must be a number!"));
        //System.Media.SystemSounds.Exclamation.Play();
        ui->txtRouteNbr->setFocus();
        return;
    }
    QList<QString> _routeNamesList = sql->getRouteNames(_routeNbr);
    ui->cbRouteName->clear();
    ui->cbRouteName->addItem(strNoRoute);
    if (_routeNamesList.count()>0)
    {
        //foreach (string str in myArray)
        for (int i=0; i < _routeNamesList.count(); i++)
        {
            QString str = (QString)_routeNamesList.at(i);
            ui->cbRouteName->addItem(str);
        }
        ui->cbRouteName->setCurrentIndex(1);
    }
    else
    {
        _rd = RouteData();
        ui->cbRouteName->setCurrentIndex(0);
        ui->cbRouteName->setFocus();
        Parameters parms = sql->getParameters();
//        dateStart.MinDate = parms.minDate;
//        dateStart.MaxDate = parms.maxDate;
//        dateEnd.MaxDate = parms.maxDate;
//        dateEnd.MinDate = parms.minDate;
    }
    if (_rd.route() > 0 && _routeNbr == _rd.route())
    {
        //ui->cbRouteName->setCurrentIndex(0);
        for(int i=0; i < _routeNamesList.count(); i++)
        {
            if(_rd.route() > 0 && _rd.routeName() == _routeNamesList.at(i))
            {
                ui->cbRouteName->setCurrentIndex(i+1);
                break;
            }
        }
    }
    ui->cbTractionType->setCurrentIndex(0);
    bRouteChanging = false;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::txtRouteName_Leave()
{
    if(ui->cbRouteName->currentIndex() == 0)
        ui->btnAdd->setText(tr("Add"));
    if (ui->rnw->alphaRoute().length() == 0)
    {
        ui->txtRouteNbr->setFocus();
        //System.Media.SystemSounds.Beep.Play();
        return;
    }
    if (ui->rnw->alphaRoute().length() > 75)
    {
        ui->txtRouteNbr->setFocus();
        ui->lblHelpText->setText(tr("name > 75 characters!"));
        //System.Media.SystemSounds.Beep.Play();
        return;
    }
    sd->setRouteName(ui->rnw->alphaRoute());

    QList<RouteData> rdList = sql->getRouteDataForRouteName(sd->route(), ui->rnw->alphaRoute());
    if (rdList.count()>0)
    {
        //foreach (routeData rd in rdList)
        for(int i = 0; i < rdList.count(); i++)
        {
            RouteData rd = rdList.at(i);
            if (rd.route() > 0)
            {
             ui->dateStart->setDate( rd.startDate());
             ui->dateEnd->setDate(rd.endDate());
             displayDates(__FUNCTION__);

                //cbTractionType.SelectedIndex = rd.tractionType - 1;

                //int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
                //ui->cbTractionType->setCurrentIndex(ix);
                for( int j=0; j < _tractionList.count(); j++)
                {
                    TractionTypeInfo tt = (TractionTypeInfo)_tractionList.values().at(j);
                    if(tt.tractionType == sd->tractionType())
                    {
                        ui->cbTractionType->setCurrentIndex(j);
                        break;
                    }
                }
                //i = cbCompany.FindString(rd.companyKey.ToString(), -1);
                for( int j=0; j < _companyList.count(); j++)
                {
                    CompanyData* cd = (CompanyData*)_companyList.at(j);
                    if(cd->companyKey == sd->companyKey())
                    {
                        ui->cbCompany->setCurrentIndex(j);
                        break;
                    }
                }
            }
            else
            {
                Parameters parms = sql->getParameters();
                ui->dateStart->setMinimumDate(parms.minDate);
                ui->dateStart->setMaximumDate(parms.maxDate);
                ui->dateEnd->setMaximumDate(parms.maxDate);
                ui->dateEnd->setMinimumDate(parms.minDate);
                displayDates(__FUNCTION__);

            }
        }
    }
    else
    {
//        _rd = routeData();  // new route
//        _rd.route = _routeNbr;
//        _rd.name = ui->rnw->alphaRoute();

    }

    checkUpdate(__FUNCTION__);

}
#endif
void RouteDlg::fillSegmentsComboBox()
{
    ui->cbSegments->clear();
    _segmentDataList = sql->getSegmentDataList(_rd);
    if ( _segmentDataList.count() == 0 )
        return;
    //int count = 0;
    //cbSegments.SelectedText = "";
    //foreach (routeData rd in _segmentInfoList)
    int selection = -1;
    for(int i=0; i < _segmentDataList.count(); i++)
    {
        SegmentData* sd = _segmentDataList.at(i);
        if(_routeNbr == sd->route())
        {
         if(_rd.route() == sd->route() && _rd.routeName() == sd->routeName() && _rd.endDate() == sd->endDate())
             selection = i;
         ui->cbSegments->addItem(sd->toString(), QVariant::fromValue(*sd));
         if (i == 0)
         {
             for(int j=0; j < _tractionList.count(); j++)
             {
                 TractionTypeInfo tti = _tractionList.values().at(j);
                 if(tti.tractionType == sd->tractionType())
                 {
                     ui->cbTractionType->setCurrentIndex(j);
                     break;
                 }
             }
          }
        }
        //count++;
    }
    if(selection > 0)
        ui->cbSegments->setCurrentIndex(selection);
    //cbSegments.Text = "";

}

void RouteDlg::fillCompanies()
{
    ui->cbCompany->clear();
    //cbCompany.Text = " ";
    _companyList = sql->getCompanies();
    if (_companyList.count() == 0)
        return;
    //int count = 0;
    //foreach (companyData cd in _companyList)
    for(int i=0; i < _companyList.count(); i++)
    {
        CompanyData* cd = (CompanyData*)_companyList.at(i);
        ui->cbCompany->addItem(cd->toString(), cd->companyKey);
        //count++;
    }
    //cbCompany.SelectedIndex = 0;
    //cbCompany.Text = "";

}
void RouteDlg::fillTractionTypes()
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

void RouteDlg::cbSegments_SelectedIndexChanged(int row)
{
    //SegmentData sd;
    int ix = row;
    if (ix < 0)
        return;
    sd = _segmentDataList.at(ix);
    ui->lblSegmentText->setText(sd->toString2());
    //ui->cbRouteName->setCurrentIndex(ui->cbRouteName->findText(sd->routeName()));
    ui->rnw->setRouteName(sd->routeName());
    _routeNbr = sd->route();
    //txtRouteNbr.Text = rd.route.ToString();
    //ui->txtRouteNbr->setText( sd->alphaRoute());
    ui->rnw->setAlphaRoute(sd->alphaRoute());
    ui->cbOneWay->setChecked(sd->oneWay() == "Y");
    ui->gbUsage->setVisible(sd->tracks()==2 && sd->oneWay() == "Y");
    ui->dateStart->setDate(sd->startDate());
    ui->dateEnd->setDate(sd->endDate());
    //cbTractionType.SelectedIndex = rd.tractionType-1;
//    int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
//    cbTractionType.SelectedIndex = i;
    for(int i = 0; i < _tractionList.count(); i++)
    {
     TractionTypeInfo tti = _tractionList.values().at(i);
     if(tti.tractionType == sd->tractionType())
     {
      ui->cbTractionType->setCurrentIndex(i);
      break;
     }
    }
    if (sd->companyKey() == -1)
    {
     //cbCompany.Text = "";
     ui->cbCompany->setCurrentIndex(-1);
    }
    else
    {
     setCompany(sd->companyKey());
    }

    checkUpdate(__FUNCTION__);
    ui->dateStart->setFocus();
  #if 1
    if(sd->oneWay() == "Y")
    {
        if (sd->direction() != "" && sd->direction() != "  ")
        {
            Bearing b = Bearing();
            b.DirectionString( sd->direction());
            QString str = b.ReverseDirectionString();
            if (str != "")
            {
                ui->rbNormal->setText( sd->direction());
                ui->rbNormal->setChecked(true);
                ui->rbReverse->setText(str);
                ui->gbDirection->setVisible(true);
            }
            else
                ui->gbDirection->setVisible(false);
        }
        else
        {
            ui->rbNormal->setText( sd->bearing().strDirection());
            ui->rbReverse->setText( sd->bearing().strReverseDirection());
            ui->gbDirection->setVisible(true);
        }
        ui->rbNormal->setChecked(true);
    }
#endif

    //btnUpdateTurn_Click();
}

void RouteDlg::setCompany(qint32 companyKey)
{
 if (_companyList.count() > 0)
 {
  //foreach (companyData cd in _companyList)
  for(int i =0; i < _companyList.count(); i++)
  {
   CompanyData* cd = _companyList.at(i);
   if (cd->companyKey == companyKey)
   {
       ui->cbCompany->setCurrentIndex(i);
       break;
   }
  }
 }
 CalculateDates();
}

void RouteDlg::dateStart_Leave()
{
    ui->lblHelpText->setText("");
    if(_rd.route() < 0)
        return;
    if(ui->dateStart->date() < sd->startDate())
    {
        ui->lblHelpText->setText((tr("Warning: date is prior to previous date for this segment.")));
    }
    QDate dt = sql->getRoutesEarliestDateForSegment(_rd.route(),_rd.routeName(),-1, _rd.startDate().toString("yyyy/MM/dd"));
    if(dt.isNull() || !dt.isValid())
        dt = _rd.startDate();
//    if(ui->cbRouteName->currentIndex() == 0)
//    {
//        checkUpdate(__FUNCTION__);
//        return;
//    }
//    if(ui->cbRouteName->currentIndex() > 0 || ui->rnw->alphaRoute() != _rd.routeName())
//    {
//        if(ui->dateStart->date() < dt)
//        {
//            ui->lblHelpText->setText(tr("date can't be before route start date (")+ dt.toString("yyyy/MM/dd")+")");
//            //ui->dateStart->setFocus();
//            return;
//        }
//        if(ui->dateStart->date() > _rd.endDate())
//        {
//            ui->lblHelpText->setText(tr("date can't be after route end date (")+ _rd.endDate().toString("yyyy/MM/dd")+")");
//            //ui->dateStart->setFocus();
//            return;
//        }
//    }
    checkUpdate(__FUNCTION__);
}
void RouteDlg::dateEnd_Leave()
{
    ui->lblHelpText->setText("");
    if(_rd.route() < 0)
    {
     checkUpdate(__FUNCTION__);
     return;
    }
    if(ui->dateEnd->date() > sd->endDate())
    {
        ui->lblHelpText->setText((tr("Warning: date is after previous date for this segment.")));
    }

//    if(ui->cbRouteName->currentIndex() == 0)
//    {
//        checkUpdate(__FUNCTION__);
//        return;
//    }
//    if(ui->cbRouteName->currentIndex() > 0 || (ui->rnw->alphaRoute() != _rd.routeName()
//       && _rd.companyKey() == ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt()))
//    {
//        if(ui->dateEnd->date() < _rd.startDate())
//        {
//            ui->lblHelpText->setText(tr("date can't be before route start date (")+ _rd.startDate().toString("yyyy/MM/dd")+")");
//            //ui->dateEnd->setFocus();
//            return;
//        }
//        if(ui->dateEnd->date() > _rd.endDate())
//        {
//            ui->lblHelpText->setText(tr("Warning: extending route end date (")+ _rd.endDate().toString("yyyy/MM/dd")+")");
//            //ui->dateEnd->setFocus();
//            //return;
//        }
//    }
    checkUpdate(__FUNCTION__);
}
#if 0
void RouteDlg::checkUpdate(QString func)
{
    ui->lblHelpText->setText("");
    setDefaultTurnInfo();
    checkTurnInfo();
    if (bAddMode)
    {
        ui->btnAdd->setText(tr("Add"));
        ui->btnDelete->setEnabled(false);
        ui->btnAdd->setEnabled(true);
//        parameters parms = sql->getParameters();
//        ui->dateStart->setMinimumDateTime( parms.minDate);
//        //dateStart.MaxDate = parms.maxDate;
//        ui->dateEnd->setMinimumDateTime( parms.minDate);
//        //dateEnd.MaxDate = parms.maxDate;
//        if (_rd.route > 0)
//        {
//            //dateStart.MinDate = _rd.startDate;
//            //dateStart.MaxDate = _rd.endDate;
//            //dateStart.Value = parms.minDate;
//            //dateEnd.MinDate = _rd.startDate;
//            //dateEnd.MaxDate = _rd.endDate;
//            ui->dateEnd->setDateTime(_rd.endDate);
//        }
//        displayDates(QString(__FUNCTION__).append(QString("/").append(func)));

        //else
        //{
        //    dateStart.MaxDate = parms.maxDate;
        //    dateEnd.MaxDate = parms.maxDate;
        //}


        ui->cbSegments->setCurrentIndex(-1);
        ui->cbCompany->setCurrentIndex(-1);
        ui->cbTractionType->setCurrentIndex(-1);
        return;
    }

    if (ui->cbSegments->currentIndex() < 0)
    {
        bool bUpdate = false;
        int count = 0;
        //foreach (routeData rd in _segmentInfoList)
        for(int i=0; i < _segmentInfoList.count(); i++)
        {
            routeData rd = (routeData)_segmentInfoList.at(i);
            if (_routeNbr == rd.route && ui->rnw->alphaRoute() == rd.name &&
                ui->dateStart->dateTime() >= rd.startDate && ui->dateStart->dateTime()<= rd.endDate &&
                ui->dateEnd->dateTime() >= rd.startDate && ui->dateEnd->dateTime() <= rd.endDate)
            {
                ui->btnAdd->setText(tr("Update"));
                ui->btnDelete->setEnabled(true);
                ui->btnAdd->setEnabled(true);
                bUpdate = true;
                ui->cbSegments->setCurrentIndex(count);
                break;
            }
            count++;
        }
        if (!bUpdate)
        {
            // cbSegments.Focus();
            ui->btnAdd->setText(tr("Add"));
            ui->btnDelete->setEnabled(false);
            ui->btnAdd->setEnabled(true);
            if (_rd.route > 0 && _rd.alphaRoute == ui->txtRouteNbr->text())
            {
                if (!ui->chkLockDate->isChecked())
                {
                    QDateTime dt = sql->getRoutesEarliestDateForSegment(_rd.route, _rd.name, _SegmentId, _rd.startDate.toString("yyyy/MM/dd"));
                    QDateTime dt1 = sql->getRoutesLatestDateForSegment(_rd.route, _rd.name, _SegmentId, _rd.endDate.toString("yyyy/MM/dd"));
                    QDateTime dt2 = sql->getRoutesNextDateForSegment(_routeNbr, _rd.name, _SegmentId, _rd.endDate.toString("yyyy/MM/dd"));
                    if (dt == _rd.endDate)
                    {
                        parameters parms = sql->getParameters();
                        //dateStart.MinDate = parms.minDate;
                        //dateStart .MaxDate = parms.maxDate;
                        //dateEnd.MinDate = parms.minDate;
                        //dateEnd.MaxDate = parms.maxDate;

                        //dateStart.MinDate = _rd.startDate;
                        ui->dateStart->setMaximumDateTime(_rd.endDate);
                        //dateStart.Value = parms.minDate;
                        //dateEnd.MinDate = _rd.startDate;
                        ui->dateEnd->setMaximumDateTime(_rd.endDate);
                        ui->dateEnd->setDateTime(_rd.endDate);
                    }
                    else
                    {
                        //dateEnd = new DateTimePicker();
                        ui->dateEnd->setMinimumDateTime( dt.addDays(1));
                        ui->dateEnd->setMaximumDateTime(_rd.endDate);
                        ui->dateEnd->setDateTime( _rd.endDate);
                        //dateStart = new DateTimePicker();
                        ui->dateStart->setMinimumDateTime( dt.addDays(1));
                        ui->dateStart->setMaximumDateTime( dt2);
                        ui->dateStart->setDateTime( dt1.addDays(1));
                    }
                    displayDates(QString(__FUNCTION__).append("/").append(func).append("(2)"));

                }
            }
        }
        return;
    }
    int ix = ui->cbSegments->currentIndex();
    routeData rd1;
    rd1 = (routeData)_segmentInfoList.at(ix);
    ui->lblHelpText->setText( "");
    if (rd1.route < 1)
    {
        ui->btnAdd->setText(tr("Add"));
        ui->btnDelete->setEnabled(false);
        ui->btnAdd->setEnabled(true);
    }
    else
    {
        if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->alphaRoute(), _SegmentId, rd1.startDate.toString("yyyy/MM/dd"), rd1.endDate.toString("yyyy/MM/dd")))
        {

            {
                ui->btnAdd->setText(tr("Update"));
                ui->btnAdd->setEnabled(true);
                ui->btnDelete->setEnabled(true);
            }
        }
        else
        {
            ui->btnAdd->setEnabled( true);
            ui->btnAdd->setText(tr( "Add"));
            ui->btnDelete->setEnabled(false);
//            QDateTime dt = sql->getRoutesEarliestDateForSegment(_rd.route, _rd.name, _SegmentId, _rd.endDate.toString("yyyy/MM/dd"));
//            ui->dateEnd->setMinimumDateTime( _rd.startDate);
//            ui->dateEnd->setMaximumDateTime(_rd.endDate);
//            ui->dateEnd->setDateTime( _rd.endDate);
//            ui->dateStart->setDateTime( dt) ;
//            ui->dateStart->setMinimumDateTime( dt) ;
//            ui->dateStart->setMaximumDateTime(_rd.endDate);
//            displayDates(QString(__FUNCTION__).append("/").append(func).append("(3)"));

            setDefaultTurnInfo();
        }
    }
}
#else
void RouteDlg::checkUpdate(QString func)
{
    ui->lblHelpText->setText("");
    if(bRouteChanging || bSegmentChanging)
        return;

    if(ui->rnw->alphaRoute() == "")
    {
        ui->btnAdd->setEnabled(false);
        ui->btnDelete->setEnabled(false);
//        ui->txtRouteNbr->setFocus();
        ui->lblHelpText->setText(tr("no route number"));
        return;
    }
//    if((ui->cbRouteName->currentIndex() < 1 && (ui->rnw->alphaRoute() == ""
//                                            || ui->rnw->alphaRoute() == strNoRoute ))
//                                            || ui->cbRouteName->currentIndex() < 0)
//    {
//        ui->btnAdd->setEnabled(false);
//        ui->btnDelete->setEnabled(false);
//        ui->cbRouteName->setFocus();
//        ui->lblHelpText->setText(tr("Route name invalid"));
//        return;
//    }
//    int ix = ui->cbSegments->currentIndex();

//    if (ix < 0)
//    {
//        ui->btnAdd->setText(tr("Add"));
//        ui->btnDelete->setEnabled(false);
//        ui->btnAdd->setEnabled(true);
//    }
//    else
//    {
//        //routeData rd1 = (routeData)_segmentInfoList.at(ui->cbSegments->currentIndex());
//        if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->alphaRoute(), _SegmentId, _rd.startDate.toString("yyyy/MM/dd"), _rd.endDate.toString("yyyy/MM/dd")))
//        {
//            ui->btnAdd->setText(tr("Update"));
//            ui->btnDelete->setEnabled(true);
//        }
//    }
    if( !ui->dateStart->date().isValid() || !ui->dateEnd->date().isValid()
        || (ui->dateStart->date() > ui->dateEnd->date()))
    {
     ui->lblHelpText->setText(tr("check dates!"));
     return;
    }

    if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->alphaRoute(), _segmentId, ui->dateStart->date(),
                                   ui->dateEnd->date()))
    {
        ui->btnAdd->setText(tr("Update"));
        ui->btnDelete->setEnabled(true);
    }
    else
    {
        ui->btnAdd->setText(tr("Add"));
        ui->btnDelete->setEnabled(false);
        ui->btnAdd->setEnabled(true);
    }


    if(ui->cbTractionType->currentIndex() < 0)
    {
        ui->btnAdd->setEnabled(false);
        ui->cbTractionType->setFocus();
        return;
    }
    if(ui->cbCompany->currentIndex()< 0)
    {
        ui->btnAdd->setEnabled(false);
        ui->cbCompany->setFocus();
        ui->lblHelpText->setText(tr("Select a company"));
        return;
    }
    CompanyData* cd = _companyList.at(ui->cbCompany->currentIndex());

    if(ui->dateEnd->date() < ui->dateStart->date())
    {
        ui->btnAdd->setEnabled(false);
        //ui->dateEnd->setFocus();
        ui->lblHelpText->setText(tr("End date must be later"));
        return;
    }
    if(ui->dateStart->date() < cd->startDate)
    {
        ui->btnAdd->setEnabled(false);
        ui->lblHelpText->setText(tr("can not be before company start date (")+ cd->startDate.toString("yyyy/MM/dd")+")");
        //ui->cbCompany->setFocus();
        return;
    }

    if(ui->dateEnd->date()> cd->endDate)
    {
        ui->btnAdd->setEnabled(false);
        ui->lblHelpText->setText(tr("can not be after company end date (")+ cd->endDate.toString("yyyy/MM/dd")+")");
        //ui->cbCompany->setFocus();
        return;
    }
    if(!(ui->rbNFromBack->isChecked() || ui->rbNFromLeft->isChecked() || ui->rbNFromRight->isChecked()))
    {
        ui->btnAdd->setEnabled(false);
        ui->gbNormalEnter->setFocus();
        ui->lblHelpText->setText(tr("check radio button settings"));
        return;
    }
    if(!(ui->rbNToLeft->isChecked() || ui->rbNToRight->isChecked() || ui->rbNAhead->isChecked()))
    {
        ui->btnAdd->setEnabled(false);
        ui->gbNormalLeave->setFocus();
        ui->lblHelpText->setText(tr("check radio button settings"));
        return;
    }
    if(sd->oneWay() == "N")
    {
        if(!(ui->rbRFromBack->isChecked() || ui->rbRFromLeft->isChecked() || ui->rbRFromRight->isChecked()))
        {
            ui->btnAdd->setEnabled(false);
            ui->gbNormalLeave->setFocus();
            ui->lblHelpText->setText(tr("check radio button settings"));
            return;
        }
        if(!(ui->rbRToLeft->isChecked() || ui->rbRToRight->isChecked() || ui->rbRAhead->isChecked()))
        {
            ui->btnAdd->setEnabled(false);
            ui->gbReverseEnter->setFocus();
            ui->lblHelpText->setText(tr("check radio button settings"));
            return;
        }
    }
    setDefaultTurnInfo();
    checkTurnInfo();

    ui->btnAdd->setEnabled(true);
}
#endif

void RouteDlg::setDefaultTurnInfo()
{
 ui->rbNFromBack->setEnabled(true);
 ui->rbNFromLeft->setEnabled(true);
 ui->rbNFromRight->setEnabled(true);
 ui->rbRFromBack->setEnabled(true);
 ui->rbRFromLeft->setEnabled(true);
 ui->rbRFromRight->setEnabled(true);
 ui->rbNToLeft->setEnabled(true);
 ui->rbNToRight->setEnabled(true);
 ui->rbNAhead->setEnabled(true);
 ui->rbRToLeft->setEnabled(true);
 ui->rbRToRight->setEnabled(true);
 ui->rbRAhead->setEnabled(true);

 double dToBegin = 0, dToEnd =0, a1=0,a2=0,diff=0;
 if (sd->segmentId() < 1)
  return;
 // get the segments that intersect with the start of this segment (Normal Enter and Reverse Leave)
 //sql->OpenConnection();
 QList<SegmentData*> intersects = sql->getIntersectingRouteSegmentsAtPoint(sd->segmentId(), sd->startLat(),
                                                                           sd->startLon(), .020, _routeNbr,
                                                                           ui->rnw->alphaRoute(),
                                                                           ui->dateEnd->text());
 //foreach (segmentInfo si1 in intersects)
// ui->rbNFromBack->setEnabled(false);
// ui->rbNFromLeft->setEnabled(false);
// ui->rbNFromRight->setEnabled(false);
// ui->rbRToLeft->setEnabled(false);
// ui->rbRToRight->setEnabled(false);
// ui->rbRAhead->setEnabled(false);
 for(int i = 0; i < intersects.count(); i++)
 {
  SegmentData* sd1 = intersects.at(i);
  if (sd->segmentId() == sd1->segmentId())
   continue;
//  if (si.oneWay == "Y")
//  {
//   if (si1.oneWay == "Y" && si1.whichEnd == "S")
//    continue;
//  }
//  if (si1.oneWay == "Y" && si1.whichEnd == "S")
//      continue;
  dToBegin = sql->Distance(sd->startLat(), sd->startLon(), sd1->startLat(), sd1->startLon());
  dToEnd = sql->Distance(sd->startLat(), sd->startLon(), sd1->endLat(), sd1->endLon());
  if (dToBegin > .020 && dToEnd > .020)
      continue;   // only match to a begin points

  a1 = sd->bearingStart().angle();
  // meeting start to end
  if (sd1->whichEnd() == "S")
      a2 = sd1->bearingStart().angle()+180;
  else
      a2 = sd1->bearingEnd().angle();

  diff = sql->angleDiff(a1, a2);
  if((/*si.oneWay == "N" &&*/ sd1->oneWay() == "N") ||(sd1->oneWay()== "Y" && sd1->whichEnd()=="E"))
  {
   if (diff > 45)
   {
       //rbNFromLeft.Checked = true;
//    ui->rbNFromLeft->setEnabled(true);
    sd->setNormalEnter(1);
   }
   if (diff < -45)
   {
       //rbNFromRight.Checked = true;
//    ui->rbNFromRight->setEnabled(true);
    sd->setNormalEnter(2);
   }
   if(qAbs(diff) <= 45)
   {
       //rbNFromBack.Checked = true;
//    ui->rbNFromBack->setEnabled(true);
    sd->setNormalEnter(0);
   }
  }
  if (sd->oneWay() == "N")
  {
   if (sd1->oneWay() == "Y" && sd1->whichEnd() == "E")
    continue;
   if (diff < -45)
   {
    //rbRToLeft.Checked = true;
//    ui->rbRToLeft->setEnabled(true);
    sd->setReverseLeave(1);
   }
   if (diff > 45)
   {
       //rbRToRight.Checked = true;
//    ui->rbRToRight->setEnabled(true);
     sd->setReverseLeave(2);
   }
   if(qAbs(diff) <= 45)
   {
       //rbRAhead.Checked = true;
    ui->rbRAhead->setEnabled(true);
    sd->setReverseLeave(0);
   }
  }
 }

 // get the segments that intersect with the end of this segment (Normal Leave and Reverse enter)
 intersects = sql->getIntersectingRouteSegmentsAtPoint(sd->segmentId(), sd->endLat(), sd->endLon(),
                                                       .020, _routeNbr, ui->rnw->alphaRoute(),
                                                       ui->dateEnd->text());
// ui->rbNToLeft->setEnabled(false);
// ui->rbNToRight->setEnabled(false);
// ui->rbNAhead->setEnabled(false);
// ui->rbRFromBack->setEnabled(false);
// ui->rbRFromLeft->setEnabled(false);
// ui->rbRFromRight->setEnabled(false);

 for(int i = 0; i < intersects.count(); i++)
 {
  SegmentData* sd1 = intersects.at(i);
  if (sd->segmentId() == sd1->segmentId())
      continue;
//  if (si.oneWay == "Y")
//  {
//   if (si1.oneWay == "Y" && si1.whichEnd == "E")
//    continue;
//  }
//  if(si1.oneWay == "Y" && si1.whichEnd == "S")
//      continue;
  dToBegin = sql->Distance(sd->endLat(), sd->endLon(), sd1->startLat(), sd1->startLon());
  dToEnd = sql->Distance(sd->endLat(), sd->endLon(), sd1->endLat(), sd1->endLon());
  if (dToBegin > .020 && dToEnd > .020)
      continue;   // only match to a begin points
  a1 = sd->bearingEnd().angle();

  // meeting end to start
  if (sd1->whichEnd() == "S")
      a2 = sd1->bearingStart().angle();
  else
      a2 = sd1->bearingEnd().angle()+180;

  diff = sql->angleDiff(a1, a2);
  if((/*si.oneWay == "N" &&*/ sd1->oneWay() == "N") ||(sd1->oneWay()== "Y" && sd1->whichEnd()=="S"))
  {
   if (diff < -45)
   {
       //rbNToLeft.Checked = true;
    ui->rbNToLeft->setEnabled(true);
    sd->setNormalLeave(1);
   }
   if (diff > 45)
   {
       //rbNToRight.Checked = true;
    ui->rbNToRight->setEnabled(true);
    sd->setNormalLeave(2);
   }
   if(qAbs(diff < 45))
   {
       //rbNAhead.Checked = true;
    ui->rbNAhead->setEnabled(true);
    sd->setNormalLeave(0);
   }
  }
  if (sd->oneWay() == "N")
  {
   if (sd1->oneWay() == "Y" && sd1->whichEnd() == "S")
           continue;
   if (diff > 45)
   {
    //rbRFromLeft.Checked = true;
    ui->rbRFromLeft->setEnabled(true);
    sd->setReverseEnter(1);
   }
   if (diff < -45)
   {
       //rbRFromRight.Checked = true;
    ui->rbRFromRight->setEnabled(true);
    sd->setReverseEnter(2);
   }
   if(qAbs(diff) < 45)
   {
       //rbRFromBack.Checked = true;
    ui->rbRFromBack->setEnabled(true);
    sd->setReverseEnter(0);
   }
  }
 }
}

void RouteDlg::checkTurnInfo()
{
 QPalette* pRed = new QPalette();
 pRed->setColor(QPalette::WindowText, Qt::red);
 QPalette* pBlack = new QPalette();
 pBlack->setColor(QPalette::WindowText, Qt::black);
 QString redSS = "color: red";
 QString blackSS = "color: black";

 if(sd->segmentId()<1)
     return;
 ui->rbNFromBack->setStyleSheet(blackSS);
 ui->rbNFromLeft->setStyleSheet(blackSS);
 ui->rbNFromRight->setStyleSheet(blackSS);
// switch (sd->normalEnter())
// {
//  case 0:
//   if (!ui->rbNFromBack->isChecked())
//       ui->rbNFromBack->setStyleSheet(redSS);
//   break;
//  case 1:
//   if (!ui->rbNFromLeft->isChecked())
//       ui->rbNFromLeft->setStyleSheet(redSS);
//   break;
//  case 2:
//   if (!ui->rbNFromRight->isChecked())
//       ui->rbNFromRight->setStyleSheet(redSS);
//   break;
// }
 ui->rbNAhead->setStyleSheet(blackSS);
 ui->rbNToLeft->setStyleSheet(blackSS);
 ui->rbNToRight->setStyleSheet(blackSS);
// switch (sd->normalLeave())
// {
//  case 0:
//   if (!ui->rbNAhead->isChecked())
//    ui->rbNAhead->setStyleSheet(redSS);
//   break;
//  case 1:
//   if (!ui->rbNToLeft->isChecked())
//    ui->rbNToLeft->setStyleSheet(redSS);
//   break;
//  case 2:
//   if (!ui->rbNToRight->isChecked())
//    ui->rbNToRight->setStyleSheet(redSS);
//   break;
// }
 if (sd->oneWay() != "Y")
 {
  ui-> rbRFromBack->setStyleSheet(blackSS);
  ui-> rbRFromLeft->setStyleSheet(blackSS);
  ui->rbRFromRight->setStyleSheet(blackSS);
//  switch (sd->reverseEnter())
//  {
//   case 0:
//    if (!ui->rbRFromBack->isChecked())
//        ui->rbRFromBack->setStyleSheet(redSS);
//    break;
//   case 1:
//    if (!ui->rbRFromLeft->isChecked())
//      ui->rbRFromLeft->setStyleSheet(redSS);
//    break;
//   case 2:
//    if (!ui->rbRFromRight->isChecked())
//     ui->rbRFromRight->setStyleSheet(redSS);
//    break;
//  }
  ui->rbRAhead->setStyleSheet(blackSS);
  ui->rbRToLeft->setStyleSheet(blackSS);
  ui->rbRToRight->setStyleSheet(blackSS);
//  switch (sd->reverseLeave())
//  {
//   case 0:
//       if (!ui->rbRAhead->isChecked())
//           ui->rbRAhead->setStyleSheet(redSS);
//       break;
//   case 1:
//       if (!ui->rbRToLeft->isChecked())
//           ui->rbRToLeft->setStyleSheet(redSS);
//       break;
//   case 2:
//       if (!ui->rbRToRight->isChecked())
//           ui->rbRToRight->setStyleSheet(redSS);
//       break;
//  }
 }
}

void RouteDlg::gbNormalEnter_Leave()      // SLOT   Normal Enter
{
 checkUpdate(__FUNCTION__);
}

void RouteDlg::gbNormalLeave_Leave()      // SLOT   Normal Leave
{
 checkUpdate(__FUNCTION__);
}

void RouteDlg::gbReverseEnter_Leave()      //SLOT    Reverse Enter
{
 checkUpdate(__FUNCTION__);
}

void RouteDlg::gbReverseLeave_Leave()      // SLOT   Reverse Leave
{
    checkUpdate(__FUNCTION__);
}

void RouteDlg::btnClose_click()      //SLOT
{
 this->setVisible(false);
}

/// <summary>
/// Delete a segment from the route.
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void RouteDlg::btnDelete_Click()              // SLOT
{
 if(ui->rnw->alphaRoute() == strNoRoute)
 {
  //ui->cbRouteName->setFocus();
  return;
 }

 // get the company key selected
 int companyKey = -1;

 if (ui->cbCompany->currentIndex() >= 0)
 {
  CompanyData* cd = _companyList.at(ui->cbCompany->currentIndex());
  companyKey = cd->companyKey;
 }
 QString direction = "";
 si = sql->getSegmentInfo(_segmentId);
 if (si.segmentId() <=0)
 {
//  if (si->oneWay() == "Y")
//  {
//      if (ui->rbNormal->isChecked())
//          direction = ui->rbNormal->text();
//      else
//          direction = ui->rbReverse->text();
//  }
//  else
      direction = si.bearing().strDirection() + "-" + si.bearing().strReverseDirection();
 }
//    sql->OpenConnection();
 sql->beginTransaction("deleteSegment");

 if (!sql->deleteRouteSegment(_routeNbr, ui->rnw->getRouteId(), _segmentId, ui->dateStart->text(), ui->dateEnd->text()) == true)
 {
  ui->lblHelpText->setText(tr("deleteRoute failed!"));
  //System.Media.SystemSounds.Beep.Play();
  //sql->myConnection.Close();
  //return;
 }
 QString trackUsage = " ";
 if(ui->cbOneWay->isChecked())
 {
  if(ui->rbLeft->isChecked()) trackUsage = "L";
  if(ui->rbRight->isChecked()) trackUsage = "R";
 }
 // update routes before and or after to exclude the segment for the specified dates
 if (_rd.route() >0 && ui->dateStart->date() < _rd.startDate())
 {
  int tractionType = _tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType;
  // if (!sql->addSegmentToRoute(_routeNbr, ui->rnw->alphaRoute(), ui->dateStart->date(),
  //                             sd->startDate().addDays(-1), sd->segmentId(), sd->companyKey(), sd->tractionType(),
  //                             sd->direction(), sd->next(), sd->prev(), sd->normalEnter(), sd->normalLeave(),
  //                             sd->reverseEnter(), sd->reverseLeave(),
  //                             sd->sequence(), sd->returnSeq(),
  //                             ui->cbOneWay->isChecked()?"Y":"N", sd->trackUsage(), sd->doubleDate()))
  SegmentData sd1 = SegmentData(*sd);
  sd1.setRoute(_routeNbr);
  sd1.setRouteName(ui->rnw->newRouteName());
  sd1.setStartDate(ui->dateStart->date());
  sd1.setEndDate( sd->startDate().addDays(-1));
  sd1.setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
  if (!sql->addSegmentToRoute(&sd1))
  {
      ui->lblHelpText->setText(tr("deleteRoute failed!"));
      //System.Media.SystemSounds.Beep.Play();
      //sql->myConnection.Close();
      return;
  }
 }
 if (_rd.route() > 0 && ui->dateEnd->date() > _rd.endDate())
 {
     int tractionType = _tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType;

//     if (!sql->addSegmentToRoute(_routeNbr, ui->rnw->alphaRoute(), _rd.endDate.addDays(1),
//                                 ui->dateEnd->date(), _segmentId, companyKey, /*cbTractionType.SelectedIndex + 1*/tractionType,
//                                 direction, _rd.next, _rd.prev, _normalEnter, _normalLeave, _reverseEnter, _reverseLeave, ui->cbOneWay->isChecked()?"Y":"N", trackUsage))
     sd->setRouteName(ui->rnw->alphaRoute());
     sd->setStartDate(_rd.endDate().addDays(1));
     sd->setEndDate(ui->dateEnd->date());
     sd->setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
     sd->setTractionType(tractionType);
     sd->setTrackUsage(trackUsage);
     if (!sql->addSegmentToRoute(sd))
     {
         ui->lblHelpText->setText(tr("deleteRoute failed!"));
         //System.Media.SystemSounds.Beep.Play();
         //sql->myConnection.Close();
         return;
     }
 }
 sql->commitTransaction("deleteSegment");
 //sql->myConnection.Close();
 fillSegmentsComboBox();

 ui->btnDelete->setEnabled(false);
 ui->btnAdd->setEnabled(false);
//    if(routeChanged )
//        routeChanged(this, new routeChangedEventArgs(_routeNbr, cbRouteName.Text, _SegmentId, 0, 0, new DateTime(), routeChangedType.Delete));
 //qint32 r, QString n, qint32 s, qint32 tt, qint32 ck, QDateTime de, QString type)
 QDate dt;
 RouteChangedEventArgs  args = RouteChangedEventArgs(_routeNbr, ui->rnw->alphaRoute(),_segmentId, (qint32)0, (qint32)0, dt, "Delete");
 emit routeChangedEvent(args);
}

void RouteDlg::btnUpdateTurn_Click()      // SLOT
{
    //setDefaultTurnInfo();
    QPalette* pBlack = new QPalette();
    pBlack->setColor(QPalette::WindowText, Qt::black);
    QString redSS = "color: red";
    QString blackSS = "color: black";

    switch (sd->normalEnter())
    {
        case 0:
            ui->rbNFromBack->setChecked(true);
            ui->rbNFromBack->setStyleSheet(blackSS);
            break;
        case 1:
            ui->rbNFromLeft->setChecked(true);
            ui->rbNFromLeft->setStyleSheet(blackSS);
            break;
        case 2:
            ui->rbNFromRight->setChecked(true);
            ui->rbNFromRight->setStyleSheet(blackSS);
            break;
    }

    switch (sd->normalLeave())
    {
        case 0:
            ui->rbNAhead->setChecked(true);
            ui->rbNAhead->setStyleSheet(blackSS);
            break;
        case 1:
            ui->rbNToLeft->setChecked(true);
            ui->rbNToLeft->setStyleSheet(blackSS);
            break;
        case 2:
            ui->rbNToRight->setChecked(true);
            ui->rbNToRight->setStyleSheet(blackSS);
            break;
    }
    if (sd->oneWay() != "Y")
    {
        ui->rbRFromBack->setStyleSheet(blackSS);
        switch (sd->reverseEnter())
        {
            case 0:
                ui->rbRFromBack->setChecked(true);
                ui->rbRFromBack->setStyleSheet(blackSS);
                break;
            case 1:
                ui->rbRFromLeft->setChecked(true);
                ui->rbRFromLeft->setStyleSheet(blackSS);
                break;
            case 2:
                ui->rbRFromRight->setChecked(true);
                ui->rbRFromRight->setStyleSheet(blackSS);
                break;
        }
        ui->rbRAhead->setStyleSheet(blackSS);
        switch (sd->reverseLeave())
        {
            case 0:
                ui->rbRAhead->setChecked(true);
                ui->rbRAhead->setStyleSheet(blackSS);
                break;
            case 1:
                ui->rbRToLeft->setChecked(true);
                ui->rbRToLeft->setStyleSheet(blackSS);
                break;
            case 2:
                ui->rbRToRight->setChecked(true);
                ui->rbRToRight->setStyleSheet(blackSS);
                break;
        }
    }
//    if (ui->btnAdd->isEnabled())
//        btnAdd_Click();
    this->setVisible(false);

}

void RouteDlg::btnAdd_Click()         // SLOT
{
 //Sql mySql = new Sql();
 if(ui->rnw->alphaRoute() == strNoRoute)
 {
  //ui->cbRouteName->setFocus();
  return;
 }
 _routeNbr = ui->rnw->newRoute();
 ui->lblHelpText->setText("");

 try
 {
     QDate nextStartDate = sql->getNextStartOrEndDate(_routeNbr, ui->dateStart->date(), 0, true);
     if(nextStartDate.isValid() && ui->dateEnd->date() > nextStartDate)
     {
        ui->lblHelpText->setText( tr("end date must be before %1").arg(nextStartDate.toString("yyyy/MM/dd")));
        return;
     }
  if (ui->dateStart->dateTime() > ui->dateEnd->dateTime())
  {
   ui->lblHelpText->setText(tr( "Start date must be < end date!"));
   ui->dateStart->setFocus();
   throw  Exception("add/update failed");
  }
  QDate ds = ui->dateStart->date();
  QDate de = ui->dateEnd->date();
  if(ds > de)
  {
   ui->lblHelpText->setText(tr("Invalid start date"));
   return;
  }
  Q_ASSERT(ui->dateStart->date() >= minDate);
  if(maxDate.isValid())
  Q_ASSERT(ui->dateEnd->date() <= maxDate);
  // get the company key selected
  int companyKey = -1;

  if (ui->cbCompany->currentIndex() >= 0)
  {
   companyKey = _companyList.at(ui->cbCompany->currentIndex())->companyKey;
  }
  else
  {
   if (ui->cbCompany->currentText().length() > 0)
   {
    companyKey = sql->addCompany(ui->cbCompany->currentText(),_routeNbr, ui->dateStart->text(), ui->dateEnd->text());
   }
   else
   {
    ui->lblHelpText->setText(tr( "Select a company!"));
    //System.Media.SystemSounds.Beep.Play();
    ui->cbCompany->setFocus();
    return;
   }
  }
  checkDirection(ui->rbNormal->isChecked() ? ui->rbNormal->text() : ui->rbReverse->text());

  // Check for conflicting segments
  QList<SegmentData*> myArray = sql->getConflictingRouteSegments(_routeNbr, ui->rnw->alphaRoute(),
                                ui->dateStart->date(), ui->dateEnd->date(),
                                ui->cbCompany->currentData().toInt(), _segmentId);
  if (myArray.count() > 0)
  {
   //foreach (routeData rd in myArray)
   for(int i = 0; i > myArray.count(); i++)
   {
    SegmentData* csd = myArray.at(i);
    if (csd->startDate() == oldSd->startDate() && csd->endDate() == oldSd->endDate())
        continue;
    if (csd->endDate() >= ui->dateStart->date() && csd->endDate() < ui->dateEnd->date())
    {
 //                DialogResult rslt = MessageBox.Show("This item conflicts with:\n" +
 //                    rd.name + " " + sql->formatDate(rd.startDate) + " to " + sql->formatDate(rd.endDate) +
 //                    " Enter Yes to change it's end date to " +
 //                    sql->formatDate(dateStart.Value - TimeSpan.FromDays(1)),
 //                    "Date conflict", MessageBoxButtons.YesNoCancel);
        QMessageBox::StandardButtons rslt;
        rslt = QMessageBox::warning(this,tr("Date conflict"), tr("This item conflicts with:\n")
                                    + csd->routeName() + " " + csd->startDate().toString("yyyy/MM/dd")
                                    + " to " + csd->endDate().toString("yyyy/MM/dd") +                                                                                 " Enter Yes to change it's end date to " + ui->dateStart->date().addDays(1).toString("yyyy/MM/dd"), QMessageBox::Yes | QMessageBox::No|QMessageBox::Cancel);
        switch (rslt)
        {
            case QMessageBox::Yes:
                if(!sql->deleteRouteSegment(csd->route(), csd->routeId(), csd->segmentId()
                                            ,oldSd->startDate().toString("yyyy/MM/dd"),
                                            oldSd->endDate().toString("yyyy/MM/dd")))
                {
                 ui->lblHelpText->setText(tr("delete failed"));
                 throw  Exception("add/update failed");
                }
                if(!sql->addSegmentToRoute(csd))
                {
                 ui->lblHelpText->setText(tr("add failed"));
                 throw  Exception("add/update failed");
                }
                break;
            case QMessageBox::No:
                break;
            default:
                throw  Exception("add/update failed");

        }
    }

    if (csd->startDate() <= ui->dateEnd->date())
    {
     QMessageBox::StandardButtons rslt;
     rslt = QMessageBox::warning(this, tr("Date conflict"), tr("This item conflicts with:\n" )
                                 + csd->routeName() + " " + csd->startDate().toString("yyyy/MM/dd") + " to "
                                 + csd->endDate().toString("yyyy/MM/dd")
                                 + tr(" Enter Yes to change it's start date to ") +                                            ui->dateEnd->dateTime().addDays(1).toString("yyyy/MM/dd"), QMessageBox::Yes | QMessageBox::No|QMessageBox::Cancel);
     QString trackUsage = " ";
     if(ui->cbOneWay->isChecked() && csd->tracks() ==2)
     {
      if(ui->rbLeft)
       trackUsage = "L";
      if(ui->rbRight)
       trackUsage = "R";
     }
     switch (rslt)
     {
      case QMessageBox::Yes:
      if(!sql->deleteRouteSegment(csd->route(), csd->routeId(), csd->segmentId(), csd->startDate().toString("yyyy/MM/dd"),
                              csd->endDate().toString("yyyy/MM/dd")))
      {
       ui->lblHelpText->setText(tr("delete failed"));
       throw  Exception("add/update failed");
      }
 //     sql->addSegmentToRoute(rd.route, rd.name, rd.startDate,
 //                            ui->dateEnd->date().addDays(1), rd.lineKey,
 //                            rd.companyKey, rd.tractionType, rd.direction, rd.next, rd.prev,
 //                            rd.normalEnter, rd.normalLeave, rd.reverseEnter, rd.reverseLeave,
 //                            ui->cbOneWay->isChecked()?"Y":"N",trackUsage);
          csd->setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
          csd->setTrackUsage(trackUsage);
          sql->addSegmentToRoute(csd);
          _rd = RouteData(*csd);
          break;
      case QMessageBox::No:
          break;
      default:
      throw  Exception("add/update failed");
     }
    }
   }
  }

  QString direction="";
  //sd = sql->getSegmentInfo(_segmentId);
  if (sd->segmentId() < 1)
  {
   if (sd->oneWay() == "Y")
   {
    if (ui->rbNormal->isChecked())
        direction = ui->rbNormal->text();
    else
        direction = ui->rbReverse->text();
   }
   else
    direction = sd->bearing().strDirection() + "-" + sd->bearing().strReverseDirection();
  }
  if (ui->btnAdd->text() == "Add")
  {
   CompanyData* cd = sql->getCompany(companyKey);
   if (ui->rnw->routeNbrMustBeAdded())
   {
       _routeNbr = ui->rnw->newRoute();
       if(!sql->addAltRoute(_routeNbr, ui->rnw->alphaRoute(), cd->routePrefix))
           throw Exception("sql err");
   }
   else
   {
    _routeNbr = ui->rnw->newRoute();
    _alphaRoute = ui->rnw->alphaRoute();
   }
   sql->beginTransaction("addRouteSegment");
   if(!sql->doesAltRouteExist(sd->route(), sd->alphaRoute()))
   {
    if(!sql->addAltRoute(sd->route(), sd->alphaRoute(),cd->routePrefix))
    {
     sql->rollbackTransaction("addRouteSegment");
     return;
    }
   }
   QString trackUsage = " ";
   //SegmentData sd = sql->getSegmentInfo(_segmentId);
   if(ui->cbOneWay->isChecked() && sd->tracks() ==2)
   {
    if(ui->rbLeft->isChecked()) trackUsage = "L";
    if(ui->rbRight->isChecked()) trackUsage = "R";
   }
   if (!sql->doesRouteSegmentExist(sd->route(), ui->rnw->newRouteName(), _segmentId,
                                   ui->dateStart->date(), ui->dateEnd->date()))
   {
       sd->setRoute(ui->rnw->newRoute());
       sd->setRouteName(ui->rnw->newRouteName());
       sd->setStartDate(ui->dateStart->date());
       sd->setEndDate(ui->dateEnd->date());
       sd->setOneWay( ui->cbOneWay->isChecked()?"Y":"N");
       sd->setTractionType(_tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType);
       sd->setTrackUsage(trackUsage);
       sd->setCompanyKey(cd->companyKey);
       if (!sql->addSegmentToRoute(sd))
       {
           ui->lblHelpText->setText(tr( "Add Error"));
           //System.Media.SystemSounds.Beep.Play();
           fillSegmentsComboBox();
           sql->rollbackTransaction("addRouteSegment");
       }
       else
       {
        sql->commitTransaction("addRouteSegment");
        _rd = RouteData(*sd);
       }
   }
   else
    sql->rollbackTransaction("addRouteSegment");
   Q_ASSERT(sql->currentTransaction != "addRouteSegment");
  }
  else
  {
   // update

   {
    sd->setRouteName(ui->rnw->alphaRoute());
    sd->setCompanyKey(ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt());
    sd->setTractionType(_tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType);
    sd->setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
    QString trackUsage = " ";
    //SegmentData sd = sql->getSegmentInfo(_segmentId);
    if(ui->cbOneWay->isChecked() && sd->tracks() ==2)
    {
     if(ui->rbLeft->isChecked()) trackUsage = "L"; // running head to tail
     if(ui->rbRight->isChecked()) trackUsage = "R"; // running tail to head (normal direction)
    }
//    sd->setRoute(ui->txtRouteNbr->text().toInt());
    if(!sql->updateRoute(*oldSd, *sd))
    {
     ui->lblHelpText->setText(tr( "Update Error"));
     //System.Media.SystemSounds.Beep.Play();
     throw  Exception("add/update failed");
    }
   }
  }

  if (ui->chkOtherRoutes->isChecked())
  {
      QList<RouteIntersects> likeRoutes = sql->updateLikeRoutes(_segmentId, _routeNbr, ui->rnw->alphaRoute(), ui->dateEnd->text());
      //foreach (routeIntersects ri in likeRoutes)
      for(int i=0; i < likeRoutes.count(); i++)
      {
          RouteIntersects ri = likeRoutes.at(i);
          if (ri.sd.normalEnter() != sd->normalEnter() || ri.sd.normalLeave() != sd->normalLeave() ||
              (sd->oneWay() != "Y" && (ri.sd.reverseEnter() != sd->reverseEnter() || ri.sd.reverseLeave() != sd->reverseLeave())))
          {
              //if (MessageBox.Show("Update route " + ri.rd.alphaRoute + " " + ri.rd.name + " " + sql->formatDate(ri.rd.startDate) + " " + sql->formatDate(ri.rd.endDate) +
                  //" with the same data?", "Action required", MessageBoxButtons.YesNo) == DialogResult.Yes)
              QMessageBox::StandardButtons rslt;
              rslt = QMessageBox::warning(this, tr("Action required"), tr("Update route ")+ri.sd.alphaRoute()
                                          + " " + ri.sd.routeName() + " " + ri.sd.startDate().toString("yyyy/MM/dd")
                                          + " " + ri.sd.endDate().toString("yyyy/MM/dd") + tr(" with the same data?"),
                                          QMessageBox::Yes | QMessageBox::No);
              switch(rslt)
              {
                  case QMessageBox::Yes:
                  // sql->updateSegmentToRoute(ri.sd.route(), ri.sd.routeName(), ri.sd.startDate().toString("yyyy/MM/dd"),
                  //                           ri.sd.endDate().toString("yyyy/MM/dd"), ri.sd.segmentId(), ri.sd.companyKey(),
                  //                           ri.sd.tractionType(), sd->normalEnter(), sd->normalLeave(), sd->reverseEnter(),
                  //                           sd->reverseLeave(), ri.sd.oneWay(), ri.sd.newerName());
                      ri.sd.setNormalEnter(sd->normalEnter());
                      ri.sd.setNormalLeave(sd->normalLeave());
                      ri.sd.setReverseEnter(sd->reverseEnter());
                      ri.sd.setReverseLeave(sd->reverseLeave());
                      if(!sql->updateRoute(ri.sd, ri.sd, true))
                          return;
                      break;
                  default:
                      break;
              }
          }
      }
  }

  // notify client btnAdd.Text == "Add"
//  //if (routeChanged != null)
//  {
//      int tractionType = _tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType;
//      //routeChanged(this, new routeChangedEventArgs(_routeNbr, ui->rnw->alphaRoute(), _SegmentId, /*cbTractionType.SelectedIndex + 1*/tractionType, companyKey, dateEnd.Value, btnAdd.Text == "Add" ? routeChangedType.Add : routeChangedType.Update));
// //     RouteChangedEventArgs args =RouteChangedEventArgs(_routeNbr, ui->rnw->alphaRoute(), _SegmentId, /*cbTractionType.SelectedIndex + 1*/tractionType, companyKey, ui->dateEnd->date(), ui->btnAdd->text() == "Add" ? "Add" : "Update");
//     RouteChangedEventArgs args = RouteChangedEventArgs(_rd, ui->btnAdd->text() == "Add" ? "Add" : "Update");
//     emit routeChangedEvent(args);
//  }

  //fillCompanies();
  //fillComboBox();
  ui->btnClose->setEnabled(true);
  ui->btnAdd->setEnabled( false);
  bAddMode = false;

  myParent->On_displayRoute(_rd);
  fillSegmentsComboBox();
  this->setVisible(false);
 }
 catch(Exception e)
 {
  //sql->RollbackTransaction("add/update");
  if(!sql->currentTransaction.isEmpty())
   sql->rollbackTransaction(sql->currentTransaction);
  ui->lblHelpText->setText(tr("changes abandoned: %1").arg(e.msg));
  return;
 }
 //sql->CommitTransaction("add/Update");

 RouteChangedEventArgs args = RouteChangedEventArgs(_rd, sd->segmentId(), ui->btnAdd->text() == "Add" ? "Add" : "Update");
 emit routeChangedEvent(args);

  //myParent->refreshRoutes();
  myParent->btnDisplayRouteClicked();

} // btnAdd_Click

void RouteDlg::checkDirection(QString routeDirection)
{
    //if (sd->oneWay() != "Y")
    if(!ui->cbOneWay->isChecked())
        return;
    if (routeDirection == sd->bearing().strDirection())
        return;

    SegmentInfo si = SegmentInfo(*sd);
    QList<SegmentInfo> siReverseList = sql->getSegmentsInOppositeDirection(si);
    if(siReverseList.isEmpty())
     return;
    SegmentInfo siReverse =  siReverseList.at(0);
    int seq = 0;
    int companyKey = -1;
    companyKey = -1;
    //SegmentData sd;
    if (ui->cbCompany->currentIndex() >= 0)
    {
        //companyData cd = (companyData)cbCompany.SelectedItem;
        companyKey = _companyList.at(ui->cbCompany->currentIndex())->companyKey;
    }
    QList<SegmentData> myArray;

    if (siReverse.segmentId()< 1)
    {
//        DialogResult rslt = MessageBox.Show("You have selected a direction opposite to the defined direction for this segment.\n" +
//            "If this is a street with routes running in both directions, enter 'Yes' to create a new segment\n in the " +
//            "opposite direction.\n Otherwise select 'No' to reverse the direction for the selected segment.", "Direction conflict", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
     QMessageBox::StandardButton  rslt;
     rslt = QMessageBox::warning(this,tr("Direction conflict"), tr("You have selected a direction opposite to the defined direction for this segment.\nIf this is a street with routes running in both directions, enter 'Yes' to create a new segment\n in the opposite direction.\n Otherwise select 'No' to reverse the direction for the selected segment."), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
     switch (rslt)
     {
      case QMessageBox::Yes:
      {
          // Create a new segment in the opposite direction
          siReverse =  SegmentData(*sd);
//          sdReverse.setStartLat(sd->endLat());
//          sdReverse.startLon = si.endLon;
//          sdReverse.endLat = si.startLat;
//          sdReverse.endLon = si.startLon;
//          siReverse.setBearing(Bearing(siReverse.startLat(), siReverse.startLon(), siReverse.endLat(), siReverse.endLon()));
//          siReverse.setDirection(siReverse.bearing().strDirection());
//          sdReverse.length = si.length;
//          sdReverse.startDate = si.startDate;
//          sdReverse.endDate = si.endDate;
//          siReverse.setOneWay("Y");
//          sdReverse.routeType = si.routeType;
//          sdReverse.lineSegments = si.pointList.count();
          for(int i = siReverse.pointList().count()-1; i >= 0; i --)
           siReverse.pointList().append(sd->pointList().at(i));
          // Parse out the description
          siReverse.setDescription(SegmentDescription(sd->description()).reverseDescription() + " (1 way) "
                                    +  siReverse.bearing().strDirection());
          siReverse.setLocation(sd->location());
          siReverse.setTracks(sd->tracks());
          bool bAlreadyExists = false;
          seq =0;
          // siReverse.setSegmentId(sql->addSegment(siReverse.description(), "Y", sd->tracks(),
          //                                        siReverse.routeType(), siReverse.pointList(),
          //                                        sd->location(), & bAlreadyExists));
          siReverse.setSegmentId(sql->addSegment(siReverse, & bAlreadyExists, false));
//          if (sdReverse.segmentId != -1 && !bAlreadyExists)
//          {
//              SegmentData sd = sql->getSegmentData(si.segmentId);
//              //sql->addPoint(seq, sdReverse.segmentId, sd->endLat, sd->endLon, sd->startLat, sd->startLon, sd->streetName);
//              sql->addPoint(seq, sdReverse.segmentId,sd->getEndLatLng().lat(), sd->getEndLatLng().lon(), sd->getStartLatLng().lat(), sd->getStartLatLng().lon(), sd->getStreetName());
//          }
          int ix = ui->cbSegments->currentIndex();
          if (ix >= 0)
          {
           sd = _segmentDataList.at(ix);
           if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->alphaRoute(), _segmentId, sd->startDate(), sd->endDate()))
           {
            if (sql->deleteRouteSegment(_routeNbr, sd->routeId(), _segmentId,  sd->startDate().toString("yyyy/MM/dd"), sd->endDate().toString("yyyy/MM/dd")) != true)
            {
                ui->lblHelpText->setText(tr("Delete Error"));
                //System.Media.SystemSounds.Beep.Play();
                return;
            }
            int tractionType = _tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType;
            QString trackUsage = " ";
            if(ui->cbOneWay->isChecked())
            {
             if(ui->rbLeft->isChecked()) trackUsage = "L";
             if(ui->rbRight->isChecked()) trackUsage = "R";
            }
//            if (!sql->addSegmentToRoute(_routeNbr, ui->rnw->alphaRoute(), ui->dateStart->date(), ui->dateEnd->date(),
//                                        sdReverse.segmentId(), companyKey, /*cbTractionType.SelectedIndex+1*/tractionType,
//                                        sdReverse.direction(), sdReverse.next(), sdReverse.prev(), _normalEnter, _normalLeave,
//                                        _reverseEnter, _reverseLeave,
//                                        ui->cbOneWay->isChecked()?"Y":"N", trackUsage))
            sd->setRouteName(ui->rnw->alphaRoute());
            sd->setStartDate(ui->dateStart->date());
            sd->setEndDate(ui->dateEnd->date());
            sd->setSegmentId(siReverse.segmentId());
            sd->setDirection(siReverse.direction());
            sd->setNext(siReverse.next());
            //sd->setPrev(siReverse.prev());
            sd->setTractionType(tractionType);
            sd->setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
            sd->setTrackUsage(trackUsage);
            sd->setRouteType((RouteType)ui->cbRouteType->currentData().toInt());
            if (!sql->addSegmentToRoute(sd))
            {
                ui->lblHelpText->setText(tr( "Update Error"));
                //System.Media.SystemSounds.Beep.Play();
                return;
            }
           }
          }
          // Notify whoever is interested that a segment has changed.
          ui->lblSegmentText->setText(siReverse.description());
          _segmentId = siReverse.segmentId();
//??          _routeNbr = _routeNbr;
//                if (SegmentChanged != null)
//                    SegmentChanged(this, new segmentChangedEventArgs(si.SegmentId, sdReverse.SegmentId));
          //segmentChangedEventArgs args = segmentChangedEventArgs(si.SegmentId, sdReverse.SegmentId);
          emit SegmentChangedEvent(sd->segmentId(), siReverse.segmentId());
      }
          break;


      case QMessageBox::No:
      {
          // Reverse the segment and all it's points
          siReverse = SegmentInfo(*sd);
//          siReverse.setBearing(Bearing(siReverse.startLat(), siReverse.startLon(), siReverse.endLat(), siReverse.endLon()));
//          siReverse.setDirection(siReverse.bearing().strDirection());
          // Parse out the description
          siReverse.setDescription(SegmentDescription(sd->description()).reverseDescription() + " (1 way) " + siReverse.direction());
//          sdReverse.lineSegments = si.pointList.count();

          siReverse.pointList().clear();
          for(int i = sd->pointList().count()-1; i >= 0; i --)
           siReverse.pointList().append(sd->pointList().at(i));
      // Change the segment's description and then delete all the existing segments.
          ui->lblSegmentText->setText( sd->description());
          if(!sql->updateSegment(&siReverse))
          {
              ui->lblHelpText->setText(tr("Reverse segment failed"));
              ui->rbNormal->setChecked(true);
          }
          emit SegmentChangedEvent(sd->segmentId(), -1);
      }
          break;
      default:    // cancel
          return;
     }

 }
 else
 {
//        DialogResult rslt = MessageBox.Show("You have selected a direction opposite to the defined direction for this segment.\n A segment running in the opposite direction is already defined.\n Enter 'OK' to use " +
//        "this segment.", "Direction conflict", MessageBoxButtons.OKCancel, MessageBoxIcon.Question);
     QMessageBox::StandardButtons rslt;
     rslt = QMessageBox::question(this,tr("Direction conflict"), tr("You have selected a direction opposite to the defined direction for this segment.\n A segment running in the opposite direction is already defined.\n Enter 'OK' to use this segment."), QMessageBox::Ok|QMessageBox::No);
     if (rslt == QMessageBox::Ok)
     {
         int ix = ui->cbSegments->currentIndex();
         if (ix >= 0)
         {
             sd = _segmentDataList[ix];
             if (sql->doesRouteSegmentExist(_routeNbr, ui->rnw->alphaRoute(),
                                            _segmentId, sd->startDate(), sd->endDate()))
             {
                 if (sql->deleteRouteSegment(_routeNbr,sd->routeId(),
                                             _segmentId, sd->startDate().toString("yyyy/MM/dd"),
                                             sd->endDate().toString("yyyy/MM/dd")) == false)
                 {
                     ui->lblHelpText->setText(tr( "route not deleted"));
                     //System.Media.SystemSounds.Beep.Play();
                     ui->rbNormal->setChecked(true);
                     return;
                 }
             }
         }

         ui->rbNormal->setChecked(true);


         QString direction = sd->bearing().strDirection();
         int tractionType = _tractionList.values().at(ui->cbTractionType->currentIndex()).tractionType;
         QString trackUsage = " ";
         if(ui->cbOneWay->isChecked())
         {
          if(ui->rbLeft->isChecked()) trackUsage = "L";
          if(ui->rbRight->isChecked()) trackUsage = "R";
         }
//         if (sql->addSegmentToRoute(_routeNbr, ui->rnw->alphaRoute(), sd->startDate(),
//                                    sd->endDate(), sdReverse.segmentId(), companyKey,
//                                    /*cbTractionType.SelectedIndex+1*/tractionType, direction, 0,0,
//                                    _normalEnter, _normalLeave, _reverseEnter, _reverseLeave, ui->cbOneWay->isChecked()?"Y":"N", trackUsage))
         sd->setRouteName(ui->rnw->alphaRoute());
         sd->setSegmentId(siReverse.segmentId());
         sd->setTractionType(tractionType);
         sd->setOneWay(ui->cbOneWay->isChecked()?"Y":"N");
         sd->setTrackUsage(trackUsage);
         if (sql->addSegmentToRoute(sd))
         {
         fillSegmentsComboBox();
         checkUpdate(__FUNCTION__);
//            if(SegmentChanged != null)
//                SegmentChanged(this, new segmentChangedEventArgs(si.SegmentId,sdReverse.SegmentId));
         emit SegmentChangedEvent(sd->segmentId(),siReverse.segmentId());
         _segmentId = siReverse.segmentId();
         ui->lblSegmentText->setText(siReverse.description());

       }
     }
     else
         if (rslt == QMessageBox::Cancel)
         {
             return;
         }
 }

 checkUpdate(__FUNCTION__);

}

void RouteDlg::rbNFromLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
        sd->setNormalEnter(1);
    checkUpdate(__FUNCTION__);
}
void RouteDlg::rbNFromBack_CheckedChanged(bool bChecked)
{
    if(bChecked)
      sd->setNormalEnter(0);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNFromRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setNormalEnter(2);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNToLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setNormalLeave(1);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNAhead_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setNormalLeave(0);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNToRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setNormalLeave(2);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setReverseEnter(1);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromBack_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setReverseEnter(0);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
      sd->setReverseEnter(2);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRToLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setReverseLeave(1);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRAhead_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setReverseLeave(0);
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRToRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
     sd->setReverseLeave(2);
    checkUpdate(__FUNCTION__);
}
void RouteDlg::displayDates(QString str)
{
    //qDebug()<< str +": dateStart min-"+ui->dateStart->minimumDateTime().toString("yyyy/MM/dd")+ ",max-"+ui->dateStart->maximumDateTime().toString("yyyy/MM/dd")+",val-"+ui->dateStart->dateTime().toString("yyyy/MM/dd")+": dateEnd min-"+ui->dateEnd->minimumDateTime().toString("yyyy/MM/dd")+ ",max-"+ui->dateEnd->maximumDateTime().toString("yyyy/MM/dd")+",val-"+ui->dateEnd->dateTime().toString("yyyy/MM/dd");
}

void RouteDlg::cbCompany_SelectedIndexChanged(int i)
{
 if(i < 0) return;
 CompanyData* cd = _companyList.at(i);
 if(i < 0)
 {
     ui->lblHelpText->setText(tr("Select a company"));
     return;
 }
 ui->rnw->setCompanyKey(cd->companyKey);
 if(!sd )
  return;
 if(ui->dateStart->date() > cd->endDate || ui->dateStart->date() < cd->startDate)
  ui->dateStart->setDate(cd->startDate);
 if(ui->dateEnd->date() > cd->endDate)
  ui->dateEnd->setDate(cd->endDate);
 if(sd->startDate() >= cd->startDate)
 {
    ui->dateStart->setDate(sd->startDate());
 }
 if(sd->endDate() <= cd->endDate)
 {
  ui->dateEnd->setDate(cd->endDate);
 }

 CalculateDates();

 if(cd->startDate > _rd.startDate())
 {
     ui->lblHelpText->setText(tr("company start date (")+ cd->startDate.toString("yyyy/MM/dd") + tr(") is later than route start date"));
     QApplication::beep();
     ui->cbCompany->setFocus();
     return;
 }
 if(cd->endDate < _rd.endDate())
 {
     ui->lblHelpText->setText(tr("company end date (")+cd->endDate.toString("yyyy/MM/dd")+ tr(") is before route's' end date"));
     QApplication::beep();
     ui->cbCompany->setFocus();
     return;
 }
 if(ui->dateEnd->date() > cd->endDate)
     ui->dateEnd->setDate(cd->endDate);
 checkUpdate(__FUNCTION__);
}

void RouteDlg::OnNewCity()
{
 fillCompanies();
 fillTractionTypes();
}

void RouteDlg::CalculateDates()
{
 CompanyData* cd = _companyList.at(ui->cbCompany->currentIndex());
 minDate = cd->startDate;
 maxDate = cd->endDate;
 //si = sql->getSegmentInfo(_SegmentId);
// if(QDate::fromString(si.startDate, "yyyy/MM/dd") > minDate)
//  minDate = QDate::fromString(si.startDate, "yyyy/MM/dd");
// if(QDate::fromString(si.endDate, "yyyy/MM/dd") < maxDate)
//  maxDate = QDate::fromString(si.endDate, "yyyy/MM/dd");

 QDate start = ui->dateStart->date();
// ui->dateStart->clearMaximumDateTime();
// ui->dateStart->clearMinimumDateTime();
 if(ui->dateStart->date() < minDate)
 {
  ui->dateStart->setDate(minDate);
  //emit setStartDate(minDate);
  ui->dateStart->setTime(QTime::fromString("00:00.00", "hh:mm.ss"));
  //ui->dateStart->repaint();
 }
// ui->dateStart->setMinimumDate(minDate);
// ui->dateStart->setMinimumTime(QTime::fromString("00:00.00", "hh:mm.ss"));
// ui->dateStart->setMaximumDate(maxDate);
// ui->dateStart->setMaximumTime(QTime::fromString("00:00.00", "hh:mm.ss"));
 QDate ds = ui->dateStart->date();

 QDate end = ui->dateEnd->date();
// ui->dateEnd->clearMaximumDateTime();
// ui->dateEnd->clearMinimumDateTime();
 if(ui->dateEnd->date() > maxDate)
 {
  ui->dateEnd->setDate(maxDate);
  //emit setEndDate(maxDate);
  ui->dateEnd->setTime(QTime::fromString("00:00.00", "hh:mm.ss"));
  //ui->dateEnd->repaint();
 }
// ui->dateEnd->setMinimumDate(minDate);
// ui->dateEnd->setMinimumTime(QTime::fromString("00:00.00", "hh:mm.ss"));
// ui->dateEnd->setMaximumDate(maxDate);
// ui->dateEnd->setMinimumTime(QTime::fromString("00:00.00", "hh:mm.ss"));

 QDate de = ui->dateEnd->date();

}

void RouteDlg::cbOneWay_checkedChanged(bool oneWay)
{
 if (oneWay)
 {
     ui->gbDirection->setVisible(true);
     ui->gbReverseEnter->setVisible(false);
     ui->gbReverseLeave->setVisible(false);
     ui->rbNormal->setText(sd->bearing().strDirection());
     ui->rbReverse->setText( sd->bearing().strReverseDirection());
     ui->rbNormal->setChecked(true);
     ui->gbUsage->setVisible(sd->tracks() == 2);
 }
 else
 {
  ui->gbDirection->setVisible(false);
  ui->gbNormalEnter->setVisible(true);
  ui->gbNormalLeave->setVisible(true);
  ui->gbReverseEnter->setVisible(true);
  ui->gbReverseLeave->setVisible(true);
  ui->gbUsage->setVisible(false);
  ui->rbLeft->setChecked(false);
  ui->rbRight->setChecked(false);
  ui->gbUsage->setVisible(sd->tracks() == 2);
 }

}
