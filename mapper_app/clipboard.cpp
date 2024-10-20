#include "clipboard.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QLineEdit>
#include <QMenu>
#include <QActionGroup>

Clipboard* Clipboard::_instance = nullptr;
Clipboard::Clipboard(QObject *parent)
    : QObject{parent}
{
    systemClipboard = QGuiApplication::clipboard();
    connect(systemClipboard, &QClipboard::dataChanged, [=]{
        const QMimeData* mimeData = systemClipboard->mimeData();
        if(mimeData->hasText())
        {
            if(mimeData->text().trimmed().isEmpty())
                return;
            if(history.contains(mimeData->text()))
                return;
            history.prepend(mimeData->text());
            if(history.length() > 10)
                history.removeLast();
        }
    });
}

Clipboard* Clipboard::instance()
{
    if(_instance == nullptr)
        _instance = new Clipboard();
    return _instance;
}

void Clipboard::setContextMenu(QLineEdit* tgt)
{
    tgt->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tgt, &QLineEdit::customContextMenuRequested, [=](QPoint pt)
            {
                QMenu * menu = tgt->createStandardContextMenu();
                if(history.size() >1)
                {
                    menu->addMenu(getHistoryMenu());

                    connect(group, &QActionGroup::triggered, [=](QAction* act){
                        tgt->insert(act->data().toString());
                        systemClipboard->setText(act->data().toString());
                    });
                    menu->popup(pt);
                }
            });
}

int Clipboard::historyCount()
{
    return history.size();
}
QMenu *Clipboard::getHistoryMenu()
{
    QMenu* menu1 = new QMenu(tr("Paste from Clipboard History"));
    group = new QActionGroup(this);
    QAction* act;
    foreach (QString str, history) {
        act = new QAction(str, this);
        act->setData(str);
        group->addAction(act);
        menu1->addAction(act);
    }
    return menu1;
}
