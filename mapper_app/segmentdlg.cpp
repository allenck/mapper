#include "segmentdlg.h"
#include "ui_segmentdlg.h"
#include "webviewbridge.h"
#include "mainwindow.h"
#include "companyview.h"
#include "clipboard.h"
#include "segmentdescription.h"

SegmentDlg::SegmentDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SegmentDlg)
{
 ui->setupUi(this);
 myParent = parent;
 config = Configuration::instance();
 sql = SQL::instance();
 _segmentId = -1;
 _routeNbr = 0;
 _alphaRoute = "";
 _routeName = tr("No Route");
 bOriginalChanged = false;
 bNewChanged = false;
 bRouteChanged = false;
 ui->lblErrorText->setText("");
 bSplitting = false;
 strNoRoute = tr("No route selected!");
 bNewRouteNbr=false;
 tractionTypeList = sql->getTractionTypes();
 _locations = sql->getLocations();
 ui->cbLocation->clear();
 ui->cbLocation->addItems(_locations);
 ((MainWindow*)myParent)->addModeToggled(false);
// infoLat = m_latitude;
// infoLon = m_longitude;
 ui->gbUsage->setVisible(false);
 ui->rnw->configure(new RouteData(), ui->lblErrorText);
 //Clipboard::instance()->setContextMenu(ui->txtOriginalName);
 //Clipboard::instance()->setContextMenu(ui->txtNewName);
 QPalette pal = ui->txtOriginalName->palette();
 txtSegment_color = pal.color(ui->txtOriginalName->backgroundRole());

 connect(ui->txtOriginalName, SIGNAL(textChanged(QString)), this, SLOT(txtOriginalName_TextChanged(QString)));
 connect(ui->txtOriginalName, SIGNAL(editingFinished()),this,SLOT(txtOriginalName_Leave()));
 connect(ui->txtNewerName_1, &QLineEdit::editingFinished, [=]{
     SegmentDescription sdsc = SegmentDescription();
     ui->txtNewerName_1->setText(sdsc.replaceAbbreviations(ui->txtNewerName_1->text()));
 });
 Clipboard::instance()->setContextMenu(ui->txtNewerName_1);


 connect(ui->txtNewName, SIGNAL(textChanged(QString)), this, SLOT(txtNewName_TextChanged(QString)));
 connect(ui->txtNewName, SIGNAL(editingFinished()),this,SLOT(txtNewName_Leave()));
 connect(ui->txtNewerName_2, &QLineEdit::editingFinished, [=]{
     SegmentDescription sdsc = SegmentDescription();
     ui->txtNewerName_2->setText(sdsc.replaceAbbreviations(ui->txtNewerName_2->text()));
 });
 Clipboard::instance()->setContextMenu(ui->txtNewerName_2);
 connect(ui->btnCancel,SIGNAL(clicked()), this, SLOT(btnCancel_click()));
 connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(btnOK_Click()));
 connect(ui->rbUseNew, SIGNAL(clicked()), this, SLOT(rbUseNew_CheckedChanged()));
 connect(ui->chkNewOneWay, SIGNAL(clicked(bool)), this, SLOT(chkNewOneWay_Changed(bool)));
 connect(ui->cbCompany, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCompany_currentIndexChanged(int)));
 connect(ui->rbUseL, &QRadioButton::clicked, [=]{
  if(sd)
   sd->setTrackUsage("L");
 });
 connect(ui->rbUseR, &QRadioButton::clicked, [=]{
  if(sd)
   sd->setTrackUsage("R");
 });
 connect(ui->sbTracks, &QSpinBox::textChanged, [=]()
 {
  ui->gbUsage->setVisible((ui->chkNewOneWay->isChecked() && ui->sbTracks->value()==2));
 });
 connect(CompanyView::instance()->model(), SIGNAL(companyChange()), this, SLOT(fillCompanies()));

 //_routeTypeList <<"0 - Surface in street"<< "1 - Surface PRW"<< "2 - Rapid Transit" << "3 - Subway/Metro/U-Bahn"<< "4 - Heavy Rail" <<  "5 - Incline" << "6 - Other";
 _routeTypeList = SegmentData::ROUTETYPES;
 ui->cbRouteType->addItems(_routeTypeList);
 normalEnter = 0, normalLeave = 0, reverseEnter = 0, reverseLeave = 0;
 ui->btnOK->setEnabled(false);
 if (_segmentId > 0)
 {
     // setup for creating a new segment
  SegmentInfo si = sql->getSegmentInfo(_segmentId);
  ui->gbOriginal->setVisible(true);
  ui->txtOriginalName->setText(si.description());
  ui->txtNewerName_1->setText(si.newerName());
  //ui->chkOriginalOneWay->setChecked( si.oneWay()== "Y");
  ui->txtOriginalLocation->setText(si.location());

  ui->txtNewName->setText( si.description());
  ui->txtNewLocation->setText(si.location());
  ui->lbl_newerName_2->setText(si.newerName());
  //ui->chkNewOneWay->setChecked(ui->chkOriginalOneWay->isChecked());
  ui->rbUseOriginal->setEnabled(true);
  ui->rbUseNew->setChecked( true);
  bOriginalChanged = false;
  bNewChanged = false;
  if(!ui->chkOriginalOneWay->isChecked())
      ui->groupBox2->setVisible(false);
  ui->rbNfromBack->setChecked(true);
  ui->rbNAhead->setChecked(true);
  ui->rbRAhead->setChecked(true);
  ui->rbRFromBack->setChecked(true);
  ui->lbl_newerName_2->setVisible(false);
  ui->txtNewerName_2->setVisible(false);
  ui->lblNewLocation->setVisible(false);
  ui->txtNewLocation->setVisible(false);
 }
 else
 {
  //ui->gbOriginal->setVisible(false);
  ui->gbOriginal->setTitle(tr("Street"));
  ui->gbNew->setTitle(tr("Segment Description"));
  //txtOriginalName->setEnabled(false);
  ui->chkOriginalOneWay->setVisible( true);
  ui->rbUseOriginal->setEnabled( false);

  ui->chkOriginalOneWay->setEnabled( false);
  ui->chkOriginalOneWay->setVisible(false);
  this->setWindowTitle( tr("New Segment"));
  ui->rbUseNew->setChecked( true);
  bOriginalChanged = false;
  bNewChanged = false;
 }
 Parameters parms = sql->getParameters();
