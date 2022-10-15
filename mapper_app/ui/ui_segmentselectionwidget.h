/********************************************************************************
** Form generated from reading UI file 'segmentselectionwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEGMENTSELECTIONWIDGET_H
#define UI_SEGMENTSELECTIONWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>
#include "ccombobox.h"

QT_BEGIN_NAMESPACE

class Ui_SegmentSelectionWidget
{
public:
    QGridLayout *gridLayout_2;
    QLabel *label_9;
    QRadioButton *rbSingle;
    QRadioButton *rbDouble;
    QRadioButton *rbBoth;
    QComboBox *cbStreets;
    CComboBox *cbSegments;

    void setupUi(QWidget *SegmentSelectionWidget)
    {
        if (SegmentSelectionWidget->objectName().isEmpty())
            SegmentSelectionWidget->setObjectName(QString::fromUtf8("SegmentSelectionWidget"));
        SegmentSelectionWidget->resize(557, 84);
        gridLayout_2 = new QGridLayout(SegmentSelectionWidget);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_9 = new QLabel(SegmentSelectionWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_2->addWidget(label_9, 0, 0, 1, 1);

        rbSingle = new QRadioButton(SegmentSelectionWidget);
        rbSingle->setObjectName(QString::fromUtf8("rbSingle"));

        gridLayout_2->addWidget(rbSingle, 0, 1, 1, 1);

        rbDouble = new QRadioButton(SegmentSelectionWidget);
        rbDouble->setObjectName(QString::fromUtf8("rbDouble"));

        gridLayout_2->addWidget(rbDouble, 0, 2, 1, 1);

        rbBoth = new QRadioButton(SegmentSelectionWidget);
        rbBoth->setObjectName(QString::fromUtf8("rbBoth"));
        rbBoth->setChecked(true);

        gridLayout_2->addWidget(rbBoth, 0, 3, 1, 1);

        cbStreets = new QComboBox(SegmentSelectionWidget);
        cbStreets->setObjectName(QString::fromUtf8("cbStreets"));
        cbStreets->setEditable(true);

        gridLayout_2->addWidget(cbStreets, 0, 4, 1, 1);

        cbSegments = new CComboBox(SegmentSelectionWidget);
        cbSegments->setObjectName(QString::fromUtf8("cbSegments"));
        cbSegments->setContextMenuPolicy(Qt::ActionsContextMenu);
        cbSegments->setEditable(true);

        gridLayout_2->addWidget(cbSegments, 1, 0, 1, 5);


        retranslateUi(SegmentSelectionWidget);

        QMetaObject::connectSlotsByName(SegmentSelectionWidget);
    } // setupUi

    void retranslateUi(QWidget *SegmentSelectionWidget)
    {
        SegmentSelectionWidget->setWindowTitle(QCoreApplication::translate("SegmentSelectionWidget", "Form", nullptr));
        label_9->setText(QCoreApplication::translate("SegmentSelectionWidget", "Select Segment:", nullptr));
#if QT_CONFIG(tooltip)
        rbSingle->setToolTip(QCoreApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display only single track segments</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        rbSingle->setText(QCoreApplication::translate("SegmentSelectionWidget", "Single", nullptr));
#if QT_CONFIG(tooltip)
        rbDouble->setToolTip(QCoreApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display only double track segments</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        rbDouble->setText(QCoreApplication::translate("SegmentSelectionWidget", "Double", nullptr));
#if QT_CONFIG(tooltip)
        rbBoth->setToolTip(QCoreApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display both single and double track segments.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        rbBoth->setText(QCoreApplication::translate("SegmentSelectionWidget", "Both", nullptr));
#if QT_CONFIG(tooltip)
        cbSegments->setToolTip(QCoreApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Select a segment from the combobox to be displayed. Or type a segment number.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
    } // retranslateUi

};

namespace Ui {
    class SegmentSelectionWidget: public Ui_SegmentSelectionWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEGMENTSELECTIONWIDGET_H
