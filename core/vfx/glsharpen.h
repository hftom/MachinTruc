#ifndef GLSHARPEN_H
#define GLSHARPEN_H

#include "vfx/glfilter.h"



class GLSharpen : public GLFilter
{
public:
    GLSharpen( QString id, QString name );
    ~GLSharpen();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	float amount;
};

#endif //GLSHARPEN_H
