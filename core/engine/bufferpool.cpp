#include <math.h>

#include <QDebug>

#include "bufferpool.h"



static BufferPool globalBufferPool;



BufferPool::BufferPool()
	: totalBytes( 0 ),
	totalBuffers( 0 )
{
	time.start();
}



BufferPool::~BufferPool()
{
}


	
Buffer* BufferPool::getBuffer( int size )
{
	QMutexLocker ml( &mutex );
	int i, index = -1, indexDiff = 0;
	
	if ( time.elapsed() > 30000 ) {
		QTime t = QTime::currentTime();
		for ( i = 0; i < freeBuffers.count(); ++i ) {
			Buffer *b = freeBuffers[i];
			if ( b->releasedTime.secsTo( t ) > 10 ) {
				totalBytes -= b->getBufferSize();
				--totalBuffers;
				delete freeBuffers.takeAt( i-- );
			}
		}
		time.restart();
		qDebug() << "clean - total buffers:" << totalBuffers << "total bytes:" << totalBytes;
	}

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
		totalBytes -= b->getBufferSize();
		b->resizeBuffer( size );
		totalBytes += size;
		b->use();
		//qDebug() << "total buffers:" << totalBuffers << "total bytes:" << totalBytes;
		return b;
	}

	totalBytes += size;
	++totalBuffers;
	//qDebug() << "total buffers:" << totalBuffers << "total bytes:" << totalBytes;
	return new Buffer( size );
}



void BufferPool::enlargeBuffer( Buffer *buf, int size )
{
	QMutexLocker ml( &mutex );
	totalBytes -= buf->getBufferSize();
	buf->resizeBuffer( size );
	totalBytes += size;
}



void BufferPool::releaseBuffer( Buffer *buf )
{
	QMutexLocker ml( &mutex );
	if ( buf->release() )
		freeBuffers.prepend( buf );
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
