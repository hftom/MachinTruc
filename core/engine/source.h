#ifndef SOURCE_H
#define SOURCE_H

#include "input/input.h"
#include "engine/flist.h"
#include "afx/audiofilter.h"



class Source
{
public:
	Source( QString path );
	Source( InputBase::InputType t, QString path, Profile prof, QString nameToDisplay = "" );
	
	void use();
	void release();
	
	const QString & getFileName() const;
	const QString & getDisplayName() const;
	qint64 getSize() const;
	const Profile & getProfile() const;
	InputBase::InputType getType() const;

	FList< QSharedPointer<GLFilter> > videoFilters;
	FList< QSharedPointer<AudioFilter> > audioFilters;
	
	void setAfter( InputBase::InputType t, Profile prof );

private:
	QString fileName;
	QString displayName;
	qint64 size;
	Profile profile;
	InputBase::InputType type;
	
	// HACK for glstabilize
	int refcount;
};
#endif //SOURCE_H
