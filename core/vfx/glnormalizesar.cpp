#include "vfx/glnormalizesar.h"



GLNormalizeSAR::GLNormalizeSAR( QString id, QString name ) : GLResize( id, name )
{
}



GLNormalizeSAR::~GLNormalizeSAR()
{
}



void GLNormalizeSAR::preProcess( Frame *src, Profile *p )
{
	src->glWidth = (double)src->glWidth * src->glSAR  / p->getVideoSAR();
	src->glSAR = p->getVideoSAR();

	if ( src->glWidth < 1 )
		src->glWidth = 1;
	if ( src->glHeight < 1 )
		src->glHeight = 1;
}
