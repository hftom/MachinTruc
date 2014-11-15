// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef INPUTIMAGE_H
#define INPUTIMAGE_H

#include <QImage>
#include <QSemaphore>

#include "input/input.h"



class InputImage : public InputBase
{
	Q_OBJECT
public:
	InputImage();
	~InputImage();
	bool open( QString fn );
	void openSeekPlay( QString fn, double p, bool backward = false );
	void close() {}
	void flush() {}
	void seekFast( float percent );
	void seekNext() {}
	double seekTo( double p );
	void play( bool ) {}
	Frame *getVideoFrame();
	Frame *getAudioFrame( int ) { return NULL; }

	bool probe( QString fn, Profile *prof );

protected:
	void run();

private:
	bool upload( Frame *f );

	double fps;
	double currentVideoPTS;

	MQueue<Frame*> freeVideoFrames;

	Buffer *buffer;
	int width, height;
	bool rgba;
	QSemaphore *semaphore;
};

#endif //INPUTIMAGE_H
