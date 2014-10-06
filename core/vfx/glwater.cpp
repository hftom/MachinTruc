#include "vfx/glwater.h"



GLWater::GLWater( QString id, QString name ) : GLFilter( id, name )
{
	speed = addParameter( "speed", tr("Speed:"), Parameter::PDOUBLE, 0.3, 0.01, 2.0, false );
	emboss = addParameter( "emboss", tr("Emboss:"), Parameter::PDOUBLE, 0.4, 0.01, 2.0, true );
	intensity = addParameter( "intensity", tr("Intensity:"), Parameter::PDOUBLE, 3.0, 0.01, 10.0, false );
	frequency = addParameter( "frequency", tr("Frequency:"), Parameter::PDOUBLE, 4.0, 0.01, 10.0, false );
	delta = addParameter( "delta", tr("Delta:"), Parameter::PDOUBLE, 160.0, 10.0, 500.0, true );
	intence = addParameter( "intence", tr("Intence:"), Parameter::PDOUBLE, 3000.0, 0.0, 10000.0, true );
}



GLWater::~GLWater()
{
}



bool GLWater::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p )
	Effect *e = el[0];
	double pts = src->pts();
	return e->set_float( "time", pts / MICROSECOND )
		&& e->set_float( "speed", getParamValue( speed ).toFloat() )
		&& e->set_float( "emboss", getParamValue( emboss, pts ).toFloat() )
		&& e->set_float( "intensity", getParamValue( intensity ).toFloat() )
		&& e->set_float( "frequency", getParamValue( frequency ).toFloat() )
		&& e->set_float( "delta", getParamValue( delta, pts ).toFloat() )
		&& e->set_float( "intence", getParamValue( intence, pts ).toFloat() );
}



QList<Effect*> GLWater::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new WaterEffect() );
	return list;
}