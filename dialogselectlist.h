#ifndef DIALOGSELECTLIST_H
#define DIALOGSELECTLIST_H

#include <QDialog>

namespace Ui { class DialogSelectList; }

class DialogSelectList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectList(QWidget *parent = nullptr);
    DialogSelectList(QStringList list, QWidget *parent = nullptr);
    ~DialogSelectList();
    void setList(QStringList);
    QString getResult();

private:
    Ui::DialogSelectList *ui;
    QStringList list;
    QString result;
};

#endif // DIALOGSELECTLIST_H
