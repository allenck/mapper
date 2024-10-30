TEMPLATE = lib
TARGET = sqlfun

SOURCES += \
    sqlite3ext_sqlfun.c

HEADERS += \
    sqlite3ext.h \
    sqlite3.h


build_pass:CONFIG(debug, debug|release) {
 DESTDIR = ../mapper_app/debug
} else {
 DESTDIR = ../mapper_app/release
}
