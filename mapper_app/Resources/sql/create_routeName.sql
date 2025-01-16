BEGIN;
drop table if exists RouteName;
CREATE TABLE if not exists `RouteName` (
  `RouteId` integer NOT NULL primary key AUTOINCREMENT,
  `Name` varchar(100) NOT NULL,   
  `StartDate` date NOT NULL DEFAULT '1800-01-01',
  `endDate` date NOT NULL DEFAULT '1800-01-01',
  `routeAlpha` varchar(8) NOT NULL,
  `companykey` int NOT NULL DEFAULT -1,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (routeAlpha, `Name`,`StartDate`, enddate, companykey));

  insert into RouteName (Name, startdate, enddate, routealpha, companyKey)
      select distinct name, startDate, enddate, routeAlpha from routes r
      join altRoute  a on r.route = a.route;

COMMIT;
