
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

 QT       += core gui  network sql xml widgets websockets webchannel
 greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport webenginewidgets gui
 DEFINES += USE_WEBENGINE
 message("using WebEngine")

RC_ICONS = Resources/tram-icon.ico

CONFIG += c++17

DEFINES += "BUILD_DIR=\"\\\""$$OUT_PWD"\\\"\""
DEFINES += QT_MESSAGELOGCONTEXT
# enable NO_UDF to not use Sqlite User Defined functions
#DEFINES += NO_UDF

# enable to provide a console window to display debug and info messages
DEFINES += HAVE_CONSOLE
debug {
  DEFINES += MYPREFIX_DEBUG
}
release {
  DEFINES += MYPREFIX_RELEASE
}
TARGET = mapper
TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR = obj
RCC_DIR = ui
UI_DIR = ui
UI_HEADERS_DIR = ui
UI_SOURCES_DIR = ui

CONFIG += embed_manifest_exe
CONFIG+=use_gold_linker

include(mapper_app.pri)

OTHER_FILES += \
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
    Resources/sql/MySql/CreateMySqlFunction.sql \
    Resources/sql/recreateStationTable.sql \
    Resources/sql/recreateTractionTypes.sql \
    Resources/sql/recreateCompanies.sql \
    Resources/mapper.icns

RESOURCES += \
#    Resources/mapper.qrc \
#    Resources/mapper.qrc \
    Resources/mapper.qrc


DISTFILES += \
    README.txt \
    # Resources/GoogleMaps2n.htm \
    # Resources/copyList.txt \
    Resources/databases/.gitignore \
    Resources/dump_databases.cmd \
    Resources/dump_databases.sh \
    # Resources/initMap.js \
    Resources/restore_databases.cmd \
    Resources/restore_databases.sh \
    # Resources/scripts/createMsSqlDistance.sql \
    Resources/sql/CreatePostgreSQLFunction.sql \
    Resources/sql/PostgreSQL/.gitignore \
    Resources/sql/PostgreSQL/PostgreSQL_create_distance_function.sql \
    Resources/sql/mysql_create_stations.sql \
    Resources/sql/sqlite3_create_route.sql \
    Resources/sql/addBiDirectionalToRoutes.sql \
    Resources/sql/create_stations.sql \
    Resources/sql/mssql_create_routes.sql \
    Resources/sql/mssql_create_routeseq.sql \
    Resources/sql/mssql_create_stations.sql \
    Resources/sql/mssql_recreate_routes.sql \
    Resources/sql/mysql_create_routes.sql \
    Resources/sql/mysql_recreate_routes.sql \
    Resources/sql/recreateRouteComments.sql \
    Resources/sql/recreateSegmentsTable.sql \
    Resources/sql/recreate_routes.sql \
    # Resources/scripts \
    # Resources/scripts/opacity-slider2.png \
    # Resources/scripts/opacity-slider3d14.png \
    # Resources/scripts/opacity-slider3d6.png \
    # Resources/scripts/opacity-slider3d7.png \
    # Resources/scripts/qwebchannel.js \
    Resources/GoogleMaps2b.htm \
    Resources/sql/sqlite3_recreateCompanies.sql \
    Resources/sql/sqlite3_recreate_routes.sql \
    Resources/sql/updateOneWay.sql \
    Resources/sql/updaterouteoneway.sql \
    # Resources/test.htm \
    Resources/white.png \
    Resources/orange.png \
    Resources/tram.png \
    Resources/tram.shadow.png \
    Resources/wiki/Credits.html \
    # compile_MySql_plugin.sh \
    html/.gitignore \
    Resources/mapper.icns
    # mysql-qt-driver.sh

ICON = Resources/mapper.icns
#CONFIG-=app_bundle

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../console/release/ -lConsole
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../console/debug/ -lConsole
#else:unix: LIBS += -L$$PWD/../console/ -lConsole

#INCLUDEPATH += $$PWD/../console/debug
#DEPENDPATH += $$PWD/../console/debug

#unix:macx: LIBS += -L$$PWD/./ -lConsole

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400/' -lsqlite3

#INCLUDEPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'
#DEPENDPATH += $$PWD/'../../../../Downloads/sqlite-dll-win64-x64-3390400'


