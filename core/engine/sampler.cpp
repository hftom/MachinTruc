#include "engine/composer.h"
#include "engine/sampler.h"

#define MAXINPUTS 20



Sampler::Sampler()
{	
	playMode = PlaySource;

	projectProfile.setVideoFrameRate( 25 );
	projectProfile.setVideoFrameDuration( MICROSECOND / projectProfile.getVideoFrameRate() );
	projectProfile.setVideoWidth( 1920 );
	projectProfile.setVideoHeight( 1080 );
	projectProfile.setVideoSAR( 1 );
	projectProfile.setVideoInterlaced( false );
	projectProfile.setVideoTopFieldFirst( true );
	projectProfile.setAudioSampleRate( DEFAULTSAMPLERATE );
	projectProfile.setAudioChannels( DEFAULTCHANNELS );
	projectProfile.setAudioLayout( DEFAULTLAYOUT );
	
	setProfile( projectProfile );
	
	scene = new Scene( projectProfile );
	sceneList.append( scene );
	
	scene->tracks.append( new Track() );
	scene->tracks.append( new Track() );
	scene->tracks.append( new Track() );
	scene->tracks.append( new Track() );
	scene->tracks.append( new Track() );
	//scene->tracks.append( new Track() );
	
	InputFF *in = new InputFF();
	Profile prof;
	QString path = "/partage/SD66/2012-04-21-20ans_amandine/00001.mkv";
	in->probe( path, &prof );
	Source *source = new Source( InputBase::FFMPEG, path, prof );
	Clip *clip = scene->createClip( source, 0, prof.getStreamStartTime(), prof.getStreamDuration() );
	scene->addClip( clip, 0 );
	scene->tracks.first()->insertTransition( new Transition( 8 * MICROSECOND, prof.getStreamDuration() - (8 * MICROSECOND) ) );
	path = "/partage/Films/burn_after_reading.vob";//"/partage/SD66/2012-04-21-20ans_amandine/00005.mkv";
	in->probe( path, &prof );
	delete in;
	source = new Source( InputBase::FFMPEG, path, prof );
	clip = scene->createClip( source, 8 * MICROSECOND, prof.getStreamStartTime(), prof.getStreamDuration() );
	scene->addClip( clip, 0 );
	
	InputImage *im = new InputImage();
	path = "/home/cris/title.png";
	im->probe( path, &prof );
	source = new Source( InputBase::IMAGE, path, prof );
	clip = scene->createClip( source, 0, 0, 40000 * 3 );
	scene->addClip( clip, 2 );
	path = "/home/cris/titre.png";
	im->probe( path, &prof );
	delete im;
	source = new Source( InputBase::IMAGE, path, prof );
	clip = scene->createClip( source, 40000, 0, 40000 * 3 );
	scene->addClip( clip, 2 );
	scene->tracks.at( 2 )->insertTransition( new Transition( clip->position(), 40000 * 2 ) );
	
	metronom = new Metronom();
	composer = new Composer( this );
	connect( composer, SIGNAL(newFrame(Frame*)), this, SIGNAL(newFrame(Frame*)) );
	connect( composer, SIGNAL(paused(bool)), this, SIGNAL(paused(bool)) );
	connect( metronom, SIGNAL(discardFrame()), composer, SLOT(discardFrame()) );
}



Sampler::~Sampler()
{
}



void Sampler::switchMode( bool down )
{
	if ( (down ? PlayScene : PlaySource) == playMode )
		return;

	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}

	if ( down ) {
		playMode = PlayScene;
		rewardPTS();
		seekTo( currentPTS() );
	}
	else {
		playMode = PlaySource;
		emit modeSwitched();
	}	
}



void Sampler::setSource( Source *source, double pts )
{
	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}

	preview.play( false );
	metronom->flush();
	playMode = PlaySource;

	preview.setSource( source, getInput( source->getFileName(), source->getType() ) );
	seekTo( pts );
}



void Sampler::setProfile( Profile p )
{
	projectProfile = p;
}



Profile Sampler::getProfile()
{
	if ( playMode == PlaySource )
		return preview.getProfile();
	
	return projectProfile;
}



