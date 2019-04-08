#-------------------------------------------------
#
# Project created by QtCreator 2016-02-06T11:55:53
#
#-------------------------------------------------

#QT       -= core gui

TARGET = functions
TEMPLATE = lib

build_pass:CONFIG(debug, debug|release) {
 DESTDIR = ../mapper_app
} else {
 DESTDIR = ../mapper_app
}

DEFINES += FUNCTIONS_LIBRARY

SOURCES += $$PWD/functions.cpp \
    $$PWD//distance.c

HEADERS += $$PWD/functions.h\
        $$PWD/functions_global.h \

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3
else:unix: LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3

INCLUDEPATH += $$PWD/'../../../../Program Files/Sqlite3_3_13/src'
DEPENDPATH += $$PWD/'../../../../Program Files/Sqlite3_3_13/src'

build_pass:CONFIG(debug, debug|release) {
 DESTDIR = ../mapper_app/debug
} else {
 DESTDIR = ../mapper_app/release
}
