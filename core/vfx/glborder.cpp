#include "glborder.h"



GLBorder::GLBorder( QString id, QString name ) : GLFilter( id, name )
{
	borderSize = addParameter( tr("Border size:"), Parameter::PDOUBLE, 1.0, 0.0, 100.0, false, "%" );
}



bool GLBorder::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el[0]->set_float( "borderSize", getParamValue( borderSize, src->pts() ) * src->glHeight / 2.0 / 100.0 );
}



QList<Effect*> GLBorder::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyBorderEffect() );
	return list;
}



MyBorderEffect::MyBorderEffect()
	: borderSize(1.0)
{
	register_float("borderSize", &borderSize);
}
