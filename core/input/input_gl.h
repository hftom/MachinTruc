// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef INPUTGL_H
#define INPUTGL_H

#include "input/input.h"



class InputGL : public InputBase
{
	Q_OBJECT
public:
	InputGL();
	~InputGL();
	bool open( QString fn );
	void openSeekPlay( QString fn, double p );
	void close() {}
	void flush() {}
	void seekFast( float percent );
	void seekNext() {}
	double seekTo( double p );
	void play( bool ) {}
	Frame *getVideoFrame();
	Frame *getAudioFrame( int ) { return NULL; }

	bool probe( QString fn, Profile *prof );

private:
	bool process( Frame *f );
	
	double fps;
	double currentVideoPTS;

	MQueue<Frame*> freeVideoFrames;
};

#endif //INPUTGL_H
