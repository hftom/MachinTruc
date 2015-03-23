TEMPLATE = app

QT += opengl
QT += xml

SOURCES = \
	main.cpp \
	gui/renderingdialog.cpp \
	gui/projectprofiledialog.cpp \
	gui/projectfile.cpp \
	gui/topwindow.cpp \
	gui/projectclipspage.cpp \
	gui/fxpage.cpp \
	gui/fxsettingspage.cpp \
	gui/filtersdialog.cpp \
	gui/graph.cpp \
	gui/grapheffectitem.cpp \
	\
	gui/filter/headereffect.cpp \
	gui/filter/sliderdouble.cpp \
	gui/filter/sliderint.cpp \
	gui/filter/checkbox.cpp \
	gui/filter/colorchooser.cpp \
	gui/filter/filterwidget.cpp \
	\
	timeline/timeline.cpp \
	timeline/abstractviewitem.cpp \
	timeline/clipviewitem.cpp \
	timeline/transitionviewitem.cpp \
	timeline/clipeffectviewitem.cpp \
	timeline/cursorviewitem.cpp \
	timeline/trackviewitem.cpp \
	\
	animation/animeditor.cpp \
	animation/animscene.cpp \
	animation/animitem.cpp \
	animation/keyitem.cpp
	
HEADERS = \
	gui/renderingdialog.h \
	gui/projectprofiledialog.h \
	gui/projectfile.h \
	gui/topwindow.h \
	gui/projectclipspage.h \
	gui/fxpage.h \
	gui/fxsettingspage.h \
	gui/profiledialog.h \
	gui/filtersdialog.h \
	gui/sourcelistwidget.h \
	gui/effectlistwidget.h \
	gui/effectlistview.h \
	gui/graph.h \
	gui/grapheffectitem.h \
	\
	gui/filter/parameterwidget.h \
	gui/filter/headereffect.h \
	gui/filter/sliderdouble.h \
	gui/filter/sliderint.h \
	gui/filter/checkbox.h \
	gui/filter/colorchooser.h \
	gui/filter/filterwidget.h \
	\
	timeline/timelinegraphicsview.h \
	timeline/timeline.h \
	timeline/typeitem.h \
	timeline/abstractviewitem.h \
	timeline/clipviewitem.h \
	timeline/transitionviewitem.h \
	timeline/clipeffectviewitem.h \
	timeline/cursorviewitem.h \
	timeline/trackviewitem.h \
	\
	animation/animeditor.h \
	animation/animscene.h \
	animation/animitem.h \
	animation/keyitem.h
	
FORMS = \
	ui/mainwindow.ui \
	ui/projectprofile.ui \
	ui/projectclipspage.ui \
	ui/fxpage.ui \
	ui/fxsettingspage.ui \
	ui/profile.ui \
	ui/filters.ui \
	ui/effectheader.ui \
	ui/animeditor.ui \
	ui/render.ui

TARGET = ../machintruc

LIBS += ../core/libcore.a
INCLUDEPATH += ../core
DEPENDPATH += ../core
PRE_TARGETDEPS += ../core/libcore.a

RESOURCES = resources.qrc

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
