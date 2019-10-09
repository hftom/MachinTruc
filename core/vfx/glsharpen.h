#ifndef GLSHARPEN_H
#define GLSHARPEN_H

#include "vfx/glfilter.h"



class GLSharpen : public GLFilter
{
public:
	GLSharpen( QString id, QString name );
	~GLSharpen();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *amount, *blur;
};

#endif //GLSHARPEN_H
