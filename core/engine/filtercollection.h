#ifndef FILTERCOLLECTION_H
#define FILTERCOLLECTION_H

// video filters
#include "vfx/gltest.h"
#include "vfx/glblur.h"
#include "vfx/glcut.h"
#include "vfx/gldeconvolutionsharpen.h"
#include "vfx/gldeinterlace.h"
#include "vfx/gldiffusion.h"
#include "vfx/gledge.h"
#include "vfx/glglow.h"
#include "vfx/glliftgammagain.h"
#include "vfx/glopacity.h"
#include "vfx/glcrop.h"
#include "vfx/glresize.h"
#include "vfx/glsaturation.h"
#include "vfx/glsharpen.h"
#include "vfx/glvignette.h"
#include "vfx/glwater.h"
#include "vfx/glsize.h"
#include "vfx/gldropshadow.h"
#include "vfx/glpixelize.h"
#include "vfx/glsoftborder.h"
#include "vfx/glborder.h"
#include "vfx/glbackgroundcolor.h"

// video compositions
#include "vfx/glmix.h"
#include "vfx/gloverlay.h"

// audio filters
#include "afx/audiovolume.h"

// audio compositions
#include "afx/audiocopy.h"
#include "afx/audiomix.h"
#include "afx/audiocrossfade.h"



template <class T>
class Maker
{
public:
	static Filter* make( QString id, QString name ) { return new T( id, name ); }
};



class FilterEntry
{
public:
	FilterEntry( QString id, QString n, Filter* (*p)(QString,QString) )
		: identifier( id ),
		name( n ),
		makeFilter( p )
	{}
	
	QSharedPointer<Filter> create() { return QSharedPointer<Filter>( makeFilter( identifier, name ) ); }

	QString identifier; // must be unique
	QString name; // translated UI name

private:
	Filter* (*makeFilter)(QString,QString);
};



class FilterCollection
{
public:
	FilterCollection();
	static FilterCollection* getGlobalInstance();
	
	QList<FilterEntry> videoFilters;
	QList<FilterEntry> videoTransitions;
	QList<FilterEntry> audioFilters;
	QList<FilterEntry> audioTransitions;
};

#endif // FILTERCOLLECTION_H
