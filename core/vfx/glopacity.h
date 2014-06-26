#ifndef GLOPACITY_H
#define GLOPACITY_H

#include "vfx/glfilter.h"



class GLOpacity : public GLFilter
{
public:
	GLOpacity( QString id, QString name );
	~GLOpacity();

	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float factor;
};

#endif //GLOPACITY_H
