#ifndef EDITSTATION_H
#define EDITSTATION_H

#include <QDialog>
#include "configuration.h"
#include "data.h"
#include "sql.h"
#include "editcomments.h"

namespace Ui {
    class editStation;
}

class EditStation : public QDialog
{
    Q_OBJECT

public:
    explicit EditStation(qint32 stationKey, bool bDisplayStationMarkers, QWidget *parent = 0);
    ~EditStation();
    qint32 SegmentId();
    void setSegmentId(qint32 value);
    qint32 StationId();
    void setStationId(qint32 value);
    QString StationName();
    void setStationName(QString value);
    LatLng Point();
    void setPoint(LatLng value);
    int Index ();
    void setIndex(qint32 value);
    //void setConfiguration (Configuration * cfg);
    qint32 infoKey ();
    bool WasStationDeleted();
    int LineSegmentId();
    QDate StartDate();
    void setStartDate(QDate value);
    QDate EndDate();
    void setEndDate(QDate value);
    qint32 Geodb_Loc_Id();
    void setGeodb_Loc_Id(qint32 value);
    void setMarkerType(QString);

private:
    Ui::editStation *ui;
    qint32 _segmentId;
    qint32 _stationKey;
    qint32 _lineSegmentId;
    QString _stationName;
    LatLng _latLng;
    qint32 _pt;
    qint32 _infoKey;
    Configuration *config;
    bool _bStationDeleted;
    qint32 _bGeodb_loc_id;
    bool bDirty;

    SQL* sql;
    //SegmentInfo si;
    SegmentInfo sd;
    void setRadioButtons();
    bool bDisplayStationMarkers;
    bool bUpdateExisting;
    QString markerType;
    void setStationId(StationInfo sti);
    void closeEvent(QCloseEvent*);

private slots:
    void txtGeodbLocId_Leave();
    void btnOK_Click();
    void btnEditText_Click();
    void btnDelete_Click();
    void txtStationName_Leave();
    void dateTimePicker2_ValueChanged();
    void txtStationName_edited(QString);
    void On_cbIcons_selectionChanged(int);
};

#endif // EDITSTATION_H
