#include "vfx/glrotate.h"



GLRotate::GLRotate( QString id, QString name ) : GLFilter( id, name )
{
	angle = addParameter( tr("Angle:"), Parameter::PDOUBLE, 0.0, -360.0, 360.0, true );
}



GLRotate::~GLRotate()
{
}



bool GLRotate::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el[0]->set_float( "angle", getParamValue( angle, src->pts() ).toDouble() * M_PI / 180.0 )
		&& el[0]->set_float( "SAR", src->glSAR );
}



QList<Effect*> GLRotate::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyRotateEffect() );
	return list;
}



MyRotateEffect::MyRotateEffect()
	: angle(0.0),
	SAR(1.0)
{
	register_float("angle", &angle);
	register_float("SAR", &SAR);
}
