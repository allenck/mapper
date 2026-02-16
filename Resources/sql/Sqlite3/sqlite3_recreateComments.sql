BEGIN;
CREATE TEMPORARY TABLE if not exists `t_Comments` (`commentKey`, `tags`, `comments`,  `routeList`, lastUpdate);
INSERT INTO `t_Comments`  (`commentKey`, `tags`, `comments`, `routeList`, lastUpdate)
	select `commentKey`, `tags`, `comments`,  `routeList`, lastUpdate FROM `Comments` ;
DROP TABLE `Comments`;
CREATE TABLE if not exists `Comments` (
	`commentKey` int(11) NOT NULL PRIMARY KEY,
	`tags`  varchar(1000) NOT NULL,
	`routeList` varchar(100) NOT NULL,
	`comments` mediumtex NOT NULL,
	lastUpdate timestamp  NOT NULL DEFAULT CURRENT_TIMESTAMP);
	
INSERT INTO `Comments` (`commentKey`, `tags`, `comments`,  `routeList`, lastUpdate)
        select `commentKey`, `tags`, `comments`, `routeList`, lastUpdate FROM `t_Comments` ;
DROP TABLE `t_Comments` ;
COMMIT;
