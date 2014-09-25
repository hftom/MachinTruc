#include "vfx/glrotate.h"



GLRotate::GLRotate( QString id, QString name ) : GLFilter( id, name )
{
	angle = addParameter( tr("Angle:"), Parameter::PDOUBLE, 0.0, -360.0, 360.0, true );
	aaBorder = addParameter( tr("Soft border width:"), Parameter::PINT, 2, 1, 10, false );
}



GLRotate::~GLRotate()
{
}



bool GLRotate::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el[0]->set_float( "angle", getParamValue( angle, src->pts() ).toDouble() * M_PI / 180.0 )
		&& el[0]->set_int( "aaBorder", getParamValue( aaBorder ).toInt() );
}



QList<Effect*> GLRotate::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyRotateEffect() );
	return list;
}



MyRotateEffect::MyRotateEffect()
	: angle(0.0),
	aaBorder(5)
{
	register_float("angle", &angle);
	register_int("aaBorder", &aaBorder);
}
