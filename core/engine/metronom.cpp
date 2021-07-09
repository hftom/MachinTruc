// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include "output/common_ff.h"

#include <QApplication>

#include "engine/metronom.h"

#include <QGLFramebufferObject>

// 1 frame in composer
// 1 in opengl
// 1 in metronom::runShow
// 1 in display
// less than 2 would freeze the app.
#define NUMOUTPUTFRAMES 5



static const int NSPS = 5;
static const int slowPlaybackSpeed[NSPS][2] = {
	{ 1, 1 }, // 1
	{ 7, 8 }, // 0.875
	{ 3, 4 }, // 0.75
	{ 1, 2 }, // 0.5
	{ 1, 4 }, // 0.25
};

static const int NFPS = 10;
static const int fastPlaybackSpeed[NFPS][2] = {
	{ 1, 1 }, // 1
	{ 9, 8 }, // 1.125
	{ 5, 4 }, // 1.25
	{ 11, 8 }, // 1.375
	{ 3, 2 }, // 1.5
	{ 7, 4 }, // 1.75
	{ 2, 1 }, // 2
	{ 3, 1 }, // 3
	{ 4, 1 }, // 4
	{ 8, 1 }, // 8
};



Metronom::Metronom( PlaybackBuffer *pb )
	: speed( 0 ),
	playBackward( false ),
	running( false ),
	sclock( 0 ),
	videoLate( 0 ),
	playbackBuffer( pb ),
	fencesContext( NULL ),
	renderMode( false ),
	lastFrame( NULL )
{
	for ( int i = 0; i < NUMOUTPUTFRAMES; ++i ) {
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames ) );
		freeAudioFrames.enqueue( new Frame( &freeAudioFrames ) );
	}

	ao.setReadCallback( (void*)readData, (void*)this );
	ao.setPlaybackBuffer( playbackBuffer );
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
	if ( lastFrame && lastFrame != f ) {
		if ( lastFrame->isDuplicate )
			lastFrame->release();
		else
			playbackBuffer->releasedVideoFrame( lastFrame );
	}
	lastFrame = f;
	emit currentFramePts( f->pts() );
}



Frame* Metronom::getLastFrame()
{
	QMutexLocker ml( &lastFrameMutex );
	return (lastFrame) ? lastFrame : NULL;
}



Frame* Metronom::getAndLockLastFrame()
{
	lastFrameMutex.lock();
	return lastFrame;
}



void Metronom::unlockLastFrame()
{
	lastFrameMutex.unlock();
}



void Metronom::flush()
{
	Frame *f;
	while ( (f = encodeVideoFrames.dequeue()) )
		f->release();
	while ( (f = videoFrames.dequeue()) )
		playbackBuffer->releasedVideoFrame( f );
	while ( (f = audioFrames.dequeue()) )
		playbackBuffer->releasedAudioFrame( f );
	// make sure all queued frames are shown
	qApp->processEvents();
}



void Metronom::setRenderMode( bool b )
{
	renderMode = b;
}



void Metronom::play( bool b, bool backward )
{
	if ( b ) {
		playBackward = backward;
		speed = 0;
		sclock = videoLate = 0;
		running = true;
		if ( !renderMode )
			ao.go();
		start();
		emit osdTimer( true );
	}
	else {
		running = false;
		ao.stop();
		wait();
		emit osdMessage( "", 0 );
		emit osdTimer( false );
	}
}



void Metronom::changeSpeed( int s )
{
	if ( !isRunning() )
		return;
	speed = qMax( qMin( speed + s, NFPS - 1 ), 1 - NSPS  );
	double d;
	if ( speed >= 0 )
		d = (double)fastPlaybackSpeed[speed][0] / (double)fastPlaybackSpeed[speed][1];
	else
		d = (double)slowPlaybackSpeed[-speed][0] / (double)slowPlaybackSpeed[-speed][1];
	emit osdMessage( QString( "Speed: %1" ).arg( d, 0, 'g', 4 ), (speed == 0) ? 1 : 0 );
}



