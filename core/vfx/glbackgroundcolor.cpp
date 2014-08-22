#include "glbackgroundcolor.h"



GLBackgroundColor::GLBackgroundColor( QString id, QString name ) : GLFilter( id, name )
{
	color = addParameter( tr("Color:"), Parameter::PCOLOR, QColor::fromRgbF( 1, 1, 1 ), QColor::fromRgbF( 0, 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
}



bool GLBackgroundColor::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	QColor c = getParamValue( color ).value<QColor>();
	float a = c.alphaF();
	// pass premultiplied to Movit
	RGBATuple col = RGBATuple( c.redF() * a, c.greenF() * a, c.blueF() * a, a );
	return el[0]->set_vec4( "color", (float*)&col );
}



QList<Effect*> GLBackgroundColor::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyBackgroundColorEffect() );
	return list;
}



MyBackgroundColorEffect::MyBackgroundColorEffect()
	: color( 1.0f, 1.0f, 1.0f, 1.0f )
{
	register_vec4( "color", (float *)&color );
}
