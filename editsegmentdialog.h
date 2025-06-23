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
 EditSegmentDialog(SegmentData* sd, QWidget *parent = 0);
 EditSegmentDialog(SegmentInfo si, QWidget *parent=0);
 ~EditSegmentDialog();

private:
 Ui::EditSegmentDialog *ui;
 Configuration* config;
 SQL* sql;
 //QList<SegmentInfo> segmentlist;
 MainWindow* myParent;
 SegmentInfo si;
 void setUpdate();
 //QPushButton* btnUpdate;
 //QPushButton* btnVerifyDates;
 bool b_cbSegments_TextChanged;
 void common();
 bool bStartDateEdited;
 bool bEndDateEdited;
 bool bDoubleTrackedDateEdited;
 bool bReplaceDups = false;
 WebViewBridge* m_bridge;
 QString m_segmentStatus;
 QString m_segmentColor;
 RouteData* rd = nullptr;
 QStringList _locations;
 //void processAdd();
 QList<SegmentInfo> dupSegments;
 QDate oldestStartDate;
 QDate latestEndDate;
 QDate oldestDoubleTrackDate;
 QList<SegmentInfo>reversed;
 //bool bSegmentDisplayed = false;
 QString oneWay = " ";
 SegmentData* _sd = nullptr;
 QString direction = " ";

private slots:
 //void fillSegments();
 void segmentSelected(SegmentInfo si);
 void On_cbRouteType_currentIndexChanged(int);
 void On_sbTracks_valueChanged(int);
 //void On_chkOneWay_toggled(bool);
 void On_txtDescription_editingFinished();
 void On_dtBegin_dateChanged(QDate);
 void On_dtEnd_dateChanged(QDate);
 void On_btnSave_clicked();
// void On_cbSegmentsTextChanged(QString );
// void On_cbSegments_Leave();
 void On_dtBegin_editingFinished();
 void On_dtEnd_editingFinished();
 void On_segmentStatusSignal(QString, QString);
 //void On_trackUsageChanged(int);
 void On_doubleTrackedDate_editingFinished();
 void On_doubleTracked_dateChanged(QDate dt);

};

#endif // EDITSEGMENTDIALOG_H
