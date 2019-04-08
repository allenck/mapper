#ifndef ADDGEOREFERENCEDDIALOG_H
#define ADDGEOREFERENCEDDIALOG_H

#include <QDialog>

namespace Ui {
class AddGeoreferencedDialog;
}

class Bounds;
class Configuration;
class AddGeoreferencedDialog : public QDialog
{
 Q_OBJECT

public:
 explicit AddGeoreferencedDialog(QWidget *parent = 0);
 ~AddGeoreferencedDialog();

public slots:
 void on_buttonBoxAccepted();
 void on_nameEditingFinished();
 void on_nameTextEdited(QString txt);
 void on_sourceChanged(QString);

private slots:
 void checkBounds();
 void validateValues();


private:
 Ui::AddGeoreferencedDialog *ui;
 Configuration* config;
 Bounds* bounds;
 QStringList overlayNames;
 bool bUpdate;
};

#endif // ADDGEOREFERENCEDDIALOG_H
