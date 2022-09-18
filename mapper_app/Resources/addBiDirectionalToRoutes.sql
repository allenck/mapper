BEGIN TRANSACTION;
CREATE TEMPORARY if not exists `t_routes` (`Route`, 'Name', `StartDate`, `EndDate`, `LineKey`,
   BiDirectional, `CompanyKey, `tractionType`, `Direction`, `next`, `prev`, `normalEnter, `normalLeave,
   `reverseEnter`, `reverseLeave`, `lastUpdate`);
INSERT INTO `t_routes` (`Route`, 'Name', `StartDate`, `EndDate`, `LineKey`, BiDirectional,
   `CompanyKey, `tractionType`, `Direction`, `next`, `prev`, `normalEnter, `normalLeave, `reverseEnter`,
   `reverseLeave`, `lastUpdate`)
   SELECT (`Route`, 'Name', `StartDate`, `EndDate`, `LineKey`, BiDirectional, `CompanyKey,
   `tractionType`, `Direction`, `next`, `prev`, `normalEnter, `normalLeave1, `reverseEnter`,
   `reverseLeave`, `lastUpdate`) from `Routes`;
DROP TABLE `Routes';
CREATE TABLE `Routes` (
            `Route` int(11) NOT NULL,
            `Name` varchar(125) NOT NULL,
            `StartDate` date NOT NULL,
            `EndDate` date NOT NULL,
            `LineKey` int(11) NOT NULL,
            `BiDirectional` char(1) DEFAULT 'Y',
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
            CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),
            CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
            CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
            CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`));
INSERT INTO `Routes` (`Route`, 'Name', `StartDate`, `EndDate`, `LineKey`, BiDirectional, `CompanyKey,
   `tractionType`, `Direction`, `next`, `prev`, `normalEnter, `normalLeave, `reverseEnter`,
   `reverseLeave`, `lastUpdate`)
   SELECT (`Route`, 'Name', `StartDate`, `EndDate`, `LineKey`, BiDirectional, `CompanyKey,
   `tractionType`, `Direction`, `next`, `prev`, `normalEnter, `normalLeave, `reverseEnter`,
   `reverseLeave`, `lastUpdate`) from `t_Routes`;
UPDATE Routes a set Bidirectional = 'N' join segments s on s.SegmentId = a.LineKey
   where s.tracks=2 and OneWay = 'N';
DROP TABLE `t_routes`;
;COMMIT TRANSACTION;
