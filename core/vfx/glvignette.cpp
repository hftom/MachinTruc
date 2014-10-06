#include <movit/vignette_effect.h>
#include "vfx/glvignette.h"



GLVignette::GLVignette( QString id, QString name ) : GLFilter( id, name )
{
	radius = addParameter( "radius", tr("Radius:"), Parameter::PDOUBLE, 1.0, 0.0, 2.0, true );
	softness = addParameter( "softness", tr("Softness:"), Parameter::PDOUBLE, 0.01, 0.01, 0.5, false );
	centerX = addParameter( "centerX", tr("X center:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, true );
	centerY = addParameter( "centerY", tr("Y center:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, true );
}



GLVignette::~GLVignette()
{
}



bool GLVignette::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	float center[2] = { getParamValue( centerX, pts ).toFloat(), getParamValue( centerY, pts ).toFloat() };
	Effect *e = el[0];
	return e->set_vec2( "center", center )
		&& e->set_float( "radius", getParamValue( softness ).toFloat() )
		&& e->set_float( "inner_radius", getParamValue( radius, pts ).toFloat() - 0.5 );
}



QList<Effect*> GLVignette::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new VignetteEffect() );
	return list;
}
