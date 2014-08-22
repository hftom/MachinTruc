#ifndef GLVIGNETTE_H
#define GLVIGNETTE_H

#include "vfx/glfilter.h"



class GLVignette : public GLFilter
{
public:
	GLVignette( QString id, QString name );
	~GLVignette();

	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *softness, *radius;
	Parameter *centerX, *centerY;
};

#endif //GLVIGNETTE_H
