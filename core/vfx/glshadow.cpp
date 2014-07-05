#include "vfx/glshadow.h"



GLShadow::GLShadow( QString id, QString name ) : GLFilter( id, name )
{
	opacity = 0.6;
	xoffset = yoffset = 10.0;
	addParameter( tr("X offset:"), PFLOAT, -100.0, 100.0, true, &xoffset );
	addParameter( tr("Y offset:"), PFLOAT, -100.0, 100.0, true, &yoffset );
	addParameter( tr("Opacity:"), PFLOAT, 0.0, 1.0, true, &opacity );
}



GLShadow::~GLShadow()
{
}



bool GLShadow::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	return el.at(0)->set_float( "width", p->getVideoWidth() )
		&& el.at(0)->set_float( "height", p->getVideoHeight() )
		&& el.at(0)->set_float( "xoffset", xoffset )
		&& el.at(0)->set_float( "yoffset", yoffset )
		&& el.at(0)->set_float( "opacity", opacity );
}



QList<Effect*> GLShadow::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyShadowEffect() );
	return list;
}






