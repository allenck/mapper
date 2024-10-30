#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T15:06:32
#
#-------------------------------------------------
QT += core widgets gui

TARGET = Console
TEMPLATE = lib

DEFINES += CONSOLE_LIBRARY
DEFINES += QT_MESSAGELOGCONTEXT

SOURCES += \
    consoleinterface.cpp \
    systemconsole.cpp \
    systemconsoleaction.cpp \
    jtextarea.cpp \
    flowlayout.cpp \
    jtogglebutton.cpp

HEADERS +=\
    console_global.h \
    consoleinterface.h \
    systemconsole.h \
    systemconsoleaction.h \
    jtextarea.h \
    flowlayout.h \
    jtogglebutton.h \
    preferencespanel.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32: {
 build_pass:CONFIG(debug, debug|release) {
  DESTDIR = ../mapper_app/debug
 } else {
  DESTDIR = ../mapper_app/release
 }
}
unix:macx: {
 build_pass:CONFIG(debug, debug|release) {
  DESTDIR = ../mapper_app
 } else {
 DESTDIR = ../mapper_app
 }
}
