#-------------------------------------------------
#
# Project created by QtCreator 2011-10-23T13:25:51
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): {
 greaterThan(QT_MINOR_VERSION, 4): {
  WEBENGINE = Y
 }
}
message(WEBENGINE  " = "  $$WEBENGINE)

isEmpty(WEBENGINE) {
 QT       += core gui  webkit network sql xml
 greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport webkitwidgets gui
 message("not  using WebEngine")
}
else {
 QT       += core gui  network sql xml widgets websockets webchannel
 greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport webenginewidgets gui
 DEFINES += USE_WEBENGINE
 message("using WebEngine")
HEADERS += \
    websocketclientwrapper.h \
    websockettransport.h

SOURCES += \
    websocketclientwrapper.cpp \
    websockettransport.cpp
}

DEFINES += "BUILD_DIR=\"\\\""$$OUT_PWD"\\\"\""

TARGET = mapper
TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR = obj
RCC_DIR = ui
UI_DIR = ui
UI_HEADERS_DIR = ui
UI_SOURCES_DIR = ui
SUBDIRS += ui
    console #WebView

CONFIG += embed_manifest_exe

#DEFINES += MYPREFIX_RELEASE
#debug{
#  DEFINES += MYPREFIX_DEBUG
#  DEFINES -= MYPREFIX_RELEASE
#}

include(mapper_app.pri)

OTHER_FILES += \
    Resources/GoogleMaps.htm \
    Resources/sqlite3_create_tables.sql \
    Resources/arrow-right-double.png \
    Resources/gtk-add.png \
    Resources/list-add.png \
    Resources/arrow-left-double.png \
    Resources/edit-cut.png \
    Resources/go-next.png \
    Resources/format-remove-node.png \
    Resources/arrow-right.png \
    Resources/go-first-view.png \
    Resources/go-last.png \
    Resources/go-previous.png \
    Resources/arrow-left.png \
    Resources/edit-delete.png \
    Resources/go-last-view.png \
    Resources/gtk-delete.png \
    Resources/go-first.png \
    Resources/green00.png \
    Resources/red00.png \
    Resources/yellowblank.png \
    Resources/redblank.png \
    Resources/greenblank.png \
    Resources/blue-red-blank.png \
    Resources/blueblank.png \
    Resources/GoogleMaps.js \
    Resources/S-bahn.PNG \
    Resources/trolley-icon_78.gif \
    Resources/diropen.png \
    wiki/index.htm \
    Resources/recreateAltRoute.sql \
    Resources/CreateMySqlFunction.sql \
    Resources/recreateStationTable.sql \
    Resources/recreateTractionTypes.sql \
    Resources/recreateCompanies.sql

RESOURCES += \
    Resources/mapper.qrc

win32:{
#INCLUDEPATH += \
#    C:/Program Files (x86)/SQLite3
}



#win32:CONFIG(release, debug|release): LIBS += -L'c:/Program Files (x86)/Sqlite3//' -lsqlite3
#else:win32:CONFIG(debug, debug|release): LIBS += -L'c:/Program Files (x86)/Sqlite3//' -lsqlite3
#else:unix:!macx: LIBS += -lsqlite3

#win32: {
#INCLUDEPATH += 'c:/Program Files (x86)/Sqlite3/source'
#DEPENDPATH += 'c:/Program Files (x86)/Sqlite3/source'
#}

DISTFILES += \
    Resources/addBiDirectionalToRoutes.sql \
    Resources/recreateRouteComments.sql \
    Resources/scripts \
    Resources/scripts/qwebchannel.js \
    Resources/GoogleMaps2.htm \
    Resources/GoogleMaps2b.htm \
    Resources/updateOneWay.sql \
    Resources/white.png \
    Resources/orange.png \
    Resources/tram.png \
    Resources/tram.shadow.png

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/release/ -lConsole
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/debug/ -lConsole
else:unix: LIBS += -L$$PWD/../console/ -lConsole

INCLUDEPATH += $$PWD/../console
DEPENDPATH += $$PWD/../console

#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/release/ -lWebView
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/debug/ -lWebView
#else:unix: LIBS += -L$$OUT_PWD/WebView/ -lWebView

#INCLUDEPATH += $$PWD/WebView
#DEPENDPATH += $$PWD/WebView


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./release/ -lConsole
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./debug/ -lConsole

INCLUDEPATH += $$PWD/../console
DEPENDPATH += $$PWD/../console


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3
else:unix: LIBS += -L$$PWD/'../../../../Program Files/Sqlite3_3_13/' -lsqlite3

INCLUDEPATH += $$PWD/'../../../../Program Files/Sqlite3_3_13/src'
DEPENDPATH += $$PWD/'../../../../Program Files/Sqlite3_3_13/src'

FORMS += \
    addgeoreferenceddialog.ui

HEADERS += \
    addgeoreferenceddialog.h

SOURCES += \
    addgeoreferenceddialog.cpp

