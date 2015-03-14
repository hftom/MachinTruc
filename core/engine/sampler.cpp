#include "engine/composer.h"
#include "engine/sampler.h"

#define MAXINPUTS 20



Sampler::Sampler()
	: playBackward( false ),
	bufferedPlaybackPts( -1 )
{	
	metronom = new Metronom( &playbackBuffer );
	composer = new Composer( this, &playbackBuffer );
	connect( composer, SIGNAL(newFrame(Frame*)), this, SIGNAL(newFrame(Frame*)) );
	connect( composer, SIGNAL(paused(bool)), this, SIGNAL(paused(bool)) );
	connect( metronom, SIGNAL(discardFrame(int)), composer, SLOT(discardFrame(int)) );

	Profile prof;

	preview = new Scene( prof );
	preview->tracks.append( new Track() );
	currentScene = preview;
	
	newProject( prof );
}



Sampler::~Sampler()
{
}



bool Sampler::isProjectEmpty()
{
	for ( int i = 0; i < sceneList.count(); ++i ) {
		Scene *s = sceneList[i];
		for ( int j = 0; j < s->tracks.count(); ++j ) {
			if ( s->tracks[j]->clipCount() )
				return false;
		}
	}
	return true;
}



bool Sampler::trackRequest( bool rm, int index )
{
	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}
		
	bool ok;
	if ( rm )
		ok = timelineScene->removeTrack( index );
	else
		ok = timelineScene->addTrack( index );
	
	if ( ok )
		updateFrame();
	return ok;
}


void Sampler::newProject( Profile p )
{
	drainScenes();
	
	QList<Scene*> list;
	Scene *s = new Scene( p );
	s->tracks.append( new Track() );
	s->tracks.append( new Track() );
	s->tracks.append( new Track() );
	list.append( s );
	
	setSceneList( list );
}



void Sampler::drainScenes()
{
	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}

	for ( int i = 0; i < sceneList.count(); ++i ) {
		sceneList[i]->drain();
	}
}



void Sampler::setSceneList( QList<Scene*> list )
{
	if ( !list.count() )
		return;

	while ( sceneList.count() ) {
		delete sceneList.takeFirst();
	}
	sceneList = list;
	timelineScene = sceneList.first();
	if ( currentScene != preview )
		currentScene = timelineScene;
	preview->drain();
}



bool Sampler::setProfile( Profile p )
{
	bool ok = true;

	for ( int i = 0; i < sceneList.count(); ++i ) {
		bool b = sceneList[i]->setProfile( p );
		if ( !b )
			ok = false;
	}

	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}
	seekTo( currentPTS() );
	
	return ok;
}



void Sampler::switchMode( bool down )
{
	if ( (down ? timelineScene : preview) == currentScene )
		return;

	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}

	if ( down ) {
		currentScene = timelineScene;
		rewardPTS();
		seekTo( currentPTS() );
	}
	else {
		currentScene = preview;
		emit modeSwitched();
	}	
}



void Sampler::setSource( Source *source, double pts )
{
	if ( composer->isRunning() ) {
		composer->play( false );
		emit paused( true );
	}

	metronom->flush();

	preview->drain();
	Profile p = source->getProfile();
	p.setAudioSampleRate( DEFAULTSAMPLERATE );
	p.setAudioChannels( DEFAULTCHANNELS );
	p.setAudioLayout( DEFAULTLAYOUT );
	p.setAudioFormat( Profile::SAMPLE_FMT_S16 );
	preview->setProfile( p );
	preview->addClip( preview->createClip( source, 0, p.getStreamStartTime(), p.getStreamDuration() ), 0 );
	qDebug() << "setSource" << p.getStreamStartTime() << pts;
	seekTo( pts );
}



Profile Sampler::getProfile()
{
	return currentScene->getProfile();
}



void Sampler::setSharedContext( QGLWidget *shared )
{	
	composer->setSharedContext( shared );
}



void Sampler::setFencesContext( QGLWidget *shared )
{	
	metronom->setSharedContext( shared );
}



