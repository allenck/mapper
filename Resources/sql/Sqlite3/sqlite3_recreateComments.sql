BEGIN;
CREATE TEMPORARY TABLE if not exists `t_Comments` (`commentKey`, `tags`, `comments`,  `routeList`, `date`, `jRouteList`, latitude, longitude, lastUpdate);
INSERT INTO `t_Comments`  (`commentKey`, `tags`, `comments`, `routeList`, `date`, `jRouteList`, latitude, longitude, lastUpdate)
        select `commentKey`, `tags`, `comments`,  `routeList`, `date`, `jRouteList`, latitude, longitude, lastUpdate FROM `Comments` ;
DROP TABLE `Comments`;
CREATE TABLE if not exists `Comments` (
        `commentKey` INTEGER PRIMARY KEY AUTOINCREMENT,
	`tags`  varchar(1000) NOT NULL,
	`routeList` varchar(100) NOT NULL,
	`date` date ,
	`jRouteList` JSON NOT NULL DEFAULT '[]',
	`comments` mediumtext NOT NULL,
	`latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
	`longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
	lastUpdate timestamp  NOT NULL DEFAULT CURRENT_TIMESTAMP);
	
INSERT INTO `Comments` (`commentKey`, `tags`, `comments`,  `routeList`, `date`, `jRouteList`, latitude, longitude, lastUpdate)
        select `commentKey`, `tags`, `comments`, `routeList`, `date`, `jRouteList`, latitude, longitude, lastUpdate FROM `t_Comments` ;
DROP TABLE `t_Comments` ;
COMMIT;
