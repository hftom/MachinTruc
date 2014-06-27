#include "vfx/glwater.h"



GLWater::GLWater( QString id, QString name ) : GLFilter( id, name )
{
	speed = 0.3;
	emboss = 0.4;
	intensity = 3.0;
	frequency = 4.0;
	delta = 160.0;
	intence = 300.0;
	addParameter( tr("Speed:"), PFLOAT, 0.01, 2.0, true, &speed );
	addParameter( tr("Emboss:"), PFLOAT, 0.01, 2.0, true, &emboss );
	addParameter( tr("Intensity:"), PFLOAT, 0.01, 10.0, true, &intensity );
	addParameter( tr("Frequency:"), PFLOAT, 0.01, 10.0, true, &frequency );
	addParameter( tr("Delta:"), PFLOAT, 10.0, 500.0, true, &delta );
	addParameter( tr("Intence:"), PFLOAT, 0.0, 1000.0, true, &intence );
}



GLWater::~GLWater()
{
}



bool GLWater::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	Effect *e = el.at(0);
	return e->set_float( "time", src->pts() / MICROSECOND )
		&& e->set_float( "speed", speed )
		&& e->set_float( "emboss", emboss )
		&& e->set_float( "intensity", intensity )
		&& e->set_float( "frequency", frequency )
		&& e->set_float( "delta", delta )
		&& e->set_float( "intence", intence * 10.0 );
}



QList<Effect*> GLWater::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new WaterEffect() );
	return list;
}