#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QList>
#include <QClipboard>
#include <QLineEdit>
class QActionGroup;
class QMenu;
class Clipboard : public QObject
{
    Q_OBJECT
public:
    explicit Clipboard(QObject *parent = nullptr);
    QList<QString> list;
    QClipboard* systemClipboard = nullptr;
    bool isActive();
    void setContextMenu(QLineEdit* tgt);
    QMenu* getMenu(QLineEdit *tgt);
signals:

private:
    QActionGroup* group = nullptr;
};

#endif // CLIPBOARD_H
