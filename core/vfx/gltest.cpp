#include "vfx/gltest.h"



GLTest::GLTest( QString id, QString name ) : GLFilter( id, name )
{
	loop = 1.0f;
	addParameter( tr("Loop:"), PFLOAT, 1.0, 200.0, true, &loop );
}



GLTest::~GLTest()
{
}



bool GLTest::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	qDebug() << "Loop" << (int)loop;
	return el.at(0)->set_float( "loop", (int)loop );
}



QList<Effect*> GLTest::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MTestEffect() );
	return list;
}
