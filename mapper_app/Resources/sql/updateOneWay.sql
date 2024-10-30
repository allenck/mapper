# fix OneWay in Routes by restoring from Segments
update Segments set oneway = " " where oneway not in(" ","Y","N");
UPDATE Routes set OneWay =  (select oneway from Segments where routes.linekey = segments.segmentid)
          where linekey in (select segmentid from Segments );

