#include "vfx/glcontrast.h"



GLContrast::GLContrast( QString id, QString name ) : GLMask( id, name )
{
	contrast = addParameter( "contrast", tr("Contrast:"), Parameter::PDOUBLE, 0.0, -10.0, 10.0, true );
	brightness = addParameter( "brightness", tr("Brightness:"), Parameter::PDOUBLE, 0.0, -10.0, 10.0, true );

	GLMask::setParameters();
}



QString GLContrast::getDescriptor( double pts, Frame *src, Profile *p  )
{
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLContrast::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	bool ok = el.at(0)->set_float( "contrast", getParamValue(contrast, pts).toFloat() )
		&& el.at(0)->set_float( "brightness", getParamValue(brightness, pts).toFloat() );
	ok |= GLMask::processMask(pts, src, p);

	return ok;
}



QList<Effect*> GLContrast::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new MContrastEffect) );
	return list;
}
