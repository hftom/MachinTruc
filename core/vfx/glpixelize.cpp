#include "vfx/glpixelize.h"



GLPixelize::GLPixelize( QString id, QString name ) : GLMask( id, name )
{
	pixelSize = addParameter( "pixelSize", tr("Pixel size:"), Parameter::PDOUBLE, 10.0, 1.0, 100.0, true );
	
	GLMask::setParameters();
}



GLPixelize::~GLPixelize()
{
}



QString GLPixelize::getDescriptor( double pts, Frame *src, Profile *p  )
{
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLPixelize::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	bool ok = el.at(0)->set_float( "pixelSize", getParamValue( pixelSize, pts ).toFloat() );
	ok |= GLMask::processMask(pts, src, p);

	return ok;
}



QList<Effect*> GLPixelize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new MyPixelizeEffect) );
	return list;
}



MyPixelizeEffect::MyPixelizeEffect()
	: iwidth(1),
	iheight(1),
	pixelSize(2.0)
{
	register_float("pixelSize", &pixelSize);
}
