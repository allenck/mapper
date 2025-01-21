#include "routenamewidget.h"
#include "ui_routenamewidget.h"
#include "mainwindow.h"
#include "clipboard.h"

RouteNameWidget::RouteNameWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RouteNameWidget)
{
 ui->setupUi(this);
 sql = SQL::instance();
 config = Configuration::instance();
 Clipboard::instance()->setContextMenu( ui->cbRouteName->lineEdit());
 connect(ui->txtRouteNbr, SIGNAL(editingFinished()), this, SLOT(txtRouteNbr_Leave()));
 connect(ui->txtRouteNbr, &QLineEdit::textEdited, config,[=](QString text){
     if(!text.isEmpty())
         bNbrEdited = true;
 });
 connect(ui->cbRouteName->lineEdit(), SIGNAL(editingFinished()),this, SLOT(txtRouteName_Leave()));
 connect(ui->cbRouteName, SIGNAL(currentTextChanged(QString)), this, SLOT(txtRouteName_Leave()));
 connect(ui->cbRouteName, SIGNAL(currentIndexChanged(int)), this, SLOT(routeChange(int)));
}

RouteNameWidget::~RouteNameWidget()
{
 delete ui;
}

void RouteNameWidget::setAlphaRoute(QString s)
{
 ui->txtRouteNbr->setText(s);
}

void RouteNameWidget::setCompanyKey(int companyKey)
{
 this->companyKey = companyKey;
}

void RouteNameWidget::setSegmentData(SegmentData* sd)
{
 this->sd = sd;
 ui->txtRouteNbr->setText(_alphaRoute = QString::number(sd->route()));
 setRouteName(sd->routeName());
}

void RouteNameWidget::setRouteData(RouteData* rd)
{
 this->rd = rd;
 ui->txtRouteNbr->setText(rd->alphaRoute());
 //setRouteName(rd->routeName());
 ui->cbRouteName->addItem(rd->toString(), QVariant::fromValue(rd));
}

void RouteNameWidget::setRouteName(QString name)
{
 if(ui->cbRouteName->findText(name)==-1)
  ui->cbRouteName->addItem(name);
 ui->cbRouteName->setCurrentText(name);
}

void RouteNameWidget::setRouteName(RouteData rd)
{

    if(ui->cbRouteName->findText(rd.routeName())==-1)
        ui->cbRouteName->addItem(rd.routeName(), QVariant::fromValue(rd));
    ui->cbRouteName->setCurrentText(rd.routeName());
}

void RouteNameWidget::configure(SegmentData* sd, QLabel *lblHelpText)
{
 this->sd = sd;
 this->lblHelpText = lblHelpText;
 if(sd == nullptr)
  return;
 ui->txtRouteNbr->setText(_alphaRoute = QString::number(sd->route()));
 setRouteName(sd->routeName());
}

void RouteNameWidget::configure(RouteData* rd, QLabel *lblHelpText)
{
 this->companyKey = rd->companyKey();
 this->rd = rd;
 _routeNbr = rd->route();
 _alphaRoute = rd->alphaRoute();
 this->lblHelpText = lblHelpText;
 ui->txtRouteNbr->setText(rd->alphaRoute());
 setRouteName(rd->routeName());
}

int RouteNameWidget::newRoute()
{
 return _routeNbr;
}

QString RouteNameWidget::newRouteName()
{
 return ui->cbRouteName->currentText();
}

