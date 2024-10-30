#include "dialogeditstreets.h"
#include "ui_dialogeditstreets.h"
#include <QMenu>

DialogEditStreets::DialogEditStreets(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogEditStreets)
{
    ui->setupUi(this);
    Parameters parms = sql->getParameters();
    abbreviations = parms.abbreviationsList;
    ui->cbAbbreviations->clear();
    for(const QPair<QString, QString> &pair : abbreviations)
    {
        ui->cbAbbreviations->addItem(pair.first+":"+pair.second);
    }
    ui->ssw->initialize();
    ui->ssw->cbSegments()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ssw->cbSegments(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_cbSegmentsCustomContextMenuRequested(QPoint)));
}

void DialogEditStreets::on_cbSegmentsCustomContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu();
        QAction* replaceAct = new QAction(tr("Replace abbreviations"),this);
    connect(replaceAct, SIGNAL(triggered(bool)), this, SLOT(replaceAbbreviations()));
        menu->addAction(replaceAct);
        menu->exec(QCursor::pos());
};

DialogEditStreets::~DialogEditStreets()
{
    delete ui;
}
#if 0
bool DialogEditStreets::isValidFormat(SegmentInfo si)
{
    return !tokenize(si.description()).isEmpty();
}

QStringList DialogEditStreets::tokenize(QString descr)
{
    QStringList rslt;
    QString tgt = descr.remove('.');
    QStringList sl1 = tgt.split(",");
    if(sl1.isEmpty() || sl1.count() !=2)
        return rslt;
    QStringList sl2 = sl1.at(1).split(" - ");
    if(sl2.count()==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    sl2 = sl1.at(1).split(" to ");
    if(sl2.count()==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    sl2 = sl1.at(1).split(" zur ");
    if(sl2.count()==2)
    {
        rslt.append(sl1.at(0));
        rslt.append(sl2);
        return rslt;
    }
    return rslt;
}

QString DialogEditStreets::buildDescription(QStringList tokens)
{
    return QString("%1, %2 - %3").arg(tokens.at(0).trimmed(),tokens.at(1).trimmed(),tokens.at(2).trimmed());
}
#endif
void DialogEditStreets::replaceAbbreviations(bool refresh)
{
    int segmentId = ui->ssw->cbSegments()->currentData().toInt();
    SegmentInfo si = sql->getSegmentInfo(segmentId);
    QString descr = si.description();

    //descr = sd->replaceAbbreviations(descr);
    si.setDescription(descr);
    sql->updateSegment(&si);
    if(refresh)
        ui->ssw->refreshSegmentCB();
}
#if 0
QString DialogEditStreets::updateToken(QString str){
    QString result = str;
    for(const QPair<QString,QString> pair : abbreviations)
    {
        if(result.endsWith(pair.second))
            break;
        if(result.endsWith(pair.first))
        {
            int loc = result.lastIndexOf(pair.first);
            QString remainder = str.mid(0,loc );
            result = remainder.append(pair.second);
            break;
        }
    }
    return result;
}
#endif
