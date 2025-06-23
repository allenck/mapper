#ifndef NEWCITYDIALOG_H
#define NEWCITYDIALOG_H

#include <QDialog>

namespace Ui {
class NewCityDialog;
}

class City;
class WebViewBridge;
class NewCityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewCityDialog(QWidget *parent = nullptr);
    ~NewCityDialog();

signals:
    void newCityCreated(City*);

private:
    Ui::NewCityDialog *ui;
    WebViewBridge* webViewBridge;
    void on_cityNameEntered();
    City* city = nullptr;
};

#endif // NEWCITYDIALOG_H
