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
	Parameter *left, *right, *top, *bottom;
	double ptop, pleft;
};

#endif //GLCROP_H
