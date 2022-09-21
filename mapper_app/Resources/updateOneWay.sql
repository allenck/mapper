UPDATE Routes set OneWay =  (select oneway from segments where routes.linekey = segments.segmentid)
          where linekey in (select segmentid from segments );
