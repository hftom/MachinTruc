#ifndef INPUT_H
#define INPUT_H

#include <QThread>

#include "engine/frame.h"



class InputBase : public QThread
{
	Q_OBJECT
public:
	enum InputType{ UNDEF, FFMPEG, OPENGL, IMAGE };

	InputBase() {
		haveAudio = haveVideo = false;
		usedByClip = false;
		inputType = UNDEF;
	}
	virtual ~InputBase() {}
	virtual bool probe( QString fn, Profile *prof ) = 0;
	virtual bool open( QString fn ) = 0;
	virtual void openSeekPlay( QString fn, double pts ) = 0;
	virtual void close() = 0;
	virtual void flush() = 0;
	virtual void seekFast( float percent ) = 0;
	virtual void seekNext() = 0;
	virtual double seekTo( double ) = 0;
	virtual void play( bool ) = 0;
	virtual Frame *getVideoFrame() = 0;
	virtual Frame *getAudioFrame( int nSamples ) = 0;

	void setProfile( const Profile &in, const Profile &out ) { inProfile = in; outProfile = out; }
	bool hasAudio() { return haveAudio; }
	bool hasVideo() { return haveVideo; }
	QString getSource() { return sourceName; }
	InputType getType() { return inputType; }
	bool isUsed() { return usedByClip; }
	void setUsed( bool b ) { usedByClip = b; }

protected:
	bool haveAudio, haveVideo;
	bool usedByClip;
	QString sourceName;
	InputType inputType;

	Profile inProfile, outProfile;
};
#endif //INPUT_H
