#include <movit/multiply_effect.h>
#include "vfx/glopacity.h"



GLOpacity::GLOpacity( QString id, QString name ) : GLFilter( id, name )
{
	factor = 0.5;
	addParameter( tr("Opacity:"), PFLOAT, 0.0, 1.0, true, &factor );
}



GLOpacity::~GLOpacity()
{
}



bool GLOpacity::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	float col[4] = { factor, factor, factor, factor };
	return el.at(0)->set_vec4( "factor", col );
}



QList<Effect*> GLOpacity::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MultiplyEffect() );
	return list;
}
