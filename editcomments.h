#ifndef EDITCOMMENTS_H
#define EDITCOMMENTS_H

#include <QDialog>
#include "configuration.h"
#include <QVBoxLayout>


class QAction;
class QMenu;
class QDialogButtonBox;
class QGroupBox;
class QLineEdit;
class QMenuBar;
class QPushButton;
class QTextEdit;

class EditComments : public QDialog
{
    Q_OBJECT

public:
    EditComments();
    void setConfiguration(Configuration *cfg);
    void setHTMLText(QString html);
    void setTags(QString tags);
    QString HTML();
    QString Tags();

private:
    QMenu *fileMenu;
    QMenu *formatMenu;
    Configuration *config;
    void createMenu();
    void createHorizontalGroupBox();
    QMenuBar *menuBar;
    QGroupBox *horizontalGroupBox;
    QTextEdit *txtEdit;
    QLineEdit *txtTags;
    //qint32 NumButtons;
    QPushButton *buttons[3];
    QDialogButtonBox *buttonBox;
    QAction *exitAction;
    QAction *boldAction;
    QAction *italicAction;
    QAction *underlineAct;

private slots:
    void OnBoldAction(bool checked);
    void OnItalicAction(bool checked);
    void OnUnderlineAct(bool checked);
    void OnSelectionChanged();

signals:
};

#endif // EDITCOMMENTS_H
