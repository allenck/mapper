#include <stdlib.h>
#include <sqlite3ext.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


#ifndef SQLFUN
#define SQLFUN(NAME) sqlite3ext_sqlfun_##NAME
#endif

#ifndef SQLFUN_NAME
#define SQLFUN_NAME "define_function"
#endif

SQLITE_EXTENSION_INIT1;



// 1. Structure declarations ------------------------

typedef struct SQLFUN (funcdata_struct) {
	char *name;
	char *sqltext;
} SQLFUN(funcdata);



// 2. Function declarations -------------------------

char *SQLFUN(malloc_strcpy_value) (sqlite3_value * v);

void SQLFUN(genericFunc) (sqlite3_context * context, int argc,
			  sqlite3_value ** argv);

void SQLFUN(defineFunc) (sqlite3_context * context, int argc,
			 sqlite3_value ** argv);



// 3. Function definitions --------------------------

char *SQLFUN(malloc_strcpy_value) (sqlite3_value * v) {
	char *tmp, *ret;
	tmp = (char *) sqlite3_value_text(v);
	ret = sqlite3_malloc(strlen(tmp) + 1);
	strcpy(ret, tmp);
	return ret;
}

void SQLFUN(genericFunc) (sqlite3_context * context, int argc,
			  sqlite3_value ** argv) {
	int row;
	int i;
	SQLFUN(funcdata) * f = sqlite3_user_data(context);
	sqlite3_stmt *stmt;
	char *ret;

	sqlite3_prepare(sqlite3_context_db_handle(context),
			(char *) f->sqltext, -1, &stmt, NULL);

	for (i = 0; i < argc; i++) {
		char *arg_i = (char *) sqlite3_value_text(argv[i]);
		sqlite3_bind_text(stmt, i + 1, arg_i, -1,
				  SQLITE_TRANSIENT);
	}

	row = sqlite3_step(stmt);
	if (row == SQLITE_ROW) {
		ret = (char *) sqlite3_column_text(stmt, 0);
		sqlite3_result_text(context, ret, -1, SQLITE_TRANSIENT);
	} else {
		sqlite3_result_null(context);
	}

	sqlite3_finalize(stmt);
}

void SQLFUN(defineFunc) (sqlite3_context * context, int argc,
			 sqlite3_value ** argv) {
	char *name;
	int numpar;
	char *sqltext;
	int ret;

	SQLFUN(funcdata) * f;

	//assert(argc == 3);

	// 0.allocate f
	f = (SQLFUN(funcdata) *) sqlite3_malloc(sizeof(SQLFUN(funcdata)));

	// 1.allocate and initialize f->name
	f->name = SQLFUN(malloc_strcpy_value) (argv[0]);

	// 2.initialize numpar
	numpar = sqlite3_value_int(argv[1]);

	// 3.allocate and initialize f->sqltext
	f->sqltext = SQLFUN(malloc_strcpy_value) (argv[2]);

	ret=sqlite3_create_function(sqlite3_context_db_handle(context),
				f->name, numpar, SQLITE_UTF8, f,
				SQLFUN(genericFunc), NULL, NULL);

	sqlite3_result_int(context,(ret==0)?1:0);
}

#ifdef WIN32
__declspec(dllexport)
#endif
void SQLFUN(Distance)(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    //double R;
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
    double R;
    int type;

    /*compute the distance in km between two points.*/
    if(argc == 4)
    {
        type = sqlite3_value_numeric_type( argv[0] );
        if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) )
        {
            inLat1 = sqlite3_value_double( argv[0] );
        }
        else
            sqlite3_result_error(context, "distance: invalid parameter 1", -1);
        type = sqlite3_value_numeric_type( argv[1] );
        if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) )
        {
            inLon1 = sqlite3_value_double( argv[1] );
        }
        else
            sqlite3_result_error(context, "distance: invalid parameter 2", -1);
        type = sqlite3_value_numeric_type( argv[2] );
        if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) )
        {
            inLat2 = sqlite3_value_double( argv[2] );
        }
        else
            sqlite3_result_error(context, "distance: invalid parameter 3", -1);
        type = sqlite3_value_numeric_type( argv[3] );
        if ( ( type == SQLITE_FLOAT )||( type == SQLITE_INTEGER ) )
        {
            inLon2 = sqlite3_value_double( argv[3] );
        }
        else
            sqlite3_result_error(context, "distance: invalid parameter 4", -1);

        if( inLat1 == inLat2 && inLon1 == inLon2 )
            sqlite3_result_double(context, d);


        R = 6371.0; /*earth radius in km */
        lat1 = inLat1 * dToRad;
        lat2 = inLat2 * dToRad;
        dLat = (inLat2 - inLat1 )* dToRad ;
        dLon = (inLon2 - inLon1 )* dToRad ;
        a = asin( dLat / 2 ) * asin( dLat / 2 ) + acos( lat1 ) * acos( lat2 ) * asin( dLon / 2 ) * asin( dLon / 2 ) ;
        c = 2.0 * atan2( sqrt( a ) , sqrt( 1.0 - a ) ) ;
        d = R * c;
            /* distance in km */
        sqlite3_result_double(context, d);
        //printf("%3.8f %3.8f %3.8f %3.8f\n", inLat1, inLon1, inLat2, inLon2);
        //printf("a=%3.8f c=%3.8f d=%3.8f R=%3.8f\n", a, c, d, R);
     }
     else
        sqlite3_result_null(context);
}


/* SQLite invokes this routine once when it loads the extension. */
#ifdef WIN32
__declspec(dllexport)
#endif
int sqlite3_extension_init(sqlite3 * db, char **pzErrMsg,
			   const sqlite3_api_routines * pApi)
{
	int retval;
	SQLITE_EXTENSION_INIT2(pApi);
	retval=sqlite3_create_function(db, SQLFUN_NAME, 3, SQLITE_UTF8, NULL,
				SQLFUN(defineFunc), NULL, NULL);
    retval=sqlite3_create_function(db, "distance", 4, SQLITE_UTF8, NULL, SQLFUN(Distance), NULL,NULL);
	return retval;
}
