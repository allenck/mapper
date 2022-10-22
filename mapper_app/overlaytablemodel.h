#ifndef OVERLAYTABLEMODEL_H
#define OVERLAYTABLEMODEL_H

#include <QAbstractTableModel>

class Configuration;
class Overlay;
class OverlayTableModel : public QAbstractTableModel
{
 Q_OBJECT
public:
 OverlayTableModel();
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
  DESCRIPTION,
  MINZOOM,
  MAXZOOM,
  OPACITY,
  LOCAL,
  SOURCE,
  URLS,
  NUMCOLUMNS
 };
 void setCity(int);

signals:
 void setDirty();
 void overlaySelectionChanged(QModelIndex index, bool checked);

private:
 Configuration* config;
 int currCityId;
 QList<Overlay*> overlayList; // list of available overlays.
};

#endif // OVERLAYTABLEMODEL_H
