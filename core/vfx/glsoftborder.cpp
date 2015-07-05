#include "glsoftborder.h"



GLSoftBorder::GLSoftBorder( QString id, QString name ) : GLFilter( id, name )
{
	borderSize = addParameter( "borderSize", tr("Border size:"), Parameter::PDOUBLE, 1.0, 0.0, 100.0, false, "%" );
}



bool GLSoftBorder::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	return el[0]->set_float( "borderSize", getParamValue( borderSize ).toDouble() * src->glHeight / 2.0 / 100.0 );
}



QList<Effect*> GLSoftBorder::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MySoftBorderEffect() );
	return list;
}



MySoftBorderEffect::MySoftBorderEffect()
	: iwidth( 1 ),
	iheight( 1 ),
	borderSize(1.0)
{
	register_float("borderSize", &borderSize);
}
