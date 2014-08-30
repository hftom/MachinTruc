#include "vfx/gltest.h"



GLTest::GLTest( QString id, QString name ) : GLFilter( id, name )
{
	loop = addParameter( tr("Loop:"), Parameter::PINT, 1.0, 1.0, 200.0, false );
}



GLTest::~GLTest()
{
}



bool GLTest::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	int lp = getParamValue( loop ).toInt();
	return el.at(0)->set_float( "loop", lp );
}



QList<Effect*> GLTest::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MTestEffect() );
	return list;
}
