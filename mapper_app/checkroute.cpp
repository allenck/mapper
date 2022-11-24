#include "checkroute.h"

checkRoute::checkRoute(QList<SegmentInfo> list, Configuration *cfg, QObject *parent) : QObject(parent)
{
    segmentInfoList = segmentInfoList_old = list;
    config = cfg;
    //sql->setConfig(config);
    sql = SQL::instance();
    bError = checkConnectingSegments();
}

bool checkRoute::checkConnectingSegments()
{
    notConnectedList.clear();
    multipleConnectionsList.clear();
    bChangesToBeMade = false;
    startSegment = -1;
    endSegment = -1;

    int segmentsNotConnected = 0;
    int segmentsMultiplyConnected=0;
    for(int i=0; i < segmentInfoList.count(); i++)
    {
        SegmentInfo *si = (SegmentInfo*)&segmentInfoList.at(i);
        // check connections to start of segment
        int startConnects = 0;
        int nextSegment=-1;
        int endConnects =0;
        int prevSegment=-1;

        foreach(SegmentInfo si2, segmentInfoList)
        {
            if(si->segmentId == si2.segmentId)  // ignore self
                continue;
            if(si->oneWay == "N")
            {
                // Only check connections to start if a twoway segment
                if(si->oneWay == "N" && si2.oneWay =="N" || (si->oneWay == "N" && si2.oneWay == "Y")) // only twoway can connect start to start!
                {
                    double dist = sql->Distance(si->startLat, si->startLon, si2.startLat, si2.startLon);

                    if(dist < .020)
                    {
                        if(startConnects == 0)
                        {
                            nextSegment = si2.segmentId;
                            startConnects++;
                        }
                        else
                        if(isDirectionCorrect(si->normalEnter, reverseBearing( si->bearingStart.getBearing()), reverseBearing(si2.bearingStart.getBearing())))
                        {
                            nextSegment = si2.segmentId;
                        }
                        else
                            startConnects++;
                    }
                }
                if((si->oneWay == "N" && si2.oneWay =="N") || si2.oneWay == "Y" || (si->oneWay == "Y" && si2.oneWay =="Y") )
                {
                    double dist = sql->Distance(si->startLat, si->startLon, si2.endLat, si2.endLon);

                    if(dist < .020 )
                    {
                        if(startConnects == 0)
                        {
                            nextSegment = si2.segmentId;
                            startConnects++;
                        }
                        else
                        if(isDirectionCorrect(si->normalEnter,reverseBearing( si->bearingStart.getBearing()), reverseBearing(si2.bearingStart.getBearing()) ))
                        {
                            nextSegment = si2.segmentId;
                        }
                        else
                            startConnects++;
                    }
                }
            }
            // Now check connections to end
            if((si->oneWay == "N" && si2.oneWay =="N") || si2.oneWay == "Y" || (si->oneWay == "N" && si2.oneWay =="Y"))
            {
                double dist = sql->Distance(si->endLat, si->endLon, si2.startLat, si2.startLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                    {
                        prevSegment = si2.segmentId;
                        endConnects++;
                    }
                    else
                    if(isDirectionCorrect(si->normalLeave, si->bearing.getBearing(), reverseBearing(si2.bearingStart.getBearing())))
                    {
                        prevSegment = si2.segmentId;
                    }
                    else
                        endConnects++;
                }
            }
            if(si->oneWay == "N" && si2.oneWay =="N") // only twoway can connect end to end
            {
                double dist = sql->Distance(si->endLat, si->endLon, si2.endLat, si2.endLon);

                if(dist < .020)
                {
                    if(endConnects == 0)
                    {
                        prevSegment = si2.segmentId;
                        endConnects++;
                    }
                    else
                    if(isDirectionCorrect(si->normalLeave, si->bearing.getBearing(), reverseBearing(si2.bearingEnd.getBearing())))
                    {
                        prevSegment = si2.segmentId;
                    }
                    else
                        endConnects++;

                }
            }
        }
        if(startConnects > 0)
        {
            if(si->next != nextSegment)
            {
                si->next = nextSegment;
                bChangesToBeMade = true;
            }
        }
        if(endConnects > 0)
        {
            if(si->prev != prevSegment)
            {
                si->prev = prevSegment;
                bChangesToBeMade = true;
            }
        }
        if(startConnects == 0 && endConnects == 0)
        {
            segmentsNotConnected++;
//            qDebug()<< "segment "+ QString("%1").arg(si->segmentId)+ " not connected to another!";
            notConnectedList.append(*si);
        }
        if(startConnects > 1 || endConnects > 1)
        {
            segmentsMultiplyConnected ++;
            qDebug()<< "segment "+ QString("%1").arg(si->segmentId)+ " connects to multiple segments!";
            multipleConnectionsList.append(*si);
        }
    }
    if(segmentsNotConnected > 1 || segmentsMultiplyConnected > 0)
        return false;
    return true;
}
bool checkRoute::isError()
{
    return bError;
}
QList<SegmentInfo> checkRoute::getnotConnected()
{
    return notConnectedList;
}
QList<SegmentInfo> checkRoute::getMultipleConnections()
{
    return multipleConnectionsList;
}
SegmentInfo* checkRoute::findSegment(qint32 segId)
{
 SegmentInfo* si = NULL;
 for(int i = 0; i < segmentInfoList.count(); i++)
 {
  si = (SegmentInfo *)&segmentInfoList.at(i);
  if(si->segmentId == segId)
   return si;
 }
 return si;
}
void checkRoute::setStart(qint32 seg)
{
    startSegment = seg;
}
void checkRoute::setEnd(qint32 seg)
{
    endSegment = seg;
}
qint32 checkRoute::getStart()
{
    return startSegment;
}
qint32 checkRoute::getEnd()
{
    return endSegment;
}
bool checkRoute::setSeqNbrs()
{
    if(startSegment < 1)
        return false;
    if(endSegment < 1)
        return false;

    qint32 nextSegment = startSegment;
    qint32 lastSegment = -1;
    qint32 nextSequence = -1;
    bool bWorking = true;

    while( bWorking)
    {
     SegmentInfo* si = findSegment(nextSegment);
     if(si == NULL)
      return false;
     if(si->next == lastSegment && si->oneWay == "N")
     {
         // swap next & prev
         qint32 temp = si->next;
         si->next = si->prev;
         si->prev = temp;
     }
     if(si->next < 1 && si->oneWay == "Y")
     {
         // swap next & prev
         qint32 temp = si->next;
         si->next = si->prev;
         si->prev = temp;
     }
     nextSegment = si->next;
     si->sequence = ++nextSequence;
     if(si->segmentId == endSegment || nextSequence > segmentInfoList.count())
     {
         bWorking = false;
         break;
     }
     lastSegment = si->segmentId;
    }
    bWorking = true;
    nextSequence = -1;
    nextSegment = endSegment;
    while(bWorking)
    {
        SegmentInfo* si = findSegment(nextSegment);
//        if(si->next == lastSegment)
//        {
//            qint32 temp = si->next;
//            si->prev = si->next;
//            si->next = temp;
//        }
        nextSegment = si->prev;
        si->returnSeq = ++nextSequence;

        if(si->segmentId == startSegment || nextSequence > segmentInfoList.count())
        {
            nextSequence = 0;
            bWorking = false;
            break;
        }
        lastSegment = si->segmentId;
    }
    return true;
}
double checkRoute::reverseBearing(double b)
{
    if(b > 180)
        return b-180.0;
    else
        return b + 180.0;
}
// b1 is the bearing of the source turning to bearing b2
bool checkRoute::isDirectionCorrect(qint32 turn, double b1, double b2)
{
    double diff = b2 - b1;
    if(diff < 0)
        diff = -diff;
    switch(turn)
    {
    case 0: // ahead
        if(diff < 45.0 || diff > 315.0)
            return true;
        break;
    case 1: // to left
        if(diff > 225.0 && diff < 315.0)
            return true;
        break;
    case 2: // to right
        if(diff > 45 && diff < 135.0)
        break;
    }
    return false;
}
