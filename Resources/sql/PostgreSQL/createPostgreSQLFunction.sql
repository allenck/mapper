DELIMITER $$
CREATE FUNCTION distance(lat1 NUMERIC(11,6), lon1 NUMERIC(11,6), lat2 NUMERIC(11,6), lon2 NUMERIC(11,6))
RETURNS NUMERIC(11,6) AS $$
BEGIN
SELECT
    ST_DistanceSphere(
        ST_MakePoint(lon1, lat1),
        ST_MakePoint(lon2, lat2)
    ) / 1000 AS distance_in_km; -- Divide by 1000 to get kilometers
END;
$$  LANGUAGE plpgsql;
