#include <QFile>
#include <QTime>

#include <movit/init.h>

#include "engine/composer.h"

#include "afx/audiocopy.h"
#include "afx/audiomix.h"
#include "afx/audiovolume.h"

#include "input/input_color.h"



Composer::Composer( Sampler *samp )
{
	/*guiThread = QThread::currentThread();
	qDebug() << "guiThread" << guiThread;*/
	
    running = false;
    oneShot = false;
	audioSampleDelta = 0;
	sampler = samp;
}



Composer::~Composer()
{
}



void Composer::setSharedContext( QGLWidget *shared )
{
    hiddenContext = shared;
    hiddenContext->makeCurrent();
	
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

	QString movitPath;
	if ( QFile("/usr/share/movit/header.frag").exists() )
		movitPath = "/usr/share/movit";
	else if ( QFile("/usr/local/share/movit/header.frag").exists() )
		movitPath = "/usr/local/share/movit";
	else
		assert(false);
	
    bool ok = init_movit( movitPath.toLocal8Bit().data(), MOVIT_DEBUG_ON );
	
	movitPool = new ResourcePool( 100, 300 << 20, 100 );

	mask_texture = hiddenContext->bindTexture( QImage("/home/cris/mask.png") );
	glBindTexture( GL_TEXTURE_2D, mask_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	printf("mask_texture = %u\n", mask_texture);

    hiddenContext->doneCurrent();
}



void Composer::discardFrame()
{
	++skipFrame;
}



void Composer::seeking()
{
    audioSampleDelta = 0;
    runOneShot();
}



void Composer::play( bool b )
{
	if ( b ) {
		sampler->getMetronom()->play( b );
		//hiddenContext->context()->moveToThread( this );
		running = true;
		start();
	}
	else {
		sampler->getMetronom()->play( b );
		running = false;
		wait();
	}
}



void Composer::updateFrame( Frame *dst )
{
	if ( isRunning() )
		return;
	
	skipFrame = 0;
	hiddenContext->makeCurrent();
	
	movitRender( dst, true );
	
	hiddenContext->doneCurrent();
	emit newFrame( dst );
}


#define PROCESSWAITINPUT 0
#define PROCESSCONTINUE 1
#define PROCESSEND 2
#define PROCESSONESHOTVIDEO 3

void Composer::runOneShot()
{
	int ret;
	Frame *f = NULL;
	
	if ( isRunning() )
		return;
	
	skipFrame = 0;
	hiddenContext->makeCurrent();
	oneShot = true;

	ret = process( &f );

	oneShot = false;
	hiddenContext->doneCurrent();

	if ( (ret == PROCESSONESHOTVIDEO) && f )
		emit newFrame( f );
}



void Composer::run()
{
	int ret;
	Frame *f = NULL;

	skipFrame = 0;
	hiddenContext->makeCurrent();

	while ( running ) {
		ret = process( &f );

		if ( ret == PROCESSEND ) {
			while ( !sampler->getMetronom()->videoFrames.queueEmpty() )
				usleep( 5000 );
			sampler->getMetronom()->play( false );
			emit paused( true );
			break;
		}
		
		if ( ret == PROCESSWAITINPUT )
			usleep( 1000 );
	}

	hiddenContext->doneCurrent();
	//hiddenContext->context()->moveToThread( guiThread );
}



int Composer::process( Frame **frame )
{
	Frame *dst, *dsta;
	bool video = false;
	int ret = PROCESSWAITINPUT;
	
	Profile projectProfile = sampler->getProfile();

	if ( sampler->currentPTS() > sampler->getEndPTS() )
		return PROCESSEND;

	if ( oneShot)
		sampler->prepareInputs();

	if ( (dst = sampler->getMetronom()->freeVideoFrames.dequeue()) ) {
		if ( !renderVideoFrame( dst ) ) {
			dst->release();
			dst = NULL;
		}
		else {
			dst->setPts( sampler->currentPTS() );
			video = true;
		}
		ret = PROCESSCONTINUE;
	}

	double ns = projectProfile.getAudioSampleRate() * projectProfile.getVideoFrameDuration() / MICROSECOND;
	int nSamples = ns;
	audioSampleDelta += (ns - nSamples);
	if ( audioSampleDelta >= 1. ) {
		nSamples += 1;
		audioSampleDelta -= 1.;
	}

	if ( (dsta = sampler->getMetronom()->freeAudioFrames.dequeue()) ) {
		if ( !renderAudioFrame( dsta, nSamples ) ) {
			dsta->release();
			dsta = NULL;
		}
		else {
			if ( oneShot )
				dsta->release();
			else {
				dsta->setPts( sampler->currentPTSAudio() );
				sampler->getMetronom()->audioFrames.enqueue( dsta );
			}
			sampler->shiftCurrentPTSAudio();
			ret = PROCESSCONTINUE;
		}
	}

	if ( video ) {
		double d = (dst->sample->frames.count() && dst->sample->frames[0]->frame) ? dst->sample->frames[0]->frame->pts() : 0;
		sampler->shiftCurrentPTS( d );
	}
	
	if ( !oneShot)
		sampler->prepareInputs();

	if ( video ) {
		if ( oneShot ) {
			ret = PROCESSONESHOTVIDEO;
			*frame = dst;
		}
		else
			sampler->getMetronom()->videoFrames.enqueue( dst );
	}

	return ret;
}



bool Composer::renderVideoFrame( Frame *dst )
{
    if ( !sampler->getVideoTracks( dst ) ) {
        // make black
		Profile projectProfile = sampler->getProfile();
        dst->setVideoFrame( Frame::GLTEXTURE, projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), projectProfile.getVideoSAR(),
                            false, false, sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
        if ( !gl.black( dst ) )
            return false;
		dst->glWidth = dst->profile.getVideoWidth();
		dst->glHeight = dst->profile.getVideoHeight();
		dst->glSAR = dst->profile.getVideoSAR();
		dst->setFence( gl.getFence() );
		glFlush();
        return true;
    }
    
    if ( skipFrame > 0 ) {
		--skipFrame;
		Profile projectProfile = sampler->getProfile();
		dst->setVideoFrame( Frame::NONE, projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), projectProfile.getVideoSAR(),
                            false, false, sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
		return true;
	}

	movitRender( dst );

	return true;
}



void Composer::movitRender( Frame *dst, bool update )
{
    int i, j, k, l, start=0;
    Frame *f;
	bool reload = !update;
	
	//QTime time;
	//time.start();
	
	Profile projectProfile = sampler->getProfile();

    // find the lowest frame to process
    for ( j = 0 ; j < dst->sample->frames.count(); ++j ) {
        if ( (f = dst->sample->frames[j]->frame) ) {
            start = j;
			break;
        }
    }

	// build a "description" of the required chain
	// processing frames from bottom to top
	i = start;
	QStringList currentDescriptor;
	while ( (f = getNextFrame( dst, i )) ) {
		bool paddingAuto = false;
		bool resizeAuto = false;
		f->paddingAuto = f->resizeAuto = true;
		f->glWidth = f->profile.getVideoWidth();
		f->glHeight = f->profile.getVideoHeight();
		f->glSAR = f->profile.getVideoSAR();
		
		currentDescriptor.append( MovitInput::getDescriptor( f ) );
		
		// deinterlace
		if ( f->profile.getVideoInterlaced() ) {
			currentDescriptor.append( GLDeinterlace().getDescriptor() );
		}
		
		for ( k = 0; k < dst->sample->frames[i - 1]->videoFilters.count(); ++k ) {
			currentDescriptor.append( dst->sample->frames[i - 1]->videoFilters[k]->getDescriptor() );
			dst->sample->frames[i - 1]->videoFilters[k]->preProcess( f, &projectProfile );
		}
		
		// resize to match destination aspect ratio
		if ( f->resizeAuto && fabs( projectProfile.getVideoSAR() - f->profile.getVideoSAR() ) > 1e-3 ) {
			GLResize resize;
			currentDescriptor.append( resize.getDescriptor() );
			resize.preProcess( f, &projectProfile );
			resizeAuto = true;
		}

		// padding
		if ( f->paddingAuto && !sampler->previewMode() ) {
			if ( f->glWidth != projectProfile.getVideoWidth() || f->glHeight != projectProfile.getVideoHeight() ) {
				GLPadding padding;
				currentDescriptor.append( padding.getDescriptor() );
				padding.preProcess( f, &projectProfile );
				paddingAuto = true;
			}
		}
		
		// compose
		if ( (i - 1) > start ) {
			if ( !dst->sample->frames[i - 1]->composition ) {
				// overlay is the default
				currentDescriptor.append( GLOverlay().getDescriptor() );
			}	
			else {
				currentDescriptor.append( dst->sample->frames[i - 1]->composition->getDescriptor() );
			}
		}
		
		f->paddingAuto = paddingAuto;
		f->resizeAuto = resizeAuto;
	}

	// rebuild the chain if neccessary
	if ( currentDescriptor !=  movitDescriptor ) {
		for ( k = 0; k < currentDescriptor.count(); k++ )
			printf("%s\n", currentDescriptor[k].toLocal8Bit().data());
		movitDescriptor = currentDescriptor;
		movitChain.reset();
		movitChain.chain = new EffectChain( projectProfile.getVideoSAR() * projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), movitPool );

		i = start;
		Effect *last, *current = NULL;
		
		//current = movitChain.chain->add_input( new InputColor() );
			
		while ( (f = getNextFrame( dst, i )) ) {
			last = current;
			MovitInput *in = new MovitInput();
			MovitBranch *branch = new MovitBranch( in );
			movitChain.branches.append( branch );
			current = movitChain.chain->add_input( in->getMovitInput( f ) );
			
			// deinterlace
			if ( f->profile.getVideoInterlaced() ) {
				GLDeinterlace *deint = new GLDeinterlace();
				QList<Effect*> el = deint->getMovitEffects();
				branch->filters.append( new MovitFilter( el, deint, true ) );
				for ( l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ) );
			}
			
			// apply filters
			for ( k = 0; k < dst->sample->frames[i - 1]->videoFilters.count(); ++k ) {
				QString s = dst->sample->frames[i - 1]->videoFilters[k]->getDescriptor();
				// auto resize before padding
				if ( f->resizeAuto && s == "GLPadding" ) {
					GLResize *resize = new GLResize();
					QList<Effect*> el = resize->getMovitEffects();
					branch->filters.append( new MovitFilter( el, resize, true ) );
					for ( l = 0; l < el.count(); ++l )
						current = movitChain.chain->add_effect( el.at( l ) );
					f->resizeAuto = false;
				}
				QList<Effect*> el = dst->sample->frames[i - 1]->videoFilters[k]->getMovitEffects();
				branch->filters.append( new MovitFilter( el, dst->sample->frames[i - 1]->videoFilters[k] ) );
				for ( l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ) );
			}
			
			// auto resize to match destination aspect ratio
			if ( f->resizeAuto ) {
				GLResize *resize = new GLResize();
				QList<Effect*> el = resize->getMovitEffects();
				branch->filters.append( new MovitFilter( el, resize, true ) );
				for ( l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ) );
			}

			// padding
			if ( f->paddingAuto ) {
				GLPadding *padding = new GLPadding();
				QList<Effect*> el = padding->getMovitEffects();
				branch->filters.append( new MovitFilter( el, padding, true ) );
				for ( l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ) );
			}
					
			// compose
			if ( last ) {
				if ( !dst->sample->frames[i - 1]->composition ) {
					GLOverlay *overlay = new GLOverlay();
					Effect *e = overlay->getMovitEffect();
					branch->composition = new MovitComposition( e, overlay, true );
					current = movitChain.chain->add_effect( e, last, current );
				}
				else {
					Effect *e = dst->sample->frames[i - 1]->composition->getMovitEffect();
					branch->composition = new MovitComposition( e, dst->sample->frames[i - 1]->composition );
					current = movitChain.chain->add_effect( e, last, current );
				}
			}
		}

		movitChain.chain->set_dither_bits( 8 );
		ImageFormat output_format;
		output_format.color_space = COLORSPACE_REC_709;//COLORSPACE_sRGB;
		output_format.gamma_curve = GAMMA_REC_709;//GAMMA_sRGB;
		movitChain.chain->add_output( output_format, OUTPUT_ALPHA_FORMAT_POSTMULTIPLIED );
		movitChain.chain->finalize();
		
		reload = true;
	}

	// update inputs data and filters parameters
	i = start, j = 0;
	int w = projectProfile.getVideoWidth();
	int h = projectProfile.getVideoHeight();
	while ( (f = getNextFrame( dst, i )) ) {
		f->glWidth = f->profile.getVideoWidth();
		f->glHeight = f->profile.getVideoHeight();
		f->glSAR = f->profile.getVideoSAR();
		
		MovitBranch *branch = movitChain.branches[ j++ ];
		if ( reload )
			branch->input->process( f, &gl );
		for ( k = 0; k < branch->filters.count(); ++k ) {
			branch->filters[k]->filter->process( branch->filters[k]->effects, f, &projectProfile );
		}
		if ( branch->composition )
			branch->composition->composition->process( branch->composition->effect, f, f, &projectProfile );
		
		w = f->glWidth;
		h = f->glHeight;
	}

	// render
	FBO *fbo = gl.getFBO( w, h, GL_RGBA );
	movitChain.chain->render_to_fbo( fbo->fbo(), w, h );
	
	dst->glWidth = w;
	dst->glHeight = h;
	dst->glSAR = projectProfile.getVideoSAR();
	if ( !update ) {
		dst->setVideoFrame( Frame::GLTEXTURE, w, h, dst->glSAR,
						projectProfile.getVideoInterlaced(), projectProfile.getVideoTopFieldFirst(),
						sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
	}
	dst->setFBO( fbo );
	dst->setFence( gl.getFence() );
	glFlush();
	
	//qDebug() << "elapsed" << time.elapsed();
}