void Metronom::readData( Frame **data, double time, void *userdata )
{
	Metronom *m = (Metronom*)userdata;
	Frame *f;
	int waitVideo = 0;

	if ( (f = m->audioFrames.dequeue()) ) {
		m->clockMutex.lock();
		if ( m->videoLate > 0 ) {
			waitVideo = m->videoLate;
			time += waitVideo;
			m->videoLate = 0;
		}
		if ( m->playBackward )
			m->sclock = time + f->pts();
		else
			m->sclock = time - f->pts();
		m->clockMutex.unlock();
		if ( waitVideo ) {
			qDebug() << "videoLate" << waitVideo;
			usleep( waitVideo );
			waitVideo = 0;
		}

		int speed = m->speed;
		if ( speed != 0 ) {
			int sampleSize = f->profile.getAudioChannels() * Profile::bytesPerChannel(&f->profile);
			int offset, keep;
			Buffer *buffer = NULL;
			uint8_t *buf = f->data();
			uint8_t *dst = buf;
			if ( speed > 0 ) {
				offset = fastPlaybackSpeed[speed][0];
				keep = fastPlaybackSpeed[speed][1];
			}
			else {
				offset = slowPlaybackSpeed[-speed][0];
				keep = slowPlaybackSpeed[-speed][1];
				int size = f->audioSamples() * keep / offset + 1;
				size *= sampleSize;
				buffer = BufferPool::globalInstance()->getBuffer( size );
				dst = buffer->data();
			}
			int chunkSize = sampleSize * keep;
			uint8_t *end = buf + sampleSize * f->audioSamples();
			int nSamples = 0;
			while ( buf + chunkSize < end ) {
				memcpy( dst, buf, chunkSize );
				dst += chunkSize;
				buf += sampleSize * offset;
				nSamples += keep;
			}
			if ( buf < end ) {
				memcpy( dst, buf, end - buf );
				nSamples += (end - buf) / sampleSize;
			}
			if ( buffer ) {
				f->setSharedBuffer( buffer );
				BufferPool::globalInstance()->releaseBuffer( buffer );
			}
			f->setAudioSamples( nSamples );
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
	QGLFramebufferObject *fb = NULL;
	struct SwsContext *swsCtx;
	GLuint pbo = 0;

	while ( running ) {
		if ( (f = videoFrames.dequeue()) ) {
			int w = f->profile.getVideoWidth();
			int h = f->profile.getVideoHeight();
			if ( !pbo ) {
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

				glGenBuffers(1, &pbo);
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
				glBufferData(GL_PIXEL_PACK_BUFFER_ARB, w * h * 3 + 32, NULL, GL_STREAM_READ);
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
			glPixelStorei( GL_PACK_ALIGNMENT, 1 );

			glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
			glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0) );
			uint8_t *rgbData = (uint8_t*)glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);

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

			glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
			glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
			fb->release();

			encodeVideoFrames.enqueue( f );
		}
		else
			usleep( 1000 );
	}

	if (pbo)
		glDeleteBuffers( 1, &pbo );
	if ( fb )
		delete fb;
	if ( swsCtx )
		sws_freeContext( swsCtx );
}



void Metronom::runShow()
{
	Frame *f;
	double lastpts = 0, lastct = 0, ptsDiff;
	int skipped = -1;
	struct timeval tv;
	double sc, ct, t, predict = 0;
	double frameDuration, speedFactor = 1;

	while ( running ) {
		if ( (f = videoFrames.dequeue()) ) {
			bool show = f->type() == Frame::GLTEXTURE;

			if ( playBackward )
				ptsDiff = lastpts - f->pts();
			else
				ptsDiff = f->pts() - lastpts;
			frameDuration = f->profile.getVideoFrameDuration();
			speedFactor = 1;
			if ( speed != 0 ) {
				if ( speed > 0 )
					speedFactor = (double)fastPlaybackSpeed[speed][0] / (double)fastPlaybackSpeed[speed][1];
				else
					speedFactor = (double)slowPlaybackSpeed[-speed][0] / (double)slowPlaybackSpeed[-speed][1];
				ptsDiff /= speedFactor;
				frameDuration /= speedFactor;
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
					t = lastct + ptsDiff;
				}
				lastct = t;
			}
			else {
				if ( playBackward )
					t = sc - f->pts();
				else
					t = sc + f->pts();
				if ( !predict )
					 predict = t;
				else {
					predict += ptsDiff;
					if ( qAbs( predict - t ) < frameDuration * 3 )
						t = predict;
					else
						predict = t;
				}
			}
			lastpts = f->pts();

			if ( t < ct ) {
				if ( (ct - t) > frameDuration && skipped > -1 ) {
					//predict = 0;
					emit discardFrame( 1 );
					if ( ++skipped > MICROSECOND / frameDuration / 4.0 ) {
						clockMutex.lock();
						videoLate = qMin( ct -t, MICROSECOND / 2.0 );
						clockMutex.unlock();
						skipped = -1;
					}
				}
				else {
					skipped = 0;
				}
			}
			else
				skipped = 0;

			if ( show ) {
				t = t - ct;
				if ( t > 0 ) {
					//qDebug() << "sleep:" << t;
					usleep( qMin( t, frameDuration * 3.0 ) );
				}
				emit newFrame( f );
			}
			else
				playbackBuffer->releasedVideoFrame( f );
		}
		else
			usleep( 1000 );
	}
}
