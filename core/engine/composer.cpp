#include <QApplication>
#include <QFile>
#include <QTime>

#include <movit/init.h>

#include "engine/composer.h"
#include "input/input_color.h"



Composer::Composer( Sampler *samp )
	: running( false ),
	oneShot( false ),
	skipFrame( 0 ),
	hiddenContext( NULL ),
	movitPool( NULL ),
	sampler( samp ),
	audioSampleDelta( 0 )
{
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
	
	assert( init_movit( movitPath.toLocal8Bit().data(), MOVIT_DEBUG_ON ) );
	
	movitPool = new ResourcePool( 100, 300 << 20, 100 );

	/*mask_texture = hiddenContext->bindTexture( QImage("/home/cris/mask.png") );
	glBindTexture( GL_TEXTURE_2D, mask_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	printf("mask_texture = %u\n", mask_texture);*/

	hiddenContext->doneCurrent();
}



void Composer::discardFrame( int n )
{
	skipFrame += n;
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
#if QT_VERSION >= 0x050000
		hiddenContext->context()->moveToThread( this );
#endif
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
			while ( running && !sampler->getMetronom()->videoFrames.queueEmpty() )
				usleep( 5000 );
			sampler->getMetronom()->play( false );
			emit paused( true );
			break;
		}
		
		if ( ret == PROCESSWAITINPUT )
			usleep( 1000 );
	}

	hiddenContext->doneCurrent();
#if QT_VERSION >= 0x050000
	hiddenContext->context()->moveToThread( qApp->thread() );
#endif	
}



int Composer::process( Frame **frame )
{
	Frame *dst, *dsta;
	bool video = false;
	int ret = PROCESSWAITINPUT;
	
	Profile projectProfile = sampler->getProfile();

	if ( ( sampler->getEndPTS() - sampler->currentPTS() ) <  ( projectProfile.getVideoFrameDuration() / 2.0 ) )
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
		Frame *f;
		bool skip = true;
		for ( int i = 0 ; i < dst->sample->frames.count(); ++i) {
			if ( (f = dst->sample->frames[i]->frame) ) {
				if ( f->mmi == 0 ) {
					skip = false;
					break;
				}
			}
		}
		if ( skip ) {
			qDebug() << "skipFrame" << sampler->currentPTS();
			--skipFrame;
			Profile projectProfile = sampler->getProfile();
			dst->setVideoFrame( Frame::NONE, projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), projectProfile.getVideoSAR(),
								false, false, sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
			return true;
		}
	}

	movitRender( dst );

	return true;
}



void Composer::movitFrameDescriptor( QString prefix, Frame *f, QList< QSharedPointer<GLFilter> > *filters, QStringList &desc, Profile *projectProfile )
{
	f->paddingAuto = f->resizeAuto = false;
	f->glWidth = f->profile.getVideoWidth();
	f->glHeight = f->profile.getVideoHeight();
	f->glSAR = f->profile.getVideoSAR();
		
	desc.append( prefix + MovitInput::getDescriptor( f ) );
	
	// deinterlace
	if ( f->profile.getVideoInterlaced() ) {
		desc.append( prefix + GLDeinterlace().getDescriptor( f, projectProfile ) );
	}
	
	// correct orientation
	if ( f->orientation() ) {
		desc.append( prefix + GLOrientation().getDescriptor( f, projectProfile ) );
	}
		
	for ( int k = 0; k < filters->count(); ++k ) {
		desc.append( prefix + filters->at(k)->getDescriptor( f, projectProfile ) );
	}

	// resize to match destination aspect ratio and/or output size
	if ( !sampler->previewMode() ) {
		if ( fabs( projectProfile->getVideoSAR() - f->glSAR ) > 1e-3
			|| f->glWidth > projectProfile->getVideoWidth()
			|| f->glHeight > projectProfile->getVideoHeight()
			|| ( f->glWidth != projectProfile->getVideoWidth() &&
			f->glHeight != projectProfile->getVideoHeight() ) )
		{		
			GLResize resize;
			desc.append( prefix + resize.getDescriptor( f, projectProfile ) );
			f->resizeAuto = true;
		}
	}

	// padding
	if ( !sampler->previewMode() ) {
		if ( f->glWidth != projectProfile->getVideoWidth() || f->glHeight != projectProfile->getVideoHeight() ) {
			GLPadding padding;
			desc.append( prefix + padding.getDescriptor( f, projectProfile ) );
			f->paddingAuto = true;
		}
	}
}



