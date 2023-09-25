#ifndef CHECKROUTE_H
#define CHECKROUTE_H

#include <QObject>
#include "configuration.h"
#include "sql.h"
#include "data.h"
#include <QList>

class CheckRoute : public QObject
{
    Q_OBJECT
public:
    explicit CheckRoute(QList<SegmentData> list, Configuration * cfg, QObject *parent = 0);
    QList<SegmentData> getnotConnected();
    QList<SegmentData> getMultipleConnections();
    void setStart(qint32 seg);
    void setEnd(qint32 seg);
    bool setSeqNbrs();

    
signals:
    
public slots:
    
private:
//    QList<SegmentInfo> segmentInfoList;
//    QList<SegmentInfo> segmentInfoList_old;
    QList<SegmentData> segmentDataList;
    QList<SegmentData> segmentDataList_old;
    Configuration * config;
    SQL* sql;
    QList<SegmentData> notConnectedList;
    QList<SegmentData> multipleConnectionsList;
    bool checkConnectingSegments();
    bool bError;
    bool bChangesToBeMade;
    bool isError();
    SegmentData *findSegment(qint32 segId);
    qint32 startSegment, endSegment;
    qint32 getStart();
    qint32 getEnd();
    double reverseBearing(double b);
    bool isDirectionCorrect(qint32 turn, double b1, double b2);

};

#endif // CHECKROUTE_H
