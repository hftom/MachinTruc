#ifndef GLRESIZE_H
#define GLRESIZE_H

#include "vfx/glfilter.h"



class GLResize : public GLFilter
{
public:
	GLResize( QString id = "ResizeAuto", QString name = "ResizeAuto" );
	~GLResize();

	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	void preProcess( Frame *src, Profile *p );

	Parameter *percent;
};

#endif //GLRESIZE_H
