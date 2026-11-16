#ifndef SEGMENTSELECTIONWIDGET_H
#define SEGMENTSELECTIONWIDGET_H

#include "segmentinfo.h"
#include <QWidget>
#include <QButtonGroup>
#include <qcombobox.h>
#include <qmutex.h>

namespace Ui {
 class SegmentSelectionWidget;
}

class SegmentSelectionWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit SegmentSelectionWidget(QWidget *parent = nullptr);
  ~SegmentSelectionWidget();
  //SQL* sql;
  QComboBox* cbSegments();
  QComboBox* cbStreets();
  QComboBox* cbLocation();
  SegmentInfo segmentSelected();
  void refresh();
  void setCurrentSegment(int segmentId);
  void initialize(QMap<int, SegmentInfo> cbSegmentInfoMap);
  QStringList locations() {return _locations;}
  void setCurrentStreet(QString st);
  void segmentChanged(const SegmentInfo si, SegmentInfo::CHANGETYPE t);
  void segmentNameChange(const SegmentInfo oldName, const SegmentInfo newName);

 public slots:
  void refreshLocations();
  void refreshSegmentCB();
  void refreshStreetsCb();
  void cbStreets_editingFinished();
  void cbStreets_currentIndexChanged(int);
  void segmentSelected(int pt, int segmentId);
  void cbSegments_currentIndexChanged(int);
  void cbSegmentsTextChanged(QString );
  void cbSegments_editingFinished();

 signals:
  void segmentSelected(SegmentInfo si);

 private:
  Ui::SegmentSelectionWidget *ui;
  bool bRefreshingSegments = false;
  bool bCbStreetsRefreshing = false;
  //QList<SegmentInfo> cbSegmentInfoList;  // list of segmentInfo items in cbSegments
  QMap<int, SegmentInfo> cbSegmentInfoMap;
  QMultiMap<QString, SegmentInfo> _locationMap;
  QMap<QString,SegmentInfo> _segmentsByStreet;
  qint32 m_SegmentId;
  bool bCbStreets_text_changed=false;
  QString saveStreet;
  QButtonGroup* cbSegmentsGrp;
  QButtonGroup* enterGrp;
  //SegmentData currSd;
  SegmentInfo currSi;
  bool initialized = false;
  QMutex mutex;
  QMutex mutex2;
  QMap<QString, int> mapDescriptions;
  bool b_cbSegments_TextChanged = false;
  QStringList _locations;
};

#endif // SEGMENTSELECTIONWIDGET_H
