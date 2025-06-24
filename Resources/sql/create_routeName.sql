drop table if exists RouteName;
CREATE TABLE if not exists `RouteName` (
  `RouteId` integer NOT NULL primary key AUTOINCREMENT,
  `Name` varchar(140) NOT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (`Name`));
