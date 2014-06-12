#ifndef AUDIOOUTSDL_H
#define AUDIOOUTSDL_H

#include <sys/time.h>

#include <SDL/SDL_audio.h>

#include "engine/frame.h"

typedef void (*READDATACALLBACK) ( Frame **data, double time, void *userdata );



class AudioOutSDL
{
public:
    AudioOutSDL();
    ~AudioOutSDL();
    void setReadCallback( void *func, void *userdata );

    void go();
    void stop();

private:
    static void streamRequestCallback( void *userdata, uint8_t *stream, int len );

    uint8_t *buffer;
    int bufferSize, bufferLen, bufferOffset;
    int bytesPerSample, sampleRate;

    READDATACALLBACK readData;
    void *readUserData;
};
#endif //AUDIOOUTSDL_H
