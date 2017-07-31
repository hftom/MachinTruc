#include "vfx/glpixelize.h"



GLPixelize::GLPixelize( QString id, QString name ) : GLFilter( id, name )
{
	pixelSize = addParameter( "pixelSize", tr("Pixel size:"), Parameter::PDOUBLE, 10.0, 1.0, 100.0, true );
}



GLPixelize::~GLPixelize()
{
}



bool GLPixelize::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	bool ok = el.at(0)->set_float( "pixelSize", getParamValue( pixelSize, pts ).toFloat() );

	return ok;
}



QList<Effect*> GLPixelize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyPixelizeEffect() );
	return list;
}



MyPixelizeEffect::MyPixelizeEffect()
	: iwidth(1),
	iheight(1),
	pixelSize(2.0)
{
	register_float("pixelSize", &pixelSize);
}
