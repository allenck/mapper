BEGIN;
drop table if exists RouteName;
CREATE TABLE if not exists `RouteName` (
  `RouteId` integer NOT NULL primary key AUTO_INCREMENT,
  `Name` varchar(140) NOT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (name));

  

COMMIT;
