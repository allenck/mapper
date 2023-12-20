/********************************************************************************
** Form generated from reading UI file 'systemconsole2.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYSTEMCONSOLE2_H
#define UI_SYSTEMCONSOLE2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SystemConsole2
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QTextEdit *console;
    QPushButton *btnClose;
    QCheckBox *chkAutoscroll;
    QCheckBox *chkAlwaysOnTop;

    void setupUi(QDialog *SystemConsole2)
    {
        if (SystemConsole2->objectName().isEmpty())
            SystemConsole2->setObjectName("SystemConsole2");
        SystemConsole2->resize(400, 300);
        gridLayout = new QGridLayout(SystemConsole2);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        console = new QTextEdit(SystemConsole2);
        console->setObjectName("console");

        verticalLayout->addWidget(console);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 3);

        btnClose = new QPushButton(SystemConsole2);
        btnClose->setObjectName("btnClose");

        gridLayout->addWidget(btnClose, 1, 0, 1, 1);

        chkAutoscroll = new QCheckBox(SystemConsole2);
        chkAutoscroll->setObjectName("chkAutoscroll");

        gridLayout->addWidget(chkAutoscroll, 1, 1, 1, 1);

        chkAlwaysOnTop = new QCheckBox(SystemConsole2);
        chkAlwaysOnTop->setObjectName("chkAlwaysOnTop");

        gridLayout->addWidget(chkAlwaysOnTop, 1, 2, 1, 1);


        retranslateUi(SystemConsole2);

        QMetaObject::connectSlotsByName(SystemConsole2);
    } // setupUi

    void retranslateUi(QDialog *SystemConsole2)
    {
        SystemConsole2->setWindowTitle(QCoreApplication::translate("SystemConsole2", "System Console", nullptr));
        btnClose->setText(QCoreApplication::translate("SystemConsole2", "Close", nullptr));
        chkAutoscroll->setText(QCoreApplication::translate("SystemConsole2", "Auto-scroll", nullptr));
        chkAlwaysOnTop->setText(QCoreApplication::translate("SystemConsole2", "Always on top", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SystemConsole2: public Ui_SystemConsole2 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSTEMCONSOLE2_H
