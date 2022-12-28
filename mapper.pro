#-------------------------------------------------
#
# Project created by QtCreator 2011-10-23T13:25:51
#
#-------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

QT += xml

SUBDIRS =  functions sqlfun \
    #mapper_app/WebView
    mapper_app

DEPENDPATH += . mapper_app
INCLUDEPATH += . mapper_app

