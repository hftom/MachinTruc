#include "vfx/gledge.h"



GLEdge::GLEdge( QString id, QString name ) : GLFilter( id, name )
{
	amp = 5.0f;
	depth = 1.0f;
	opacity = 1.0f;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 20.0, true, &amp );
	addParameter( tr("Depth:"), PFLOAT, 0.0, 1.0, true, &depth );
	addParameter( tr("Opacity:"), PFLOAT, 0.0, 1.0, true, &opacity );
}



GLEdge::~GLEdge()
{
}



bool GLEdge::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	return el.at(0)->set_float( "amp", amp )
		&& el.at(0)->set_float( "depth", 1.0f - depth )
		&& el.at(0)->set_float( "opacity", opacity );
}



QList<Effect*> GLEdge::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new EdgeEffect() );
	return list;
}
