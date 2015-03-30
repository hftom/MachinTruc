#include "engine/util.h"
#include "engine/scene.h"
#include "engine/filtercollection.h"



Scene::Scene( Profile p )
	: update( true ),
	currentPTS( 0 ),
	currentPTSAudio( 0 ),
	profile( p )
{
}



Scene::~Scene()
{
	drain();
	while ( tracks.count() )
		delete tracks.takeFirst();
}



bool Scene::setProfile( Profile &p )
{
	bool ok = true;
	double duration = profile.getVideoFrameDuration();
	profile = p;
	double margin = profile.getVideoFrameDuration() / 4.0;
	
	if ( duration != profile.getVideoFrameDuration() ) {
		duration = profile.getVideoFrameDuration();
		for ( int i = 0; i < tracks.count(); ++i ) {
			Track *t = tracks[i];
			for ( int j = 0; j < t->clipCount(); ++j ) {
				Clip *c = t->clipAt( j );
				c->setFrameDuration( duration );
				double newPos = nearestPTS( c->position(), duration );
				if ( !c->getTransition() && j > 0 ) {
					Clip *prev = t->clipAt( j - 1 );
					if ( newPos < prev->position() + prev->length() - margin )
						newPos = prev->position() + prev->length();
					c->setPosition( newPos );
				}
				else if ( canMove( c, c->length(), newPos, i ) )
					move( c, i, newPos, i );
				else
					ok = false;
			}
		}
	}
	
	return ok;
}
	

	
Clip* Scene::createClip( Source *src, double posInTrackPTS, double strt, double len )
{
	Clip *clip = new Clip( src, posInTrackPTS, strt, len );
	clip->setFrameDuration( profile.getVideoFrameDuration() );
	return clip;
}



Clip* Scene::sceneSplitClip( Clip *clip, int track, double pts )
{
	double margin = profile.getVideoFrameDuration() / 4.0;
	pts = nearestPTS( pts, profile.getVideoFrameDuration() );

	if ( pts <= clip->position() + profile.getVideoFrameDuration() - margin )
		return NULL;
	if ( pts >= clip->position() + clip->length() - profile.getVideoFrameDuration() + margin )
		return NULL;

	double oldLength = clip->length();
	double newLength = pts - clip->position();
	if ( canResize( clip, newLength, track ) ) {
		Clip *nc = createClip( clip->getSource(), pts, clip->start() + newLength, oldLength - newLength );
		double newPos = nc->position();
		if ( canMove( nc, nc->length(), newPos, track ) ) {
			nc->setPosition( newPos );
			FilterCollection *fc = FilterCollection::getGlobalInstance();
			for ( int i = 0; i < clip->videoFilters.count(); ++i ) {
				QSharedPointer<GLFilter> f = clip->videoFilters.at( i );
				for ( int j = 0; j < fc->videoFilters.count(); ++j ) {
					if ( fc->videoFilters[ j ].identifier == f->getIdentifier() ) {
						QSharedPointer<Filter> nf = fc->videoFilters[ j ].create();
						GLFilter *gf = (GLFilter*)nf.data();
						f->splitParameters( gf, newLength );
						nf->setPosition( nc->position() );
						nf->setLength( nc->length() );
						nc->videoFilters.append( nf.staticCast<GLFilter>() );
					}
				}
			}
			resize( clip, newLength, track );
			addClip( nc, track );
			return nc;
		}
		else
			delete nc;
	}

	return NULL;
}



bool Scene::effectCanMove( Clip *clip, double &newPos, bool isVideo, int index )
{
	double margin = profile.getVideoFrameDuration() / 4.0;
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );

	if ( f->getSnap() == Filter::SNAPSTART || f->getSnap() == Filter::SNAPEND )
		return false;

	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	if ( newPos < clip->position() - margin )
		newPos = clip->position();
	if ( newPos + f->getLength() > clip->position() + clip->length() + margin )
		newPos = clip->position() + clip->length() - f->getLength();
	
	return true;
}



void Scene::effectMove( Clip *clip, double newPos, bool isVideo, int index )
{
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );

	f->setPositionOffset( newPos - f->getPosition() );
	if ( f->getSnap() == Filter::SNAPALL && f->getPositionOffset() != 0 )
		f->setSnap( Filter::SNAPNONE );
}



bool Scene::effectCanResizeStart( Clip *clip, double &newPos, double endPos, bool isVideo, int index )
{
	double margin = profile.getVideoFrameDuration() / 4.0;
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );
	
	if ( f->getSnap() == Filter::SNAPSTART )
		return false;
	
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	if ( newPos < clip->position() - margin )
		newPos = clip->position();
	if ( endPos - newPos < profile.getVideoFrameDuration() )
		newPos = endPos - profile.getVideoFrameDuration();
	
	return true;
}



