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
	~FList() {
		QMutexLocker ml( &mutex );
		list.clear();
	}
	
	void append( T t ) {
		QMutexLocker ml( &mutex );
		list.append( t );
	}
	
	void removeAt( int i ) {
		QMutexLocker ml( &mutex );
		return list.removeAt( i );
	}
	
	bool remove( T t ) {
		QMutexLocker ml( &mutex );
		return list.removeOne( t );
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
	
private:
	QList<T> list;
	QMutex mutex;
};

#endif // FLIST_H
