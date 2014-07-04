#include <movit/blur_effect.h>
#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLFilter( id, name )
{
	amount = 1.0f;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 10.0, true, &amount );
}



GLBlur::~GLBlur()
{
}



bool GLBlur::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "radius", src->glWidth * amount / 100.0f );
}



QList <Effect*> GLBlur::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new BlurEffect() );
	return list;
}
