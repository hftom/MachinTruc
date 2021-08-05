#include <QFile>

#include "glstabilize.h"



GLStabilize::GLStabilize( QString id, QString name ) : GLFilter( id, name ),
	source( NULL ),
	transforms( NULL )
{
	strength = addParameter( "strength", tr("Strength:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	stabStatus = addParameter( "status", tr("Stabilization:"), Parameter::PSTATUS, tr("update..."), "", "", false );

	connect( &checkStabTimer, SIGNAL(timeout()), this, SLOT(checkStabData()) );
}



GLStabilize::~GLStabilize()
{
	filterRemoved();
	if (source) {
		source->release();
	}
}



void GLStabilize::filterRemoved()
{
	if ( checkStabTimer.isActive() ) {
		checkStabTimer.stop();
	}

	QMutexLocker ml(&transformsMutex);
	if ( transforms ) {
		StabilizeCollection::getGlobalInstance()->releaseTransforms( transforms );
		transforms = NULL;
	}
}



void GLStabilize::setSource( Source *aSource )
{
	if (!source) {
		aSource->use();
	}
	source = aSource;
	int status;
	int progress;

	transformsMutex.lock();
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
	else {
		stabStatus->value = tr("done.");
	}
	transformsMutex.unlock();

	emit statusUpdate( stabStatus->value.toString() );
}



void GLStabilize::checkStabData()
{
	int status;
	int progress;
	
	transformsMutex.lock();
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
	transformsMutex.unlock();
	
	emit statusUpdate( stabStatus->value.toString() );
}



bool GLStabilize::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );

	Effect *e = el[0];
	
	QMutexLocker ml(&transformsMutex);
	if ( transforms && transforms->size() > 1 ) {
		double srcpts = src->pts();
		double d =  transforms->at( 1 ).pts - transforms->first().pts;
		int nframe = ((srcpts - transforms->first().pts) / d) + 0.5;
		nframe = qMax( qMin( nframe, transforms->count() - 1 ), 0 );

		d /= 4.0;
		while ( (transforms->at( nframe ).pts - srcpts) > d && nframe > 0 )
			--nframe;
		while ( (srcpts - transforms->at( nframe ).pts) > d && nframe < transforms->count() - 1 )
			++nframe;

		StabilizeTransform ts = transforms->at( nframe );
		double st = getParamValue( strength, pts ).toDouble();
		double rad = ts.alpha * st;
		double zoom = 1.0 - ((ts.zoom * st) / 100.0);
		double left = ts.x * st;
		double top = ts.y * st;
		double tmp;

		switch ( src->orientation() ) {
			case 270:
				tmp = left;
				left = top;
				top = -tmp;
				break;
			case 180:
				left = -left;
				top = -top;
				break;
			case 90:
				tmp = left;
				left = -top;
				top = tmp;
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
