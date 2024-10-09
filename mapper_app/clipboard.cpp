#include "clipboard.h"
#include <QApplication>
#include "QMimeData"
#include "qactiongroup.h"
#include "qlineedit.h"
#include <QMenu>
#include <QLineEdit>

Clipboard::Clipboard(QObject *parent)
    : QObject{parent}
{
    list = QList<QString>();
    systemClipboard = QApplication::clipboard();
    connect(systemClipboard, &QClipboard::dataChanged, [=]{
        const QMimeData *mimeData = systemClipboard->mimeData();
        QString text;
        if(mimeData->hasText() )
        {
            text = mimeData->text();
            if(list.count() > 10)
              list.removeLast();

            int ix =list.indexOf(text);
            if(ix  >=0)
            {
                list.removeAt(ix);
            }
            list.prepend(text);
            qDebug() << "list has " << list.count() << " items";
        }
    });
}

bool Clipboard::isActive()
{
    return list.count() > 1;
}

void Clipboard::setContextMenu(QLineEdit* tgt)
{
    tgt->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tgt, &QLineEdit::customContextMenuRequested, [=] (const QPoint& ){
        getMenu(tgt)->exec(QCursor::pos());

    });
}



QMenu* Clipboard::getMenu(QLineEdit* tgt)
{
    QMenu* menu = new QMenu(tr("Paste from clipboard history") ) ;
    QAction* act;
    QActionGroup* group = new QActionGroup(this);
    QLineEdit* edit = tgt;
    connect(group, &QActionGroup::triggered, [=](QAction* selected){
        edit->insert(selected->text());
    });
    for (int i=0; i < list.count(); i++) {
        act = new QAction(list.at(i), this);
        group->addAction(act);
        menu->addAction(act);
    }
    return menu;
}
