BEGIN TRANSACTION;
CREATE TEMPORARY TABLE `t1_backup` (SegmentId, Description, OneWay, Tracks, `Type`, StartLat, StartLon, EndLat, EndLon, Length, points, StartDate, endDate, Direction, lastUpdate, pointArray, street, locality);
insert into t1_backup SELECT SegmentId, Description, OneWay, Tracks, `Type`, StartLat, StartLon, EndLat, EndLon, Length, points, StartDate, endDate, Direction, lastUpdate, pointArray, street, locality from `Segments`;
DROP TABLE `Segments`;
CREATE TABLE `Segments` ( `SegmentId` integer  primary key AUTOINCREMENT NOT NULL, `Description` varchar(100) NOT NULL, `OneWay` char(1) NOT NULL DEFAULT 'N', `Tracks` int(11) NOT NULL DEFAULT 0,  `street` text not null default '', `location` text not null default '', `Type` int(11) NOT NULL DEFAULT 0, `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0, `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0, `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0, `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0, `Length` decimal(15,5) NOT NULL DEFAULT 0, `points` int(11) NOT NULL default 0, `StartDate` date NOT NULL DEFAULT '0000-00-00',`EndDate` date NOT NULL DEFAULT '0000-00-00',`Direction` varchar(6) NOT NULL DEFAULT ' ',`pointArray` text,`lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
INSERT INTO `Segments` (SegmentId, Description, OneWay, Tracks,street, location, `Type`, StartLat, StartLon,EndLat, EndLon, Length, points, StartDate, endDate, Direction, lastUpdate, pointArray) select SegmentId, Description, OneWay, Tracks,street, locality, `Type`, StartLat, StartLon,EndLat, EndLon, Length, points, StartDate, endDate, Direction, lastUpdate, pointArray FROM `t1_backup`;
drop table t1_backup;
COMMIT;

