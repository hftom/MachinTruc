#include "engine/util.h"

#include "glbackgroundcolor.h"



GLBackgroundColor::GLBackgroundColor( QString id, QString name ) : GLFilter( id, name )
{
	color = addParameter( "color", tr("Color:"), Parameter::PRGBACOLOR, QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 0, 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
}



bool GLBackgroundColor::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	QColor c = getParamValue( color ).value<QColor>();
	// convert gamma and premultiply
	sRgbColorToPremultipliedLinear( c );
	RGBATuple col = RGBATuple( c.redF(), c.greenF(), c.blueF(), c.alphaF() );
	return el[0]->set_vec4( "color", (float*)&col );
}



QList<Effect*> GLBackgroundColor::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyBackgroundColorEffect() );
	return list;
}



MyBackgroundColorEffect::MyBackgroundColorEffect()
	: color( 0.0f, 0.0f, 0.0f, 1.0f )
{
	register_vec4( "color", (float *)&color );
}
