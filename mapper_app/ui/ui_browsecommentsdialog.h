/********************************************************************************
** Form generated from reading UI file 'browsecommentsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BROWSECOMMENTSDIALOG_H
#define UI_BROWSECOMMENTSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include "htmltextedit.h"

QT_BEGIN_NAMESPACE

class Ui_BrowseCommentsDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *txtKey;
    QPushButton *btnPrev;
    QPushButton *btnNext;
    HtmlTextEdit *txtComments;
    QLabel *label_2;
    QLineEdit *txtUsedByS;
    QLabel *label_3;
    QDialogButtonBox *buttonBox;
    QLineEdit *txtUsedByR;

    void setupUi(QDialog *BrowseCommentsDialog)
    {
        if (BrowseCommentsDialog->objectName().isEmpty())
            BrowseCommentsDialog->setObjectName(QStringLiteral("BrowseCommentsDialog"));
        BrowseCommentsDialog->resize(400, 342);
        gridLayout = new QGridLayout(BrowseCommentsDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(BrowseCommentsDialog);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        txtKey = new QLineEdit(BrowseCommentsDialog);
        txtKey->setObjectName(QStringLiteral("txtKey"));

        gridLayout->addWidget(txtKey, 0, 1, 1, 2);

        btnPrev = new QPushButton(BrowseCommentsDialog);
        btnPrev->setObjectName(QStringLiteral("btnPrev"));

        gridLayout->addWidget(btnPrev, 0, 3, 1, 1);

        btnNext = new QPushButton(BrowseCommentsDialog);
        btnNext->setObjectName(QStringLiteral("btnNext"));

        gridLayout->addWidget(btnNext, 0, 4, 1, 1);

        txtComments = new HtmlTextEdit(BrowseCommentsDialog);
        txtComments->setObjectName(QStringLiteral("txtComments"));

        gridLayout->addWidget(txtComments, 1, 0, 1, 5);

        label_2 = new QLabel(BrowseCommentsDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 2);

        txtUsedByS = new QLineEdit(BrowseCommentsDialog);
        txtUsedByS->setObjectName(QStringLiteral("txtUsedByS"));

        gridLayout->addWidget(txtUsedByS, 2, 2, 1, 3);

        label_3 = new QLabel(BrowseCommentsDialog);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 2);

        buttonBox = new QDialogButtonBox(BrowseCommentsDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 4, 1, 1, 4);

        txtUsedByR = new QLineEdit(BrowseCommentsDialog);
        txtUsedByR->setObjectName(QStringLiteral("txtUsedByR"));
        txtUsedByR->setClearButtonEnabled(true);

        gridLayout->addWidget(txtUsedByR, 3, 2, 1, 3);


        retranslateUi(BrowseCommentsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), BrowseCommentsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), BrowseCommentsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(BrowseCommentsDialog);
    } // setupUi

    void retranslateUi(QDialog *BrowseCommentsDialog)
    {
        BrowseCommentsDialog->setWindowTitle(QApplication::translate("BrowseCommentsDialog", "Browse Comments", nullptr));
        label->setText(QApplication::translate("BrowseCommentsDialog", "Key:", nullptr));
#ifndef QT_NO_TOOLTIP
        txtKey->setToolTip(QApplication::translate("BrowseCommentsDialog", "<html><head/><body><p>Key of currently displayed comment. Edit to display a specific comment.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        btnPrev->setText(QApplication::translate("BrowseCommentsDialog", "<-", nullptr));
        btnNext->setText(QApplication::translate("BrowseCommentsDialog", "->", nullptr));
        label_2->setText(QApplication::translate("BrowseCommentsDialog", "Used by Stations:", nullptr));
#ifndef QT_NO_TOOLTIP
        txtUsedByS->setToolTip(QApplication::translate("BrowseCommentsDialog", "<html><head/><body><p>Keys of Stations which refer to this comment.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("BrowseCommentsDialog", "Used by Routes:", nullptr));
#ifndef QT_NO_TOOLTIP
        txtUsedByR->setToolTip(QApplication::translate("BrowseCommentsDialog", "<html><head/><body><p>List of routes using this comment.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class BrowseCommentsDialog: public Ui_BrowseCommentsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BROWSECOMMENTSDIALOG_H
