#include "input/input_ff.h"

#include "testinputff.h"



// test.mp4 : 720x576@25p, 14 frames.
#define VIDEOTEST "test.mp4"
#define VIDEOTESTNFRAMES 14



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



void TestInputFF::allFramesDecoded()
{
	InputFF *in = new InputFF();
	Profile prof;
	in->probe( VIDEOTEST, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	int i = 0;
	Frame *f;
	while ( (f = in->getVideoFrame()) ) {
		++i;
		f->release();
	}
	in->play( false );
	delete in;
    QVERIFY( i == VIDEOTESTNFRAMES );
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
	while ( (f = in->getVideoFrame()) ) {
		pts = f->pts();
		duration = f->profile.getVideoFrameDuration();
		f->release();
	}
	in->openSeekPlay( VIDEOTEST, pts - duration );
	f = in->getVideoFrame();
	pts = f->pts();
	f->release();
	in->play( false );
	delete in;
    QVERIFY( pts == prof.getStreamStartTime() + prof.getStreamDuration() - (prof.getVideoFrameDuration() * 2.0) );
}



void TestInputFF::seekStart()
{
	InputFF *in = new InputFF();
	Profile prof;
	in->probe( VIDEOTEST, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	Frame *f;
	while ( (f = in->getVideoFrame()) ) {
		f->release();
	}
	in->openSeekPlay( VIDEOTEST, prof.getStreamStartTime() );
	f = in->getVideoFrame();
	double pts = f->pts();
	f->release();
	in->play( false );
	delete in;
    QVERIFY( pts == prof.getStreamStartTime() );
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
	Frame *f;
	int i = 0;
	while ( (f = in->getVideoFrame()) ) {
		++i;
		f->release();
	}
	in->play( false );
	delete in;
    QVERIFY( i == 2 * VIDEOTESTNFRAMES );
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
	Frame *f;
	int i = 0;
	while ( (f = in->getVideoFrame()) ) {
		++i;
		f->release();
	}
	in->play( false );
	delete in;
    QVERIFY( i == 3 * VIDEOTESTNFRAMES );
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
	Frame *f;
	int i = 0;
	while ( (f = in->getVideoFrame()) ) {
		++i;
		f->release();
	}
	in->play( false );
	delete in;
    QVERIFY( i == VIDEOTESTNFRAMES / 2 );
}
