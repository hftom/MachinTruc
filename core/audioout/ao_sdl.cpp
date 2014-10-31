#include <unistd.h>

#include <QMutexLocker>

#include <SDL/SDL.h>

#include "audioout/ao_sdl.h"



AudioOutSDL::AudioOutSDL()
	: bufferSize( 1024 ),
	bufferLen( 0 ),
	bufferOffset( 0 ),
	readData( NULL ),
	readUserData( NULL )
{
	buffer = (uint8_t*)malloc( bufferSize );

	SDL_Init( SDL_INIT_AUDIO );

	SDL_AudioSpec ss, obt;
	ss.freq = sampleRate = DEFAULTSAMPLERATE;
	ss.format = AUDIO_S16SYS;
	ss.channels = DEFAULTCHANNELS;
	bytesPerSample = ss.channels * 2;
	ss.silence = 0;
	ss.samples = bufferSize;
	ss.callback = streamRequestCallback;
	ss.userdata = this;

	SDL_OpenAudio( &ss, &obt );
	
	printf("SDL audio obtained : freq=%d format=%d channels=%d\n", obt.freq, obt.format, obt.channels);
}



AudioOutSDL::~AudioOutSDL()
{
	SDL_Quit();
}



void AudioOutSDL::go()
{
	SDL_PauseAudio( 0 );
}



void AudioOutSDL::stop()
{
	SDL_PauseAudio( 1 );
	QMutexLocker ml( &mutex );
	bufferLen = bufferOffset = 0;
}



void AudioOutSDL::setReadCallback( void *func, void *userdata )
{
	printf("setReadCallback\n");
	readData = (READDATACALLBACK)func;
	readUserData = userdata;
}



void AudioOutSDL::streamRequestCallback( void *userdata, uint8_t *stream, int len )
{
	Frame *data = NULL;
	AudioOutSDL *ao = (AudioOutSDL*)userdata;

	if ( !ao->readData ) {
		return;
	}
	struct timeval tv;
	gettimeofday( &tv, NULL );
	double latency = MICROSECOND * ( (double)len / (double)ao->bytesPerSample ) / (double)ao->sampleRate;

	uint8_t *dst = stream;
	int size = len;
	
	QMutexLocker ml( &ao->mutex );

	// first, copy the remaining samples from previous Frame
	if ( ao->bufferLen ) {
		if ( ao->bufferLen >= size ) {
			memcpy( dst, ao->buffer + ao->bufferOffset, size );
			ao->bufferOffset += size;
			ao->bufferLen -= size;
			if ( !ao->bufferLen )
				ao->bufferOffset = 0;
			return;
		}
		else {
			memcpy( dst, ao->buffer + ao->bufferOffset, ao->bufferLen );
			dst += ao->bufferLen;
			size -= ao->bufferLen;
			latency += MICROSECOND * ( (double)ao->bufferLen / (double)ao->bytesPerSample ) / (double)ao->sampleRate;
			ao->bufferOffset = 	ao->bufferLen = 0;
		}
	}

	while ( size ) {
		ao->readData( &data, (tv.tv_sec * MICROSECOND) + tv.tv_usec + latency, ao->readUserData );
		if ( data ) {
			int s = data->profile.bytesPerChannel( &data->profile ) * data->profile.getAudioChannels() * data->audioSamples();
			latency += MICROSECOND * ( (double)s / (double)ao->bytesPerSample ) / (double)ao->sampleRate;
			if ( size < s ) {
				memcpy( dst, data->data(), size );
				s -= size;
				// buffer the remaining samples
				if ( ao->bufferSize < s ) {
					ao->bufferSize = s;
					ao->buffer = (uint8_t*)realloc( ao->buffer, ao->bufferSize );
					if ( !ao->buffer )
						qDebug() << "FATAL! Realloc failed. Expect a crash very soon.";
				}
				memcpy( ao->buffer, data->data() + size, s );
				ao->bufferLen += s;
				size = 0;
			}
			else {
				memcpy( dst, data->data(), s );
				dst += s;
				size -= s;
			}
			data->release();
			data = NULL;
		}
		else {
			usleep( 1000 );
			latency += 1000;
		}
	}
}

