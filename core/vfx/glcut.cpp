#include "vfx/glcut.h"



GLCut::GLCut( QString id, QString name ) : GLFilter( id, name )
{
}



GLCut::~GLCut()
{
}



Effect* GLCut::getMovitEffect()
{
	return new MyCutEffect();
}







