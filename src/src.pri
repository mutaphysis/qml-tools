
SOURCES += \
    $$PWD/jslint.cpp \
    $$PWD/jsinstrument.cpp \
    $$PWD/scriptcollector.cpp \
    $$PWD/qmlinstrumenttask.cpp


HEADERS += \
    $$PWD/jslint.h \
    $$PWD/jsinstrument.h \
    $$PWD/scriptcollector.h \
    $$PWD/qmlinstrumenttask.h

OTHER_FILES += \
    $$PWD/../externals/escodegen/escodegen.browser.js \
    $$PWD/../externals/esprima/esprima.js \
    $$PWD/../externals/istanbul/instrumenter.js \
    $$PWD/../externals/jslint/jslint.js


RESOURCES += \
    $$PWD/../externals/externals.qrc

INCLUDEPATH += $$PWD/
