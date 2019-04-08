/* distance.c */

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1;

#include <stdlib.h>
#include <math.h>

static void Distance(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	double R; 
	double lat1; 
	double lat2; 
	double dLat; 
	double dLon; 
	double a; 
	double c; 
	double d=0.0; 
 double dToRad = 0.0174532925;
 double inLat1=0;
 double inLon1=0;
 double inLat2=0;
 double inLon2=0;
	int type;

	/*compute the distance in km between two points.*/ 
	if(argc == 4)
	{
		type = sqlite3_value_numeric_type( argv[0] );
  if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) 
		{
   inLat1 = sqlite3_value_double( argv[0] );
  }
  type = sqlite3_value_numeric_type( argv[1] );
  if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) 
		{
   inLon1 = sqlite3_value_double( argv[1] );
  }
  type = sqlite3_value_numeric_type( argv[2] );
  if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) 
		{
   inLat2 = sqlite3_value_double( argv[2] );
  }
  type = sqlite3_value_numeric_type( argv[3] );
  if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) ) 
		{
   inLon2 = sqlite3_value_double( argv[3] );
  }

		if( inLat1 == inLat2 && inLon1 == inLon2 ) 
			sqlite3_result_double(context, d);
	

		R = 6371; /*earth radius in km */
 	lat1 = inLat1 * dToRad;
 	lat2 = inLat2 * dToRad;
 	dLat = (inLat2 - inLat1 ) * dToRad; 
 	dLon = (inLon2 - inLon1 ) * dToRad; 
 	a = asin( dLat / 2 ) * asin( dLat / 2 ) + acos( lat1 ) * acos( lat2 ) * asin( dLon / 2 ) * asin( dLon / 2 ) ; 
 	c = 2.0 * atan2( sqrt( a ) , sqrt( 1.0 - a ) ) ; 
 	d = R * c; 
		/* distance in km */ 
 	sqlite3_result_double(context, d);
	}
 else
		sqlite3_result_null(context);
}


int distance_init( sqlite3 *db, char **error, const sqlite3_api_routines *api )
{
 SQLITE_EXTENSION_INIT2(api);

 sqlite3_create_function( db, "Distance", 4, SQLITE_UTF8,
            NULL, &Distance, NULL, NULL );

 return SQLITE_OK;
}

//allen@ubuntu:~$ gcc -shared -fPIC -o distance.so distance.c -Wall -W -O2 -L/usr/local/lib -lsqlite3 -lm
//distance.c: In function ‘distance_init’:
//distance.c:68:40: warning: unused parameter ‘error’ [-Wunused-parameter]


