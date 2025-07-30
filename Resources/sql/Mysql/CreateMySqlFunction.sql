DELIMITER $$
CREATE FUNCTION `distance`( inLat1 DOUBLE , inLon1 DOUBLE , inLat2 DOUBLE , inLon2 DOUBLE ) 
RETURNS double 
DETERMINISTIC 
BEGIN 
    DECLARE R double; 
    DECLARE lat1 double; 
    DECLARE lat2 double; 
    DECLARE dLat double;  
    DECLARE dLon double; 
    DECLARE a double; 
    DECLARE c double; 
    DECLARE d double; 
 
    /*compute the distance in km between two points.*/
    IF ( inLat1 = inLat2 AND inLon1 = inLon2 ) 
        THEN RETURN 0; 
    END IF; 


    SET R = 6371; /*earth radius in km */ 
    SET lat1 = radians( inLat1 );  
    SET lat2 = radians( inLat2 ); 
    SET dLat = radians( inLat2 - inLat1 ); 
    SET dLon = radians( inLon2 - inLon1 );  
    SET a = asin( dLat / 2 ) * asin( dLat / 2 ) + acos( lat1 ) * acos( lat2 ) * asin( dLon / 2 ) * asin( dLon / 2 ) ; 
    SET c = 2.0 * atan2( sqrt( a ) , sqrt( 1.0 - a ) ) ; 
    SET d = R * c;  
    /* distance in km */
    RETURN d; 
END$$