void Sampler::setPlaybackBuffer( bool backward )
{
	metronom->flush();
	Frame *last = metronom->getLastFrame();
	double pts = currentPTS();
	if ( last ) {
		if ( playBackward )
			pts = last->pts() + last->profile.getVideoFrameDuration();
		else
			pts = last->pts() - last->profile.getVideoFrameDuration();
	}
	int bufferedSamples = playbackBuffer.getBuffer( pts, backward );
	if ( bufferedSamples > 0 ) {
		if ( backward )
			bufferedPlaybackPts = pts - ( bufferedSamples * currentScene->getProfile().getVideoFrameDuration() );
		else
			bufferedPlaybackPts = pts + ( bufferedSamples * currentScene->getProfile().getVideoFrameDuration() );
	}
	else
		bufferedPlaybackPts = -1;
	qDebug() << "bufferedPlaybackPts" << bufferedPlaybackPts;
	seekTo( pts, backward, false );
}



bool Sampler::play( bool b, bool backward )
{
	if ( b ) {	
		if ( playBackward != backward ) {
			if ( composer->isRunning() )
				composer->play( false );
			setPlaybackBuffer( backward );
		}
		else if ( composer->isRunning() )
			return false;
	}

	composer->play( b, backward );
	return b;
}



void Sampler::updateFrame()
{
	if ( composer->isRunning() )
		return;

	Frame *last = metronom->getLastFrame();
	if ( !last )
		return;
	
	if ( updateVideoFrame( last ) ) {
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

	if ( a == 1 || a == -1 ) {
		bool backward = a < 0;
		if ( playBackward != backward ) {
			setPlaybackBuffer( backward );
			composer->seeking();
		}
		else {
			bool shown = false;
			if ( !metronom->videoFrames.isEmpty() ) {
				do {
					Frame *f = metronom->videoFrames.dequeue();
					if ( f->type() == Frame::GLTEXTURE ) {
						emit newFrame( f );
						shown = true;
					}
					else
						playbackBuffer.releasedVideoFrame( f );
				} while ( !metronom->videoFrames.isEmpty() && !shown );
			}
			if ( !shown ) {
				metronom->flush();
				composer->runOneShot();
			}
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



void Sampler::seekTo( double p, bool backward, bool seek )
{
	int i, j;

	metronom->flush();

	if ( p < 0 ) {
		if ( currentScene->currentPTS == 0 )
			return;
		p = 0;
	}
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		for ( i = 0 ; i < currentScene->tracks[j]->clipCount(); ++i )
			currentScene->tracks[j]->clipAt( i )->setInput( NULL );
		currentScene->tracks[j]->resetIndexes( backward );
	}
	currentScene->currentPTS = p;
	currentScene->currentPTSAudio = p;
	
	playBackward = backward;
	
	if ( seek ) {
		double skipPts = -1;
		if ( metronom->getLastFrame() )
			skipPts = metronom->getLastFrame()->pts();
		playbackBuffer.reset( currentScene->getProfile().getVideoFrameRate() + 1, skipPts );
	}

	if ( !playBackward && seek )
		composer->seeking();
}



double Sampler::currentSceneDuration()
{
	return sceneDuration( currentScene );
}



double Sampler::currentTimelineSceneDuration()
{
	return sceneDuration( timelineScene );
}



double Sampler::sceneDuration( Scene *s )
{
	int i;
	double duration = 0;
	for ( i = 0; i < s->tracks.count(); ++i ) {
		if ( !s->tracks[ i ]->clipCount() )
			continue;
		Clip *c = s->tracks[ i ]->clipAt( s->tracks[ i ]->clipCount() - 1 );
		double d = c->position() + c->length();
		if ( d > duration )
			duration = d;
	}
	
	return duration;
}



bool Sampler::sceneEndReached()
{
	if ( playBackward )
		return currentPTS() < 0;
	return currentSceneDuration() - currentPTS() <  currentScene->getProfile().getVideoFrameDuration() / 2.0;
}



double Sampler::currentPTS()
{
	return currentScene->currentPTS;
}



double Sampler::currentPTSAudio()
{
	return currentScene->currentPTSAudio;
}



void Sampler::shiftCurrentPTS()
{
	if ( playBackward )
		currentScene->currentPTS -= currentScene->getProfile().getVideoFrameDuration();
	else
		currentScene->currentPTS += currentScene->getProfile().getVideoFrameDuration();
}



void Sampler::shiftCurrentPTSAudio()
{
	if ( playBackward )
		currentScene->currentPTSAudio -= currentScene->getProfile().getVideoFrameDuration();
	else
		currentScene->currentPTSAudio += currentScene->getProfile().getVideoFrameDuration();
}



void Sampler::rewardPTS()
{
	currentScene->currentPTS -= currentScene->getProfile().getVideoFrameDuration();
	if ( currentScene->currentPTS < 0 )
		currentScene->currentPTS = 0;
	currentScene->currentPTSAudio = currentScene->currentPTS;
}



InputBase* Sampler::getInput( QString fn, InputBase::InputType type )
{
	InputBase *in = NULL, *candidate = NULL;

	for ( int j = 0; j < inputs.count(); ++j ) {
		in = inputs.at( j );
		if ( !in->isUsed() && in->getType() == type ) {
			if ( in->getSource() == fn )
				return in;
			if ( !candidate )
				candidate = in;
		}
	}
	
	if ( candidate )
		return candidate;
	
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
	return in;
}



InputBase* Sampler::getClipInput( Clip *c, double pts )
{
	InputBase *in = getInput( c->getSource()->getFileName(), c->getSource()->getType() );
	double speed = c->getSpeed();
	double absSpeed = qAbs( speed );
	Profile p = c->getProfile();
	Profile cur = currentScene->getProfile();
	p.setAudioChannels( cur.getAudioChannels() );
	p.setAudioLayout( cur.getAudioLayout() );
	p.setAudioFormat( cur.getAudioFormat() );
	if ( speed != 1 ) {
		p.setVideoFrameRate( cur.getVideoFrameRate() / absSpeed );
		p.setVideoFrameDuration( MICROSECOND / p.getVideoFrameRate() );
		p.setAudioSampleRate( cur.getAudioSampleRate() / absSpeed );
	}
	else {
		p.setVideoFrameRate( cur.getVideoFrameRate() );
		p.setVideoFrameDuration( cur.getVideoFrameDuration() );
		p.setAudioSampleRate( cur.getAudioSampleRate() );
	}
	
	double pos;
	if ( playBackward ) {
		if ( speed < 0 ) {
			if ( pts < c->position() + c->length() )
				pos = c->start() + ((c->position() + c->length() - pts) * absSpeed);
			else
				pos = c->start();
		}
		else {
			if ( pts < c->position() + c->length() )
				pos = c->start() + ((pts - c->position()) * absSpeed);
			else
				pos = c->start() + (c->length() * absSpeed) - currentScene->getProfile().getVideoFrameDuration();
		}
	}
	else {
		if ( speed < 0 ) {
			pos = c->start() + (c->length() * absSpeed) - currentScene->getProfile().getVideoFrameDuration();
			if ( c->position() < pts )
				pos -= (pts - c->position()) * absSpeed;
		}
		else {
			pos = c->start();
			if ( c->position() < pts )
				pos += (pts - c->position()) * absSpeed;
		}
	}
	printf("%f %s cpos:%f, cstart:%f, seek:%f\n", pts, c->sourcePath().toLatin1().data(), c->position(), c->start(), pos);
	
	in->setSpeed( c->getSpeed() );
	in->setProfile( c->getProfile(), p );
	in->openSeekPlay( c->sourcePath(), pos, speed < 0 ? !playBackward : playBackward );
	c->setInput( in );

	return in;
}



Clip* Sampler::searchCurrentClip( int &i, Track *t, int clipIndex, double pts, double margin )
{
	Clip *c = NULL;
	
	if ( playBackward ) {
		for ( i = qMin( t->clipCount() - 1, clipIndex + 1 ); i >= 0; --i ) {
			c = t->clipAt( i );
			if ( (c->position() + c->length() - margin) > pts ) {
				if ( (c->position() - margin) <= pts ) {
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
	}
	else {
		for ( i = qMax(clipIndex - 1, 0); i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - margin) <= pts ) {
				if ( (c->position() + c->length() - margin) > pts ) {
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
	}
	
	return c;
}



void Sampler::getVideoTracks( Frame *dst )
{
	int i, j;
	Frame *f;
	Clip *c = NULL;
	InputBase *in = NULL;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;

	ProjectSample *ps = playbackBuffer.getVideoSample( currentScene->currentPTS );
	if ( ps ) {
		dst->sample = ps;
		dst->setPts( currentScene->currentPTS );
		updateVideoFrame( dst );
		return;
	}

	QMutexLocker ml( &currentScene->mutex );
	
	if ( dst->sample )
		delete dst->sample;
	dst->sample = new ProjectSample();
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes( playBackward );
		// find the clip at currentScene->currentPTS
		c = searchCurrentClip( i, t, t->currentClipIndex(), currentScene->currentPTS, margin );
		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		if ( c ) {
			t->setCurrentClipIndex( i );
			if ( !(in = c->getInput()) )
				in = getClipInput( c, currentScene->currentPTS );
			f = in->getVideoFrame();
			if ( f ) {
				f->setPts( currentScene->currentPTS );
				fs->frame = f;
				fs->videoFilters = c->getSource()->videoFilters.copy();
				fs->videoFilters.append( c->videoFilters.copy() );
			}
			// Check for transition
			if ( playBackward ? i > 0 : i < t->clipCount() - 1 ) {
				Clip *ct = playBackward ? t->clipAt( i - 1 ) : t->clipAt( i + 1 );
				if ( (ct->position() - margin) <= currentScene->currentPTS && (ct->position() + ct->length() - margin) > currentScene->currentPTS ) {
					if ( !(in = ct->getInput()) )
						in = getClipInput( ct, currentScene->currentPTS );
					f = in->getVideoFrame();
					if ( f ) {
						f->setPts( currentScene->currentPTS );
						if ( !playBackward ) {
							fs->transitionFrame.frame = f;
							fs->transitionFrame.videoFilters = ct->getSource()->videoFilters.copy();
							fs->transitionFrame.videoFilters.append( ct->videoFilters.copy() );
							Transition *trans;
							if ( (trans = ct->getTransition()) ) {
								if ( ct->position() + trans->length() + margin > currentScene->currentPTS )
									fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
							}
						}
						else {
							fs->transitionFrame.frame = fs->frame;
							fs->frame = f;
							fs->transitionFrame.videoFilters = fs->videoFilters;
							fs->videoFilters = ct->getSource()->videoFilters.copy();
							fs->videoFilters.append( ct->videoFilters.copy() );
							Transition *trans;
							if ( (trans = c->getTransition()) ) {
								if ( c->position() + trans->length() + margin > currentScene->currentPTS )
									fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
							}
						}
					}
				}
			}
		}
	}

	currentScene->update = false;
}



int Sampler::updateVideoFrame( Frame *dst )
{
	int i, j;
	Clip *c = NULL;
	int nframes = 0;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;
	
	if ( !dst->sample )
		return 0;

	QMutexLocker ml( &currentScene->mutex );
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			return 0;
		
		// find the clip at dst->pts
		c = searchCurrentClip( i, t, t->currentClipIndex(), dst->pts(), margin );
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
			if ( playBackward ? i > 0 : i < t->clipCount() - 1 ) {
				Clip *ct = playBackward ? t->clipAt( i - 1 ) : t->clipAt( i + 1 );
				if ( (ct->position() - margin) <= dst->pts() && (ct->position() + ct->length() - margin) > dst->pts() ) {
					if ( !fs->transitionFrame.frame )
						return 0;
					if ( !playBackward ) {
						fs->transitionFrame.videoFilters = ct->getSource()->videoFilters.copy();
						fs->transitionFrame.videoFilters.append( ct->videoFilters.copy() );
						Transition *trans;
						if ( (trans = ct->getTransition()) ) {
							if ( ct->position() + trans->length() + margin > dst->pts() )
								fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
						}
					}
					else {
						fs->transitionFrame.videoFilters = fs->videoFilters;
						fs->videoFilters = ct->getSource()->videoFilters.copy();
						fs->videoFilters.append( ct->videoFilters.copy() );
						Transition *trans;
						if ( (trans = c->getTransition()) ) {
							if ( c->position() + trans->length() + margin > dst->pts() )
								fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
						}
					}
				}
			}
		}
	}

	return nframes;
}



void Sampler::getAudioTracks( Frame *dst, int nSamples )
{
	int i, j;
	Frame *f;
	Clip *c = NULL;
	InputBase *in = NULL;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;
	
	ProjectSample *ps = playbackBuffer.getAudioSample( currentScene->currentPTSAudio );
	if ( ps ) {
		dst->sample = ps;
		dst->setPts( currentScene->currentPTSAudio );
		updateAudioFrame( dst );
		return;
	}

	QMutexLocker ml( &currentScene->mutex );
	
	if ( dst->sample )
		delete dst->sample;
	dst->sample = new ProjectSample();
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes( playBackward );
		// find the clip at currentScene->currentPTSAudio
		c = searchCurrentClip( i, t, t->currentClipIndexAudio(), currentScene->currentPTSAudio, margin );
		FrameSample *fs = new FrameSample();
		dst->sample->frames.append( fs );
		if ( c ) {
			t->setCurrentClipIndexAudio( i );
			if ( !(in = c->getInput()) )
				in = getClipInput( c, currentScene->currentPTSAudio );
			f = in->getAudioFrame( nSamples );
			if ( f ) {
				f->setPts( currentScene->currentPTSAudio );
				fs->frame = f;
				fs->audioFilters = c->getSource()->audioFilters.copy();
				fs->audioFilters.append( c->audioFilters.copy() );
			}
			// Check for transition
			if ( playBackward ? i > 0 : i < t->clipCount() - 1 ) {
				Clip *ct = playBackward ? t->clipAt( i - 1 ) : t->clipAt( i + 1 );
				Transition *trans = playBackward ? c->getTransition() : ct->getTransition();
				if ( trans && (ct->position() - margin) <= currentScene->currentPTSAudio && (ct->position() + ct->length() - margin) > currentScene->currentPTSAudio ) {
					if ( !(in = ct->getInput()) )
						in = getClipInput( ct, currentScene->currentPTSAudio );
					f = in->getAudioFrame( nSamples );
					if ( f )
						f->setPts( currentScene->currentPTSAudio );
					if ( !playBackward ) {
						fs->transitionFrame.frame = f;
						fs->transitionFrame.audioFilters = ct->getSource()->audioFilters.copy();
						fs->transitionFrame.audioFilters.append( ct->audioFilters.copy() );
						if ( ct->position() + trans->length() + margin > currentScene->currentPTSAudio )
							fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
					}
					else {
						fs->transitionFrame.frame = fs->frame;
						fs->frame = f;
						fs->transitionFrame.audioFilters = fs->audioFilters;
						fs->audioFilters = ct->getSource()->audioFilters.copy();
						fs->audioFilters.append( ct->audioFilters.copy() );
						if ( c->position() + trans->length() + margin > currentScene->currentPTSAudio )
							fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
					}
				}
			}
		}
	}
	
	currentScene->update = false;
}



void Sampler::updateAudioFrame( Frame *dst )
{
	int i, j;
	Clip *c = NULL;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;
	
	if ( !dst->sample )
		return;

	QMutexLocker ml( &currentScene->mutex );
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			return;
		// find the clip at currentScene->currentPTSAudio
		c = searchCurrentClip( i, t, t->currentClipIndexAudio(), dst->pts(), margin );
		if ( dst->sample->frames.count() - 1 < j ) {
			return;
		}
		if ( c && c->getProfile().hasAudio() ) {
			FrameSample *fs = dst->sample->frames.at( j );
			if ( !fs->frame )
				return;
			fs->clear( false );
			fs->audioFilters = c->getSource()->audioFilters.copy();
			fs->audioFilters.append( c->audioFilters.copy() );
			// Check for transition
			if ( playBackward ? i > 0 : i < t->clipCount() - 1 ) {
				Clip *ct = playBackward ? t->clipAt( i - 1 ) : t->clipAt( i + 1 );
				Transition *trans = playBackward ? c->getTransition() : ct->getTransition();
				if ( trans && (ct->position() - margin) <= dst->pts() && (ct->position() + ct->length() - margin) > dst->pts() ) {
					if ( !playBackward ) {
						fs->transitionFrame.audioFilters = ct->getSource()->audioFilters.copy();
						fs->transitionFrame.audioFilters.append( ct->audioFilters.copy() );
						if ( ct->position() + trans->length() + margin > dst->pts() )
							fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
					}
					else {
						fs->transitionFrame.audioFilters = fs->audioFilters;
						fs->audioFilters = ct->getSource()->audioFilters.copy();
						fs->audioFilters.append( ct->audioFilters.copy() );
						if ( c->position() + trans->length() + margin > dst->pts() )
							fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
					}
				}
			}
		}
	}
}



void Sampler::prepareInputs()
{
	if ( playBackward ) {
		prepareInputsBackward();
		return;
	}
	
	int i, j;
	Clip *c = NULL;
	InputBase *in = NULL;
	double minPTS, maxPTS;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;

	if ( bufferedPlaybackPts != -1 )
		minPTS = maxPTS = bufferedPlaybackPts;
	else if ( currentScene->currentPTS < currentScene->currentPTSAudio ) {
		minPTS = currentScene->currentPTS;
		maxPTS = currentScene->currentPTSAudio;
	}
	else {
		minPTS = currentScene->currentPTSAudio;
		maxPTS = currentScene->currentPTS;
	}
	
	QMutexLocker ml( &currentScene->mutex );

	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes( playBackward );

		if ( minPTS == currentScene->currentPTS )
			i = qMax( 0, t->currentClipIndex() - 1 );
		else
			i = qMax( 0, t->currentClipIndexAudio() - 1 );

		for ( ; i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() + c->length()) < minPTS - margin ) {
				c->setInput( NULL );
			}
			else if ( c->position() <= (maxPTS + FORWARDLOOKUP) ) {
				//if ( inputs.count() >= MAXINPUTS )
					//break;
				if ( !(in = c->getInput()) ) {
					in = getClipInput( c, minPTS );
					//break;
				}
			}
			else
				break;
		}
	}
	
	bufferedPlaybackPts = -1;
	currentScene->update = false;
	//printf("******************************************** inputs=%d\n", inputs.count() );
}



