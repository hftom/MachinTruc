TEMPLATE = app

QT += opengl
QT += xml

SOURCES = \
	main.cpp \
	DarkStyle.cpp \
	gui/appconfig.cpp \
	gui/shadercollection.cpp \
	gui/renderingdialog.cpp \
	gui/projectprofiledialog.cpp \
	gui/projectfile.cpp \
	gui/xmlizer.cpp \
	gui/clipboard.cpp \
	gui/topwindow.cpp \
	gui/projectclipspage.cpp \
	gui/fxpage.cpp \
	gui/fxsettingspage.cpp \
	gui/filtersdialog.cpp \
	gui/graph.cpp \
	gui/grapheffectitem.cpp \
	gui/addclipsdialog.cpp \
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
	gui/filter/statustext.cpp \
	gui/filter/combobox.cpp \
	\
	timeline/timeline.cpp \
	timeline/abstractviewitem.cpp \
	timeline/clipviewitem.cpp \
	timeline/transitionviewitem.cpp \
	timeline/clipeffectviewitem.cpp \
	timeline/cursorviewitem.cpp \
	timeline/trackviewitem.cpp \
	timeline/rulerdock.cpp \
	timeline/rulerviewitem.cpp \
	timeline/selectwindowitem.cpp \
	\
	animation/animeditor.cpp \
	animation/animscene.cpp \
	animation/animitem.cpp \
	animation/keyitem.cpp
	
HEADERS = \
	undo.h \
	DarkStyle.h \
	gui/appconfig.h \
	gui/shadercollection.h \
	gui/renderingdialog.h \
	gui/projectprofiledialog.h \
	gui/projectfile.h \
	gui/xmlizer.h \
	gui/clipboard.h \
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
	gui/addclipsdialog.h \
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
	gui/filter/statustext.h \
	gui/filter/combobox.h \
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
	timeline/rulerdock.h \
	timeline/rulerviewitem.h \
	timeline/selectwindowitem.h \
	\
	timeline/undoclipadd.h\
	timeline/undoclipmove.h\
	timeline/undoclipremove.h\
	timeline/undoclipresize.h\
	timeline/undoclipspeed.h\
	timeline/undoclipsplit.h\
	timeline/undoeffectadd.h\
	timeline/undoeffectmove.h\
	timeline/undoeffectparam.h\
	timeline/undoeffectremove.h\
	timeline/undoeffectreorder.h\
	timeline/undoeffectresize.h\
	timeline/undotrackadd.h\
	timeline/undotransitionchanged.h\
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
	ui/addclips.ui

TARGET = ../machintruc

INCLUDEPATH += ../core
DEPENDPATH += ../core

unix {
	LIBS += ../core/libcore.a
	PRE_TARGETDEPS += ../core/libcore.a
	RESOURCES = resources_unix.qrc
}

win32 {
	LIBS += ../core/release/libcore.a
	PRE_TARGETDEPS += ../core/release/libcore.a
	RC_ICONS += umovit.ico
	RESOURCES = resources_win.qrc
}

CONFIG += c++11
CONFIG += debug

CONFIG += link_pkgconfig
PKGCONFIG += movit
PKGCONFIG += libavformat libavcodec libavutil libswresample libswscale libavfilter
PKGCONFIG += sdl2
PKGCONFIG += libexif
QMAKE_CXXFLAGS += -fopenmp
QMAKE_CFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

unix {
	PKGCONFIG += x11
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
