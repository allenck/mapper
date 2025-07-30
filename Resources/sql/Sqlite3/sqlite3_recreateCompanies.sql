update companies set info = '' where info is null;
BEGIN TRANSACTION;
CREATE TEMPORARY TABLE if not exists `t_companies` (`key`, mnemonic, Description, routePrefix, info, url,
            startDate, endDate, firstRoute, lastRoute, lastUpdate);
INSERT INTO `t_companies`  (`key`, mnemonic, Description, routePrefix, info, url, startDate, endDate,
            firstRoute, lastRoute, lastUpdate)
        select `key`, mnemonic, Description, routePrefix, info, url, startDate, endDate,
            firstRoute, lastRoute, lastUpdate FROM `Companies`;
PRAGMA foreign_keys = 0;
DROP TABLE `Companies`;
CREATE TABLE if not exists `Companies` (
          `key` integer NOT NULL primary key AUTOINCREMENT,
          `mnemonic` varchar(10) NOT NULL default '',
          `Description` varchar(100) NOT NULL default '',
          `routePrefix` varchar(10) NOT NULL default '',
          `info` varchar(60) NOT NULL DEFAULT '',
          `Url` varchar(150) NOT NULL DEFAULT '',
          `startDate` date DEFAULT NULL,
          `endDate` date DEFAULT NULL,
          `firstRoute` int(11) DEFAULT NULL,
          `lastRoute` int(11) DEFAULT NULL,
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
PRAGMA foreign_keys = 1;
INSERT INTO `Companies` (`key`, mnemonic,  Description, routePrefix, info, url, startDate, endDate,
          firstRoute, lastRoute, lastUpdate) select `key`, mnemonic, Description, routePrefix, info, url,
          startDate, endDate, firstRoute, lastRoute, lastUpdate FROM `t_companies`;
DROP TABLE `t_companies`;
COMMIT TRANSACTION;

