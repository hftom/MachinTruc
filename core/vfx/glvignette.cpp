#include <movit/vignette_effect.h>
#include "vfx/glvignette.h"



GLVignette::GLVignette( QString id, QString name ) : GLFilter( id, name )
{
	radius = addParameter( "radius", tr("Radius:"), Parameter::PDOUBLE, 1.0, 0.0, 2.0, true );
	softness = addParameter( "softness", tr("Softness:"), Parameter::PDOUBLE, 0.5, 0.01, 1.0, false );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
}



GLVignette::~GLVignette()
{
}



void GLVignette::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		ovdOffset = val;
	}
}



bool GLVignette::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	
	if (!ovdOffset.isNull()) {
		QPointF pos = ovdOffset.toPointF();
		if ( !xOffset->graph.keys.count() )
			xOffset->value = pos.x() * 100.0 / src->glWidth;
		if ( !yOffset->graph.keys.count() )
			yOffset->value = pos.y() * 100.0 / src->glHeight;
		
		ovdOffset = QVariant();
	}
	
	double xof = src->glWidth * getParamValue( xOffset, pts ).toDouble() / 100.0;
	double yof = src->glHeight * getParamValue( yOffset, pts ).toDouble() / 100.0;
	
	float center[2] = { ((float)xof / src->glWidth) + 0.5f, ((float)yof / src->glHeight) + 0.5f };
	
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::TRANSLATE;
		src->glOVDRect = QRectF( -(double)src->glWidth / 2.0, -(double)src->glHeight / 2.0, src->glWidth, src->glHeight );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, xof, yof ) );
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
