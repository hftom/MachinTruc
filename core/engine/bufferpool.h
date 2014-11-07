#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include <stdint.h>

#include <QList>
#include <QMutex>



class BufferPool;

class Buffer
{
public:
	uint8_t* data() {
		return buf;
	}
	
private:
	friend class BufferPool;
	explicit Buffer( int size ) {
		bufSize = size;
		buf = (uint8_t*)malloc( bufSize );
		refCount = 1;
	}
	~Buffer() {
		free( buf );
	}
	int getBufferSize() {
		return bufSize;
	}
	void resizeBuffer( int size ) {
		bufSize = size;
		buf = (uint8_t*)realloc( buf, bufSize );
		if ( !buf )
			qDebug() << "FATAL! Realloc failed. Expect a crash very soon." << size;
	}
	void use() {
		++refCount;
	}
	bool release() {
		return --refCount == 0;
	}

	int bufSize;
	uint8_t *buf;
	int refCount;
};



class BufferPool
{
public:
	BufferPool();
	~BufferPool();
	
	Buffer* getBuffer( int size );
	void enlargeBuffer( Buffer *buf, int size );
	void releaseBuffer( Buffer *buf );
	void useBuffer( Buffer *buf );
	
	static BufferPool* globalInstance();
	
private:
	QList<Buffer*> freeBuffers;
	QMutex mutex;
	quint64 totalBytes, totalBuffers;
};

#endif // BUFFERPOOL_H
