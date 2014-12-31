#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include <stdint.h>

#include <QList>
#include <QMutex>
#include <QTime>



class MPool;
class BufferPool;

class Buffer
{
public:
	uint8_t* data() {
		return buf;
	}
	
private:
	friend class BufferPool;
	friend class MPool;
	explicit Buffer( int size, int index ) : bufSize(size), poolIndex(index), refCount(1) {
		buf = (uint8_t*)malloc( bufSize );
	}
	~Buffer() {
		free( buf );
	}
	void use() {
		++refCount;
	}
	bool release() {
		return --refCount == 0;
	}

	int bufSize;
	int poolIndex;
	uint8_t *buf;
	int refCount;
};



class MPool
{
public:
	MPool( int size, int index ) : bufsize(size), poolIndex(index), totalBuffers(0) {
	}
	Buffer *getBuffer() {
		if ( bufferList.count() ) {
			Buffer *b = bufferList.takeLast();
			b->use();
			return b;
		}
		
		++totalBuffers;
		return new Buffer( bufsize, poolIndex );
	}
		
	int bufsize;
	int poolIndex;
	QList<Buffer*> bufferList;
	int totalBuffers;
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
	
private:
	QList<MPool*> freeBuffers;
	QMutex mutex;
};

#endif // BUFFERPOOL_H
