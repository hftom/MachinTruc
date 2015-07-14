#include <sys/resource.h>

extern "C" { 
	#include <vid.stab/libvidstab.h>
}

#include <QDebug>

#include "input/input_ff.h"
#include "stabilizecollection.h"

#define MACHINTRUC_DIR "machintruc"
#define STABILIZE_DIR "stab"
#define STABILIZE_EXTENSION ".vidstab"



StabilizeCollection StabilizeCollection::globalInstance = StabilizeCollection();



StabilizeCollection* StabilizeCollection::getGlobalInstance()
{
	return &globalInstance;
}



StabilizeCollection::StabilizeCollection() : QObject()
{
	QDir dir;
	cdStabilizeDir( dir );
	
	connect( &checkDetectionTimer, SIGNAL(timeout()), this, SLOT(checkDetectionThreads()) );
}



bool StabilizeCollection::cdStabilizeDir( QDir &dir )
{
	dir = QDir::home();
	if ( !dir.cd( MACHINTRUC_DIR ) ) {
		if ( !dir.mkdir( MACHINTRUC_DIR ) ) {
			qDebug() << "Can't create" << MACHINTRUC_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( MACHINTRUC_DIR ) )
			return false;
	}
	if ( !dir.cd( STABILIZE_DIR ) ) {
		if ( !dir.mkdir( STABILIZE_DIR ) ) {
			qDebug() << "Can't create" << STABILIZE_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( STABILIZE_DIR ) )
			return false;
	}

	return true;
}



QString StabilizeCollection::pathToFileName( QString path )
{
	return path.replace( "/", "_" ).replace( "\\", "-" ).replace( ":", "." );
}



void StabilizeCollection::checkDetectionThreads()
{
	int working = 0;

	for ( int i = 0; i < stabDetect.count(); ++i ) {
		StabMotionDetect *stab = stabDetect.at( i );
		if ( stab->getStarted() ) {
			if ( !stab->isRunning() ) {
				if ( stab->getFinishedSuccess() ) {
					QList<StabilizeTransform> *list = stab->getTransformsOwnership();
					QDir dir;
					// save to file
					if ( cdStabilizeDir( dir ) ) {
						QFile f( dir.filePath( pathToFileName( stab->getFileName() ) + STABILIZE_EXTENSION ) );
						if ( f.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
							QDataStream data( &f );
							for ( int i = 0; i < list->count(); ++i ) {
								StabilizeTransform t = list->at( i );
								data << t.pts;
								data << t.x;
								data << t.y;
								data << t.alpha;
								data << t.zoom;
							}
							f.close();
						}
					}
					stabItems.append( new StabilizeItem( stab->getFileName(), list ) );
				}
				else {
					stabItems.append( new StabilizeItem( stab->getFileName(), NULL ) );
				}
				stabDetect.takeAt( i-- );
				delete stab;
			}
			else {
				++working;
			}
		}
	}
	
	for ( int i = 0; i < stabDetect.count(); ++i ) {
		if ( working > 1 )
			break;
		StabMotionDetect *stab = stabDetect.at( i );
		if ( !stab->getStarted() ) {
			stab->go();
			++working;
		}
	}
	
	if ( !stabDetect.count() )
		checkDetectionTimer.stop();
}



QList<StabilizeTransform>* StabilizeCollection::getTransforms( Source *source, int &status, int &progress )
{
	foreach( StabilizeItem *it, stabItems ) {
		if ( it->getSourceName() == source->getFileName() ) {
			if ( it->getTransforms() ) {
				it->use();
				status = StabilizeTransform::STABREADY;
				return it->getTransforms();
			}
			else {
				status = StabilizeTransform::STABERROR;
				return NULL;
			}
		}
	}
	
	QDir dir;
	if ( !cdStabilizeDir( dir ) ) {
		status = StabilizeTransform::STABERROR;
		return NULL;
	}
	
	QFile f( dir.filePath( pathToFileName( source->getFileName() ) + STABILIZE_EXTENSION ) );
	if ( f.exists() && f.open( QIODevice::ReadOnly ) ) {
		QList<StabilizeTransform> *list = new QList<StabilizeTransform>();
		QDataStream data( &f );
		while ( !data.atEnd() ) {
			double pts;
			data >> pts;
			float x;
			data >> x;
			float y;
			data >> y;
			float alpha;
			data >> alpha;
			float zoom;
			data >> zoom;
			list->append( StabilizeTransform( pts, x, y, alpha, zoom ) );
		}
		f.close();
		
		if ( list->count() < 1 ) {
			list->clear();
			delete list;
			status = StabilizeTransform::STABERROR;
			return NULL;
		}
		else {
			StabilizeItem *stab = new StabilizeItem( source->getFileName(), list );
			stabItems.append( stab );
			stab->use();
			status = StabilizeTransform::STABREADY;
			return stab->getTransforms();
		}
	}
	
	for ( int i = 0; i < stabDetect.count(); ++i ) {
		StabMotionDetect *detect = stabDetect.at( i );
		if ( detect->getFileName() == source->getFileName() ) {
			if ( detect->getStarted() ) {
				status = StabilizeTransform::STABINPROGRESS;
				progress = detect->getProgress();
			}
			else {
				status = StabilizeTransform::STABENQUEUED;
			}
			return NULL;
		}
	}
	
	stabDetect.append( new StabMotionDetect( source ) );
	if ( !checkDetectionTimer.isActive() )
		checkDetectionTimer.start( 3000 );
	
	status = StabilizeTransform::STABENQUEUED;
	return NULL;
}



