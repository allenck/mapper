SET ANSI_PADDING ON;
DROP TABLE IF NOT EXISTS [dbo].[Routes];
CREATE TABLE [dbo].[Routes](
                   [Route] [int] NOT NULL,
                   [name] [varchar](125) NOT NULL, 
                   [StartDate] [date] NOT NULL,
                   [EndDate] [date] NOT NULL,
                   [LineKey] [int] NOT NULL,
                   [OneWay] char(1) DEFAULT 'Y',
                   [TrackUsage] char(1) DEFAULT ' ',
                   [CompanyKey] [int] NOT NULL,
                   [tractionType] [int] NOT NULL,
                   [Direction] [varchar](6) NOT NULL,
                   [next] [int] NOT NULL,
                   [prev] [int] NOT NULL,
                   [nextR] [int] NOT NULL,
                   [prevR] [int] NOT NULL,
                   [normalEnter] [int] NOT NULL,
                   [normalLeave] [int] NOT NULL,
                   [reverseEnter] [int] NOT NULL,
                   [reverseLeave] [int] NOT NULL,
                   [Sequence] int(11) NOT NULL,
                   [ReverseSeq] int(11) NOT NULL,
                   [lastUpdate] [datetime] NOT NULL,
                  CONSTRAINT [PK_Routes] PRIMARY KEY CLUSTERED
                 (
                     [Route] ASC,
                     [name] ASC
                     [CompanyKey] ASC,
                     [StartDate] ASC,
                     [EndDate] ASC,
                     [LineKey] ASC,
                    )WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
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
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_next]  DEFAULT ((-1)) FOR [next];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_prev]  DEFAULT ((-1)) FOR [prev];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_normalEnter]  DEFAULT ((0)) FOR [normalEnter];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_normalLeave]  DEFAULT ((0)) FOR [normalLeave];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_reverseEnter]  DEFAULT ((0)) FOR [reverseEnter];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_reverseLeave]  DEFAULT ((0)) FOR [reverseLeave];
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_newName]  DEFAULT ('') FOR [name]
ALTER TABLE [dbo].[Routes] ADD  CONSTRAINT [DF_Routes_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];
 
