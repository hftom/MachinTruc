#ifndef SAMPLER_H
#define SAMPLER_H

#include "input/input_ff.h"
#include "input/input_image.h"
#include "input/input_blank.h"

#include "engine/scene.h"
#include "engine/metronom.h"



class Composer;



class Sampler : public QObject
{
	Q_OBJECT
public:
	Sampler();
	~Sampler();

	void getVideoTracks( Frame *dst );
	void getAudioTracks( Frame *dst, int nSamples );
	void prepareInputs();
	bool sceneEndReached();
	double currentSceneDuration();
	double currentTimelineSceneDuration();
	double currentPTS();
	double currentPTSAudio();
	void shiftCurrentPTS();
	void shiftCurrentPTSAudio();
	void rewardPTS();
	
	void setOutputResize( QSize size );
	
	void newProject( Profile p );
	bool setProfile( Profile p );
	Profile getProfile();
	Metronom* getMetronom() { return metronom; }
	bool play( bool b, bool backward = false );
	
	bool trackRequest( bool rm, int index );
	void clearAll();
	bool isProjectEmpty();
	QList<Scene*> getSceneList() { return sceneList; }
	Scene* getCurrentScene() { return timelineScene; }
	void setSceneList( QList<Scene*> list );
	bool previewMode() { return currentScene == preview; }
	
	// called from composer thread
	void fromComposerSeekTo( double p, bool backward = false, bool seek = true );
	double fromComposerSetPlaybackBuffer( bool backward );
	bool fromComposerUpdateFrame( Frame *f );
	void fromComposerReleaseVideoFrame( Frame *f );

public slots:
	void setSharedContext( QGLWidget *shared );
	void setFencesContext( QGLWidget *shared );
	void switchMode( bool down );
	void setSource( Source *source, double pts );
	void wheelSeek( int a );
	void slideSeek( double p );
	void updateFrame();

private:
	void drainScenes();
	void stopComposer();
	Clip* searchCurrentClip( int &i, Track *t, int clipIndex, double pts, double margin );
	void prepareInputsBackward();
	double sceneDuration( Scene *s );
	int updateVideoFrame( Frame *dst );
	void updateAudioFrame( Frame *dst );
	InputBase* getInput( QString fn, InputBase::InputType type );
	InputBase* getClipInput( Clip *c, double pts );

	QList<Scene*> sceneList;
	Scene *timelineScene;
	Scene *preview;
	Scene *currentScene;
	QList<InputBase*> inputs;

	bool playBackward;
	PlaybackBuffer playbackBuffer;
	double bufferedPlaybackPts;
	
	Metronom *metronom;
	Composer *composer;

signals:
	void modeSwitched();
	void paused( bool );
	void newFrame( Frame* );
};

#endif // SAMPLER_H
