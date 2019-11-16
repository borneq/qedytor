QT += core
QT += widgets
CONFIG += c++17
requires(qtConfig(listwidget))

include(../SingleApplication/qtsingleapplication.pri)

HEADERS       += \
    crypt/blockcipher.h \
    crypt/blockcipher128.h \
    crypt/cipher.h \
    crypt/exception.h \
    crypt/hash.h \
    crypt/md5.h \
    crypt/rijndael.h \
    crypt/rijndael_tabs.h \
    crypt/sha3.h \
    crypt/util.h \
    codeeditor.h \
    een.h \
    config.h \
    edytorexception.h \
    configdialog.h \
    mainwindow.h \
    searchdialog.h \
    tabcontrolable.h \
    infowindow.h \
    properties.h \
    tablemodel.h \
    configitem.h \
    codeeditorsidebar.h \
    configtableview.h
SOURCES       += main.cpp \
    crypt/blockcipher.cpp \
    crypt/blockcipher128.cpp \
    crypt/cipher.cpp \
    crypt/md5.cpp \
    crypt/rijndael.cpp \
    crypt/sha3.cpp \
    crypt/util.cpp \
    codeeditor.cpp \
    een.cpp \
    config.cpp \
    edytorexception.cpp \
    configdialog.cpp \
    mainwindow.cpp \
    infowindow.cpp \
    searchdialog.cpp \
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

FORMS += \
    searchdialog.ui

