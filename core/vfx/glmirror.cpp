#include "vfx/glmirror.h"



GLMirror::GLMirror( QString id, QString name ) : GLFilter( id, name )
{
	horizontal = addBooleanParameter( "horizontal", tr("Horizontal"), 1 );
	vertical = addBooleanParameter( "vertical", tr("Vertical"), 0 );
}



bool GLMirror::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );
	Q_UNUSED( src );
	return el.at(0)->set_float("horizontal", getParamValue(horizontal).toInt())
		&& el.at(0)->set_float("vertical", getParamValue(vertical).toInt());
}



QList<Effect*> GLMirror::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MMirrorEffect() );
	return list;
}
