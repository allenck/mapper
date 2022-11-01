/********************************************************************************
** Form generated from reading UI file 'replacesegmentdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REPLACESEGMENTDIALOG_H
#define UI_REPLACESEGMENTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include "segmentselectionwidget.h"

QT_BEGIN_NAMESPACE

class Ui_ReplaceSegmentDialog
{
public:
    QGridLayout *gridLayout;
    SegmentSelectionWidget *ssw;
    QLabel *label;
    QRadioButton *rbDelete;
    QPlainTextEdit *oldSegments;
    QLabel *label_2;
    QRadioButton *rbAdd;
    QPlainTextEdit *newSegments;
    QLabel *lblDetails;
    QTextEdit *details;
    QLabel *label_4;
    QDateEdit *ignoreDate;
    QLabel *lblHelp;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ReplaceSegmentDialog)
    {
        if (ReplaceSegmentDialog->objectName().isEmpty())
            ReplaceSegmentDialog->setObjectName(QStringLiteral("ReplaceSegmentDialog"));
        ReplaceSegmentDialog->resize(452, 552);
        gridLayout = new QGridLayout(ReplaceSegmentDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        ssw = new SegmentSelectionWidget(ReplaceSegmentDialog);
        ssw->setObjectName(QStringLiteral("ssw"));

        gridLayout->addWidget(ssw, 0, 0, 1, 3);

        label = new QLabel(ReplaceSegmentDialog);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 2);

        rbDelete = new QRadioButton(ReplaceSegmentDialog);
        rbDelete->setObjectName(QStringLiteral("rbDelete"));

        gridLayout->addWidget(rbDelete, 1, 2, 1, 1);

        oldSegments = new QPlainTextEdit(ReplaceSegmentDialog);
        oldSegments->setObjectName(QStringLiteral("oldSegments"));

        gridLayout->addWidget(oldSegments, 2, 0, 1, 3);

        label_2 = new QLabel(ReplaceSegmentDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        rbAdd = new QRadioButton(ReplaceSegmentDialog);
        rbAdd->setObjectName(QStringLiteral("rbAdd"));

        gridLayout->addWidget(rbAdd, 3, 2, 1, 1);

        newSegments = new QPlainTextEdit(ReplaceSegmentDialog);
        newSegments->setObjectName(QStringLiteral("newSegments"));

        gridLayout->addWidget(newSegments, 4, 0, 1, 3);

        lblDetails = new QLabel(ReplaceSegmentDialog);
        lblDetails->setObjectName(QStringLiteral("lblDetails"));

        gridLayout->addWidget(lblDetails, 5, 0, 1, 1);

        details = new QTextEdit(ReplaceSegmentDialog);
        details->setObjectName(QStringLiteral("details"));

        gridLayout->addWidget(details, 6, 0, 1, 3);

        label_4 = new QLabel(ReplaceSegmentDialog);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 7, 0, 1, 1);

        ignoreDate = new QDateEdit(ReplaceSegmentDialog);
        ignoreDate->setObjectName(QStringLiteral("ignoreDate"));
        ignoreDate->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));

        gridLayout->addWidget(ignoreDate, 7, 1, 1, 2);

        lblHelp = new QLabel(ReplaceSegmentDialog);
        lblHelp->setObjectName(QStringLiteral("lblHelp"));
        QPalette palette;
        QBrush brush(QColor(239, 41, 41, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        QBrush brush1(QColor(190, 190, 190, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        lblHelp->setPalette(palette);
        lblHelp->setWordWrap(true);

        gridLayout->addWidget(lblHelp, 8, 0, 1, 1);

        buttonBox = new QDialogButtonBox(ReplaceSegmentDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::SaveAll);

        gridLayout->addWidget(buttonBox, 9, 0, 1, 3);


        retranslateUi(ReplaceSegmentDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ReplaceSegmentDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ReplaceSegmentDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ReplaceSegmentDialog);
    } // setupUi

    void retranslateUi(QDialog *ReplaceSegmentDialog)
    {
        ReplaceSegmentDialog->setWindowTitle(QApplication::translate("ReplaceSegmentDialog", "Replace Segments", nullptr));
        label->setText(QApplication::translate("ReplaceSegmentDialog", "Segment(s) being deleted:", nullptr));
        rbDelete->setText(QApplication::translate("ReplaceSegmentDialog", "Delete", nullptr));
        label_2->setText(QApplication::translate("ReplaceSegmentDialog", "New segment(s):", nullptr));
        rbAdd->setText(QApplication::translate("ReplaceSegmentDialog", "Add", nullptr));
        lblDetails->setText(QApplication::translate("ReplaceSegmentDialog", "Details:", nullptr));
        label_4->setText(QApplication::translate("ReplaceSegmentDialog", "Ignore after this date:", nullptr));
        ignoreDate->setDisplayFormat(QApplication::translate("ReplaceSegmentDialog", "yyyy/MM/dd", nullptr));
        lblHelp->setText(QApplication::translate("ReplaceSegmentDialog", "help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ReplaceSegmentDialog: public Ui_ReplaceSegmentDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REPLACESEGMENTDIALOG_H
