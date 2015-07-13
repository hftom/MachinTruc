#include <QFile>

#include "glstabilize.h"



GLStabilize::GLStabilize( QString id, QString name ) : GLFilter( id, name ),
	source( NULL ),
	transforms( NULL )
{
	stabStatus = addParameter( "status", tr("Stabilization:"), Parameter::PSTATUS, tr("update..."), "", "", false );

	connect( &checkStabTimer, SIGNAL(timeout()), this, SLOT(checkStabData()) );
}



GLStabilize::~GLStabilize()
{
	if ( checkStabTimer.isActive() )
		checkStabTimer.stop();
	if ( transforms )
		StabilizeCollection::getGlobalInstance()->releaseTransforms( transforms );
}



void GLStabilize::setSource( Source *aSource )
{
	source = aSource;
	int status;
	int progress;
	transforms = StabilizeCollection::getGlobalInstance()->getTransforms( source, status, progress );
	if ( !transforms ) {
		if ( status != StabilizeTransform::STABERROR )
			checkStabTimer.start( 5000 );
		switch ( status ) {
			case StabilizeTransform::STABENQUEUED:
				stabStatus->value = tr("enqueued...");
				break;
			case StabilizeTransform::STABINPROGRESS:
				stabStatus->value = QString(tr("in progress... %1%")).arg(progress);
				break;
			default:
				stabStatus->value = tr("an error occured.");
		}
	}
	else
		stabStatus->value = tr("done.");
	
	emit statusUpdate( stabStatus->value.toString() );
}



void GLStabilize::checkStabData()
{
	int status;
	int progress;
	transforms = StabilizeCollection::getGlobalInstance()->getTransforms( source, status, progress );
	if ( !transforms ) {
		switch ( status ) {
			case StabilizeTransform::STABENQUEUED:
				stabStatus->value = tr("enqueued...");
				break;
			case StabilizeTransform::STABINPROGRESS:
				stabStatus->value = QString(tr("in progress... %1%")).arg(progress);
				break;
			default:
				stabStatus->value = tr("an error occured.");
				checkStabTimer.stop();
		}
	}
	else {
		stabStatus->value = tr("done.");
		checkStabTimer.stop();
	}
	
	emit statusUpdate( stabStatus->value.toString() );
}



bool GLStabilize::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );

	Effect *e = el[0];
	
	if ( transforms ) {
		double dur =  transforms->at( 1 ).pts - transforms->first().pts;
		int nframe = ((src->pts() - transforms->first().pts) / dur) + 0.5;
		nframe = qMax( qMin( nframe, transforms->count() - 1 ), 0 );
		StabilizeTransform ts = transforms->at( nframe );
		double rad = ts.alpha;
		double zoom = 1.0 - (ts.zoom / 100.0);
		double left = ts.x;
		double top = ts.y;

		switch ( src->orientation() ) {
			case 270:
				left = top;
				top = -left;
				break;
			case 180:
				left = -left;
				top = -top;
				break;
			case 90:
				left = -top;
				top = left;
		}
	
		return e->set_float( "angle", rad )
			&& e->set_float( "SAR", src->glSAR )
			&& e->set_float( "top", top )
			&& e->set_float( "left", left )
			&& e->set_float( "zoom", zoom );
	}
	else {
		return e->set_float( "SAR", src->glSAR );
	}
}



QList<Effect*> GLStabilize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyStabilizeEffect() );

	return list;
}
