#include <QApplication>
#include <QFile>
#include <QTime>

#include <movit/init.h>

#include "engine/composer.h"



Composer::Composer( Sampler *samp, PlaybackBuffer *pb )
	: playBackward( false ),
	running( false ),
	playing( false ),
	oneShot( false ),
	skipFrame( 0 ),
	hiddenContext( NULL ),
	composerFence( NULL ),
	movitPool( NULL ),
	sampler( samp ),
	playbackBuffer( pb ),
	audioSampleDelta( 0 )
{
}



Composer::~Composer()
{
	running = false;
	wait();
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

	hiddenContext->doneCurrent();

#if QT_VERSION >= 0x050000	
	hiddenContext->context()->moveToThread( this );
#endif
	running = true;
	start();
}



void Composer::discardFrame( int n )
{
	skipFrame += n;
}



bool Composer::isPlaying()
{
	return playing || lastMsg.msgType != ItcMsg::RENDERSTOP;
}



void Composer::play( bool b, bool backward )
{
	if ( b ) {
		sampler->getMetronom()->play( b, backward );
		itcMutex.lock();
		itcMsgList.append( ItcMsg( ItcMsg::RENDERPLAY ) );
		itcMutex.unlock();
		playing = true;
	}
	else {
		itcMutex.lock();
		itcMsgList.append( ItcMsg( ItcMsg::RENDERSTOP ) );
		itcMutex.unlock();
		while ( playing || lastMsg.msgType != ItcMsg::RENDERSTOP )
			usleep( 1000 );
	}
}



void Composer::setPlaybackBuffer( bool backward )
{
	itcMutex.lock();
	itcMsgList.append( ItcMsg( backward ) );
	itcMutex.unlock();
}



void Composer::seekTo( double p, bool backward, bool seek )
{
	itcMutex.lock();
	// remove previously queued RENDERSEEK msg ...
	while ( !itcMsgList.isEmpty() ) {
		if ( itcMsgList.last().msgType != ItcMsg::RENDERSEEK )
			break;
		itcMsgList.takeLast();
	}
	// ... and add this one
	itcMsgList.append( ItcMsg( p, backward, seek ) );
	itcMutex.unlock();
}



void Composer::frameByFrame()
{
	itcMutex.lock();
	// remove previously queued RENDERFRAMEBYFRAME msg ...
	while ( !itcMsgList.isEmpty() ) {
		if ( itcMsgList.last().msgType != ItcMsg::RENDERFRAMEBYFRAME )
			break;
		itcMsgList.takeLast();
	}
	// ... and add this one
	itcMsgList.append( ItcMsg( ItcMsg::RENDERFRAMEBYFRAME ) );
	itcMutex.unlock();
}



void Composer::frameByFrameSetPlaybackBuffer( bool backward )
{
	itcMutex.lock();
	itcMsgList.append( ItcMsg( ItcMsg::RENDERFRAMEBYFRAMESETPLAYBACKBUFFER, backward ) );
	itcMutex.unlock();
}



void Composer::skipBy( int step )
{
	itcMutex.lock();
	// remove previously queued RENDERSKIPBY msg ...
	while ( !itcMsgList.isEmpty() ) {
		if ( itcMsgList.last().msgType != ItcMsg::RENDERSKIPBY )
			break;
		itcMsgList.takeLast();
	}
	// ... and add this one
	itcMsgList.append( ItcMsg( ItcMsg::RENDERSKIPBY, step ) );
	itcMutex.unlock();
}



void Composer::updateFrame()
{
	itcMutex.lock();
	// remove previously queued RENDERUPDATE msg ...
	while ( !itcMsgList.isEmpty() ) {
		if ( itcMsgList.last().msgType != ItcMsg::RENDERUPDATE )
			break;
		itcMsgList.takeLast();
	}
	// ... and add this one
	itcMsgList.append( ItcMsg( ItcMsg::RENDERUPDATE ) );
	itcMutex.unlock();
}


#define PROCESSWAITINPUT 0
#define PROCESSCONTINUE 1
#define PROCESSEND 2
#define PROCESSONESHOTVIDEO 3

