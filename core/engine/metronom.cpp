// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <QApplication>

#include "engine/metronom.h"



Metronom::Metronom()
	: running( false ),
	sclock( 0 ),
	videoLate( 0 ),
	fencesContext( NULL ),
	lastFrame( NULL )
{
	for ( int i = 0; i < NUMOUTPUTFRAMES; ++i ) {
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames, true ) );
		freeAudioFrames.enqueue( new Frame( &freeAudioFrames, true ) );
	}

	ao.setReadCallback( (void*)readData, (void*)this );
	//ao.go();
}



Metronom::~Metronom()
{
}



void Metronom::setSharedContext( QGLWidget *shared )
{
	fencesContext = shared;
#if QT_VERSION >= 0x050000
	fencesContext->context()->moveToThread( this );
#endif
}



void Metronom::setLastFrame( Frame *f )
{
	QMutexLocker ml( &lastFrameMutex );
	if ( lastFrame && lastFrame != f )
		lastFrame->release();
	lastFrame = f;
	emit currentFramePts( f->pts() );
}



Frame* Metronom::getLastFrame()
{
	QMutexLocker ml( &lastFrameMutex );
	return (lastFrame) ? lastFrame : NULL;
}



void Metronom::flush()
{
	Frame *f;
	while ( (f = videoFrames.dequeue()) )
		f->release();
	while ( (f = audioFrames.dequeue()) )
		f->release();
	// make sure all queued frames are shown
	qApp->processEvents();
}



void Metronom::play( bool b )
{
	if ( b ) {
		sclock = videoLate = 0;
		running = true;
		ao.go();
		start();
	}
	else {
		running = false;
		ao.stop();
		wait();
	}
}



void Metronom::readData( Frame **data, double time, void *userdata )
{
	Metronom *m = (Metronom*)userdata;
	Frame *f;
	int waitVideo = 0;

	if ( (f = m->audioFrames.dequeue()) ) {
		m->clockMutex.lock();
		m->sclock = time - f->pts();
		if ( m->videoLate > 0 ) {
			waitVideo = m->videoLate;
			m->videoLate = 0;
		}
		m->clockMutex.unlock();
		if ( waitVideo ) {
			usleep( waitVideo );
			waitVideo = 0;
		}
		*data = f;
	}
}



void Metronom::run()
{
	Frame *f;
	double lastpts = 0, lastct = 0;
	int skipped = -1;
	struct timeval tv;
	double sc, ct, t, predict = 0;

	fencesContext->makeCurrent();

	while ( running ) {
		if ( (f = videoFrames.dequeue()) ) {

			if ( f->type() == Frame::NONE ) {
				f->release();
				continue;
			}

			if ( f->fence() )
				glClientWaitSync( f->fence()->fence(), 0, GL_TIMEOUT_IGNORED );

			clockMutex.lock();
			sc = sclock;
			clockMutex.unlock();
			gettimeofday( &tv, NULL );
			ct = tv.tv_sec * MICROSECOND + tv.tv_usec;

			if ( !sc ) {
				if ( lastpts == 0 ) {
					t = ct;
				}
				else {
					t = lastct + f->pts() - lastpts;
				}
				lastct = t;
			}
			else {
				t = sc + f->pts();
				if ( !predict )
					 predict = t;
				else {
					predict += f->pts() - lastpts;
					if ( qAbs( predict - t ) < f->profile.getVideoFrameDuration() * 3 )
						t = predict;
					else
						predict = t;
				}
			}
			lastpts = f->pts();

			if ( t < ct ) {
				if ( (ct - t) > f->profile.getVideoFrameDuration() && skipped > -1 ) {
					double delta = ( (ct - t) / f->profile.getVideoFrameDuration() );
					skipped += delta;
					emit discardFrame( delta + 1 );
					if ( skipped >  f->profile.getVideoFrameRate() / 3 ) {
						clockMutex.lock();
						videoLate = qMin( ct -t, MICROSECOND / 4.0 );
						//qDebug() << "videoLate" << videoLate;
						clockMutex.unlock();
						skipped = -1;
						predict = 0;
					}
				}
				else {
					skipped = 0;
				}
			}
			else
				skipped = 0;

			t = t - ct;
			if ( t > 0 ) {
				//qDebug() << "sleep:" << t;
				usleep( t );
			}
			emit newFrame( f );
		}
		else
			usleep( 1000 );
	}

	fencesContext->doneCurrent();
}
