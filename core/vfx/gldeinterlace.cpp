#include "vfx/gldeinterlace.h"



GLDeinterlace::GLDeinterlace( QString id, QString name ) : GLFilter( id, name )
{
}



GLDeinterlace::~GLDeinterlace()
{
}



QList<Effect*> GLDeinterlace::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyDeinterlaceEffect() );
	return list;
}







