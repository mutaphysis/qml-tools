#-------------------------------------------------
#
# Project created by QtCreator 2013-03-09T16:50:21
#
#-------------------------------------------------

TEMPLATE = app
TARGET = tst_jslint

QT       += testlib v8 qml quick
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle

DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES += tst_jslint.cpp

include(../../src/src.pri)
include(../../qml-privates.pri)

