#ifndef GLDISTORT_H
#define GLDISTORT_H

#include "vfx/glfilter.h"



class GLDistort : public GLFilter
{
	Q_OBJECT
public:
	GLDistort( QString id, QString name );
	~GLDistort();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *ratio;
};

#endif //GLDISTORT_H