void Sampler::setSharedContext( QGLWidget *shared )
{	
	composer->setSharedContext( shared );
}



void Sampler::setFencesContext( QGLWidget *shared )
{	
	metronom->setSharedContext( shared );
}



void Sampler::play( bool b )
{
	if ( playMode == PlaySource && !preview.isValid() ) {
		emit paused( true );
		return;
	}

	composer->play( b );
}



void Sampler::updateFrame()
{
	if ( composer->isRunning() )
		return;
	if ( playMode == PlaySource && !preview.isValid() )
		return;
	Frame *last = metronom->getLastFrame();
	if ( !last )
		return;
	
	if ( updateLastFrame( last ) ) {
		metronom->flush();
		composer->updateFrame( last );
	}
	else {
		metronom->flush();
		seekTo( last->pts() );
	}		
}



void Sampler::wheelSeek( int a )
{
	if ( composer->isRunning() )
		return;
	if ( playMode == PlaySource && !preview.isValid() )
		return;

	printf("running=%d, a=%d\n", composer->isRunning(), a);

	if ( a == 1 ) {
		if ( !metronom->videoFrames.isEmpty() ) {
			Frame *f = metronom->videoFrames.dequeue();
			emit newFrame( f );
		}
		else {
			metronom->flush();
			composer->runOneShot();
		}
	}
	else {
		Frame *last = metronom->getLastFrame();
		if ( !last )
			return;
		metronom->flush();
		printf("wheelSeek PTS=%f\n", last->pts() + (last->profile.getVideoFrameDuration() * a) );
		seekTo( last->pts() + (last->profile.getVideoFrameDuration() * a) );
	}
}



void Sampler::slideSeek( double p )
{
	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}
	seekTo( p );
}



void Sampler::seekTo( double p )
{
	int i, j;

	metronom->flush();
	
	if ( playMode == PlaySource ) {
		if ( !preview.seekTo( p ) )
			return;
	}
	else {
		if ( p < 0 ) {
			if ( scene->currentPTS == 0 )
				return;
			p = 0;
		}
		for ( j = 0; j < scene->tracks.count(); ++j ) {
			for ( i = 0 ; i < scene->tracks[j]->clipCount(); ++i )
				scene->tracks[j]->clipAt( i )->setInput( NULL );
			scene->tracks[j]->resetIndexes();
		}
		scene->currentPTS = p;
		scene->currentPTSAudio = p;
	}
	
	composer->seeking();
}



double Sampler::getEndPTS()
{
	if ( playMode == PlaySource )
		return preview.endPTS();
	
	return samplerDuration();
}



double Sampler::getSamplerDuration()
{
	if ( playMode == PlaySource )
		return preview.duration();

	return samplerDuration();
}



double Sampler::samplerDuration()
{
	int i;
	double duration = 0;
	for ( i = 0; i < scene->tracks.count(); ++i ) {
		if ( !scene->tracks[ i ]->clipCount() )
			continue;
		Clip *c = scene->tracks[ i ]->clipAt( scene->tracks[ i ]->clipCount() - 1 );
		double d = c->position() + c->length();
		if ( d > duration )
			duration = d;
	}
	
	return duration;
}



double Sampler::currentPTS()
{
	if ( playMode == PlaySource )
		return preview.currentPTS;

	return scene->currentPTS;
}



double Sampler::currentPTSAudio()
{
	if ( playMode == PlaySource )
		return preview.currentPTSAudio;
	
	return scene->currentPTSAudio;
}



void Sampler::shiftCurrentPTS( double d )
{
	if ( playMode == PlaySource ) {
		if ( d )
			preview.currentPTS = d + preview.getProfile().getVideoFrameDuration();
		else
			preview.currentPTS += preview.getProfile().getVideoFrameDuration();
	}
	else
		scene->currentPTS +=  projectProfile.getVideoFrameDuration();
}



