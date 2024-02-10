BEGIN;
CREATE TEMPORARY TABLE `t1_backup`  as  SELECT * from `Stations`;
DROP TABLE `Stations`;
CREATE TABLE `Stations` (
  `stationKey` integer NOT NULL primary key AUTOINCREMENT,
  `routes` varchar(50) NOT NULL DEFAULT '',
  `name` varchar(75) NOT NULL,
  `latitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `longitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `startDate` date DEFAULT NULL,
  `endDate` date DEFAULT NULL,
  `SegmentId` int DEFAULT -1,
  `segments` varchar(50) NOT NULL DEFAULT '',
  `infoKey` int(11) NOT NULL DEFAuLT -1,
  `markerType` varchar(15) default '',
  `routeType` int(11) NOT NULL DEFAULT -1,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint main unique (`name`,`startDate`,`endDate`),
  constraint `stationKey` UNIQUE (`stationKey`)
);
INSERT INTO `Stations` (stationKey, routes, name, latitude, longitude, startDate, endDate,
                        segmentId, segments, infoKey, markerType, routeType, lastUpdate )
                        SELECT stationKey, '', name, latitude, longitude,
                          startDate, endDate, segmentid, format('%d',segmentid), infoKey,
                          MarkerType, routeType, lastUpdate FROM `t1_backup`;
drop table t1_backup;
COMMIT ;


