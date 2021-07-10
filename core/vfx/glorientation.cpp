#include "glorientation.h"
#include "glmirror.h"



GLOrientation::GLOrientation( QString id, QString name ) : GLFilter( id, name ),
	angle( 0 ), mirror( false )
{
}



GLOrientation::~GLOrientation()
{
}



QString GLOrientation::getDescriptor( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );
	computeAngleAndMirror(src->orientation());
	if ( angle && angle != 180 ) {
		int w = src->glWidth;
		src->glWidth = src->glHeight;
		src->glHeight = w;
	}

	return QString("%1-%2-%3").arg(getIdentifier()).arg(angle).arg(mirror);
}



bool GLOrientation::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( el );
	Q_UNUSED( p );
	if ( angle && angle != 180 ) {
		int w = src->glWidth;
		src->glWidth = src->glHeight;
		src->glHeight = w;
	}
	return true;
}



void GLOrientation::setOrientation( int o )
{
	computeAngleAndMirror(o);
}



void GLOrientation::computeAngleAndMirror(int o)
{
	switch (o) {
		// exif values
		case 2: angle = 0; mirror = true; break;
		case 5: angle = 90; mirror = true; break;
		case 6: angle = 90; mirror = false; break;
		case 3: angle = 180; mirror = true; break;
		case 4: angle = 180; mirror = false; break;
		case 7: angle = 270; mirror = true; break;
		case 8: angle = 270; mirror = false; break;
		// ffmpeg rotation angle
		default: angle = o; mirror = false;
	}
}



QList<Effect*> GLOrientation::getMovitEffects()
{
	QList<Effect*> list;

	if (angle) {
		Effect *e = new MyOrientationEffect();
		bool ok = e->set_int( "angle", abs(angle) );
		Q_UNUSED( ok );
		list.append( e );
	}
 
	if (mirror) {
		Effect *e = new MyMirrorEffect();
		list.append( e );
	}

	return list;
}
