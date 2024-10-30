#include "editsegmentdescr.h"
#include "segmentdescription.h"
#include "clipboard.h"

EditSegmentDescr::EditSegmentDescr(QWidget *parent) : QLineEdit(parent) {

    QPalette pal = this->palette();
    defaultBgColor = pal.color(backgroundRole());
    //defaultBgColor = QColor(Qt::white);
    sql = SQL::instance();
    connect(this, &QLineEdit::textEdited,[=]{
        bTextEdited = true;
    } );
    //sd = new SegmentDescription();
    connect(this, &QLineEdit::textChanged, [=]{
        bTextEdited = true;
    } );
    //connect(this, SIGNAL(editingFinished()), this, SLOT(on_editingFinished()));
    connect(this, &QLineEdit::editingFinished, [=]{
        on_editingFinished();
    });
    Clipboard::instance()->setContextMenu(this);
}

void EditSegmentDescr::setText(QString txt)
{
    sd = new SegmentDescription(txt);
    if(!sd->isValidFormat(txt))
        QLineEdit::setStyleSheet("QLineEdit { background-color: #FFC0CB }");
    else if(txt.contains(" to ") || txt.contains(" zur ") || sd->hasAbbreviations(txt))
        QLineEdit::setStyleSheet("QLineEdit { background-color: #FFFF00 }");
    else
        QLineEdit::setStyleSheet(QString("QLineEdit { background-color: rgb(%1,%2,%3) }")
                                          .arg(defaultBgColor.red(),defaultBgColor.green(),defaultBgColor.blue()));
    QLineEdit::setText(txt);
}

void EditSegmentDescr::setSegmentId(int segmentId)
{
    m_segmentId = segmentId;
}

void EditSegmentDescr::on_editingFinished()
{
    if (bTextEdited)
    {
        QString descr = text();
        if(!sd)
            sd = new SegmentDescription(descr);
        if(!sd->isValidFormat(descr))
            QLineEdit::setStyleSheet("QLineEdit { background-color: #FFC0CB }");
        else if(descr.contains(" to ") || descr.contains(" zur ") || sd->hasAbbreviations(descr))
            QLineEdit::setStyleSheet("QLineEdit { background-color: #FFFF00 }");
        else
            QLineEdit::setStyleSheet(QString("QLineEdit { background-color: rgb(%1,%2,%3) }")
                                              .arg(defaultBgColor.red(),defaultBgColor.green(),defaultBgColor.blue()));
        setText(sd->replaceAbbreviations(descr));
        bTextEdited = false;
    }
}

//****************************************** helper functions ***********
bool EditSegmentDescr::isValidFormat() {
    return sd->isValidFormat(text());
}

QString EditSegmentDescr::replaceAbbreviations(QString txt)
{
    return sd->replaceAbbreviations(txt);
}

QString EditSegmentDescr::streetName()
{
    return sd->Street();
}
