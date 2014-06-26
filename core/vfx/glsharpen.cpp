#include <movit/unsharp_mask_effect.h>
#include "vfx/glsharpen.h"



GLSharpen::GLSharpen( QString id, QString name ) : GLFilter( id, name )
{
	amount = 0.3;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 2.0, true, &amount );
}



GLSharpen::~GLSharpen()
{
}



bool GLSharpen::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "amount", amount );
}



QList<Effect*> GLSharpen::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new UnsharpMaskEffect() );
	return list;
}
