#ifndef GLCROP_H
#define GLCROP_H

#include "vfx/glfilter.h"



class GLCrop : public GLFilter
{
public:
    GLCrop( QString id, QString name );
    ~GLCrop();

	bool preProcess( Frame *src, Profile *p ); 
    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float left, right, top, bottom;
	float ptop, pleft;
};

#endif //GLCROP_H
