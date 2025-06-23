#ifndef EDITSEGMENTDIALOG_H
#define EDITSEGMENTDIALOG_H

#include <QDialog>
#include "data.h"

namespace Ui {
class EditSegmentDialog;
}

class WebViewBridge;
class MainWindow;
class SQL;
class Configuration;
class EditSegmentDialog : public QDialog
{
 Q_OBJECT

public:
 explicit EditSegmentDialog(QWidget *parent = 0);
 EditSegmentDialog(SegmentInfo sd, QWidget *parent = 0);
 EditSegmentDialog(SegmentData *sd, SegmentInfo si, QWidget *parent=0);
 ~EditSegmentDialog();

private:
 Ui::EditSegmentDialog *ui;
 Configuration* config;
 SQL* sql;
 //QList<SegmentInfo> segmentlist;
 MainWindow* myParent;
 SegmentInfo si;
 SegmentData* sd = nullptr;
 SegmentData* osd = nullptr;
 void setUpdate();
 //QPushButton* btnUpdate;
 QPushButton* btnVerifyDates;
 bool b_cbSegments_TextChanged;
 void common();
 bool bStartDateEdited;
 bool bEndDateEdited;
 WebViewBridge* m_bridge;
 QString m_segmentStatus;
 QString m_segmentColor;
 RouteData* rd;
 QStringList _locations;

private slots:
 //void fillSegments();
 void segmentSelected(SegmentInfo si);
 void On_cbRouteType_currentIndexChanged(int);
 void On_sbTracks_valueChanged(int);
 void On_chkOneWay_toggled(bool);
 void On_txtDescription_editingFinished();
 void On_dtBegin_dateChanged(QDate);
 void On_dtEnd_dateChanged(QDate);
 void On_btnSave_clicked();
// void On_cbSegmentsTextChanged(QString );
// void On_cbSegments_Leave();
 void On_dtBegin_editingFinished();
 void On_dtEnd_editingFinished();
 void On_buttonBox_accepted();
 void On_segmentStatusSignal(QString, QString);
 void On_trackUsageChanged(int);
};

#endif // EDITSEGMENTDIALOG_H