Effect* Composer::movitFrameBuild( Frame *f, QList< QSharedPointer<GLFilter> > *filters, MovitBranch **newBranch )
{
	Effect *current = NULL;

	MovitInput *in = new MovitInput();
	MovitBranch *branch = new MovitBranch( in );
	*newBranch = branch;
	movitChain.branches.append( branch );
	current = movitChain.chain->add_input( in->getMovitInput( f ) );
	
	// deinterlace
	if ( f->profile.getVideoInterlaced() ) {
		GLDeinterlace *deint = new GLDeinterlace();
		QList<Effect*> el = deint->getMovitEffects();
		branch->filters.append( new MovitFilter( el, deint ) );
		for ( int l = 0; l < el.count(); ++l )
			current = movitChain.chain->add_effect( el.at( l ) );
	}
	
	// correct orientation
	if ( f->orientation() ) {
		GLOrientation *orient = new GLOrientation();
		orient->setOrientation( f->orientation() );
		QList<Effect*> el = orient->getMovitEffects();
		branch->filters.append( new MovitFilter( el, orient ) );
		for ( int l = 0; l < el.count(); ++l )
			current = movitChain.chain->add_effect( el.at( l ) );
	}
	
	// apply filters
	for ( int k = 0; k < filters->count(); ++k ) {
		QList<Effect*> el = filters->at(k)->getMovitEffects();
		branch->filters.append( new MovitFilter( el ) );
		for ( int l = 0; l < el.count(); ++l )
			current = movitChain.chain->add_effect( el.at( l ) );
	}
	
	// auto resize to match destination aspect ratio
	if ( f->resizeAuto ) {
		GLResize *resize = new GLResize();
		QList<Effect*> el = resize->getMovitEffects();
		branch->filters.append( new MovitFilter( el, resize ) );
		for ( int l = 0; l < el.count(); ++l )
			current = movitChain.chain->add_effect( el.at( l ) );
	}

	// padding
	if ( f->paddingAuto ) {
		GLPadding *padding = new GLPadding();
		QList<Effect*> el = padding->getMovitEffects();
		branch->filters.append( new MovitFilter( el, padding ) );
		for ( int l = 0; l < el.count(); ++l )
			current = movitChain.chain->add_effect( el.at( l ) );
	}
	
	return current;
}