void Sampler::prepareInputsBackward()
{
	int i, j;
	Clip *c = NULL;
	InputBase *in = NULL;
	double minPTS, maxPTS;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;
	
	if ( bufferedPlaybackPts != -1 )
		minPTS = maxPTS = bufferedPlaybackPts;
	else if ( currentScene->currentPTS < currentScene->currentPTSAudio ) {
		minPTS = currentScene->currentPTS;
		maxPTS = currentScene->currentPTSAudio;
	}
	else {
		minPTS = currentScene->currentPTSAudio;
		maxPTS = currentScene->currentPTS;
	}
	
	QMutexLocker ml( &currentScene->mutex );

	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes( playBackward );

		if ( minPTS == currentScene->currentPTS )
			i = qMin( t->clipCount() - 1, t->currentClipIndex() + 1 );
		else
			i = qMin( t->clipCount() - 1, t->currentClipIndexAudio() + 1 );

		for ( ; i >= 0; --i ) {
			c = t->clipAt( i );
			if ( c->position() > maxPTS + margin ) {
				c->setInput( NULL );
			}
			else if ( (c->position() + c->length()) >= (minPTS - FORWARDLOOKUP) ) {
				//if ( inputs.count() >= MAXINPUTS )
					//break;
				if ( !(in = c->getInput()) ) {
					in = getClipInput( c, maxPTS );
					//break;
				}
			}
			else
				break;
		}
	}
	
	bufferedPlaybackPts = -1;
	currentScene->update = false;
	//printf("******************************************** inputs=%d\n", inputs.count() );
}
