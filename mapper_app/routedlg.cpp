#include "routedlg.h"
#include "ui_routedlg.h"
#include "segmentdescription.h"
#include <QMessageBox>
#include "mainwindow.h"

RouteDlg::RouteDlg(Configuration *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RouteDlg)
{
    ui->setupUi(this);
    config = Configuration::instance();
    //sql->setConfig(config);
    sql = SQL::instance();
    this->setWindowTitle(tr("Update Route"));
    _routeNbr = -1;
    _SegmentId = -1;
    bRouteChanged = false;
    formNotLoaded = true;
    normalEnter = 0, normalLeave = 0, reverseEnter = 0, reverseLeave = 0;
    bSegmentChanging = false;
    bRouteChanging = false;
    bAddMode = false;
    _alphaRoute="";
    bNewRouteNbr=false;
    strNoRoute = tr("New Route Name");
    connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbr_Leave()) );
    connect(ui->cbRouteName, SIGNAL(signalFocusOut()), this, SLOT(txtRouteName_Leave()));
//    connect(ui->cbSegments, SIGNAL(currentIndexChanged(int)), this, SLOT(cbSegments_SelectedIndexChanged(int)));
    connect(ui->gbNormalEnter, SIGNAL(toggled(bool)), this, SLOT(gbNormalEnter_Leave()));
    connect(ui->gbNormalLeave, SIGNAL(toggled(bool)), this, SLOT(gbNormalLeave_Leave()));
    connect(ui->gbReverseEnter, SIGNAL(toggled(bool)), this, SLOT(gbReverseEnter_Leave()));
    connect(ui->gbReverseLeave, SIGNAL(toggled(bool)), this, SLOT(gbReverseLeave_Leave()));
    connect(ui->btnOK,SIGNAL(clicked()), this, SLOT(btnOK_click()));
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

//    connect(this, SIGNAL(setStartDate(QDate)), ui->dateStart, SLOT(setDate(QDate)));
//    connect(this,SIGNAL(setEndDate(QDate)), ui->dateEnd, SLOT(setDate(QDate)));
    connect(ui->dateStart, SIGNAL(editingFinished()), this, SLOT(dateStart_Leave()));
    myParent = (mainWindow*)parent;
    connect(myParent, SIGNAL(newCitySelected()), this, SLOT(OnNewCity()));
    fillCompanies();
    fillTractionTypes();

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
 _SegmentId = segmentid;
 bSegmentChanging = true;
 sql->updateSegment(_SegmentId);
 si = sql->getSegmentInfo(_SegmentId);
 if(qobject_cast<mainWindow*>(parent()))
 {
    mainWindow* main = qobject_cast<mainWindow*>(parent());
    _rd = sql->getSegmentInfoForRouteDates(main->m_routeNbr, main->m_routeName, _SegmentId, main->m_currRouteStartDate,
                                           main->m_currRouteEndDate);
 }

 if (si.segmentId < 1)
     return;
 //lblSegment.Text = sql->getSegmentDescription(_SegmentId);
 ui->lblSegmentText->setText(si.toString());
 //fillCompanies();
 fillSegmentsComboBox(); // list of routes using this segment
 //fillTractionTypes();
 if (_rd.route >0)
 {
  //foreach (tractionTypeInfo tti in cbTractionType.Items)
  for(int i = 0; i < _tractionList.count(); i++ )
  {
   tractionTypeInfo tti = (tractionTypeInfo)_tractionList.at(i);
   if (tti.tractionType == _rd.tractionType)
   {
       ui->cbTractionType->setCurrentIndex(i);
       break;
   }
  }
 }
 //ui->cbSegments->setFocus();
 ui->cbOneWay->setChecked(_rd.oneWay == "Y");
 //ui->cbOneWay->setEnabled(si.tracks==1);
 if (_rd.oneWay == "Y" && si.tracks == 1)
 {
     ui->gbDirection->setVisible(true);
     ui->gbReverseEnter->setVisible(false);
     ui->gbReverseLeave->setVisible(false);
     ui->rbNormal->setText(si.bearing.strDirection());
     ui->rbReverse->setText( si.bearing.strReverseDirection());
     ui->rbNormal->setChecked(true);
 }
 if(_rd.oneWay != "Y" && si.tracks == 1)
 {
  ui->gbDirection->setVisible(false);
  ui->gbReverseEnter->setVisible(true);
  ui->gbReverseLeave->setVisible(true);
 }
 if(_rd.oneWay == "Y" && si.tracks == 2)
 {
  ui->gbUsage->setVisible(true);
  ui->rbLeft->setChecked(_rd.trackUsage == "L");
  ui->rbRight->setChecked(_rd.trackUsage == "R");

 }
 else
 {
  ui->gbUsage->setVisible(false);
 }

 RouteData rd = sql->getRouteData(_routeNbr, segmentid, ui->dateStart->date().toString("yyyy/MM/dd"), ui->dateEnd->date().toString("yyyy/MM/dd"));
 ui->dateStart->setDate(rd.startDate);
 ui->dateEnd->setDate(rd.endDate);
 bSegmentChanging = false;
 //ui->txtRouteNbr->setFocus();


 CalculateDates();
}

