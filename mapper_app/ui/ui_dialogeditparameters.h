/********************************************************************************
** Form generated from reading UI file 'dialogeditparameters.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGEDITPARAMETERS_H
#define UI_DIALOGEDITPARAMETERS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_DialogEditParameters
{
public:
    QGridLayout *gridLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *txtName;
    QLabel *label_2;
    QLineEdit *txtTitle;
    QCheckBox *chkAlpha;
    QLabel *bndsLabel;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogEditParameters)
    {
        if (DialogEditParameters->objectName().isEmpty())
            DialogEditParameters->setObjectName("DialogEditParameters");
        DialogEditParameters->resize(400, 136);
        gridLayout = new QGridLayout(DialogEditParameters);
        gridLayout->setObjectName("gridLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label = new QLabel(DialogEditParameters);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        txtName = new QLineEdit(DialogEditParameters);
        txtName->setObjectName("txtName");

        formLayout->setWidget(0, QFormLayout::FieldRole, txtName);

        label_2 = new QLabel(DialogEditParameters);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        txtTitle = new QLineEdit(DialogEditParameters);
        txtTitle->setObjectName("txtTitle");

        formLayout->setWidget(1, QFormLayout::FieldRole, txtTitle);

        chkAlpha = new QCheckBox(DialogEditParameters);
        chkAlpha->setObjectName("chkAlpha");

        formLayout->setWidget(2, QFormLayout::FieldRole, chkAlpha);

        bndsLabel = new QLabel(DialogEditParameters);
        bndsLabel->setObjectName("bndsLabel");

        formLayout->setWidget(3, QFormLayout::FieldRole, bndsLabel);


        gridLayout->addLayout(formLayout, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(DialogEditParameters);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(DialogEditParameters);

        QMetaObject::connectSlotsByName(DialogEditParameters);
    } // setupUi

    void retranslateUi(QDialog *DialogEditParameters)
    {
        DialogEditParameters->setWindowTitle(QCoreApplication::translate("DialogEditParameters", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("DialogEditParameters", "Name:", nullptr));
        label_2->setText(QCoreApplication::translate("DialogEditParameters", "Title:", nullptr));
        chkAlpha->setText(QCoreApplication::translate("DialogEditParameters", "AlphaRoutes allowed", nullptr));
        bndsLabel->setText(QCoreApplication::translate("DialogEditParameters", "Bounds are valid", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogEditParameters: public Ui_DialogEditParameters {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGEDITPARAMETERS_H
