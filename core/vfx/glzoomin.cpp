#include <movit/overlay_effect.h>
#include "vfx/glzoomin.h"



GLZoomIn::GLZoomIn( QString id, QString name ) : GLSize( id, name )
{
	sizePercent->hidden = true;
	xOffset->hidden = true;
	yOffset->hidden = true;
	rotateAngle->hidden = true;
	//softBorder->hidden = true;
	rotateStart = addParameter( "rotateStart", tr("Start angle:"), Parameter::PDOUBLE, 15.0, -360.0, 360.0, false );
	inverse = addBooleanParameter( "inverse", tr("Inverse"), 0 );
	sizePercent->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
	sizePercent->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 100.0 / sizePercent->max.toDouble() ) );
	rotateAngle->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, (getParamValue( rotateStart ).toDouble() / (rotateAngle->max.toDouble() - rotateAngle->min.toDouble())) ) );
	rotateAngle->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
}



GLZoomIn::~GLZoomIn()
{
}



QString GLZoomIn::getDescriptor( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2").arg( getIdentifier() ).arg( getParamValue( inverse ).toInt() );
}



QString GLZoomIn::getDescriptorFirst( double pts, Frame *f, Profile *p )
{
	if ( getParamValue( inverse ).toInt() )
		return GLSize::getDescriptor( pts, f, p );
	return "";
}



QString GLZoomIn::getDescriptorSecond( double pts, Frame *f, Profile *p )
{
	if ( getParamValue( inverse ).toInt() )
		return "";
	return GLSize::getDescriptor( pts, f, p );
}



bool GLZoomIn::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( el );
	
	if ( getParamValue( inverse ).toInt() ) {
		rotateAngle->graph.keys.last().y = getParamValue( rotateStart ).toDouble() / (rotateAngle->max.toDouble() - rotateAngle->min.toDouble());
		return GLSize::process( firstList, pts, first, p );
	}
	else {
		rotateAngle->graph.keys.first().y = getParamValue( rotateStart ).toDouble() / (rotateAngle->max.toDouble() - rotateAngle->min.toDouble());
		return GLSize::process( secondList, pts, second, p );
	}
}



QList<Effect*> GLZoomIn::getMovitEffects()
{
	QList<Effect*> list;
	Effect *e = new OverlayEffect();
	
	firstList.clear();
	secondList.clear();
	if ( getParamValue( inverse ).toInt() ) {
		sizePercent->graph.keys.first().y = 100.0 / sizePercent->max.toDouble();
		sizePercent->graph.keys.last().y = 0;
		rotateAngle->graph.keys.first().y = 0;
		rotateAngle->graph.keys.last().y = getParamValue( rotateStart ).toDouble() / (rotateAngle->max.toDouble() - rotateAngle->min.toDouble());
		firstList.append( GLSize::getMovitEffects() );
		e->set_int( "swap_inputs", 1 ) && true;
	}
	else {
		sizePercent->graph.keys.first().y = 0;
		sizePercent->graph.keys.last().y = 100.0 / sizePercent->max.toDouble();
		rotateAngle->graph.keys.first().y = getParamValue( rotateStart ).toDouble() / (rotateAngle->max.toDouble() - rotateAngle->min.toDouble());
		rotateAngle->graph.keys.last().y = 0;
		secondList.append( GLSize::getMovitEffects() );
	}
	
	list.append( e );
	return list;
}



QList<Effect*> GLZoomIn::getMovitEffectsFirst()
{
	return firstList;
}



QList<Effect*> GLZoomIn::getMovitEffectsSecond()
{
	return secondList;
}
