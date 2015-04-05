#include <QObject>
#include "filtercollection.h"



static FilterCollection globalFilterCollection;



FilterCollection::FilterCollection()
{
	// video filters
	//videoFilters.append( FilterEntry( "lens", "GLTest", QObject::tr("Test"), &Maker<GLTest>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", QObject::tr("Color grading"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBlur", QObject::tr("Blur"), &Maker<GLBlur>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDiffusion", QObject::tr("Diffusion"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLGlow", QObject::tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "size", "GLSize", QObject::tr("Size and position"), &Maker<GLSize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCut", QObject::tr("Cut"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "size", "GLVignette", QObject::tr("Vignette"), &Maker<GLVignette>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLText", QObject::tr("Text"), &Maker<GLText>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBorder", QObject::tr("Border color"), &Maker<GLBorder>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBackgroundColor", QObject::tr("Background color"), &Maker<GLBackgroundColor>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLFiber", QObject::tr("Optical fiber"), &Maker<GLFiber>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLEdge", QObject::tr("Edge"), &Maker<GLEdge>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLSoftBorder", QObject::tr("Soft border"), &Maker<GLSoftBorder>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLOpacity", QObject::tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeIn", QObject::tr("Fade in"), &Maker<GLFadeIn>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeOut", QObject::tr("Fade out"), &Maker<GLFadeOut>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLDropShadow", QObject::tr("Drop shadow"), &Maker<GLDropShadow>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLPixelize", QObject::tr("Pixelize"), &Maker<GLPixelize>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLWater", QObject::tr("Water"), &Maker<GLWater>::make ) );
	
	// source video filters, a subset of videoFilters
	sourceVideoFilters.append( FilterEntry( "colors", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	
	// video transitions
	videoTransitions.append( FilterEntry( "colors", "GLMix", QObject::tr("Crossfade"), &Maker<GLMix>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLPush", QObject::tr("Push"), &Maker<GLPush>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLCover", QObject::tr("Cover"), &Maker<GLCover>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLHardCut", QObject::tr("Hard cut"), &Maker<GLHardCut>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLFrostedGlass", QObject::tr("Frosted glass"), &Maker<GLFrostedGlass>::make ) );

	// audio filters
	audioFilters.append( FilterEntry( "sound", "AudioFadeIn", QObject::tr("Fade in"), &Maker<AudioFadeIn>::make ) );
	audioFilters.append( FilterEntry( "sound", "AudioFadeOut", QObject::tr("Fade out"), &Maker<AudioFadeOut>::make ) );
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
