#-------------------------------------------------
#
# Project created by QtCreator 2011-10-23T13:25:51
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

QT += xml

SUBDIRS =  functions sqlfun \
    console \
    #mapper_app/WebView
    mapper_app
mapper_app.depends = console

DEPENDPATH += . mapper_app
INCLUDEPATH += . mapper_app

