#include "vfx/glcontrast.h"



GLContrast::GLContrast( QString id, QString name ) : GLFilter( id, name )
{
	contrast = addParameter( "contrast", tr("Contrast:"), Parameter::PDOUBLE, 0.0, -10.0, 10.0, true );
	brightness = addParameter( "brightness", tr("Brightness:"), Parameter::PDOUBLE, 0.0, -10.0, 10.0, true );
}



bool GLContrast::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	return el.at(0)->set_float( "contrast", getParamValue(contrast, pts).toFloat() )
		&& el.at(0)->set_float( "brightness", getParamValue(brightness, pts).toFloat() );
}



QList<Effect*> GLContrast::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MContrastEffect() );
	return list;
}
