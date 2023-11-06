SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
SET ANSI_PADDING ON;
CREATE TABLE  [dbo].[Stations](
      [stationKey] [int] IDENTITY(1,1) NOT NULL,
      [route] [int] NOT NULL,
      [name] [varchar](75) NOT NULL,
      [suffix] varchar(4) NOT NULL DEFAULT '',
      [latitude] [decimal](15, 13) NOT NULL,
      [longitude] [decimal](15, 13) NOT NULL,
      [startDate] [date] NOT NULL,
      [endDate] [date] NOT NULL,
      [SegmentId] [int] DEFAULT -1,
      [point] [int] NOT NULL DEFAULT 0,
      [infoKey] [int] NULL,
      [geodb_loc_id] [int] NULL,
      [routeType] [int] NOT NULL,
      [markerType] varchar(15) default '',
      [lastUpdate] [datetime] NOT NULL,
   CONSTRAINT [PK_station] PRIMARY KEY CLUSTERED
  (
      [route] ASC,
      [name] ASC,
      [startDate] ASC,
      [endDate] ASC
  ) WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
  ) ON [PRIMARY];
SET ANSI_PADDING OFF;
//  ALTER TABLE [dbo].[Stations]  WITH CHECK ADD  CONSTRAINT [FK__stations__segmen__4AB81AF0] FOREIGN KEY([lineSegmentId])"\
//    "REFERENCES [dbo].[LineSegment] ([Key]);
//ALTER TABLE [dbo].[stations] CHECK CONSTRAINT [FK__stations__segmen__4AB81AF0];
ALTER TABLE [dbo].[Stations] ADD  CONSTRAINT [DF_stations_route]  DEFAULT ((0)) FOR [route];
ALTER TABLE [dbo].[Stations] ADD  CONSTRAINT [DF_stations_routeType]  DEFAULT ((0)) FOR [routeType];
ALTER TABLE [dbo].[Stations] ADD  CONSTRAINT [DF_stations_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];
