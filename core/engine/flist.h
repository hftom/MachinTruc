#ifndef FLIST_H
#define FLIST_H

#include <QList>
#include <QMutex>

#include "vfx/glfilter.h"
#include "afx/audiofilter.h"



// template constrained to Filter based objects
template <class T>
class FList
{
public:
	~FList() {
		QMutexLocker ml( &mutex );
		while ( !list.isEmpty() ) {
			Filter *t = (Filter*)list.takeFirst();
			t->release();
		}
	}
	
	void append( T t ) {
		QMutexLocker ml( &mutex );
		list.append( t );
	}
	
	bool remove( T t ) {
		QMutexLocker ml( &mutex );
		bool ok = list.removeOne( t );
		((Filter*)t)->release();
		return ok;
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
	
	void copy( QList<GLFilter*> *clist ) {
		QMutexLocker ml( &mutex );
		int i, j = list.count();
		for ( i = 0; i < j; ++i ) {
			GLFilter *f = (GLFilter*)list.at( i );
			f->use();
			clist->append( f );
		}
	}
	
	void copy( QList<AudioFilter*> *clist ) {
		QMutexLocker ml( &mutex );
		int i, j = list.count();
		for ( i = 0; i < j; ++i ) {
			AudioFilter *f = (AudioFilter*)list.at( i );
			f->use();
			clist->append( f );
		}
	}
	
	QStringList videoFiltersNames() {
		QMutexLocker ml( &mutex );
		QStringList s;
		int i;
		for ( i = 0; i < list.count(); ++i ) {
			GLFilter *f = (GLFilter*)list.at( i );
			s.append( f->getFilterName() );
		}
		return s;
	}
	
	QStringList audioFiltersNames() {
		QMutexLocker ml( &mutex );
		QStringList s;
		int i;
		for ( i = 0; i < list.count(); ++i ) {
			AudioFilter *a = (AudioFilter*)list.at( i );
			s.append( a->getFilterName() );
		}
		return s;
	}
	
private:
	QList<T> list;
	QMutex mutex;
};

#endif // FLIST_H
