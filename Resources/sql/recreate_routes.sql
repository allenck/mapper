BEGIN TRANSACTION;
ALTER TABLE `Routes` RENAME TO `old_Routes`;
CREATE TABLE `newRoutes` (
             `Route` int(11) NOT NULL,
             `Name` varchar(125) NOT NULL,
             `StartDate` date NOT NULL,
             `EndDate` date NOT NULL,
             `LineKey` int(11) NOT NULL,
             `OneWay` char(1) DEFAULT 'N',
             `TrackUsage` text check(`TrackUsage` in ('B', 'L', 'R', ' ')) default ' ' NOT NULL,
             `CompanyKey` int(11) NOT NULL DEFAULT 0,
             `tractionType` int(11) NOT NULL DEFAULT 0,
             `Direction` varchar(6) NOT NULL DEFAULT ' ',
             `next` int(11) NOT NULL DEFAULT -1,
             `prev` int(11) NOT NULL DEFAULT -1,
             `normalEnter` int(11) NOT NULL DEFAULT 0,
             `normalLeave` int(11) NOT NULL DEFAULT 0,
             `reverseEnter` int(11) NOT NULL DEFAULT 0,
             `reverseLeave` int(11) NOT NULL DEFAULT 0,
             `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
             constraint pk PRIMARY key (`Route`,`Name`,`StartDate`,`EndDate`,`LineKey`),
             CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`) ON DELETE RESTRICT,
             CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
             CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
             CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`));
   INSERT INTO `newRoutes` (`Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`, `CompanyKey`,
            `tractionType`, `Direction`, `next`, `prev`, `normalEnter`,  `normalLeave`,  `reverseEnter`,  `reverseLeave`, `lastUpdate`)
   SELECT `Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`, `CompanyKey`,
            `tractionType`, `Direction`, `next`, `prev`, `normalEnter`,  `normalLeave`,  `reverseEnter`,  `reverseLeave`, `lastUpdate`
            from `old_Routes`;
ALTER TABLE newRoutes RENAME TO `Routes`;
DROP TABLE old_Routes;
COMMIT TRANSACTION;
