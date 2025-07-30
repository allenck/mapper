/****** Object:  UserDefinedFunction [dbo].[fnCalcDistanceKM]    Script Date: 12/20/2022 12:27:23 PM ******/
SET ANSI_NULLS ON;


SET QUOTED_IDENTIFIER ON;

CREATE OR ALTER FUNCTION [dbo].[distance](@lat1 FLOAT, @lat2 FLOAT, @lon1 FLOAT, @lon2 FLOAT)
RETURNS FLOAT 
AS
BEGIN

    RETURN ACOS(SIN(PI()*@lat1/180.0)*SIN(PI()*@lat2/180.0)+COS(PI()*@lat1/180.0)*COS(PI()*@lat2/180.0)*COS(PI()*@lon2/180.0-PI()*@lon1/180.0))*6371
END
