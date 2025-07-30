CREATE TABLE  [StreetDef] ( [StreetId] INTEGER   IDENTITY(1,1) ,
                                          [Street] text NOT NULL DEFAULT '' ,
                                          [Location] text ,
                                          [StartDate] date NOT NULL DEFAULT '2050-01-01',
                                          [EndDate] date ,
                                          [StartLatLng] text , 
                                          [EndLatLng] text , 
                                          [Length] decimal(15,5) ,
                                          [Bounds] text  ,               
                                          [Segments] text ,
                                          [Comment] text ,
                                          [Seq] INT NOT NULL DEFAULT 1,
                                          [lastUpdate] datetime NOT NULL 
                                           );
