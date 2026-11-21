#ifndef SPLITSEGMENTDLG_H
#define SPLITSEGMENTDLG_H

#include <QWidget>
#include "data.h"
#include <QDialog>

namespace Ui {
 class SplitSegmentDlg;
}

class SplitSegmentDlg : public QDialog
{
  Q_OBJECT

 public:
  explicit SplitSegmentDlg(int segmentId, QWidget *parent = nullptr);
  SplitSegmentDlg(SegmentData* sd, QWidget *parent = nullptr);
  ~SplitSegmentDlg();
  void setSegment(int segmentId);

 private:
  Ui::SplitSegmentDlg *ui;
  QMap<int, SegmentInfo> cbSegmentInfoList;
  SegmentData* sd = nullptr;
  void setupDates(SegmentData* sd);
  bool processChanges();
  //void refreshSegments();
  void common(int segmentId);
  QMap<int,CompanyData*> companyMap;
  bool company1Valid();
  bool company2Valid();
  bool canBeChanged();
  bool sdSpecified = false;
  QString _dtFormat;
};

#endif // SPLITSEGMENTDLG_H
