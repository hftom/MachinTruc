SOURCES = \
	vidstab/boxblur.c \
	vidstab/frameinfo.c \
	vidstab/vidstab.c \
	vidstab/localmotion2transform.c \
	vidstab/motiondetect.c \
	vidstab/motiondetect_opt.c \
	vidstab/serialize.c \
	vidstab/transform.c \
	vidstab/transformfixedpoint.c \
	#vidstab/transformfloat.c \
	vidstab/transformtype.c \
	vidstab/vsvector.c \
	\
	engine/bufferpool.cpp \
	engine/util.cpp \
	engine/parameter.cpp \
	engine/filter.cpp \
	engine/filtercollection.cpp \
	engine/stabilizecollection.cpp \
	engine/profile.cpp \
	engine/frame.cpp \
	engine/source.cpp \
	engine/cut.cpp \
	engine/clip.cpp \
	engine/sampler.cpp \
	engine/scene.cpp \
	engine/composer.cpp \
	engine/track.cpp \
	engine/metronom.cpp \
	engine/glresource.cpp \
	engine/movitchain.cpp \
	engine/transition.cpp \
	engine/thumbnailer.cpp \
	engine/playbackbuffer.cpp \
	\
	input/ffdecoder.cpp \
	input/input_ff.cpp \
	input/input_image.cpp \
	input/input_blank.cpp \
	\
	output/common_ff.cpp \
	output/output_ff.cpp \
	\
	audioout/ao_sdl.cpp \
	\
	vfx/movitbackground.cpp \
	vfx/gltest.cpp \
	vfx/glmix.cpp \
	vfx/glpush.cpp \
	vfx/glcover.cpp \
	vfx/glfrostedglass.cpp \
	vfx/glhardcut.cpp \
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
	vfx/glorientation.cpp \
	vfx/glwhitebalance.cpp \
	vfx/gldefish.cpp \
	vfx/glfiber.cpp \
	vfx/glnoise.cpp \
	vfx/gltext.cpp \
	vfx/glcustom.cpp \
	vfx/gldenoise.cpp \
	vfx/glstabilize.cpp \
	vfx/glfadeoutin.cpp \
	vfx/glzoomin.cpp \
	\
	videoout/videowidget.cpp

HEADERS = \
	engine/bufferpool.h \
	engine/filter.h \
	engine/filtercollection.h \
	engine/stabilizecollection.h \
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
	engine/transition.h \
	engine/thumbnailer.h \
	engine/playbackbuffer.h \
	\
	input/input.h \
	input/ffdecoder.h \
	input/input_ff.h \
	input/input_image.h \
	input/input_blank.h \
	\
	output/common_ff.h \
	output/output_ff.h \
	\
	audioout/ao_sdl.h \
	\
	afx/audiofilter.h \
	afx/audiocopy.h \
	afx/audiomix.h \
	afx/audiovolume.h \
	afx/audiocrossfade.h \
	afx/audiohardcut.h \
	\
	vfx/movitbackground.h \
	vfx/gltest.h \
	vfx/glfilter.h \
	vfx/glmix.h \
	vfx/glpush.h \
	vfx/glcover.h \
	vfx/glfrostedglass.h \
	vfx/glhardcut.h \
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
	vfx/glorientation.h \
	vfx/glwhitebalance.h \
	vfx/gldefish.h \
	vfx/glfiber.h \
	vfx/glnoise.h \
	vfx/gltext.h \
	vfx/glcustom.h \
	vfx/gldenoise.h \
	vfx/glstabilize.h \
	vfx/glfadeoutin.h \
	vfx/glzoomin.h \
	\
	videoout/videowidget.h

TEMPLATE = lib

QT += opengl
QT += concurrent

CONFIG += staticlib debug
unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += movit
	PKGCONFIG += libavformat libavcodec libavutil libswresample libswscale libavfilter
	#PKGCONFIG += vidstab
	PKGCONFIG += sdl
	PKGCONFIG += x11
}

# ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