void Sampler::shiftCurrentPTSAudio( double d )
{
	if ( playMode == PlaySource ) {
		if ( d )
			preview.currentPTSAudio = d + preview.getProfile().getVideoFrameDuration();
		else
			preview.currentPTSAudio += preview.getProfile().getVideoFrameDuration();
	}
	else
		scene->currentPTSAudio +=  projectProfile.getVideoFrameDuration();
}



void Sampler::rewardPTS()
{
	if ( playMode == PlaySource ) {
		preview.currentPTS -= preview.getProfile().getVideoFrameDuration();
		preview.currentPTSAudio = preview.currentPTS;
	}
	else {
		scene->currentPTS -=  projectProfile.getVideoFrameDuration();
		if ( scene->currentPTS < 0 )
			scene->currentPTS = 0;
		scene->currentPTSAudio = scene->currentPTS;
	}
}



InputBase* Sampler::getInput( QString fn, InputBase::InputType type )
{
	int j;
	InputBase *in = NULL, *candidate = NULL;

	for ( j = 0; j < inputs.count(); ++j ) {
		in = inputs.at( j );
		if ( !in->isUsed() && in->getType() == type ) {
			if ( in->getSource() == fn )
				break;
			else if ( !candidate )
				candidate = in;
		}
		in = NULL;
	}
	if ( !in && candidate )
		in = candidate;
	if ( !in ) {
		switch ( type ) {
			case InputBase::FFMPEG:
				in = new InputFF();
				break;
			case InputBase::OPENGL:
				in = new InputGL();
				break;
			case InputBase::IMAGE:
			default:
				in = new InputImage();
				break;
		}
		inputs.append( in );
	}
	return in;
}



InputBase* Sampler::getClipInput( Clip *c, double pts )
{
	InputBase *in = getInput( c->getSource()->getFileName(), c->getSource()->getType() );
	double pos = c->start();
	if ( c->position() < pts )
		pos += pts - c->position();
	printf("%s cpos:%f, cstart:%f, seek:%f\n", c->sourcePath().toLatin1().data(), c->position(), c->start(), pos);
	Profile p = c->getProfile();
	p.setVideoFrameRate( projectProfile.getVideoFrameRate() );
	p.setVideoFrameDuration( projectProfile.getVideoFrameDuration() );
	p.setAudioSampleRate( projectProfile.getAudioSampleRate() );
	p.setAudioChannels( projectProfile.getAudioChannels() );
	p.setAudioLayout( projectProfile.getAudioLayout() );
	p.setAudioFormat( projectProfile.getAudioFormat() );
	in->setProfile( c->getProfile(), p );
	in->openSeekPlay( c->sourcePath(), pos );
	c->setInput( in );

	return in;
}



Transition* Sampler::getVideoTransition( int track, double pts )
{
	int i;
	Transition *trans;
	Track *t = scene->tracks[track];
	double margin = projectProfile.getVideoFrameDuration() / 4.0;

	for ( i = t->currentTransitionIndex() ; i < t->transitionCount(); ++i ) {
		trans = t->transitionAt( i );
		if ( trans->position() - margin <= pts ) {
			if ( trans->position() + trans->length() - margin > pts ) {
				t->setCurrentTransitionIndex( i );
				return trans;
			}
		}
		else
			break;
	}

	return NULL;
}



Transition* Sampler::getAudioTransition( int track, double pts )
{
	int i;
	Transition *trans;
	Track *t = scene->tracks[track];
	double margin = projectProfile.getVideoFrameDuration() / 4.0;

	for ( i = t->currentTransitionIndexAudio() ; i < t->transitionCount(); ++i ) {
		trans = t->transitionAt( i );
		if ( trans->position() - margin <= pts ) {
			if ( trans->position() + trans->length() - margin > pts ) {
				t->setCurrentTransitionIndexAudio( i );
				return trans;
			}
		}
		else
			break;
	}

	return NULL;
}



