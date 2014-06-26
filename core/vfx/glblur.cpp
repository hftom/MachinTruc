#include <movit/blur_effect.h>
#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLFilter( id, name )
{
	radius = 4.0;
	addParameter( tr("Radius:"), PFLOAT, 0.0, 100.0, true, &radius );
}



GLBlur::~GLBlur()
{
}



bool GLBlur::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "radius", radius );
}



QList <Effect*> GLBlur::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new BlurEffect() );
	return list;
}
