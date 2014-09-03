// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <unistd.h>

#include "input/input_gl.h"



InputGL::InputGL() : InputBase(),
	fps( 30 ),
	currentVideoPTS( 0 )
{
	inputType = OPENGL;

	int i;
	for ( i = 0; i < NUMINPUTFRAMES; ++i )
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames ) );
}



InputGL::~InputGL()
{
	Frame *f;
	while ( (f = freeVideoFrames.dequeue()) )
		delete f;
}



bool InputGL::probe( QString fn, Profile *prof )
{
	Q_UNUSED( fn );
	Q_UNUSED( prof );
	return false;
}



bool InputGL::open( QString fn )
{
	sourceName = fn;

	return true;
}



void InputGL::openSeekPlay( QString fn, double p )
{
	if ( fn != sourceName )
		open( fn );
	seekTo( p );
}



void InputGL::seekFast( float percent )
{
	currentVideoPTS = 60000000 * percent / 100.0;
}



double InputGL::seekTo( double p )
{
	currentVideoPTS = p;
	return p;
}



bool InputGL::process( Frame *f )
{
	Q_UNUSED( f );
	return false;
}



Frame* InputGL::getVideoFrame()
{
	Frame *f = NULL;

	while ( freeVideoFrames.queueEmpty() )
		usleep( 500 );

	f = freeVideoFrames.dequeue();
	if ( !process( f ) ) {
		f->release();
		return NULL;
	}
	return f;
}
