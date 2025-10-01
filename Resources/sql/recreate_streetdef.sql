BEGIN TRANSACTION;

CREATE TEMPORARY TABLE `t1_backup` ( `StreetId`, `Street`, `Location`,`StartDate`, `EndDate`,`StartLatLng`, `EndLatLng`, `Length`, 
                                          `Bounds` ,  `Segments` , `Comment`, `Seq`, `lastUpdate`);
insert into `t1_backup` SELECT `StreetId`, `Street`, `Location`,`StartDate`, `EndDate`,`StartLatLng`, `EndLatLng`, `Length`, 
                                          `Bounds` ,  `Segments` , `Comment`,`Seq`, `lastUpdate` from `StreetDef`;
DROP TABLE streetdef;

CREATE TABLE IF NOT EXISTS `StreetDef` ( `StreetId` INTEGER   AUTO_INCREMENT ,
                                          `Street` varchar(30) NOT NULL DEFAULT '' ,
                                          `Location` varchar(30),
                                          `StartDate` date NOT NULL DEFAULT '2050-01-01',
                                          `EndDate` date ,
                                          `StartLatLng` varvhar(20),
                                          `EndLatLng` varchar(20),
                                          `Length` decimal(15,5) NOT NULL DEFAULT 0,
                                          `Bounds` varchar(60),
                                          `Segments`varchar(100),
                                          `Comment` varchar(100),
                                          `Seq` INT NOT NULL DEFAULT 1,
                                          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
                                          PRIMARY KEY(`StreetId`, `Seq`),
                                          UNIQUE (`Street`, `StartDate`)
                                           );

insert into `StreetDef` SELECT `StreetId`, `Street`, `Location`,`StartDate`, `EndDate`,`StartLatLng`, `EndLatLng`, `Length`,
                                          `Bounds` ,  `Segments` , `Comment`,`Seq`, `lastUpdate` from `t1_backup`;


drop table t1_backup;

#COMMIT;
