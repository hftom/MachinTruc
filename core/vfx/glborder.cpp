#include "glborder.h"



GLBorder::GLBorder( QString id, QString name ) : GLFilter( id, name )
{
	borderSize = addParameter( tr("Border size:"), Parameter::PDOUBLE, 1.0, 0.0, 100.0, false, "%" );
	color = addParameter( tr("Border color:"), Parameter::PCOLOR, QColor::fromRgbF( 1, 1, 1 ), QColor::fromRgbF( 0, 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
}



bool GLBorder::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	QColor c = getParamValue( color ).value<QColor>();
	float a = c.alphaF();
	// pass premultiplied to Movit
	RGBATuple col = RGBATuple( c.redF() * a, c.greenF() * a, c.blueF() * a, a );
	return el[0]->set_float( "borderSize", getParamValue( borderSize ).toDouble() * src->glHeight / 2.0 / 100.0 )
		&& el[0]->set_vec4( "color", (float*)&col );
}



QList<Effect*> GLBorder::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyBorderEffect() );
	return list;
}



MyBorderEffect::MyBorderEffect()
	: borderSize(1.0),
	  color( 1.0f, 1.0f, 1.0f, 1.0f )
{
	register_float( "borderSize", &borderSize );
	register_vec4( "color", (float *)&color );
}
