BEGIN;
CREATE TEMPORARY TABLE if not exists `t_routeComments`  select * from RouteComments ;

DROP TABLE `RouteComments`;
CREATE TABLE if not exists `RouteComments` ( 
          `route` int NOT NULL, 
          `date` date NOT NULL, 
          `commentKey` int  NOT NULL, 
          `companyKey` int  NOT NULL, 
          `latitude` decimal(15,5) NOT NULL DEFAULT '0.00000', 
          `longitude` decimal(15,5) NOT NULL DEFAULT '0.00000', 
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP, 
          constraint pk PRIMARY KEY (`route`,`date`), 
          CONSTRAINT `RouteComments_ibfk_1` FOREIGN KEY (commentKey) REFERENCES Comments(commentKey) ON DELETE RESTRICT); 

INSERT INTO `RouteComments` (`route`, `date`, `commentKey`, `CompanyKey`, latitude, longitude, lastUpdate) 
       select `route`, `date`, `commentKey`, `CompanyKey`, latitude, longitude, lastUpdate FROM `t_routeComments`;
DROP TABLE `t_routeComments`;
COMMIT;

select * from t_routeComments
