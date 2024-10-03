
create temporary table t_temp as select linekey, min(r.startDate) startDate, max(r.endDate)enddate , description  from routes  r join Segments s on s.segmentid = r.linekey   where s.tracks = 2 group by linekey;

update Segments 
	set startdate = t_temp.startdate,
                              doubledate = t_temp.startdate,
	       enddate = t_temp.enddate
from t_temp where Segments.segmentid = t_temp.linekey;


select * from t_temp;
