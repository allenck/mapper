#-------------------------------------------------
#
# Project created by QtCreator 2011-10-23T13:25:51
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

QT += xml

SUBDIRS =  functions \
    sqlfun \
#    sqlite3 \
    mapper_app

DEPENDPATH += . mapper_app
INCLUDEPATH += . mapper_app

# enable NO_UDF to not use Sqlite User Defined functions
#DEFINES += NO_UDF

# enable to provide a console window to display debug and info messages

