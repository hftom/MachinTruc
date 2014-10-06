#include "engine/util.h"

#include "glborder.h"



GLBorder::GLBorder( QString id, QString name ) : GLFilter( id, name )
{
	borderSize = addParameter( "borderSize", tr("Border size:"), Parameter::PDOUBLE, 1.0, 0.0, 100.0, false, "%" );
	color = addParameter( "color", tr("Color:"), Parameter::PRGBACOLOR, QColor::fromRgbF( 1, 1, 1 ), QColor::fromRgbF( 0, 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
}



bool GLBorder::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	QColor c = getParamValue( color ).value<QColor>();
	// convert gamma and premultiply
	sRgbColorToPremultipliedLinear( c );
	RGBATuple col = RGBATuple( c.redF(), c.greenF(), c.blueF(), c.alphaF() );
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
	: iwidth(1),
	iheight(1),
	borderSize(1.0),
	color( 1.0f, 1.0f, 1.0f, 1.0f )
{
	register_float( "borderSize", &borderSize );
	register_vec4( "color", (float *)&color );
}
