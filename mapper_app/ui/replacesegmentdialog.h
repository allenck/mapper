#ifndef REPLACESEGMENTDIALOG_H
#define REPLACESEGMENTDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include "data.h"
#include "sql.h"

namespace Ui {
 class ReplaceSegmentDialog;
}

class ReplaceSegmentDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit ReplaceSegmentDialog(QWidget *parent = nullptr);
  ~ReplaceSegmentDialog();
 public slots:
  void Process(QAbstractButton *button);
  void segmentSelected(SegmentData sd);

 private:
  Ui::ReplaceSegmentDialog *ui;
  QButtonGroup* cbSegmentsGrp;
  QButtonGroup* enterGrp;
  //QList<SegmentInfo> cbSegmentInfoList;
  bool bRefreshingSegments;
  SQL* sql;
};

#endif // REPLACESEGMENTDIALOG_H