// ui->dateStart->setMinimumDate( parms.minDate);
// ui->dateStart->setMaximumDate( parms.maxDate);
// ui->dateEnd->setMinimumDate( parms.minDate);
// ui->dateEnd->setMaximumDate( parms.maxDate);
 if(_rd)
 {
  ui->dateStart->setDate( _rd->startDate());
  ui->dateEnd->setDate( _rd->endDate());
 }

 fillCompanies();
// ui->cbRouteName->clear();
// ui->cbRouteName->addItem(strNoRoute);
 if (_rd && _rd->route() > 0)
 {
     _routeNbr = _rd->route();
     ui->rnw->setAlphaRoute(_rd->alphaRoute());
     //cbRouteName.Text = _rd->name;
//     for(int i = 0; i < _routeNameList.count(); i++)
//     {
//         QString routeName = _routeNameList.at(i);
//         if(routeName == _rd->routeName())
//         {
//             ui->cbRouteName->setCurrentIndex(i+1);
//             break;
//         }

//     }
 }
 fillTractionTypes();

 //cbTractionType.SelectedIndex = 0;
 adjustSize();
}

SegmentDlg::~SegmentDlg()
{
    delete ui;
}

void SegmentDlg::configure(RouteData* rd, int segmentId, int point)
{
 setRouteData(rd);
 setSegmentId(segmentId);
 setPt(point);
 if(segmentId >0)
 {
  if(sql->doesRouteSegmentExist(rd->route(), rd->routeName(),segmentId, rd->startDate(),
                                rd->endDate()))
  {
   ui->rbNoAdd->setChecked(true);
   //ui->groupBox7->setEnabled(false);
  }
 }
 else
 {
  ui->rbUseOriginal->setVisible(false);
  ui->groupBox7->setEnabled(true);
  ui->dateStart->setDate(rd->startDate());
  ui->dateEnd->setDate(rd->endDate());
 }
}

void SegmentDlg::setPt(int value)
{
    _pt = value;
}

