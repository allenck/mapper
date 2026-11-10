CREATE TABLE [RouteName] (
  [RouteId] integer NOT NULL primary key IDENTITY(1,1),
  [Name] varchar(140) NOT NULL,
  [canonicalName] varchar(140),
  [lastUpdate] datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE ([Name]));

