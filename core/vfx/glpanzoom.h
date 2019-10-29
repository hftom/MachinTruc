#ifndef GLPANZOOM_H
#define GLPANZOOM_H

#include "vfx/glsize.h"



class GLPanZoom : public GLSize
{
public:
	GLPanZoom( QString id, QString name );
	
protected:
	virtual void preProcess(double pts, Frame *src, Profile *p );
	
private:
	int animType;
	double  pw, ph, sw, sh;
	double xDirection, yDirection;

};

#endif //GLPANZOOM_H
