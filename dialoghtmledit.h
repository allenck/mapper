#ifndef DIALOGHTMLEDIT_H
#define DIALOGHTMLEDIT_H

#include <QDialog>
#include "configuration.h"

namespace Ui {
class DialogHtmlEdit;
}

class DialogHtmlEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHtmlEdit(QWidget *parent = nullptr);
    ~DialogHtmlEdit();
    void setData(QString text);
    QString getSource();

private:
    Ui::DialogHtmlEdit *ui;
    QString htmlText;
    Configuration* config;
    bool textChangeing = false;
};

#endif // DIALOGHTMLEDIT_H
