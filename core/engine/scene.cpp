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
		FilterCollection *fc = FilterCollection::getGlobalInstance();
		for ( int i = 0; i < clip->videoFilters.count(); ++i ) {
			GLFilter *f = clip->videoFilters.at( i );
			for ( int j = 0; j < fc->videoFilters.count(); ++j ) {
				if ( fc->videoFilters[ j ].identifier == f->getIdentifier() ) {
					GLFilter *nf = (GLFilter*)fc->videoFilters[ j ].create();
					f->splitParameters( nf, newLength );
					nf->setPosition( nc->position() );
					nf->setLength( nc->length() );
					nc->videoFilters.append( nf );
				}
			}
		}
		resize( clip, newLength, track );
		addClip( nc, track );
		return nc;
	}

	return NULL;
}
	


bool Scene::canResizeStart( Clip *clip, double &newPos, double endPos, int track ) {
	QMutexLocker ml( &mutex );
	double margin = profile.getVideoFrameDuration() / 4.0;
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );
	double newLength = endPos - newPos;
	double start = clip->getSource()->getProfile().getStreamStartTime();
	if ( newLength < profile.getVideoFrameDuration() )
		return false;
	if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + newPos - clip->position() < start )
		return false;

	int i;
	Track *t = tracks[track];
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			continue;
		}
		if ( clipLessThan( margin, newPos, newLength, c->position() ) ) {
			break;
		}
		if ( collidesWith( margin, newPos, c->position(), c->length() ) ) {
			return false;
		}
	}

	return true;
}


	
void Scene::resizeStart( Clip *clip, double newPos, double newLength, int track )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->position() == newPos && clip->length() == newLength )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	int i, insert, self = 0;
	Track *t = tracks[track];
	insert = t->clipCount();
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			++self;
			continue;
		}
		if ( clipLessThan( margin, newPos, newLength, c->position() ) ) {
			insert = i;
			break;
		}
	}
				
	insert -= self;
	double old = clip->position();
	t->removeClip( clip );
	t->insertClipAt( clip, insert );
	if ( clip->getSource()->getType() == InputBase::FFMPEG )
		clip->setStart( clip->start() + clip->length() - newLength );
	clip->setLength( newLength );
	clip->setPosition( newPos );
	clip->setInput( NULL );
	update = updateCurrentPosition( qMin( old, clip->position() ), qMax( old, clip->position() )  );
}



bool Scene::canResize( Clip *clip, double &newLength, int track )
{
	QMutexLocker ml( &mutex );
	double margin = profile.getVideoFrameDuration() / 4.0;
	newLength = nearestPTS( newLength, profile.getVideoFrameDuration() );
	double end = clip->getSource()->getProfile().getStreamStartTime() + clip->getSource()->getProfile().getStreamDuration();
	if ( newLength < profile.getVideoFrameDuration() )
		return false;
	if ( clip->getSource()->getType() == InputBase::FFMPEG && clip->start() + newLength > end )
		return false;

	int i;
	Track *t = tracks[track];
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			continue;
		}
		if ( clipLessThan( margin, clip->position(), newLength, c->position() ) ) {
			break;
		}
		if ( collidesWith( margin, clip->position(), c->position(), c->length() ) ) {
			return false;
		}
	}

	return true;
}


	
void Scene::resize( Clip *clip, double newLength, int track )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->length() == newLength )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	int i, insert, self = 0;
	Track *t = tracks[track];
	insert = t->clipCount();
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			++self;
			continue;
		}
		if ( clipLessThan( margin, clip->position(), newLength, c->position() ) ) {
			insert = i;
			break;
		}
	}
				
	insert -= self;
	double old = clip->position() + clip->length();
	t->removeClip( clip );
	t->insertClipAt( clip, insert );
	clip->setLength( newLength );
	clip->setInput( NULL );
	update = updateCurrentPosition( qMin( old, clip->position() + clip->length() ), qMax( old, clip->position() + clip->length() )  );
}



bool Scene::canMove( Clip *clip, double clipLength, double &newPos, int newTrack )
{
	QMutexLocker ml( &mutex );
	double margin = profile.getVideoFrameDuration() / 4.0;
	newPos = nearestPTS( newPos, profile.getVideoFrameDuration() );

	int i;
	Track *t = tracks[newTrack];
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			continue;
		}
		if ( clipLessThan( margin, newPos, clipLength, c->position() ) ) {
			break;
		}
		if ( collidesWith( margin, newPos, c->position(), c->length() ) ) {
			return false;
		}
	}

	return true;
}


	
void Scene::move( Clip *clip, int clipTrack, double newPos, int newTrack )
{
	QMutexLocker ml( &mutex );
	
	if ( clip->position() == newPos && clipTrack == newTrack )
		return;
	
	double margin = profile.getVideoFrameDuration() / 4.0;
	int i, insert, self = 0;
	Track *t = tracks[newTrack];
	insert = t->clipCount();
	for ( i = 0; i < t->clipCount(); ++i ) {
		Clip *c = t->clipAt( i );
		if ( clip && c == clip ) {
			++self;
			continue;
		}
		if ( clipLessThan( margin, newPos, clip->length(), c->position() ) ) {
			insert = i;
			break;
		}
	}
			
	insert -= self;
	tracks[clipTrack]->removeClip( clip );
	t->insertClipAt( clip, insert );
	clip->setPosition( newPos );
	clip->setInput( NULL );
	update = true;
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
	Track *t = tracks[track];
	int i = 0, cc = t->clipCount();
	while ( i < cc ) {
		Clip *c = t->clipAt( i );
		if ( clip->position() < c->position() ) {
			t->insertClipAt( clip, i );
			update = updateCurrentPosition( clip->position(), clip->position() + clip->length() );
			return;
		}
		++i;
	}
	t->insertClipAt( clip, i );
}


	
bool Scene::removeClip( Clip *clip )
{
	QMutexLocker ml( &mutex );
	int i, j;
	for ( i = 0; i < tracks.count(); ++i ) {
		Track *t = tracks[ i ];
		for ( j = 0; j < t->clipCount(); ++j ) {
			if ( clip == t->clipAt( j ) ) {
				Clip *c = t->removeClip( j );
				if ( c ) {
					update = updateCurrentPosition( c->position(), c->position() + c->length() );
					delete c;
				}
				return true;
			}
		}
	}
	return false;
}
