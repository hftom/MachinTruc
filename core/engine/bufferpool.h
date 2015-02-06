#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include <stdint.h>
#include <math.h>

#include <QList>
#include <QMutex>
#include <QTime>



/* We now allocate chunks that are stored in pools.
 * A Buffer then gets assigned a set of contiguous chunk's slices.
 * Each chunk is made of 128 slices, and we have 3 pools of
 * different slice size : 10K, 100K and 1M.
 * This reduces memory fragmentation better than malloc/free
 * while allowing a better reusability than non sliced pools.
 * We haven't observed any significant slowdown, 
 * even on an old Celeron M 430 laptop.
 * 
 * At that time, unused chunks are not freed.
 * */


class MemChunk;
class BufferPool;

class Buffer
{
public:
	uint8_t* data() {
		return buf;
	}
	
private:
	friend class BufferPool;
	friend class MemChunk;
	explicit Buffer( uint8_t *b, int cpos, int ns, MemChunk *mchunk )
		: posInChunk(cpos),
		numSlices( ns ),
		buf(b),
		chunk(mchunk),
		refCount(1) {
	}
	~Buffer() {
	}
	void use() {
		++refCount;
	}
	bool release() {
		return --refCount == 0;
	}

	int posInChunk;
	int numSlices;
	uint8_t *buf;
	MemChunk *chunk;
	int refCount;
};



class MemChunk
{
public:
	MemChunk( int ssize ) : sliceSize(ssize), freeSlices(128) {
		buf = (uint8_t*)malloc( 128 * sliceSize );
		memset( used, 0, sizeof(bool) * 128 );
	}
	Buffer *getBuffer( int size ) {
		int i, j;
		int n = qMax( 1, (int)ceil( (float)size / (float)sliceSize ) );
		if ( n > freeSlices )
			return NULL;

		for ( i = 0; i < 128 - n + 1; ++i ) {
			if ( !used[i] ) {
				bool u = false;
				for ( j = 0; j < n; ++j ) {
					if ( used[i + j] ) {
						u = true;
						break;
					}
				}
				if ( !u ) {
					memset( &used[i], 1, n * sizeof(bool) );
					freeSlices -= n;
					return new Buffer( buf + (i * sliceSize), i, n, this );
				}
			}
		}
		return NULL;
	}
	void release( Buffer *buffer ) {
		memset( &used[buffer->posInChunk], 0, buffer->numSlices * sizeof(bool) );
		freeSlices += buffer->numSlices;
	}
	
	uint8_t *buf;
	int sliceSize;
	int freeSlices;
	bool used[128];
};



class BufferPool
{
public:
	BufferPool();
	~BufferPool();
	
	Buffer* getBuffer( int size );
	Buffer* enlargeBuffer( Buffer *buf, int size );
	void releaseBuffer( Buffer *buf );
	void useBuffer( Buffer *buf );
	
	static BufferPool* globalInstance();
	
//private:
	QList<MemChunk*>* chunkList[3];
	QMutex mutex;
};

#endif // BUFFERPOOL_H
