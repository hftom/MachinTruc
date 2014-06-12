#include <QObject>
#include "filtercollection.h"



static FilterCollection globalFilterCollection;



FilterCollection::FilterCollection()
{
	videoFilters.append( FilterEntry( "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "GLBlur", QObject::tr("Blur"), &Maker<GLBlur>::make ) );
	videoFilters.append( FilterEntry( "GLCut", QObject::tr("Cut"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	videoFilters.append( FilterEntry( "GLEdge", QObject::tr("Edge"), &Maker<GLEdge>::make ) );
	videoFilters.append( FilterEntry( "GLDiffusion", QObject::tr("Diffusion"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "GLGlow", QObject::tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "GLOpacity", QObject::tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "GLWater", QObject::tr("Water"), &Maker<GLWater>::make ) );
	
	audioFilters.append( FilterEntry( "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
};



FilterCollection* FilterCollection::getGlobal()
{
	return &globalFilterCollection;
}
