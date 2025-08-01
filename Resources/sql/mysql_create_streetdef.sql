DROP TABLE IF EXISTS `StreetDef` ;
CREATE TABLE IF NOT EXISTS `StreetDef` ( `StreetId` INTEGER   AUTO_INCREMENT ,
                                          `Street` varchar(25) NOT NULL  ,
                                          `Location` varchar(25),
                                          `StartDate` date NOT NULL DEFAULT '2050-01-01',
                                          `EndDate` date ,
                                          `StartLatLng` text NOT NULL , 
                                          `EndLatLng` text NOT NULL , 
                                          `Length` decimal(15,5) NOT NULL DEFAULT 0,
                                          `Bounds` text,
                                          `Segments` text ,
                                          `Comment` text,
                                          `Seq` INT NOT NULL DEFAULT 1,
                                          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                          PRIMARY KEY(`StreetId`, `Seq`),
                                          UNIQUE (`Street`, `StartDate`)
                                       );
