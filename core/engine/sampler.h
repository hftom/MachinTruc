#ifndef SAMPLER_H
#define SAMPLER_H

#include "input/input_ff.h"
#include "input/input_gl.h"
#include "input/input_image.h"

#include "engine/scene.h"
#include "engine/metronom.h"



class Composer;



class Sampler : public QObject
{
	Q_OBJECT
public:
	Sampler();
	~Sampler();

	int getVideoTracks( Frame *dst );
	int getAudioTracks( Frame *dst, int nSamples );
	void prepareInputs();
	bool sceneEndReached();
	double currentSceneDuration();
	double currentTimelineSceneDuration();
	double currentPTS();
	double currentPTSAudio();
	void shiftCurrentPTS();
	void shiftCurrentPTSAudio();
	void rewardPTS();
	void seekTo( double p, bool backward = false );
	
	void newProject( Profile p );
	bool setProfile( Profile p );
	Profile getProfile();
	Metronom* getMetronom() { return metronom; }
	bool play( bool b, bool backward = false );
	
	bool trackRequest( bool rm, int index );
	void drainScenes();
	bool isProjectEmpty();
	QList<Scene*> getSceneList() { return sceneList; }
	Scene* getCurrentScene() { return timelineScene; }
	void setSceneList( QList<Scene*> list );
	bool previewMode() { return currentScene == preview; }

public slots:
	void setSharedContext( QGLWidget *shared );
	void setFencesContext( QGLWidget *shared );
	void switchMode( bool down );
	void setSource( Source *source, double pts );
	void wheelSeek( int a );
	void slideSeek( double p );
	void updateFrame();

private:
	void prepareInputsBackward();
	double sceneDuration( Scene *s );
	int updateLastFrame( Frame *dst );
	InputBase* getInput( QString fn, InputBase::InputType type );
	InputBase* getClipInput( Clip *c, double pts );

	QList<Scene*> sceneList;
	Scene *timelineScene;
	Scene *preview;
	Scene *currentScene;
	QList<InputBase*> inputs;

	bool playBackward;
	
	Metronom *metronom;
	Composer *composer;

signals:
	void modeSwitched();
	void paused( bool );
	void newFrame( Frame* );
};

#endif // SAMPLER_H
