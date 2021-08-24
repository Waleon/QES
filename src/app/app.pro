QT += core gui

CONFIG += c++11 console
TEMPLATE  = app

INCLUDEPATH += \
    $$PWD/ \
    $$PWD/../ \
    $$PWD/../../3rdparty/ \
    $$PWD/../plugins/

CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../../3rdparty/extensionsystem/debug/ -lextensionsystem
} else {
    LIBS += -L$$OUT_PWD/../../3rdparty/extensionsystem/release/ -lextensionsystem
}

SOURCES += \
    main.cpp
