CREATE TABLE IF NOT EXISTS `StreetDef` ( `StreetId` INTEGER   AUTO_INCREMENT ,
                                          `Street` `text` NOT NULL DEFAULT '' ,
                                          `Location` text NOT NULL DEFAULT '',
                                          `StartDate` date NOT NULL DEFAULT '2050-01-01',
                                          `EndDate` date NOT NULL DEFAULT '2050-01-01',
                                          `StartLatLng` `text` NOT NULL DEFAULT '', 
                                          `EndLatLng` `text` NOT NULL DEFAULT '', 
                                          `Length` decimal(15,5) NOT NULL DEFAULT 0,
                                          `Bounds` `text` NOT NULL DEFAULT "0,0,0,0" ,               
                                          `Segments` text NOT NULL DEFAULT '',
                                          `Comment` `text` NOT NULL DEFAULT '',
                                          `Seq` INT NOT NULL DEFAULT 1,
                                          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
                                           );
