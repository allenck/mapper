BEGIN ;
CREATE TEMPORARY TABLE t1_backup (route, routePrefix, routeAlpha, baseRoute, lastUpdate);
INSERT INTO t1_backup SELECT route, routePrefix, routeAlpha, baseRoute, lastUpdate FROM altRoute;
CREATE TEMPORARY TABLE t2_backup(route, `name`,StartDate,endDate,LineKey, CompanyKey, TractionType,Direction,
                                                                              next, prev,nextR,prevR,normalEnter,normalleave, reverseEnter, reverseLeave,
                                                                             sequence, reverseSeq, lastUpdate);
INSERT INTO t2_backup SELECT route, `name`,StartDate,endDate,LineKey, CompanyKey, TractionType,Direction, next, prev,nextR,prevR,
                                                                normalEnter,normalleave, reverseEnter, reverseLeave, sequence, reverseSeq,lastUpdate FROM Routes;
Drop Table Routes;
DROP TABLE AltRoute;
CREATE TABLE if not exists  `AltRoute` (
  `route` integer NOT NULL primary key AUTOINCREMENT,
  `routePrefix` varchar(10) default '',
  `routeAlpha` varchar(10) NOT NULL,
  `baseRoute` int(11) NOT NULL DEFAULT 0,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  CONSTRAINT `index` unique ( routePrefix, routeAlpha)
);
INSERT INTO AltRoute (route, routePrefix, routeAlpha, baseRoute, lastUpdate)
            SELECT route, routePrefix, routeAlpha, baseRoute, lastUpdate FROM t1_backup;
DROP TABLE t1_backup;
CREATE TABLE `Routes` (  `Route` int(11) NOT NULL,  `Name` varchar(125) NOT NULL,
             `StartDate` date NOT NULL,  `EndDate` date NOT NULL,  `LineKey` int(11) NOT NULL,
             `CompanyKey` int(11) NOT NULL DEFAULT 0,  `tractionType` int(11) NOT NULL DEFAULT 0,
             `Direction` varchar(6) NOT NULL DEFAULT ' ',
             `next` int(11) NOT NULL DEFAULT -1,  `prev` int(11) NOT NULL DEFAULT -1,
             `normalEnter` int(11) NOT NULL DEFAULT 0,  `normalLeave` int(11) NOT NULL DEFAULT 0,
             `reverseEnter` int(11) NOT NULL DEFAULT 0,  `reverseLeave` int(11) NOT NULL DEFAULT 0,
             `nextR` int(11) NOT NULL DEFAULT -1,  `prevR` int(11) NOT NULL DEFAULT -1,
             `sequence` int(11) NOT NULL DEFAULT -1,  `reverseSeq` int(11) NOT NULL DEFAULT -1,
             `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
             constraint pk PRIMARY key (`Route`,`Name`,`StartDate`,`EndDate`,`LineKey`),
             CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),
             CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
             CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
             CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`));
INSERT INTO Routes (route, `name`,StartDate,endDate,LineKey, CompanyKey, TractionType,Direction,
             next, prev,nextR,prevR,normalEnter,normalleave, reverseEnter, reverseLeave, 
             sequence, reverseSeq,lastUpdate)
             SELECT route, `name`,StartDate,endDate,LineKey, CompanyKey, TractionType,Direction,
             next, prev,nextR,prevR,normalEnter,normalleave, reverseEnter, reverseLeave, 
              sequence, reverseSeq,lastUpdate FROM t2_backup;
COMMIT;
