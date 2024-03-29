TEMPLATE = lib
TARGET = sqlite3

SOURCES += \
    sqlite3.c

HEADERS += \
    sqlite3ext.h \
    sqlite3.h

# DEFINES += SQLITE_ENABLE_COLUMN_METADATA \
#     SQLITE_ENABLE_FTS3 \
#     SQLITE_ENABLE_FTS3_PARENTHESIS \
#     SQLITE_ENABLE_FTS5 \
#     SQLITE_ENABLE_JSON1 \
#     SQLITE_ENABLE_RTREE \
#     SQLITE_OMIT_COMPLETE

DEFINES += SQLITE_ENABLE_COLUMN_METADATA \
SQLITE_ENABLE_FTS3 \
SQLITE_ENABLE_FTS3_PARENTHESIS \
SQLITE_ENABLE_FTS4 \
SQLITE_ENABLE_FTS5 \
SQLITE_ENABLE_GEOPOLY \
SQLITE_ENABLE_JSON1 \
SQLITE_ENABLE_MATH_FUNCTIONS \
SQLITE_ENABLE_RTREE \
SQLITE_OMIT_COMPLETE

#build_pass:CONFIG(debug, debug|release) {
# DESTDIR = ../mapper_app/debug
#} else {
# DESTDIR = ../mapper_app/release
#}
