QT += core gui network printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    promptwindow.cpp \
    registrowindow.cpp

HEADERS += \
    mainwindow.h \
    promptwindow.h \
    registrowindow.h

FORMS += \
    mainwindow.ui \
    promptwindow.ui \
    registrowindow.ui

RESOURCES += \
    resources.qrc

