QT += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

LIBS += -lcfitsio

INCLUDEPATH += \
    fits/include \
    gui/include

SOURCES += \
    fits/src/fitsexception.cpp \
    fits/src/fitsimage.cpp \
    fits/src/fitsraster.cpp \
    fits/src/fitstantrum.cpp \
    gui/src/main.cpp \
    gui/src/mainwindow.cpp \
    gui/src/fitswidget.cpp \
    gui/src/stretch.cpp

HEADERS += \
    fits/include/fitsexception.h \
    fits/include/fitsimage.h \
    fits/include/fitsraster.h \
    fits/include/fitstantrum.h \
    gui/include/mainwindow.h \
    gui/include/fitswidget.h \
    gui/include/stretch.h

RESOURCES += \
    icon/icon.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target





