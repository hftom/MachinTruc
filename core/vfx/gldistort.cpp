#include "vfx/gldistort.h"



GLDistort::GLDistort( QString id, QString name ) : GLFilter( id, name )
{
	ratio = addParameter( "ratio", tr("Ratio:"), Parameter::PDOUBLE, 1.0, 0.1, 10.0, true );
}



GLDistort::~GLDistort()
{
}



void GLDistort::preProcess( Frame *src, Profile *p )
{
	src->glSAR *=  getParamValue( ratio ).toFloat();
}



QString GLDistort::getDescriptor( double pts, Frame *src, Profile *p )
{
	preProcess( src, p );
	return getIdentifier();
}



bool GLDistort::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	preProcess( src, p );

	if ( src->glOVD )
		src->glOVDTransformList.append( FilterTransform( FilterTransform::SCALE, (double)src->glWidth, (double)src->glHeight * src->glSAR ) );
	
	return true;
}



QList<Effect*> GLDistort::getMovitEffects()
{
	QList<Effect*> list;
	return list;
}
