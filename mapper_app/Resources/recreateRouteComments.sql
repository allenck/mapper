BEGIN TRANSACTION;
CREATE TEMPORARY TABLE if not exists `t_routeComments` (`route`, `date`, `commentKey`, `CompanyKey`, lastUpdate);
INSERT INTO `t_routeComments`  (`route`, `date`, `commentKey`, `CompanyKey`, lastUpdate)
    select `route`, `date`, `commentKey`, `CompanyKey`, lastUpdate FROM `RouteComments`;
DROP TABLE `RouteComments`;
CREATE TABLE if not exists `RouteComments` (
          `route` int(6) NOT NULL,
          `date` date NOT NULL,
          `commentKey` int(11) NOT NULL,
          `companyKey` int(11) NOT NULL,
          `latitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
          `longitude` decimal(15,5) NOT NULL DEFAULT '0.00000',
          `lastUpdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
          constraint pk PRIMARY KEY (`route`,`date`),
          CONSTRAINT `RouteComments_ibfk_1` FOREIGN KEY (commentKey) REFERENCES Comments(commentKey) ON DELETE RESTRICT);

INSERT INTO `RouteComments` (`route`, `date`, `commentKey`, `CompanyKey`, lastUpdate)
       select `route`, `date`, `commentKey`, `CompanyKey`, lastUpdate FROM `t_routeComments`;
DROP TABLE `t_routeComments`;
COMMIT TRANSACTION;
