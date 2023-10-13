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

#isEmpty(WEBENGINE) {
# QT       += core gui  webkit network sql xml
# greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport webkitwidgets gui
# message("not  using WebEngine")
#}
#else {
 QT       += core gui  network sql xml widgets websockets webchannel
 greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport webenginewidgets gui
 DEFINES += USE_WEBENGINE
 message("using WebEngine")

RC_ICONS = Resources/tram-icon.ico

#}

DEFINES += "BUILD_DIR=\"\\\""$$OUT_PWD"\\\"\""
DEFINES += QT_MESSAGELOGCONTEXT

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
    Resources/sql/sqlite3_create_tables.sql \
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
    Resources/sql/recreateAltRoute.sql \
    Resources/sql/CreateMySqlFunction.sql \
    Resources/sql/recreateStationTable.sql \
    Resources/sql/recreateTractionTypes.sql \
    Resources/sql/recreateCompanies.sql

RESOURCES += \
    Resources/mapper.qrc \
    Resources/mapper.qrc \
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
    README.txt \
    Resources/scripts/createMsSqlDistance.sql \
    Resources/sql/addBiDirectionalToRoutes.sql \
    Resources/sql/recreateRouteComments.sql \
    Resources/sql/recreateSegmentsTable.sql \
    Resources/sql/recreate_routes.sql \
    Resources/scripts \
    Resources/scripts/opacity-slider2.png \
    Resources/scripts/opacity-slider3d14.png \
    Resources/scripts/opacity-slider3d6.png \
    Resources/scripts/opacity-slider3d7.png \
    Resources/scripts/qwebchannel.js \
    Resources/GoogleMaps2.htm \
    Resources/GoogleMaps2b.htm \
    Resources/sql/updateOneWay.sql \
    Resources/sql/updaterouteoneway.sql \
    Resources/white.png \
    Resources/orange.png \
    Resources/tram.png \
    Resources/tram.shadow.png \
    html/.gitignore


#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/release/ -lWebView
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/debug/ -lWebView
#else:unix: LIBS += -L$$OUT_PWD/WebView/ -lWebView

#INCLUDEPATH += $$PWD/WebView
#DEPENDPATH += $$PWD/WebView

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../console/release/ -lConsole
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../console/debug/ -lConsole
#else:unix: LIBS += -L$$PWD/../console/ -lConsole

#INCLUDEPATH += $$PWD/../console/debug
#DEPENDPATH += $$PWD/../console/debug

#unix:!macx: LIBS += -L$$PWD/./ -lConsole

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3

INCLUDEPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'
DEPENDPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'


unix:!macx: LIBS += -L$$PWD/../../../sqlite-amalgamation-3390400/ -lsqlite3

INCLUDEPATH += $$PWD/../../../sqlite-amalgamation-3390400
DEPENDPATH += $$PWD/../../../sqlite-amalgamation-3390400


#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/release/ -lConsole
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/debug/ -lConsole
#else:unix: LIBS += -L$$OUT_PWD/../console/ -lConsole

INCLUDEPATH += $$PWD/debug
DEPENDPATH += $$PWD/debug