int Sampler::updateLastFrame( Frame *dst )
{
	int i, j;
	Clip *c = NULL;
	Transition *trans;
	int nframes = 0;
	double margin = projectProfile.getVideoFrameDuration() / 4.0;
	
	if ( playMode == PlaySource ) {
		if ( !preview.isValid() || !dst->sample->frames.count() )
			return 0;
		FrameSample *fs = dst->sample->frames.at( 0 );
		if ( !fs->frame )
			return 0;
		fs->clear( false );
		fs->videoFilters = preview.getSource()->videoFilters.copy();
		return 1;
	}

	QMutexLocker ml( &scene->mutex );
	
	for ( j = 0; j < scene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = scene->tracks[j];
		if ( scene->update )
			return 0;
		
		// find the clip at dst->pts
		for ( i = qMax(t->currentClipIndex() - 1, 0) ; i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - margin) <= dst->pts() ) {
				if ( (c->position() + c->length() - margin) > dst->pts() ) {
					// we have one
					break;
				}
				else
					c = NULL;
			}
			else {
				c = NULL;
				break;
			}
		}
		if ( dst->sample->frames.count() - 1 < j ) {
			return 0;
		}
		if ( c && c->getProfile().hasVideo() ) {
			FrameSample *fs = dst->sample->frames.at( j );
			if ( !fs->frame )
				return 0;
			++nframes;
			fs->clear( false );
			fs->videoFilters = c->getSource()->videoFilters.copy();
			fs->videoFilters.append( c->videoFilters.copy() );
			// Check for transition
			if ( i < t->clipCount() - 1 ) {
				c = t->clipAt( i + 1 );
				if ( (c->position() - margin) <= dst->pts() && (c->position() + c->length() - margin) > dst->pts() ) {
					if ( !fs->transitionFrame.frame )
						return 0;
					fs->transitionFrame.videoFilters = c->getSource()->videoFilters.copy();
					fs->transitionFrame.videoFilters.append( c->videoFilters.copy() );
					if ( (trans = getVideoTransition( j, dst->pts() )) )
						fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
				}
			}
		}
	}

	return nframes;
}



int Sampler::getVideoTracks( Frame *dst )
{
	int i, j;
	Frame *f;
	Clip *c = NULL;
	InputBase *in = NULL;
	Transition *trans;
	int nFrames = 0;
	double margin = projectProfile.getVideoFrameDuration() / 4.0;
	
	if ( playMode == PlaySource ) {
		if ( !preview.isValid() )
			return 0;
		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		f = preview.getInput()->getVideoFrame();
		fs->frame = f;
		fs->videoFilters = preview.getSource()->videoFilters.copy();
		return ( f ? 1 : 0 );
	}

	QMutexLocker ml( &scene->mutex );
	
	for ( j = 0; j < scene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = scene->tracks[j];
		if ( scene->update )
			t->resetIndexes();
		// find the clip at scene->currentPTS
		for ( i = t->currentClipIndex(); i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - margin) <= scene->currentPTS ) {
				if ( (c->position() + c->length() - margin) > scene->currentPTS ) {
					// we have one
					t->setCurrentClipIndex( i );
					break;
				}
				else
					c = NULL;
			}
			else {
				c = NULL;
				break;
			}
		}
		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		if ( c ) {
			if ( !(in = c->getInput()) )
				in = getClipInput( c, scene->currentPTS );
			f = in->getVideoFrame();
			if ( f ) {
				++nFrames;
				f->setPts( scene->currentPTS );
				fs->frame = f;
				fs->videoFilters = c->getSource()->videoFilters.copy();
				fs->videoFilters.append( c->videoFilters.copy() );
			}
			// Check for transition
			if ( i < t->clipCount() - 1 ) {
				c = t->clipAt( i + 1 );
				if ( (c->position() - margin) <= scene->currentPTS && (c->position() + c->length() - margin) > scene->currentPTS ) {
					if ( !(in = c->getInput()) )
					in = getClipInput( c, scene->currentPTS );
					f = in->getVideoFrame();
					if ( f ) {
						f->setPts( scene->currentPTS );
						fs->transitionFrame.frame = f;
						fs->transitionFrame.videoFilters = c->getSource()->videoFilters.copy();
						fs->transitionFrame.videoFilters.append( c->videoFilters.copy() );
						if ( (trans = getVideoTransition( j, scene->currentPTS )) )
							fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
					}
				}
			}
		}
	}

	scene->update = false;
	return nFrames;
}



