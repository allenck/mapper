BEGIN TRANSACTION;
CREATE TEMPORARY TABLE `t1_backup` (stationKey, route, name, suffix, latitude, longitude, startDate, endDate, segmentId, point, infoKey, geodb_loc_id, routeType, lastUpdate);
insert into t1_backup SELECT stationKey, route, name, suffix, latitude, longitude, startDate, endDate, segmentId, point, infoKey, geodb_loc_id, routeType, lastUpdate from `Stations`;
DROP TABLE `Stations`;
CREATE TABLE `Stations` (
  `stationKey` integer NOT NULL primary key AUTOINCREMENT,
  `route` int(11) NOT NULL DEFAULT 0,
  `name` varchar(75) NOT NULL,
  `suffix` varchar(4) NOT NULL DEFAULT '',
  `latitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `longitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `startDate` date DEFAULT NULL,
  `endDate` date DEFAULT NULL,
  `segmentId` int(11) NOT NULL,
  `point` int(11) NOT NULL DEFAULT 0,
  `infoKey` int(11) DEFAULT NOT NULL DEFAULT -1,
  `geodb_loc_id` varchar(15) DEFAULT NULL,
  `routeType` int(11) NOT NULL DEFAULT -1,
  `markerType` varchar(15) default '',
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint main unique (`segmentId`,`name`,`startDate`,`endDate`),
  constraint `stationKey` UNIQUE (`stationKey`),
  CONSTRAINT `SegmentId_ibfk_1` FOREIGN KEY (`segmentId`) REFERENCES `Segments` (`SegmentId`)
);
INSERT INTO `Stations` (stationKey, route, segmentId, point, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, lastUpdate ) SELECT stationKey, route, segmentId, point, name, suffix, latitude, longitude, startDate, endDate, infoKey, geodb_loc_id, routeType, lastUpdate FROM `t1_backup`;
drop table t1_backup;
COMMIT;

