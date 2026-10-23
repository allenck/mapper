BEGIN TRANSACTION;
CREATE TEMPORARY TABLE  t_routeName (RouteId,name,canonicalName, lastUpdate);
INSERT INTO t_routeName  (RouteId,name,canonicalName, lastUpdate)  select  RouteId,name,canonicalName, lastUpdate from RouteName;
DROP TABLE RouteName;
CREATE TABLE  RouteName (
  RouteId integer NOT NULL primary key AUTOINCREMENT,
  Name varchar(140) NOT NULL,
  canonicalName VARCHAR(140) NOT NULL,
  lastUpdate timestamp DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (canonicalName));
INSERT INTO RouteName (RouteId,name,canonicalName, lastUpdate) select  RouteId,name,canonicalName, lastUpdate from t_routeName;
DROP TABLE t_routeName;
COMMIT TRANSACTION;

