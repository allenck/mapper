/* run this script in psql */
/* the user who runs this script must be a superuser and NOT postgres! */
/* the server must implement SSL */
/* before running this script, REPLACE all occurrences of 'your_user_name' with your PostgreSQL logon userid;
    REPLACE all occurrences of 'your_password' with your PostgreSQL logon password

   hostname, userid and password must be the same as used to login to psql! */

 \set ON_ERROR_STOP on

DO
$do$
BEGIN
   IF  (
      SELECT CURRENT_USER = 'postgres') THEN
      RAISE EXCEPTION 'User cannot be postgres! Skipping.';
   END IF;
END
$do$;

DO
$do$
BEGIN
   IF NOT  (
      select usesuper from pg_user where usename = CURRENT_USER) THEN
      RAISE EXCEPTION 'User must be a superuser and not postgres! Skipping.';
    END IF;
END
$do$;



DO
$do$
BEGIN
   IF EXISTS (
      SELECT FROM pg_catalog.pg_roles
      WHERE  rolname = 'mapper') THEN
          RAISE notice 'Role "mapper" already exists. Skipping.';
   ELSE
      CREATE ROLE mapper;
   END IF;
END
$do$;

GRANT mapper TO current_user;

CREATE EXTENSION IF NOT EXISTS dblink;

DO $$
BEGIN
    PERFORM dblink_exec('host=localhost port=5432 user=your_user_name password=your_password dbname=postgres sslmode = prefer', 'CREATE DATABASE stlouis');
EXCEPTION WHEN duplicate_database THEN
    RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
END $$;
ALTER DATABASE stlouis OWNER TO mapper;

DO $$
BEGIN
    PERFORM dblink_exec('host=localhost port=5432 user=your_user_name password=your_password dbname=postgres sslmode = prefer', 'CREATE DATABASE cincinnati');
EXCEPTION WHEN duplicate_database THEN
    RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
END $$;
ALTER DATABASE cincinnati OWNER TO mapper;

DO $$
BEGIN
    PERFORM dblink_exec('host=localhost port=5432 user=your_user_name password=your_password dbname=postgres sslmode = prefer', 'CREATE DATABASE louisville');
EXCEPTION WHEN duplicate_database THEN
    RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
END $$;
ALTER DATABASE louisville OWNER TO mapper;

DO $$
BEGIN
    PERFORM dblink_exec('host=localhost port=5432 user=your_user_name password=your_password dbname=postgres sslmode = prefer', 'CREATE DATABASE indianapolis');
EXCEPTION WHEN duplicate_database THEN
    RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
END $$;
ALTER DATABASE indianapolis OWNER TO mapper;

DO $$
BEGIN
    PERFORM dblink_exec('host=localhost port=5432 user=your_user_name password=your_password dbname=postgres sslmode = prefer', 'CREATE DATABASE berlin');
EXCEPTION WHEN duplicate_database THEN
    RAISE NOTICE '%, skipping', SQLERRM USING ERRCODE = SQLSTATE;
END $$;
ALTER DATABASE berlin OWNER TO mapper;

DROP FUNCTION distance(double precision,double precision,double precision,double precision);
CREATE OR REPLACE FUNCTION public.distance(inLat1 double precision, inLon1 double precision,
                                                                                        inLat2 double precision, inLon2 double precision)
                                                                                        RETURNS double precision AS $dist$
    DECLARE
        R double precision;
        lat1 double precision;
        lat2 double precision;
        dLat double precision;
        dLon double precision;
        a double precision;
        c double precision;
        dist double precision =0;
     BEGIN
        /*compute the distance in km between two points.*/
        IF ( inLat1 = inLat2 AND inLon1 = inLon2 )
            THEN RETURN dist;
        END IF;

                R = 6371; /*earth radius in km */
                lat1 =  radians(inLat1);
                lat2 =  radians(inLat2);
                dLat = radians(inLat2 - inLat1 );
                dLon = radians(inLon2 - inLon1 );
                a = sin( dLat / 2 ) * sin( dLat / 2 ) + cos( lat1 ) * cos( lat2 ) * sin( dLon / 2 ) * sin( dLon / 2 ) ;
                c = 2.0 * atan2( sqrt( a ) , sqrt( 1.0 - a ) ) ;
                dist = R * c;
                /* distance in km */
                RETURN dist;
    END;
$dist$ LANGUAGE plpgsql;
ALTER FUNCTION distance(double precision,double precision,double precision,double precision) OWNER TO mapper;

quit
