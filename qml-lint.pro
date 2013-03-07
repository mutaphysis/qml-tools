#-------------------------------------------------
#
# Project created by QtCreator 2012-04-12T11:02:28
#
#-------------------------------------------------

QT       += core v8 qml quick
QT       -= gui

TARGET = qml_parser
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
SOURCES += main.cpp
OTHER_FILES += test/Test.qml


# preconfigured qml parser sources
isEmpty(QT5_DECLARATIVE_PARSER_PATH) {
    QT5_DECLARATIVE_PARSER_PATH = $$[QT5_DECLARATIVE_PARSER_PATH]
}

# take qml parser from source
isEmpty(QT5_DECLARATIVE_PARSER_PATH) {
    # determine source path
    isEmpty(QT_INSTALL_SOURCE) {
        QT_INSTALL_SOURCE = $$[QT_INSTALL_SOURCE]
    }
    isEmpty(QT_INSTALL_SOURCE) {
        QT_INSTALL_SOURCE = $$[QT_INSTALL_PREFIX]/../Src
    }

    QT5_DECLARATIVE_PARSER_PATH = $$QT_INSTALL_SOURCE/qtdeclarative/src/qml/qml/parser
}

message(Using qml parser from $$QT5_DECLARATIVE_PARSER_PATH)

INCLUDEPATH += $$QT5_DECLARATIVE_PARSER_PATH

include($$QT5_DECLARATIVE_PARSER_PATH/parser.pri)


