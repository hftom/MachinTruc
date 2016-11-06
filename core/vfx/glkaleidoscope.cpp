#include "vfx/glkaleidoscope.h"



GLKaleidoscope::GLKaleidoscope( QString id, QString name ) : GLFilter( id, name )
{
	size = addParameter( "size", tr("Size:"), Parameter::PINT, 7.0, 1.0, 30.0, false );
}



GLKaleidoscope::~GLKaleidoscope()
{
}



bool GLKaleidoscope::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	Effect *e = el[0];
	return e->set_float( "time", pts / MICROSECOND )
		&& e->set_float( "size", getParamValue( size ).toFloat() );
}



QList<Effect*> GLKaleidoscope::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new KaleidoscopeEffect() );
	return list;
}