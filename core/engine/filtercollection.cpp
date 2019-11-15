#include <QObject>
#include "filtercollection.h"



//FilterCollection FilterCollection::globalInstance = FilterCollection();
FilterCollection* FilterCollection::globalInstance = NULL;



FilterCollection::FilterCollection()
{
	// video filters
	//videoFilters.append( FilterEntry( "lens", "GLTest", tr("Test"), &Maker<GLTest>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLSaturation", tr("Saturation"), &Maker<GLSaturation>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLContrast", tr("Contrast Brightness"), &Maker<GLContrast>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLWhiteBalance", tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	videoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", tr("Color grading"), &Maker<GLLiftGammaGain>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLBlur", tr("Blur"), &Maker<GLBlur>::make ) );
	//videoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", tr("Sharpen (deconvolution)"), &Maker<GLDeconvolutionSharpen>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLSharpen", tr("Sharpen"), &Maker<GLSharpen>::make ) );
	//videoFilters.append( FilterEntry( "lens", "GLDenoise", tr("Denoise"), &Maker<GLDenoise>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLDiffusion", tr("Soften"), &Maker<GLDiffusion>::make ) );
	videoFilters.append( FilterEntry( "lens", "GLGlow", tr("Glow"), &Maker<GLGlow>::make ) );
	videoFilters.append( FilterEntry( "size", "GLMirror", tr("Mirror"), &Maker<GLMirror>::make ) );
	videoFilters.append( FilterEntry( "size", "GLSize", tr("Size and position"), &Maker<GLSize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLPanZoom", tr("Animated image"), &Maker<GLPanZoom>::make ) );
	videoFilters.append( FilterEntry( "size", "GLDistort", tr("Distort"), &Maker<GLDistort>::make ) );
	videoFilters.append( FilterEntry( "size", "GLCrop", tr("Crop"), &Maker<GLCrop>::make ) );
	videoFilters.append( FilterEntry( "size", "GLStabilize", tr("Stabilize"), &Maker<GLStabilize>::make ) );
	videoFilters.append( FilterEntry( "size", "GLVignette", tr("Vignette"), &Maker<GLVignette>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLText", tr("Text"), &Maker<GLText>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBorder", tr("Border color"), &Maker<GLBorder>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLBackgroundColor", tr("Background color"), &Maker<GLBackgroundColor>::make ) );
	//videoFilters.append( FilterEntry( "draw", "GLEdge", tr("Edge"), &Maker<GLEdge>::make ) );
	videoFilters.append( FilterEntry( "draw", "GLHandDrawing", tr("Hand drawing"), &Maker<GLHandDrawing>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLSoftBorder", tr("Soft border"), &Maker<GLSoftBorder>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLOpacity", tr("Opacity"), &Maker<GLOpacity>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLCut", tr("Erase"), &Maker<GLCut>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeIn", tr("Fade in"), &Maker<GLFadeIn>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLFadeOut", tr("Fade out"), &Maker<GLFadeOut>::make ) );
	videoFilters.append( FilterEntry( "alpha", "GLDropShadow", tr("Drop shadow"), &Maker<GLDropShadow>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLDefish", tr("Defish"), &Maker<GLDefish>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLPixelize", tr("Pixelize"), &Maker<GLPixelize>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLWater", tr("Water"), &Maker<GLWater>::make ) );
	videoFilters.append( FilterEntry( "deform", "GLKaleidoscope", tr("Kaleidoscope"), &Maker<GLKaleidoscope>::make ) );
	//videoFilters.append( FilterEntry( "deform", "GLCustom", tr("Custom"), &Maker<GLCustom>::make ) );
	
	// source video filters, a subset of videoFilters
	sourceVideoFilters.append( FilterEntry( "colors", "GLSaturation", tr("Saturation"), &Maker<GLSaturation>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLWhiteBalance", tr("White balance"), &Maker<GLWhiteBalance>::make ) );
	sourceVideoFilters.append( FilterEntry( "colors", "GLLiftGammaGain", tr("Lift Gamma Gain"), &Maker<GLLiftGammaGain>::make ) );
	//sourceVideoFilters.append( FilterEntry( "lens", "GLDeconvolutionSharpen", tr("Sharpen"), &Maker<GLDeconvolutionSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "lens", "GLSharpen", tr("Sharpen (fast)"), &Maker<GLSharpen>::make ) );
	sourceVideoFilters.append( FilterEntry( "size", "GLCrop", tr("Crop"), &Maker<GLCrop>::make ) );
	sourceVideoFilters.append( FilterEntry( "size", "GLStabilize", tr("Stabilize"), &Maker<GLStabilize>::make ) );
	sourceVideoFilters.append( FilterEntry( "deform", "GLDefish", tr("Defish"), &Maker<GLDefish>::make ) );
	
	// video transitions
	videoTransitions.append( FilterEntry( "colors", "GLMix", tr("Crossfade"), &Maker<GLMix>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLPush", tr("Push"), &Maker<GLPush>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLCover", tr("Cover"), &Maker<GLCover>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLHardCut", tr("Hard cut"), &Maker<GLHardCut>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLFadeOutIn", tr("Fade out in"), &Maker<GLFadeOutIn>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLFrostedGlass", tr("Frosted glass"), &Maker<GLFrostedGlass>::make ) );
	videoTransitions.append( FilterEntry( "colors", "GLZoomIn", tr("Zoom in"), &Maker<GLZoomIn>::make ) );

	// audio filters
	audioFilters.append( FilterEntry( "sound", "AudioFadeIn", tr("Fade in"), &Maker<AudioFadeIn>::make ) );
	audioFilters.append( FilterEntry( "sound", "AudioFadeOut", tr("Fade out"), &Maker<AudioFadeOut>::make ) );
	audioFilters.append( FilterEntry( "sound", "AudioVolume", tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// source audio filters, a subset of audioFilters
	sourceAudioFilters.append( FilterEntry( "sound", "AudioVolume", tr("Volume"), &Maker<AudioVolume>::make ) );
	
	// audio transitions
	audioTransitions.append( FilterEntry( "sound", "AudioCrossFade", tr("Crossfade"), &Maker<AudioCrossFade>::make ) );
	audioTransitions.append( FilterEntry( "sound", "AudioHardCut", tr("Hard cut"), &Maker<AudioHardCut>::make ) );
};



FilterCollection* FilterCollection::getGlobalInstance()
{
	if (!globalInstance) {
		globalInstance = new FilterCollection();
	}
	return globalInstance;
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
