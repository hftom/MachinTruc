#ifndef SCENE_H
#define SCENE_H

#define FORWARDLOOKUP 2 * MICROSECOND // µs

#include <QMutex>

#include "engine/clip.h"
#include "engine/track.h"



class Scene
{
public:
	Scene( Profile p );
	~Scene();
	
	bool setProfile( Profile &p );
	Profile & getProfile() { return profile; }
	
	void drain();
	bool removeTrack( int index );
	bool addTrack( int index );
	
	Clip* createClip( Source *src, double posInTrackPTS, double strt, double len );
	Clip* duplicateClip(Clip *c);
	
	bool effectCanMove( Clip *clip, double &newPos, bool isVideo, int index );
	void effectMove( Clip *clip, double newPos, bool isVideo, int index );
	bool effectCanResizeStart( Clip *clip, double &newPos, double endPos, bool isVideo, int index );
	void effectResizeStart( Clip *clip, double newPos, double newLength, bool isVideo, int index );
	bool effectCanResize( Clip *clip, double &newLength, bool isVideo, int index );
	void effectResize( Clip *clip, double newLength, bool isVideo, int index );
	
	bool canResizeStart( Clip *clip, double &newPos, double endPos, int track );
	void resizeStart( Clip *clip, double newPos, double newLength, int track );
	bool canResize( Clip *clip, double &newLength, int track );
	void resize( Clip *clip, double newLength, int track );
	bool canMove( Clip *clip, double clipLength, double &newPos, int newTrack );
	void move( Clip *clip, int clipTrack, double newPos, int newTrack );
	bool canMoveMulti( Clip *clip, double clipLength, double &newPos, int track );
	void moveMulti( Clip *clip, int clipTrack, double newPos );
	void addClip( Clip *clip, int track );
	bool removeClip( Clip *clip );
	Transition* getTailTransition(Clip *clip, int track);
	Clip* getTailClip(Clip *clip, int track);
	bool canSplitClip(Clip *clip, int track, double pts);
	Clip* sceneSplitClip( Clip *clip, int track, double pts );
	
	double nextEdge(double pts);
	double previousEdge(double pts);

	bool update;
	QList<Track*> tracks;
	double currentPTS, currentPTSAudio;
	QMutex mutex;
	
private:
	bool checkPlacement( Clip *clip, int track, double clipPos, double clipLength );
	void removeTransitions( Clip *clip, int oldTrack, int newTrack, int newIndex, double clipPos, double clipLength, double margin, bool multi = false );
	void updateTransitions( Clip *clip, int newTrack, double margin );
	bool clipLessThan( double margin, double cpos, double clen, double pos );	
	bool collidesWith( double margin, double cpos, double pos, double len );
	bool updateCurrentPosition( double begin, double end );
	
	Profile profile;
};

#endif //SCENE_H