#unix:!macx: LIBS += -L$$PWD/../../../sqlite-amalgamation-3390400/ -lsqlite3

INCLUDEPATH += $$(QTDIR)/../Src/qtbase/src/3rdparty/sqlite


VERSION=1.2.2
MY_VERSION_STR = '\\"$${VERSION}\\"'
DEFINES += MY_VERSION=\"$${MY_VERSION_STR}\"
macx: {
# copy necessary resources to MacOS mapper.app/Contents/
QMAKE_INFO_PLIST = $$PWD/myInfo.plist

APP_DB_FILES.files += $$PWD/Resources/databases/StLouis.sqlite3 \
        $$PWD/Resources/databases/berlinerstrassenbahn.sqlite3 \
        $$PWD/Resources/databases/indianapolis.sqlite3 \
        $$PWD/Resources/databases/louisville.sqlite3 \
        $$PWD/Resources/databases/cincinnati.sqlite3
APP_DB_FILES.path = Contents/Resources/databases
QMAKE_BUNDLE_DATA +=APP_DB_FILES

APP_WIKI_FILES.files = $$files($$PWD/Resources/wiki/*)
APP_WIKI_FILES.path = Contents/Resources/wiki
QMAKE_BUNDLE_DATA +=APP_WIKI_FILES

APP_WIKI_IMAGES.files = $$files($$PWD/Resources/wiki/images/*)
APP_WIKI_IMAGES.path = Contents/Resources/wiki/images
QMAKE_BUNDLE_DATA +=APP_WIKI_IMAGES

APP_RESOURCES.files += $$PWD/Resources/GoogleMaps.js \
      $$PWD/Resources/GoogleMaps.js \
      $$PWD/Resources/ExtDraggableObject.js \
      $$PWD/Resources/opacityControl.js \
      $$PWD/Resources/qwebchannel.js \
      $$PWD/Resources/WebChannel.js \
      $$PWD/Resources/GoogleMaps2b.htm \
      $$PWD/Resources/GoogleMaps2n.htm \
      $$PWD/Resources/apikey.js \
      $$PWD/Resources/opacity-slider2.png \
      $$PWD/Resources/S-Bahn-logo.svg \
      $$PWD/Resources/sl-metro-logo.svg \
      $$PWD/Resources/Strassenbahn-Haltestelle.svg \
      $$PWD/Resources/U-Bahn.svg \
      $$PWD/Resources/overlays.xml
APP_RESOURCES.path = Contents/Resources
QMAKE_BUNDLE_DATA +=APP_RESOURCES
}
include(sqlite.pri)

#INCLUDEPATH += $$PWD/../../../sqlite-amalgamation-3390400
#DEPENDPATH += $$PWD/../../../sqlite-amalgamation-3390400

INCLUDEPATH += $$PWD/debug
DEPENDPATH += $$PWD/debug


#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../sqlite3/release/ -lsqlite3
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../sqlite3/debug/ -lsqlite3
#else:unix: LIBS += -L$$OUT_PWD/../sqlite3/ -lsqlite3

#INCLUDEPATH += $$PWD/../sqlite3
#DEPENDPATH += $$PWD/../sqlite3

lessThan(QT_MAJOR_VERSION, 6): {
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/release/ -lsqlite3
}

# Please download the SQLite amalgamation corresponding to the version of QT that you are using from https://www.sqlite.org/download.html
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../sqlite-amalgamation-3460100/release/ -lsqlite3
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../sqlite-amalgamation-3460100/debug/ -lsqlite3
#else:unix:!macx: LIBS += -L$$PWD/../../../sqlite-amalgamation-3460100/ -lsqlite3

INCLUDEPATH += $$PWD/../../../sqlite-amalgamation-3460100
DEPENDPATH += $$PWD/../../../sqlite-amalgamation-3460100


#unix|win32: LIBS += -ldl
unix: LIBS += -ldl

FORMS += \
    dialogeditstreets.ui \
    dialogpreferences.ui \
    dialogupdatestreets.ui \
    findreplacewidget.ui

HEADERS += \
    dialogeditstreets.h \
    dialogpreferences.h \
    dialogupdatestreets.h \
    findreplacewidget.h

SOURCES += \
    dialogeditstreets.cpp \
    dialogpreferences.cpp \
    dialogupdatestreets.cpp \
    findreplacewidget.cpp
