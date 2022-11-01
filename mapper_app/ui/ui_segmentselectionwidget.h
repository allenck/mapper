/********************************************************************************
** Form generated from reading UI file 'segmentselectionwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
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

QT_BEGIN_NAMESPACE

class Ui_SegmentSelectionWidget
{
public:
    QGridLayout *gridLayout;
    QLabel *label_9;
    QRadioButton *rbSingle;
    QRadioButton *rbDouble;
    QRadioButton *rbBoth;
    QLabel *label;
    QComboBox *cbStreets;
    QComboBox *cbSegments;

    void setupUi(QWidget *SegmentSelectionWidget)
    {
        if (SegmentSelectionWidget->objectName().isEmpty())
            SegmentSelectionWidget->setObjectName(QStringLiteral("SegmentSelectionWidget"));
        SegmentSelectionWidget->resize(557, 84);
        gridLayout = new QGridLayout(SegmentSelectionWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_9 = new QLabel(SegmentSelectionWidget);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        rbSingle = new QRadioButton(SegmentSelectionWidget);
        rbSingle->setObjectName(QStringLiteral("rbSingle"));

        gridLayout->addWidget(rbSingle, 0, 1, 1, 1);

        rbDouble = new QRadioButton(SegmentSelectionWidget);
        rbDouble->setObjectName(QStringLiteral("rbDouble"));

        gridLayout->addWidget(rbDouble, 0, 2, 1, 1);

        rbBoth = new QRadioButton(SegmentSelectionWidget);
        rbBoth->setObjectName(QStringLiteral("rbBoth"));
        rbBoth->setChecked(true);

        gridLayout->addWidget(rbBoth, 0, 3, 1, 1);

        label = new QLabel(SegmentSelectionWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 0, 4, 1, 1);

        cbStreets = new QComboBox(SegmentSelectionWidget);
        cbStreets->setObjectName(QStringLiteral("cbStreets"));
        cbStreets->setEditable(true);

        gridLayout->addWidget(cbStreets, 0, 5, 1, 1);

        cbSegments = new QComboBox(SegmentSelectionWidget);
        cbSegments->setObjectName(QStringLiteral("cbSegments"));
        cbSegments->setContextMenuPolicy(Qt::ActionsContextMenu);
        cbSegments->setEditable(true);

        gridLayout->addWidget(cbSegments, 1, 0, 1, 6);


        retranslateUi(SegmentSelectionWidget);

        QMetaObject::connectSlotsByName(SegmentSelectionWidget);
    } // setupUi

    void retranslateUi(QWidget *SegmentSelectionWidget)
    {
        SegmentSelectionWidget->setWindowTitle(QApplication::translate("SegmentSelectionWidget", "Form", nullptr));
        label_9->setText(QApplication::translate("SegmentSelectionWidget", "Select Segment:", nullptr));
#ifndef QT_NO_TOOLTIP
        rbSingle->setToolTip(QApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display only single track segments</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        rbSingle->setText(QApplication::translate("SegmentSelectionWidget", "Single", nullptr));
#ifndef QT_NO_TOOLTIP
        rbDouble->setToolTip(QApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display only double track segments</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        rbDouble->setText(QApplication::translate("SegmentSelectionWidget", "Double", nullptr));
#ifndef QT_NO_TOOLTIP
        rbBoth->setToolTip(QApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Click to display both single and double track segments.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        rbBoth->setText(QApplication::translate("SegmentSelectionWidget", "Both", nullptr));
        label->setText(QApplication::translate("SegmentSelectionWidget", "Street: ", nullptr));
#ifndef QT_NO_TOOLTIP
        cbSegments->setToolTip(QApplication::translate("SegmentSelectionWidget", "<html><head/><body><p>Select a segment from the combobox to be displayed. Or type a segment number.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class SegmentSelectionWidget: public Ui_SegmentSelectionWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEGMENTSELECTIONWIDGET_H
