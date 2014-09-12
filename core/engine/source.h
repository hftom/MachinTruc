#ifndef SOURCE_H
#define SOURCE_H

#include "input/input.h"
#include "engine/flist.h"
#include "afx/audiofilter.h"



class Source
{
public:
	Source( InputBase::InputType t, QString path, Profile prof );
	
	const QString & getFileName() const;
	const Profile & getProfile() const;
	InputBase::InputType getType() const;

	FList< QSharedPointer<GLFilter> > videoFilters;
	FList< QSharedPointer<AudioFilter> > audioFilters;

private:
	QString fileName;
	Profile profile;
	InputBase::InputType type;
};
#endif //SOURCE_H