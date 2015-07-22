#include <movit/mix_effect.h>
#include "vfx/glmix.h"



GLMix::GLMix( QString id, QString name ) : GLFilter( id, name )
{
	mix = addParameter( "mix", tr("Mix:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, true );
	mix->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 1 ) );
	mix->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
	mix->hidden = true;
}



bool GLMix::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( first );
	Q_UNUSED( second );
	Q_UNUSED( p );
	float m = getParamValue( mix, pts ).toFloat();
	return el[0]->set_float( "strength_first", m )
		&& el[0]->set_float( "strength_second", 1.0f - m );
}



QList<Effect*> GLMix::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MixEffect() );
	return list;
}
