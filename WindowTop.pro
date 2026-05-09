QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = WindowTopManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    processmanager.cpp

HEADERS += \
    mainwindow.h \
    processmanager.h

LIBS += -luser32 -lkernel32 -lpsapi -ladvapi32

RESOURCES += resources.qrc
