USE [master]
GO

/****** Object:  UserDefinedFunction [dbo].[distance]    Script Date: 10/12/2023 8:04:06 PM ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO



CREATE FUNCTION [dbo].[distance](@lat1 FLOAT, @lat2 FLOAT, @long1 FLOAT, @long2 FLOAT )
RETURNS FLOAT(18)
AS
Begin
      Declare @R Float(8);
      Declare @dLat Float(18);
      Declare @dLon Float(18);
      Declare @a Float(18);
      Declare @c Float(18);
      Declare @d Float(18);
      Set @R =  6367.45
            --Miles 3956.55
            --Kilometers 6367.45
            --Feet 20890584
            --Meters 6367450


      Set @dLat = Radians(@lat2 - @lat1);
      Set @dLon = Radians(@long2 - @long1);
      Set @a = Sin(@dLat / 2)
                 * Sin(@dLat / 2)
                 + Cos(Radians(@lat1))
                 * Cos(Radians(@lat2))
                 * Sin(@dLon / 2)
                 * Sin(@dLon / 2);
      Set @c = 2 * Asin(Min(Sqrt(@a)));

      Set @d = @R * @c;
      Return @d;

End
GO
