#ifndef EDITCITYDIALOG_H
#define EDITCITYDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class EditCityDialog;
}

class QSortFilterProxyModel;
class OverlayTableModel;
class Overlay;
class City;
class Configuration;
class EditCityDialog : public QDialog
{
 Q_OBJECT

public:
 explicit EditCityDialog(QWidget *parent = 0);
 ~EditCityDialog();

private slots:
 void cbCitysSelectionChanged(int);
 void overlaySelectionChanged(QModelIndex, bool);
 void edDescriptionTextChanged();
 void sbMinZoomValueChanged(int);
 void sbMaxZoomValueChanged(int);
 void sbOpacityValueChanged(int);
 void onClicked(QModelIndex);
// void btnAddToCityClicked();
// void btnDeleteFromCityClicked();
 void OnDescriptionChanged(bool);
 void ok_clicked();
 void on_setDirty();

private:
 Ui::EditCityDialog *ui;
 Configuration* config;
 City* c;
 int cityId;
 Overlay* ov;
 Overlay* cov;
 bool bRefreshing;
 OverlayTableModel* model;
 void setControls(bool);
 QSortFilterProxyModel* sorter;
 QHash<QString, Overlay*>* cityOverlays;
 void newCity(int i);
 bool dirty;
};

#endif // EDITCITYDIALOG_H
