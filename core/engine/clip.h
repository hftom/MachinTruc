#ifndef CLIP_H
#define CLIP_H

#include <QList>

#include "engine/cut.h"

#include "vfx/glfilter.h"
#include "afx/audiofilter.h"
#include "input/input.h"



class Clip
{
public:
	Clip( Source *src, double posInTrackPTS, double strt, double len );
	~Clip();

	FList<GLFilter*> videoFilters;
	FList<AudioFilter*> audioFilters;

	QString sourcePath() { return source->getFileName(); }
	const Profile & getProfile() const { return source->getProfile(); }
	
	double position() { return posInTrack; }
	void setPosition( double p ) { posInTrack = p; }
	double start() { return clipStart; }
	void setStart( double st ) { clipStart = st; }
	double length();
	void setLength( double len ) { clipLength = len; }
	
	void setInput( InputBase *i );
	InputBase *getInput() { return in; }
	InputBase::InputType getType() { return source->getType(); }
	
	Source & getSource() const { return *source; }
	
	void setFrameDuration( double d ) { frameDuration = d; }

private:
	Source *source;
	double posInTrack;
	double clipStart;
	double clipLength;
	double frameDuration;
	InputBase *in;
};

#endif //CLIP_H
