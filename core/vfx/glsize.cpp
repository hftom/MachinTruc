#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include <movit/padding_effect.h>

#include "glsize.h"



GLSize::GLSize( QString id, QString name ) : GLFilter( id, name ),
	resizeActive( true ),
	rotateActive( false ),
	rotateLeft( 0 ),
	rotateTop( 0 ),
	left( 0 ),
	top( 0 )
{
	sizePercent = addParameter( "sizePercent", tr("Size:"), Parameter::PDOUBLE, 100.0, 0.0, 500.0, true, "%" );
	xOffsetPercent = addParameter( "xOffsetPercent", tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yOffsetPercent = addParameter( "yOffsetPercent", tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	rotateAngle = addParameter( "rotateAngle", tr("Rotation angle:"), Parameter::PDOUBLE, 0.0, -180.0, 180.0, true );
	softBorder = addParameter( "softBorder", tr("Soft border:"), Parameter::PINT, 2.0, 0, 10, false );
}



GLSize::~GLSize()
{
}



void GLSize::preProcessResize( Frame *src, Profile *p )
{
	double pc = getParamValue( sizePercent, src->pts() ).toDouble();
	src->glWidth = (double)src->glWidth * src->glSAR  / p->getVideoSAR() * pc / 100.0;
	if ( src->glWidth < 1 )
		src->glWidth = 1;
	src->glHeight = (double)src->glHeight * pc / 100.0;
	if ( src->glHeight < 1 )
		src->glHeight = 1;
	src->glSAR = p->getVideoSAR();
}



void GLSize::preProcessRotate( Frame *src, Profile *p )
{
	Q_UNUSED( p );
	int rotateSize = sqrt( src->glWidth * src->glWidth + src->glHeight * src->glHeight ) + 1;
	rotateLeft = (rotateSize - src->glWidth) / 2.0;
	rotateTop = (rotateSize - src->glHeight) / 2.0;
	src->glWidth = rotateSize;
	src->glHeight = rotateSize;
}



void GLSize::preProcessPadding( Frame *src, Profile *p )
{
	left = (p->getVideoWidth() - src->glWidth) / 2.0;
	top = (p->getVideoHeight() - src->glHeight) / 2.0;
	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
	double pts = src->pts();
	left += getParamValue( xOffsetPercent, pts ).toDouble() * src->glWidth / 100.0;
	top += getParamValue( yOffsetPercent, pts ).toDouble() * src->glHeight / 100.0;
}



QString GLSize::getDescriptor( Frame *src, Profile *p )
{
	QString s;
	bool samesar = qAbs( p->getVideoSAR() - src->glSAR ) < 1e-3;

	if ( samesar && !sizePercent->graph.keys.count() && 100.0 == getParamValue( sizePercent ).toDouble() ) {
		resizeActive = false;
	}
	else {
		preProcessResize( src, p );
		s += "Resize";
		resizeActive = true;
	}
	
	if ( !rotateAngle->graph.keys.count() && getParamValue( rotateAngle ).toDouble() == 0.0 )
		rotateActive = false;
	else {
		preProcessRotate( src, p );
		s += "Rotate";
		rotateActive = true;
	}

	preProcessPadding( src, p );
	return s + "Padding";
}



bool GLSize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	bool ok = true;
	int index = 0;
	
	if ( resizeActive ) {
		Effect *e = el[0];
		preProcessResize( src, p );
		ok = e->set_int( "width", src->glWidth )
			&& e->set_int( "height", src->glHeight );
		++index;
	}
	
	if ( rotateActive ) {
		ok |= el[index]->set_float( "borderSize", getParamValue( softBorder ).toInt() );
		++index;
		
		Effect *e = el[index];
		preProcessRotate( src, p );
		ok |= e->set_int( "width", src->glWidth )
			&& e->set_int( "height", src->glHeight )
			&& e->set_float( "top", rotateTop )
			&& e->set_float( "left", rotateLeft );
		++index;
		
		ok |= el[index]->set_float( "angle", getParamValue( rotateAngle, src->pts() ).toDouble() * M_PI / 180.0 )
			&& el[index]->set_float( "SAR", src->glSAR );
		++index;
	}
	
	preProcessPadding( src, p );
	Effect *e = el[index];
	return ok && e->set_int( "width", src->glWidth )
		&& e->set_int( "height", src->glHeight )
		&& e->set_float( "top", top )
		&& e->set_float( "left", left );
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if ( resizeActive )
		list.append( new ResampleEffect() );
	if ( rotateActive ) {
		list.append( new MySoftBorderEffect() );
		list.append( new PaddingEffect() );
		list.append( new MyRotateEffect() );
	}
	list.append( new PaddingEffect() );

	return list;
}
