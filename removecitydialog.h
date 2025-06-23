#ifndef REMOVECITYDIALOG_H
#define REMOVECITYDIALOG_H

#include <QDialog>

namespace Ui {
class RemoveCityDialog;
}

class RemoveCityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveCityDialog(QWidget *parent = nullptr);
    ~RemoveCityDialog();

private:
    Ui::RemoveCityDialog *ui;
};

#endif // REMOVECITYDIALOG_H
