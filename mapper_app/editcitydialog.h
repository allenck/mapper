#ifndef EDITCITYDIALOG_H
#define EDITCITYDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class EditCityDialog;
}

class QTableView;
class QItemSelection;
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
 void overlaySelectionChanged(Overlay *ov, bool);
 void edDescriptionTextChanged();
 void sbMinZoomValueChanged(int);
 void sbMaxZoomValueChanged(int);
 void sbOpacityValueChanged(int);
 void OnDescriptionChanged(bool);
 void ok_clicked();
 void on_setDirty();
 void onLatitudeChanged();
 void onLongitudeChanged();
 void apply_clicked();
 void tablev_customContextMenu( const QPoint& pt);
 void tableViewPaste();
 void on_copyCellText();
 void rowSelected(QModelIndex);
 void onDeleteRow();
 void onUpdateProperties();
 void on_deleteConnection();
 void on_pasteLatLng();
 void cbCity_customContextMenu(QPoint pt);

private:
 Ui::EditCityDialog *ui;
 Configuration* config;
 City* city;
 int cityId;
 Overlay* ov = nullptr;
 Overlay* cov;
 bool bRefreshing;
 OverlayTableModel* model;
 void setControls(bool);
 QSortFilterProxyModel* sorter;
 QMap<QString, Overlay*>* cityOverlays;
 void newCity(int i);
 bool dirty;
 void closeEvent(QCloseEvent *event);
 QModelIndex currentIndexTableView;
 bool boolGetItemTableView(QTableView *view);
 QAction* pasteLatLng;
 QAction* deleteConnection;
};

#endif // EDITCITYDIALOG_H
