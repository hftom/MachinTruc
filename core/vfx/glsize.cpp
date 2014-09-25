#include <movit/resample_effect.h>
#include <movit/padding_effect.h>

#include "glsize.h"



GLSize::GLSize( QString id, QString name ) : GLFilter( id, name )
{
	sizePercent = addParameter( tr("Size:"), Parameter::PDOUBLE, 100.0, 0.0, 500.0, true, "%" );
	xOffsetPercent = addParameter( tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yOffsetPercent = addParameter( tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	resizeActive = true;
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
		s = "GLResize";
		resizeActive = true;
	}

	preProcessPadding( src, p );
	return s +"GLPadding";
}



bool GLSize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Effect *ep = el[0];
	bool ok = true;
	
	if ( el.count() == 2 ) {
		ep = el[1];
		preProcessResize( src, p );
		ok = el[0]->set_int( "width", src->glWidth )
			&& el[0]->set_int( "height", src->glHeight );
	}
	
	preProcessPadding( src, p );
	return ep->set_int( "width", src->glWidth )
		&& ep->set_int( "height", src->glHeight )
		&& ep->set_float( "top", top )
		&& ep->set_float( "left", left );
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if ( resizeActive )
		list.append( new ResampleEffect() );
	list.append( new PaddingEffect );
	return list;
}
