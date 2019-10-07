#ifndef GLRESIZE_H
#define GLRESIZE_H

#include "vfx/glfilter.h"



class GLResize : public GLFilter
{
public:
	GLResize( QString id = "ResizeAuto", QString name = "ResizeAuto" );
	virtual ~GLResize();

	virtual QString getDescriptor( double pts, Frame *src, Profile *p );
	virtual bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	virtual QList<Effect*> getMovitEffects();
	
protected:
	virtual void preProcess( Frame *src, Profile *p );
};

#endif //GLRESIZE_H
