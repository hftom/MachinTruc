#include <movit/mix_effect.h>
#include "vfx/glmix.h"



GLMix::GLMix() : GLComposition()
{
	compositionName = "MixEffect";
	strength_first = 0.5;
	strength_second = 0.5;
}



GLMix::~GLMix()
{
}



bool GLMix::process( Effect *e, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( dst );
	Q_UNUSED( p );
	return e->set_float( "strength_first", strength_first )
		&& e->set_float( "strength_second", strength_second );
}



Effect* GLMix::getMovitEffect()
{
	return new MixEffect();
}