bool Composer::renderAudioFrame( Frame *dst, int nSamples )
{
    int i = 0, k;
    Frame *f;

    if ( !sampler->getAudioTracks( dst, nSamples ) ) {
		// make silence
		Profile profile = sampler->getProfile();
		int bps = profile.getAudioChannels() * profile.bytesPerChannel( &profile );
		dst->setAudioFrame( profile.getAudioChannels(), profile.getAudioSampleRate(), profile.bytesPerChannel( &profile ), nSamples, sampler->currentPTSAudio() );
		memset( dst->data(), 0, nSamples * bps );
		return true;
    }
    
    // process first frame
    f = getNextFrame( dst, i );
    for ( k = 0; k < dst->sample->frames[i - 1]->audioFilters.count(); ++k )
        dst->sample->frames[i - 1]->audioFilters[k]->process( f );
    // copy in dst
    AudioCopy ac;
    ac.process( f, dst );

    // process remaining frames
    while ( (f = getNextFrame( dst, i )) ) {
        for ( k = 0; k < dst->sample->frames[i - 1]->audioFilters.count(); ++k )
            dst->sample->frames[i - 1]->audioFilters[k]->process( f );
        // mix
        AudioMix am;
        am.process( f, dst );
    }

    return true;
}



Frame* Composer::getNextFrame( Frame *dst, int &track )
{
    Frame *f;

    while ( track < dst->sample->frames.count() ) {
        if ( ( f = dst->sample->frames[track++]->frame ) ) {
            return f;
        }
    }

    return NULL;
}
