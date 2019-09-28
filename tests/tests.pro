QT += testlib
QT += opengl

TARGET = tests

TEMPLATE = app

SOURCES += main.cpp \
	testinputff.cpp \
	testoutputff.cpp

HEADERS += AutoTest.h \
	testinputff.h \
	testoutputff.h

LIBS += ../build/core/libcore.a
INCLUDEPATH += ../core
DEPENDPATH += ../core
PRE_TARGETDEPS += ../build/core/libcore.a

CONFIG += c++11
CONFIG += debug
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += movit
	PKGCONFIG += libavformat libavcodec libavutil libswresample libswscale libavfilter
	PKGCONFIG += sdl2
	PKGCONFIG += x11
	
	QMAKE_CXXFLAGS += -fopenmp
	QMAKE_CFLAGS += -fopenmp
	QMAKE_LFLAGS += -fopenmp
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
