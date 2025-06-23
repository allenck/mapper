#include "clipboard.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QLineEdit>
#include <QMenu>
#include <QActionGroup>
#include <QComboBox>

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
            if(history.length() > 10)
                history.removeLast();
            history.prepend(mimeData->text());
        }
    });
}

Clipboard* Clipboard::instance()
{
    if(_instance == nullptr)
        _instance = new Clipboard();
    return _instance;
}

void Clipboard::setContextMenu(QWidget* tgt)
{
    if(tgt == nullptr)
        return;
    int policy = tgt->contextMenuPolicy();
    // if the policy is already set, assume that this is a sub-menu to the overriding context menu,
    // s we won't execute it here!
    if(policy != Qt::CustomContextMenu)
        tgt->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tgt, &QWidget::customContextMenuRequested, tgt, [=](QPoint)
    {
        QMenu* menu = getContextMenu(tgt);
        if(policy != Qt::CustomContextMenu)
        {
            menu->exec(QCursor::pos());
        }
        menu->deleteLater();

    });
}

QMenu* Clipboard::getContextMenu(QWidget *tgt)
{
    QMenu * menu;
    if(qobject_cast<QLineEdit*>(tgt))
        menu = ((QLineEdit*)tgt)->createStandardContextMenu();
    else if(qobject_cast<QComboBox*>(tgt) && qobject_cast<QComboBox*>(tgt)->isEditable())
        menu = ((QComboBox*)tgt)->lineEdit()->createStandardContextMenu();
    else
        menu = new QMenu();
    _tgt = tgt;
    if(history.size() >1)
    {
        menu->addMenu(getHistoryMenu());

        // connect(group, &QActionGroup::triggered, menu,[=](QAction* act){
        //     qDebug() << tr("insert '%1 into %2").arg(act->data().toString(), tgt->objectName());
        //     tgt->insert(act->data().toString());
        //     systemClipboard->setText(act->data().toString());
        // });
        connect(group, SIGNAL(triggered(QAction*)), this, SLOT(on_triggered(QAction*)));
        //menu->popup(pt);
    }

    return menu;
}

void Clipboard::on_triggered(QAction* act)
{
    qDebug() << tr("insert '%1 into %2").arg(act->data().toString(), _tgt->objectName());
    if(qobject_cast<QLineEdit*>(_tgt))
            ((QLineEdit*)_tgt)->insert(act->data().toString());
    else if(qobject_cast<QComboBox*>(_tgt) && qobject_cast<QComboBox*>(_tgt)->isEditable())
        ((QComboBox*)_tgt)->lineEdit()->insert(act->data().toString());
    systemClipboard->setText(act->data().toString());
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
        connect(act, &QAction::triggered,menu1,[=](bool){
            qDebug() << tr("insert '%1 into %2").arg(act->data().toString(), _tgt->objectName());
            if(qobject_cast<QLineEdit*>(_tgt))
                ((QLineEdit*)_tgt)->insert(act->data().toString());
            else if(qobject_cast<QComboBox*>(_tgt) && qobject_cast<QComboBox*>(_tgt)->isEditable())
                ((QComboBox*)_tgt)->lineEdit()->insert(act->data().toString());
            systemClipboard->setText(act->data().toString());

        });
    }
    return menu1;
}
