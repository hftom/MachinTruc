#ifndef GLPADDING_H
#define GLPADDING_H

#include "vfx/glfilter.h"



class GLPadding : public GLFilter
{
public:
    GLPadding( QString id = "PaddingAuto", QString name = "PaddingAuto" );
    ~GLPadding();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
	
private:
	float xoffsetpercent, yoffsetpercent;
};

#endif //GLPADDING_H
