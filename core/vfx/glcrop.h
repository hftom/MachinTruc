#ifndef GLCROP_H
#define GLCROP_H

#include "vfx/glfilter.h"



class GLCrop : public GLFilter
{
public:
	GLCrop( QString id, QString name );
	~GLCrop();

	QString getDescriptor( double pts, Frame *src, Profile *p ); 
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	void preProcess( double pts, Frame *src, Profile *p );

	Parameter *left, *right, *top, *bottom;
	double ptop, pleft, pright, pbottom;
};

#endif //GLCROP_H
