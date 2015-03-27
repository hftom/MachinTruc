#ifndef INPUT_BLANK_H
#define INPUT_BLANK_H

#include "input/input.h"



class InputBlank : public InputBase
{
	Q_OBJECT
public:
	InputBlank();
	bool open( QString fn );
	void openSeekPlay( QString fn, double p, bool backward = false );
	void seekFast( float percent );
	double seekTo( double p );
	void play( bool ) {}
	Frame *getVideoFrame();
	Frame *getAudioFrame( int ) { return NULL; }

	bool probe( QString fn, Profile *prof );

private:
	void upload( Frame *f );

	double fps;
	double currentVideoPTS;
	int width, height;
};
#endif // INPUT_BLANK_H