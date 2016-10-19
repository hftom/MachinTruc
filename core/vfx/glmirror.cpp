#include "vfx/glmirror.h"



GLMirror::GLMirror( QString id, QString name ) : GLFilter( id, name )
{
}



bool GLMirror::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( el );
	Q_UNUSED( pts );
	Q_UNUSED( p );
	Q_UNUSED( src );
	return true;
}



QList<Effect*> GLMirror::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MMirrorEffect() );
	return list;
}
