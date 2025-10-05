PRAGMA foreign_keys = 0;
BEGIN TRANSACTION;
update Segments set tracks=1 where tracks not in(1,2);
CREATE TEMPORARY TABLE `t1_backup` (SegmentId, Description, OneWay, FormatOK, Tracks, `Type`, StartLat, StartLon,
                       EndLat, EndLon, Length, Points, StartDate, DoubleDate, endDate, Direction, lastUpdate,
                       pointArray, street, NewerName, location, StreetId);

insert into t1_backup SELECT SegmentId, Description, OneWay, FormatOK, Tracks, `Type`, StartLat, StartLon,
                       EndLat, EndLon, Length, Points, StartDate, DoubleDate, endDate, Direction,
                       lastUpdate, pointArray, street, NewerName, location, StreetId from `Segments`;

DROP TABLE `Segments`;
CREATE TABLE `Segments` ( `SegmentId` integer  primary key AUTOINCREMENT NOT NULL,
                          `Description` varchar(100) NOT NULL,
                          `FormatOK` int(1) NOT NULL DEFAULT FALSE,
                          `Tracks` int(11) check(`tracks` in (1,2) )NOT NULL DEFAULT 1,
                          `Street` varchar(60) not null default '',
                          `StreetId` integer NOT NULL DEFAULT -1,
                          `NewerName` varchar(60) not null default '',
                          `Location` varchar(30) not null default '',
                          `Type` int(11) NOT NULL DEFAULT 0,
                          `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0,
                          `Length` decimal(15,5) NOT NULL DEFAULT 0,
                          `StartDate` date NOT NULL DEFAULT '0000-00-00',
                          `DoubleDate` date NOT NULL DEFAULT '0000-00-00',
                          `EndDate` date NOT NULL DEFAULT '0000-00-00',
                          `Direction` varchar(6) NOT NULL DEFAULT ' ',
                          `OneWay` char(1) NOT NULL DEFAULT 'N',
                          `Points` int(11) NOT NULL default 0,
                          `PointArray` text,
                          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
                          CONSTRAINT `Segments_ibfk_1` FOREIGN KEY (`StreetId`) REFERENCES `StreetDef` (`StreetId`));
INSERT INTO `Segments` (SegmentId, Description, OneWay, FormatOK, Tracks,Street, NewerName, Location, `Type`,
                        StartLat, StartLon,EndLat, EndLon,Length, Points, StartDate, DoubleDate, endDate, Direction,
                        lastUpdate, pointArray, StreetId)
                        SELECT SegmentId, Description, OneWay, FormatOK, Tracks, Street, NewerName, Location, `Type`,
                        StartLat, StartLon,EndLat, EndLon, Length, points, StartDate, DoubleDate, endDate, Direction,
                        lastUpdate, pointArray, IIF(StreetId>0, StreetId, NULL) FROM `t1_backup`;
drop table t1_backup;
PRAGMA foreign_keys = 1;
COMMIT;

