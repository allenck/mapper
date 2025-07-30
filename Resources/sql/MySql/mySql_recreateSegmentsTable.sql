BEGIN;
CREATE TEMPORARY TABLE  `t1_backup`SELECT * FROM Segments;
ALTER TABLE `Routes` DROP FOREIGN Key `Routes_ibfk_1`;
ALTER TABLE `Stations` DROP FOREIGN Key `SegmentId_ibfk_1`;
DROP TABLE `Segments`;
CREATE TABLE `Segments` ( `SegmentId` integer  primary key AUTO_INCREMENT NOT NULL,
                          `Description` varchar(100) NOT NULL,
                          `Tracks` int(11) NOT NULL DEFAULT 0,
                          `Street` text not null default '',
                          `StreetId` int(11),
                          `Location` text not null default '',
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
ALTER TABLE `Routes` ADD CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`);
ALTER TABLE `Stations` ADD CONSTRAINT `SegmentId_ibfk_1` FOREIGN KEY (`segmentId`) REFERENCES `Segments` (`SegmentId`);

INSERT INTO `Segments` (SegmentId, Description, OneWay, Tracks,Street, Location, `Type`, StartLat, StartLon,EndLat, EndLon,
                        Length, Points, StartDate, DoubleDate, endDate, Direction, lastUpdate, pointArray, StreetId)
                        SELECT SegmentId, Description, OneWay, Tracks,Street, Location, `Type`, StartLat, StartLon,EndLat, EndLon,
                        Length, points, StartDate, doubleDate, endDate, Direction, lastUpdate, pointArray,
                        case when streetId > 0 then streetId else NULL END
                        FROM `t1_backup`;
drop table t1_backup;
COMMIT;

