#include "vfx/glcut.h"



GLCut::GLCut( QString id, QString name ) : GLFilter( id, name )
{
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
}



GLCut::~GLCut()
{
}



bool GLCut::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el[0]->set_float( "opacity", getParamValue( opacity, src->pts() ).toFloat() );
}



QList<Effect*> GLCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyCutEffect() );
	return list;
}







