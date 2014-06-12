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



bool GLEdge::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return e->set_float( "height", src->glHeight )
		&& e->set_float( "width", src->glWidth )
		&& e->set_float( "amp", amp )
		&& e->set_float( "depth", depth )
		&& e->set_float( "opacity", opacity );
}



Effect* GLEdge::getMovitEffect()
{
	return new EdgeEffect();
}







