#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "qaction.h"
#include <QObject>

class QMenu;
class QActionGroup;
class QLineEdit;
class QClipboard;
class Clipboard : public QObject
{
    Q_OBJECT
public:
    explicit Clipboard(QObject *parent = nullptr);
    static Clipboard* _instance;
    static Clipboard* instance();
    void setContextMenu(QWidget *tgt);
    int historyCount();
    QMenu* getHistoryMenu();
    QMenu* getContextMenu(QWidget* tgt);

signals:
private slots:
    void on_triggered(QAction* act);
private:
    QClipboard* systemClipboard = nullptr;
    QList<QString> history;
    QActionGroup * group = nullptr;
    QWidget* _tgt = nullptr;
};

#endif // CLIPBOARD_H
