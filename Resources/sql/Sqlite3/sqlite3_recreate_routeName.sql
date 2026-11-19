BEGIN TRANSACTION;
CREATE TEMPORARY TABLE  t_routeName (RouteId,name,canonicalName, lastUpdate, description, startdate, enddate);
INSERT INTO t_routeName  (RouteId,name,canonicalName, lastUpdate, description, startdate, enddate)
select  RouteId,name,canonicalName, lastUpdate, description, startdate, enddate from RouteName;
DROP TABLE RouteName;
CREATE TABLE  RouteName (
  RouteId integer NOT NULL primary key AUTOINCREMENT,
  Name varchar(140) NOT NULL,
  Description VARCHAR(1000),
  StartDate date,
  Enddate date,
  canonicalName VARCHAR(140) NOT NULL,
  lastUpdate timestamp DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (canonicalName, startdate));
INSERT INTO RouteName (RouteId,name,description, startdate,enddate, canonicalName, lastUpdate)
select  RouteId,name,description, startdate,enddate, canonicalName, lastUpdate from t_routeName;
DROP TABLE t_routeName;
COMMIT TRANSACTION;

