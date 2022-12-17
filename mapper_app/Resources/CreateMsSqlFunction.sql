IF OBJECT_ID (N'dbo.distance', N'FN') IS NOT NULL
    DROP FUNCTION distance;

create function [dbo].[fnCalcDistanceMiles] (@Lat1 decimal(8,4), @Long1 decimal(8,4), @Lat2 decimal(8,4), @Long2 decimal(8,4))
returns decimal (8,4) as
begin
declare @d decimal(28,10)
-- Convert to radians
set @Lat1 = @Lat1 / 57.2958
set @Long1 = @Long1 / 57.2958
set @Lat2 = @Lat2 / 57.2958
set @Long2 = @Long2 / 57.2958
-- Calc distance
set @d = (Sin(@Lat1) * Sin(@Lat2)) + (Cos(@Lat1) * Cos(@Lat2) * Cos(@Long2 - @Long1))
END

select dbo.distance(1,2,3,4)