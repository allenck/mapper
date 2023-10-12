BEGIN TRANSACTION;
CREATE TEMPORARY TABLE if not exists `t_companies` (`key`, Description, routePrefix, info, startDate, endDate, firstRoute, lastRoute, lastUpdate);
INSERT INTO `t_companies`  (`key`, Description, routePrefix, info, startDate, endDate, firstRoute, lastRoute, lastUpdate) select `key`, Description, routePrefix, info, startDate, endDate, firstRoute, lastRoute, lastUpdate FROM `Companies`;
PRAGMA foreign_keys = 0;
DROP TABLE `Companies`;
CREATE TABLE if not exists `Companies` (
          `key` integer NOT NULL primary key AUTOINCREMENT,
          `Description` varchar(50) NOT NULL,
          `routePrefix` varchar(10) default '',
          `info` varchar(50),
          `startDate` date DEFAULT NULL,
          `endDate` date DEFAULT NULL,
          `firstRoute` int(11) DEFAULT NULL,
          `lastRoute` int(11) DEFAULT NULL,
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
PRAGMA foreign_keys = 1;
INSERT INTO `Companies` (`key`, Description, routePrefix, info, startDate, endDate,
          firstRoute, lastRoute, lastUpdate) select `key`, Description, routePrefix, info, startDate, endDate,
          firstRoute, lastRoute, lastUpdate FROM `t_companies`;
DROP TABLE `t_companies`;
COMMIT TRANSACTION;
