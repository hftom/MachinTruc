// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include "output/common_ff.h"

#include <QApplication>

#include "engine/metronom.h"

#include <QGLFramebufferObject>



Metronom::Metronom()
	: running( false ),
	sclock( 0 ),
	videoLate( 0 ),
	fencesContext( NULL ),
	renderMode( false ),
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
	while ( (f = encodeVideoFrames.dequeue()) )
		f->release();
	while ( (f = videoFrames.dequeue()) )
		f->release();
	while ( (f = audioFrames.dequeue()) )
		f->release();
	// make sure all queued frames are shown
	qApp->processEvents();
}



void Metronom::setRenderMode( bool b )
{
	renderMode = b;
}



void Metronom::play( bool b )
{
	if ( b ) {
		sclock = videoLate = 0;
		running = true;
		if ( !renderMode )
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
	fencesContext->makeCurrent();

	if ( renderMode )
		runRender();
	else
		runShow();

	fencesContext->doneCurrent();
}



void Metronom::runRender()
{
	Frame *f;
	uint8_t *rgbData = NULL;
	QGLFramebufferObject *fb = NULL;
	struct SwsContext *swsCtx;

	while ( running ) {
		if ( (f = videoFrames.dequeue()) ) {
			glGetError();
			int w = f->profile.getVideoWidth();
			int h = f->profile.getVideoHeight();
			if ( !rgbData ) {
				rgbData = (uint8_t*)malloc( w  * h * 3 + 32/*extra space for sws_scale*/ );
				swsCtx = sws_getContext( w, h, AV_PIX_FMT_RGB24,
										w, h, AV_PIX_FMT_YUV420P,
										0, NULL, NULL, NULL );
				const int *coefs;
				if ( w * h > 1280 * 576 )
					coefs = sws_getCoefficients( SWS_CS_ITU709 );
				else
					coefs = sws_getCoefficients( SWS_CS_ITU601 );
				sws_setColorspaceDetails( swsCtx, coefs, 1, coefs, 0, 0, 0, 0 );
				fb = new QGLFramebufferObject( w, h );
				glViewport( 0, 0, w, h );
				glMatrixMode( GL_PROJECTION );
				glLoadIdentity();
				glOrtho( 0.0, w, 0.0, h, -1.0, 1.0 );
				glMatrixMode( GL_MODELVIEW );
				glEnable( GL_TEXTURE_2D );
				glActiveTexture( GL_TEXTURE0 );
			}

			if ( f->fence() )
				glClientWaitSync( f->fence()->fence(), 0, GL_TIMEOUT_IGNORED );

			fb->bind();
			glBindTexture( GL_TEXTURE_2D, f->fbo()->texture() );
			glBegin( GL_QUADS );
				glTexCoord2f( 0, 1 ); glVertex3f( 0, 0, 0.);
				glTexCoord2f( 0, 0 ); glVertex3f( 0, h, 0.);
				glTexCoord2f( 1, 0 ); glVertex3f( w, h, 0.);
				glTexCoord2f( 1, 1 ); glVertex3f( w, 0, 0.);
			glEnd();
        	glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, rgbData );
			fb->release();

			f->setVideoFrame( Frame::YUV420P, w, h, f->profile.getVideoSAR(),
							  f->profile.getVideoInterlaced(),
							  f->profile.getVideoTopFieldFirst(),
							  f->pts(),
							  f->profile.getVideoFrameDuration() );

			const uint8_t * const src[4] = { rgbData, NULL, NULL, NULL };
			const int srcStride[4] = { w * 3, 0, 0, 0 };
			uint8_t *dst[4] = { f->data(), f->data() + w * h, f->data() + w * h * 5 / 4 };
			const int dstStride[4] = { w, w / 2, w / 2 };

			sws_scale( swsCtx, src, srcStride, 0, h, dst, dstStride );

			encodeVideoFrames.enqueue( f );
		}
		else
			usleep( 1000 );
	}

	if ( fb )
		delete fb;
	if ( rgbData )
		free( rgbData );
	if ( swsCtx )
		sws_freeContext( swsCtx );
}



void Metronom::runShow()
{
	Frame *f;
	double lastpts = 0, lastct = 0;
	int skipped = -1;
	struct timeval tv;
	double sc, ct, t, predict = 0;

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
}