void Composer::movitRender( Frame *dst, bool update )
{
	int i, j, start=0;
	Frame *f;
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
	int ow = projectProfile.getVideoWidth();
	int oh = projectProfile.getVideoHeight();
	while ( (f = getNextFrame( dst, i )) ) {
		FrameSample *sample = dst->sample->frames[i - 1];
		// input and filters
		movitFrameDescriptor( "-", f, &sample->videoFilters, currentDescriptor, &projectProfile );
		// transition
		if ( sample->transitionFrame.frame && !sample->transitionFrame.videoTransitionFilter.isNull() ) {
			movitFrameDescriptor( "->", sample->transitionFrame.frame, &sample->transitionFrame.videoFilters, currentDescriptor, &projectProfile );
			currentDescriptor.append( "-<" + sample->transitionFrame.videoTransitionFilter->getDescriptor( sample->transitionFrame.frame, &projectProfile ) );
		}
		// overlay
		if ( (i - 1) > start )
			currentDescriptor.append( GLOverlay().getDescriptor( f, &projectProfile ) );
		ow = f->glWidth;
		oh = f->glHeight;
	}
	// background
	currentDescriptor.append( movitBackground.getDescriptor( NULL, &projectProfile ) );
	// output
	currentDescriptor.append( QString("OUTPUT %1 %2").arg( ow ).arg( oh ) );

	// rebuild the chain if neccessary
	if ( currentDescriptor !=  movitDescriptor ) {
		for ( int k = 0; k < currentDescriptor.count(); k++ )
			printf("%s\n", currentDescriptor[k].toLocal8Bit().data());
		movitDescriptor = currentDescriptor;
		movitChain.reset();
		movitChain.chain = new EffectChain( projectProfile.getVideoSAR() * projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), movitPool );

		i = start;
		Effect *last, *current = NULL;
		
		while ( (f = getNextFrame( dst, i )) ) {
			last = current;
			// input and filters
			MovitBranch *branch;
			FrameSample *sample = dst->sample->frames[i - 1];
			current = movitFrameBuild( f, &sample->videoFilters, &branch );
			// transition
			if ( sample->transitionFrame.frame && !sample->transitionFrame.videoTransitionFilter.isNull() ) {
				MovitBranch *branchTrans;
				Effect *currentTrans = movitFrameBuild( sample->transitionFrame.frame, &sample->transitionFrame.videoFilters, &branchTrans );
				QList<Effect*> el = sample->transitionFrame.videoTransitionFilter->getMovitEffects();
				branchTrans->filters.append( new MovitFilter( el ) );
				for ( int l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ), current, currentTrans );
			}
			// overlay
			if ( last ) {
				GLOverlay *overlay = new GLOverlay();
				QList<Effect*> el = overlay->getMovitEffects();
				branch->overlay = new MovitFilter( el, overlay );
				for ( int l = 0; l < el.count(); ++l )
					current = movitChain.chain->add_effect( el.at( l ), last, current );
			}
		}
		// background
		QList<Effect*> el = movitBackground.getMovitEffects();
		movitChain.chain->add_effect( el[0] );
		// output
		movitChain.chain->set_dither_bits( 8 );
		ImageFormat output_format;
		output_format.color_space = COLORSPACE_sRGB;
		output_format.gamma_curve = GAMMA_REC_709;
		movitChain.chain->add_output( output_format, OUTPUT_ALPHA_FORMAT_POSTMULTIPLIED );
		movitChain.chain->finalize();
	}

	// update inputs data and filters parameters
	i = start, j = 0;
	int w = projectProfile.getVideoWidth();
	int h = projectProfile.getVideoHeight();
	while ( (f = getNextFrame( dst, i )) ) {
		f->glWidth = f->profile.getVideoWidth();
		f->glHeight = f->profile.getVideoHeight();
		f->glSAR = f->profile.getVideoSAR();
		
		// input and filters
		MovitBranch *branch = movitChain.branches[ j++ ];
		branch->input->process( f, &gl );
		int vf = 0;
		FrameSample *sample = dst->sample->frames[i - 1];
		for ( int k = 0; k < branch->filters.count(); ++k ) { 
			if ( !branch->filters[k]->filter )
				sample->videoFilters[vf++]->process( branch->filters[k]->effects, f, &projectProfile );
			else
				branch->filters[k]->filter->process( branch->filters[k]->effects, f, &projectProfile );
		}
		// transition
		if ( sample->transitionFrame.frame && !sample->transitionFrame.videoTransitionFilter.isNull() ) {
			sample->transitionFrame.frame->glWidth = sample->transitionFrame.frame->profile.getVideoWidth();
			sample->transitionFrame.frame->glHeight = sample->transitionFrame.frame->profile.getVideoHeight();
			sample->transitionFrame.frame->glSAR = sample->transitionFrame.frame->profile.getVideoSAR();
			MovitBranch *branchTrans = movitChain.branches[ j++ ];
			branchTrans->input->process( sample->transitionFrame.frame, &gl );
			int tvf = 0;
			int k;
			for ( k = 0; k < branchTrans->filters.count() - 1; ++k ) { 
				if ( !branchTrans->filters[k]->filter )
					sample->transitionFrame.videoFilters[tvf++]->process( branchTrans->filters[k]->effects, sample->transitionFrame.frame, &projectProfile );
				else
					branchTrans->filters[k]->filter->process( branchTrans->filters[k]->effects, sample->transitionFrame.frame, &projectProfile );
			}
			sample->transitionFrame.videoTransitionFilter->process( branchTrans->filters[k]->effects, f, sample->transitionFrame.frame, &projectProfile );
		}
		// overlay
		if ( branch->overlay && branch->overlay->filter )
			branch->overlay->filter->process( branch->overlay->effects, f, f, &projectProfile );
		
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



void Composer::processAudioFrame( FrameSample *sample, Frame *f, Profile *profile )
{
	// filters
	for ( int k = 0; k < sample->audioFilters.count(); ++k )
		sample->audioFilters[k]->process( f, profile );
	// transition
	if ( sample->transitionFrame.frame && !sample->transitionFrame.audioTransitionFilter.isNull() ) {
		for ( int k = 0; k < sample->transitionFrame.audioFilters.count(); ++k )
			sample->transitionFrame.audioFilters[k]->process( sample->transitionFrame.frame, profile );
		sample->transitionFrame.audioTransitionFilter->process( f, sample->transitionFrame.frame, profile );
	}
}



bool Composer::renderAudioFrame( Frame *dst, int nSamples )
{
	int i = 0;
	Frame *f;
	FrameSample *sample;
	
	Profile profile = sampler->getProfile();

	if ( !sampler->getAudioTracks( dst, nSamples ) ) {
		// make silence
		int bps = profile.getAudioChannels() * profile.bytesPerChannel( &profile );
		dst->setAudioFrame( profile.getAudioChannels(), profile.getAudioSampleRate(), profile.bytesPerChannel( &profile ), nSamples, sampler->currentPTSAudio() );
		memset( dst->data(), 0, nSamples * bps );
		return true;
	}
	
	// process first frame
	f = getNextFrame( dst, i );
	sample = dst->sample->frames[i - 1];
	processAudioFrame( sample, f, &profile );
	// copy in dst
	AudioCopy ac;
	ac.process( f, dst, &profile );

	// process remaining frames
	AudioMix am;
	while ( (f = getNextFrame( dst, i )) ) {
		sample = dst->sample->frames[i - 1];
		processAudioFrame( sample, f, &profile );
		// mix
		am.process( f, dst, &profile );
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
