#! /bin/sh

# this script compiles the sqlite library for the version of sqlite used by the current Qt version.
# must run in a terminal that hast the Qt build environment set.
# must be run by root to save the output in /usr/lical/lib

echo echo $QTDIR/../Src/qtbase/src/3rdparty/sqlite/
cd $QTDIR/../Src/qtbase/src/3rdparty/sqlite/
gcc -shared sqlite3.c -ldl -o /usr/local/lib/libsqlite3.so -lpthread -fPIC  \
   -DSQLITE_ENABLE_COLUMN_METADATA \
   -DSQLITE_ENABLE_FTS3 \
   -DSQLITE_ENABLE_FTS3_PARENTHESIS \
   -DSQLITE_ENABLE_FTS5 \
   -DSQLITE_ENABLE_JSON1 \
   -DSQLITE_ENABLE_RTREE \
   -DSQLITE_OMIT_COMPLETE
