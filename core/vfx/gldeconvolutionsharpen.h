#ifndef GLDECONVOLUTIONSHARPEN_H
#define GLDECONVOLUTIONSHARPEN_H

#include "vfx/glfilter.h"



class GLDeconvolutionSharpen : public GLFilter
{
public:
    GLDeconvolutionSharpen( QString id, QString name );
    ~GLDeconvolutionSharpen();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
	QString getDescriptor();
	
private:
	int R;
	float circleRadius;
	float gaussianRadius;
	float correlation;
	float noise;
};

#endif //GLDECONVOLUTIONSHARPEN_H
