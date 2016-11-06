#ifndef FILTERCOLLECTION_H
#define FILTERCOLLECTION_H

// auto filters
#include "vfx/glpadding.h"
#include "vfx/glresize.h"
#include "vfx/glorientation.h"

// video filters
#include "vfx/glbackgroundcolor.h"
#include "vfx/glblur.h"
#include "vfx/glborder.h"
#include "vfx/glcontrast.h"
#include "vfx/glcrop.h"
#include "vfx/glcustom.h"
#include "vfx/glcut.h"
#include "vfx/gldeconvolutionsharpen.h"
#include "vfx/gldenoise.h"
#include "vfx/gldiffusion.h"
#include "vfx/gldefish.h"
#include "vfx/gldropshadow.h"
#include "vfx/gledge.h"
#include "vfx/glfiber.h"
#include "vfx/glglow.h"
#include "vfx/glkaleidoscope.h"
#include "vfx/glliftgammagain.h"
#include "vfx/glmirror.h"
#include "vfx/glopacity.h"
#include "vfx/glpixelize.h"
#include "vfx/glsaturation.h"
#include "vfx/glsharpen.h"
#include "vfx/glsize.h"
#include "vfx/glsoftborder.h"
#include "vfx/glstabilize.h"
#include "vfx/gltext.h"
#include "vfx/glvignette.h"
#include "vfx/glwater.h"
#include "vfx/glwhitebalance.h"
#include "vfx/gltest.h"

// video compositions
#include "vfx/glmix.h"
#include "vfx/glpush.h"
#include "vfx/glcover.h"
#include "vfx/glfadeoutin.h"
#include "vfx/glfrostedglass.h"
#include "vfx/glhardcut.h"
#include "vfx/gloverlay.h"
#include "vfx/glzoomin.h"

// audio filters
#include "afx/audiovolume.h"

// audio compositions
#include "afx/audiocopy.h"
#include "afx/audiomix.h"
#include "afx/audiocrossfade.h"
#include "afx/audiohardcut.h"



template <class T>
class Maker
{
public:
	static Filter* make( QString id, QString name ) { return new T( id, name ); }
};



class FilterEntry
{
public:
	FilterEntry( QString ic, QString id, QString n, Filter* (*p)(QString,QString) )
		: identifier( id ),
		name( n ),
		icon( ic ),
		makeFilter( p )
	{}
	
	QSharedPointer<Filter> create() { return QSharedPointer<Filter>( makeFilter( identifier, name ) ); }

	QString identifier; // must be unique
	QString name; // translated UI name
	QString icon;

private:
	Filter* (*makeFilter)(QString,QString);
};



class FilterCollection
{
public:
	static FilterCollection* getGlobalInstance();
	
	QList<FilterEntry> videoFilters;
	QList<FilterEntry> sourceVideoFilters;
	QList<FilterEntry> videoTransitions;
	QList<FilterEntry> audioFilters;
	QList<FilterEntry> sourceAudioFilters;
	QList<FilterEntry> audioTransitions;
	
private:
	FilterCollection();
	~FilterCollection() {}
	
	static FilterCollection globalInstance;
};

#endif // FILTERCOLLECTION_H