void SegmentDlg::setSegmentId(qint32 value)
{
 _segmentId = value;
 si = sql->getSegmentInfo(value);
 sd = new SegmentData(si);
 if(_segmentId> 0)
 {
  bSplitting = true;
  this->setWindowTitle(tr("Split Segment"));
  ui->gbOriginal->setVisible(true);
  ui->gbOriginal->setTitle(tr("Original segment:"));
  ui->gbNew->setTitle(tr("New segment:"));
  ui->txtOriginalName->setText( sql->getSegmentDescription(_segmentId));
  ui->cbLocation->setCurrentText(si.location());
  ui->dateStart->setDate(si.startDate());
  ui->dateEnd->setDate(si.endDate());

  ui->txtNewName->setText( ui->txtOriginalName->text());
  ui->chkNewOneWay->setChecked(ui->chkOriginalOneWay->isChecked());
  ui->rbUseOriginal->setEnabled(true);
  bOriginalChanged = false;
  bNewChanged = false;
  ui->chkOriginalOneWay->setEnabled(true);
  si =sql->getSegmentInfo(_segmentId);
  ui->cbRouteType->setCurrentIndex((qint32)si.routeType());
  if(!ui->chkOriginalOneWay->isChecked())
   ui->groupBox2->setVisible(false);
  ui->rbNfromBack->setChecked(true);
  ui->rbNAhead->setChecked(true);
  ui->rbRAhead->setChecked(true);
  ui->rbRFromBack->setChecked(true);
  ui->rnw->configure(sd, ui->lblErrorText);
 }
 else
 {
  bSplitting = false;
  this->setWindowTitle(tr("Add Segment"));
  ui->gbOriginal->setTitle(tr("Street"));
  ui->chkOriginalOneWay->setVisible(false);
  ui->lblNewLocation->setVisible(false);
  ui->txtNewLocation->setVisible(false);
  ui->lbl_newerName_2->setVisible(false);
  ui->txtNewerName_2->setVisible(false);
  ui->gbNew->setTitle(tr("Segment Description"));
  connect(ui->txtOriginalName, &QLineEdit::editingFinished, [=] {
   if(ui->txtNewName->text().isEmpty())
    ui->txtNewName->setText(ui->txtOriginalName->text()+", ");
  });

  ui->rbUseOriginal->setEnabled( false);
  ui->chkOriginalOneWay->setEnabled( false);
  ui->rbUseNew->setChecked( true);
  ui->rbUseOriginal->setEnabled( false);
  bOriginalChanged = false;
  bNewChanged = false;
  ui->rnw->configure((SegmentData*)nullptr, ui->lblErrorText);
 }
}

qint32 SegmentDlg::SegmentId()
{
    return _segmentId;
}

qint32 SegmentDlg::newSegmentId()
{
    return _newSegmentId;
}

QString SegmentDlg::street()
{
    return streetName;
}

QString SegmentDlg::routeName()
{
    return _routeName;
}

qint32 SegmentDlg::tractionType()
{
    int ix = ui->cbTractionType->currentIndex();
    TractionTypeInfo tti = (TractionTypeInfo)_tractionTypeList.values().at(ix);
    return tti.tractionType;
}
bool SegmentDlg::oneWay() { return ui->chkOriginalOneWay->isChecked(); }
int SegmentDlg::tracks() { return si.tracks(); }

void SegmentDlg::setRouteData(RouteData* value)
{
    int i = 0;
    //bRouteChanging = true;
    if (!value || value->route() < 1)
        return;
    _rd = value;
    _routeNbr = _rd->route();
    ui->rnw->configure(value, ui->lblErrorText);
    ui->groupBox7->setEnabled(true);
    QThread::msleep(100);
    ui->groupBox7->setEnabled(false);

    _routeNameList = sql->getRouteNames(_routeNbr);
    Parameters parms = sql->getParameters();
//    ui->dateStart->setMinimumDate( parms.minDate);
//    ui->dateStart->setMaximumDate( parms.maxDate);
//    ui->dateEnd->setMinimumDate( parms.minDate);
//    ui->dateEnd->setMaximumDate( parms.maxDate);
    ui->dateStart->setDate( _rd->startDate());
    ui->dateEnd->setDate( _rd->endDate());
    ui->cbTractionType->setCurrentIndex(ui->cbTractionType->findData(_rd->tractionType()));

    //if (ui->cbCompany->currentIndex() == -1)
    {
        //qint32 companyKey = sql->getDefaultCompany(_routeNbr, ui->dateEnd->dateTime().toString("yyyy/MM/dd"));
        for (i=0;i < _companyList.count(); i++)
        {
            CompanyData* cd = (CompanyData*)_companyList.at(i);
            if(cd->companyKey == _rd->companyKey())
            {
                ui->cbCompany->setCurrentIndex(i);
                break;
            }
        }
    }
    ui->rnw->setCompanyKey(ui->cbCompany->currentData().toInt());
}

void SegmentDlg::fillCompanies()
{
    ui->cbCompany->clear();
    //cbCompany.Text = " ";
    _companyList = sql->getCompanies();
    if ( _companyList.count() == 0)
        return;
    //foreach (companyData cd in _companyList)
    for(int i = 0; i< _companyList.count(); i++)
    {
     CompanyData* cd = _companyList.at(i);
     ui->cbCompany->addItem(cd->toString(), cd->companyKey);
    }
    ui->cbCompany->findData(((MainWindow*)myParent)->ui->cbCompany->currentData());

}

void SegmentDlg::cbCompany_currentIndexChanged(int companyKey)
{
    ui->lblErrorText->setText("");
    if(companyKey < 1)
     return;
 CompanyData* cd = _companyList.at(companyKey);
// ui->dateStart->setMaximumDate(cd->startDate);
// ui->dateEnd->setMaximumDate(cd->endDate);
 if(ui->dateStart->date() > cd->endDate || ui->dateStart->date() < cd->startDate)
 {
  ui->dateStart->setDate(cd->startDate);
 }
 if(ui->dateEnd->date() > cd->endDate)
 {
  ui->dateEnd->setDate(cd->endDate);
 }

 emit companySelectionChanged(companyKey);
}

