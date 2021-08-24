QT += core gui

TEMPLATE = lib
CONFIG += plugin

CONFIG(debug, debug|release){
    DESTDIR = $$OUT_PWD/../../app/debug/plugins/
    LIBS += -L$$OUT_PWD/../../../3rdparty/extensionsystem/debug/ -lextensionsystem
} else {
    DESTDIR = $$OUT_PWD/../../app/debug/release/
    LIBS += -L$$OUT_PWD../../../3rdparty/extensionsystem/release/ -lextensionsystem
}

INCLUDEPATH += $$PWD/../../../3rdparty/

HEADERS += \
    coreplugin.h

SOURCES += \
    coreplugin.cpp

DISTFILES += coreplugin.json
