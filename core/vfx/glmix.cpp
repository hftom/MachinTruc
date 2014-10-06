#include <movit/mix_effect.h>
#include "vfx/glmix.h"



GLMix::GLMix( QString id, QString name ) : GLFilter( id, name )
{
	mix = addParameter( "mix", tr("Mix:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, true );
	mix->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 1 ) );
	mix->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
}



bool GLMix::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	float m = getParamValue( mix, src->pts() ).toFloat();
	return el[0]->set_float( "strength_first", m )
		&& el[0]->set_float( "strength_second", 1.0f - m );
}



QList<Effect*> GLMix::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MixEffect() );
	return list;
}