void Composer::run()
{
	int ret;
	Frame *f = NULL;
	ItcMsg lastMsg( ItcMsg::RENDERSTOP );

	hiddenContext->makeCurrent();

	while ( running ) {
		itcMutex.lock();
		if ( !itcMsgList.isEmpty() )
			lastMsg = itcMsgList.takeFirst();
		itcMutex.unlock();
		
		switch ( lastMsg.msgType ) {
			case ItcMsg::RENDERPLAY: {
				ret = process( &f );
				if ( ret == PROCESSEND ) {
					while ( !sampler->getMetronom()->videoFrames.queueEmpty() )
						usleep( 5000 );
					lastMsg.msgType = ItcMsg::RENDERSTOP;
					break;
				}
				else if ( ret == PROCESSWAITINPUT )
					usleep( 1000 );
				break;
			}
			case ItcMsg::RENDERSETPLAYBACKBUFFER: {
				double pts = sampler->fromComposerSetPlaybackBuffer( lastMsg.backward );
				sampler->fromComposerSeekTo( pts, lastMsg.backward, false );
				sampler->getMetronom()->play( true, lastMsg.backward );
				playing = true;
				skipFrame = 0;
				audioSampleDelta = 0;
				lastMsg.msgType = ItcMsg::RENDERPLAY;
				break;
			}
			case ItcMsg::RENDERFRAMEBYFRAMESETPLAYBACKBUFFER: {
				double pts = sampler->fromComposerSetPlaybackBuffer( lastMsg.backward );
				sampler->fromComposerSeekTo( pts, lastMsg.backward, false );
				runOneShot( f );
				lastMsg.msgType = ItcMsg::RENDERSTOP;
				break;
			}
			case ItcMsg::RENDERFRAMEBYFRAME: {
				bool shown = false;
				Metronom *m = sampler->getMetronom();
				if ( !m->videoFrames.isEmpty() ) {
					do {
						f = m->videoFrames.dequeue();
						if ( f->type() == Frame::GLTEXTURE ) {
							emit newFrame( f );
							shown = true;
						}
						else
							sampler->fromComposerReleaseVideoFrame( f );
					} while ( !m->videoFrames.isEmpty() && !shown );
				}
				if ( !shown ) {
					runOneShot( f );
				}
				lastMsg.msgType = ItcMsg::RENDERSTOP;
				break;
			}
			case ItcMsg::RENDERSKIPBY: {
				f = sampler->getMetronom()->getLastFrame();
				if ( f ) {
					sampler->fromComposerSeekTo( f->pts() + (f->profile.getVideoFrameDuration() * lastMsg.step) );
					runOneShot( f );
				}
				lastMsg.msgType = ItcMsg::RENDERSTOP;
				break;
			}
			case ItcMsg::RENDERSEEK: {
				sampler->fromComposerSeekTo( lastMsg.pts, lastMsg.backward, lastMsg.seek );
				runOneShot( f );
				lastMsg.msgType = ItcMsg::RENDERSTOP;
				break;
			}
			case ItcMsg::RENDERUPDATE: {
				f = sampler->getMetronom()->getAndLockLastFrame();
				Frame *dst = sampler->getMetronom()->freeVideoFrames.dequeue();
				if ( f && dst ) {
					dst->sample = new ProjectSample();
					dst->sample->copyVideoSample( f->sample );
					dst->setPts( f->pts() );
					if ( sampler->fromComposerUpdateFrame( dst ) ) {
						sampler->getMetronom()->unlockLastFrame();
						movitRender( dst, true );
						if ( dst->fence() )
							glClientWaitSync( dst->fence()->fence(), 0, GL_TIMEOUT_IGNORED );
						dst->isDuplicate = true;
						emit newFrame( dst );
					}
					else {
						sampler->getMetronom()->unlockLastFrame();
						dst->release();
						sampler->fromComposerSeekTo( f->pts() );
						f = NULL;
						runOneShot( f );
					}
				}
				else {
					sampler->getMetronom()->unlockLastFrame();
					if ( dst )
						dst->release();
				}
				lastMsg.msgType = ItcMsg::RENDERSTOP;
				break;
			}
			default: {
				if ( playing ) {
					sampler->getMetronom()->play( false );
					playing = false;
					emit paused( true );
					skipFrame = 0;
					audioSampleDelta = 0;
				}
				usleep( 1000 );
			}
		}
	}

	hiddenContext->doneCurrent();
#if QT_VERSION >= 0x050000
	hiddenContext->context()->moveToThread( qApp->thread() );
#endif	
}



