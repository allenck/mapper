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
  SegmentData segmentSelected();
  void refresh();
  void setCurrentSegment(int segmentId);
  void initialize();

 public slots:
  void refreshSegmentCB();
  void refreshStreetsCb();
  void cbStreets_editingFinished();
  void cbStreets_currentIndexChanged(int);
  void segmentSelected(int pt, int segmentId);
  void cbSegments_currentIndexChanged(int);

 signals:
  void segmentSelected(SegmentData sd);

 private:
  Ui::SegmentSelectionWidget *ui;
  bool bRefreshingSegments = false;
  bool bCbStreetsRefreshing = false;
  //QList<SegmentInfo> cbSegmentInfoList;  // list of segmentInfo items in cbSegments
  QMap<int, SegmentData> cbSegmentDataList;
  qint32 m_SegmentId;
  bool bCbStreets_text_changed=false;
  QString saveStreet;
  QButtonGroup* cbSegmentsGrp;
  QButtonGroup* enterGrp;
  SegmentData currSd;
  bool initialized = false;
};

#endif // SEGMENTSELECTIONWIDGET_H
