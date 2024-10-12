#ifndef CLIPBOARD_H
#define CLIPBOARD_H

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
    void setContextMenu(QLineEdit* tgt);
    int historyCount();
    QMenu* getHistoryMenu();

signals:
private:
    QClipboard* systemClipboard;
    QList<QString> history;
    QActionGroup * group;
};

#endif // CLIPBOARD_H
