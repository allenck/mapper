#include "editsegmentdescr.h"
#include "segmentdescription.h"
#include "clipboard.h"
#include <QContextMenuEvent>
//#include <functional>
#include <QMenu>

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
    setContextMenuPolicy(Qt::CustomContextMenu);
    Clipboard::instance()->setContextMenu(this);
    //this->func = Clipboard::instance()->getContextMenu(this);
    //connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_contextMenuEvent(QPoint)));
    connect(this, &QLineEdit::customContextMenuRequested, [=](const QPoint pos){
        QMenu* menu = this->createStandardContextMenu();
        if(Clipboard::instance()->historyCount()>1)
        {
            menu->addMenu(Clipboard::instance()->getHistoryMenu());
        }
        if(!addMenu.isEmpty())
        {
            menu->addSeparator();
            for(QAction* act : addMenu)
                menu->addAction(act);
        }
        menu->exec(QCursor::pos());
    });
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
        auto placeholder = "QLineEdit { background-color: #FFFF00 }";
        if (!sd->isValidFormat(descr))
            QLineEdit::setStyleSheet("QLineEdit { background-color: #FFC0CB }");
        else if(descr.contains(" to ") || descr.contains(" zur ") || sd->hasAbbreviations(descr))
            QLineEdit::setStyleSheet(placeholder);
        else
            QLineEdit::setStyleSheet(QString("QLineEdit { background-color: rgb(%1,%2,%3) }")
                                              .arg(defaultBgColor.red(),defaultBgColor.green(),defaultBgColor.blue()));
        setText(sd->replaceAbbreviations(descr));
        bTextEdited = false;
        emit descrUpdated(descr, sd->street());
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
    return sd->street();
}

void EditSegmentDescr::setContextMenu(QList<QAction*> list)
{
    this->addMenu = list;
}

// void EditSegmentDescr::contextMenuEvent(QContextMenuEvent *event)
// {
//     if(func != nullptr)
//         func(event->pos());
//     else
//         Clipboard::instance()->getContextMenu(this);
// }

// void EditSegmentDescr::on_contextMenuEvent(const QPoint& pt)
// {
//     if(func != nullptr)
//         func(pt);
//     else
//         QMenu menu = Clipboard::instance()->getContextMenu(this);
//     menu.exec(pt)
// }
