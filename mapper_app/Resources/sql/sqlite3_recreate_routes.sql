BEGIN;
CREATE TEMPORARY TABLE if not exists `t_routes` (`Route` ,  `Name` ,  `StartDate`, `EndDate` ,
             `LineKey` ,  `OneWay`,  `TrackUsage` ,  `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`);
INSERT INTO  `t_routes` (`Route` ,  `Name` ,  `StartDate`, `EndDate` ,   `LineKey` ,  `OneWay`,
             `TrackUsage` ,  `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`)
      SELECT `Route` ,  `Name` ,  `StartDate`, `EndDate` ,   `LineKey` ,  `OneWay`,  `TrackUsage` ,
             `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`
             from `Routes`;
DROP TABLE `Routes`;
CREATE TABLE `Routes` (
             `Route` int(11) NOT NULL,
             `Name` varchar(125) NOT NULL,
             `StartDate` date NOT NULL,
             `EndDate` date NOT NULL,
             `LineKey` int(11) NOT NULL,
             `OneWay` char(1) check(`oneWay` in ('Y','N',' ')) default ' ' NOT NULL,
             `TrackUsage` text check(`TrackUsage` in ('B', 'L', 'R', ' ')) default ' ' NOT NULL,
             `CompanyKey` int(11) NOT NULL DEFAULT 0,
             `TractionType` int(11) NOT NULL DEFAULT 0,
             `Direction` varchar(6) NOT NULL DEFAULT ' ',
             `Next` int(11) NOT NULL DEFAULT -1,
             `Prev` int(11) NOT NULL DEFAULT -1,
             `NormalEnter` int(11) NOT NULL DEFAULT 0,
             `NormalLeave` int(11) NOT NULL DEFAULT 0,
             `ReverseEnter` int(11) NOT NULL DEFAULT 0,
             `ReverseLeave` int(11) NOT NULL DEFAULT 0,
             `NextR` int(11) NOT NULL DEFAULT -1,
             `PrevR` int(11) NOT NULL DEFAULT -1,
             `Sequence` int(11) NOT NULL DEFAULT -1,
             `ReverseSeq` int(11) NOT NULL DEFAULT -1,
             `LastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
             constraint pk PRIMARY key (`Route`,`Name`,`CompanyKey`,`StartDate`,`EndDate`,`LineKey`),
             CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`) ON DELETE RESTRICT,
             CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
             CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
             CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `AltRoute` (`route`));
   INSERT INTO `Routes` (`Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`,
               `CompanyKey`, `tractionType`, `Direction`, `next`, `prev`, `NextR`,   `PrevR`,
               `normalEnter`,  `normalLeave`,
               `reverseEnter`,  `reverseLeave`, `Sequence`, `ReverseSeq`, `lastUpdate`)
           SELECT `Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`, `CompanyKey`,
                `tractionType`, `Direction`, `next`, `prev`, `NextR`,   `PrevR`,
                `normalEnter`,  `normalLeave`,  `reverseEnter`,
                `reverseLeave`, `Sequence`, `ReverseSeq`, `lastUpdate`s
             from `t_routes`;
DROP TABLE `t_routes`;
COMMIT;

