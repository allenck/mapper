/********************************************************************************
** Form generated from reading UI file 'replacesegmentdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REPLACESEGMENTDIALOG_H
#define UI_REPLACESEGMENTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_ReplaceSegmentDialog
{
public:
    QGridLayout *gridLayout;
    QRadioButton *rbSingle;
    QLabel *label;
    QRadioButton *rbAdd;
    QPlainTextEdit *oldSegments;
    QLabel *label_2;
    QLabel *lblHelp;
    QRadioButton *rbBoth;
    QComboBox *cbSegments;
    QPlainTextEdit *newSegments;
    QLabel *lblDetails;
    QTextEdit *details;
    QLabel *label_3;
    QLabel *label_4;
    QRadioButton *rbDouble;
    QRadioButton *rbDelete;
    QDialogButtonBox *buttonBox;
    QDateEdit *ignoreDate;

    void setupUi(QDialog *ReplaceSegmentDialog)
    {
        if (ReplaceSegmentDialog->objectName().isEmpty())
            ReplaceSegmentDialog->setObjectName(QString::fromUtf8("ReplaceSegmentDialog"));
        ReplaceSegmentDialog->resize(403, 418);
        gridLayout = new QGridLayout(ReplaceSegmentDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        rbSingle = new QRadioButton(ReplaceSegmentDialog);
        rbSingle->setObjectName(QString::fromUtf8("rbSingle"));

        gridLayout->addWidget(rbSingle, 0, 1, 1, 1);

        label = new QLabel(ReplaceSegmentDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 2, 0, 1, 2);

        rbAdd = new QRadioButton(ReplaceSegmentDialog);
        rbAdd->setObjectName(QString::fromUtf8("rbAdd"));

        gridLayout->addWidget(rbAdd, 4, 2, 1, 1);

        oldSegments = new QPlainTextEdit(ReplaceSegmentDialog);
        oldSegments->setObjectName(QString::fromUtf8("oldSegments"));

        gridLayout->addWidget(oldSegments, 3, 0, 1, 4);

        label_2 = new QLabel(ReplaceSegmentDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 4, 0, 1, 1);

        lblHelp = new QLabel(ReplaceSegmentDialog);
        lblHelp->setObjectName(QString::fromUtf8("lblHelp"));
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

        gridLayout->addWidget(lblHelp, 10, 0, 1, 4);

        rbBoth = new QRadioButton(ReplaceSegmentDialog);
        rbBoth->setObjectName(QString::fromUtf8("rbBoth"));

        gridLayout->addWidget(rbBoth, 0, 3, 1, 1);

        cbSegments = new QComboBox(ReplaceSegmentDialog);
        cbSegments->setObjectName(QString::fromUtf8("cbSegments"));

        gridLayout->addWidget(cbSegments, 1, 0, 1, 4);

        newSegments = new QPlainTextEdit(ReplaceSegmentDialog);
        newSegments->setObjectName(QString::fromUtf8("newSegments"));

        gridLayout->addWidget(newSegments, 5, 0, 1, 4);

        lblDetails = new QLabel(ReplaceSegmentDialog);
        lblDetails->setObjectName(QString::fromUtf8("lblDetails"));

        gridLayout->addWidget(lblDetails, 6, 0, 1, 1);

        details = new QTextEdit(ReplaceSegmentDialog);
        details->setObjectName(QString::fromUtf8("details"));

        gridLayout->addWidget(details, 7, 0, 1, 4);

        label_3 = new QLabel(ReplaceSegmentDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        label_4 = new QLabel(ReplaceSegmentDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 8, 0, 1, 2);

        rbDouble = new QRadioButton(ReplaceSegmentDialog);
        rbDouble->setObjectName(QString::fromUtf8("rbDouble"));

        gridLayout->addWidget(rbDouble, 0, 2, 1, 1);

        rbDelete = new QRadioButton(ReplaceSegmentDialog);
        rbDelete->setObjectName(QString::fromUtf8("rbDelete"));

        gridLayout->addWidget(rbDelete, 2, 2, 1, 1);

        buttonBox = new QDialogButtonBox(ReplaceSegmentDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::SaveAll);

        gridLayout->addWidget(buttonBox, 11, 0, 1, 4);

        ignoreDate = new QDateEdit(ReplaceSegmentDialog);
        ignoreDate->setObjectName(QString::fromUtf8("ignoreDate"));
        ignoreDate->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));

        gridLayout->addWidget(ignoreDate, 8, 2, 1, 1);


        retranslateUi(ReplaceSegmentDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ReplaceSegmentDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ReplaceSegmentDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ReplaceSegmentDialog);
    } // setupUi

    void retranslateUi(QDialog *ReplaceSegmentDialog)
    {
        ReplaceSegmentDialog->setWindowTitle(QCoreApplication::translate("ReplaceSegmentDialog", "Replace Segments", nullptr));
        rbSingle->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Single", nullptr));
        label->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Segment(s) being deleted:", nullptr));
        rbAdd->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Add", nullptr));
        label_2->setText(QCoreApplication::translate("ReplaceSegmentDialog", "New segment(s):", nullptr));
        lblHelp->setText(QCoreApplication::translate("ReplaceSegmentDialog", "help", nullptr));
        rbBoth->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Both", nullptr));
        lblDetails->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Details:", nullptr));
        label_3->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Display segments:", nullptr));
        label_4->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Ignore after this date:", nullptr));
        rbDouble->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Double", nullptr));
        rbDelete->setText(QCoreApplication::translate("ReplaceSegmentDialog", "Delete", nullptr));
        ignoreDate->setDisplayFormat(QCoreApplication::translate("ReplaceSegmentDialog", "yyyy/MM/dd", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ReplaceSegmentDialog: public Ui_ReplaceSegmentDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REPLACESEGMENTDIALOG_H
