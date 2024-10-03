#ifndef ADDGEOREFERENCEDDIALOG_H
#define ADDGEOREFERENCEDDIALOG_H

#include <QDialog>
#include "filedownloader.h"
#include <QtSql>

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
  Overlay* overlay() {return ov;}

public slots:
 void on_buttonBoxAccepted();
 void on_nameEditingFinished();
 void on_nameTextEdited(QString txt);
 void on_sourceChanged(QString);
 void validateWMTS(QString);

 signals:
 void overlayAdded(Overlay* ov);
 void wmtsComplete();

private slots:
 void checkBounds();
 void validateValues();
 void onWmtsComplete();
 void xmlFinished();

private:
 Ui::AddGeoreferencedDialog *ui;
 Configuration* config;
 Bounds* bounds;
 QStringList overlayNames;
 bool bUpdate;
 FileDownloader* downloader = nullptr;
 QString wmtsUrl;
 QString url;
 bool dupName = false;
 Overlay* ov;
 bool bLoading = false;
};

#endif // ADDGEOREFERENCEDDIALOG_H
