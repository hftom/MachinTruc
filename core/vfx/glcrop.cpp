#include <movit/padding_effect.h>
#include "vfx/glcrop.h"



GLCrop::GLCrop( QString id, QString name ) : GLFilter( id, name )
{
	left = addParameter( "left", tr("Left:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	right = addParameter( "right", tr("Right:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	top = addParameter( "top", tr("Top:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	bottom = addParameter( "bottom", tr("Bottom:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
}



GLCrop::~GLCrop()
{
}



void GLCrop::preProcess( Frame *src, Profile *p )
{
	Q_UNUSED( p );
	
	double pts = src->pts();
	pleft = getParamValue( left, pts ).toDouble() * src->glWidth / 100.0;
	double r = getParamValue( right, pts ).toDouble() * src->glWidth / 100.0;
	ptop = getParamValue( top, pts ).toDouble() * src->glHeight / 100.0;
	double b = getParamValue( bottom, pts ).toDouble() * src->glHeight / 100.0;
	src->glWidth = qMax( src->glWidth - pleft - r, 1.0 );
	src->glHeight = qMax( src->glHeight - ptop - b, 1.0 );
}



QString GLCrop::getDescriptor( Frame *src, Profile *p )
{
	preProcess( src, p );
	return getIdentifier();
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
