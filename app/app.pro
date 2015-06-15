TEMPLATE = app

QT += opengl
QT += xml

SOURCES = \
	main.cpp \
	gui/shadercollection.cpp \
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
	gui/filter/inputdouble.cpp \
	gui/filter/sliderdouble.cpp \
	gui/filter/sliderint.cpp \
	gui/filter/checkbox.cpp \
	gui/filter/colorchooser.cpp \
	gui/filter/filterwidget.cpp \
	gui/filter/colorwheel.cpp \
	gui/filter/textedit.cpp \
	gui/filter/shaderedit.cpp \
	\
	timeline/timeline.cpp \
	timeline/abstractviewitem.cpp \
	timeline/clipviewitem.cpp \
	timeline/transitionviewitem.cpp \
	timeline/clipeffectviewitem.cpp \
	timeline/cursorviewitem.cpp \
	timeline/trackviewitem.cpp \
	timeline/rulerviewitem.cpp \
	\
	animation/animeditor.cpp \
	animation/animscene.cpp \
	animation/animitem.cpp \
	animation/keyitem.cpp
	
HEADERS = \
	gui/shadercollection.h \
	gui/renderingdialog.h \
	gui/projectprofiledialog.h \
	gui/projectfile.h \
	gui/topwindow.h \
	gui/projectclipspage.h \
	gui/fxgraphicsview.h \
	gui/fxpage.h \
	gui/fxsettingspage.h \
	gui/profiledialog.h \
	gui/filtersdialog.h \
	gui/sourcelistwidget.h \
	gui/effectlistview.h \
	gui/graph.h \
	gui/grapheffectitem.h \
	\
	gui/filter/parameterwidget.h \
	gui/filter/inputdouble.h \
	gui/filter/sliderdouble.h \
	gui/filter/sliderint.h \
	gui/filter/checkbox.h \
	gui/filter/colorchooser.h \
	gui/filter/filterwidget.h \
	gui/filter/colorwheel.h \
	gui/filter/textedit.h \
	gui/filter/shaderedit.h \
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
	timeline/rulerviewitem.h \
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
	ui/animeditor.ui \
	ui/render.ui \
	ui/blankdialog.ui

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
