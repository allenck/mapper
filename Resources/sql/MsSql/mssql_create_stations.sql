SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
SET ANSI_PADDING ON;
CREATE TABLE  [dbo].[Stations](
      [stationKey] [int] IDENTITY(1,1) NOT NULL,
      [routes] [varchar](250) NOT NULL DEFAULT '',
      [name] [varchar](140) NOT NULL,
      [latitude] [decimal](15, 13) NOT NULL,
      [longitude] [decimal](15, 13) NOT NULL,
      [startDate] [date] NOT NULL,
      [endDate] [date] NOT NULL,
      [segmentId] int NOT NULL,
      [segments] varchar(100) NOT NULL DEFAULT '',
      [infoKey] int NOT NULL DEFAuLT -1,
      [markerType] varchar(15) default '',
      [routeType] int NOT NULL DEFAULT -1,
      [lastUpdate] [datetime] NOT NULL,
   CONSTRAINT [PK_station] PRIMARY KEY CLUSTERED
  (
      [name] ASC,
      [startDate] ASC,
      [endDate] ASC
  ) WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
  ) ON [PRIMARY];
SET ANSI_PADDING OFF;
ALTER TABLE [dbo].[Stations] ADD  CONSTRAINT [DF_stations_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];
