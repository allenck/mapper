PRAGMA foreign_keys = 0;
BEGIN;
CREATE TEMPORARY TABLE if not exists `t_routes` (`Route` ,  `Name` ,  `StartDate`, `EndDate` ,
             `LineKey` ,  `OneWay`,  `TrackUsage` ,  `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`);
INSERT INTO  `t_routes` (`Route` ,  `Name` ,  `StartDate`, `EndDate` ,   `LineKey` ,  `OneWay`,
             `TrackUsage` ,  `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`)
      SELECT `Route` ,  `Name` ,  `StartDate`, `EndDate` ,   `LineKey` ,  `OneWay`,  `TrackUsage` ,
             `CompanyKey`,
             `TractionType`,  `Direction`,  `Next`,   `Prev`,  `NextR`,   `PrevR`,
             `NormalEnter`,  `NormalLeave`,  `ReverseEnter`,   `ReverseLeave` ,  `Sequence` ,
             `ReverseSeq`,  `LastUpdate`
             from `Routes`;
#include sqlite3_create_routes.sql
   INSERT INTO `Routes` (`Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`,
               `CompanyKey`, `tractionType`, `Direction`, `next`, `prev`, `NextR`,   `PrevR`,
               `normalEnter`,  `normalLeave`,
               `reverseEnter`,  `reverseLeave`, `Sequence`, `ReverseSeq`, `lastUpdate`)
           SELECT `Route`, `Name`,  `StartDate`,  `EndDate`,  `LineKey`,  `OneWay` ,`TrackUsage`, `CompanyKey`,
                `tractionType`, `Direction`, `next`, `prev`, `NextR`,   `PrevR`,
                `normalEnter`,  `normalLeave`,  `reverseEnter`,
                `reverseLeave`, `Sequence`, `ReverseSeq`, `lastUpdate`s
             from `t_routes`;
DROP TABLE `t_routes`;
COMMIT;

