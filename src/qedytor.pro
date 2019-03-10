QT += core
QT += widgets
CONFIG += c++14
requires(qtConfig(listwidget))

include(../SingleApplication/qtsingleapplication.pri)

HEADERS       += \
    tabwindow.h \
    codeeditor.h \
    een.h \
    config.h \
    edytorexception.h \
    configdialog.h \
    tabcontrolable.h \
    searchdialog.h \
    infowindow.h \
    properties.h \
    tablemodel.h \
    configitem.h \
    codeeditorsidebar.h \
    configtableview.h
SOURCES       += main.cpp \
    tabwindow.cpp \
    codeeditor.cpp \
    een.cpp \
    config.cpp \
    edytorexception.cpp \
    configdialog.cpp \
    searchdialog.cpp \
    infowindow.cpp \
    tablemodel.cpp \
    codeeditorsidebar.cpp \
    configtableview.cpp

DEFINES += plugin_ctrltab

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/ -lhisyntax
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/ -lhisyntax
else:unix: LIBS += -L$$PWD/../bin/ -lhisyntax

INCLUDEPATH += $$PWD/../hisyntax/src
DEPENDPATH += $$PWD/../hisyntax/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../bin/libhisyntax.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../bin/libhisyntax.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../bin/hisyntax.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../bin/hisyntax.lib
else:unix: PRE_TARGETDEPS += $$PWD/../bin/libhisyntax.a

unix|win32: LIBS += -L$$PWD/../bin/ -ldcpcrypt

INCLUDEPATH += $$PWD/../DCPCryptCpp
DEPENDPATH += $$PWD/../DCPCryptCpp

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../bin/dcpcrypt.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../bin/libdcpcrypt.a
