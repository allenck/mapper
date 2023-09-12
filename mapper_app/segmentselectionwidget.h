#ifndef SEGMENTSELECTIONWIDGET_H
#define SEGMENTSELECTIONWIDGET_H

#include <QWidget>
#include "data.h"
#include "sql.h"
#include <QButtonGroup>

namespace Ui {
 class SegmentSelectionWidget;
}

class SegmentSelectionWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit SegmentSelectionWidget(QWidget *parent = nullptr);
  ~SegmentSelectionWidget();
  SQL* sql;
  QComboBox* cbSegments();
  SegmentInfo segmentSelected();
  void refresh();
  void setCurrentSegment(int segmentId);
  void initialize();
  QStringList locations() {return _locations;}

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
  void segmentSelected(SegmentData sd);

 private:
  Ui::SegmentSelectionWidget *ui;
  bool bRefreshingSegments = false;
  bool bCbStreetsRefreshing = false;
  //QList<SegmentInfo> cbSegmentInfoList;  // list of segmentInfo items in cbSegments
  QMap<int, SegmentInfo> cbSegmentDataMap;
  qint32 m_SegmentId;
  bool bCbStreets_text_changed=false;
  QString saveStreet;
  QButtonGroup* cbSegmentsGrp;
  QButtonGroup* enterGrp;
  SegmentInfo currSd;
  bool initialized = false;
  QMutex mutex;
  QMutex mutex2;
  QMap<QString, int> mapDescriptions;
  bool b_cbSegments_TextChanged = false;
  QStringList _locations;
};

#endif // SEGMENTSELECTIONWIDGET_H
