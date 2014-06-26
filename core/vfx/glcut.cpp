#include "vfx/glcut.h"



GLCut::GLCut( QString id, QString name ) : GLFilter( id, name )
{
}



GLCut::~GLCut()
{
}



QList<Effect*> GLCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyCutEffect() );
	return list;
}







