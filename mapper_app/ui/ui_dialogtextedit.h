/********************************************************************************
** Form generated from reading UI file 'dialogtextedit.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGTEXTEDIT_H
#define UI_DIALOGTEXTEDIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_DialogTextEdit
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogTextEdit)
    {
        if (DialogTextEdit->objectName().isEmpty())
            DialogTextEdit->setObjectName("DialogTextEdit");
        DialogTextEdit->resize(400, 111);
        gridLayout = new QGridLayout(DialogTextEdit);
        gridLayout->setObjectName("gridLayout");
        label = new QLabel(DialogTextEdit);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEdit = new QLineEdit(DialogTextEdit);
        lineEdit->setObjectName("lineEdit");

        gridLayout->addWidget(lineEdit, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(DialogTextEdit);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 1);


        retranslateUi(DialogTextEdit);
        //QObject::connect(buttonBox, &QDialogButtonBox::accepted, DialogTextEdit, QOverload<>(&QDialog::accept));
        //QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogTextEdit, QOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(DialogTextEdit);
    } // setupUi

    void retranslateUi(QDialog *DialogTextEdit)
    {
        DialogTextEdit->setWindowTitle(QCoreApplication::translate("DialogTextEdit", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("DialogTextEdit", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogTextEdit: public Ui_DialogTextEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGTEXTEDIT_H
