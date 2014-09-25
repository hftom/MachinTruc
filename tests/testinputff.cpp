#include "input/input_ff.h"

#include "testinputff.h"



// test.mp4 : 720x576@25p, 14 frames.
#define VIDEOTEST "test.mp4"
#define VIDEOTESTNFRAMES 14

// Y[0] + 2 * U[0]
static int frameColors[] = { 491, 261, 521, 253, 242, 510, 502, 267, 397, 263, 257, 391, 387, 382 };




void TestInputFF::probeReturnsTrue()
{
	InputFF *in = new InputFF();
	Profile prof;
	bool b = in->probe( VIDEOTEST, &prof );
	delete in;
    QVERIFY( b == true );
}



void TestInputFF::probeReturnsFalse()
{
	InputFF *in = new InputFF();
	Profile prof;
	bool b = in->probe( "testinputff.h", &prof );
	delete in;
    QVERIFY( b == false );
}



void TestInputFF::streamDurationCorrectlyDetected()
{
	InputFF *in = new InputFF();
	Profile prof;
	bool b = in->probe( VIDEOTEST, &prof );
	delete in;
    QVERIFY( b == true && prof.getStreamDuration() == prof.getVideoFrameDuration() * VIDEOTESTNFRAMES );
}


bool checkEqual( int *first, int *second, int size )
{
	for ( int i = 0; i < size; ++i ) {
		if ( first[i] != second[i] ) {
			for ( i = 0; i < size; ++i )
				qDebug() << first[i] << second[i];
			return false;
		}
	}
	return true;
}



