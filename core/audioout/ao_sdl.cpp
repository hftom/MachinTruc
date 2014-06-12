#include <unistd.h>

#include <SDL/SDL.h>

#include "audioout/ao_sdl.h"



AudioOutSDL::AudioOutSDL()
{
    readData = NULL;
    readUserData = NULL;

    bufferLen = bufferOffset = 0;
    bufferSize = 1024;
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
    int latency = MICROSECOND * (len / ao->bytesPerSample) / ao->sampleRate;

    uint8_t *dst = stream;
    int size = len;

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
            latency += MICROSECOND * ((len-size) / ao->bytesPerSample) / ao->sampleRate;
            ao->bufferOffset = 	ao->bufferLen = 0;
        }
    }

    while ( size ) {
        ao->readData( &data, (tv.tv_sec * MICROSECOND) + tv.tv_usec + latency, ao->readUserData );
        if ( data ) {
            int s = data->profile.bytesPerChannel( &data->profile ) * data->profile.getAudioChannels() * data->audioSamples();
            if ( size < s ) {
                memcpy( dst, data->data(), size );
                s -= size;
                if ( ao->bufferSize < s ) {
                    ao->bufferSize = s;
                    ao->buffer = (uint8_t*)realloc( ao->buffer, ao->bufferSize );
                }
                // buffer the remaining samples
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
    }
}

