#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include "vfx/glresize.h"



GLResize::GLResize( QString id, QString name ) : GLFilter( id, name )
{
	percent = addParameter( tr("Size:"), Parameter::PDOUBLE, 100.0, 0.0, 500.0, true, "%" );
}



GLResize::~GLResize()
{
}



bool GLResize::preProcess( Frame *src, Profile *p )
{
	double pc = getParamValue( percent, src->pts() ).toDouble();
	src->glWidth = (double)src->glWidth * src->glSAR  / p->getVideoSAR() * pc / 100.0;
	if ( src->glWidth < 1 )
		src->glWidth = 1;
	src->glHeight = (double)src->glHeight * pc / 100.0;
	if ( src->glHeight < 1 )
		src->glHeight = 1;
	src->glSAR = p->getVideoSAR();
	src->resizeAuto = false;
	
	return true;
}



bool GLResize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	preProcess( src, p );
	
	return el.at(0)->set_int( "width", src->glWidth )
		&& el.at(0)->set_int( "height", src->glHeight );
}



QList<Effect*> GLResize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	return list;
}
