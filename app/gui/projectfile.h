#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include "xmlizer.h"
#include "engine/sampler.h"



class ProjectFile
{
public:
	ProjectFile() : readError( false ) {}

	bool loadProject( QString filename );
	bool saveProject( QList<Source*> sources, Sampler *sampler, QString filename, QString backupProjectFilename = "" );

	QList<Source*> sourcesList;
	QList<Scene*> sceneList;
	QString backupFilename;
	bool readError;
	
private:
	void readSource( QDomElement &element );
	void readScene( QDomElement &element );
	void readTrack( QDomElement &element, Scene *scene, int index );
	void readClip( QDomElement &element, Scene *scene, int trackIndex );
	void readTransition( QDomElement &element, Clip *clip );
	
	void writeSource( QDomNode &parent, Source *source );
	void writeTracks( QDomNode &parent, Sampler *sampler );
	void writeClip( QDomNode &parent, Clip *clip );

	QDomDocument document;
	Profile projectProfile;
};

#endif //PROJECTFILE_H
