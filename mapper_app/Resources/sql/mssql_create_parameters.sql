SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
SET ANSI_PADDING ON;
  CREATE TABLE [dbo].[Parameters](
        [key] [int] IDENTITY(1,1) NOT NULL,
        [lat] [decimal](18, 15) NOT NULL,
        [lon] [decimal](18, 15) NOT NULL,
        [title] [varchar](50) NOT NULL,
        [city] [varchar](50) NOT NULL,
        [minDate] [date] NOT NULL,
        [maxDate] [date] NOT NULL,
        [alphaRoutes] [char](1) NOT NULL,
        [abbreviationsList] VARCHAR(200) NOT NULL DEFAULT '',
        [lastUpdate] [datetime] NOT NULL
    ) ON [PRIMARY];
   SET ANSI_PADDING OFF;
   ALTER TABLE [dbo].[Parameters]  WITH CHECK ADD  CONSTRAINT [CK_parameters] CHECK  (([alphaRoutes]='Y' OR [alphaRoutes]='N'));
   ALTER TABLE [dbo].[Parameters] CHECK CONSTRAINT [CK_parameters];
   ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_minDate]  DEFAULT ('1899-01-01') FOR [minDate];
   ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_maxDate]  DEFAULT ('1966-5-21') FOR [maxDate];
   ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_alphaRoutes]  DEFAULT ('N') FOR [alphaRoutes];
   ALTER TABLE [dbo].[Parameters] ADD  CONSTRAINT [DF_parameters_lastUpdate]  DEFAULT (getdate()) FOR [lastUpdate];
 
