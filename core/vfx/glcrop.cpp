#include <movit/padding_effect.h>
#include "vfx/glcrop.h"



GLCrop::GLCrop( QString id, QString name ) : GLFilter( id, name )
{
	left = 0.0;
	right = 0.0;
	top = 0.0;
	bottom = 0.0;
	addParameter( tr("Left:"), PFLOAT, 0.0, 100.0, true, &left );
	addParameter( tr("Right:"), PFLOAT, 0.0, 100.0, true, &right );
	addParameter( tr("Top:"), PFLOAT, 0.0, 100.0, true, &top );
	addParameter( tr("Bottom:"), PFLOAT, 0.0, 100.0, true, &bottom );
}



GLCrop::~GLCrop()
{
}



bool GLCrop::preProcess( Frame *src, Profile *p )
{
	pleft = left * src->glWidth / 100.0f;
	float r = right * src->glWidth / 100.0f;
	ptop = top * src->glHeight / 100.0f;
	float b = bottom * src->glHeight / 100.0f;
	src->glWidth = qMax( src->glWidth - pleft - r, 1.0f );
	src->glHeight = qMax( src->glHeight - ptop - b, 1.0f );
	return true;
}



bool GLCrop::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	preProcess( src, p );
	return el.at(0)->set_int( "width", src->glWidth )
		&& el.at(0)->set_int( "height", src->glHeight )
		&& el.at(0)->set_float( "top", -ptop )
		&& el.at(0)->set_float( "left", -pleft );
}



QList<Effect*> GLCrop::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PaddingEffect() );
	return list;
}
