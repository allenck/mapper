BEGIN TRANSACTION;
PRAGMA foreign_keys = 0;
CREATE TEMPORARY TABLE `t1_backup` (SegmentId, Description, OneWay, Tracks, `Type`, StartLat, StartLon, EndLat, EndLon,
                       Length, Points, StartDate, endDate, Direction, lastUpdate, pointArray, street, location);
insert into t1_backup SELECT SegmentId, Description, OneWay, Tracks, `Type`, StartLat, StartLon, EndLat, EndLon,
                       Length, Points, StartDate, endDate, Direction, lastUpdate, pointArray, street, location from `Segments`;
DROP TABLE `Segments`;
CREATE TABLE `Segments` ( `SegmentId` integer  primary key AUTOINCREMENT NOT NULL,
                          `Description` varchar(100) NOT NULL,
                          `Tracks` int(11) NOT NULL DEFAULT 0,
                          `Street` text not null default '',
                          `Location` text not null default '',
                          `Type` int(11) NOT NULL DEFAULT 0,
                          `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `Length` decimal(15,5) NOT NULL DEFAULT 0,
                          `StartDate` date NOT NULL DEFAULT '0000-00-00',
                          `EndDate` date NOT NULL DEFAULT '0000-00-00',
                          `Direction` varchar(6) NOT NULL DEFAULT ' ',
                          `OneWay` char(1) NOT NULL DEFAULT 'N',
                          `Points` int(11) NOT NULL default 0,
                          `PointArray` text,
                          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
INSERT INTO `Segments` (SegmentId, Description, OneWay, Tracks,Street, Location, `Type`, StartLat, StartLon,EndLat, EndLon,
                        Length, Points, StartDate, endDate, Direction, lastUpdate, pointArray)
                        select SegmentId, Description, OneWay, Tracks,Street, Location, `Type`, StartLat, StartLon,EndLat, EndLon,
                        Length, points, StartDate, endDate, Direction, lastUpdate, pointArray FROM `t1_backup`;
drop table t1_backup;
PRAGMA foreign_keys = 1;
COMMIT;

