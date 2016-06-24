#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T20:26:36
#
#-------------------------------------------------

QT       += core gui
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pic5000
TEMPLATE = app

qtHaveModule(printsupport): QT += printsupport
qtHaveModule(opengl): QT += opengl

SOURCES += main.cpp\
        mainwindow.cpp \
    udp_pic5000.c \
    view.cpp \
    camerathread.cpp \
    settingdialog.cpp \
    tranfertorgb.cpp    \
    #mtig_host.c \
    control_config.c

HEADERS  += mainwindow.h \
    udp_pic5000.h \
    view.h \
    camerathread.h \
    settingdialog.h \
    tranfertorgb.h \
    raw_pic5000.h \
    control_config.h \
    mtig_host.h

FORMS    += mainwindow.ui \
    settingdialog.ui

message("compiling...")

win32-mingw* {
INCLUDEPATH += C:\winpcap\WpdPack\Include
LIBS += -LC:\winpcap\WpdPack\Lib -lwpcap \
 -LC:\work\tiff-3.8.2\mingw -lport -ltiff -lws2_32
INCLUDEPATH += c:\work\tiff-3.8.2\libtiff
}

win32-msvc*{
message("win32 vc")
INCLUDEPATH += C:\WpdPack\Include
LIBS += -LC:\WpdPack\Lib\x64 -lwpcap -lPacket

INCLUDEPATH += C:\jansson-2.7-build\include
LIBS += -LC:\jansson-2.7-build\lib -ljansson_d

INCLUDEPATH += C:\pthread\Pre-built.2\include
LIBS += -LC:\pthread\Pre-built.2\lib\x64 -lpthreadVC2

}


mac* {
message("macos clang env")
#INCLUDEPATH += /home/lmice/work/lib/libpcap-1.5.3
#LIBS += -L/home/lmice/work/lib/libpcap-1.5.3 -lpcap
INCLUDEPATH += ../lib/jansson/osx/include
LIBS += -L../lib/jansson/osx/lib -ljansson
LIBS += -lpcap -lncurses
}

OTHER_FILES += \
    Makefile

TRANSLATIONS += pic5000.ts

RESOURCES += \
    pic5000.qrc
