QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    testffmpeg.cpp

HEADERS += \
    mainwindow.h \
    testffmpeg.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix{
    INCLUDEPATH += $$PWD/3rdlib/linux/ffmpeg/include
    INCLUDEPATH += $$PWD/3rdlib/linux/sdl3/include

    LIBS += $$PWD/3rdlib/linux/ffmpeg/lib/libavformat.so   \
            $$PWD/3rdlib/linux/ffmpeg/lib/libavcodec.so    \
            $$PWD/3rdlib/linux/ffmpeg/lib/libavdevice.so   \
            $$PWD/3rdlib/linux/ffmpeg/lib/libavfilter.so   \
            $$PWD/3rdlib/linux/ffmpeg/lib/libavutil.so     \
            $$PWD/3rdlib/linux/ffmpeg/lib/libpostproc.so   \
            $$PWD/3rdlib/linux/ffmpeg/lib/libswresample.so \
            $$PWD/3rdlib/linux/ffmpeg/lib/libswscale.so    \
            $$PWD/3rdlib/linux/sdl3/lib/libSDL3.so \
            $$PWD/3rdlib/linux/sdl3/lib/libSDL3_test.a

    # opengl
    LIBS += /usr/lib/x86_64-linux-gnu/libEGL.so \
            /usr/lib/x86_64-linux-gnu/libGL.so
}