void Composer::runOneShot( Frame *f )
{
	oneShot = true;
	int ret = process( &f );
	oneShot = false;
	if ( (ret == PROCESSONESHOTVIDEO) && f ) {
		if ( f->fence() )
			glClientWaitSync( f->fence()->fence(), 0, GL_TIMEOUT_IGNORED );
		emit newFrame( f );
	}
}



int Composer::process( Frame **frame )
{
	Frame *dst, *dsta;
	bool video = false;
	int ret = PROCESSWAITINPUT;
	
	Profile projectProfile = sampler->getProfile();

	if ( sampler->sceneEndReached() )
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
				playbackBuffer->releasedAudioFrame( dsta );
			else {
				dsta->setPts( sampler->currentPTSAudio() );
				sampler->getMetronom()->audioFrames.enqueue( dsta );
			}
			sampler->shiftCurrentPTSAudio();
			ret = PROCESSCONTINUE;
		}
	}

	if ( video ) {
		sampler->shiftCurrentPTS();
	}
	
	if ( !oneShot && ret == PROCESSCONTINUE )
		sampler->prepareInputs();

	if ( video ) {
		if ( oneShot ) {
			ret = PROCESSONESHOTVIDEO;
			*frame = dst;
		}
		else {
			sampler->getMetronom()->videoFrames.enqueue( dst );
		}
	}

	return ret;
}



void Composer:: waitFence()
{
	if ( composerFence ) {
		glClientWaitSync( composerFence->fence(), 0, GL_TIMEOUT_IGNORED );
		composerFence->setFree();
		composerFence = NULL;
	}
}