void StabilizeCollection::releaseTransforms( QList<StabilizeTransform> *list )
{
	for( int i = 0; i < stabItems.count(); ++i ) {
		StabilizeItem *it = stabItems.at( i );
		if ( it->getTransforms() == list ) {
			if ( it->release() )
				delete stabItems.takeAt( i );
			break;
		}
	}
}



StabMotionDetect::StabMotionDetect( Source *aSource )
	: progress( 0 ),
	started( false ),
	finishedSuccess( false ),
	source( aSource ),
	transforms( NULL )
{
	
}



StabMotionDetect::~StabMotionDetect()
{
	stop();
}



QList<StabilizeTransform>* StabMotionDetect::getTransformsOwnership()
{
	QList<StabilizeTransform>* ret = transforms;
	transforms = NULL;
	return ret;
}



void StabMotionDetect::go()
{
	if ( isRunning() )
		return;
	transforms = new QList<StabilizeTransform>();
	running = true;
	start( QThread::LowestPriority );
	started = true;
}



void StabMotionDetect::stop()
{
	running = false;
	wait();
	if ( transforms ) {
		transforms->clear();
		delete transforms;
	}
}
	

	
void StabMotionDetect::run()
{
	setpriority( PRIO_PROCESS, 0, 10 );
	
	Profile sourceProfile = source->getProfile();
	Profile outProfile = sourceProfile;
	if ( sourceProfile.getVideoInterlaced() ) {
		outProfile.setVideoFrameRate( outProfile.getVideoFrameRate() * 2.0 );
		outProfile.setVideoFrameDuration( outProfile.getVideoFrameDuration() / 2.0 );
	}

	InputFF *input = new InputFF();
	if ( !input->open( source->getFileName() ) ) {
		delete input;
		return;
	}
	input->setProfile( sourceProfile, outProfile );
	input->openSeekPlay( source->getFileName(), sourceProfile.getStreamStartTime() );
	Frame *f = input->getVideoFrame();
	if ( !f ) {
		input->play( false );
		delete input;
		return;
	}
	
	VSMotionDetect md;
	VSMotionDetectConfig conf = vsMotionDetectGetDefaultConfig( "detect" );
	conf.shakiness = 10;
	conf.accuracy = 15;
	VSPixelFormat format = PF_YUV420P;
	if ( f->type() == Frame::YUV422P )
		format = PF_YUV422P;
	VSFrameInfo fi;
	vsFrameInfoInit( &fi, sourceProfile.getVideoWidth(), sourceProfile.getVideoHeight(), format );
	vsMotionDetectInit( &md, &conf, &fi );
	
	VSTransformData data;
	VSTransformConfig config;
	VSTransformations trans;
	config = vsTransformGetDefaultConfig( "stabilize" );
	config.smoothing = outProfile.getVideoFrameRate() / 2;
	config.optZoom = 2;
	config.zoomSpeed = 6.0 / outProfile.getVideoFrameRate();
	VSFrameInfo fi_src, fi_dst;
	vsFrameInfoInit( &fi_src, sourceProfile.getVideoWidth(), sourceProfile.getVideoHeight(), format );
	vsFrameInfoInit( &fi_dst, sourceProfile.getVideoWidth(), sourceProfile.getVideoHeight(), format );
	vsTransformDataInit( &data, &config, &fi_src, &fi_dst );
	vsTransformationsInit( &trans );
	
	int nSamples = sourceProfile.getAudioSampleRate() * outProfile.getVideoFrameDuration() / MICROSECOND;
	double startPts = sourceProfile.getStreamStartTime();
	double endPts = sourceProfile.getStreamStartTime() + sourceProfile.getStreamDuration();
	
	QList<double> ptsList;
	QList<VSTransform> transList;

	do {
		progress = (f->pts() - startPts) * 100 / (endPts - startPts);
		LocalMotions *localmotions = (LocalMotions*)malloc( sizeof(LocalMotions) );
		VSFrame vsFrame;
		vsFrameFillFromBuffer( &vsFrame, f->data(), &md.fi );
		if ( vsMotionDetection( &md, localmotions, &vsFrame ) == VS_OK ) {
			transList.append( vsMotionsToTransform( &data, localmotions, 0 ) );
			vs_vector_del( localmotions );
			if ( f->pts() >= endPts || (ptsList.count() > 0 && f->pts() <= ptsList.last() ) )
				finishedSuccess = true;
			ptsList.append( f->pts() );
		}
		else{
			vs_vector_del( localmotions );
			running = false;
		}

		delete f;
		// consume audio
		if ( (f = input->getAudioFrame( nSamples )) )
			delete f;
		f = NULL;
		
		if ( !finishedSuccess )
			f = input->getVideoFrame();
		
	} while ( running && f );
	
	if ( f )
		delete f;
	input->play( false );
	delete input;
	
	if ( finishedSuccess ) {
		trans.ts = (VSTransform*)vs_malloc( sizeof(VSTransform) * transList.count() );
		trans.len = transList.count();
		for ( int i = 0; i < transList.count(); ++i )
			trans.ts[i] = transList.at(i);
		vsPreprocessTransforms( &data, &trans );
		
		if ( trans.len < 2 )
			finishedSuccess = false;
		else {
			for ( int i = 0; i < trans.len; ++i )
				transforms->append( StabilizeTransform( ptsList[i], trans.ts[i].x, trans.ts[i].y, trans.ts[i].alpha, trans.ts[i].zoom ) );
		}
		
		vsTransformDataCleanup( &data );
		vsTransformationsCleanup( &trans );
	}
	
	vsMotionDetectionCleanup( &md );
}
