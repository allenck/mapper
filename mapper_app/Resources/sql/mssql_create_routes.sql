SET ANSI_PADDING ON;
DROP TABLE IF EXISTS [dbo].[Routes];
CREATE TABLE [dbo].[Routes](
                   [Route] [int] NOT NULL,
                   [RouteId] int,
                   [StartDate]  [date] NOT NULL,
                   [EndDate] [date] NOT NULL,
                   [LineKey] [int] NOT NULL,
                   [OneWay] char(1) check([oneWay] in ('Y','N',' ')) DEFAULT 'Y',
                   [TrackUsage] char(1) check([TrackUsage] in ('B', 'L', 'R', ' ')) DEFAULT ' ',
                   [CompanyKey] [int] NOT NULL,
                   [tractionType] [int] NOT NULL,
                   [Direction] [varchar](6) NOT NULL,
                   [next] [int] NOT NULL DEFAULt -1,
                   [prev] [int] NOT NULL DEFAULt -1,
                   [nextR] [int] NOT NULL DEFAULt -1,
                   [prevR] [int] NOT NULL DEFAULt -1,
                   [normalEnter] [int] NOT NULL DEFAULt 0,
                   [normalLeave] [int] NOT NULL DEFAULt 0,
                   [reverseEnter] [int] NOT NULL DEFAULt 0,
                   [reverseLeave] [int] NOT NULL DEFAULt 0,
                   [Sequence] int NOT NULL DEFAULt -1,
                   [ReverseSeq] int NOT NULL DEFAULt -1,
                   [lastUpdate] [datetime] NOT NULL,
                  CONSTRAINT [PK_Routes] PRIMARY KEY CLUSTERED
                 (
                     [Route] ASC,
                     [RouteId] ASC,
                     [CompanyKey] ASC,
                     [StartDate] ASC,
                     [EndDate] ASC,
                     [LineKey] ASC
                    ) WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
                   ) ON [PRIMARY];
SET ANSI_PADDING OFF;
ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_AltRoute_route] FOREIGN KEY([Route])
                   REFERENCES [dbo].[AltRoute] ([route]);
ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_AltRoute_route];
ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Companies] FOREIGN KEY([CompanyKey])
REFERENCES [dbo].[Companies] ([key]);
ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Companies];
ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Routes_Segments] FOREIGN KEY([LineKey])
REFERENCES [dbo].[Segments] ([SegmentId]);
ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Routes_Segments];
ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_Routes_TractionType] FOREIGN KEY([tractionType])
REFERENCES [dbo].[TractionTypes] ([tractionType]);
ALTER TABLE [dbo].[Routes] CHECK CONSTRAINT [FK_Routes_Routes_TractionType];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_CompanyKey]  DEFAULT ((-1)) FOR [CompanyKey];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [[DF_Routes_tractionType]]]  DEFAULT ((1)) FOR [tractionType];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [[DF_Routes_Direction]]]  DEFAULT ('') FOR [Direction];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];
ALTER TABLE [dbo].[Routes]  WITH CHECK ADD  CONSTRAINT [FK_Routes_RouteName_routeId] FOREIGN KEY([RouteId])
                   REFERENCES [dbo].[RouteName] ([routeId]);
