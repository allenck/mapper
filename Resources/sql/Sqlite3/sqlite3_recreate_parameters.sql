BEGIN TRANSACTION;
CREATE TEMPORARY TABLE if not exists `t_parameters` (`key`, lat, lon, title, city,minDate,MaxDate, abbreviationsList, lastUpdate);
INSERT INTO `t_parameters`  (`key`, lat, lon, title, city,minDate,MaxDate, abbreviationsList, lastUpdate)
 select `key`, lat, lon, title, city,minDate,MaxDate, abbreviationsList, lastUpdate FROM `Parameters`;
DROP TABLE `Parameters`;
CREATE TABLE if not exists `Parameters` (
          `key` integer NOT NULL primary key AUTOINCREMENT,
          `lat` decimal(18,15) NOT NULL DEFAULT 0,
          `lon` decimal(18,15) NOT NULL DEFAULT 0,
          `title` varchar(50) NOT NULL DEFAULT '',
          `city` varchar(50) NOT NULL DEFAULT '',
          `minDate` date NOT NULL,
          `maxDate` date NOT NULL,
          `alphaRoutes` char(1) NOT NULL DEFAULT 'N',
          `abbreviationsList` VARCHAR(200) NOT NULL DEFAULT '',
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
INSERT INTO `Parameters` (`key`, lat, lon, title, city,minDate,MaxDate, abbreviationsList, lastUpdate) select `key`, lat, lon, title, city,minDate,MaxDate, abbreviationsList, lastUpdate FROM `t_parameters`;
DROP TABLE `t_parameters`;
COMMIT TRANSACTION;
