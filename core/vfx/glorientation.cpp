#include "glorientation.h"



GLOrientation::GLOrientation( QString id, QString name ) : GLFilter( id, name ),
	angle( 0 )
{
}



GLOrientation::~GLOrientation()
{
}



QString GLOrientation::getDescriptor( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );
	angle = src->orientation();
	if ( angle != 180 ) {
		int w = src->glWidth;
		src->glWidth = src->glHeight;
		src->glHeight = w;
	}

	return getIdentifier() + QString::number( angle );
}



bool GLOrientation::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( el );
	Q_UNUSED( p );
	if ( angle != 180 ) {
		int w = src->glWidth;
		src->glWidth = src->glHeight;
		src->glHeight = w;
	}
	return true;
}



void GLOrientation::setOrientation( int a )
{
	angle = a;
}



QList<Effect*> GLOrientation::getMovitEffects()
{
	Effect *e = new MyOrientationEffect();
	bool ok = e->set_int( "angle", angle );
	Q_UNUSED( ok );
	
	QList<Effect*> list;
	list.append( e );

	return list;
}
