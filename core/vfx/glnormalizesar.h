#ifndef GLNORMALIZESAR_H
#define GLNORMALIZESAR_H

#include "vfx/glresize.h"



class GLNormalizeSAR : public GLResize
{
public:
	GLNormalizeSAR( QString id = "NormalizeSARAuto", QString name = "NormalizeSARAuto" );
	virtual ~GLNormalizeSAR();
	
protected:
	virtual void preProcess( Frame *src, Profile *p );
};

#endif //GLNORMALIZESAR_H
