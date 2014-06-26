#include "vfx/gldeinterlace.h"



GLDeinterlace::GLDeinterlace( QString id, QString name ) : GLFilter( id, name )
{
}



GLDeinterlace::~GLDeinterlace()
{
}



bool GLDeinterlace::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el.at(0)->set_float( "height", src->glHeight );
}



QList<Effect*> GLDeinterlace::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyDeinterlaceEffect() );
	return list;
}







