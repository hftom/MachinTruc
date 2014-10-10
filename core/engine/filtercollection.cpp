#include <QObject>
#include "filtercollection.h"



static FilterCollection globalFilterCollection;



FilterCollection::FilterCollection()
{
	// video filters
	videoFilters.append( FilterEntry( "GLTest", QObject::tr("Test"), &Maker<GLTest>::make ) );
	videoFilters.append( FilterEntry( "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "GLBlur", QObject::tr("Blur"), &Maker<GLBlur>::make ) );
	videoFilters.append( FilterEntry( "GLPixelize", QObject::tr("Pixelize"), &Maker<GLPixelize>::make ) );
	videoFilters.append( FilterEntry( "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	videoFilters.append( FilterEntry( "GLSize", QObject::tr("Size and position"), &Maker<GLSize>::make ) );
	videoFilters.append( FilterEntry( "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	videoFilters.append( FilterEntry( "GLCut", QObject::tr("Cut"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "GLSoftBorder", QObject::tr("Soft border"), &Maker<GLSoftBorder>::make ) );
	videoFilters.append( FilterEntry( "GLBorder", QObject::tr("Border color"), &Maker<GLBorder>::make ) );
	videoFilters.append( FilterEntry( "GLBackgroundColor", QObject::tr("Background color"), &Maker<GLBackgroundColor>::make ) );
	videoFilters.append( FilterEntry( "GLOpacity", QObject::tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "GLDropShadow", QObject::tr("Drop shadow"), &Maker<GLDropShadow>::make ) );
	videoFilters.append( FilterEntry( "GLVignette", QObject::tr("Vignette"), &Maker<GLVignette>::make ) );
	videoFilters.append( FilterEntry( "GLDiffusion", QObject::tr("Diffusion"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "GLGlow", QObject::tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "GLWater", QObject::tr("Water"), &Maker<GLWater>::make ) );
	videoFilters.append( FilterEntry( "GLEdge", QObject::tr("Edge"), &Maker<GLEdge>::make ) );
	
	// source video filters, a subset of videoFilters
	sourceVideoFilters.append( FilterEntry( "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	sourceVideoFilters.append( FilterEntry( "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	sourceVideoFilters.append( FilterEntry( "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	
	// video transitions
	videoTransitions.append( FilterEntry( "GLMix", QObject::tr("Mix"), &Maker<GLMix>::make ) );



	// audio filters
	audioFilters.append( FilterEntry( "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// source audio filters, a subset of audioFilters
	sourceAudioFilters.append( FilterEntry( "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// audio transitions
	audioTransitions.append( FilterEntry( "AudioCrossFade", QObject::tr("Crossfade"), &Maker<AudioCrossFade>::make ) );
};



FilterCollection* FilterCollection::getGlobalInstance()
{
	return &globalFilterCollection;
}
