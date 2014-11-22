#include "engine/composer.h"
#include "engine/sampler.h"

#define MAXINPUTS 20



Sampler::Sampler()
	: playBackward( false )
{	
	metronom = new Metronom();
	composer = new Composer( this );
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



bool Sampler::play( bool b, bool backward )
{
	if ( b ) {	
		if ( playBackward != backward ) {
			if ( composer->isRunning() )
				composer->play( false );
			metronom->flush();
			Frame *last = metronom->getLastFrame();
			double pts = currentPTS();
			if ( last ) {
				if ( playBackward )
					pts = last->pts() + last->profile.getVideoFrameDuration();
				else
					pts = last->pts() - last->profile.getVideoFrameDuration();
			}
			seekTo( pts, backward );
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

	if ( a == 1 ) {
		bool shown = false;
		if ( !metronom->videoFrames.isEmpty() ) {
			do {
				Frame *f = metronom->videoFrames.dequeue();
				if ( f->type() == Frame::GLTEXTURE ) {
					emit newFrame( f );
					shown = true;
				}
				else
					f->release();
			} while ( !metronom->videoFrames.isEmpty() && !shown );
		}
		if ( !shown ) {
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



void Sampler::seekTo( double p, bool backward )
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
		currentScene->tracks[j]->resetIndexes();
	}
	currentScene->currentPTS = p;
	currentScene->currentPTSAudio = p;
	
	playBackward = backward;

	if ( !playBackward )
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
	Profile cur = currentScene->getProfile();
	p.setVideoFrameRate( cur.getVideoFrameRate() );
	p.setVideoFrameDuration( cur.getVideoFrameDuration() );
	p.setAudioSampleRate( cur.getAudioSampleRate() );
	p.setAudioChannels( cur.getAudioChannels() );
	p.setAudioLayout( cur.getAudioLayout() );
	p.setAudioFormat( cur.getAudioFormat() );
	in->setProfile( c->getProfile(), p );
	in->openSeekPlay( c->sourcePath(), pos, playBackward );
	c->setInput( in );

	return in;
}



int Sampler::updateLastFrame( Frame *dst )
{
	int i, j;
	Clip *c = NULL;
	int nframes = 0;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;

	QMutexLocker ml( &currentScene->mutex );
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
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
					Transition *trans;
					if ( (trans = c->getTransition()) ) {
						if ( c->position() + trans->length() + margin > dst->pts() )
							fs->transitionFrame.videoTransitionFilter = trans->getVideoFilter();
					}
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
	int nFrames = 0;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;

	QMutexLocker ml( &currentScene->mutex );
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes();
		// find the clip at currentScene->currentPTS
		for ( i = qMax(t->currentClipIndex() - 1, 0); i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - margin) <= currentScene->currentPTS ) {
				if ( (c->position() + c->length() - margin) > currentScene->currentPTS ) {
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
				in = getClipInput( c, currentScene->currentPTS );
			f = in->getVideoFrame();
			if ( f ) {
				++nFrames;
				f->setPts( currentScene->currentPTS );
				fs->frame = f;
				fs->videoFilters = c->getSource()->videoFilters.copy();
				fs->videoFilters.append( c->videoFilters.copy() );
			}
			// Check for transition
			if ( i < t->clipCount() - 1 ) {
				c = t->clipAt( i + 1 );
				if ( (c->position() - margin) <= currentScene->currentPTS && (c->position() + c->length() - margin) > currentScene->currentPTS ) {
					if ( !(in = c->getInput()) )
						in = getClipInput( c, currentScene->currentPTS );
					f = in->getVideoFrame();
					if ( f ) {
						f->setPts( currentScene->currentPTS );
						fs->transitionFrame.frame = f;
						fs->transitionFrame.videoFilters = c->getSource()->videoFilters.copy();
						fs->transitionFrame.videoFilters.append( c->videoFilters.copy() );
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

	currentScene->update = false;
	return nFrames;
}



int Sampler::getAudioTracks( Frame *dst, int nSamples )
{
	int i, j;
	Frame *f;
	Clip *c = NULL;
	InputBase *in = NULL;
	int nFrames = 0;
	double margin = currentScene->getProfile().getVideoFrameDuration() / 4.0;

	QMutexLocker ml( &currentScene->mutex );
	
	for ( j = 0; j < currentScene->tracks.count(); ++j ) {
		c = NULL;
		Track *t = currentScene->tracks[j];
		if ( currentScene->update )
			t->resetIndexes();
		// find the clip at currentScene->currentPTSAudio
		for ( i = qMax(t->currentClipIndexAudio() - 1, 0) ; i < t->clipCount(); ++i ) {
			c = t->clipAt( i );
			if ( (c->position() - margin) <= currentScene->currentPTSAudio ) {
				if ( (c->position() + c->length() - margin) > currentScene->currentPTSAudio ) {
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
				in = getClipInput( c, currentScene->currentPTSAudio );
			f = in->getAudioFrame( nSamples );
			if ( f ) {
				f->setPts( currentScene->currentPTSAudio );
				fs->frame = f;
				fs->audioFilters = c->getSource()->audioFilters.copy();
				fs->audioFilters.append( c->audioFilters.copy() );
			}
			// Check for transition
			if ( i < t->clipCount() - 1 ) {
				c = t->clipAt( i + 1 );
				Transition *trans = c->getTransition();
				if ( trans && (c->position() - margin) <= currentScene->currentPTSAudio
						&& (c->position() + c->length() - margin) > currentScene->currentPTSAudio ) {
					if ( !(in = c->getInput()) )
						in = getClipInput( c, currentScene->currentPTSAudio );
					f = in->getAudioFrame( nSamples );
					if ( f )
						f->setPts( currentScene->currentPTSAudio );
					fs->transitionFrame.frame = f;
					fs->transitionFrame.audioFilters = c->getSource()->audioFilters.copy();
					fs->transitionFrame.audioFilters.append( c->audioFilters.copy() );
					if ( c->position() + trans->length() + margin > currentScene->currentPTSAudio )
						fs->transitionFrame.audioTransitionFilter = trans->getAudioFilter();
				}
			}
			if ( fs->frame || fs->transitionFrame.frame )
				++nFrames;
		}
	}
	
	currentScene->update = false;
	return nFrames;
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

	if ( currentScene->currentPTS < currentScene->currentPTSAudio ) {
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
			t->resetIndexes();

		if ( minPTS == currentScene->currentPTS )
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
	
	currentScene->update = false;
	//printf("******************************************** inputs=%d\n", inputs.count() );
}



void Sampler::prepareInputsBackward()
{
	int i, j;
	Clip *c = NULL;
	InputBase *in = NULL;
	double minPTS, maxPTS;

	if ( currentScene->currentPTS < currentScene->currentPTSAudio ) {
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
			t->resetIndexes();

		if ( minPTS == currentScene->currentPTS )
			i = qMin( t->clipCount() - 1, t->currentClipIndex() + 1 );
		else
			i = qMin( t->clipCount() - 1, t->currentClipIndexAudio() + 1 );

		for ( ; i >= 0; --i ) {
			c = t->clipAt( i );
			if ( c->position() > maxPTS ) {
				c->setInput( NULL );
			}
			else if ( (c->position() + c->length()) >= (minPTS - FORWARDLOOKUP) ) {
				//if ( inputs.count() >= MAXINPUTS )
					//break;
				if ( !(in = c->getInput()) ) {
					in = getClipInput( c, maxPTS );
					break;
				}
			}
			else
				break;
		}
	}
	
	currentScene->update = false;
	//printf("******************************************** inputs=%d\n", inputs.count() );
}
