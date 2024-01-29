BEGIN TRANSACTION;
drop table #t1_backup;
SELECT stationKey, route, name, suffix, latitude, longitude, startDate, endDate, segmentId,
  point, infoKey, geodb_loc_id, routeType, lastUpdate into from #t1_backup `Stations`;
DROP TABLE `Stations`;
#include mssql_create_stations.sql
INSERT INTO `Stations` (stationKey, route, segmentId, point, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, lastUpdate ) SELECT stationKey, route, segmentId, point, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, lastUpdate FROM `t1_backup`;
#drop table #t1_backup;
COMMIT TRANSACTION;
