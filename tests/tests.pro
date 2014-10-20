QT += testlib
QT += opengl

TARGET = tests

TEMPLATE = app

SOURCES += main.cpp \
	testinputff.cpp

HEADERS += AutoTest.h \
	testinputff.h

LIBS += ../build/core/libcore.a
INCLUDEPATH += ../core
DEPENDPATH += ../core
PRE_TARGETDEPS += ../build/core/libcore.a

CONFIG += debug
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += movit
	PKGCONFIG += libavformat libavcodec libavutil libswresample libswscale
	PKGCONFIG += sdl
	PKGCONFIG += x11
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
