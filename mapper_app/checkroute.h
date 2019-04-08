#ifndef CHECKROUTE_H
#define CHECKROUTE_H

#include <QObject>
#include "configuration.h"
#include "sql.h"
#include "data.h"
#include <QList>

class checkRoute : public QObject
{
    Q_OBJECT
public:
    explicit checkRoute(QList<SegmentInfo> list, Configuration * cfg, QObject *parent = 0);
    QList<SegmentInfo> getnotConnected();
    QList<SegmentInfo> getMultipleConnections();
    void setStart(qint32 seg);
    void setEnd(qint32 seg);
    bool setSeqNbrs();

    
signals:
    
public slots:
    
private:
    QList<SegmentInfo> segmentInfoList;
    QList<SegmentInfo> segmentInfoList_old;
    Configuration * config;
    SQL* sql;
    QList<SegmentInfo> notConnectedList;
    QList<SegmentInfo> multipleConnectionsList;
    bool checkConnectingSegments();
    bool bError;
    bool bChangesToBeMade;
    bool isError();
    SegmentInfo* findSegment(qint32 segId);
    qint32 startSegment, endSegment;
    qint32 getStart();
    qint32 getEnd();
    double reverseBearing(double b);
    bool isDirectionCorrect(qint32 turn, double b1, double b2);

};

#endif // CHECKROUTE_H
