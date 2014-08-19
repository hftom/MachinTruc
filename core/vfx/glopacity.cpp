#include <movit/multiply_effect.h>
#include "vfx/glopacity.h"



GLOpacity::GLOpacity( QString id, QString name ) : GLFilter( id, name )
{
	factor = addParameter( tr("Opacity:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, true );
}



GLOpacity::~GLOpacity()
{
}



bool GLOpacity::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	float f = getParamValue( factor, src->pts() ).toFloat();
	float col[4] = { f, f, f, f };
	return el.at(0)->set_vec4( "factor", col );
}



QList<Effect*> GLOpacity::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MultiplyEffect() );
	return list;
}
