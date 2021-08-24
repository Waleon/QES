shared {
    DEFINES += UTILS_LIBRARY
} else {
    DEFINES += QTCREATOR_UTILS_STATIC_LIB
}

!win32:{
    isEmpty(IDE_LIBEXEC_PATH) | isEmpty(IDE_BIN_PATH): {
        warning("using utils-lib.pri without IDE_LIBEXEC_PATH or IDE_BIN_PATH results in empty QTC_REL_TOOLS_PATH")
        DEFINES += QTC_REL_TOOLS_PATH=$$shell_quote(\"\")
    } else {
        DEFINES += QTC_REL_TOOLS_PATH=$$shell_quote(\"$$relative_path($$IDE_LIBEXEC_PATH, $$IDE_BIN_PATH)\")
    }
}

QT += core gui

TARGET = utils
TEMPLATE = lib

CONFIG += exceptions # used by portlist.cpp, textfileformat.cpp, and ssh/*

win32: LIBS += -luser32 -lshell32
# PortsGatherer
win32: LIBS += -liphlpapi -lws2_32

INCLUDEPATH += \
    $$PWD/ \
    $$PWD/../ \
    $$PWD/../../

SOURCES += \
    $$PWD/benchmarker.cpp \
    $$PWD/stringutils.cpp \
    $$PWD/synchronousprocess.cpp \
    $$PWD/qtcassert.cpp \
    $$PWD/hostosinfo.cpp

HEADERS += \
    $$PWD/benchmarker.h \
    $$PWD/utils_global.h \
    $$PWD/stringutils.h \
    $$PWD/synchronousprocess.h \
    $$PWD/executeondestruction.h \
    $$PWD/qtcassert.h \
    $$PWD/hostosinfo.h \
    $$PWD/algorithm.h \
    $$PWD/optional.h \
    $$PWD/../3rdparty/optional/optional.hpp \
    $$PWD/../3rdparty/variant/variant.hpp \
    $$PWD/predicates.h

include(mimetypes/mimetypes.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
