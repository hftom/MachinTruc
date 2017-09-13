#include "vfx/glcut.h"



GLCut::GLCut( QString id, QString name ) : GLMask( id, name )
{
	GLMask::setParameters();
}



GLCut::~GLCut()
{
}



QString GLCut::getDescriptor( double pts, Frame *src, Profile *p  )
{
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLCut::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	return GLMask::processMask(pts, src, p);
}



QList<Effect*> GLCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new MyCutEffect) );
	return list;
}







