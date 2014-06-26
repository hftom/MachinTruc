TEMPLATE = app

QT += opengl

SOURCES = \
	main.cpp \
	gui/topwindow.cpp \
	gui/projectclipspage.cpp \
	gui/fxpage.cpp \
	gui/fxsettingspage.cpp \
	gui/filtersdialog.cpp \
	gui/headereffect.cpp \
	\
	timeline/timeline.cpp \
	timeline/typerectitem.cpp \
	timeline/abstractviewitem.cpp \
	timeline/clipviewitem.cpp \
	timeline/cursorviewitem.cpp \
	timeline/trackviewitem.cpp
	
HEADERS = \
	gui/topwindow.h \
	gui/projectclipspage.h \
	gui/fxpage.h \
	gui/fxsettingspage.h \
	gui/profiledialog.h \
	gui/filtersdialog.h \
	gui/headereffect.h \
	\
	timeline/timelinegraphicsview.h \
	timeline/timeline.h \
	timeline/typerectitem.h \
	timeline/abstractviewitem.h \
	timeline/clipviewitem.h \
	timeline/cursorviewitem.h \
	timeline/trackviewitem.h
	
FORMS = \
	ui/mainwindow.ui \
	ui/projectclipspage.ui \
	ui/fxpage.ui \
	ui/fxsettingspage.ui \
	ui/profile.ui \
	ui/filters.ui \
	ui/effectheader.ui

LIBS = ../core/libcore.a

TARGET = ../machintruc

PRE_TARGETDEPS += ../core/libcore.a

INCLUDEPATH = ../core

RESOURCES = resources.qrc

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
