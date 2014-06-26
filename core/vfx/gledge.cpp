#include "vfx/gledge.h"



GLEdge::GLEdge( QString id, QString name ) : GLFilter( id, name )
{
	amp = 2.0;
	depth = 0.7;
	opacity = 1.0;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 10.0, true, &amp );
	addParameter( tr("Depth:"), PFLOAT, 0.0, 1.0, true, &depth );
	addParameter( tr("Opacity:"), PFLOAT, 0.0, 1.0, true, &opacity );
}



GLEdge::~GLEdge()
{
}



bool GLEdge::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el.at(0)->set_float( "height", src->glHeight )
		&& el.at(0)->set_float( "width", src->glWidth )
		&& el.at(0)->set_float( "amp", amp )
		&& el.at(0)->set_float( "depth", depth )
		&& el.at(0)->set_float( "opacity", opacity );
}



QList<Effect*> GLEdge::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new EdgeEffect() );
	return list;
}







