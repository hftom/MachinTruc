#include <movit/vignette_effect.h>
#include "vfx/glvignette.h"



GLVignette::GLVignette( QString id, QString name ) : GLFilter( id, name )
{
	radius = addParameter( "radius", tr("Radius:"), Parameter::PDOUBLE, 1.0, 0.0, 2.0, true );
	softness = addParameter( "softness", tr("Softness:"), Parameter::PDOUBLE, 0.01, 0.01, 0.5, false );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.0, -10000.0, 10000.0, true );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.0, -10000.0, 10000.0, true );
}



GLVignette::~GLVignette()
{
}



void GLVignette::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		QPointF pos = val.toPointF();
		if ( !xOffset->graph.keys.count() )
			xOffset->value = pos.x();
		if ( !yOffset->graph.keys.count() )
			yOffset->value = pos.y();
	}
}



bool GLVignette::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	float center[2] = { (getParamValue( xOffset, pts ).toFloat() / src->glWidth) + 0.5f, (getParamValue( yOffset, pts ).toFloat() / src->glHeight) + 0.5f };
	
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::TRANSLATE;
		src->glOVDRect = QRectF( -(double)src->glWidth / 2.0, -(double)src->glHeight / 2.0, src->glWidth, src->glHeight );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, getParamValue( xOffset, pts ).toDouble(), getParamValue( yOffset, pts ).toDouble() ) );
	}
	
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
