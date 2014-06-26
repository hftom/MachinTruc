#ifndef GLPADDING_H
#define GLPADDING_H

#include "vfx/glfilter.h"



class GLPadding : public GLFilter
{
public:
    GLPadding( QString id = "PaddingAuto", QString name = "PaddingAuto" );
    ~GLPadding();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float xoffsetpercent, yoffsetpercent;
};

#endif //GLPADDING_H
