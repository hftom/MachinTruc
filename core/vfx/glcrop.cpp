#include <movit/padding_effect.h>
#include "vfx/glcrop.h"



GLCrop::GLCrop( QString id, QString name ) : GLFilter( id, name ),
	ptop( 0 ),
	pleft( 0 )
{
	left = addParameter( "left", tr("Left:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	right = addParameter( "right", tr("Right:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	top = addParameter( "top", tr("Top:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
	bottom = addParameter( "bottom", tr("Bottom:"), Parameter::PDOUBLE, 0.0, 0.0, 100.0, true, "%" );
}



GLCrop::~GLCrop()
{
}



void GLCrop::preProcess( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );

	pleft = getParamValue( left, pts ).toDouble() * src->glWidth / 100.0;
	pright = getParamValue( right, pts ).toDouble() * src->glWidth / 100.0;
	ptop = getParamValue( top, pts ).toDouble() * src->glHeight / 100.0;
	pbottom = getParamValue( bottom, pts ).toDouble() * src->glHeight / 100.0;
	src->glWidth = qMax( src->glWidth - pleft - pright, 1.0 );
	src->glHeight = qMax( src->glHeight - ptop - pbottom, 1.0 );
}



QString GLCrop::getDescriptor( double pts, Frame *src, Profile *p )
{
	preProcess( pts, src, p );
	return getIdentifier();
}



bool GLCrop::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	double glw = src->glWidth;
	double glh = src->glHeight;

	preProcess( pts, src, p );
	Effect *e = el[0];
	
	if ( src->glOVD ) {
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, (glw - src->glWidth) / 2.0 - pleft, (glh - src->glHeight) / 2.0 - ptop ) );
	}

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
