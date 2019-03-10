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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Release/release/ -lhisyntax
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Debug/debug/ -lhisyntax
else:unix: LIBS += -L$$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_GCC_64bit-Debug/ -lhisyntax

INCLUDEPATH += $$PWD/../../hisyntax_temp/src
DEPENDPATH += $$PWD/../../hisyntax_temp/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Release/release/libhisyntax.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Debug/debug/libhisyntax.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Debug/release/hisyntax.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Debug/debug/hisyntax.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../hisyntax_temp/build-hisyntax-Desktop_Qt_5_12_0_GCC_64bit-Debug/libhisyntax.a

unix|win32: LIBS += -L$$PWD/../../DCPCryptCpp/ -ldcpcrypt

INCLUDEPATH += $$PWD/../../DCPCryptCpp
DEPENDPATH += $$PWD/../../DCPCryptCpp

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../DCPCryptCpp/dcpcrypt.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../DCPCryptCpp/libdcpcrypt.a
