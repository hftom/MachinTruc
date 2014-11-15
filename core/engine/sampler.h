#ifndef SAMPLER_H
#define SAMPLER_H

#include "input/input_ff.h"
#include "input/input_gl.h"
#include "input/input_image.h"

#include "engine/scene.h"
#include "engine/metronom.h"



class Composer;



class Preview
{
public:
	Preview() : currentPTS(0), currentPTSAudio(0), input(NULL), source(NULL) {
	}
	
	bool seekTo( double p ) {
		if ( !isValid() )
			return false;
		// do not seek back beyond the probed first frame
		if ( p < profile.getStreamStartTime() ) {
			if ( currentPTS == profile.getStreamStartTime() )
				return false;
			p = profile.getStreamStartTime();
		}
		input->play( false );
		currentPTS = currentPTSAudio = input->seekTo( p );
		input->play( true );
		return true;
	}
	
	void setSource( Source *s, InputBase *in ) {
		if ( input ) {
			input->play(false);
			input->setUsed( false );
		}
		input = in;
		source = s;
		if ( source ) {
			profile = source->getProfile();
			profile.setAudioSampleRate( DEFAULTSAMPLERATE );
			profile.setAudioChannels( DEFAULTCHANNELS );
			profile.setAudioLayout( DEFAULTLAYOUT );
			profile.setAudioFormat( Profile::SAMPLE_FMT_S16 );
			
			if ( input ) {
				input->play(false);
				input->setUsed( true );
				input->setProfile( profile, profile );
				input->open( source->getFileName() );
			}
		}
		else {
			input = NULL;
			source = NULL;
			currentPTS = currentPTSAudio = 0;
		}
	}
	
	void play( bool b ) {
		if ( input )
			input->play( b );
	}
	
	double endPTS() {
		if ( !isValid() )
			return 0;
		return profile.getStreamStartTime() + profile.getStreamDuration();
	}
	
	double duration() {
		if ( !isValid() )
			return 0;
		return profile.getStreamDuration();
	}
	
	bool isValid() { return source && input; }
	const Profile & getProfile() const { return profile; }
	Source * getSource() const { return source; }
	InputBase * getInput() const { return input; }
	
	double currentPTS, currentPTSAudio;
	
private:
	InputBase *input;
	Source *source;
	Profile profile;
};



class Sampler : public QObject
{
	Q_OBJECT
public:
	Sampler();
	~Sampler();

	int getVideoTracks( Frame *dst );
	int getAudioTracks( Frame *dst, int nSamples );
	void prepareInputs();
	double getSamplerDuration();
	double currentSceneDuration();
	double getEndPTS();
	double currentPTS();
	double currentPTSAudio();
	void shiftCurrentPTS( double d = 0 );
	void shiftCurrentPTSAudio( double d = 0 );
	void rewardPTS();
	void seekTo( double p, bool backward = false );
	
	void newProject( Profile p );
	bool setProfile( Profile p );
	Profile getProfile();
	Metronom* getMetronom() { return metronom; }
	void play( bool b, bool backward = false );
	
	bool trackRequest( bool rm, int index );
	void drainScenes();
	bool isProjectEmpty();
	QList<Scene*> getSceneList() { return sceneList; }
	Scene* getCurrentScene() { return currentScene; }
	void setSceneList( QList<Scene*> list );
	bool previewMode() { return playMode == PlaySource; }

public slots:
	void setSharedContext( QGLWidget *shared );
	void setFencesContext( QGLWidget *shared );
	void switchMode( bool down );
	void setSource( Source *source, double pts );
	void wheelSeek( int a );
	void slideSeek( double p );
	void updateFrame();

private:
	enum PlayMode{ PlaySource, PlayScene };
	
	int updateLastFrame( Frame *dst );
	InputBase* getInput( QString fn, InputBase::InputType type );
	InputBase* getClipInput( Clip *c, double pts );

	QList<Scene*> sceneList;
	Scene *currentScene;
	QList<InputBase*> inputs;

	Profile projectProfile;

	bool playBackward;
	PlayMode playMode;
	Preview preview;
	
	Metronom *metronom;
	Composer *composer;

signals:
	void modeSwitched();
	void paused( bool );
	void newFrame( Frame* );
};

#endif // SAMPLER_H
