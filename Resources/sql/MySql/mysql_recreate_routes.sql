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
   `Route` int NOT NULL,
   `Name` varchar(140) CHARACTER SET latin1 COLLATE latin1_german1_ci NOT NULL,
   `StartDate` date NOT NULL,
   `EndDate` date NOT NULL,
   `LineKey` int NOT NULL,
   `OneWay` char(1) DEFAULT ''Y'',
   `TrackUsage` char(1) DEFAULT '''',
   `CompanyKey` int NOT NULL,
   `tractionType` int NOT NULL,
   `Direction` varchar(6) NOT NULL, 
   `next` int NOT NULL,
   `prev` int NOT NULL,
   `normalEnter` int NOT NULL,
   `normalLeave` int NOT NULL,
   `reverseEnter` int NOT NULL,
   `reverseLeave` int NOT NULL,
   `nextR` int NOT NULL,
   `prevR` int NOT NULL,
   `Sequence` int NOT NULL,
   `ReverseSeq` int NOT NULL,
   `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
   PRIMARY KEY (`Route`,`Name`,`CompanyKey`,`StartDate`,`EndDate`,`LineKey`),
   KEY `LineKey` (`LineKey`),
   KEY `companyKey` (`CompanyKey`),
   KEY `tractionType` (`tractionType`),
   CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),
   CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
   CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
   CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `AltRoute` (`route`)
 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;
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

