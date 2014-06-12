#ifndef GLOPACITY_H
#define GLOPACITY_H

#include "vfx/glfilter.h"



class GLOpacity : public GLFilter
{
public:
	GLOpacity( QString id, QString name );
	~GLOpacity();

	bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
	
private:
	float factor;
};

#endif //GLOPACITY_H
