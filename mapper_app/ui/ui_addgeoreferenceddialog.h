/********************************************************************************
** Form generated from reading UI file 'addgeoreferenceddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDGEOREFERENCEDDIALOG_H
#define UI_ADDGEOREFERENCEDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include "htmltextedit.h"

QT_BEGIN_NAMESPACE

class Ui_AddGeoreferencedDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QLabel *label_8;
    QTextEdit *edUrl;
    QLabel *label_2;
    QSpinBox *sbMinZoom;
    QLabel *label_3;
    QSpinBox *sbMaxZoom;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLineEdit *swLat;
    QLineEdit *swLon;
    QLineEdit *neLat;
    QLineEdit *neLon;
    QLabel *label;
    QLineEdit *edName;
    QLabel *label_9;
    QLabel *lblErr;
    HtmlTextEdit *description;
    QLabel *label_10;
    QComboBox *comboBox;

    void setupUi(QDialog *AddGeoreferencedDialog)
    {
        if (AddGeoreferencedDialog->objectName().isEmpty())
            AddGeoreferencedDialog->setObjectName(QString::fromUtf8("AddGeoreferencedDialog"));
        AddGeoreferencedDialog->resize(527, 456);
        gridLayout = new QGridLayout(AddGeoreferencedDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        buttonBox = new QDialogButtonBox(AddGeoreferencedDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 10, 0, 1, 2);

        label_8 = new QLabel(AddGeoreferencedDialog);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 7, 0, 1, 1);

        edUrl = new QTextEdit(AddGeoreferencedDialog);
        edUrl->setObjectName(QString::fromUtf8("edUrl"));

        gridLayout->addWidget(edUrl, 8, 0, 1, 4);

        label_2 = new QLabel(AddGeoreferencedDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        sbMinZoom = new QSpinBox(AddGeoreferencedDialog);
        sbMinZoom->setObjectName(QString::fromUtf8("sbMinZoom"));
        sbMinZoom->setMaximum(18);

        gridLayout->addWidget(sbMinZoom, 1, 1, 1, 1);

        label_3 = new QLabel(AddGeoreferencedDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 2, 1, 1);

        sbMaxZoom = new QSpinBox(AddGeoreferencedDialog);
        sbMaxZoom->setObjectName(QString::fromUtf8("sbMaxZoom"));
        sbMaxZoom->setMaximum(21);
        sbMaxZoom->setValue(17);

        gridLayout->addWidget(sbMaxZoom, 1, 3, 1, 1);

        groupBox = new QGroupBox(AddGeoreferencedDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 0, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 0, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_2->addWidget(label_6, 0, 2, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_2->addWidget(label_7, 0, 3, 1, 1);

        swLat = new QLineEdit(groupBox);
        swLat->setObjectName(QString::fromUtf8("swLat"));

        gridLayout_2->addWidget(swLat, 1, 0, 1, 1);

        swLon = new QLineEdit(groupBox);
        swLon->setObjectName(QString::fromUtf8("swLon"));

        gridLayout_2->addWidget(swLon, 1, 1, 1, 1);

        neLat = new QLineEdit(groupBox);
        neLat->setObjectName(QString::fromUtf8("neLat"));

        gridLayout_2->addWidget(neLat, 1, 2, 1, 1);

        neLon = new QLineEdit(groupBox);
        neLon->setObjectName(QString::fromUtf8("neLon"));

        gridLayout_2->addWidget(neLon, 1, 3, 1, 1);


        gridLayout->addWidget(groupBox, 2, 0, 1, 4);

        label = new QLabel(AddGeoreferencedDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        edName = new QLineEdit(AddGeoreferencedDialog);
        edName->setObjectName(QString::fromUtf8("edName"));

        gridLayout->addWidget(edName, 0, 1, 1, 2);

        label_9 = new QLabel(AddGeoreferencedDialog);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 3, 0, 1, 1);

        lblErr = new QLabel(AddGeoreferencedDialog);
        lblErr->setObjectName(QString::fromUtf8("lblErr"));
        QPalette palette;
        QBrush brush(QColor(255, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        QBrush brush1(QColor(190, 190, 190, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        lblErr->setPalette(palette);

        gridLayout->addWidget(lblErr, 9, 0, 1, 4);

        description = new HtmlTextEdit(AddGeoreferencedDialog);
        description->setObjectName(QString::fromUtf8("description"));

        gridLayout->addWidget(description, 4, 0, 1, 4);

        label_10 = new QLabel(AddGeoreferencedDialog);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout->addWidget(label_10, 5, 0, 1, 1);

        comboBox = new QComboBox(AddGeoreferencedDialog);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        gridLayout->addWidget(comboBox, 5, 1, 1, 1);


        retranslateUi(AddGeoreferencedDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AddGeoreferencedDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AddGeoreferencedDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AddGeoreferencedDialog);
    } // setupUi

    void retranslateUi(QDialog *AddGeoreferencedDialog)
    {
        AddGeoreferencedDialog->setWindowTitle(QCoreApplication::translate("AddGeoreferencedDialog", "Update Overlay Information", nullptr));
        label_8->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Url:", nullptr));
#if QT_CONFIG(tooltip)
        edUrl->setToolTip(QCoreApplication::translate("AddGeoreferencedDialog", "<html><head/><body><p>Enter either a Web Map Tile Service(<span style=\" font-weight:600;\">WMTS</span>) capabilities request URL request or an XYZ link Url.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Minimum Zoom:", nullptr));
        label_3->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Maximum Zoom:", nullptr));
        groupBox->setTitle(QCoreApplication::translate("AddGeoreferencedDialog", "Bounds:", nullptr));
        label_4->setText(QCoreApplication::translate("AddGeoreferencedDialog", "SW Latitude:", nullptr));
        label_5->setText(QCoreApplication::translate("AddGeoreferencedDialog", "SW Longitude:", nullptr));
        label_6->setText(QCoreApplication::translate("AddGeoreferencedDialog", "NE Latitude", nullptr));
        label_7->setText(QCoreApplication::translate("AddGeoreferencedDialog", "NE Longitude", nullptr));
        label->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Name:", nullptr));
#if QT_CONFIG(tooltip)
        edName->setToolTip(QCoreApplication::translate("AddGeoreferencedDialog", "<html><head/><body><p>Enter a name by which this overlay will be known.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_9->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Description:", nullptr));
        lblErr->setText(QString());
        label_10->setText(QCoreApplication::translate("AddGeoreferencedDialog", "Source:", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("AddGeoreferencedDialog", "acksoft", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("AddGeoreferencedDialog", "georeferencer", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("AddGeoreferencedDialog", "mbtiles", nullptr));

    } // retranslateUi

};

namespace Ui {
    class AddGeoreferencedDialog: public Ui_AddGeoreferencedDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDGEOREFERENCEDDIALOG_H
