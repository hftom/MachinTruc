#include <movit/padding_effect.h>
#include "vfx/glcrop.h"



GLCrop::GLCrop( QString id, QString name ) : GLFilter( id, name )
{
	left = addParameter( tr("Left:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	right = addParameter( tr("Right:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	top = addParameter( tr("Top:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	bottom = addParameter( tr("Bottom:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
}



GLCrop::~GLCrop()
{
}



bool GLCrop::preProcess( Frame *src, Profile *p )
{
	double pts = src->pts();
	pleft = getParamValue( left, pts ) * src->glWidth / 100.0;
	double r = getParamValue( right, pts ) * src->glWidth / 100.0;
	ptop = getParamValue( top, pts ) * src->glHeight / 100.0;
	double b = getParamValue( bottom, pts ) * src->glHeight / 100.0;
	src->glWidth = qMax( src->glWidth - pleft - r, 1.0 );
	src->glHeight = qMax( src->glHeight - ptop - b, 1.0 );
	return true;
}



bool GLCrop::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	preProcess( src, p );
	Effect *e = el[0];
	return e->set_int( "width", src->glWidth )
		&& e->set_int( "height", src->glHeight )
		&& e->set_float( "top", -ptop )
		&& e->set_float( "left", -pleft );
}



QList<Effect*> GLCrop::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PaddingEffect() );
	return list;
}
