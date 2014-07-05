#include "vfx/glpixelize.h"



GLPixelize::GLPixelize( QString id, QString name ) : GLFilter( id, name )
{
	pixelSize = 2.0;
	addParameter( tr("Pixel size:"), PFLOAT, 1.0, 100.0, true, &pixelSize );
}



GLPixelize::~GLPixelize()
{
}



bool GLPixelize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "pixelSize", pixelSize );
}



QList<Effect*> GLPixelize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyPixelizeEffect() );
	return list;
}



MyPixelizeEffect::MyPixelizeEffect()
	: pixelSize(2.0)
{
	register_float("pixelSize", &pixelSize);
}
