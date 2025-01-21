BEGIN;
drop table if exists RouteName;
CREATE TABLE if not exists `RouteName` (
  `RouteId` integer NOT NULL primary key AUTO_INCREMENT,
  `Name` varchar(140) NOT NULL,
  `StartDate` date NOT NULL DEFAULT '1800-01-01',
  `endDate` date NOT NULL DEFAULT '1800-01-01',
  `routeAlpha` varchar(8) NOT NULL,
  `companykey` int NOT NULL DEFAULT -1,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (routeAlpha, `Name`,`StartDate`, enddate, companykey));

  

COMMIT;