void Scene::effectResizeStart( Clip *clip, double newPos, double newLength, bool isVideo, int index )
{
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );
	
	f->setPositionOffset( newPos - f->getPosition() );
	f->setLength( newLength );
	if ( f->getSnap() == Filter::SNAPALL && f->getLength() != clip->length() )
		f->setSnap( Filter::SNAPNONE );
	else if ( f->getSnap() == Filter::SNAPNONE && f->getLength() == clip->length() )
		f->setSnap( Filter::SNAPALL );
}



bool Scene::effectCanResize( Clip *clip, double &newLength, bool isVideo, int index )
{
	double margin = profile.getVideoFrameDuration() / 4.0;
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );
	
	if ( f->getSnap() == Filter::SNAPEND )
		return false;
	
	newLength = nearestPTS( newLength, profile.getVideoFrameDuration() );
	if ( f->getPositionOffset() + newLength > clip->length() + margin )
		newLength = clip->length() - f->getPositionOffset();
	if ( newLength < profile.getVideoFrameDuration() )
		newLength = profile.getVideoFrameDuration();
	
	return true;
}



void Scene::effectResize( Clip *clip, double newLength, bool isVideo, int index )
{
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );
	
	f->setLength( newLength );
	if ( f->getSnap() == Filter::SNAPALL && f->getLength() != clip->length() )
		f->setSnap( Filter::SNAPNONE );
	else if ( f->getSnap() == Filter::SNAPNONE && f->getLength() == clip->length() )
		f->setSnap( Filter::SNAPALL );
}
	


bool Scene::canResizeStart( Clip *clip, double &newPos, double endPos, int track )
{
	QMutexLocker ml( &mutex );
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	double newLength = endPos - newPos;
	double start = clip->getSource()->getProfile().getStreamStartTime();
	double end = clip->getSource()->getProfile().getStreamStartTime() + clip->getSource()->getProfile().getStreamDuration();
	if ( newLength < profile.getVideoFrameDuration() )
		return false;
	if ( clip->getSpeed() < 0 ) {
		if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + (newLength * qAbs(clip->getSpeed())) > end )
			return false;
	}
	else {
		if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + ((newPos - clip->position()) * qAbs(clip->getSpeed())) < start )
			return false;
	}
	
	return checkPlacement( clip, track, newPos, newLength );
}


	
void Scene::resizeStart( Clip *clip, double newPos, double newLength, int track )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->position() == newPos && clip->length() == newLength )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	int insert, self = 0;
	Track *t = tracks[track];
	insert = t->clipCount();
	for ( int i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( c == clip ) {
			++self;
			continue;
		}
		if ( newPos < c->position() ) {
			insert = i;
			break;
		}
	}
				
	insert -= self;
	double old = clip->position();
	removeTransitions( clip, track, track, insert, newPos, newLength, margin );
	t->removeClip( clip );
	t->insertClipAt( clip, insert );
	if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->getSpeed() >= 0 )
		clip->setStart( clip->start() + ((clip->length() - newLength) * qAbs(clip->getSpeed())) );
	clip->setLength( newLength );
	clip->setPosition( newPos );
	updateTransitions( clip, track, margin );
	clip->setInput( NULL );
	update = updateCurrentPosition( qMin( old, clip->position() ), qMax( old, clip->position() )  );
}



bool Scene::canResize( Clip *clip, double &newLength, int track )
{
	newLength = nearestPTS( newLength, profile.getVideoFrameDuration() );
	double start = clip->getSource()->getProfile().getStreamStartTime();
	double end = clip->getSource()->getProfile().getStreamStartTime() + clip->getSource()->getProfile().getStreamDuration();
	if ( newLength < profile.getVideoFrameDuration() )
		return false;
	if ( clip->getSpeed() < 0 ) {
		if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + ((clip->length() - newLength) * qAbs(clip->getSpeed())) < start )
			return false;
	}
	else {
		if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + (newLength * qAbs(clip->getSpeed())) > end )
			return false;
	}
	
	return checkPlacement( clip, track, clip->position(), newLength );
}


	
void Scene::resize( Clip *clip, double newLength, int track )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->length() == newLength )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	removeTransitions( clip, track, track, tracks[track]->indexOf( clip ), clip->position(), newLength, margin );
	double old = clip->position() + clip->length();
	if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->getSpeed() < 0 )
		clip->setStart( clip->start() + ((clip->length() - newLength) * qAbs(clip->getSpeed())) );
	clip->setLength( newLength );
	updateTransitions( clip, track, margin );
	clip->setInput( NULL );
	update = updateCurrentPosition( qMin( old, clip->position() + clip->length() ), qMax( old, clip->position() + clip->length() )  );
}



