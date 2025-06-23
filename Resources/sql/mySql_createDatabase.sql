CREATE TABLE if not exists `Parameters` (
  `key` integer NOT NULL primary key asc AUTOINCREMENT,
  `lat` decimal(18,15) NOT NULL default (0),
  `lon` decimal(18,15) NOT NULL default (0),
  `title` varchar(50) NOT NULL,
  `city` varchar(50) NOT NULL,
  `minDate` date NOT NULL,
  `maxDate` date NOT NULL,
  `alphaRoutes` char(1) NOT NULL default ('Y'),
  `lastUpdate` timestamp NOT NULL DEFAULT (CURRENT_TIMESTAMP));

CREATE TABLE if not exists `TractionTypes` (
  `tractionType` integer NOT NULL primary key AUTOINCREMENT,
  `description` varchar(50) NOT NULL DEFAULT '',
  `displayColor` char(7) NOT NULL DEFAULT '#000000',
  `routeType` int(11) NOT NULL DEFAULT '0',
  `icon` varchar(10) NOT NULL DEFAULT '',
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE if not exists `Companies` (
  `key` integer NOT NULL primary key AUTOINCREMENT,
  `Description` varchar(50) NOT NULL,
  `routePrefix` varchar(10) default '',
  `startDate` date DEFAULT NULL,
  `endDate` date DEFAULT NULL,
  `firstRoute` int(11) DEFAULT NULL,
  `lastRoute` int(11) DEFAULT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE if not exists `Comments` (
  `commentKey` integer NOT NULL primary key AUTOINCREMENT,
  `tags` varchar(1000) NOT NULL,
  `comments` mediumtext NOT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE if not exists `Intersections` (
  `key` integer NOT NULL primary key AUTOINCREMENT,
  `Street1` varchar(50) NOT NULL,
  `Street2` varchar(50) NOT NULL,
  `Latitude` decimal(15,13) NOT NULL,
  `Longitude` decimal(15,13) NOT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE if not exists  `altRoute` (
  `route` int(11) NOT NULL primary key AUTOINCREMENT,
  `routePrefix` varchar(10) default '',
  `routeAlpha` varchar(8) NOT NULL,
  `baseRoute` int(11) NOT NULL DEFAULT 0,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  CONSTRAINT `index` unique ( routePrefix, routeAlpha)
);


CREATE TABLE if not exists `Segments` (
  `SegmentId` integer NOT NULL primary key AUTOINCREMENT,
  `Description` varchar(100) NOT NULL,
  `OneWay` char(1) NOT NULL DEFAULT 'N',
  `Tracks` int(2) NOT NULL DEFAULT 2,
  `street` text NOT NULL DEFAULT '',
  `Locality` varchar(15) NOT NULL DEFAULT '',
  `Type` int(11) NOT NULL DEFAULT 0,
  `StartLat` decimal(15,13) NOT NULL DEFAULT 0.0,
  `StartLon` decimal(15,13) NOT NULL DEFAULT 0.0,
  `EndLat` decimal(15,13) NOT NULL DEFAULT 0.0,
  `EndLon` decimal(15,13) NOT NULL DEFAULT 0.0,
  `Length` decimal(15,5) NOT NULL DEFAULT 0,
  `points` int(11) NOT NULL default 0,
  `StartDate` date NOT NULL DEFAULT '1800-01-01',
  `endDate` date NOT NULL DEFAULT '1800-01-01',
  `Direction` varchar(6) NOT NULL DEFAULT ' ',
  `pointArray` text,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE if not exists `LineSegment` (
  `Key` integer NOT NULL primary key AUTOINCREMENT,
  `StartLat` decimal(15,13) NOT NULL,
  `StartLon` decimal(15,13) NOT NULL,
  `EndLat` decimal(15,13) NOT NULL,
  `EndLon` decimal(15,13) NOT NULL,
  `StreetName` varchar(50) NOT NULL,
  `SegmentId` int(11) NOT NULL,
  `Sequence` int(11) NOT NULL,
  `Length` decimal(15,5) NOT NULL DEFAULT '0.00000',
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint  `segment_sequence` UNIQUE (`SegmentId`,`Sequence`)
);

CREATE TABLE if not exists `Routes` (
  `Route` int(11) NOT NULL,
  `Name` varchar(149) NOT NULL,
  `StartDate` date NOT NULL,
  `EndDate` date NOT NULL,
  `LineKey` int(11) NOT NULL,
  `OneWay` char(1) DEFAULT 'Y',
  `TrackUsage` char(1) DEFAULT ' ',
  `CompanyKey` int(11) NOT NULL DEFAULT 0,
  `tractionType` int(11) NOT NULL DEFAULT 0,
  `Direction` varchar(6) NOT NULL DEFAULT ' ',
  `next` int(11) NOT NULL DEFAULT -1,
  `prev` int(11) NOT NULL DEFAULT -1,
  `normalEnter` int(11) NOT NULL DEFAULT 0,
  `normalLeave` int(11) NOT NULL DEFAULT 0,
  `reverseEnter` int(11) NOT NULL DEFAULT 0,
  `reverseLeave` int(11) NOT NULL DEFAULT 0,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint pk PRIMARY key (`Route`,`Name`,`StartDate`,`EndDate`,`LineKey`),
  CONSTRAINT `Routes_ibfk_1` FOREIGN KEY (`LineKey`) REFERENCES `Segments` (`SegmentId`),
  CONSTRAINT `Routes_ibfk_3` FOREIGN KEY (`CompanyKey`) REFERENCES `Companies` (`key`),
  CONSTRAINT `Routes_ibfk_4` FOREIGN KEY (`tractionType`) REFERENCES `TractionTypes` (`tractionType`),
  CONSTRAINT `Routes_ibfk_5` FOREIGN KEY (`Route`) REFERENCES `altRoute` (`route`)
);


CREATE TABLE if not exists `RouteComments` (
  `route` int(6) NOT NULL,
  `date` date NOT NULL,
  `commentKey` int(11) NOT NULL,
  `companyKey` int(11) NOT NULL,
  `latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
  `longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint pk PRIMARY KEY (`route`,`date`)
);

CREATE TABLE `Stations` (
  `stationKey` integer NOT NULL primary key AUTOINCREMENT,
  `route` int(11) NOT NULL DEFAULT 0,
  `name` varchar(75) NOT NULL,
  `suffix` varchar(4) NOT NULL DEFAULT '',
  `latitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `longitude` decimal(15,13) NOT NULL DEFAULT 0.0,
  `startDate` date DEFAULT NULL,
  `endDate` date DEFAULT NULL,
  `segmentId` int(11) NOT NULL,
  `point` int(11) NOT NULL DEFAULT 0,
  `infoKey` int(11) DEFAULT NULL,
  `geodb_loc_id` varchar(15) DEFAULT NULL,
  `routeType` int(11) NOT NULL DEFAULT -1,
  `markerType` varchar(15) default '',
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint main unique (`segmentId`,`name`,`startDate`,`endDate`),
  constraint `stationKey` UNIQUE (`stationKey`),
  CONSTRAINT `SegmentId_ibfk_1` FOREIGN KEY (`segmentId`) REFERENCES `Segments` (`SegmentId`)
);

CREATE TABLE if not exists `Terminals` (
  `Route` int(11) NOT NULL,
  `Name` varchar(125) NOT NULL,
  `StartDate` date NOT NULL,
  `EndDate` date NOT NULL,
  `StartSegment` int(11) NOT NULL,
  `StartWhichEnd` char(1) NOT NULL,
  `EndSegment` int(11) NOT NULL,
  `EndWhichEnd` char(1) NOT NULL,
  `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  constraint pk PRIMARY KEY (`Route`,`Name`,`StartDate`,`EndDate`)
);
