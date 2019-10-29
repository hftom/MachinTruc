#include <QObject>
#include "filtercollection.h"



FilterCollection FilterCollection::globalInstance = FilterCollection();



FilterCollection::FilterCollection()
{
	// video filters
	//videoFilters.append( FilterEntry( "lens", "GLTest", QObject::tr("Test"), &Maker<GLTest>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLContrast", QObject::tr("Contrast Brightness"), &Maker<GLContrast>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", QObject::tr("Color grading"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBlur", QObject::tr("Blur"), &Maker<GLBlur>::make ) );
	//videoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", QObject::tr("Sharpen (deconvolution)"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLSharpen", QObject::tr("Sharpen"), &Maker<GLSharpen>::make ) );
	//videoFilters.append( FilterEntry( "lens", "GLDenoise", QObject::tr("Denoise"), &Maker<GLDenoise>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDiffusion", QObject::tr("Soften"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLGlow", QObject::tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "size", "GLMirror", QObject::tr("Mirror"), &Maker<GLMirror>::make ) );
	videoFilters.append( FilterEntry( "size", "GLSize", QObject::tr("Size and position"), &Maker<GLSize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLPanZoom", QObject::tr("Animated image"), &Maker<GLPanZoom>::make ) );
	videoFilters.append( FilterEntry( "size", "GLDistort", QObject::tr("Distort"), &Maker<GLDistort>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	videoFilters.append( FilterEntry( "size", "GLStabilize", QObject::tr("Stabilize"), &Maker<GLStabilize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLVignette", QObject::tr("Vignette"), &Maker<GLVignette>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLText", QObject::tr("Text"), &Maker<GLText>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBorder", QObject::tr("Border color"), &Maker<GLBorder>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBackgroundColor", QObject::tr("Background color"), &Maker<GLBackgroundColor>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLFiber", QObject::tr("Optical fiber"), &Maker<GLFiber>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLLaser", QObject::tr("Laser"), &Maker<GLLaser>::make ) );
	//videoFilters.append( FilterEntry( "draw", "GLEdge", QObject::tr("Edge"), &Maker<GLEdge>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLHandDrawing", QObject::tr("Hand drawing"), &Maker<GLHandDrawing>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLSoftBorder", QObject::tr("Soft border"), &Maker<GLSoftBorder>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLOpacity", QObject::tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLCut", QObject::tr("Erase"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeIn", QObject::tr("Fade in"), &Maker<GLFadeIn>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeOut", QObject::tr("Fade out"), &Maker<GLFadeOut>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLDropShadow", QObject::tr("Drop shadow"), &Maker<GLDropShadow>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLPixelize", QObject::tr("Pixelize"), &Maker<GLPixelize>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLWater", QObject::tr("Water"), &Maker<GLWater>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLKaleidoscope", QObject::tr("Kaleidoscope"), &Maker<GLKaleidoscope>::make ) );
	//videoFilters.append( FilterEntry( "deform", "GLCustom", QObject::tr("Custom"), &Maker<GLCustom>::make ) );
	
	// source video filters, a subset of videoFilters
	sourceVideoFilters.append( FilterEntry( "colors", "GLSaturation", QObject::tr("Saturation"), &Maker<GLSaturation>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLWhiteBalance", QObject::tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", QObject::tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	//sourceVideoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", QObject::tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "lens", "GLSharpen", QObject::tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "size", "GLCrop", QObject::tr("Crop"), &Maker<GLCrop>::make ) );
	sourceVideoFilters.append( FilterEntry( "size", "GLStabilize", QObject::tr("Stabilize"), &Maker<GLStabilize>::make ) );
	sourceVideoFilters.append( FilterEntry( "deform", "GLDefish", QObject::tr("Defish"), &Maker<GLDefish>::make ) );
	
	// video transitions
	videoTransitions.append( FilterEntry( "colors", "GLMix", QObject::tr("Crossfade"), &Maker<GLMix>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLPush", QObject::tr("Push"), &Maker<GLPush>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLCover", QObject::tr("Cover"), &Maker<GLCover>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLHardCut", QObject::tr("Hard cut"), &Maker<GLHardCut>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLFadeOutIn", QObject::tr("Fade out in"), &Maker<GLFadeOutIn>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLFrostedGlass", QObject::tr("Frosted glass"), &Maker<GLFrostedGlass>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLZoomIn", QObject::tr("Zoom in"), &Maker<GLZoomIn>::make ) );

	// audio filters
	audioFilters.append( FilterEntry( "sound", "AudioFadeIn", QObject::tr("Fade in"), &Maker<AudioFadeIn>::make ) );
	audioFilters.append( FilterEntry( "sound", "AudioFadeOut", QObject::tr("Fade out"), &Maker<AudioFadeOut>::make ) );
	audioFilters.append( FilterEntry( "sound", "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// source audio filters, a subset of audioFilters
	sourceAudioFilters.append( FilterEntry( "sound", "AudioVolume", QObject::tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// audio transitions
	audioTransitions.append( FilterEntry( "sound", "AudioCrossFade", QObject::tr("Crossfade"), &Maker<AudioCrossFade>::make ) );
	audioTransitions.append( FilterEntry( "sound", "AudioHardCut", QObject::tr("Hard cut"), &Maker<AudioHardCut>::make ) );
};



FilterCollection* FilterCollection::getGlobalInstance()
{
	return &globalInstance;
}



QSharedPointer<Filter> FilterCollection::createVideoFilter(QString filterName)
{
	QSharedPointer<Filter> f;
	for ( int i = 0; i < videoFilters.count(); ++i ) {
		if ( videoFilters.at(i).identifier == filterName ) {
			f = videoFilters[i].create();
			break;
		}
	}
	
	return f;
}



QSharedPointer<Filter> FilterCollection::createAudioFilter(QString filterName)
{
	QSharedPointer<Filter> f;
	for ( int i = 0; i < audioFilters.count(); ++i ) {
		if ( audioFilters.at(i).identifier == filterName ) {
			f = audioFilters[i].create();
			break;
		}
	}
	
	return f;
}



QSharedPointer<Filter> FilterCollection::createVideoTransitionFilter(QString filterName)
{
	QSharedPointer<Filter> f;
	for ( int i = 0; i < videoTransitions.count(); ++i ) {
		if ( videoTransitions.at(i).identifier == filterName ) {
			f = videoTransitions[i].create();
			break;
		}
	}
	
	return f;
}



QSharedPointer<Filter> FilterCollection::createAudioTransitionFilter(QString filterName)
{
	QSharedPointer<Filter> f;
	for ( int i = 0; i < audioTransitions.count(); ++i ) {
		if ( audioTransitions.at(i).identifier == filterName ) {
			f = audioTransitions[i].create();
			break;
		}
	}
	
	return f;
}
