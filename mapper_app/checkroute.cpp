#include "checkroute.h"

CheckRoute::CheckRoute(QList<SegmentData> list, Configuration *cfg, QObject *parent) : QObject(parent)
{
    //segmentInfoList = segmentInfoList_old = list;
    segmentDataList = segmentDataList_old = list;
    config = cfg;
    //sql->setConfig(config);
    sql = SQL::instance();
    bError = checkConnectingSegments();
}

bool CheckRoute::checkConnectingSegments()
{
    notConnectedList.clear();
    multipleConnectionsList.clear();
    bChangesToBeMade = false;
    startSegment = -1;
    endSegment = -1;

    int segmentsNotConnected = 0;
    int segmentsMultiplyConnected=0;
    for(int i=0; i < segmentDataList.count(); i++)
    {
        //SegmentInfo *si = (SegmentInfo*)&segmentInfoList.at(i);
     SegmentData* sd = (SegmentData*)&segmentDataList.at(i);
        // check connections to start of segment
        int startConnects = 0;
        int nextSegment=-1;
        int endConnects =0;
        int prevSegment=-1;

        foreach(SegmentData sd2, segmentDataList)
        {
            if(sd->segmentId() == sd2.segmentId())  // ignore self
                continue;
            if(sd->oneWay() == "N")
            {
                // Only check connections to start if a twoway segment
                if(sd->oneWay() == "N" && sd2.oneWay() =="N" || (sd->oneWay() == "N" && sd2.oneWay() == "Y")) // only twoway can connect start to start!
                {
                    double dist = sql->Distance(sd->startLat(), sd->startLon(), sd2.startLat(), sd2.startLon());

                    if(dist < .020)
                    {
                        if(startConnects == 0)
                        {
                            nextSegment = sd2.segmentId();
                            startConnects++;
                        }
                        else
                        if(isDirectionCorrect(sd->normalEnter(), reverseBearing( sd->bearingStart().angle()), reverseBearing(sd2.bearingStart().angle())))
                        {
                            nextSegment = sd2.segmentId();
                        }
                        else
                            startConnects++;
                    }
                }
                if((sd->oneWay() == "N" && sd2.oneWay() =="N") || sd2.oneWay() == "Y" || (sd->oneWay() == "Y" && sd2.oneWay() =="Y") )
                {
                    double dist = sql->Distance(sd->startLat(), sd->startLon(), sd2.endLat(), sd2.endLon());

                    if(dist < .020 )
                    {
                        if(startConnects == 0)
                        {
                            nextSegment = sd2.segmentId();
                            startConnects++;
                        }
                        else
                        if(isDirectionCorrect(sd->normalEnter(),reverseBearing( sd->bearingStart().angle()), reverseBearing(sd2.bearingStart().angle()) ))
                        {
                            nextSegment = sd2.segmentId();
                        }
                        else
                            startConnects++;
                    }
                }
            }
            // Now check connections to end
            if((sd->oneWay() == "N" && sd2.oneWay() =="N") || sd2.oneWay() == "Y" || (sd->oneWay() == "N" && sd2.oneWay() =="Y"))
            {
                double dist = sql->Distance(sd->endLat(), sd->endLon(), sd2.startLat(), sd2.startLon());

                if(dist < .020)
                {
                    if(endConnects == 0)
                    {
                        prevSegment = sd2.segmentId();
                        endConnects++;
                    }
                    else
                    if(isDirectionCorrect(sd->normalLeave(), sd->bearing().angle(), reverseBearing(sd2.bearingStart().angle())))
                    {
                        prevSegment = sd2.segmentId();
                    }
                    else
                        endConnects++;
                }
            }
            if(sd->oneWay() == "N" && sd2.oneWay() =="N") // only twoway can connect end to end
            {
                double dist = sql->Distance(sd->endLat(), sd->endLon(), sd2.endLat(), sd2.endLon());

                if(dist < .020)
                {
                    if(endConnects == 0)
                    {
                        prevSegment = sd2.segmentId();
                        endConnects++;
                    }
                    else
                    if(isDirectionCorrect(sd->normalLeave(), sd->bearing().angle(), reverseBearing(sd2.bearingEnd().angle())))
                    {
                        prevSegment = sd2.segmentId();
                    }
                    else
                        endConnects++;

                }
            }
        }
        if(startConnects > 0)
        {
            if(sd->next() != nextSegment)
            {
                sd->setNext(nextSegment);
                bChangesToBeMade = true;
            }
        }
        if(endConnects > 0)
        {
            if(sd->prev() != prevSegment)
            {
                sd->setPrev(prevSegment);
                bChangesToBeMade = true;
            }
        }
        if(startConnects == 0 && endConnects == 0)
        {
            segmentsNotConnected++;
//            qDebug()<< "segment "+ QString("%1").arg(si->segmentId)+ " not connected to another!";
            notConnectedList.append(*sd);
        }
        if(startConnects > 1 || endConnects > 1)
        {
            segmentsMultiplyConnected ++;
            qDebug()<< "segment "+ QString("%1").arg(sd->segmentId())+ " connects to multiple segments!";
            multipleConnectionsList.append(*sd);
        }
    }
    if(segmentsNotConnected > 1 || segmentsMultiplyConnected > 0)
        return false;
    return true;
}

