BEGIN TRANSACTION;
CREATE TEMPORARY if not exists `t_tractionTypes` (tractionType, description, displayColor, routeType, icon, lastUpdate);
INSERT INTO `t_tractionTypes`  (tractionType, description, displayColor, routeType, icon, lastUpdate) select tractionType, description, displayColor, routeType, icon, lastUpdate FROM `TractionTypes`;
DROP TABLE `TractionTypes`;
CREATE TABLE if not exists `TractionTypes` (
          `tractionType` integer NOT NULL primary key AUTOINCREMENT,
          `description` varchar(50) NOT NULL DEFAULT '',
          `displayColor` char(7) NOT NULL DEFAULT '#000000',
          `routeType` int(11) NOT NULL DEFAULT '0',
          `icon` varchar(10) NOT NULL DEFAULT '',
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);
INSERT INTO `TractionTypes` (tractionType, description, displayColor, routeType, icon, lastUpdate) select tractionType, description, displayColor, routeType, icon, lastUpdate FROM `t_tractionTypes`;
DROP TABLE `t_tractionTypes`;
COMMIT TRANSACTION;
