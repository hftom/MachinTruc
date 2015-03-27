#include <QObject>
#include "filtercollection.h"



static FilterCollection globalFilterCollection;



FilterCollection::FilterCollection()
{
	// video filters
	//videoFilters.append( FilterEntry( "lens", "GLTest", QObject::tr("Test"), &Maker<GLTest>::make ) );
	videoFilters.append( FilterEntry( "color", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "color", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	videoFilters.append( FilterEntry( "color", "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBlur", QObject::tr("Blur"), &Maker<GLBlur>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLPixelize", QObject::tr("Pixelize"), &Maker<GLPixelize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLSize", QObject::tr("Size and position"), &Maker<GLSize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCut", QObject::tr("Cut"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "size", "GLVignette", QObject::tr("Vignette"), &Maker<GLVignette>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBorder", QObject::tr("Border color"), &Maker<GLBorder>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBackgroundColor", QObject::tr("Background color"), &Maker<GLBackgroundColor>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLSoftBorder", QObject::tr("Soft border"), &Maker<GLSoftBorder>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLOpacity", QObject::tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLDropShadow", QObject::tr("Drop shadow"), &Maker<GLDropShadow>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDiffusion", QObject::tr("Diffusion"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLGlow", QObject::tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLWater", QObject::tr("Water"), &Maker<GLWater>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLEdge", QObject::tr("Edge"), &Maker<GLEdge>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLFiber", QObject::tr("Optical fiber"), &Maker<GLFiber>::make ) );
	
	// source video filters, a subset of videoFilters
	sourceVideoFilters.append( FilterEntry( "color", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	sourceVideoFilters.append( FilterEntry( "color", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	
	// video transitions
	videoTransitions.append( FilterEntry( "color", "GLMix", QObject::tr("Crossfade"), &Maker<GLMix>::make ) );
	videoTransitions.append( FilterEntry( "color", "GLPush", QObject::tr("Push"), &Maker<GLPush>::make ) );
	videoTransitions.append( FilterEntry( "color", "GLCover", QObject::tr("Cover"), &Maker<GLCover>::make ) );
	videoTransitions.append( FilterEntry( "color", "GLHardCut", QObject::tr("Hard cut"), &Maker<GLHardCut>::make ) );
	videoTransitions.append( FilterEntry( "color", "GLFrostedGlass", QObject::tr("Frosted glass"), &Maker<GLFrostedGlass>::make ) );

	// audio filters
	audioFilters.append( FilterEntry( "sound", "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// source audio filters, a subset of audioFilters
	sourceAudioFilters.append( FilterEntry( "sound", "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// audio transitions
	audioTransitions.append( FilterEntry( "sound", "AudioCrossFade", QObject::tr("Crossfade"), &Maker<AudioCrossFade>::make ) );
};



FilterCollection* FilterCollection::getGlobalInstance()
{
	return &globalFilterCollection;
}
