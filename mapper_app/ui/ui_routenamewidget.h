/********************************************************************************
** Form generated from reading UI file 'routenamewidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROUTENAMEWIDGET_H
#define UI_ROUTENAMEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "ccombobox.h"

QT_BEGIN_NAMESPACE

class Ui_RouteNameWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QLineEdit *txtRouteNbr;
    QLabel *label_6;
    CComboBox *cbRouteName;

    void setupUi(QWidget *RouteNameWidget)
    {
        if (RouteNameWidget->objectName().isEmpty())
            RouteNameWidget->setObjectName("RouteNameWidget");
        RouteNameWidget->resize(422, 47);
        verticalLayout = new QVBoxLayout(RouteNameWidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label = new QLabel(RouteNameWidget);
        label->setObjectName("label");

        horizontalLayout_4->addWidget(label);

        txtRouteNbr = new QLineEdit(RouteNameWidget);
        txtRouteNbr->setObjectName("txtRouteNbr");
        txtRouteNbr->setMaxLength(8);

        horizontalLayout_4->addWidget(txtRouteNbr);

        label_6 = new QLabel(RouteNameWidget);
        label_6->setObjectName("label_6");

        horizontalLayout_4->addWidget(label_6);

        cbRouteName = new CComboBox(RouteNameWidget);
        cbRouteName->setObjectName("cbRouteName");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(cbRouteName->sizePolicy().hasHeightForWidth());
        cbRouteName->setSizePolicy(sizePolicy);
        cbRouteName->setEditable(true);

        horizontalLayout_4->addWidget(cbRouteName);


        verticalLayout->addLayout(horizontalLayout_4);


        retranslateUi(RouteNameWidget);

        QMetaObject::connectSlotsByName(RouteNameWidget);
    } // setupUi

    void retranslateUi(QWidget *RouteNameWidget)
    {
        RouteNameWidget->setWindowTitle(QCoreApplication::translate("RouteNameWidget", "Form", nullptr));
        label->setText(QCoreApplication::translate("RouteNameWidget", "Route:", nullptr));
#if QT_CONFIG(tooltip)
        txtRouteNbr->setToolTip(QCoreApplication::translate("RouteNameWidget", "<html><head/><body><p>Enter an existing route number, e.g. '20' or 'M1'. Enter a range, eg. '200,299' to create a new numeric number in that range.</p><p>Alpha routes are only allowed if the alpha route option is set in Parameters. However, route numbers like '01' are treated as numeric  and the numeric route number will be the numeric value, eg '1'. The equivalence between an alpha route and numeric route number is stored in the AltRoute table.</p><p>Alpha routes such as 'M1' will have a numeric route assignedin the range 1000-9999'.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_6->setText(QCoreApplication::translate("RouteNameWidget", "Name:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RouteNameWidget: public Ui_RouteNameWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROUTENAMEWIDGET_H
