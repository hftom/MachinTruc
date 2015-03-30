#include <QtXml>

#include "engine/sampler.h"



class ProjectFile
{
public:
	ProjectFile() : readError( false ) {}

	bool loadProject( QString filename );
	bool saveProject( QList<Source*> sources, Sampler *sampler, QString filename );

	
	QList<Source*> sourcesList;
	QList<Scene*> sceneList;
	bool readError;
	
private:
	void readSource( QDomElement &element );
	void readScene( QDomElement &element );
	void readTrack( QDomElement &element, Scene *scene, int index );
	void readClip( QDomElement &element, Scene *scene, int trackIndex );
	void readTransition( QDomElement &element, Clip *clip );
	QSharedPointer<Filter> readFilter( QDomElement &element, bool audio, bool transition = false );
	void readParameter( QDomElement &element, QSharedPointer<Filter> f );
	
	void writeSource( QDomNode &parent, Source *source );
	void writeTracks( QDomNode &parent, Sampler *sampler );
	void writeClip( QDomNode &parent, Clip *clip );
	void writeFilter( QDomNode &parent, bool audio, QSharedPointer<Filter> f );
	
	void createText( QDomNode &parent, QString name, QString val );
	void createInt( QDomNode &parent, QString name, int val );
	void createDouble( QDomNode &parent, QString name, double val );
	
	QDomDocument document;
	Profile projectProfile;
};
