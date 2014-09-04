SOURCES = \
	engine/bufferpool.cpp \
	engine/util.cpp \
	engine/parameter.cpp \
	engine/filter.cpp \
	engine/filtercollection.cpp \
	engine/profile.cpp \
	engine/frame.cpp \
	engine/source.cpp \
	engine/cut.cpp \
	engine/clip.cpp \
	engine/scene.cpp \
	engine/sampler.cpp \
	engine/composer.cpp \
	engine/track.cpp \
	engine/metronom.cpp \
	engine/glresource.cpp \
	engine/movitchain.cpp \
	\
	input/input_ff.cpp \
	input/input_gl.cpp \
	input/input_image.cpp \
	\
	audioout/ao_sdl.cpp \
	\
	vfx/gltest.cpp \
	vfx/glmix.cpp \
	vfx/gloverlay.cpp \
	vfx/glsaturation.cpp \
	vfx/glvignette.cpp \
	vfx/glblur.cpp \
	vfx/glglow.cpp \
	vfx/gldeconvolutionsharpen.cpp \
	vfx/glwater.cpp \
	vfx/glliftgammagain.cpp \
	vfx/glresize.cpp \
	vfx/glpadding.cpp \
	vfx/glcrop.cpp \
	vfx/gldeinterlace.cpp \
	vfx/glblurmask.cpp \
	vfx/glcut.cpp \
	vfx/gledge.cpp \
	vfx/glopacity.cpp \
	vfx/glsharpen.cpp \
	vfx/gldiffusion.cpp \
	vfx/glsize.cpp \
	vfx/gldropshadow.cpp \
	vfx/glpixelize.cpp \
	vfx/glsoftborder.cpp \
	vfx/glborder.cpp \
	vfx/glbackgroundcolor.cpp \
	\
	videoout/videowidget.cpp

HEADERS = \
	engine/bufferpool.h \
	engine/filter.h \
	engine/filtercollection.h \
	engine/profile.h \
	engine/frame.h \
	engine/source.h \
	engine/cut.h \
	engine/clip.h \
	engine/scene.h \
	engine/sampler.h \
	engine/composer.h \
	engine/track.h \
	engine/metronom.h \
	engine/glresource.h \
	engine/movitchain.h \
	\
	input/input.h \
	input/input_ff.h \
	input/input_gl.h \
	input/input_image.h \
	\
	audioout/ao_sdl.h \
	\
	afx/audiofilter.h \
	afx/audiocopy.h \
	afx/audiomix.h \
	afx/audiovolume.h \
	afx/audiocomposition.h \
	\
	vfx/gltest.h \
	vfx/glfilter.h \
	vfx/glcomposition.h \
	vfx/glmix.h \
	vfx/gloverlay.h \
	vfx/glsaturation.h \
	vfx/glvignette.h \
	vfx/glblur.h \
	vfx/glglow.h \
	vfx/gldeconvolutionsharpen.h \
	vfx/glwater.h \
	vfx/glliftgammagain.h \
	vfx/glresize.h \
	vfx/glpadding.h \
	vfx/glcrop.h \
	vfx/gldeinterlace.h \
	vfx/glblurmask.h \
	vfx/glcut.h \
	vfx/gledge.h \
	vfx/glopacity.h \
	vfx/glsharpen.h \
	vfx/gldiffusion.h \
	vfx/glsize.h \
	vfx/gldropshadow.h \
	vfx/glpixelize.h \
	vfx/glsoftborder.h \
	vfx/glborder.h \
	vfx/glbackgroundcolor.h \
	\
	videoout/videowidget.h

TEMPLATE = lib

QT += opengl

CONFIG += staticlib debug
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += movit
	PKGCONFIG += libavformat libavcodec libavutil libswresample
	PKGCONFIG += sdl
	PKGCONFIG += x11
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
