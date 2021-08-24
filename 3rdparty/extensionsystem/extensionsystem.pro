QT += core gui

TARGET = extensionsystem
TEMPLATE = lib

DEFINES += EXTENSIONSYSTEM_LIBRARY

CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
} else {
    LIBS += -L$$OUT_PWD/../utils/release/ -lutils
}

INCLUDEPATH += $$PWD/../

HEADERS += \
    invoker.h \
    iplugin.h \
    iplugin_p.h \
    extensionsystem_global.h \
    pluginmanager.h \
    pluginmanager_p.h \
    pluginspec.h \
    pluginspec_p.h \
    optionsparser.h

SOURCES += \
    invoker.cpp \
    iplugin.cpp \
    pluginmanager.cpp \
    pluginspec.cpp \
    optionsparser.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
