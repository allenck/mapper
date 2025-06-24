CREATE TABLE `Stations` (
  `stationKey` integer NOT NULL primary key AUTOINCREMENT,
  `routes` varchar(250) NOT NULL DEFAULT '',
  `name` varchar(75) NOT NULL,
  `latitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `longitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `startDate` date DEFAULT NULL,
  `endDate` date DEFAULT NULL,
  `segmentId` int(11) NOT NULL,
  `segments` varchar(100) NOT NULL DEFAULT '',
  `infoKey` int(11) NOT NULL DEFAuLT -1,
  `markerType` varchar(15) default '',
  `routeType` int(11) NOT NULL DEFAULT -1,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint main unique (`name`,`startDate`,`endDate`),
  constraint `stationKey` UNIQUE (`stationKey`)
);