int Sampler::getAudioTracks( Frame *dst, int nSamples )
{
	int i, j;
	Frame *f;
	Clip *c = NULL;
	InputBase *in = NULL;
	Transition *trans;
	int nFrames = 0;
	double margin = projectProfile.getVideoFrameDuration() / 4.0;
	
	if ( playMode == PlaySource ) {
		if ( !preview.isValid() )
			return 0;
		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		Frame *f = preview.getInput()->getAudioFrame( nSamples );
		fs->frame = f;
		fs->audioFilters = preview.getSource()->audioFilters.copy();
		return ( f ? 1 : 0 );
	}

	QMutexLocker ml( &scene->mutex );
	
	for ( j = 0; j < scene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = scene->tracks[j];
		if ( scene->update )
			t->resetIndexes();
		// find the clip at scene->currentPTSAudio
		for ( i = t->currentClipIndexAudio() ; i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - (projectProfile.getVideoFrameDuration() / 4.0)) <= scene->currentPTSAudio ) {
				if ( (c->position() + c->length() - (projectProfile.getVideoFrameDuration() / 4.0)) > scene->currentPTSAudio ) {
					// we have one
					t->setCurrentClipIndexAudio( i );
					break;
				}
				else
					c = NULL;
			}
			else {
				c = NULL;
				break;
			}
		}

		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		if ( c ) {
			if ( !(in = c->getInput()) )
				in = getClipInput( c, scene->currentPTSAudio );
			f = in->getAudioFrame( nSamples );
			if ( f ) {
				++nFrames;
				f->setPts( scene->currentPTSAudio );
				fs->frame = f;
				fs->audioFilters = c->getSource()->audioFilters.copy();
				fs->audioFilters.append( c->audioFilters.copy() );
			}
			// Check for transition
			if ( i < t->clipCount() - 1 ) {
				c = t->clipAt( i + 1 );
				if ( (c->position() - margin) <= scene->currentPTSAudio && (c->position() + c->length() - margin) > scene->currentPTSAudio ) {
					if ( !(in = c->getInput()) )
						in = getClipInput( c, scene->currentPTSAudio );
					f = in->getAudioFrame( nSamples );
					if ( f ) {
						f->setPts( scene->currentPTSAudio );
						fs->transitionFrame.frame = f;
						fs->transitionFrame.audioFilters = c->getSource()->audioFilters.copy();
						fs->transitionFrame.audioFilters.append( c->audioFilters.copy() );
						if ( (trans = getAudioTransition( j, scene->currentPTSAudio )) )
							fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
					}
				}
			}
		}
	}
	
	scene->update = false;
	return nFrames;
}



void Sampler::prepareInputs()
{
	int i, j;
	Clip *c = NULL;
	InputBase *in = NULL;
	double minPTS, maxPTS;
	
	if ( playMode == PlaySource )
		return;

	if ( scene->currentPTS < scene->currentPTSAudio ) {
		minPTS = scene->currentPTS;
		maxPTS = scene->currentPTSAudio;
	}
	else {
		minPTS = scene->currentPTSAudio;
		maxPTS = scene->currentPTS;
	}
	
	QMutexLocker ml( &scene->mutex );

	for ( j = 0; j < scene->tracks.count(); ++j ) {
		Track *t = scene->tracks[j];
		if ( scene->update )
			t->resetIndexes();

		if ( minPTS == scene->currentPTS )
			i = qMax( 0, t->currentClipIndex() - 1 );
		else
			i = qMax( 0, t->currentClipIndexAudio() - 1 );

		for ( ; i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() + c->length()) < minPTS ) {
				c->setInput( NULL );
			}
			else if ( c->position() <= (maxPTS + FORWARDLOOKUP) ) {
				//if ( inputs.count() >= MAXINPUTS )
					//break;
				if ( !(in = c->getInput()) ) {
					in = getClipInput( c, minPTS );
					break;
				}
			}
			else
				break;
		}
	}
	
	scene->update = false;
	//printf("******************************************** inputs=%d\n", inputs.count() );
}
