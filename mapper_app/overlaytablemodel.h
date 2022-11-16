#ifndef OVERLAYTABLEMODEL_H
#define OVERLAYTABLEMODEL_H

#include <QAbstractTableModel>

class Configuration;
class Overlay;
class OverlayTableModel : public QAbstractTableModel
{
 Q_OBJECT
public:
 OverlayTableModel(int cityId, QObject* parent = nullptr);
 int rowCount(const QModelIndex &parent) const;
 int columnCount(const QModelIndex &parent) const;
 Qt::ItemFlags flags(const QModelIndex &index) const;
 QVariant headerData(int section, Qt::Orientation orientation, int role) const;
 QVariant data(const QModelIndex &index, int role) const;
 bool setData(const QModelIndex &index, const QVariant &value, int role);
 enum COLUMNS
 {
  NAME,
  SELECTED,
  CITYNAME,
  YEAR,
  DESCRIPTION,
  SOURCE,
  BOUNDS,
//  CENTER,
  MINZOOM,
  MAXZOOM,
  OPACITY,
  LOCAL,
  URLS,
  NUMCOLUMNS
 };
 void setCity(int);
 void addOverlay(Overlay* ov);
 QMap<QString, Overlay *> *getOverlayMap();
 void deleteRow(int row);
 Overlay* selectedOverlay(int row);
signals:
 void setDirty();
 void overlaySelectionChanged(Overlay* ov, bool checked);
 void overlayChanged(QString, QString, Overlay*);
private:
 Configuration* config;
 int currCityId;
 QMap<QString, Overlay*>* overlayMap; // list of available overlays.

 private slots:

};

#endif // OVERLAYTABLEMODEL_H
