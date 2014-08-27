#include <math.h>

#include <QDebug>

#include "bufferpool.h"



static BufferPool globalBufferPool;



BufferPool::BufferPool()
{
}



BufferPool::~BufferPool()
{
}


	
Buffer* BufferPool::getBuffer( int size )
{
	QMutexLocker ml( &mutex );
	int i, index = -1, indexDiff = 0;

	for ( i = 0; i < freeBuffers.count(); ++i ) {
		Buffer *b = freeBuffers[i];
		int s = b->getBufferSize();
		if ( s == size ) {
			freeBuffers.takeAt( i );
			b->use();
			return b;
		}
		int diff = abs( size - s );
		if ( index == -1 || diff < indexDiff ) {
			indexDiff = diff;
			index = i;
		}		
	}
	
	if ( index != -1 ) {
		Buffer *b = freeBuffers.takeAt( index );
		b->resizeBuffer( size );
		b->use();
		return b;
	}

	return new Buffer( size );
}



void BufferPool::releaseBuffer( Buffer *buf )
{
	QMutexLocker ml( &mutex );
	if ( buf->release() )
		freeBuffers.prepend( buf );
}



BufferPool* BufferPool::globalInstance()
{
	return &globalBufferPool;
}
