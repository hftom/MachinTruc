#include <math.h>

#include <QDebug>

#include "bufferpool.h"

#define MINSHIFT 12
#define MAXSHIFT 29



static BufferPool globalBufferPool;



BufferPool::BufferPool()
{
	/*for ( int i = MINSHIFT; i < MAXSHIFT; ++i )
		freeBuffers.append( new MPool( 1 << i, i - MINSHIFT ) );*/
}



BufferPool::~BufferPool()
{
}


	
Buffer* BufferPool::getBuffer( int size )
{
	QMutexLocker ml( &mutex );
	
	return new Buffer( size, 0 );
	
	/*int index = MINSHIFT;
	while ( (1 << index) < size )
		++index;

	return freeBuffers[index - MINSHIFT]->getBuffer();*/
}



Buffer* BufferPool::enlargeBuffer( Buffer *buf, int size )
{
	QMutexLocker ml( &mutex );
	
	Buffer *b = new Buffer( size, 0 );
	memcpy( b->data(), buf->data(), buf->bufSize );
	if ( buf->release() )
		delete buf;
	return b;
	
	/*int index = MINSHIFT;
	while ( (1 << index) < size )
		++index;

	if ( buf->poolIndex != index - MINSHIFT ) {
		Buffer *b = freeBuffers[index - MINSHIFT]->getBuffer();
		memcpy( b->data(), buf->data(), buf->bufSize );
		if ( buf->release() )
			freeBuffers[buf->poolIndex]->bufferList.append( buf );
		return b;
	}
	
	return buf;*/
}



void BufferPool::releaseBuffer( Buffer *buf )
{
	QMutexLocker ml( &mutex );

	if ( buf->release() )
		delete buf;
	
	/*if ( buf->release() )
		freeBuffers[buf->poolIndex]->bufferList.append( buf );*/
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
