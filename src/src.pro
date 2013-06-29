#-------------------------------------------------
#
# Project created by QtCreator 2012-04-12T11:02:28
#
#-------------------------------------------------

QT       += core v8 qml quick
QT       -= gui

TARGET = qml-tools

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
SOURCES += main.cpp

LIBS += -lboost_program_options-mt


include(src.pri)
include(../qml-privates.pri)


