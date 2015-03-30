#ifndef FLIST_H
#define FLIST_H

#include <QList>
#include <QMutex>

#include "vfx/glfilter.h"
#include "afx/audiofilter.h"



// template constrained to QSharedPointer<Filter based objects>
template <class T>
class FList
{
public:
	FList() : current( -1 ) {}
	
	~FList() {
		QMutexLocker ml( &mutex );
		list.clear();
	}
	
	void append( T t ) {
		QMutexLocker ml( &mutex );
		list.append( t );
		current = list.count() - 1;
	}
	
	void insert( int index, T t ) {
		QMutexLocker ml( &mutex );
		list.insert( index, t );
		current = index;
	}	
	
	void removeAt( int i ) {
		QMutexLocker ml( &mutex );
		list.removeAt( i );
		current = list.count() - 1;
	}
	
	bool remove( T t ) {
		QMutexLocker ml( &mutex );
		int id = list.indexOf( t );
		if ( list.removeOne( t ) ) {
			if ( id == current )
				current = list.count() - 1;
			else if ( current > id )
				--current;				
			return true;
		}
		return false;
	}
	
	void move( int from, int to ) {
		QMutexLocker ml( &mutex );
		T t = list.takeAt( from );
		list.insert( to, t );
		current = to;
	}
	
	void swap( int i, int j ) {
		QMutexLocker ml( &mutex );
		list.swap( i, j );
	}
	
	int count() {
		QMutexLocker ml( &mutex );
		return list.count();
	}
	
	T at( int index ) {
		QMutexLocker ml( &mutex );
		return list.at( index );
	}
	
	QList<T> getCurrentFilters( double pts, double frameDuration ) {
		QMutexLocker ml( &mutex );
		QList<T> cur;
		for ( int i = 0; i < list.count(); ++i ) {
			T f = list.at( i );
			if ( f->getPosition() + f->getPositionOffset() <= pts + (frameDuration / 4 )
				&& f->getPosition() + f->getPositionOffset() + f->getLength() - frameDuration >= pts - (frameDuration / 4) )
				cur.append( f );
		}
		return cur;
	}
	
	QList<T> copy() {
		QMutexLocker ml( &mutex );
		return list;
	}
	
	QStringList filtersNames() {
		QMutexLocker ml( &mutex );
		QStringList s;
		int i;
		for ( i = 0; i < list.count(); ++i ) {
			s.append( list[i]->getFilterName() );
		}
		return s;
	}
	
	int currentIndex() {
		return current;
	}
	
	int setCurrentIndex( int cur ) {
		QMutexLocker ml( &mutex );
		current = qMax( qMin( cur, list.count() - 1 ), -1 );
		return current;
	}
	
private:
	QList<T> list;
	QMutex mutex;
	int current;
};

#endif // FLIST_H
