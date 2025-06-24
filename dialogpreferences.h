#ifndef DIALOGPREFERENCES_H
#define DIALOGPREFERENCES_H

#include <QDialog>
#include "configuration.h"
#include<QUrl>

namespace Ui {
class DialogPreferences;
}

class FileDownloader;
class DialogPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreferences(QWidget *parent = nullptr);
    ~DialogPreferences();

private:
    Ui::DialogPreferences *ui;
    QUrl* tileServerUrl = nullptr;
    Configuration* config = nullptr;
    bool bDirty = false;
    bool checkUrl();
    bool check_all();
    Qt::CheckState sDisplayInBrowser = Qt::CheckState::PartiallyChecked;
    FileDownloader* m_dataCtrl = nullptr;
};

#endif // DIALOGPREFERENCES_H