bool CheckRoute::isError()
{
    return bError;
}

QList<SegmentData> CheckRoute::getnotConnected()
{
    return notConnectedList;
}
QList<SegmentData> CheckRoute::getMultipleConnections()
{
    return multipleConnectionsList;
}

SegmentData* CheckRoute::findSegment(qint32 segId)
{
 SegmentData* sd = NULL;
 for(int i = 0; i < segmentDataList.count(); i++)
 {
  sd = (SegmentData *)&segmentDataList.at(i);
  if(sd->segmentId() == segId)
   return sd;
 }
 return sd;
}

void CheckRoute::setStart(qint32 seg)
{
    startSegment = seg;
}

void CheckRoute::setEnd(qint32 seg)
{
    endSegment = seg;
}

qint32 CheckRoute::getStart()
{
    return startSegment;
}

qint32 CheckRoute::getEnd()
{
    return endSegment;
}

bool CheckRoute::setSeqNbrs()
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
     SegmentData* sd = findSegment(nextSegment);
     if(sd == NULL)
      return false;
     if(sd->next() == lastSegment && sd->oneWay() == "N")
     {
         // swap next & prev
         qint32 temp = sd->next();
         sd->setNext(sd->prev());
         sd->setPrev(temp);
     }
     if(sd->next() < 1 && sd->oneWay() == "Y")
     {
         // swap next & prev
         qint32 temp = sd->next();
         sd->setNext(sd->prev());
         sd->setPrev(temp);
     }
     nextSegment = sd->next();
     sd->setSequence(++nextSequence);
     if(sd->segmentId() == endSegment || nextSequence > segmentDataList.count())
     {
         bWorking = false;
         break;
     }
     lastSegment = sd->segmentId();
    }
    bWorking = true;
    nextSequence = -1;
    nextSegment = endSegment;
    while(bWorking)
    {
        SegmentData* sd = findSegment(nextSegment);
//        if(si->next == lastSegment)
//        {
//            qint32 temp = si->next;
//            si->prev = si->next;
//            si->next = temp;
//        }
        nextSegment = sd->prev();
        sd->setReturnSeq(++nextSequence);

        if(sd->segmentId() == startSegment || nextSequence > segmentDataList.count())
        {
            nextSequence = 0;
            bWorking = false;
            break;
        }
        lastSegment = sd->segmentId();
    }
    return true;
}

double CheckRoute::reverseBearing(double b)
{
    if(b > 180)
        return b-180.0;
    else
        return b + 180.0;
}
// b1 is the bearing of the source turning to bearing b2
bool CheckRoute::isDirectionCorrect(qint32 turn, double b1, double b2)
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