bool Composer::renderVideoFrame( Frame *dst )
{
	int i = 0;
	sampler->getVideoTracks( dst );
	if ( !getNextFrame( dst, i ) ) {
		Profile projectProfile = sampler->getProfile();
		
		if ( skipFrame > 0 ) {
			//qDebug() << "skipFrame" << sampler->currentPTS();
			--skipFrame;
			dst->setVideoFrame( Frame::NONE, projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), projectProfile.getVideoSAR(),
								false, false, sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
			return true;
		}
		
		// make black
		dst->setVideoFrame( Frame::GLTEXTURE, projectProfile.getVideoWidth(), projectProfile.getVideoHeight(), projectProfile.getVideoSAR(),
							false, false, sampler->currentPTS(), projectProfile.getVideoFrameDuration() );
		waitFence();
		if ( !gl.black( dst ) )
			return false;
		dst->glWidth = dst->profile.getVideoWidth();
		dst->glHeight = dst->profile.getVideoHeight();
		dst->glSAR = dst->profile.getVideoSAR();
		dst->setFence( gl.getFence() );
		composerFence = gl.getFence();
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
			//qDebug() << "skipFrame" << sampler->currentPTS();
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
	double pts = sampler->currentPTS();
	f->paddingAuto = f->resizeAuto = false;
	f->glWidth = f->profile.getVideoWidth();
	f->glHeight = f->profile.getVideoHeight();
	f->glSAR = f->profile.getVideoSAR();
		
	desc.append( prefix + MovitInput::getDescriptor( f ) );
	
	// correct orientation
	if ( f->orientation() ) {
		desc.append( prefix + GLOrientation().getDescriptor( pts, f, projectProfile ) );
	}
		
	for ( int k = 0; k < filters->count(); ++k ) {
		desc.append( prefix + filters->at(k)->getDescriptor( pts, f, projectProfile ) );
	}

	// resize to match destination aspect ratio and/or output size
	if ( !sampler->previewMode() ) {
		if ( fabs( projectProfile->getVideoSAR() - f->glSAR ) > 1e-3
			|| f->glWidth > projectProfile->getVideoWidth()
			|| f->glHeight > projectProfile->getVideoHeight()
			|| ( f->glWidth != projectProfile->getVideoWidth() &&
			f->glHeight != projectProfile->getVideoHeight() ) )
		{		
			desc.append( prefix + resizeFilter.getDescriptor( pts, f, projectProfile ) );
			f->resizeAuto = true;
		}
	}

	// padding
	if ( !sampler->previewMode() ) {
		if ( f->glWidth != projectProfile->getVideoWidth() || f->glHeight != projectProfile->getVideoHeight() ) {
			desc.append( prefix + paddingFilter.getDescriptor( pts, f, projectProfile ) );
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
	double pts = sampler->currentPTS();
	QStringList currentDescriptor;
	int ow = projectProfile.getVideoWidth();
	int oh = projectProfile.getVideoHeight();
	while ( (f = getNextFrame( dst, i )) ) {
		FrameSample *sample = dst->sample->frames[i - 1];
		// input and filters
		movitFrameDescriptor( "-", f, &sample->videoFilters, currentDescriptor, &projectProfile );
		// transition
		if ( sample->transitionFrame.frame && !sample->transitionFrame.videoTransitionFilter.isNull() ) {
			// filters applied on first transition frame, if any
			currentDescriptor.append( "--" + sample->transitionFrame.videoTransitionFilter->getDescriptorFirst( pts, f, &projectProfile ) );
			movitFrameDescriptor( "->", sample->transitionFrame.frame, &sample->transitionFrame.videoFilters, currentDescriptor, &projectProfile );
			// filters applied on second transition frame, if any
			currentDescriptor.append( "-->" + sample->transitionFrame.videoTransitionFilter->getDescriptorSecond( pts, sample->transitionFrame.frame, &projectProfile ) );
			currentDescriptor.append( "-<" + sample->transitionFrame.videoTransitionFilter->getDescriptor( pts, sample->transitionFrame.frame, &projectProfile ) );
		}
		// overlay
		if ( (i - 1) > start )
			currentDescriptor.append( GLOverlay().getDescriptor( pts, f, &projectProfile ) );
		ow = f->glWidth;
		oh = f->glHeight;
	}
	// background
	currentDescriptor.append( movitBackground.getDescriptor( pts, NULL, &projectProfile ) );
	// output
	currentDescriptor.append( QString("OUTPUT %1 %2").arg( ow ).arg( oh ) );

	// rebuild the chain if neccessary
	if ( currentDescriptor !=  movitChain.descriptor ) {
		for ( int k = 0; k < currentDescriptor.count(); k++ )
			printf("%s\n", currentDescriptor[k].toLocal8Bit().data());
		movitChain.descriptor = currentDescriptor;
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
				QList<Effect*> el = sample->transitionFrame.videoTransitionFilter->getMovitEffects();
				
				// filters applied on first transition frame, if any
				QList<Effect*> first = sample->transitionFrame.videoTransitionFilter->getMovitEffectsFirst();
				for ( int l = 0; l < first.count(); ++l )
					current = movitChain.chain->add_effect( first.at( l ) );
				
				MovitBranch *branchTrans;
				Effect *currentTrans = movitFrameBuild( sample->transitionFrame.frame, &sample->transitionFrame.videoFilters, &branchTrans );
				// filters applied on second transition frame, if any
				QList<Effect*> second = sample->transitionFrame.videoTransitionFilter->getMovitEffectsSecond();
				for ( int l = 0; l < second.count(); ++l )
					currentTrans = movitChain.chain->add_effect( second.at( l ) );
				
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
		f->glOVD = 0;
		f->glOVDTransformList.clear();
		
		// input and filters
		MovitBranch *branch = movitChain.branches[ j++ ];
		branch->input->process( f, &gl );
		int vf = 0;
		FrameSample *sample = dst->sample->frames[i - 1];
		for ( int k = 0; k < branch->filters.count(); ++k ) { 
			if ( !branch->filters[k]->filter )
				sample->videoFilters[vf++]->process( branch->filters[k]->effects, pts, f, &projectProfile );
			else
				branch->filters[k]->filter->process( branch->filters[k]->effects, pts, f, &projectProfile );
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
					sample->transitionFrame.videoFilters[tvf++]->process( branchTrans->filters[k]->effects, pts, sample->transitionFrame.frame, &projectProfile );
				else
					branchTrans->filters[k]->filter->process( branchTrans->filters[k]->effects, pts, sample->transitionFrame.frame, &projectProfile );
			}
			sample->transitionFrame.videoTransitionFilter->process( branchTrans->filters[k]->effects, pts, f, sample->transitionFrame.frame, &projectProfile );
		}
		// overlay
		if ( branch->overlay && branch->overlay->filter )
			branch->overlay->filter->process( branch->overlay->effects, pts, f, f, &projectProfile );
		
		w = f->glWidth;
		h = f->glHeight;
	}

	// render
	waitFence();
	FBO *fbo = gl.getFBO( w, h, GL_RGBA );
	movitChain.chain->render_to_fbo( fbo->fbo(), w, h );
	
	dst->glWidth = w;
	dst->glHeight = h;
	dst->glSAR = projectProfile.getVideoSAR();
	if ( !update ) {
		dst->setVideoFrame( Frame::GLTEXTURE, w, h, dst->glSAR,
						projectProfile.getVideoInterlaced(), projectProfile.getVideoTopFieldFirst(),
						pts, projectProfile.getVideoFrameDuration() );
	}
	dst->setFBO( fbo );
	dst->setFence( gl.getFence() );
	composerFence = gl.getFence();
	glFlush();
	
	//qDebug() << "elapsed" << time.elapsed();
}



Buffer* Composer::processAudioFrame( FrameSample *sample, int nsamples, int bitsPerSample, Profile *profile )
{
	Buffer *buf1 = NULL;
	if ( sample->frame ) {
		// filters
		buf1 = BufferPool::globalInstance()->getBuffer( nsamples * bitsPerSample );
		AudioCopy ac;
		ac.process( sample->frame, sample->frame->getBuffer(), buf1, profile );
		for ( int k = 0; k < sample->audioFilters.count(); ++k )
			sample->audioFilters[k]->process( sample->frame, buf1, buf1, profile );
	}
	// transition
	if ( !sample->transitionFrame.audioTransitionFilter.isNull() ) {
		Buffer *buf2 = NULL;
		if ( sample->transitionFrame.frame ) {
			buf2 = BufferPool::globalInstance()->getBuffer( nsamples * bitsPerSample );
			AudioCopy ac;
			ac.process( sample->transitionFrame.frame, sample->transitionFrame.frame->getBuffer(), buf2, profile );
			for ( int k = 0; k < sample->transitionFrame.audioFilters.count(); ++k )
				sample->transitionFrame.audioFilters[k]->process( sample->transitionFrame.frame, buf2, buf2, profile );
		}
		sample->transitionFrame.audioTransitionFilter->process( sample->frame, buf1, sample->transitionFrame.frame, buf2, buf2 ? buf2 : buf1, profile );
		if ( buf2 ) {
			if ( buf1 )
				BufferPool::globalInstance()->releaseBuffer( buf1 );
			return buf2;
		}
		else
			return buf1;			
	}
	
	return buf1;
}



bool Composer::renderAudioFrame( Frame *dst, int nSamples )
{
	int i = 0;
	Frame *f;
	FrameSample *sample;
	
	Profile profile = sampler->getProfile();
	int bps = profile.getAudioChannels() * profile.bytesPerChannel( &profile );

	sampler->getAudioTracks( dst, nSamples );
	if ( !getNextAudioFrame( dst, i ) ) {
		// make silence
		dst->setAudioFrame( profile.getAudioChannels(), profile.getAudioSampleRate(), profile.bytesPerChannel( &profile ), nSamples, sampler->currentPTSAudio() );
		memset( dst->data(), 0, nSamples * bps );
		return true;
	}
	
	// process first frame
	sample = dst->sample->frames[i - 1];
	// copy in dst
	AudioCopy ac;
	f = sample->frame;
	if ( !f )
		f = sample->transitionFrame.frame;
	nSamples = f->audioSamples();
	Buffer *buf = processAudioFrame( sample, nSamples, bps, &profile );
	dst->setAudioFrame( f->profile.getAudioChannels(), f->profile.getAudioSampleRate(), f->profile.bytesPerChannel( &f->profile ), nSamples, f->pts() );
	ac.process( f, buf, dst->getBuffer(), &profile );
	BufferPool::globalInstance()->releaseBuffer( buf );

	// process remaining frames
	AudioMix am;
	while ( getNextAudioFrame( dst, i ) ) {
		sample = dst->sample->frames[i - 1];
		buf = processAudioFrame( sample, nSamples, bps, &profile );
		// mix
		f = sample->frame;
		if ( !f )
			f = sample->transitionFrame.frame;
		am.process( f, buf, dst, dst->getBuffer(), dst->getBuffer(), &profile );
		BufferPool::globalInstance()->releaseBuffer( buf );
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



bool Composer::getNextAudioFrame( Frame *dst, int &track )
{
	while ( track < dst->sample->frames.count() ) {
		if ( dst->sample->frames[track]->frame || dst->sample->frames[track]->transitionFrame.frame ) {
			++track;
			return true;
		}
		++track;
	}

	return false;
}
