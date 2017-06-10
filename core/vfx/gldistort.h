#ifndef GLDISTORT_H
#define GLDISTORT_H

#include "vfx/glfilter.h"



class GLDistort : public GLFilter
{
public:
	GLDistort( QString id, QString name );
	~GLDistort();
	
	QString getDescriptor( double pts, Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	void preProcess( Frame *src, Profile *p );
	
	Parameter *ratio;
};

#endif //GLDISTORT_H