bool Scene::canMove( Clip *clip, double clipLength, double &newPos, int newTrack )
{
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	return checkPlacement( clip, newTrack, newPos, clipLength );
}



void Scene::move( Clip *clip, int clipTrack, double newPos, int newTrack )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->position() == newPos && clipTrack == newTrack )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	int insert, self = 0;
	Track *t = tracks[newTrack];
	insert = t->clipCount();
	for ( int i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( c == clip ) {
			++self;
			continue;
		}
		if ( newPos < c->position() ) {
			insert = i;
			break;
		}
	}
			
	insert -= self;
	removeTransitions( clip, clipTrack, newTrack, insert, newPos, clip->length(), margin );
	tracks[clipTrack]->removeClip( clip );
	t->insertClipAt( clip, insert );
	clip->setPosition( newPos );
	updateTransitions( clip, newTrack, margin );
	clip->setInput( NULL );
	update = true;
}



bool Scene::canMoveMulti( Clip *clip, double clipLength, double &newPos, int track )
{
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	double margin = profile.getVideoFrameDuration() / 4.0;
	Track *t = tracks[track];
	int count = t->clipCount();
	
	if ( newPos >= clip->position() )
		return true;

	if ( t->clipAt( 0 ) == clip )
		return true;
	
	Clip *c;
	int k = 0;
	while ( k < count ) {
		c = t->clipAt( k );
		// find clip
		if ( c == clip ) {
			break;
		}
		++k;
	}
	if ( k == count )
		return false;
	
	int clipIndex = k;
	c = t->clipAt( --k );
	if ( newPos - margin < c->position() )
			return false;
	// we are clipB, we can't end before clipA
	if ( newPos + clipLength < c->position() + c->length() - margin )
		return false;
	// we can't overlap with clip before clipA
	if ( --k >= 0 ) {
		Clip *prevA = t->clipAt( k );
		if ( newPos < prevA->position() + prevA->length() - margin )
			return false; 
	}
	// and clipC can't overlap with clipA
	if ( clipIndex < count - 1 ) {
		Clip *clipC = t->clipAt( clipIndex + 1 );
		// clipC has moved with us, calculate its new position
		double cpos = clipC->position() + ( newPos - clip->position() );
		if ( cpos < c->position() + c->length() - margin ) {
			newPos = c->position() + c->length() + clip->position() - clipC->position();
			return true;
		}
	}
	
	return true;
}



void Scene::moveMulti( Clip *clip, int clipTrack, double newPos )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->position() == newPos )
		return;
	
	double delta = newPos - clip->position();
	double margin = profile.getVideoFrameDuration() / 4.0;
	Track *t = tracks[clipTrack];
	int count = t->clipCount();
	int clipIndex = 0;
	int k = 0;
	while ( k < count ) {
		Clip *c = t->clipAt( k );
		// find clip
		if ( c == clip ) {
			clipIndex = k;
			break;
		}
		++k;
	}

	removeTransitions( clip, clipTrack, clipTrack, clipIndex, newPos, clip->length(), margin, true );
	clip->setPosition( clip->position() + delta );
	while ( ++k < count ) {
		Clip *c = t->clipAt( k );
		c->setPosition( c->position() + delta );
	}
	updateTransitions( clip, clipTrack, margin );
	clip->setInput( NULL );
	
	update = true;
}

	
	
bool Scene::checkPlacement( Clip *clip, int track, double clipPos, double clipLength )
{
	double margin = profile.getVideoFrameDuration() / 4.0;
	Track *t = tracks[track];
	int count = t->clipCount();
	
	// no clip in track yet
	if ( !count )
		return true;
	if ( count == 1 && t->clipAt( 0 ) == clip )
		return true;
	
	Clip *c;
	int k = 0;
	while ( k < count ) {
		c = t->clipAt( k );
		// obviously, don't check clip against itself
		if ( clip && c == clip ) {
			++k;
			continue;
		}
		// Track is ordered by Clip::position()
		// so 2 clips can't start at same pts on the same track.
		if ( qAbs( c->position() - clipPos ) < margin )
			return false;
		if ( !clipLessThan( margin, c->position(), c->length(), clipPos ) )
			break;
		++k;
	}
	// we are the last clip in track and don't overlap with anything
	if ( k == count )
		return true;
	// we are clipA
	if ( clipPos < c->position() ) {
		// we can't end after clipB
		if ( c->position() + c->length() < clipPos + clipLength - margin )
			return false;
		// and we can't overlap with clipC
		int j = k + 1;
		while ( j < count ) {
			Clip *next = t->clipAt( j++ );
			if ( clip && next == clip )
				continue;
			if ( next->position() < clipPos + clipLength - margin )
				return false;
			break;
		}
	}
	else {
		// we are clipB, we can't end before clipA
		if ( clipPos + clipLength < c->position() + c->length() - margin )
			return false;
		int j = k + 1;
		while ( j < count ) {
			Clip *next = t->clipAt( j++ );
			if ( clip && next == clip )
				continue;
			// clipC can't overlap with clipA
			if ( next->position() < c->position() + c->length() - margin )
				return false;
			// we can't end after clipC
			if ( next->position() + next->length() < clipPos + clipLength - margin )
				return false;
			// and we can't overlap with clipD
			if ( j < count ) {
				Clip *d = t->clipAt( j );
				if ( d->position() < clipPos + clipLength - margin )
					return false;
			}
			break;
		}
	}

	return true;
}



