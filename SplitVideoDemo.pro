#-------------------------------------------------
#
# Project created by QtCreator 2016-11-25T16:06:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SplitVideoDemo
TEMPLATE = app


LIBS += -L\ -lWS2_32
INCLUDEPATH +=  ffmpeg/include
LIBS += ffmpeg\lib\libavcodec.dll.a \
        ffmpeg/lib/libavfilter.dll.a \
        ffmpeg/lib/libavformat.dll.a \
        ffmpeg/lib/libswscale.dll.a \
        ffmpeg/lib/libavutil.dll.a \

SOURCES += main.cpp\
        mainwindow.cpp \
    SplitVideo.cpp

HEADERS  += mainwindow.h \
    SplitVideo.h

FORMS    += mainwindow.ui