void RouteDlg::setRouteNbr(qint32 rt)
{
    bRouteChanging = true;
    _routeNbr = rt;
    int companyKey= ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    _alphaRoute = sql->getAlphaRoute(_routeNbr, companyKey);
    ui->txtRouteNbr->setText(  _alphaRoute);
    _routeNamesList = sql->getRouteNames(_routeNbr);
    if (_routeNamesList.count()==0)
        return;
    ui->cbRouteName->clear();
    ui->cbRouteName->addItem("");
    //foreach (string str in myArray)
    for(int i=0;i<_routeNamesList.count();i++)
    {
        QString str = (QString)_routeNamesList.at(i);
        ui->cbRouteName->addItem(str);
    }
    if (_rd.route >0  && _rd.route == _routeNbr)
    {
        companyKey = _rd.companyKey;
        for(int j=0; j < _routeNamesList.count(); j++)
        {
            if(_routeNamesList.at(j)== _rd.name)
            {
                ui->cbRouteName->setCurrentIndex(j+1);
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

/// <summary>
/// Selects a specific route to edit.
/// </summary>
void RouteDlg::setRouteData(RouteData value)
{
 bRouteChanging = true;
 _rd = value;
 _routeNbr = _rd.route;
 _alphaRoute = _rd.alphaRoute;
 //int count = 0;
 //bool bFound = false;
 ui->txtRouteNbr->setText(_rd.alphaRoute);
 ui->cbRouteName->clear();
 ui->cbRouteName->addItem(strNoRoute);
 _routeNamesList = sql->getRouteNames(_routeNbr);
 for(int i=0; i < _routeNamesList.count(); i++)
 {
  QString name = (QString)_routeNamesList.at(i);
  ui->cbRouteName->addItem(name);
  if(name == _rd.name)
  {
   ui->cbRouteName->setCurrentIndex(i+1);
  }
 }
 if(!ui->chkLockDate->isChecked())
 {
  ui->dateStart->clearMaximumDateTime();
  ui->dateStart->clearMinimumDateTime();
  ui->dateEnd->clearMaximumDateTime();
  ui->dateEnd->clearMinimumDateTime();
  ui->dateStart->setDate(_rd.startDate);
  ui->dateEnd->setDate(_rd.endDate);
  displayDates(__FUNCTION__);
 }
 //if (ui->cbCompany->currentIndex() == -1)
 {
  int companyKey= 0;
  if(_rd.route >= 1)
   companyKey = _rd.companyKey;
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
  tractionTypeInfo tti = (tractionTypeInfo)_tractionList.at(i);
  if (tti.tractionType == _rd.tractionType)
  {
   ui->cbTractionType->setCurrentIndex(i);
   break;
  }
 }
 bRouteChanging = false;
 bAddMode = false;
 checkUpdate(__FUNCTION__);
 ui->cbCompany->setFocus();
}

void RouteDlg::setAddMode (bool value)
{
    bAddMode = value;
    parameters parms = sql->getParameters();
    ui->dateStart->setMinimumDateTime( parms.minDate);
    ui->dateStart->setMaximumDateTime( parms.maxDate);
    ui->dateEnd->setMaximumDateTime( parms.maxDate);
    ui->dateEnd->setMinimumDateTime(parms.minDate);
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

    parameters parms = sql->getParameters();
//    dateStart.MinDate = parms.minDate;
//    dateStart.MaxDate = parms.maxDate;
//    dateEnd.MinDate = parms.minDate;
//    dateEnd.MaxDate = parms.maxDate;

    fillCompanies();

    fillSegmentsComboBox();

    //fillTractionTypes();

    checkUpdate(__FUNCTION__);
}
void RouteDlg::txtRouteNbr_Leave()
{
    bool bAlphaRoute = false;
    bNewRouteNbr = false;
    //bRouteChanging = false;
    int companyKey = ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    int newRoute = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
    if(newRoute < 0)
    {
        QMessageBox::StandardButtons rslt;
        rslt = QMessageBox::warning(this,tr("Route number not found"), tr( "The route number was not found. Enter Yes to add it"), QMessageBox::Yes | QMessageBox::No);
        switch (rslt)
        {
            case QMessageBox::Yes:
                bNewRouteNbr = true;
                break;
            case QMessageBox::No:
                break;
            default:
                return;
        }
    }
    if (newRoute != _routeNbr)
    {
        ui->cbSegments->setCurrentIndex(-1);
    }
     bAddMode = false;

    _routeNbr = newRoute;
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
        parameters parms = sql->getParameters();
//        dateStart.MinDate = parms.minDate;
//        dateStart.MaxDate = parms.maxDate;
//        dateEnd.MaxDate = parms.maxDate;
//        dateEnd.MinDate = parms.minDate;
    }
    if (_rd.route > 0 && _routeNbr == _rd.route)
    {
        //ui->cbRouteName->setCurrentIndex(0);
        for(int i=0; i < _routeNamesList.count(); i++)
        {
            if(_rd.route > 0 && _rd.name == _routeNamesList.at(i))
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
    if (ui->cbRouteName->currentText().length() == 0)
    {
        ui->txtRouteNbr->setFocus();
        //System.Media.SystemSounds.Beep.Play();
        return;
    }
    if (ui->cbRouteName->currentText().length() > 75)
    {
        ui->txtRouteNbr->setFocus();
        ui->lblHelpText->setText(tr("name > 75 characters!"));
        //System.Media.SystemSounds.Beep.Play();
        return;
    }

    QList<RouteData> rdList = sql->getRouteDataForRouteName(_routeNbr, ui->cbRouteName->currentText());
    if (rdList.count()>0)
    {
        //foreach (routeData rd in rdList)
        for(int i = 0; i < rdList.count(); i++)
        {
            RouteData rd = (RouteData)rdList.at(i);
            if (rd.route > 0)
            {
                if (!ui->chkLockDate->isChecked())
                {

                    //dateStart.MaxDate = rd.endDate;
                    //dateStart.MinDate = rd.startDate;
                    //if (rd.startDate >= ui->dateStart->minimumDateTime() && rd.startDate <= ui->dateStart->maximumDateTime())
                        ui->dateStart->setDate( rd.startDate);

                    //dateEnd.MaxDate = rd.endDate;
                    //dateEnd.MinDate = rd.startDate;
                    //if (rd.endDate >= ui->dateEnd->minimumDateTime() && rd.endDate <= ui->dateEnd->maximumDateTime())
                        ui->dateEnd->setDate(rd.endDate);
                    displayDates(__FUNCTION__);

                }
                //cbTractionType.SelectedIndex = rd.tractionType - 1;

                //int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
                //ui->cbTractionType->setCurrentIndex(ix);
                for( int j=0; j < _tractionList.count(); j++)
                {
                    tractionTypeInfo tt = (tractionTypeInfo)_tractionList.at(j);
                    if(tt.tractionType == rd.tractionType)
                    {
                        ui->cbTractionType->setCurrentIndex(j);
                        break;
                    }
                }
                //i = cbCompany.FindString(rd.companyKey.ToString(), -1);
                for( int j=0; j < _companyList.count(); j++)
                {
                    CompanyData* cd = (CompanyData*)_companyList.at(j);
                    if(cd->companyKey == rd.companyKey)
                    {
                        ui->cbCompany->setCurrentIndex(j);
                        break;
                    }
                }
            }
            else
            {
                parameters parms = sql->getParameters();
                ui->dateStart->setMinimumDateTime(parms.minDate);
                ui->dateStart->setMaximumDateTime(parms.maxDate);
                ui->dateEnd->setMaximumDateTime(parms.maxDate);
                ui->dateEnd->setMinimumDateTime(parms.minDate);
                displayDates(__FUNCTION__);

            }
        }
    }
    else
    {
//        _rd = routeData();  // new route
//        _rd.route = _routeNbr;
//        _rd.name = ui->cbRouteName->currentText();

    }

    checkUpdate(__FUNCTION__);

}
void RouteDlg::fillSegmentsComboBox()
{
    ui->cbSegments->clear();
    _segmentInfoList = sql->getRouteSegmentsBySegment(_SegmentId);
    if ( _segmentInfoList.count() == 0 )
        return;
    //int count = 0;
    //cbSegments.SelectedText = "";
    //foreach (routeData rd in _segmentInfoList)
    int selection = -1;
    for(int i=0; i < _segmentInfoList.count(); i++)
    {
        RouteData rd = (RouteData)_segmentInfoList.at(i);
        if(_rd.route == rd.route && _rd.name == rd.name && _rd.endDate == rd.endDate)
            selection = i;
        ui->cbSegments->addItem(rd.toString());
        if (i == 0)
        {
            //cbSegments.SelectedText = rd.ToString();
            //txtRouteNbr.Text = rd.route.ToString();
            //txtRouteName.Text = rd.name;
            /*
            if (chkLockDate.CheckState == CheckState.Unchecked)
            {
                ui->dateStart = rd.startDate;
                dateEnd.Value = rd.endDate;
            }
             * */
            //cbTractionType.SelectedIndex = rd.tractionType-1;
            //int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
            //cbTractionType.SelectedIndex = i;
            for(int j=0; j < _tractionList.count(); j++)
            {
                tractionTypeInfo tti = _tractionList.at(j);
                if(tti.tractionType == rd.tractionType)
                {
                    ui->cbTractionType->setCurrentIndex(j);
                    break;
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
      tractionTypeInfo tti  = (tractionTypeInfo)_tractionList.at(i);
      ui->cbTractionType->addItem(tti.ToString(),tti.tractionType);
      //count++;
     }
    }
    //cbTractionType.Text = "";
}
void RouteDlg::cbSegments_SelectedIndexChanged(int row)
{
    RouteData rd;
    int ix = row;
    if (ix < 0)
        return;
    rd = (RouteData)_segmentInfoList.at(ix);
    //cbSegments.SelectedText = rd.ToString();
    //cbRouteName.Text = rd.name;
    for(int i=0; i<_routeNamesList.count(); i++)
    {
        QString str = _routeNamesList.at(i);
        if(str == rd.name)
        {
            ui->cbRouteName->setCurrentIndex(i+1);
            break;
        }
    }
    _routeNbr = rd.route;
    //txtRouteNbr.Text = rd.route.ToString();
    ui->txtRouteNbr->setText( rd.alphaRoute);
    if (!ui->chkLockDate->isChecked())
    {
        if (rd.startDate < ui->dateStart->minimumDate())
        {
            ui->dateStart->setMinimumDate(rd.startDate);
            ui->dateEnd->setMinimumDate( rd.startDate.addDays(1));
        }
        if(rd.endDate > ui->dateEnd->maximumDate())
        {
            ui->dateEnd->setMaximumDate(rd.endDate);
            ui->dateStart->setMaximumDate(rd.endDate.addDays(-1));
        }

        if (rd.startDate > ui->dateEnd->minimumDate())
        {
            ui->dateStart->setDate( rd.startDate);
        }
        ui->dateEnd->setDate( rd.endDate);
        displayDates(__FUNCTION__);

        switch (rd.normalEnter)
        {
            case 0:
                ui->rbNFromBack->setChecked(true);
                break;
            case 1:
                ui->rbNFromLeft->setChecked(true);
                break;
            case 2:
                ui->rbNFromRight->setChecked(true);
                break;
        }
        switch (rd.normalLeave)
        {
            case 0:
                ui->rbNAhead->setChecked(true);
                break;
            case 1:
                ui->rbNToLeft->setChecked(true);
                break;
            case 2:
                ui->rbNToRight->setChecked(true);
                break;
        }
        if (si.oneWay == "N")
        {

            switch (rd.reverseEnter)
            {
                case 0:
                    ui->rbRFromBack->setChecked(true);
                    break;
                case 1:
                    ui->rbRFromLeft->setChecked(true);
                    break;
                case 2:
                    ui->rbRFromRight->setChecked(true);
                    break;
            }
            switch (rd.reverseLeave)
            {
                case 0:
                    ui->rbRAhead->setChecked(true);
                    break;
                case 1:
                    ui->rbRToLeft->setChecked(true);
                    break;
                case 2:
                    ui->rbRToRight->setChecked(true);
                    break;
            }
        }
    }
    //cbTractionType.SelectedIndex = rd.tractionType-1;
//    int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
//    cbTractionType.SelectedIndex = i;
    for(int i = 0; i < _tractionList.count(); i++)
    {
     tractionTypeInfo tti = _tractionList.at(i);
     if(tti.tractionType == rd.tractionType)
     {
      ui->cbTractionType->setCurrentIndex(i);
      break;
     }
    }
    if (rd.companyKey == -1)
    {
     //cbCompany.Text = "";
     ui->cbCompany->setCurrentIndex(-1);
    }
    else
    {
        setCompany(rd.companyKey);

    }
    ui->btnAdd->setEnabled(false);
    ui->btnAdd->setEnabled(true);

    checkUpdate(__FUNCTION__);
    ui->dateStart->setFocus();
    if(si.oneWay == "Y")
    {
        if (si.direction != "" && si.direction != "  ")
        {
            Bearing b = Bearing();
            b.DirectionString( si.direction);
            QString str = b.ReverseDirectionString();
            if (str != "")
            {
                ui->rbNormal->setText( si.direction);
                ui->rbNormal->setChecked(true);
                ui->rbReverse->setText(str);
                ui->gbDirection->setVisible(true);
            }
            else
                ui->gbDirection->setVisible(false);
        }
        else
        {
            ui->rbNormal->setText( si.bearing.strDirection());
            ui->rbReverse->setText( si.bearing.strReverseDirection());
            ui->gbDirection->setVisible(true);
        }
        ui->rbNormal->setChecked(true);
    }
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
    if(_rd.route < 0)
        return;
    if(ui->dateStart->date().toString("yyyy/MM/dd") < si.startDate)
    {
        ui->lblHelpText->setText((tr("Warning: date is prior to previous date for this segment.")));
    }
    QDate dt = sql->getRoutesEarliestDateForSegment(_rd.route,_rd.name,-1, _rd.startDate.toString("yyyy/MM/dd"));
    if(dt.isNull() || !dt.isValid())
        dt = _rd.startDate;
    if(ui->cbRouteName->currentIndex() == 0)
    {
        checkUpdate(__FUNCTION__);
        return;
    }
    if(ui->cbRouteName->currentIndex() > 0 || ui->cbRouteName->currentText() != _rd.name)
    {
        if(ui->dateStart->date() < dt)
        {
            ui->lblHelpText->setText(tr("date can't be before route start date (")+ dt.toString("yyyy/MM/dd")+")");
            //ui->dateStart->setFocus();
            return;
        }
        if(ui->dateStart->date() > _rd.endDate)
        {
            ui->lblHelpText->setText(tr("date can't be after route end date (")+ _rd.endDate.toString("yyyy/MM/dd")+")");
            //ui->dateStart->setFocus();
            return;
        }
    }
    checkUpdate(__FUNCTION__);
}
void RouteDlg::dateEnd_Leave()
{
    ui->lblHelpText->setText("");
    if(_rd.route < 0)
    {
     checkUpdate(__FUNCTION__);
     return;
    }
    if(ui->dateEnd->date().toString("yyyy/MM/dd") > si.endDate)
    {
        ui->lblHelpText->setText((tr("Warning: date is after previous date for this segment.")));
    }

    if(ui->cbRouteName->currentIndex() == 0)
    {
        checkUpdate(__FUNCTION__);
        return;
    }
    if(ui->cbRouteName->currentIndex() > 0 || ui->cbRouteName->currentText() != _rd.name && _rd.companyKey == ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt())
    {
        if(ui->dateEnd->date() < _rd.startDate)
        {
            ui->lblHelpText->setText(tr("date can't be before route start date (")+ _rd.startDate.toString("yyyy/MM/dd")+")");
            //ui->dateEnd->setFocus();
            return;
        }
        if(ui->dateEnd->date() > _rd.endDate)
        {
            ui->lblHelpText->setText(tr("Warning: extending route end date (")+ _rd.endDate.toString("yyyy/MM/dd")+")");
            //ui->dateEnd->setFocus();
            //return;
        }
    }
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
            if (_routeNbr == rd.route && ui->cbRouteName->currentText() == rd.name &&
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
        if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, rd1.startDate.toString("yyyy/MM/dd"), rd1.endDate.toString("yyyy/MM/dd")))
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


    if(ui->txtRouteNbr->text() == "")
    {
        ui->btnAdd->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        ui->txtRouteNbr->setFocus();
        ui->lblHelpText->setText(tr("no route number"));
        return;
    }
    if((ui->cbRouteName->currentIndex() < 1 && (ui->cbRouteName->currentText() == "" || ui->cbRouteName->currentText() == strNoRoute )) || ui->cbRouteName->currentIndex() < 0)
    {
        ui->btnAdd->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        ui->cbRouteName->setFocus();
        ui->lblHelpText->setText(tr("Route name invalid"));
        return;
    }
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
//        if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, _rd.startDate.toString("yyyy/MM/dd"), _rd.endDate.toString("yyyy/MM/dd")))
//        {
//            ui->btnAdd->setText(tr("Update"));
//            ui->btnDelete->setEnabled(true);
//        }
//    }
    if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, ui->dateStart->text(), ui->dateEnd->text()))
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
    if(si.oneWay == "N")
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
 if (si.segmentId < 1)
  return;
 // get the segments that intersect with the start of this segment (Normal Enter and Reverse Leave)
 //sql->OpenConnection();
 QList<SegmentInfo> intersects = sql->getIntersectingRouteSegmentsAtPoint(si.startLat, si.startLon, .020, _routeNbr, ui->cbRouteName->currentText(), ui->dateEnd->text());
 //foreach (segmentInfo si1 in intersects)
 ui->rbNFromBack->setEnabled(false);
 ui->rbNFromLeft->setEnabled(false);
 ui->rbNFromRight->setEnabled(false);
 ui->rbRToLeft->setEnabled(false);
 ui->rbRToRight->setEnabled(false);
 ui->rbRAhead->setEnabled(false);
 for(int i = 0; i < intersects.count(); i++)
 {
  SegmentInfo si1 = (SegmentInfo)intersects.at(i);
  if (si.segmentId == si1.segmentId)
   continue;
//  if (si.oneWay == "Y")
//  {
//   if (si1.oneWay == "Y" && si1.whichEnd == "S")
//    continue;
//  }
//  if (si1.oneWay == "Y" && si1.whichEnd == "S")
//      continue;
  dToBegin = sql->Distance(si.startLat, si.startLon, si1.startLat, si1.startLon);
  dToEnd = sql->Distance(si.startLat, si.startLon, si1.endLat, si1.endLon);
  if (dToBegin > .020 && dToEnd > .020)
      continue;   // only match to a begin points

  a1 = si.bearingStart.getBearing();
  // meeting start to end
  if (si1.whichEnd == "S")
      a2 = si1.bearingStart.getBearing()+180;
  else
      a2 = si1.bearingEnd.getBearing();

  diff = sql->angleDiff(a1, a2);
  if((/*si.oneWay == "N" &&*/ si1.oneWay == "N") ||(si1.oneWay== "Y" && si1.whichEnd=="E"))
  {
   if (diff > 45)
   {
       //rbNFromLeft.Checked = true;
    ui->rbNFromLeft->setEnabled(true);
    _normalEnter = 1;
   }
   if (diff < -45)
   {
       //rbNFromRight.Checked = true;
    ui->rbNFromRight->setEnabled(true);
    _normalEnter = 2;
   }
   if(qAbs(diff) <= 45)
   {
       //rbNFromBack.Checked = true;
    ui->rbNFromBack->setEnabled(true);
    _normalEnter = 0;
   }
  }
  if (si.oneWay == "N")
  {
   if (si1.oneWay == "Y" && si1.whichEnd == "E")
    continue;
   if (diff < -45)
   {
    //rbRToLeft.Checked = true;
    ui->rbRToLeft->setEnabled(true);
    _reverseLeave = 1;
   }
   if (diff > 45)
   {
       //rbRToRight.Checked = true;
    ui->rbRToRight->setEnabled(true);
     _reverseLeave = 2;
   }
   if(qAbs(diff) <= 45)
   {
       //rbRAhead.Checked = true;
    ui->rbRAhead->setEnabled(true);
    _reverseLeave = 0;
   }
  }
 }

 // get the segments that intersect with the end of this segment (Normal Leave and Reverse enter)
 intersects = sql->getIntersectingRouteSegmentsAtPoint(si.endLat, si.endLon, .020, _routeNbr, ui->cbRouteName->currentText(), ui->dateEnd->text());
 ui->rbNToLeft->setEnabled(false);
 ui->rbNToRight->setEnabled(false);
 ui->rbNAhead->setEnabled(false);
 ui->rbRFromBack->setEnabled(false);
 ui->rbRFromLeft->setEnabled(false);
 ui->rbRFromRight->setEnabled(false);

 for(int i = 0; i < intersects.count(); i++)
 {
  SegmentInfo si1 = (SegmentInfo)intersects.at(i);
  if (si.segmentId == si1.segmentId)
      continue;
//  if (si.oneWay == "Y")
//  {
//   if (si1.oneWay == "Y" && si1.whichEnd == "E")
//    continue;
//  }
//  if(si1.oneWay == "Y" && si1.whichEnd == "S")
//      continue;
  dToBegin = sql->Distance(si.endLat, si.endLon, si1.startLat, si1.startLon);
  dToEnd = sql->Distance(si.endLat, si.endLon, si1.endLat, si1.endLon);
  if (dToBegin > .020 && dToEnd > .020)
      continue;   // only match to a begin points
  a1 = si.bearingEnd.getBearing();

  // meeting end to start
  if (si1.whichEnd == "S")
      a2 = si1.bearingStart.getBearing();
  else
      a2 = si1.bearingEnd.getBearing()+180;

  diff = sql->angleDiff(a1, a2);
  if((/*si.oneWay == "N" &&*/ si1.oneWay == "N") ||(si1.oneWay== "Y" && si1.whichEnd=="S"))
  {
   if (diff < -45)
   {
       //rbNToLeft.Checked = true;
    ui->rbNToLeft->setEnabled(true);
    _normalLeave = 1;
   }
   if (diff > 45)
   {
       //rbNToRight.Checked = true;
    ui->rbNToRight->setEnabled(true);
    _normalLeave = 2;
   }
   if(qAbs(diff < 45))
   {
       //rbNAhead.Checked = true;
    ui->rbNAhead->setEnabled(true);
    _normalLeave = 0;
   }
  }
  if (si.oneWay == "N")
  {
   if (si1.oneWay == "Y" && si1.whichEnd == "S")
           continue;
   if (diff > 45)
   {
    //rbRFromLeft.Checked = true;
    ui->rbRFromLeft->setEnabled(true);
    _reverseEnter = 1;
   }
   if (diff < -45)
   {
       //rbRFromRight.Checked = true;
    ui->rbRFromRight->setEnabled(true);
    _reverseEnter = 2;
   }
   if(qAbs(diff) < 45)
   {
       //rbRFromBack.Checked = true;
    ui->rbRFromBack->setEnabled(true);
    _reverseEnter = 0;
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

 if(si.segmentId<1)
     return;
 ui->rbNFromBack->setStyleSheet(blackSS);
 ui->rbNFromLeft->setStyleSheet(blackSS);
 ui->rbNFromRight->setStyleSheet(blackSS);
 switch (_normalEnter)
 {
  case 0:
   if (!ui->rbNFromBack->isChecked())
       ui->rbNFromBack->setStyleSheet(redSS);
   break;
  case 1:
   if (!ui->rbNFromLeft->isChecked())
       ui->rbNFromLeft->setStyleSheet(redSS);
   break;
  case 2:
   if (!ui->rbNFromRight->isChecked())
       ui->rbNFromRight->setStyleSheet(redSS);
   break;
 }
 ui->rbNAhead->setStyleSheet(blackSS);
 ui->rbNToLeft->setStyleSheet(blackSS);
 ui->rbNToRight->setStyleSheet(blackSS);
 switch (_normalLeave)
 {
  case 0:
   if (!ui->rbNAhead->isChecked())
    ui->rbNAhead->setStyleSheet(redSS);
   break;
  case 1:
   if (!ui->rbNToLeft->isChecked())
    ui->rbNToLeft->setStyleSheet(redSS);
   break;
  case 2:
   if (!ui->rbNToRight->isChecked())
    ui->rbNToRight->setStyleSheet(redSS);
   break;
 }
 if (si.oneWay == "N")
 {
  ui-> rbRFromBack->setStyleSheet(blackSS);
  ui-> rbRFromLeft->setStyleSheet(blackSS);
  ui->rbRFromRight->setStyleSheet(blackSS);
  switch (_reverseEnter)
  {
   case 0:
    if (!ui->rbRFromBack->isChecked())
        ui->rbRFromBack->setStyleSheet(redSS);
    break;
   case 1:
    if (!ui->rbRFromLeft->isChecked())
      ui->rbRFromLeft->setStyleSheet(redSS);
    break;
   case 2:
    if (!ui->rbRFromRight->isChecked())
     ui->rbRFromRight->setStyleSheet(redSS);
    break;
  }
  ui->rbRAhead->setStyleSheet(blackSS);
  ui->rbRToLeft->setStyleSheet(blackSS);
  ui->rbRToRight->setStyleSheet(blackSS);
  switch (_reverseLeave)
  {
   case 0:
       if (!ui->rbRAhead->isChecked())
           ui->rbRAhead->setStyleSheet(redSS);
       break;
   case 1:
       if (!ui->rbRToLeft->isChecked())
           ui->rbRToLeft->setStyleSheet(redSS);
       break;
   case 2:
       if (!ui->rbRToRight->isChecked())
           ui->rbRToRight->setStyleSheet(redSS);
       break;
  }
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

void RouteDlg::btnOK_click()      //SLOT
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
 if(ui->cbRouteName->currentText() == strNoRoute)
 {
  ui->cbRouteName->setFocus();
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
 si = sql->getSegmentInfo(_SegmentId);
 if (si.segmentId <=0)
 {
  if (si.oneWay == "Y")
  {
      if (ui->rbNormal->isChecked())
          direction = ui->rbNormal->text();
      else
          direction = ui->rbReverse->text();
  }
  else
      direction = si.bearing.strDirection() + "-" + si.bearing.strReverseDirection();
 }
//    sql->OpenConnection();
 sql->BeginTransaction("deleteSegment");

 if (!sql->deleteRouteSegment(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, ui->dateStart->text(), ui->dateEnd->text()) == true)
 {
  ui->lblHelpText->setText(tr("deleteRoute failed!"));
  //System.Media.SystemSounds.Beep.Play();
  //sql->myConnection.Close();
  //return;
 }
 QString trackUsage = " ";
 if(ui->cbOneWay)
 {
  if(ui->rbLeft->isChecked()) trackUsage = "L";
  if(ui->rbRight->isChecked()) trackUsage = "R";
 }
 // update routes before and or after to exclude the segment for the specified dates
 if (_rd.route >0 && ui->dateStart->date() < _rd.startDate)
 {
  int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
  if (!sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), ui->dateStart->text(),
                              _rd.startDate.addDays(-1).toString("yyyy/MM/dd"), _SegmentId, _rd.companyKey,
                              /*cbTractionType.SelectedIndex + 1*/tractionType, direction,
                              normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
  {
      ui->lblHelpText->setText(tr("deleteRoute failed!"));
      //System.Media.SystemSounds.Beep.Play();
      //sql->myConnection.Close();
      return;
  }
 }
 if (_rd.route > 0 && ui->dateEnd->date() > _rd.endDate)
 {
     int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;

     if (!sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), _rd.endDate.addDays(1).toString("yyyy/MM/dd"),
                                 ui->dateEnd->text(), _SegmentId, companyKey, /*cbTractionType.SelectedIndex + 1*/tractionType,
                                 direction, normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
     {
         ui->lblHelpText->setText(tr("deleteRoute failed!"));
         //System.Media.SystemSounds.Beep.Play();
         //sql->myConnection.Close();
         return;
     }
 }
 sql->CommitTransaction("deleteSegment");
 //sql->myConnection.Close();
 fillSegmentsComboBox();

 ui->btnDelete->setEnabled(false);
 ui->btnAdd->setEnabled(false);
//    if(routeChanged )
//        routeChanged(this, new routeChangedEventArgs(_routeNbr, cbRouteName.Text, _SegmentId, 0, 0, new DateTime(), routeChangedType.Delete));
 //qint32 r, QString n, qint32 s, qint32 tt, qint32 ck, QDateTime de, QString type)
 QDate dt;
 RouteChangedEventArgs  args = RouteChangedEventArgs(_routeNbr, ui->cbRouteName->currentText(),_SegmentId, (qint32)0, (qint32)0, dt, "Delete");
 emit routeChangedEvent(args);
}

void RouteDlg::btnUpdateTurn_Click()      // SLOT
{
    //setDefaultTurnInfo();
    QPalette* pBlack = new QPalette();
    pBlack->setColor(QPalette::Foreground, Qt::black);
    QString redSS = "color: red";
    QString blackSS = "color: black";

    switch (_normalEnter)
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

    switch (_normalLeave)
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
    if (si.oneWay == "N")
    {
        ui->rbRFromBack->setStyleSheet(blackSS);
        switch (_reverseEnter)
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
        switch (_reverseLeave)
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
    if (ui->btnAdd->isEnabled())
        btnAdd_Click();
}

void RouteDlg::btnAdd_Click()         // SLOT
{
 //Sql mySql = new Sql();
 if(ui->cbRouteName->currentText() == strNoRoute)
 {
  ui->cbRouteName->setFocus();
  return;
 }
 RouteData oldRd;
 int ixo = ui->cbSegments->currentIndex();
 if(ixo >= 0)
     oldRd = (RouteData)_segmentInfoList.at(ixo);
 ui->lblHelpText->setText("");
 if (ui->dateStart->dateTime() > ui->dateEnd->dateTime())
 {
  ui->lblHelpText->setText(tr( "Start date must be < end date!"));
  ui->dateStart->setFocus();
  return;
 }
 QDate ds = ui->dateStart->date();
 QDate de = ui->dateEnd->date();
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

 //// First check for conflicts
 //ArrayList myArray = sql->checkConflicts(_routeNbr, _SegmentId, ui->dateStart.text(), dateEnd.Text);
 //if (myArray != null)
 //{
 //    foreach (routeData rd in myArray)
 //    {
 //        if (oldRd.route == 0)
 //            continue;
 //        if (rd.startDate == oldRd.startDate && rd.endDate == oldRd.endDate && rd.route == oldRd.route)
 //            continue;

 //        MessageBox.Show("Adding this item would cause a conflict with:\n" +
 //            rd.name + " " + rd.startDate + " to " + rd.endDate, "Date conflict");
 //        return;
 //    }
 //}
 // Check for conflicting segments
 QList<RouteData> myArray = sql->getConflictingRouteSegments(_routeNbr, ui->cbRouteName->currentText(), ui->dateStart->text(), ui->dateEnd->text(), _SegmentId);
 if (myArray.count() > 0)
 {
  //foreach (routeData rd in myArray)
  for(int i = 0; i > myArray.count(); i++)
  {
   RouteData rd = myArray.at(i);
   if (rd.startDate == oldRd.startDate && rd.endDate == oldRd.endDate)
       continue;
   if (rd.endDate >= ui->dateStart->date() && rd.endDate < ui->dateEnd->date())
   {
//                DialogResult rslt = MessageBox.Show("This item conflicts with:\n" +
//                    rd.name + " " + sql->formatDate(rd.startDate) + " to " + sql->formatDate(rd.endDate) +
//                    " Enter Yes to change it's end date to " +
//                    sql->formatDate(dateStart.Value - TimeSpan.FromDays(1)),
//                    "Date conflict", MessageBoxButtons.YesNoCancel);
       QMessageBox::StandardButtons rslt;
       rslt = QMessageBox::warning(this,tr("Date conflict"), tr("This item conflicts with:\n") + rd.name + " " + rd.startDate.toString("yyyy/MM/dd") + " to " + rd.endDate.toString("yyyy/MM/dd") +                                                                                 " Enter Yes to change it's end date to " + ui->dateStart->date().addDays(1).toString("yyyy/MM/dd"), QMessageBox::Yes | QMessageBox::No|QMessageBox::Cancel);
       switch (rslt)
       {
           case QMessageBox::Yes:
               sql->deleteRouteSegment(rd.route, rd.name, rd.lineKey,oldRd.startDate.toString("yyyy/MM/dd"), oldRd.endDate.toString("yyyy/MM/dd"));
               sql->addSegmentToRoute(rd.route, rd.name, rd.startDate.toString("yyyy/MM/dd"),
                                       ui->dateStart->dateTime().addDays(-1).toString("yyyy/MM/dd"), rd.lineKey,
                                       rd.companyKey, rd.tractionType, rd.direction,
                                       rd.normalEnter, rd.normalLeave, rd.reverseEnter, rd.reverseLeave, ui->cbOneWay?"Y":"N", rd.trackUsage);
               break;
           case QMessageBox::No:
               break;
           default:
               return;
       }
   }
   if (rd.startDate <= ui->dateEnd->date())
   {
//                DialogResult rslt = MessageBox.Show("This item conflicts with:\n" +
//                    rd.name + " " + sql->formatDate(rd.startDate) + " to " + sql->formatDate(rd.endDate) +
//                    " Enter Yes to change it's start date to " +
//                    sql->formatDate(dateEnd.Value + TimeSpan.FromDays(1)),
//                    "Date conflict", MessageBoxButtons.YesNoCancel);
    QMessageBox::StandardButtons rslt;
    rslt = QMessageBox::warning(this, tr("Date conflict"), tr("This item conflicts with:\n" ) + rd.name + " " + rd.startDate.toString("yyyy/MM/dd") + " to " + rd.endDate.toString("yyyy/MM/dd") + tr(" Enter Yes to change it's start date to ") +                                            ui->dateEnd->dateTime().addDays(1).toString("yyyy/MM/dd"), QMessageBox::Yes | QMessageBox::No|QMessageBox::Cancel);
    QString trackUsage = " ";
    if(ui->cbOneWay )
    {
     if(ui->rbLeft)
      trackUsage = "L";
     if(ui->rbRight)
      trackUsage = "R";
    }
    switch (rslt)
    {
     case QMessageBox::Yes:
     sql->deleteRouteSegment(rd.route, rd.name, rd.lineKey, rd.startDate.toString("yyyy/MM/dd"),
                             rd.endDate.toString("yyyy/MM/dd"));
     sql->addSegmentToRoute(rd.route, rd.name, rd.startDate.toString("yyyy/MM/dd"),
                            ui->dateEnd->dateTime().addDays(1).toString("yyyy/MM/dd"), rd.lineKey,
                            rd.companyKey, rd.tractionType, rd.direction, rd.next, rd.prev,
                            rd.normalEnter, rd.normalLeave, rd.reverseEnter, rd.reverseLeave,
                            ui->cbOneWay?"Y":"N",trackUsage);
         break;
     case QMessageBox::No:
         break;
     default:
         return;
    }
   }
  }
 }

 QString direction="";
 si = sql->getSegmentInfo(_SegmentId);
 if (si.segmentId < 1)
 {
     if (si.oneWay == "Y")
     {
         if (ui->rbNormal->isChecked())
             direction = ui->rbNormal->text();
         else
             direction = ui->rbReverse->text();
     }
     else
         direction = si.bearing.strDirection() + "-" + si.bearing.strReverseDirection();
 }
 if (ui->btnAdd->text() == "Add")
 {
  CompanyData* cd = sql->getCompany(companyKey);
     //if (_routeNbr == -1 && _alphaRoute != "")
  _routeNbr = sql->addAltRoute(ui->txtRouteNbr->text(), cd->routePrefix);
  QString trackUsage = " ";
  if(ui->cbOneWay)
  {
   if(ui->rbLeft->isChecked()) trackUsage = "L";
   if(ui->rbRight->isChecked()) trackUsage = "R";
  }
  if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, ui->dateStart->text(), ui->dateEnd->text()) == false)
  {
      int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
      if (!sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), ui->dateStart->text(), ui->dateEnd->text(),
                                  _SegmentId, companyKey, /*cbTractionType.SelectedIndex+1*/tractionType, direction,
                                  normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
      {
          ui->lblHelpText->setText(tr( "Add Error"));
          //System.Media.SystemSounds.Beep.Play();
          fillSegmentsComboBox();
          return;
      }
  }
 }
 else
 {
  // update
  sql->BeginTransaction("UpdateRoute");
  int ix = ui->cbSegments->currentIndex();
  if (ix >= 0)
  {
   RouteData rd = (RouteData)_segmentInfoList[ix];
//   if (sql->deleteRouteSegment(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, _rd.startDate.toString("yyyy/MM/dd"), _rd.endDate.toString("yyyy/MM/dd")) != true)
   if (sql->deleteRouteSegment(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, ui->dateStart->text(), ui->dateEnd->text()) != true)
   {
    ui->lblHelpText->setText(tr( "Delete Error"));
    //System.Media.SystemSounds.Beep.Play();
    sql->RollbackTransaction("UpdateRoute");
    return;
   }
  }
  int companyKey= ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();

  QString alpha = sql->getAlphaRoute(_routeNbr, companyKey);
  if (alpha == "")
  {
   CompanyData* cd = sql->getCompany(companyKey);
      _routeNbr = sql->addAltRoute(_alphaRoute, cd->routePrefix);
  }
  QString trackUsage = " ";
  if(ui->cbOneWay)
  {
   if(ui->rbLeft->isChecked()) trackUsage = "L";
   if(ui->rbRight->isChecked()) trackUsage = "R";
  }
  int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
  if (!sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), ui->dateStart->text(), ui->dateEnd->text(),
                              _SegmentId, companyKey, /*cbTractionType.SelectedIndex+1*/tractionType, direction,
                              normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
  {
   ui->lblHelpText->setText(tr( "Update Error"));
   //System.Media.SystemSounds.Beep.Play();
   sql->RollbackTransaction("UpdateRoute");
   return;
  }
 }
 if (ui->chkOtherRoutes->isChecked())
 {
     QList<routeIntersects> likeRoutes = sql->updateLikeRoutes(_SegmentId, _routeNbr, ui->cbRouteName->currentText(), ui->dateEnd->text());
     //foreach (routeIntersects ri in likeRoutes)
     for(int i=0; i < likeRoutes.count(); i++)
     {
         routeIntersects ri = likeRoutes.at(i);
         if (ri.rd.normalEnter != normalEnter || ri.rd.normalLeave != normalLeave ||
             (si.oneWay != "Y" && (ri.rd.reverseEnter != reverseEnter || ri.rd.reverseLeave != reverseLeave)))
         {
             //if (MessageBox.Show("Update route " + ri.rd.alphaRoute + " " + ri.rd.name + " " + sql->formatDate(ri.rd.startDate) + " " + sql->formatDate(ri.rd.endDate) +
                 //" with the same data?", "Action required", MessageBoxButtons.YesNo) == DialogResult.Yes)
             QMessageBox::StandardButtons rslt;
             rslt = QMessageBox::warning(this, tr("Action required"), tr("Update route ")+ri.rd.alphaRoute + " " + ri.rd.name + " " + ri.rd.startDate.toString("yyyy/MM/dd") + " " + ri.rd.endDate.toString("yyyy/MM/dd") + tr(" with the same data?"),QMessageBox::Yes | QMessageBox::No);
             switch(rslt)
             {
                 case QMessageBox::Yes:
                     sql->updateSegmentToRoute(ri.rd.route, ri.rd.name, ri.rd.startDate.toString("yyyy/MM/dd"), ri.rd.endDate.toString("yyyy/MM/dd"), ri.rd.lineKey, ri.rd.companyKey, ri.rd.tractionType, normalEnter, normalLeave, reverseEnter, reverseLeave, ri.rd.oneWay);
                     break;
                 default:
                     break;
             }
         }
     }
 }
 sql->CommitTransaction("UpdateRoute");

 // notify client btnAdd.Text == "Add"
 //if (routeChanged != null)
 {
     int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
     //routeChanged(this, new routeChangedEventArgs(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, /*cbTractionType.SelectedIndex + 1*/tractionType, companyKey, dateEnd.Value, btnAdd.Text == "Add" ? routeChangedType.Add : routeChangedType.Update));
     RouteChangedEventArgs args =RouteChangedEventArgs(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, /*cbTractionType.SelectedIndex + 1*/tractionType, companyKey, ui->dateEnd->date(), ui->btnAdd->text() == "Add" ? "Add" : "Update");
     emit routeChangedEvent(args);
 }

 //fillCompanies();
 //fillComboBox();
 ui->btnOK->setEnabled(true);
 ui->btnAdd->setEnabled( false);
 bAddMode = false;

}
void RouteDlg::checkDirection(QString routeDirection)
{
    if (si.oneWay != "Y")
        return;
    if (routeDirection == si.bearing.strDirection())
        return;

    SegmentInfo siReverse = sql->getSegmentInOppositeDirection(si);
    int seq = 0;
    int companyKey = -1;
    companyKey = -1;
    RouteData rd;
    if (ui->cbCompany->currentIndex() >= 0)
    {
        //companyData cd = (companyData)cbCompany.SelectedItem;
        companyKey = _companyList.at(ui->cbCompany->currentIndex())->companyKey;
    }
    QList<SegmentData> myArray;

    if (siReverse.segmentId< 1)
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
          siReverse = SegmentInfo();
          siReverse.startLat = si.endLat;
          siReverse.startLon = si.endLon;
          siReverse.endLat = si.startLat;
          siReverse.endLon = si.startLon;
          siReverse.bearing = Bearing(siReverse.startLat, siReverse.startLon, siReverse.endLat, siReverse.endLon);
          siReverse.direction = siReverse.bearing.strDirection();
          siReverse.length = si.length;
          siReverse.startDate = si.startDate;
          siReverse.endDate = si.endDate;
          siReverse.oneWay = "Y";
          siReverse.routeType = si.routeType;
          siReverse.lineSegments = si.pointList.count();
          for(int i = siReverse.lineSegments-1; i >= 0; i --)
           siReverse.pointList.append(si.pointList.at(i));
          // Parse out the description
          siReverse.description = SegmentDescription(si.description).ReverseDescription() + " (1 way) " +  siReverse.bearing.strDirection();

          bool bAlreadyExists = false;
          siReverse.segmentId = sql->addSegment(siReverse.description, "Y", si.tracks, siReverse.routeType, siReverse.pointList, & bAlreadyExists);
          if (siReverse.segmentId != -1 && !bAlreadyExists)
          {
              myArray = sql->getSegmentData(si.segmentId);
              seq = 0;
              //foreach (segmentData sd in myArray)
              for(int i =0; i < myArray.count()-1; i++)
              {
                  SegmentData sd = myArray.at(i);
                  sql->addPoint(seq, siReverse.segmentId, sd.endLat, sd.endLon, sd.startLat, sd.startLon, sd.streetName);
                  seq++;
              }
          }
          int ix = ui->cbSegments->currentIndex();
          if (ix >= 0)
          {
           rd = _segmentInfoList.at(ix);
           if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")))
           {
            if (sql->deleteRouteSegment(_routeNbr, ui->cbRouteName->currentText(), _SegmentId,  rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")) != true)
            {
                ui->lblHelpText->setText(tr("Delete Error"));
                //System.Media.SystemSounds.Beep.Play();
                return;
            }
            int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
            QString trackUsage = " ";
            if(ui->cbOneWay)
            {
             if(ui->rbLeft->isChecked()) trackUsage = "L";
             if(ui->rbRight->isChecked()) trackUsage = "R";
            }
            if (!sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), ui->dateStart->text(), ui->dateEnd->text(),
                                        siReverse.segmentId, companyKey, /*cbTractionType.SelectedIndex+1*/tractionType,
                                        siReverse.direction, normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
            {
                ui->lblHelpText->setText(tr( "Update Error"));
                //System.Media.SystemSounds.Beep.Play();
                return;
            }
           }
          }
          // Notify whoever is interested that a segment has changed.
          ui->lblSegmentText->setText(siReverse.description);
          _SegmentId = siReverse.segmentId;
          _routeNbr = _routeNbr;
//                if (SegmentChanged != null)
//                    SegmentChanged(this, new segmentChangedEventArgs(si.SegmentId, siReverse.SegmentId));
          //segmentChangedEventArgs args = segmentChangedEventArgs(si.SegmentId, siReverse.SegmentId);
          emit SegmentChangedEvent(si.segmentId, siReverse.segmentId);
      }
          break;


      case QMessageBox::No:
      {
          // Reverse the segment and all it's points
          siReverse = SegmentInfo();
          siReverse.startLat = si.endLat;
          siReverse.startLon = si.endLon;
          siReverse.endLat = si.startLat;
          siReverse.endLon = si.startLon;
          siReverse.bearing = Bearing(siReverse.startLat, siReverse.startLon, siReverse.endLat, siReverse.endLon);
          siReverse.direction = siReverse.bearing.strDirection();
          siReverse.length = si.length;
          siReverse.startDate = si.startDate;
          siReverse.endDate = si.endDate;
          // Parse out the description
          siReverse.description =  SegmentDescription(si.description).ReverseDescription() + " (1 way) " + siReverse.direction;
          siReverse.lineSegments = si.pointList.count();

          for(int i = si.lineSegments-1; i >= 0; i --)
           siReverse.pointList.append(si.pointList.at(i));
      // Change the segment's description and then delete all the existing segments.
          ui->lblSegmentText->setText( si.description);
//                if (sql->updateSegmentDescription(si.SegmentId, si.description, si.oneWay) != true)
//                {
//                    ui->lblHelpText->setText(tr( "update description failed."));
//                    //System.Media.SystemSounds.Beep.Play();
//                    ui->rbNormal->setChecked(true);
//                }
          if(!sql->updateSegment(&siReverse))
          {
              ui->lblHelpText->setText(tr("Reverse segment failed"));
              ui->rbNormal->setChecked(true);
          }
//                myArray = sql->getSegmentData(si.SegmentId);
//                int nbrPoints = myArray.count()+1;
//                //foreach (segmentData sd in myArray)
//                for(int i=0; i < myArray.count(); i++)
//                {
//                    segmentData sd = myArray.at(i);
//                    if (sql->deletePoint(sd.sequence, si.SegmentId, nbrPoints) != true)
//                    {
//                        ui->lblHelpText->setText(tr( "delete point failed"));
//                        //System.Media.SystemSounds.Beep.Play();
//                        ui->rbNormal->setChecked(true);
//                    }
//                    nbrPoints--;
//                }

//                seq = 0;
//                //foreach (segmentData sd in myArray)
//                for(int i=0; i < myArray.count(); i++)
//                {
//                    segmentData sd = myArray.at(i);
//                    sql->addPoint(seq, si.SegmentId, sd.endLat, sd.endLon, sd.startLat, sd.startLon, sd.streetName);
//                    seq++;
//                }
          // Notify whoever is interested that a segment has changed.
//                if (SegmentChanged != null)
//                    SegmentChanged(this, new segmentChangedEventArgs(si.SegmentId, -1));
          emit SegmentChangedEvent(si.segmentId, -1);
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
             rd = (RouteData)_segmentInfoList[ix];
                                     if (sql->doesRouteSegmentExist(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")))
             {
                                     if (sql->deleteRouteSegment(_routeNbr, ui->cbRouteName->currentText(), _SegmentId, rd.startDate.toString("yyyy/MM/dd"), rd.endDate.toString("yyyy/MM/dd")) == false)
                 {
                     ui->lblHelpText->setText(tr( "route not deleted"));
                     //System.Media.SystemSounds.Beep.Play();
                     ui->rbNormal->setChecked(true);
                     return;
                 }
             }
         }

         ui->rbNormal->setChecked(true);


         QString direction = si.bearing.strDirection();
         int tractionType = _tractionList.at(ui->cbTractionType->currentIndex()).tractionType;
         QString trackUsage = " ";
         if(ui->cbOneWay)
         {
          if(ui->rbLeft->isChecked()) trackUsage = "L";
          if(ui->rbRight->isChecked()) trackUsage = "R";
         }
         if (sql->addSegmentToRoute(_routeNbr, ui->cbRouteName->currentText(), rd.startDate.toString("yyyy/MM/dd"),
                                    rd.endDate.toString("yyyy/MM/dd"), siReverse.segmentId, companyKey,
                                    /*cbTractionType.SelectedIndex+1*/tractionType, direction,
                                    normalEnter, normalLeave, reverseEnter, reverseLeave, ui->cbOneWay?"Y":"N", trackUsage))
         fillSegmentsComboBox();
         checkUpdate(__FUNCTION__);
//            if(SegmentChanged != null)
//                SegmentChanged(this, new segmentChangedEventArgs(si.SegmentId,siReverse.SegmentId));
         emit SegmentChangedEvent(si.segmentId,siReverse.segmentId);
         _SegmentId = siReverse.segmentId;
         ui->lblSegmentText->setText(siReverse.description);


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
        normalEnter = 1;
    checkUpdate(__FUNCTION__);
}
void RouteDlg::rbNFromBack_CheckedChanged(bool bChecked)
{
    if(bChecked)
        normalEnter = 0;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNFromRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
        normalEnter = 2;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNToLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
        normalLeave = 1;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNAhead_CheckedChanged(bool bChecked)
{
    if(bChecked)
        normalLeave = 0;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbNToRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
        normalLeave = 2;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseEnter = 1;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromBack_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseEnter = 0;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRFromRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseEnter = 2;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRToLeft_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseLeave = 1;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRAhead_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseLeave = 0;
    checkUpdate(__FUNCTION__);
}

void RouteDlg::rbRToRight_CheckedChanged(bool bChecked)
{
    if(bChecked)
        reverseLeave = 2;
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
 CalculateDates();

 if(cd->startDate > _rd.startDate)
 {
     ui->lblHelpText->setText(tr("company start date (")+ cd->startDate.toString("yyyy/MM/dd") + tr(") is later than route start date"));
     QApplication::beep();
     ui->cbCompany->setFocus();
     return;
 }
 if(cd->endDate < _rd.endDate)
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
     ui->rbNormal->setText(si.bearing.strDirection());
     ui->rbReverse->setText( si.bearing.strReverseDirection());
     ui->rbNormal->setChecked(true);
     if(si.tracks == 2)
      ui->gbUsage->setVisible(true);
 }
 else
 {
  ui->gbDirection->setVisible(false);
  ui->gbNormalEnter->setVisible(true);
  ui->gbNormalLeave->setVisible(true);
  ui->gbReverseEnter->setVisible(true);
  ui->gbReverseLeave->setVisible(true);
  ui->gbUsage->setVisible(false);
 }

}