void SegmentDlg::fillTractionTypes()
{
    ui->cbTractionType->clear();
    //cbTractionType.Text = " ";
    _tractionTypeList = sql->getTractionTypes();
    if ( _tractionTypeList.count() == 0)
        return;
    //foreach (tractionTypeInfo tti in tractionList)
    for(int i = 0; i < _tractionTypeList.count(); i++)
    {
     TractionTypeInfo tti = _tractionTypeList.values().at(i);
     ui->cbTractionType->addItem(tti.ToString(), tti.tractionType);
    }
    //cbTractionType.Text = "";
}

void SegmentDlg::checkUpdate()
{
 ui->lblErrorText->clear();
 //if(ui->txtRouteNbr->text() == "" || ui->txtRouteNbr->text().startsWith(" "))
 if(!ui->rbNoAdd &&(ui->rnw->alphaRoute()=="" || ui->rnw->alphaRoute().startsWith(" ")))
 {
     //ui->txtRouteNbr->setFocus();
     ui->lblErrorText->setText(tr("route number?"));
     ui->btnOK->setEnabled(false);
     return;
 }
 //if(ui->cbRouteName->currentIndex() < 0 || ui->cbRouteName->currentText() == "" ||ui->cbRouteName->currentText() ==  strNoRoute)
 if(!ui->rbNoAdd && ui->rnw->newRouteName() == "")
 {
     ui->lblErrorText->setText(tr("need Route name"));
     ui->btnOK->setEnabled(false);
     return;
 }
 if(!ui->rbNoAdd->isChecked())
 {
     if(ui->cbCompany->currentIndex()< 0)
     {
         ui->cbCompany->setFocus();
         ui->btnOK->setEnabled(false);
         return;
     }
     CompanyData* cd = _companyList.at(ui->cbCompany->currentIndex());
 }
 ui->btnOK->setEnabled(true);
}
#if 0
void SegmentDlg::txtRouteNbr_Leave()    // SLOT
{
 bool bAlphaRoute = false;
 bNewRouteNbr = false;
 if(ui->txtRouteNbr->text() == "" || ui->txtRouteNbr->text().startsWith(" "))
 {
  ui->lblErrorText->setText( tr("Cannot be blank"));
  ui->txtRouteNbr->setFocus();
  return;
 }
 ui->lblErrorText->setText("");
 int companyKey = ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
 if(ui->txtRouteNbr->text().contains(","))
 {
  int nxt = sql->findNextRouteInRange(ui->txtRouteNbr->text());
  if(nxt >= 0)
  {
   ui->txtRouteNbr->setText(QString::number(nxt));
  }
 }
 _routeNbr = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, companyKey);
 if(_routeNbr < 0)
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
 if (!config->currCity->bAlphaRoutes && bAlphaRoute)
 {
  ui->lblErrorText->setText( tr("Must be a number!"));
  ui->txtRouteNbr->setFocus();
  return;
 }
 _routeNameList = sql->getRouteNames(_routeNbr);
 ui->cbRouteName->clear();
 ui->cbRouteName->addItem(strNoRoute);
 if (_routeNameList.count() == 0)
 {
//        cbRouteName.Text = "";
//        cbRouteName.Focus();
     return;
 }
 //foreach (string str in myArray)
 for(int i=0; i <_routeNameList.count();i ++)
 {
     ui->cbRouteName->addItem(_routeNameList.at(i));
 }
 for(int i=0; i <_routeNameList.count();i ++)
 {
     if (_rd->route() > 0  && _routeNbr == _rd->route() && _rd->routeName() == _routeNameList.at(i))
     {
         ui->cbRouteName->setCurrentIndex(i+1);
         break;
     }
 }
 QList<RouteData> myArray = sql->getRouteInfo(_routeNbr);

 if (myArray.count() > 0)
 {
     //foreach (routeData rd in myArray)
     for(int i=0; i<myArray.count(); i++)
     {
         RouteData rd = myArray.at(i);
         for(int i=0; i <_routeNameList.count();i ++)
         {
             if (rd.routeName() == _routeNameList.at(i))
             {
                 ui->cbRouteName->setCurrentIndex(i+1);
                 break;
             }
         }
         if (bRouteChanged)
         {
             ui->dateStart->setDate(rd.startDate());
             ui->dateEnd->setDate( rd.endDate());
             int companyKey = sql->getRouteCompany(_routeNbr);
             //foreach (companyData cd in _companyList)
             for(int j = 0; j < _companyList.count(); j++)
             {
                 CompanyData* cd =_companyList.at(j);
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
 else
     //cbRouteName.Text = "No route selected!";
     ui->cbRouteName->setCurrentIndex(0);

 checkUpdate();
}
#endif
void SegmentDlg::rbUseOriginal_CheckedChanged()     // SLOT
{
    if(!ui->rbUseOriginal->isChecked())
        return;
    //rbUseNew.Checked = false;
//    ui->cbRouteName->setEnabled(true);
//    ui->txtRouteNbr->setEnabled(true);
    ui->rnw->setEnabled(true);
    ui->cbCompany->setEnabled(true);
    ui->cbTractionType->setEnabled(true);
    ui->dateStart->setEnabled(true);
    ui->dateEnd->setEnabled(true);
    if (ui->chkOriginalOneWay->isChecked())
    {
        ui->groupBox2->setVisible(true);
        pi = sql->getPointInfo(_pt, _segmentId);
        bearing = Bearing(si.startLat(), si.startLon(), pi.lat(), pi.lon());
        ui->rbNormal->setText( bearing.strDirection());
        ui->rbReverse->setText( bearing.strReverseDirection());

        ui->rbNormal->setChecked(true);
        ui->groupBox5->setVisible( false);
        ui->groupBox6->setVisible( false);
    }
    else
    {
        ui->groupBox2->setVisible( false);
        ui->groupBox5->setVisible( true);
        ui->groupBox6->setVisible( true);
    }
    checkUpdate();
}

void SegmentDlg::rbUseNew_CheckedChanged()      // SLOT
{
    if(!ui->rbUseNew->isChecked())
        return;
    //rbUseOriginal.Checked = false;
//    ui->cbRouteName->setEnabled(true);
//    ui->txtRouteNbr->setEnabled(true);
    ui->rnw->setEnabled(true);
    ui->cbCompany->setEnabled(true);
    ui->cbTractionType->setEnabled(true);
    ui->dateStart->setEnabled(true);
    ui->dateEnd->setEnabled(true);

    if (ui->chkNewOneWay->isChecked())
    {
        ui->groupBox2->setVisible(true);
        pi = sql->getPointInfo(_pt, _segmentId);
        bearing = Bearing(pi.lat(), pi.lon(), si.endLat(), si.endLon());
        ui->rbNormal->setText( si.bearing().strDirection());
        ui->rbReverse->setText( si.bearing().strReverseDirection());

        ui->rbNormal->setChecked( true);
    }
    else
        ui->groupBox2->setVisible(false);
    checkUpdate();
}
#if 0
void SegmentDlg::txtRouteName_TextChanged(QString text)  // SLOT
{
    Q_UNUSED(text)
    ui->cbRouteName->setEnabled(true);
    ui->txtRouteNbr->setEnabled(true);
    ui->cbCompany->setEnabled(true);
    ui->dateStart->setEnabled(true);
    ui->dateEnd->setEnabled(true);
    checkUpdate();
}
#endif
void SegmentDlg::rbNoAdd_CheckedChanged()   // SLOT
{
    if(!ui->rbNoAdd->isChecked())
        return;
//    ui->cbRouteName->setEnabled(false);
//    ui->txtRouteNbr->setEnabled(false);
    ui->rnw->setEnabled(false);
    ui->cbCompany->setEnabled(false);
    ui->dateStart->setEnabled(false);
    ui->dateEnd->setEnabled(false);
    ui->groupBox2->setVisible(false);
    checkUpdate();
}
#if 0
void SegmentDlg::cbRouteName_Leave()        // SLOT
{
    if (ui->cbRouteName->currentText().length() == 0)
    {
        ui->cbRouteName->setFocus();
        return;
    }
    ui->lblErrorText->setText("");
    QList<RouteData> rdList = sql->getRouteDataForRouteName(_routeNbr, ui->cbRouteName->currentText());
    if (rdList.count()> 0)
    {
        //foreach (routeData rd in rdList)
        for(int i=0; i < rdList.count(); i++)
        {
            RouteData rd = rdList.at(i);
            if (rd.route() > 0)
            {
             ui->dateStart->setDate( rd.startDate());
              ui->dateEnd->setDate(rd.endDate());
              if (rd.startDate() < ui->dateStart->date())
                    ui->dateStart->setDate( rd.startDate());
                if (ui->dateStart > ui->dateEnd)
                    ui->dateEnd->setDate(rd.endDate());
                ui->dateEnd->setFocus();
            }
        }
    }
    checkUpdate();
}
#endif
void SegmentDlg::txtNewName_TextChanged(QString text)       // slot
{
    Q_UNUSED(text)
    bNewChanged = true;
    if(bSplitting)
     return;

    int ix = text.indexOf(",");
    QString street;
    if(ix > 0 )
     street = text.mid(0,ix);
    if(street != ui->txtOriginalName->text())
     ui->txtOriginalName->setText(street);
    //setSegmentDescr(ui->txtOriginalName);

    //ui->txtNewName->setText(SegmentDescription::instance()->replaceAbbreviations(text));
    //setSegmentDescr(ui->txtNewName);
}

void SegmentDlg::txtNewName_Leave()         // SLOT
{
    ui->lblErrorText->setText("");
    if (bOriginalChanged && bNewChanged)
        ui->btnOK->setEnabled(true);
    checkUpdate();
}

void SegmentDlg::txtOriginalName_Leave()        // SLOT
{
    if (bOriginalChanged && bNewChanged)
        ui->btnOK->setEnabled( true);
    checkUpdate();
}

#if 0
void SegmentDlg::setSegmentDescr(QLineEdit* lineEdit)
{
    QString txt = lineEdit->text();
    if(!SegmentDescription::instance()->isValidFormat(txt))
        lineEdit->setStyleSheet("QLineEdit { background-color: #FFC0CB }");
    else if(txt.contains(" to ") || txt.contains(" zur "))
        lineEdit->setStyleSheet("QLineEdit { background-color: #FFFF00 }");
    else
        lineEdit->setStyleSheet(QString("QLineEdit { background-color: rgb(%1,%2,%3) }")
                                          .arg(txtSegment_color.red(),txtSegment_color.green(),txtSegment_color.blue()));

}
#endif

void SegmentDlg::txtOriginalName_TextChanged(QString text)              // SLOT
{
    Q_UNUSED(text)
    bOriginalChanged = true;
}
void SegmentDlg::btnCancel_click()      //SLOT
{
    this->rejected();
    this->close();
}
void SegmentDlg::chkNewOneWay_Changed(bool bChecked)
{
    if(bChecked)
    {
        ui->rbNormal->setChecked(true);
        ui->groupBox5->setVisible( false);
        ui->groupBox6->setVisible( false);
        //ui->btnOK->setEnabled(true);
        if(sd)
         ui->sbTracks->setValue(sd->tracks());
        ui->sbTracks->setEnabled(true);
        if(ui->sbTracks->value()==2)
        {
         ui->gbUsage->setVisible(true);
         ui->rbBoth->setChecked(true);
         if(sd)
         {
          if(sd->trackUsage()=="L")
           ui->rbUseL->setChecked(true);
          else if(sd->trackUsage()=="R")
           ui->rbUseR->setChecked(true);
         }
        }
    }
    else
    {
        ui->rbNormal->setChecked(false);
        ui->groupBox5->setVisible( true);
        ui->groupBox6->setVisible( true);
       // ui->btnOK->setEnabled(true);
        //ui->sbTracks->setValue(sd.tracks());
        ui->sbTracks->setEnabled(false);
        ui->gbUsage->setVisible(false);
    }
    checkUpdate();
}

void SegmentDlg::btnOK_Click()  // SLOT
{
 ui->lblErrorText->setText("");
 int companyKey = -1;
 if (!ui->rbNoAdd->isChecked())
 {
  if (ui->dateStart->dateTime() >= ui->dateEnd->dateTime())
  {
   ui->lblErrorText->setText(tr("Start date must be <= end date!"));
   return;
  }
 }
 if (_segmentId >= 0)
 {
  if (ui->txtOriginalName->text().length() == 0)
  {
   ui->lblErrorText->setText(tr( "Original name must not be blank"));
   return;
  }
 }
 if (ui->txtNewName->text().length() == 0)
 {
  ui->lblErrorText->setText(tr( "New name must not be blank"));
  return;
 }
 if (sql->doesSegmentExist(ui->txtOriginalName->text(), ui->chkOriginalOneWay->isChecked()?"Y":"N", ui->cbLocation->currentText()))
 {
  ui->lblErrorText->setText(tr( "Original name '%1' already present!").arg(ui->txtOriginalName->text()));
  return;
 }
 if (sql->doesSegmentExist(ui->txtNewName->text(), ui->chkNewOneWay->isChecked()?"Y":"N", ui->cbLocation->currentText()))
 {
  ui->lblErrorText->setText(tr( "New name '%1' already present!").arg(ui->txtNewName->text()));
  return;
 }
 if (!ui->rbNoAdd->isChecked())
 {
  if (ui->cbTractionType->currentIndex() < 0)
  {
   ui->lblErrorText->setText(tr( "select a traction type"));
   return;
  }
  // get the company key selected
  if (ui->cbCompany->currentIndex() >= 0)
  {
   CompanyData* cd = _companyList.at(ui->cbCompany->currentIndex());
   companyKey = cd->companyKey;
  }
  else
  {
   if (ui->cbCompany->currentText().length() > 0)
   {
    companyKey = sql->addCompany(ui->cbCompany->currentText(), _routeNbr, ui->dateStart->dateTime().toString("yyyy/MM/dd"), ui->dateEnd->dateTime().toString("yyyy/MM/dd"));
   }
   else
   {
    ui->lblErrorText->setText(tr( "Select a company!"));
    return;
   }
  }
 }
 QString newName = "";
 QString originalName = "";
 bool bAlreadyExists = false;
 // Do the table update
 setCursor(Qt::WaitCursor);

 if (_segmentId < 0)
 {
  newName = ui->txtNewName->text();
  // if (ui->chkNewOneWay->isChecked())
  //  newName += " (1 way)";
  QString strOneWay = ui->chkNewOneWay->isChecked()?"Y":"N";
  QString strBiDirectional = ui->chkNewOneWay->isChecked()?"Y":"Y";
  RouteType rt = (RouteType)ui->cbRouteType->currentIndex();
  QList<LatLng> pointList = QList<LatLng>();
//  _newSegmentId = sql->addSegment(newName, strOneWay, ui->sbTracks->value(), rt, pointList,
//                                  ui->cbLocation->currentText(), &bAlreadyExists);
  SegmentInfo si;
  si.setDescription(newName);
  si.setNewerName(ui->txtNewerName_1->text());
  si.setLocation(ui->txtOriginalLocation->text());
  si.setTracks(ui->sbTracks->value());
  si.setRouteType(rt);
  si.setLocation(ui->cbLocation->currentText());
  si.setStartDate(ui->dateStart->date());
  si.setDoubleDate(si.startDate());
  if(ui->sbTracks->value() == 2)
   si.setDoubleDate(ui->dateStart->date());
  si.setEndDate(ui->dateEnd->date());
  if(ui->sbTracks->value() == 2)
   si.setDoubleDate(ui->dateStart->date());
  SegmentDescription* sdr = new SegmentDescription(ui->txtNewName->text());
  si.setFormatOK(sdr->isValidFormat(si.description()));
  _newSegmentId = sql->addSegment(si, &bAlreadyExists, false);
  if (_newSegmentId < 0)
  {
   ui->lblErrorText->setText(tr( "add segment failed!"));
   setCursor(Qt::ArrowCursor);
   return;
  }
  si = sql->getSegmentInfo(_newSegmentId);
  if(!ui->cbLocation->currentText().isEmpty())
  {
   QString saveLoc = ui->cbLocation->currentText();
   SegmentInfo si = sql->getSegmentInfo(_newSegmentId);
   si.setLocation(saveLoc);
   SegmentDescription* sdr = new SegmentDescription(ui->txtNewName->text());
   si.setFormatOK(sdr->isValidFormat(si.description()));

   sql->updateSegment(&si);
   ui->cbLocation->clear();
   _locations = sql->getLocations();
   ui->cbLocation->addItems(_locations);
   ui->cbLocation->setCurrentText(saveLoc);
  }
 }
 else
 {
    // splitting
  newName = ui->txtNewName->text();
  // if (ui->chkNewOneWay->isChecked())
  //  newName += " (1 way)";
  originalName = ui->txtOriginalName->text();
  // if (ui->chkOriginalOneWay->isChecked())
  //  originalName += " (1 way)";
  _newSegmentId = sql->splitSegment(_pt, _segmentId, originalName, ui->chkOriginalOneWay->isChecked()?"Y":"N",
                                    newName, ui->chkNewOneWay->isChecked()?"Y":"N", si.routeType(),
                                    (RouteType)ui->cbRouteType->currentIndex(),
                                    si.tracks(),si.tracks(), si.streetName(), si.streetName());
  if (_newSegmentId < 0)
  {
   ui->lblErrorText->setText(tr( "split segment failed!"));
   setCursor(Qt::ArrowCursor);
   return;
  }
 }

 // Route Info
 int routeSegment;
 if(ui->rbNoAdd->isChecked())
 {
  updateOtherRoutes();
  this->accept();
  setCursor(Qt::ArrowCursor);
  this->close();
 }
 _alphaRoute = ui->rnw->alphaRoute();
 if (!ui->rbNoAdd->isChecked())
 {
  QString direction;
  if (ui->rbUseOriginal->isChecked())
   routeSegment = _segmentId;
  else
   routeSegment = _newSegmentId;

  CompanyData* cd = sql->getCompany(ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt());
  _routeNbr = ui->rnw->newRoute();
  if (ui->rnw->routeNbrMustBeAdded())
   sql->addAltRoute(ui->rnw->newRoute(), ui->rnw->alphaRoute(), cd->routePrefix);
  if (_routeNbr > 0)
  {
   int tractionType = _tractionTypeList.values().at(ui->cbTractionType->currentIndex()).tractionType;

   //SegmentData* sd = sql->getSegmentData(_routeNbr, routeSegment, ui->dateStart->text(), ui->dateEnd->text());
   //if (!sd || sd->route() < 0)
   sd->setRoute(_routeNbr);
   sd->setRouteName(ui->rnw->newRouteName());
   sd->setSegmentId(routeSegment);
   sd->setStartDate(ui->dateStart->date());
   sd->setEndDate(ui->dateEnd->date());
   sd->setCompanyKey(ui->cbCompany->currentData().toUInt());
   sd->setTractionType(ui->cbTractionType->currentData().toUInt());
   sd->setLocation(ui->txtNewLocation->text());
   sd->setRouteType((RouteType)ui->cbRouteType->currentIndex());
   if(!sql->doesRouteSegmentExist(*sd))
   {
    int companyKey= ui->cbCompany->itemData(ui->cbCompany->currentIndex()).toInt();
    CompanyData* cd = sql->getCompany(companyKey);

    QString alpha = sql->getAlphaRoute(_routeNbr,cd->routePrefix);
    if (alpha == "")
    {
     _routeNbr = sql->addAltRoute(_alphaRoute, cd->routePrefix);
    }
    QString trackUsage = " ";
    QDate doubleDate;
    // // TODO: add logic to support trackUsage!
    // if (!sql->addSegmentToRoute(_routeNbr, ui->rnw->newRouteName(), ui->dateStart->date(), ui->dateEnd->date(),
    //                             routeSegment, companyKey, tractionType, direction, -1,-1,
    //                             normalEnter, normalLeave, reverseEnter, reverseLeave,
    //                             -1, -1,
    //                             (ui->chkNewOneWay->isChecked()?"Y":"N"), trackUsage, doubleDate))

    sd->setDirection(direction);
    sd->setNormalEnter(normalEnter);
    sd->setNormalLeave(normalLeave);
    sd->setReverseEnter(reverseEnter);
    sd->setReverseLeave(reverseLeave);
    sd->setOneWay(ui->chkNewOneWay->isChecked()?"Y":"N");
    sd->setTrackUsage(trackUsage);
    sd->setDoubleDate(doubleDate);
    sd->setNewerName(ui->txtNewerName_2->text());
    sd->setLocation(ui->txtNewLocation->text());
    if(!sql->addSegmentToRoute(sd))
    {
     ui->lblErrorText->setText(tr( "Add Error"));
    }
    streetName = ui->txtOriginalName->text();
    //if (routeChanged != null)
    //routeChanged(this, new routeChangedEventArgs(_routeNbr, cbRouteName.Text, routeSegment, tractionType, companyKey, dateEnd.Value, routeChangedType.Add));
    RouteChangedEventArgs args = RouteChangedEventArgs(_routeNbr, ui->rnw->newRouteName(), routeSegment, tractionType, companyKey, ui->dateEnd->date(), "Add");
    // emit a routeChanged Signal
    emit routeChangedEvent(args);
   }
   else
   {
    QString strBiDirectional = ui->chkNewOneWay->isChecked()?"N":"Y";

    // if (!sql->updateSegmentToRoute(_routeNbr, ui->rnw->newRouteName(), ui->dateStart->text(), ui->dateEnd->text(), routeSegment, companyKey, /*cbTractionType.SelectedIndex*/tractionType,
    //                                normalEnter, normalLeave, reverseEnter, reverseLeave, strBiDirectional, ui->txtNewerName_2->text()))

    sd->setDirection(direction);
    sd->setNormalEnter(normalEnter);
    sd->setNormalLeave(normalLeave);
    sd->setReverseEnter(reverseEnter);
    sd->setReverseLeave(reverseLeave);
    sd->setOneWay(ui->chkNewOneWay->isChecked()?"Y":"N");
    sd->setNewerName(ui->txtNewerName_2->text());

    if (!sql->updateRoute( *sd,*sd,true))
        ui->lblErrorText->setText(tr( "Update Error"));
    //if (routeChanged != null)
    RouteChangedEventArgs args =  RouteChangedEventArgs(_routeNbr, ui->rnw->newRouteName(), routeSegment, tractionType, companyKey, ui->dateEnd->date(), "Update");
    emit routeChangedEvent(args);
   }
  }
 }
 else
 {
  //if (routeChanged != null && _rd != null)
  RouteChangedEventArgs args = RouteChangedEventArgs(_rd->route(), _rd->routeName(), _newSegmentId,
                                                     _rd->tractionType(), _rd->companyKey(), _rd->endDate(), "Update");
  emit routeChangedEvent(args);

 }
 if(ui->rnw->newRouteName().length() > 0)
  _routeName = ui->rnw->newRouteName();
 //this.DialogResult = DialogResult.OK;

 this->accept();
 setCursor(Qt::ArrowCursor);
 this->close();
}

QString SegmentDlg::getColor(qint32 tractionType)
{
 QString color = "#DF01D7";
 //foreach (tractionTypeInfo tti in tractionTypeList)
 for(int i=0; i < tractionTypeList.count(); i++)
 {
  TractionTypeInfo tti = tractionTypeList.values().at(i);
  if (tractionType == tti.tractionType)
      return tti.displayColor;
 }
 return color;
}

void SegmentDlg::updateOtherRoutes()
{
 if(_newSegmentId >0 && _segmentId >0)
 {
  sql->addSegmentToRoutes(_newSegmentId, _segmentId);
 }
}