void Scene::updateTransitions( Clip *clip, int track, double margin )
{
	Track *t = tracks[track];
	int index = t->indexOf( clip );
	
	if ( index > 0 ) {
		Clip *c = t->clipAt( index - 1 );
		if ( clip->position() < c->position() + c->length() - margin )
			clip->setTransition( c->position() + c->length() - clip->position() );
	}
	if ( index < t->clipCount() - 1 ) {
		Clip *c = t->clipAt( index + 1 );
		if ( c->position() < clip->position() + clip->length() - margin )
			c->setTransition( clip->position() + clip->length() - c->position() );
	}
}



void Scene::removeTransitions( Clip *clip, int oldTrack, int newTrack, int newIndex, double clipPos, double clipLength, double margin, bool multi )
{
	Track *ot = tracks[oldTrack];

	int index = ot->indexOf( clip );
	if ( newTrack != oldTrack || index != newIndex ) {
		clip->removeTransition();
		if ( index < ot->clipCount() - 1 ) {
			Clip *c = ot->clipAt( index + 1 );
			c->removeTransition();
		}
	}
	else {
		if ( !multi && index < ot->clipCount() - 1 ) {
			Clip *c = ot->clipAt( index + 1 );
			if ( clipPos + clipLength - margin < c->position() )
				c->removeTransition();
		}
		if ( index > 0 ) {
			Clip *c = ot->clipAt( index - 1 );
			if ( c->position() + c->length() - margin < clipPos )
				clip->removeTransition();
		}
	}
}


	
bool Scene::clipLessThan( double margin, double cpos, double clen, double pos )
{
	if ( (cpos + clen - pos) < margin )
		return true;
	return false;
}


	
bool Scene::collidesWith( double margin, double cpos, double pos, double len )
{
	if ( pos + len - cpos > margin )
		return true;
	return false;
}



bool Scene::updateCurrentPosition( double begin, double end )
{
	if ( currentPTS > begin - FORWARDLOOKUP && currentPTS < end + FORWARDLOOKUP )
		return true;
	return false;
}


	
void Scene::addClip( Clip *clip, int track )
{
	QMutexLocker ml( &mutex );
	double margin = profile.getVideoFrameDuration() / 4.0;
	Track *t = tracks[track];
	int i = 0, cc = t->clipCount();
	while ( i < cc ) {
		Clip *c = t->clipAt( i );
		if ( clip->position() < c->position() ) {
			t->insertClipAt( clip, i );
			updateTransitions( clip, track, margin );
			update = true;
			return;
		}
		++i;
	}
	t->insertClipAt( clip, i );
	updateTransitions( clip, track, margin );
}


	
bool Scene::removeClip( Clip *clip )
{
	QMutexLocker ml( &mutex );
	int i, j;
	for ( i = 0; i < tracks.count(); ++i ) {
		Track *t = tracks[ i ];
		for ( j = 0; j < t->clipCount(); ++j ) {
			if ( clip == t->clipAt( j ) ) {
				if ( j < t->clipCount() - 1 )
					t->clipAt( j + 1 )->removeTransition();
				Clip *c = t->removeClip( j );
				if ( c ) {
					update = true;
					delete c;
				}
				return true;
			}
		}
	}
	return false;
}



void Scene::drain()
{
	QMutexLocker ml( &mutex );
	for ( int i = 0; i < tracks.count(); ++i ) {
		Track *t = tracks[ i ];
		while ( t->clipCount() ) {
			delete t->removeClip( 0 );
		}
	}
	update = true;
}



bool Scene::removeTrack( int index )
{
	if ( index < 0 || index >= tracks.count() )
		return false;
	
	if ( tracks[index]->clipCount() )
		return false;
	delete tracks.takeAt( index );
	update = true;
	return true;
}



bool Scene::addTrack( int index )
{
	if ( index < 0 || index > tracks.count() )
		return false;
	
	tracks.insert( index, new Track() );
	update = true;
	return true;
}
