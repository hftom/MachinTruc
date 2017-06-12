#include <math.h>

#include <QDebug>

#include "bufferpool.h"

#define SMALLSLICE 10240
#define SMALLMAX 51200
#define MEDIUMSLICE 102400
#define MEDIUMMAX 512000
#define BIGSLICE 1048576



static BufferPool globalBufferPool;



BufferPool::BufferPool()
{
    for ( int i = 0; i < 3; ++i )
        chunkList[i] = new QList<MemChunk*>;
}



BufferPool::~BufferPool()
{
}



Buffer* BufferPool::getBuffer( int size )
{
    QMutexLocker ml( &mutex );

    int list = 0, s = SMALLSLICE;
    if ( size >= MEDIUMMAX ) {
        list = 2;
        s = BIGSLICE;
    }
    else if ( size >= SMALLMAX ) {
        list = 1;
        s = MEDIUMSLICE;
    }

    for ( int i = 0; i < chunkList[list]->count(); ++i ) {
        Buffer *b = chunkList[list]->at(i)->getBuffer( size );
        if ( b )
            return b;
    }

    MemChunk *m = new MemChunk( s );
    chunkList[list]->append( m );
    return m->getBuffer( size );
}



Buffer* BufferPool::enlargeBuffer( Buffer *buf, int size )
{
    QMutexLocker ml( &mutex );

    int list = 0, s = SMALLSLICE;
    if ( size >= MEDIUMMAX ) {
        list = 2;
        s = BIGSLICE;
    }
    else if ( size >= SMALLMAX ) {
        list = 1;
        s = MEDIUMSLICE;
    }

    Buffer *b = NULL;
    for ( int i = 0; i < chunkList[list]->count(); ++i ) {
        b = chunkList[list]->at(i)->getBuffer( size );
        if ( b )
            break;
    }
    if ( !b ) {
        MemChunk *m = new MemChunk( s );
        chunkList[list]->append( m );
        b = m->getBuffer( size );
    }

    memcpy( b->data(), buf->data(), buf->numSlices * buf->chunk->sliceSize );
    if ( buf->release() ) {
        buf->chunk->release( buf );
        delete buf;
    }
    return b;
}



void BufferPool::releaseBuffer( Buffer *buf )
{
    QMutexLocker ml( &mutex );

    if ( buf->release() ) {
        buf->chunk->release( buf );
        delete buf;
    }
}



void BufferPool::useBuffer( Buffer *buf )
{
    QMutexLocker ml( &mutex );
    buf->use();
}



BufferPool* BufferPool::globalInstance()
{
    return &globalBufferPool;
}
