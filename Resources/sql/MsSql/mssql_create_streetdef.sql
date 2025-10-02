CREATE TABLE  [StreetDef] ( [StreetId] INTEGER   IDENTITY(1,1) ,
                            [Street] varchar(60) NOT NULL DEFAULT '',
                            [Location] varchar(30),
                            [StartDate] date NOT NULL DEFAULT '2050-01-01',
                            [EndDate] date,
                            [StartLatLng] varchar(20),
                            [EndLatLng] varchar(20),
                            [Length] decimal(15,5),
                            [Bounds] varchar(60),
                            [Segments] varchar(100),
                            [Comment] varchar(100),
                            [Seq] INT NOT NULL DEFAULT 1,
                            [lastUpdate] datetime NOT NULL
                          );
