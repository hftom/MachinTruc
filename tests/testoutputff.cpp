#include <unistd.h>

#include "input/input_ff.h"
#include "output/output_ff.h"

#include "testoutputff.h"

// test.mp4 : 720x576@25p, 14 frames.
#define VIDEOTEST "test.mp4"
#define VIDEOTESTNFRAMES 14



void TestOutputFF::encode()
{	
	InputFF *in = new InputFF();
	Profile prof;
	QString file = VIDEOTEST;//"/home/cris/Videos/amandine-4M.mkv";
	in->probe( file, &prof );
	in->setProfile( prof, prof );
	in->openSeekPlay( file, prof.getStreamStartTime() );
	
	MQueue<Frame*> audioFrames;
	audioFrames.enqueue( new Frame( &audioFrames ) );
	
	MQueue<Frame*> encodeAudioFrames;
	MQueue<Frame*> encodeVideoFrames;
	
	OutputFF *out = new OutputFF( &encodeVideoFrames, &encodeAudioFrames );
	double end = prof.getStreamStartTime() + prof.getStreamDuration();
	end -= prof.getVideoFrameDuration() * 3.0 / 2.0;
	bool ok = out->init( "encode.mkv", prof, 1, OutputFF::VCODEC_HEVC, "", end );
	
	int bps = prof.getAudioChannels() * prof.bytesPerChannel( &prof );
	int nSamples = prof.getAudioSampleRate() / prof.getVideoFrameRate();
	
	int i = 0;
	Frame *f;
	double pts = 0;
	if ( ok ) {
		out->startEncode( false );
		while ( pts <= end ) {
			if ( audioFrames.queueEmpty() ) {
				usleep( 1000 );
				continue;
			}
			f = in->getVideoFrame();
			pts = f->pts();
			encodeVideoFrames.enqueue( f );
			//make silence
			f = audioFrames.dequeue();
			f->setAudioFrame( prof.getAudioChannels(), prof.getAudioSampleRate(),
							  prof.bytesPerChannel( &prof ), nSamples, pts );
			memset( f->data(), 0, nSamples * bps );
			encodeAudioFrames.enqueue( f );
			++i;
		}
		
		while ( out->isRunning() )
			usleep( 1000 );
	}
	
	in->play( false );
	delete in;
	delete out;
    QVERIFY( ok	&& i == VIDEOTESTNFRAMES );
}
