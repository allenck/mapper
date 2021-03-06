#ifndef EDITSEGMENTDIALOG_H
#define EDITSEGMENTDIALOG_H

#include <QDialog>
#include "data.h"

namespace Ui {
class EditSegmentDialog;
}

class webViewBridge;
class mainWindow;
class SQL;
class Configuration;
class EditSegmentDialog : public QDialog
{
 Q_OBJECT

public:
 explicit EditSegmentDialog(QWidget *parent = 0);
 EditSegmentDialog(int segmentId, QWidget *parent = 0);
 ~EditSegmentDialog();

private:
 Ui::EditSegmentDialog *ui;
 Configuration* config;
 SQL* sql;
 QList<SegmentInfo> segmentlist;
 mainWindow* myParent;
 SegmentInfo* si;
 SegmentInfo* saveSi;
 void setUpdate();
 QPushButton* btnUpdate;
 bool b_cbSegments_TextChanged;
 void common();
 bool bStartDateEdited;
 bool bEndDateEdited;
 webViewBridge* m_bridge;
 QString m_segmentStatus;
 QString m_segmentColor;

private slots:
 void fillSegments();
 void On_cbSegments_currentIndexChanged(int);
 void On_cbRouteType_currentIndexChanged(int);
 void On_sbTracks_valueChanged(int);
 void On_chkOneWay_toggled(bool);
 void On_txtDescription_editingFinished();
 void On_dtBegin_dateChanged(QDate);
 void On_dtEnd_dateChanged(QDate);
 void On_btnUpdate_clicked();
 void On_cbSegmentsTextChanged(QString );
 void On_cbSegments_Leave();
 void On_dtBegin_editingFinished();
 void On_dtEnd_editingFinished();
 void On_buttonBox_accepted();
 void On_segmentStatusSignal(QString, QString);
};

#endif // EDITSEGMENTDIALOG_H
