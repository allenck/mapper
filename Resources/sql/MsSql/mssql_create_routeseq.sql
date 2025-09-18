SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
SET ANSI_PADDING ON;
CREATE TABLE [dbo].[RouteSeq](
      [route] [int]  NOT NULL,
      [name] [varchar](125) NOT NULL,
      [startDate] [date] NOT NULL,
      [endDate] [date] NOT NULL,
      [segmentList] [varchar](500) NOT NULL,
      [firstSegment] [int] NOT NULL,
      [whichEnd] [varchar](1) ,
      [lastUpdate] [datetime] NOT NULL,
      CONSTRAINT [PK_RouteSeq] PRIMARY KEY CLUSTERED
      (
        [route] ASC,
        [name] ASC,
        [startDate] ASC,
        [endDate] ASC
      )WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
    ) ON [PRIMARY];
SET ANSI_PADDING OFF;
