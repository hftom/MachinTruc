QT += testlib
QT += opengl

TARGET = tests

TEMPLATE = app

SOURCES += main.cpp \
	testinputff.cpp

HEADERS += AutoTest.h \
	testinputff.h

LIBS = ../core/libcore.a

INCLUDEPATH = ../core

CONFIG += debug
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += movit
	PKGCONFIG += libavformat libavcodec libavutil libswresample
	PKGCONFIG += sdl
	PKGCONFIG += x11
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
