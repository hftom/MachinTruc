#include "vfx/glfiber.h"



GLFiber::GLFiber( QString id, QString name ) : GLFilter( id, name )
{
	iterations = addParameter( "iterations", tr("Iterations:"), Parameter::PINT, 2.0, 1.0, 4.0, false );
}



bool GLFiber::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	Effect *e = el.first();
	return e->set_float( "time", pts / MICROSECOND )
			&& e->set_float( "iterations", getParamValue( iterations, pts ).toFloat() );
}



QList<Effect*> GLFiber::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyFiberEffect() );
	return list;
}
