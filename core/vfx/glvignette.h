#ifndef GLVIGNETTE_H
#define GLVIGNETTE_H

#include "vfx/glfilter.h"



class GLVignette : public GLFilter
{
	Q_OBJECT
public:
	GLVignette( QString id, QString name );
	~GLVignette();

	virtual bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	virtual void ovdUpdate( QString type, QVariant val );

	virtual QList<Effect*> getMovitEffects();

private:
	Parameter *softness, *radius;
	Parameter *xOffset, *yOffset;
	QVariant ovdOffset;
};

#endif //GLVIGNETTE_H
