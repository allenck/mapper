/********************************************************************************
** Form generated from reading UI file 'splitcompanyroutesdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPLITCOMPANYROUTESDIALOG_H
#define UI_SPLITCOMPANYROUTESDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_SplitCompanyRoutesDialog
{
public:
    QGridLayout *gridLayout;
    QFrame *line;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QDateEdit *dateEdit;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnApply;
    QPushButton *btnCancel;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *cbCompany1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QComboBox *cbCompany2;
    QLabel *lblHelp;

    void setupUi(QDialog *SplitCompanyRoutesDialog)
    {
        if (SplitCompanyRoutesDialog->objectName().isEmpty())
            SplitCompanyRoutesDialog->setObjectName("SplitCompanyRoutesDialog");
        SplitCompanyRoutesDialog->resize(400, 217);
        gridLayout = new QGridLayout(SplitCompanyRoutesDialog);
        gridLayout->setObjectName("gridLayout");
        line = new QFrame(SplitCompanyRoutesDialog);
        line->setObjectName("line");
        line->setFrameShape(QFrame::Shape::HLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);

        gridLayout->addWidget(line, 2, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_4 = new QLabel(SplitCompanyRoutesDialog);
        label_4->setObjectName("label_4");

        horizontalLayout_3->addWidget(label_4);

        dateEdit = new QDateEdit(SplitCompanyRoutesDialog);
        dateEdit->setObjectName("dateEdit");

        horizontalLayout_3->addWidget(dateEdit);


        gridLayout->addLayout(horizontalLayout_3, 4, 0, 1, 1);

        label_3 = new QLabel(SplitCompanyRoutesDialog);
        label_3->setObjectName("label_3");
        label_3->setWordWrap(true);

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        btnApply = new QPushButton(SplitCompanyRoutesDialog);
        btnApply->setObjectName("btnApply");

        horizontalLayout_4->addWidget(btnApply);

        btnCancel = new QPushButton(SplitCompanyRoutesDialog);
        btnCancel->setObjectName("btnCancel");

        horizontalLayout_4->addWidget(btnCancel);


        gridLayout->addLayout(horizontalLayout_4, 6, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(SplitCompanyRoutesDialog);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        cbCompany1 = new QComboBox(SplitCompanyRoutesDialog);
        cbCompany1->setObjectName("cbCompany1");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(cbCompany1->sizePolicy().hasHeightForWidth());
        cbCompany1->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(cbCompany1);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_2 = new QLabel(SplitCompanyRoutesDialog);
        label_2->setObjectName("label_2");

        horizontalLayout_2->addWidget(label_2);

        cbCompany2 = new QComboBox(SplitCompanyRoutesDialog);
        cbCompany2->setObjectName("cbCompany2");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(cbCompany2->sizePolicy().hasHeightForWidth());
        cbCompany2->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(cbCompany2);


        gridLayout->addLayout(horizontalLayout_2, 3, 0, 1, 1);

        lblHelp = new QLabel(SplitCompanyRoutesDialog);
        lblHelp->setObjectName("lblHelp");

        gridLayout->addWidget(lblHelp, 5, 0, 1, 1);


        retranslateUi(SplitCompanyRoutesDialog);

        QMetaObject::connectSlotsByName(SplitCompanyRoutesDialog);
    } // setupUi

    void retranslateUi(QDialog *SplitCompanyRoutesDialog)
    {
        SplitCompanyRoutesDialog->setWindowTitle(QCoreApplication::translate("SplitCompanyRoutesDialog", "Dialog", nullptr));
        label_4->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "Effective date:", nullptr));
        dateEdit->setDisplayFormat(QCoreApplication::translate("SplitCompanyRoutesDialog", "yyyy/MM/dd", nullptr));
        label_3->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "Split all routes for original company and assign to the new company on the speified date.", nullptr));
        btnApply->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "Apply", nullptr));
        btnCancel->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "Cancel", nullptr));
        label->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "Original Company:", nullptr));
        label_2->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "New Company:", nullptr));
        lblHelp->setText(QCoreApplication::translate("SplitCompanyRoutesDialog", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SplitCompanyRoutesDialog: public Ui_SplitCompanyRoutesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPLITCOMPANYROUTESDIALOG_H
