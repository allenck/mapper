BEGIN ;
CREATE TEMPORARY TABLE if not exists `t_companies`like Companies;
ALTER TABLE `Routes` DROP FOREIGN Key `Routes_ibfk_3`;
DROP TABLE `Companies`;
CREATE TABLE if not exists `Companies` (
          `key` integer NOT NULL primary key AUTO_INCREMENT,
          `Description` varchar(50) NOT NULL,
          `routePrefix` varchar(10) default '',
          `info` varchar(50) default '',
          `startDate` date DEFAULT NULL,
          `endDate` date DEFAULT NULL,
          `firstRoute` int(11) DEFAULT NULL,
          `lastRoute` int(11) DEFAULT NULL,
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
ALTER TABLE `Routes` ADD CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`);
INSERT INTO `Companies` (`key`, Description, routePrefix, info, startDate, endDate, firstRoute, lastRoute, lastUpdate) select `key`, Description, routePrefix, startDate, endDate, firstRoute, lastRoute, lastUpdate FROM `t_companies`;
DROP TABLE `t_companies`;
COMMIT;