void TestInputFF::allFramesDecoded()
{
	InputFF *in = new InputFF();
	Profile prof;
	in->probe( VIDEOTEST, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	int i = 0;
	Frame *f;
	int colors[VIDEOTESTNFRAMES];
	memset( colors, 0, VIDEOTESTNFRAMES * sizeof(int) );
	while ( i < VIDEOTESTNFRAMES && (f = in->getVideoFrame()) ) {
		uint8_t *data = f->data();
		colors[i] = data[0] + 2 * data[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
		f->release();
		++i;
	}
	in->play( false );
	delete in;
    QVERIFY( i == VIDEOTESTNFRAMES 
			&& checkEqual( frameColors, colors, VIDEOTESTNFRAMES ) );
}



void TestInputFF::seekBackOneFrameFromEnd()
{
	InputFF *in = new InputFF();
	Profile prof;
	in->probe( VIDEOTEST, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	double pts = 0, duration = 0;
	Frame *f;
	int i = 0;
	while ( i < VIDEOTESTNFRAMES && (f = in->getVideoFrame()) ) {
		pts = f->pts();
		duration = f->profile.getVideoFrameDuration();
		f->release();
		++i;
	}
	in->openSeekPlay( VIDEOTEST, pts - duration );
	f = in->getVideoFrame();
	pts = f->pts();
	int color = f->data()[0] + 2 * f->data()[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
	f->release();
	in->play( false );
	delete in;
    QVERIFY( pts == prof.getStreamStartTime() + prof.getStreamDuration() - (prof.getVideoFrameDuration() * 2.0)
			&& color == frameColors[VIDEOTESTNFRAMES - 2] );
}



void TestInputFF::seekStart()
{
	InputFF *in = new InputFF();
	Profile prof;
	in->probe( VIDEOTEST, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	Frame *f;
	int i = 0;
	while ( i < VIDEOTESTNFRAMES && (f = in->getVideoFrame()) ) {
		f->release();
		++i;
	}
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	f = in->getVideoFrame();
	double pts = f->pts();
	int color = f->data()[0] + 2 * f->data()[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
	f->release();
	in->play( false );
	delete in;
    QVERIFY( pts == prof.getStreamStartTime()
			&& color == frameColors[0] );
}



void TestInputFF::resampleDoubleFrameRate()
{
	InputFF *in = new InputFF();
	Profile prof, outProf;
	in->probe( VIDEOTEST, &prof );
	outProf = prof;
	outProf.setVideoFrameRate( 2.0 * prof.getVideoFrameRate() );
	outProf.setVideoFrameDuration( MICROSECOND / outProf.getVideoFrameRate() );
	in->setProfile( prof, outProf );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	int colors[VIDEOTESTNFRAMES * 2];
	memset( colors, 0, VIDEOTESTNFRAMES * 2 * sizeof(int) );
	Frame *f;
	int i = 0;
	while ( i < VIDEOTESTNFRAMES * 2 && (f = in->getVideoFrame()) ) {
		colors[i] = f->data()[0] + 2 * f->data()[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
		f->release();
		++i;
	}
	in->play( false );
	delete in;
	int framecol[VIDEOTESTNFRAMES * 2];
	int k = 0;
	while ( k < VIDEOTESTNFRAMES * 2 ) {
		framecol[k] = frameColors[k/2];
		framecol[k+1] = frameColors[k/2];
		k += 2;
	}
    QVERIFY( i == 2 * VIDEOTESTNFRAMES
			&& checkEqual( colors, framecol, VIDEOTESTNFRAMES * 2 ) );
}



void TestInputFF::resampleTripleFrameRate()
{
	InputFF *in = new InputFF();
	Profile prof, outProf;
	in->probe( VIDEOTEST, &prof );
	outProf = prof;
	outProf.setVideoFrameRate( 3.0 * prof.getVideoFrameRate() );
	outProf.setVideoFrameDuration( MICROSECOND / outProf.getVideoFrameRate() );
	in->setProfile( prof, outProf );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	int colors[VIDEOTESTNFRAMES * 3];
	memset( colors, 0, VIDEOTESTNFRAMES * 3 * sizeof(int) );
	Frame *f;
	int i = 0;
	while ( i < VIDEOTESTNFRAMES * 3 && (f = in->getVideoFrame()) ) {
		colors[i] = f->data()[0] + 2 * f->data()[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
		f->release();
		++i;
	}
	in->play( false );
	delete in;
	int framecol[VIDEOTESTNFRAMES * 3];
	int k = 0;
	while ( k < VIDEOTESTNFRAMES * 3 ) {
		framecol[k] = framecol[k+1] = framecol[k+2] = frameColors[k/3];
		k += 3;
	}
    QVERIFY( i == 3 * VIDEOTESTNFRAMES
			&& checkEqual( colors, framecol, VIDEOTESTNFRAMES * 3 ) );
}



void TestInputFF::resampleHalfFrameRate()
{
	InputFF *in = new InputFF();
	Profile prof, outProf;
	in->probe( VIDEOTEST, &prof );
	outProf = prof;
	outProf.setVideoFrameRate( prof.getVideoFrameRate() / 2.0 );
	outProf.setVideoFrameDuration( MICROSECOND / outProf.getVideoFrameRate() );
	in->setProfile( prof, outProf );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	int colors[VIDEOTESTNFRAMES / 2];
	memset( colors, 0, VIDEOTESTNFRAMES / 2 * sizeof(int) );
	Frame *f;
	int i = 0;
	while ( i < VIDEOTESTNFRAMES / 2 && (f = in->getVideoFrame()) ) {
		colors[i] = f->data()[0] + 2 * f->data()[prof.getVideoWidth() * prof.getVideoHeight() * 5 / 4];
		f->release();
		++i;
	}
	in->play( false );
	delete in;
	int framecol[VIDEOTESTNFRAMES / 2];
	int k = 0;
	while ( k < VIDEOTESTNFRAMES / 2 ) {
		framecol[k] = frameColors[k * 2];
		k += 1;
	}
    QVERIFY( i == VIDEOTESTNFRAMES / 2
			&& checkEqual( colors, framecol, VIDEOTESTNFRAMES / 2 ) );
}
