#ifndef SPLITSEGMENTDLG_H
#define SPLITSEGMENTDLG_H

#include <QWidget>
#include "data.h"
#include "sql.h"

namespace Ui {
 class SplitSegmentDlg;
}

class SplitSegmentDlg : public QDialog
{
  Q_OBJECT

 public:
  explicit SplitSegmentDlg(int segmentId, QWidget *parent = nullptr);
  ~SplitSegmentDlg();
  void setSegment(int segmentId);

 private:
  Ui::SplitSegmentDlg *ui;
  QMap<int, SegmentData> cbSegmentDataList;
  SegmentData sd;
  void setupDates(SegmentData si);
  bool processChanges();
  void refreshSegments();
};

#endif // SPLITSEGMENTDLG_H
