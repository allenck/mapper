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
build_pass:CONFIG(debug, debug|release) {
 DESTDIR = ../mapper_app/debug
} else {
 DESTDIR = ../mapper_app/release
}


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3

INCLUDEPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'
DEPENDPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'


unix:!macx: LIBS += -L$$PWD/../../../sqlite-amalgamation-3390400/ -lsqlite3

INCLUDEPATH += $$PWD/../../../sqlite-amalgamation-3390400
DEPENDPATH += $$PWD/../../../sqlite-amalgamation-3390400