void RouteNameWidget::txtRouteNbr_Leave()
{
    lblHelpText->clear();
    bNewRouteNbr = false;
    if(!bNbrEdited)
        return;
    bool isNumeric = false;
    QString ar = ui->txtRouteNbr->text();
    if(ui->txtRouteNbr->text().contains(","))
    {
     int nxt = sql->findNextRouteInRange(ui->txtRouteNbr->text());
     if(nxt >= 0)
     {
      ui->txtRouteNbr->setText(QString::number(nxt));
     }
    }
    ui->txtRouteNbr->text().toInt(&isNumeric);
    if(companyKey < 1)
    {
     //throw IllegalArgumentException("missing company key");
        lblHelpText->setText(tr("select a company"));
        return;
    }
    CompanyData* cd = sql->getCompany(companyKey);

    _routeNbr = sql->getNumericRoute(ui->txtRouteNbr->text(), & _alphaRoute, & bAlphaRoute, cd->routePrefix);
    if(_routeNbr < 0)
    {
        QMessageBox::StandardButtons rslt;
        rslt = QMessageBox::warning(this,tr("Route number not found"),
                                    tr( "The route number %1 was not found. Enter Yes to add it")
                                    .arg(ui->txtRouteNbr->text()),
                                    QMessageBox::Yes | QMessageBox::No);
        switch (rslt)
        {
            case QMessageBox::Yes:
                bNewRouteNbr = true;
                bool bok;
                _routeNbr = ui->txtRouteNbr->text().toInt(&bok);
                if(!bok ||  (!cd->routePrefix.isEmpty() && cd->routePrefix != " "))
                {
                 _routeNbr = sql->findNextRouteInRange("1000,1999");
                 ui->cbRouteName->setCurrentIndex(-1);
                }
                break;
            case QMessageBox::No:
                ui->txtRouteNbr->setText("");
                ui->txtRouteNbr->setPlaceholderText("Enter new route name");
                ui->txtRouteNbr->setFocus();
                return;
            default:
                bRouteChanging=false;
                return;
        }
    }
//    else
//     _routeNbr = newRoute;
    emit routeNumberChange(_routeNbr);

    // if (!config->currCity->bAlphaRoutes && !isNumeric)
    // {
    //     lblHelpText->setText(tr( "Alpha route not allowed; must be a number!"));
    //     //System.Media.SystemSounds.Exclamation.Play();
    //     ui->txtRouteNbr->setFocus();
    //     return;
    // }

    if(sd)
    {
     sd->setRoute(_routeNbr);
     sd->setAlphaRoute(ui->txtRouteNbr->text());
    }
    else if(rd)
    {
     rd->setRoute(_routeNbr);
     rd->setAlphaRoute(ui->txtRouteNbr->text());
    }
//    else
//     throw IllegalArgumentException("configure error");

    QList<QString> _routeNamesList = sql->getRouteNames(_routeNbr);
    ui->cbRouteName->clear();
    ui->cbRouteName->addItem("");
    ui->cbRouteName->lineEdit()->setPlaceholderText(tr("new route name"));
    if (_routeNamesList.count()>0)
    {
        //foreach (string str in myArray)
        for (int i=0; i < _routeNamesList.count(); i++)
        {
            QString str = (QString)_routeNamesList.at(i);
            ui->cbRouteName->addItem(str);
        }
        //ui->cbRouteName->setCurrentIndex(1);

            if((ar = ui->txtRouteNbr->text().trimmed()) == _rd.alphaRoute())
        {
            ui->cbRouteName->setCurrentText(_rd.routeName());
        }
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
    //ui->cbTractionType->setCurrentIndex(0);
    bRouteChanging = false;
    //checkUpdate(__FUNCTION__);
}

void RouteNameWidget::txtRouteName_Leave()
{
    if(lblHelpText == nullptr)
        return;
    lblHelpText->clear();
    if(!sql->checkRouteName(ui->cbRouteName->currentText(), rd->startDate(), rd->endDate()).isEmpty())
    {
        lblHelpText->setText(tr("possible duplicate routes exist!"));
    }
//    if(ui->cbRouteName->currentIndex() == 0)
//        ui->btnAdd->setText(tr("Add"));
    if (ui->cbRouteName->currentText().length() == 0)
    {
        ui->txtRouteNbr->setFocus();
        //System.Media.SystemSounds.Beep.Play();
        return;
    }
    if (ui->cbRouteName->currentText().length() > 140)
    {
        ui->txtRouteNbr->setFocus();
        lblHelpText->setText(tr("name > 140 characters!"));
        //System.Media.SystemSounds.Beep.Play();
        return;
    }
    if(sd)
     sd->setRouteName(ui->cbRouteName->currentText());
    else if(rd)
     rd->setRouteName(ui->cbRouteName->currentText());

    if(sd == nullptr)
     return;
    QList<RouteData> rdList = sql->getRouteDataForRouteName(sd->route(), ui->cbRouteName->currentText());
    if (rdList.count()>0)
    {
        //foreach (routeData rd in rdList)
        for(int i = 0; i < rdList.count(); i++)
        {
            RouteData rd = rdList.at(i);
            if (rd.route() > 0)
            {
//             ui->dateStart->setDate( rd.startDate());
//             ui->dateEnd->setDate(rd.endDate());
//             displayDates(__FUNCTION__);

                //cbTractionType.SelectedIndex = rd.tractionType - 1;

                //int i = cbTractionType.FindString(rd.tractionType.ToString(), -1);
                //ui->cbTractionType->setCurrentIndex(ix);
#if 0
                for( int j=0; j < _tractionList.count(); j++)
                {
                    TractionTypeInfo tt = (TractionTypeInfo)_tractionList.values().at(j);
                    if(tt.tractionType == sd->tractionType())
                    {
//                        ui->cbTractionType->setCurrentIndex(j);
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
#endif
            }
            else
            {
//                Parameters parms = sql->getParameters();
//                ui->dateStart->setMinimumDate(parms.minDate);
//                ui->dateStart->setMaximumDate(parms.maxDate);
//                ui->dateEnd->setMaximumDate(parms.maxDate);
//                ui->dateEnd->setMinimumDate(parms.minDate);
//                displayDates(__FUNCTION__);
            }
        }
    }
    else
    {
//        _rd = routeData();  // new route
//        _rd.route = _routeNbr;
//        _rd.name = ui->cbRouteName->currentText();
    }

    //checkUpdate(__FUNCTION__);
    emit routeNameChanged(ui->cbRouteName->currentText());
}

void RouteNameWidget::companyChange(int index)
{
  companyKey = MainWindow::instance()->ui->cbCompany->currentData().toInt();
}

void RouteNameWidget::clear()
{
    ui->cbRouteName->clear();
}

void RouteNameWidget::routeChange(int )
{
    QVariant v = ui->cbRouteName->currentData();
    if(v.canConvert<RouteData>())
    {
        emit rdSelected(v.value<RouteData>());
    }
}

int RouteNameWidget::getRouteId()
{
    return sd->routeId();
}
