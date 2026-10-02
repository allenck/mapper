#ifndef DIALOGSELECTLIST_H
#define DIALOGSELECTLIST_H

#include <QDialog>
#include <QListWidgetItem>

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
    void setCheckList(QMap<QString, bool> items);
    QMap<QString, bool> getCheckList();
    void setInstructions(QString text);

private slots:
    void onItemChanged(QListWidgetItem *item);

private:
    Ui::DialogSelectList *ui;
    QStringList list;
    QString result;
    QMap<QString, bool> checkList;
};

#endif // DIALOGSELECTLIST_H
